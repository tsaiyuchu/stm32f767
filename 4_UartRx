
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"

void uart3_init(void);
void uart3_write(int ch);
char uart3_read(void);
void led_init(void);
void pseudo_dly(volatile int delay);
void led_play(int value);

char rcv;
int main(){
	uart3_init();
	led_init();

	while(1){
		rcv = uart3_read();
		led_play(rcv);
	}
}

void uart3_init(void){
	/* 1.Enable clock access for uart gpio pin */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

	/* 2.Enable clock access for uart module */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

	/* 3A.Set mode of uart tx pin to alternate function */
LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
	/* 3B.Set mode of uart rx pin to alternate function */
LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);

	/* 4A.Select uart tx alternate function type */
	LL_GPIO_SetAFPin_8_15(GPIOD, LL_GPIO_PIN_8, LL_GPIO_AF_7);
	/* 4B.Select uart tx alternate function type */
	LL_GPIO_SetAFPin_8_15(GPIOD, LL_GPIO_PIN_9, LL_GPIO_AF_7);

	/* 5.Configure uart protocol parameters */
	LL_USART_Disable(USART3);
LL_USART_SetTransferDirection(USART3, LL_USART_DIRECTION_TX_RX);
LL_USART_ConfigCharacter(USART3, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
LL_USART_SetBaudRate(USART3, 16000000, LL_USART_OVERSAMPLING_16, 115200);
	LL_USART_Enable(USART3);
}

void uart3_write(int ch){
	/* Wait for TXE flag to be raise */
	while(!LL_USART_IsActiveFlag_TXE(USART3)){
		LL_USART_TransmitData8 (USART3, ch);
	}
}

char uart3_read(void){
	/* Wait for TXE flag to be raise */
	while(!LL_USART_IsActiveFlag_RXNE(USART3)){}
	return LL_USART_ReceiveData8(USART3);
}

void led_init(void){
	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
}

void pseudo_dly(volatile int delay){
	for(int dly=0; dly<delay; dly++){
		for(int itr=0; itr<5000; itr++){}
	}
}

void led_play(int value){
	value %= 16;
	for(; value>0; value--){
		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
		pseudo_dly(100);
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
		pseudo_dly(100);
	}
}
