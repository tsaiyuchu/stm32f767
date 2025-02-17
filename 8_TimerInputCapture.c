# include "stm32f7xx_ll_gpio.h"
# include "stm32f7xx_ll_bus.h"
# include "stm32f7xx_ll_tim.h"

void tim2_init(void);
void tim3_ic_init(void);

uint32_t volatile timestamp =0;

int main(){
	tim2_init();
	tim3_ic_init();
	while(1){
	    /*wait until edge is capture*/
	    while(!(LL_TIM_IsActiveFlag_CC1(TIM3))){}

	    /*read captured counter value*/
	    timestamp = LL_TIM_IC_GetCaptureCH1(TIM3);

	    /* Clear the capture flag */
	    LL_TIM_ClearFlag_CC1(TIM3);
	}
}

void tim2_init(void){ //PB3 TIM2_CH2 AF1
	LL_AHB1_GRP1_EnableClock (LL_AHB1_GRP1_PERIPH_GPIOB);

LL_GPIO_SetPinMode (GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);

	/*Set alternate function type*/
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_3, LL_GPIO_AF_1);

	/*1 Enable clock access to timer module*/
	LL_APB1_GRP1_EnableClock (LL_APB1_GRP1_PERIPH_TIM2);

	/*2 Set the pre-scaler*/
	LL_TIM_SetPrescaler (TIM2, 1600-1);//16 000 000/1600 = 10 000

	/*Set output compare mode :TOGGLE*/
LL_TIM_OC_SetMode (TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_TOGGLE);

	/*3 Set auto-reload value*/
	LL_TIM_SetAutoReload (TIM2, 10000-1); //10 000/10 000 = 1
	/*Enable channel compare mode*/
	LL_TIM_CC_EnableChannel (TIM2, LL_TIM_CHANNEL_CH2);
	/*Reset the counter*/
	LL_TIM_SetCounter (TIM2, 0);
	/*4 Enable timer module*/
	LL_TIM_EnableCounter (TIM2);
}

void tim3_ic_init(void){ //PB4 TIM3_CH1 AF2
	/*1 Enable clock access to timer module*/
	LL_APB1_GRP1_EnableClock (LL_APB1_GRP1_PERIPH_TIM3);

	/*Enable clock for GPIOC*/
	LL_AHB1_GRP1_EnableClock (LL_AHB1_GRP1_PERIPH_GPIOB);

	/*Set PC6 as output pin*/
LL_GPIO_SetPinMode (GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ALTERNATE);

	/*Set alternate function type*/
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_4, LL_GPIO_AF_2);

	/*Set the pre-scaler*/
	LL_TIM_SetPrescaler (TIM3, 16000-1);

LL_TIM_IC_SetActiveInput (TIM3,  LL_TIM_CHANNEL_CH1,  LL_TIM_ACTIVEINPUT_DIRECTTI);

LL_TIM_IC_SetPolarity (TIM3, LL_TIM_CHANNEL_CH1,LL_TIM_IC_POLARITY_RISING);

	/*Enable channel compare mode*/
	LL_TIM_CC_EnableChannel (TIM3, LL_TIM_CHANNEL_CH1);

	/*Enable timer module*/
	LL_TIM_EnableCounter (TIM3);
}
