# include "stm32f7xx_ll_gpio.h"
# include "stm32f7xx_ll_bus.h"
# include "stm32f7xx_ll_adc.h"

void adc1_init(void);

uint32_t volatile sensor_value =0;

int main(){
	adc1_init();
	while(1){
		/* Start ADC conversion*/
		LL_ADC_REG_StartConversionSWStart(ADC1);

		/* Wait wait for conversion to be complete*/
		while(!(LL_ADC_IsActiveFlag_EOCS(ADC1))){}

		/* Read converted value */
		sensor_value = LL_ADC_REG_ReadConversionData32(ADC1);
	}
}

void adc1_init(void){ //PB0 ADC1_IN8
	/*1. Enable clock access to ADC module*/
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

	/*2. Enable clock access to ADC channel port*/
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/*3. Set ADC channel pin mode to analog*/
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);

	/*4. Set ADC trigger*/
	LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);

	/*5. Set sampling sequence*/
LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_8);

	/*6. sequence length*/
LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_DISABLE);

	/*7. Enable ADC*/
	LL_ADC_Enable(ADC1);
}

