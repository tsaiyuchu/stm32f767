#include <stdio.h>
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_adc.h"
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_dma.h"

#define NUM_OF_SENSORS				((uint8_t)3)
#define ADC_REF_V					((uint32_t)3300)

void adc1_init(void);
void dma_init(void);

uint32_t volatile sensor_value = 0;

volatile uint16_t raw_sensor_data[NUM_OF_SENSORS];
void uart3_init(void);
void uart3_write(int ch);

uint8_t dma_transfer_status;

uint32_t temp_deg_celsius;
uint32_t vref_mvolt;
uint32_t sensor_mvolt;

int __io_putchar(int ch){
	uart3_write(ch);
	return ch;
}

int main(){
	dma_transfer_status = 0;
	adc1_init();
	dma_init();
	uart3_init();

	while(1){
		LL_ADC_REG_StartConversionSWStart(ADC1);

		while(dma_transfer_status != 1){}
sensor_mvolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(ADC_REF_V, raw_sensor_data[0], LL_ADC_RESOLUTION_12B);
vref_mvolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(ADC_REF_V, raw_sensor_data[1], LL_ADC_RESOLUTION_12B);
temp_deg_celsius = __LL_ADC_CALC_TEMPERATURE(ADC_REF_V, raw_sensor_data[2], LL_ADC_RESOLUTION_12B);

printf("The temperature value : %d\n\r", temp_deg_celsius);
printf("The VREFINT voltage value in mvolts : %d\n\r", vref_mvolt);
printf("The sensor voltage value in mvolts : %d\n\r", sensor_mvolt);

		dma_transfer_status = 0;
	}
}

void adc1_init(void){ //PB0 ADC1_IN8
	// Enable clock access to adc module
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

	// Enable clock access to adc channel port
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// Set adc channel pin mode to analog
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);

	NVIC_SetPriority(ADC_IRQn, 0);
	NVIC_EnableIRQ(ADC_IRQn);

	/* Set ADC clock */
LL_ADC_SetCommonClock(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_CLOCK_SYNC_PCLK_DIV2);

	/* Select ADC internal channels- temp sensor and vref */
LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT |LL_ADC_PATH_INTERNAL_TEMPSENSOR);

	/* Set the sequencer mode */
	LL_ADC_SetSequencersScanMode(ADC1, LL_ADC_SEQ_SCAN_ENABLE);

	/* Set trigger source */
	LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);

	/* Set continuous mode */
	LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);

	/* Enable DMA transfer */
LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);

	/* Set sequencer length */
LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS);

	/* Set sequence ranks */
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_VREFINT);
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_TEMPSENSOR);

	/* Set sampling time for each channel */
LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_8, LL_ADC_SAMPLINGTIME_480CYCLES);
LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_VREFINT, LL_ADC_SAMPLINGTIME_480CYCLES);
LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_TEMPSENSOR, LL_ADC_SAMPLINGTIME_480CYCLES);

	/* Enable EOCS interrupt */
	LL_ADC_EnableIT_EOCS(ADC1);

	/* Enable ADC OVR interrupt */
	LL_ADC_EnableIT_OVR(ADC1);

	/* Enable ADC module */
	LL_ADC_Enable(ADC1);
}

void dma_init(void){
	/* Set DMA interrupt priority */
	NVIC_SetPriority(DMA2_Stream0_IRQn, 1);

	/* Enable DMA interrupt */
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	/* Enable clock access to the DMA */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

	/* Select the DMA channel */
	LL_DMA_SetChannelSelection(DMA2,
	                           LL_DMA_STREAM_0,
	                           LL_DMA_CHANNEL_0);

	/* Configure DMA transfer */
	LL_DMA_ConfigTransfer(DMA2,
	                      LL_DMA_STREAM_0,
	                      LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
	                      LL_DMA_MODE_CIRCULAR |
	                      LL_DMA_PERIPH_NOINCREMENT |
	                      LL_DMA_MEMORY_INCREMENT |
	                      LL_DMA_PDATAALIGN_HALFWORD |
	                      LL_DMA_MDATAALIGN_HALFWORD |
	                      LL_DMA_PRIORITY_HIGH);

	/* Configure DMA src and dst addresses */
	LL_DMA_ConfigAddresses(DMA2,
	                       	LL_DMA_STREAM_0,
LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
	                       (uint32_t)&raw_sensor_data,
	                       LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

	/* Set DMA transfer length */
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, NUM_OF_SENSORS);

	/* Enable DMA transfer complete interrupt */
	LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);

	/* Enable the transfer error interrupt */
	LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);

	/* Enable the DMA stream */
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
}

void ADC_IRQHandler(void){
	if(LL_ADC_IsActiveFlag_OVR(ADC1) != 0){
		printf("ADC Overrun \n\r");
	} else {
		/* Clear flag */
	    LL_ADC_ClearFlag_EOCS(ADC1);

	    printf("ADC end-of-conversion \n\r");
	}
}

void DMA2_Stream0_IRQHandler(void){
    if(LL_DMA_IsActiveFlag_TC0(DMA2) == 1){
        LL_DMA_ClearFlag_TC0(DMA2);
        /* Do something... */
        printf("DMA transfer complete \n\r");
        dma_transfer_status = 1;
    }
    if(LL_DMA_IsActiveFlag_TE0(DMA2) == 1){
        LL_DMA_ClearFlag_TE0(DMA2);
        /* Do something... */
        printf("DMA transfer error \n\r");
    }
}

void uart3_init(void){
	/* 1.Enable clock access for uart gpio pin */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

	/* 2.Enable clock access for uart module */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

	/* 3.Set mode of uart tx pin to alternate function */
LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);

	/* 4.Select uart tx alternate function type */
	LL_GPIO_SetAFPin_8_15(GPIOD, LL_GPIO_PIN_8, LL_GPIO_AF_7);

	/* 5.Configure uart protocol parameters */
	LL_USART_Disable(USART3);
	LL_USART_SetTransferDirection(USART3, LL_USART_DIRECTION_TX);
LL_USART_ConfigCharacter(USART3, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
LL_USART_SetBaudRate(USART3, 16000000, LL_USART_OVERSAMPLING_16, 115200);
	LL_USART_Enable(USART3);
}

void uart3_write(int ch){
    /* Wait for TXE flag to be raised */
    while (!LL_USART_IsActiveFlag_TXE(USART3)){}

    /* Transmit data */
    LL_USART_TransmitData8(USART3, ch);
}
