
#include <stdio.h>
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_adc.h"
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_dma.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_system.h"
#include "stm32f7xx_ll_tim.h"
#include "stm32f7xx_ll_rcc.h"

#define SYS_CLOCK					((uint32_t)16000000)
#define TIMER_PRESCALER_MAX_VAL		((uint32_t)0xFFFF-1)
#define TIMER_FREQ					1000  // Hz
#define TIMER_FREQ_RANGE_MIN		((uint32_t)1)

#define DATA_LENGTH					((uint8_t)3)
#define ADC_REF_V					((uint32_t)3300)

void adc1_init(void);
void dma_init(void);
void uart3_init(void);
void uart3_write(int ch);
void gpio_interrupt_init(void);
void timer_trig_adc_init(void);

uint32_t volatile sensor_value = 0;

volatile uint16_t raw_sensor_data[DATA_LENGTH];
volatile uint32_t inj_ch_sensor_data;

uint32_t temp_deg_celsius;
uint32_t vref_mvolt;
uint32_t sensor_mvolt;

int __io_putchar(int ch){
	uart3_write(ch);
	return ch;
}

const uint8_t APBPrescTable[8] = {0, 0, 0, 0, 1, 2, 3, 4};

int main(){
	adc1_init();
	dma_init();
	uart3_init();
	gpio_interrupt_init();
	timer_trig_adc_init();

LL_ADC_REG_StartConversionExtTrig (ADC1, LL_ADC_REG_TRIG_EXT_RISING);
	
	while(1){}
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
LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_TEMPSENSOR);

	/* Set trigger source */
LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_EXT_TIM2_TRGO);

	/* Set continuous mode */
	LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);

	/* Enable DMA transfer */
LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);

	/* Set REG sequencer length */
LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);

	/* Set REG sequence ranks */
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);

	/* Set sampling time for each channel */
LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_8, LL_ADC_SAMPLINGTIME_480CYCLES);

	/* Set INJ trigger source */
	LL_ADC_INJ_SetTriggerSource(ADC1, LL_ADC_INJ_TRIG_SOFTWARE);

	/* Set INJ sequencer length */
LL_ADC_INJ_SetSequencerLength(ADC1, LL_ADC_INJ_SEQ_SCAN_DISABLE);

	/* Set INJ sequence ranks */
LL_ADC_INJ_SetSequencerRanks(ADC1, LL_ADC_INJ_RANK_1, LL_ADC_CHANNEL_TEMPSENSOR );

	/* Set sampling time for each channel */
LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_TEMPSENSOR , LL_ADC_SAMPLINGTIME_480CYCLES);

	/* Enable JEOS interrupt */
	LL_ADC_EnableIT_JEOS(ADC1);

	/* Enable ADC OVR interrupt */
	LL_ADC_EnableIT_OVR(ADC1);

	/* Enable ADC module */
	LL_ADC_Enable(ADC1);
}

void timer_trig_adc_init(void){
	uint32_t tim_clk_frq = 0;
	uint32_t tim_prescaler = 0;
	uint32_t tim_reload = 0;

	if(LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_1){
tim_clk_frq = __LL_RCC_CALC_PCLK1_FREQ(SYS_CLOCK, LL_RCC_GetAPB1Prescaler());
	} else{
tim_clk_frq = (__LL_RCC_CALC_PCLK1_FREQ(SYS_CLOCK, LL_RCC_GetAPB1Prescaler())*2);
	}
tim_prescaler = (tim_clk_frq / (TIMER_PRESCALER_MAX_VAL * TIMER_FREQ_RANGE_MIN))+1;
	tim_reload = (tim_clk_frq / (tim_prescaler * TIMER_FREQ));

	// Enable clock access to timer module
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

	// Set the pre-scaler
	LL_TIM_SetPrescaler(TIM2, (tim_prescaler-1));

	// Set auto-reload value
	LL_TIM_SetAutoReload(TIM2, (tim_reload-1));

	// Set counter mode
	LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);

	// Set repetition
	LL_TIM_SetRepetitionCounter(TIM2, 0);

	// Set trigger output
	LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_UPDATE);

	// Enable timer
	LL_TIM_EnableCounter(TIM2);
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
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, DATA_LENGTH);

	/* Enable DMA transfer complete interrupt */
	LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);

	/* Enable the transfer error interrupt */
	LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);

	/* Enable the DMA stream */
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
}

void gpio_interrupt_init(void){
	/* Enable clock for GPIOC */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

	/* Set PC13 as input pin */
LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);

	/* Enable clock access to EXTI module */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

	/* Set EXTI source as PC13 */
LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

	/* Enable EXTI line */
	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_13);

	/* Set the trigger */
	LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_13);

	/* Enable NVIC interrupt */
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	/* Set the interrupt priority */
	NVIC_SetPriority(EXTI15_10_IRQn, 2);
}

void ADC_IRQHandler(void){
	if(LL_ADC_IsActiveFlag_OVR(ADC1) != 0){
		printf("ADC Overrun \n\r");
	}
	if(LL_ADC_IsActiveFlag_JEOS(ADC1) != 0){
		/* Clear flag */
	    LL_ADC_ClearFlag_JEOS(ADC1);

		// Do something
inj_ch_sensor_data = LL_ADC_INJ_ReadConversionData32(ADC1, LL_ADC_INJ_RANK_1);
temp_deg_celsius = __LL_ADC_CALC_TEMPERATURE(ADC_REF_V, inj_ch_sensor_data, LL_ADC_RESOLUTION_12B);
	    printf("The temperature : %ld \n\r", temp_deg_celsius);
	} else {
		/* Clear flag */
	    LL_ADC_ClearFlag_EOCS(ADC1);

	    printf("ADC end-of-conversion \n\r");
	}
}

void EXTI15_10_IRQHandler(void){
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET){
		LL_ADC_INJ_StartConversionSWStart(ADC1);
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
	}
}

void DMA2_Stream0_IRQHandler(void){
    if(LL_DMA_IsActiveFlag_TC0(DMA2) == 1){
        LL_DMA_ClearFlag_TC0(DMA2);
        /* Do something... */
        //printf("DMA transfer complete \n\r");
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
