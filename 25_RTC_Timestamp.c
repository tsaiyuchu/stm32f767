
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

#define RTC_ERROR_NONE		0

uint8_t time_buffer[20] = {0};
uint8_t timestamp_buffer[20] = {0};
uint8_t date_buffer[20] = {0};
uint8_t datestamp_buffer[20] = {0};

void rtc_init(void);
uint32_t Enter_RTC_InitMode(void);
uint32_t Exit_RTC_InitMode(void);
uint32_t wait_for_sync_rtc(void);
void rtc_timestamp_init(void);
void display_rtc_calendar(void);

void led_init(void);
void led_on(void);
void led_off(void);
void led_blink(uint32_t period);
void uart3_init(void);
void uart3_write(int ch);
void gpio_btn_init(void);

int __io_putchar(int ch){
    uart3_write(ch);
    return ch;
}

int main(){
    SysTick_Config(16000);
    led_init();
    gpio_btn_init();
    uart3_init();
    rtc_init();
    rtc_timestamp_init();

    //printf("Testing RTC Alarm IRQ handler directly\n\r");

    while(1){
        display_rtc_calendar();
    }
}

void rtc_init(void){
    // Enable PWR clock
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    // Enable Backup Access to config RTC
    LL_PWR_EnableBkUpAccess();

    // Enable LSI
    LL_RCC_LSI_Enable();
    while(LL_RCC_LSI_IsReady() != 1){}

    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
    LL_RCC_EnableRTC();

    NVIC_SetPriority(TAMP_STAMP_IRQn, 0x0F);
    NVIC_EnableIRQ(TAMP_STAMP_IRQn);

    // Disable RTC registers write protection
    LL_RTC_DisableWriteProtection(RTC);

    // Enter in initialization mode
    if (Enter_RTC_InitMode() != RTC_ERROR_NONE){
        led_blink(LED_BLINK_RATE_ERROR);
    }

    // Set Hour Format
    LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_AMPM);

    // Set Asynch Prescaler according to source clock
    LL_RTC_SetAsynchPrescaler(RTC, RTC_ASYNCH_PREDIV);

    // Set Synch Prescaler according to value according to source clock
    LL_RTC_SetSynchPrescaler(RTC, RTC_SYNCH_PREDIV);

    // Exit of initialization mode
    Exit_RTC_InitMode();

    // Enable RTC registers write protection
    LL_RTC_EnableWriteProtection(RTC);

}

uint32_t Enter_RTC_InitMode(void){
	LL_RTC_EnableInitMode(RTC);
	while(LL_RTC_IsActiveFlag_INIT(RTC) != 1){}

	return RTC_ERROR_NONE;
}

uint32_t Exit_RTC_InitMode(void){
	LL_RTC_DisableInitMode(RTC);
	return (wait_for_sync_rtc());
}

uint32_t wait_for_sync_rtc(void){
	// Clear RSF flag
	LL_RTC_ClearFlag_RS(RTC);

	// Wait for the registers to be synchronised
	while(LL_RTC_IsActiveFlag_RS(RTC) != 1){}

	return RTC_ERROR_NONE;
}

void rtc_timestamp_init(void){
    // Disable RTC registers write protection
    LL_RTC_DisableWriteProtection(RTC);

    // Set RTC TimeStamp generation to rising edge
    // Timestamp pin is connected to PC13 which is RTC_AF1
    // See ref. manual page 156
    LL_RTC_TS_SetActiveEdge(RTC, LL_RTC_TIMESTAMP_EDGE_RISING);

    LL_RTC_TS_Enable(RTC);

    // Enable IT timestamp
    LL_RTC_EnableIT_TS(RTC);

    // Configure RTC timestamp EXTI Interrupt, see ref manual
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_21);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_21);

    // Enter in initialization mode
    if (Enter_RTC_InitMode() != RTC_ERROR_NONE){
        led_blink(LED_BLINK_RATE_ERROR);
    }

    // Set Day: Friday July 26th 2024
LL_RTC_DATE_Config(RTC, LL_RTC_WEEKDAY_FRIDAY, 0x26, LL_RTC_MONTH_JULY, 0x24);

    // Set time: 11:59:55 PM
LL_RTC_TIME_Config(RTC, LL_RTC_TIME_FORMAT_PM, 0x11, 0x59, 0x55);

    // Exit of initialization mode
    if (Exit_RTC_InitMode() != RTC_ERROR_NONE){
        // Initialization Error
        led_blink(LED_BLINK_RATE_ERROR);
    }

    // Enable RTC registers write protection
    LL_RTC_EnableWriteProtection(RTC);
}

void gpio_btn_init(void){
    // Enable clock for GPIOC
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    // Set PC13 as input pin
LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_INPUT);

    // Set pull
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_13, LL_GPIO_PULL_NO);
}

void display_rtc_calendar(void){
	// Display time in hh:mm:ss
	sprintf((char *) time_buffer, "%.2d:%.2d:%.2d",
		__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC)),
		__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC)),
		__LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC)));

	// Display date in mm-dd-yyyy
	sprintf((char *) date_buffer, "%.2d-%.2d-%.2d",
		__LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC)),
		__LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC)),
2000 + __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC)));
}

void timestamp_event_callback(void){
	/* Display time Format : hh:mm:ss */
	sprintf((char*)timestamp_buffer,"%.2d:%.2d:%.2d",
	        __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetHour(RTC)),
	        __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetMinute(RTC)),
	        __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetSecond(RTC)));

	/* Display date Format : mm-dd */
	sprintf((char*)datestamp_buffer,"%.2d-%.2d-%.2d",
	        __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetDate(RTC)),
	        __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetMonth(RTC)),
2000 + __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC)));

	printf("PING : Event just occurred!!\n\r");

	printf("Date : %.2d:%.2d:%.2d\n\r",
	       __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetHour(RTC)),
	       __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetMinute(RTC)),
	       __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetSecond(RTC)));

	printf("Time : %.2d-%.2d-%.2d\n\r",
	       __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetMonth(RTC)),
	       __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TS_GetDate(RTC)),
2000 + __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC)));
}

void TAMP_STAMP_IRQHandler(void){
    // Get the TimeStamp interrupt source enable status
    if(LL_RTC_IsEnabledIT_TS(RTC) != 0){
        // Get the pending status of the TIMESTAMP Interrupt
        if(LL_RTC_IsActiveFlag_TS(RTC) != 0){
            // TIMESTAMP callback
            timestamp_event_callback();

            // Clear the TIMESTAMP interrupt pending bit
            LL_RTC_ClearFlag_TS(RTC);
        }
    }
    // Clear the EXTI's Flag for RTC TimeStamp and Tamper
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_21);
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
