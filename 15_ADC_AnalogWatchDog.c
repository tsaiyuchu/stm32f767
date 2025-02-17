
#include <stdio.h>
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_adc.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_system.h"

#define ADC_REF_V					((uint32_t)3300)
#define ADC_AWD_THRSH_LOW			((uint32_t)0)
#define ADC_AWD_THRSH_HIGH			(__LL_ADC_DIGITAL_SCALE(LL_ADC_RESOLUTION_12B)/2)

void adc1_init(void);
void gpio_interrupt_init(void);

volatile uint32_t awd1_status;

void uart3_init(void);
void uart3_write(int ch);

uint32_t temp_deg_celsius;
uint32_t vref_mvolt;
uint32_t sensor_mvlot;

int __io_putchar(int ch){
	uart3_write(ch);
	return ch;
}

int main(){
	adc1_init();
	uart3_init();
	gpio_interrupt_init();
	LL_ADC_REG_StartConversionSWStart(ADC1);

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

	/* Set the sequencer mode */
	LL_ADC_SetSequencersScanMode(ADC1, LL_ADC_SEQ_SCAN_ENABLE);

	/* Set trigger source */
	LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);

	/* Set continuous mode */
LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);

	/* Set sequencer length */
LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);

	/* Set sequence ranks */
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);

	/* Set sampling time for each channel */
LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_8, LL_ADC_SAMPLINGTIME_480CYCLES);

	/* Set ADC Analog Watchdog channels */
LL_ADC_SetAnalogWDMonitChannels(ADC1, LL_ADC_AWD_CHANNEL_8_REG);

	/* Set Watchdog high threshold */
LL_ADC_SetAnalogWDThresholds(ADC1, LL_ADC_AWD_THRESHOLD_HIGH, ADC_AWD_THRSH_HIGH);

	/* Set Watchdog low threshold */
LL_ADC_SetAnalogWDThresholds(ADC1, LL_ADC_AWD_THRESHOLD_LOW, ADC_AWD_THRSH_LOW);

	/* Enable analog Watchdog interrupt */
	LL_ADC_EnableIT_AWD1(ADC1);

	/* Enable ADC module */
	LL_ADC_Enable(ADC1);
}

void ADC_IRQHandler(void){
	if(LL_ADC_IsActiveFlag_AWD1(ADC1) != 0){
		awd1_status = 1;
		LL_ADC_ClearFlag_AWD1(ADC1);
		printf("Threshold has just been crossed. \n\r");
	}
}

void EXTI15_10_IRQHandler(void){
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET){
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
		awd1_status = 0;
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
	NVIC_SetPriority(EXTI15_10_IRQn, 1);
}

void uart3_write(int ch){
    /* Wait for TXE flag to be raised */
    while (!LL_USART_IsActiveFlag_TXE(USART3)){}

    /* Transmit data */
    LL_USART_TransmitData8(USART3, ch);
}
