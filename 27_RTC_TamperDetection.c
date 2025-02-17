
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

#define BACKUP_COUNT RTC_BKP_NUMBER

#define LED_BLINK_RATE_FAST          	200
#define LED_BLINK_RATE_SLOW          	500
#define LED_BLINK_RATE_VERY_SLOW    	800
#define LED_BLINK_RATE_ERROR         	1000

void rtc_init(void);
void rtc_tamper_init(void);
void led_init(void);
void led_on(void);
void led_off(void);
void led_blink(uint32_t period);
void uart3_init(void);
void uart3_write(int ch);

volatile FlagStatus TamperStatus = RESET;

// Backup registers table
uint32_t aBKPDataReg[BACKUP_COUNT] =
{
    LL_RTC_BKP_DR0,  LL_RTC_BKP_DR1,  LL_RTC_BKP_DR2,
	LL_RTC_BKP_DR3,  LL_RTC_BKP_DR4,  LL_RTC_BKP_DR5,
	LL_RTC_BKP_DR6,  LL_RTC_BKP_DR7,  LL_RTC_BKP_DR8,
	LL_RTC_BKP_DR9,  LL_RTC_BKP_DR10, LL_RTC_BKP_DR11,
	LL_RTC_BKP_DR12, LL_RTC_BKP_DR13, LL_RTC_BKP_DR14,
    LL_RTC_BKP_DR15, LL_RTC_BKP_DR16, LL_RTC_BKP_DR17,
	LL_RTC_BKP_DR18, LL_RTC_BKP_DR19, LL_RTC_BKP_DR20,
	LL_RTC_BKP_DR21, LL_RTC_BKP_DR22, LL_RTC_BKP_DR23,
    LL_RTC_BKP_DR24, LL_RTC_BKP_DR25, LL_RTC_BKP_DR26,
	LL_RTC_BKP_DR27, LL_RTC_BKP_DR28, LL_RTC_BKP_DR29,
	LL_RTC_BKP_DR30, LL_RTC_BKP_DR31
};

int __io_putchar(int ch){
    uart3_write(ch);
    return ch;
}

int main(){
    register uint32_t index = 0;
    SysTick_Config(16000);
    led_init();
    uart3_init();
    rtc_init();
    rtc_tamper_init();

    // Write Data in the Back Up registers
    for (index = 0; index < BACKUP_COUNT; index++){
LL_RTC_BAK_SetRegister(RTC, aBKPDataReg[index], 200 + (index * 5));
    }

    // Check if data is stored in the Back Up registers
    for (index = 0; index < BACKUP_COUNT; index++){
if (LL_RTC_BAK_GetRegister(RTC, aBKPDataReg[index]) != (200 + (index * 5))){
printf("ERR: Backup register write mismatch at index %d\n\r", index);
            led_blink(LED_BLINK_RATE_ERROR);
        }
    }

    while(TamperStatus != SET){
        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
        LL_mDelay(LED_BLINK_RATE_FAST);
    }
    led_on();
}

void rtc_init(void){
    // Enables the PWR Clock and Enables access to the backup domain
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    LL_PWR_EnableBkUpAccess();

    // Enable RTC Clock
    LL_RCC_EnableRTC();
}

void rtc_tamper_init(void){
    // Set Tamper trigger to falling edge
LL_RTC_TAMPER_EnableActiveLevel(RTC, LL_RTC_TAMPER_ACTIVELEVEL_TAMP1);

    // Enable tamper detection
    LL_RTC_TAMPER_Enable(RTC, LL_RTC_TAMPER_1);

    // Enable IT TAMPER
    LL_RTC_EnableIT_TAMP(RTC);

    // Configure the NVIC for RTC Tamper
    NVIC_SetPriority(TAMP_STAMP_IRQn, 0x0F);
    NVIC_EnableIRQ(TAMP_STAMP_IRQn);

    // Configure RTC Tamper Interrupt
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_21);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_21);

    // Clear the Tamper interrupt pending bit
    LL_RTC_ClearFlag_TAMP1(RTC);
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

void tamper_callback(void){
    register uint32_t index = 0;

    // Deactivate the tamper
    LL_RTC_TAMPER_Disable(RTC, LL_RTC_TAMPER_1);

    // Check if data is erased in the Back Up registers
    for (index = 0; index < BACKUP_COUNT; index++){
if (LL_RTC_BAK_GetRegister(RTC, aBKPDataReg[index]) != 0x00){
            led_blink(LED_BLINK_RATE_ERROR);
            printf("Tamper Detected !! \n\r");
            printf("Error : Memory could was be erased !!\n\r");
        }
    }

    TamperStatus = SET;
    printf("Tamper Detected !! \n\r");
    printf("Memory successfully erased !!\n\r");
}

void TAMP_STAMP_IRQHandler(void){
    // Get the Tamper interrupt source enable status
    if(LL_RTC_IsEnabledIT_TAMP(RTC) != 0){
        // Get the pending status of the Tamper Interrupt
        if(LL_RTC_IsActiveFlag_TAMP1(RTC) != 0){
            // Tamper callback
            tamper_callback();

            // Clear the Tamper interrupt pending bit
            LL_RTC_ClearFlag_TAMP1(RTC);
        }
    }
    // Clear the EXTI Flag for RTC Tamper
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_21);
}

void SysTick_Handler(void){

}

