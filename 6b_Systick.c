
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_rcc.h"
#include "stm32f7xx_ll_system.h"
#include "stm32f7xx_ll_utils.h"
#include "stm32f7xx_ll_gpio.h"

int main(void){
	SysTick_Config(16000);

	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);

	while(1){
		LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
		// Delay for 1000ms = ls
		LL_mDelay(1000);
		// if error: Drivers\STM32F7xx_HAL_Driver\Src\stm32f7xx_ll_utils.c paste to Src
	}
}

void SysTick_Handler(void){

}
