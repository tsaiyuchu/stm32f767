#使用cubeMX 設定好腳位生成code
#include "main.h"
#include "stdio.h"
#include <string.h>

#define I2C_SLAVE_ADDRESS  0x5A
uint8_t master_tx_data[] = "HELLO";
uint8_t slave_rx_data[6] = {0};

volatile uint8_t tx_index = 0;
volatile uint8_t rx_index = 0;
volatile uint8_t transfer_complete = 0;
/* USER CODE BEGIN PFP */
void I2C2_Master_Transmit_IT(void);
void I2C4_Slave_Receive_IT(void);
void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void led_blink(uint32_t period);
void GPIO_Button_Init(void);
/* USER CODE END PFP */

int main(void)
{
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_I2C2_Init();
MX_I2C4_Init();
MX_USART3_UART_Init();
/* USER CODE BEGIN 2 */
LED_Init();
GPIO_Button_Init();
I2C4_Slave_Receive_IT();


while (1)
{
if (transfer_complete)
{
if (memcmp(master_tx_data, slave_rx_data, 5) == 0)
{
LED_On();
printf("I2C success: %s\n", slave_rx_data);
}
else
{
LED_Off();
printf("I2C fail！\n");
}
transfer_complete = 0;
}

}

/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
/* USER CODE END WHILE */

/* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
}

/**
/* USER CODE BEGIN 4 */
void I2C2_Master_Transmit_IT(void)
{

while (LL_I2C_IsActiveFlag_BUSY(I2C2)) {}

LL_I2C_HandleTransfer(I2C2, (I2C_SLAVE_ADDRESS << 1), LL_I2C_ADDRSLAVE_7BIT, 5, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

LL_I2C_EnableIT_TX(I2C2);
LL_I2C_EnableIT_ERR(I2C2);
}

/* *********************** I2C Slave *********************** */
void I2C4_Slave_Receive_IT(void)
{

LL_I2C_Enable(I2C4);
LL_I2C_EnableIT_RX(I2C4);
LL_I2C_EnableIT_ADDR(I2C4);
LL_I2C_EnableIT_ERR(I2C4);
}

/* *********************** LED *********************** */
void LED_Init(void)
{
LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
}

void LED_On(void)
{
LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
}

void LED_Off(void)
{
LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
}
void led_blink(uint32_t period)
{
LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_0);
}
int _write(int file, char *ptr, int len) {
for (int i = 0; i < len; i++) {
LL_USART_TransmitData8(USART3, ptr[i]);
while (!LL_USART_IsActiveFlag_TXE(USART3)); 
}
return len;
}
void GPIO_Button_Init(void)
{
LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE13);

LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
EXTI_InitStruct.LineCommand = ENABLE;
EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;

EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
LL_EXTI_Init(&EXTI_InitStruct);

NVIC_SetPriority(EXTI15_10_IRQn, 0);
NVIC_EnableIRQ(EXTI15_10_IRQn);
}
void EXTI15_10_IRQHandler(void)
{

if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13))
{
LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
printf("button start, I2C transmit\n");
I2C2_Master_Transmit_IT();
}
}


/* USER CODE END 4 */
