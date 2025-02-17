
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_utils.h"
#include "stm32f7xx_ll_dma.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_system.h"

#define LED_BLINK_RATE_FAST			200
#define LED_BLINK_RATE_SLOW			500
#define LED_BLINK_RATE_VERY_SLOW		800
#define LED_BLINK_RATE_ERROR			1000

const uint8_t tx_buffer[] = "Hello from STM32F7, low level coding is awesome! -Type 'AGREED'-\r\n";
uint8_t tx_data_size = sizeof(tx_buffer);
volatile uint8_t tx_cmplt = 0;

uint8_t result;

const uint8_t expected_string[] = "AGREED";
uint8_t expected_string_size = sizeof(expected_string)-1;

uint8_t rx_buffer[sizeof(expected_string)-1];
volatile uint8_t rx_cmplt = 0;

volatile uint8_t btn_press = 0;
volatile uint8_t send = 0;

void uart3_init(void);
void dma_init(void);
void start_transfer(void);
void led_init(void);
void led_on(void);
void led_off(void);
void led_blink(uint32_t period);
void gpio_interrupt_init(void);
void wait_btn_press(void);
void wait_txrx_cmplt(void);

uint8_t cmp_8bit_buff(uint8_t* buff1, uint8_t* buff2, uint8_t buff_len);

int main(void) {
	/* Set systick to lms */
	SysTick_Config(16000);

    led_init();
    gpio_interrupt_init();
    uart3_init();
    dma_init();

    /* Wait for push button press */
    wait_btn_press();
    start_transfer();

    /* Wait for transfer completion */
    wait_txrx_cmplt();

    while(1) {}
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

void dma_init(void){ // USART3_TX Stream4 Ch7 | USART3_RX Stream1 Ch4
	/* Enable clock access to dma */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

	/* Set dma NVIC priority */
	NVIC_SetPriority(DMA1_Stream4_IRQn, 0);
	NVIC_SetPriority(DMA1_Stream1_IRQn, 0);

	/* Enable dma NVIC IRQn */
	NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	NVIC_EnableIRQ(DMA1_Stream1_IRQn);

	/* Select dma channels */
LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_4, LL_DMA_CHANNEL_7);

	/* Configure dma transfer */
	LL_DMA_ConfigTransfer(DMA1, LL_DMA_STREAM_4,
							LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
							LL_DMA_PRIORITY_HIGH |
							LL_DMA_MODE_NORMAL |
							LL_DMA_PERIPH_NOINCREMENT |
							LL_DMA_MEMORY_INCREMENT |
							LL_DMA_PDATAALIGN_BYTE |
							LL_DMA_MDATAALIGN_BYTE);

	/* Configure transfer address and direction */
	LL_DMA_ConfigAddresses(DMA1,
	                       LL_DMA_STREAM_4,
	                       (uint32_t)tx_buffer,
LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_TRANSMIT),	                       LL_DMA_GetDataTransferDirection(DMA1,LL_DMA_STREAM_4));

	/* Set the data length */
	LL_DMA_SetDataLength(DMA1,LL_DMA_STREAM_4,tx_data_size);

	/* Rx Configuration */

	/* Select dma channels */
LL_DMA_SetChannelSelection(DMA1,LL_DMA_STREAM_1,LL_DMA_CHANNEL_4);

	/* Configure dma transfer */
	LL_DMA_ConfigTransfer(DMA1,LL_DMA_STREAM_1,
	                      LL_DMA_DIRECTION_PERIPH_TO_MEMORY |
	                      LL_DMA_PRIORITY_HIGH |
	                      LL_DMA_MODE_NORMAL |
	                      LL_DMA_PERIPH_NOINCREMENT |
	                      LL_DMA_MEMORY_INCREMENT |
	                      LL_DMA_PDATAALIGN_BYTE |
	                      LL_DMA_MDATAALIGN_BYTE);

	/* Configure transfer address and direction */
	LL_DMA_ConfigAddresses(DMA1,
		                   	LL_DMA_STREAM_1,
LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_RECEIVE),
		                   	(uint32_t)rx_buffer,
LL_DMA_GetDataTransferDirection(DMA1,LL_DMA_STREAM_1));

	/* Set the data length */
LL_DMA_SetDataLength(DMA1,LL_DMA_STREAM_1, expected_string_size);

	/* Enable dma interrupts */
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_1);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_1);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_4);
	LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_4);
}

void start_transfer(void){
	/* Enable UART DMA RX Interrupt */
	LL_USART_EnableDMAReq_RX(USART3);

	/* Enable UART DMA TX Interrupt */
	LL_USART_EnableDMAReq_TX(USART3);

	/* Enable DMA stream4 */
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_4);

	/* Enable DMA stream1 */
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
}

void led_init(void){
	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
}

void led_on(void){
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
}

void led_off(void){
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
}

void led_blink(uint32_t period){
	while(1){
		/* Toggle PB0 */
		LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
		LL_mDelay(period);
	}
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
	NVIC_SetPriority(EXTI15_10_IRQn, 3);
}

void wait_btn_press(void){
	while(btn_press == 0){
		/* Toggle PB0 */
		LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
		LL_mDelay(LED_BLINK_RATE_FAST);
	}
	led_off();
}

void wait_txrx_cmplt(void){
	/* Wait for end transmission */
	while(tx_cmplt == 0){}
	/* Disable DMA1 Tx channel */
	LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_4);

	/* Wait for end transmission */
	while(rx_cmplt == 0){}
	/* Disable DMA1 Rx channel */
	LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);

	/*Compare received string and expected string*/
result = cmp_8bit_buff((uint8_t*) rx_buffer, (uint8_t*) expected_string, expected_string_size);
	if(result){
		/* The two buffers contain the same string */
		led_on();
	} else{
		led_blink(LED_BLINK_RATE_ERROR);
	}
}

uint8_t cmp_8bit_buff(uint8_t* buff1, uint8_t* buff2, uint8_t buff_len){
    while(buff_len--) {
        if(*buff1 != *buff2) {
            return 0;
        }
        buff1++;
        buff2++;
    }
    return 1;
}

void user_btn_callback(void){
	btn_press = 1;
}

void dma1_tx_complete_callback(void){
	tx_cmplt = 1;
}

void dma1_rx_complete_callback(void){
	rx_cmplt = 1;
}

void uart_transfer_err_callback(void){
	LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_4);
	LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);
	led_blink(LED_BLINK_RATE_ERROR);
}

void DMA1_Stream4_IRQHandler(void){ //Tx
	if(LL_DMA_IsActiveFlag_TC4(DMA1)){
	    LL_DMA_ClearFlag_TC4(DMA1);
	    dma1_tx_complete_callback();
	} else if(LL_DMA_IsActiveFlag_TE4(DMA1)){
	    LL_DMA_ClearFlag_TE4(DMA1);
	    uart_transfer_err_callback();
	} else{
		// Do something...
	}
}

void DMA1_Stream1_IRQHandler(void){ //Rx
	if(LL_DMA_IsActiveFlag_TC1(DMA1)) {
	    LL_DMA_ClearFlag_TC1(DMA1);
	    dma1_rx_complete_callback();
	} else if(LL_DMA_IsActiveFlag_TE1(DMA1)) {
	    LL_DMA_ClearFlag_TE1(DMA1);
	    uart_transfer_err_callback();
	} else{
		// Do something...
	}
}

void EXTI15_10_IRQHandler(void){
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET){
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
		user_btn_callback();
	}
}

void SysTick_Handler(void){

}
