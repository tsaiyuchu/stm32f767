
#include <stdio.h>
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_system.h"
#include "stm32f7xx_ll_spi.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_utils.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_dma.h"

#define LED_BLINK_RATE_FAST         	200
#define LED_BLINK_RATE_SLOW         	500
#define LED_BLINK_RATE_VERY_SLOW   	800
#define LED_BLINK_RATE_ERROR        	1000

uint8_t tx_buff[] = "Hello from SPI Master";
uint8_t tx_size = sizeof(tx_buff);
volatile uint8_t tx_idx = 0;

uint8_t rx_buff[sizeof(tx_buff)];
uint8_t rx_size = sizeof(tx_buff);
volatile uint8_t rx_idx = 0;
volatile uint8_t btn_press = 0;

void led_init(void);
void led_on(void);
void led_off(void);
void led_blink(uint32_t period);
void gpio_interrupt_init(void);
void wait_btn_press(void);
void uart3_init(void);
void uart3_write(int ch);
void spi1_master_init(void);
void spi2_slave_init(void);
void spi1_activate(void);
void spi2_activate(void);
void wait_for_end_of_tx(void);

int __io_putchar(int ch){
    uart3_write(ch);
    return ch;
}

/* Set Up
 * Connect PB3 to PB13 using a jumper wire
 * Connect PB5 to PB14 using a jumper wire */
int main(){
	SysTick_Config(16000);
	led_init();
	spi1_master_init();
	spi2_slave_init();
	gpio_interrupt_init();
	uart3_init();
	spi2_activate();
	wait_btn_press();
	spi1_activate();
	wait_for_end_of_tx();

	while(1){}
}

void spi1_master_init(void){
	/* Enable clock port of spi pins */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// SCK : PB3
	/* Set pin mode */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);
	/* Select spi alt function */
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_3, LL_GPIO_AF_5);
	/* Set pin pull */
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_3, LL_GPIO_PULL_DOWN);
	/* Set pin speed */
LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_3, LL_GPIO_SPEED_FREQ_HIGH);

	// MOSI : PB5
	/* Set pin mode */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
	/* Select spi alt function */
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_5, LL_GPIO_AF_5);
	/* Set pin pull */
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_5, LL_GPIO_PULL_DOWN);
	/* Set pin speed */
LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_HIGH);

	/* Enable NVIC */
	NVIC_SetPriority(SPI1_IRQn, 0);
	NVIC_EnableIRQ(SPI1_IRQn);

	/* Enable clock access to spi */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

	/* Set Baudrate Prescaler */
LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV256);

	/* Set transfer direction */
	LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);

	/* Set clock phase */
	LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_2EDGE);

	/* Set clock polarity */
	LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_HIGH);

	/* Set data width */
	LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);

	/* Set NSS mode */
	LL_SPI_SetNSSMode(SPI1, LL_SPI_NSS_SOFT);

	/* Set SPI mode */
	LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);

	/* Enable TXE Interrupt */
	LL_SPI_EnableIT_TXE(SPI1);

	/* Enable Error Interrupt */
	LL_SPI_EnableIT_ERR(SPI1);
}

void spi2_slave_init(void){
	/* Enable clock port of spi pins */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// SCK : PB13
	/* Set pin mode */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_13, LL_GPIO_MODE_ALTERNATE);
	/* Select spi alt function */
	LL_GPIO_SetAFPin_8_15(GPIOB, LL_GPIO_PIN_13, LL_GPIO_AF_5);
	/* Set pin pull */
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_13, LL_GPIO_PULL_DOWN);
	/* Set pin speed */
LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_13, LL_GPIO_SPEED_FREQ_HIGH);

	// MISO : PB14
	/* Set pin mode */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_14, LL_GPIO_MODE_ALTERNATE);
	/* Select spi alt function */
	LL_GPIO_SetAFPin_8_15(GPIOB, LL_GPIO_PIN_14, LL_GPIO_AF_5);
	/* Set pin pull */
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_14, LL_GPIO_PULL_DOWN);
	/* Set pin speed */
LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_14, LL_GPIO_SPEED_FREQ_HIGH);

	/* Enable NVIC */
	NVIC_SetPriority(SPI2_IRQn, 0);
	NVIC_EnableIRQ(SPI2_IRQn);

	/* Enable clock access to spi */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

	/* Set Baudrate Prescaler */
LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV256);

	/* Set transfer direction */
	LL_SPI_SetTransferDirection(SPI2, LL_SPI_HALF_DUPLEX_RX);

	/* Set clock phase */
	LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_2EDGE);

	/* Set clock polarity */
	LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_HIGH);

	/* Set data width */
	LL_SPI_SetDataWidth(SPI2, LL_SPI_DATAWIDTH_8BIT);

	/* Set NSS mode */
	LL_SPI_SetNSSMode(SPI2, LL_SPI_NSS_SOFT);

	/* Set SPI mode */
	LL_SPI_SetMode(SPI2, LL_SPI_MODE_SLAVE);

	/* Enable RXNE Interrupt */
	LL_SPI_EnableIT_RXNE(SPI2);

	/* Enable Error Interrupt */
	LL_SPI_EnableIT_ERR(SPI2);
}

void spi1_activate(void){
	LL_SPI_Enable(SPI1);
}

void spi2_activate(void){
	LL_SPI_Enable(SPI2);
}

void spi1_tx_callback(void){
    LL_SPI_TransmitData8(SPI1, tx_buff[tx_idx++]);
}

void spi2_rx_callback(void){
    rx_buff[rx_idx++] = LL_SPI_ReceiveData8(SPI2);
}

void spi_err_callback(void){
    LL_SPI_DisableIT_TXE(SPI1);
    LL_SPI_DisableIT_RXNE(SPI2);

    led_blink(LED_BLINK_RATE_ERROR);
}

uint8_t cmp_8bit_buff(uint8_t* buff1, uint8_t* buff2, uint8_t buff_len){
    while(buff_len--){
        if(*buff1 != *buff2){
            return 0;
        }
        buff1++;
        buff2++;
    }
    return 1;
}

void wait_for_end_of_tx(void){
    while(tx_idx != tx_size){}
    LL_SPI_DisableIT_TXE(SPI1);

//printf("Before waiting, rx_idx: %d, rx_size: %d\n", rx_idx, rx_size);
    while(rx_size > rx_idx){} // rx_idx+1
//printf("After waiting, rx_idx: %d, rx_size: %d\n", rx_idx, rx_size);

    LL_SPI_DisableIT_RXNE(SPI2);

if(cmp_8bit_buff((uint8_t *)tx_buff, (uint8_t *)rx_buff, tx_size)){
        printf("MATCH SUCCESS! \n\r");
        led_on();
    } else{
        printf("MATCH FAILURE! \n\r");
        led_blink(LED_BLINK_RATE_ERROR);
    }
}

void SPI1_IRQHandler(void){
	if(LL_SPI_IsActiveFlag_TXE(SPI1)){
		spi1_tx_callback();
	}
}

void SPI2_IRQHandler(void){
	if(LL_SPI_IsActiveFlag_RXNE(SPI2)){
		spi2_rx_callback();
	} else if(LL_SPI_IsActiveFlag_OVR(SPI2)){
		spi_err_callback();
	}
}

void SysTick_Handler(void){

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
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
}

void btn_callback(void){
    btn_press = 1;
    printf("----------- btn press ----------- \n\r");
}

/* Interrupt Request Handler for EXTI13 */
void EXTI15_10_IRQHandler(void){
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET){
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        btn_callback();
    }
}
