
#include "stm32f7xx_ll_bus.h"
#include "stm32f7xx_ll_gpio.h"
#include "stm32f7xx_ll_exti.h"
#include "stm32f7xx_ll_system.h"

void gpio_interrupt_init(void);
void led_init(void);

int main(){
	gpio_interrupt_init();
	led_init();
	while(1){}
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
	NVIC_SetPriority(EXTI15_10_IRQn, 0);
}

void led_init(void){
	/* Enable clock for GPIOB */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/* Set PB0 as output pin */
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
}

void user_button_callback(void){
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
}

void EXTI15_10_IRQHandler(void){
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET){
		user_button_callback();
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
	}
}
