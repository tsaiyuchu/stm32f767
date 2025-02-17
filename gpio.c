#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"

int main(){

	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);

	while(1){
		/* Toggle PB0 */
		LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
		for(int itr=0;itr<10000;itr++){}
	}

}
