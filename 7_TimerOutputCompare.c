#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_tim.h"

void tim3_oc_init(void);
void led_init(void);

int main(){
	tim3_oc_init();
	led_init();
	while(1){}
}

void tim3_oc_init(void){
	// 1.Enable clock access to timer module
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

	// 2.Set the pre-scaler
	LL_TIM_SetPrescaler(TIM3, 1600-1); //16 000 000/1600 = 10 000

	// 3.Set output compare mode :TOGGLE
LL_TIM_OC_SetMode(TIM3, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_TOGGLE);

	// 4.Enable ch1
	LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH3);

	// 5.Set auto-reload value
	LL_TIM_SetAutoReload(TIM3, 10000-1); //10 000 / 10 000 = 1

	// 6.Reset the counter
	LL_TIM_SetCounter(TIM3, 0);

	// 7.Enable timer module
	LL_TIM_EnableCounter(TIM3);
}

void led_init(void){
	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_ALTERNATE);

	/* Set alternate function type*/
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_0, LL_GPIO_AF_2);
}
