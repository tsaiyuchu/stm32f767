
#include <stdio.h>
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_adc.h"

void adc1_init(void);
void uart3_init(void);
void uart3_write(int ch);

int __io_putchar(int ch){
	uart3_write(ch);
	return ch;
}

uint32_t volatile sensor_value = 0;

int main(){
	adc1_init();
	uart3_init();
	while(1){
		// Start adc conversion
		LL_ADC_REG_StartConversionSWStart(ADC1);

		// Wait for conversion to be complete
		while(!LL_ADC_IsActiveFlag_EOCS(ADC1)){}

		// Read converted value
		sensor_value = LL_ADC_REG_ReadConversionData32(ADC1);
		printf("The sensor value is : %d\n\r", (int)sensor_value);
	}
}

void adc1_init(void){ //PB0 ADC1_IN8
	/*1. Enable clock access to ADC module*/
	LL_APB2_GRP1_EnableClock ( LL_APB2_GRP1_PERIPH_ADC1);

	/*2. Enable clock access to ADC channel port*/
	LL_AHB1_GRP1_EnableClock (LL_AHB1_GRP1_PERIPH_GPIOB);

	/*3. Set ADC channel pin mode to analog*/
LL_GPIO_SetPinMode (GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);

	/*4. Set ADC trigger*/
	LL_ADC_REG_SetTriggerSource (ADC1, LL_ADC_REG_TRIG_SOFTWARE);

	/*5. Set sampling sequence*/
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);

	/*6. sequence length*/
LL_ADC_REG_SetSequencerLength (ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);

	/*7. Enable ADC*/
	LL_ADC_Enable (ADC1);
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
