#include <stdio.h>
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_rcc.h"
#include "stm32f7xx_ll_system.h"
#include "stm32f7xx_ll_utils.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_pwr.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_rtc.h"
#include "stm32f7xx_ll_usart.h"
#include "stm32f7xx_ll_cortex.h"

#define LED_BLINK_RATE_FAST         	200
#define LED_BLINK_RATE_SLOW         	500
#define LED_BLINK_RATE_VERY_SLOW    	800
#define LED_BLINK_RATE_ERROR        	1000

#define RTC_ASYNCH_PREDIV			((uint32_t)0x7F)
#define RTC_SYNCH_PREDIV			((uint32_t)0xF9)

// Wakeup time value
#define RTC_WUT_TIME    ((uint32_t)5)    // 5 seconds

void rtc_init(void);
void enter_standby_mode(void);
void led_init(void);
void led_on(void);
void led_off(void);
void led_blink(uint32_t period);
void gpio_interrupt_init(void);
void wait_btn_press(void);
void uart3_init(void);
void uart3_write(int ch);

volatile uint8_t btn_press = 0;

int __io_putchar(int ch){
    uart3_write(ch);
    return ch;
}

int main(){
    SysTick_Config(16000);
    led_init();
    led_off();
	gpio_interrupt_init();
    uart3_init();
    rtc_init();

	// Check and handle if the system was resumed from StandBy mode
	if(LL_PWR_IsActiveFlag_SB() != 1){
	    // Run after normal reset
	    wait_btn_press();
	    printf("System going into standby...\n\r");
	    // Enable wake-up timer and enter in standby mode
	    enter_standby_mode();
	} else{
	    // Run after standby mode

	    // Clear Standby flag
	    LL_PWR_ClearFlag_SB();

	    // Reset RTC Internal Wake up flag
	    LL_RTC_ClearFlag_WUT(RTC);
	    printf("----System back from standby!!!------\n\r");

	    // Slow Toggle LED
	    led_blink(LED_BLINK_RATE_SLOW);
	}
}

void rtc_init(void){
    // Enable the PWR clock
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    // Enable write access to config RTC clock src
    LL_PWR_EnableBkUpAccess();

    // Enable LSI
    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() != 1){
        // Wait for LSI to be ready
    }
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);

    // Enable RTC Clock
    LL_RCC_EnableRTC();

    // Disable RTC write protection
    LL_RTC_DisableWriteProtection(RTC);

    // Set Asynch PRE_DIV
    LL_RTC_SetAsynchPrescaler(RTC, RTC_ASYNCH_PREDIV);

    // Set Synch PRE_DIV
    LL_RTC_SetSynchPrescaler(RTC, RTC_SYNCH_PREDIV);

    // Disable Wakeup timer in order to modify it
    LL_RTC_WAKEUP_Disable(RTC);

    while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1){
        // Wait until the WUTW flag is set
    }

    // Load Wakeup time in seconds
    LL_RTC_WAKEUP_SetAutoReload(RTC, RTC_WUT_TIME);

    // Set clock to CKSPRE to make the frequency 1Hz
    LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_CKSPRE);

    // Enable RTC registers write protection
    LL_RTC_EnableWriteProtection(RTC);
}

void enter_standby_mode(void){
    // Disable RTC write protection
    LL_RTC_DisableWriteProtection(RTC);

    // Enable Wakeup Interrupt
    LL_RTC_EnableIT_WUT(RTC);

    // Enable Wakeup Timer
    LL_RTC_WAKEUP_Enable(RTC);

    // Enable RTC registers write protection
    LL_RTC_EnableWriteProtection(RTC);

    // Reset Internal Wake up flag
    LL_RTC_ClearFlag_WUT(RTC);

    // Set Stand-by mode
    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);

    // Set SLEEPDEEP bit of Cortex-M System Control Register
    LL_LPM_EnableDeepSleep();

    // Wait For Interrupt
    __WFI();
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

void EXTI15_10_IRQHandler(void){
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET){
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        btn_callback();
    }
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

void SysTick_Handler(void){

}
