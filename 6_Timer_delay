
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_tim.h"

void tim2_1hz_init(void);
void led_init(void);

int main(){
	tim2_1hz_init();
	led_init();
	while(1){
		while(!LL_TIM_IsActiveFlag_UPDATE(TIM2)){}
		LL_TIM_ClearFlag_UPDATE(TIM2);
		LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
	}
}

void tim2_1hz_init(void){
	// 1.Enable clock access to timer module
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

	// 2.Set the pre-scaler
	LL_TIM_SetPrescaler(TIM2, 1600-1); //16 000 000/1600 = 10 000

	// 3.Set auto-reload value
	LL_TIM_SetAutoReload(TIM2, 10000-1); //10 000 / 10 000 = 1

	// 4.Enable timer module
	LL_TIM_EnableCounter(TIM2);
}

void led_init(void){
	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
}
