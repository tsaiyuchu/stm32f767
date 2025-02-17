
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"

int main(){
	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Enable clock for GPIOC */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

	/* Set PB0 as output pin */
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, L_GPIO_MODE_OUTPUT);

	/* Set PC13 as input pin */
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, L_GPIO_MODE_INPUT);

	while(1){
		/* IsInputPinSet: Push back 0 or 1*/
		if(LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13)){
			/* if 1(push), Set PB0 to high level */
			LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
		} else {
			/* if 0, Set PB0 to low level */
			LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
		}
	}
}
