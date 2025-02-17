
# include "stm32f7xx_ll_usart.h"
# include "stm32f7xx_ll_gpio.h"
# include "stm32f7xx_ll_bus.h"

static volatile uint8_t TxBuffer[] = "M\r";
static volatile uint8_t TxIndex = 0;
static volatile uint8_t TxLen   = 2;

static volatile uint8_t GxBuffer[] = "EC";
static volatile uint8_t GxIndex = 0;
static volatile uint8_t GxLen   =2;

typedef enum {
STATE_SEND_TXBUFFER,
STATE_SEND_GXBUFFER,
STATE_DONE
} UART_State;

static volatile UART_State currentState = STATE_SEND_TXBUFFER;

void uart3_init(void);
void USART3_IRQHandler(void);

int main()
{
uart3_init();
while(1)
{

}
}

void uart3_init(void)
{
/* 1. Enable clock access for UART GPIO pin */
LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

/* 2. Enable clock access for UART module */
LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

/* 3. Set mode of UART TX pin to alternate function */
LL_GPIO_SetPinMode(GPIOD, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);

/* 4. Select UART TX alternate function type */
LL_GPIO_SetAFPin_8_15(GPIOD, LL_GPIO_PIN_8, LL_GPIO_AF_7);

/* 5. Configure UART protocol parameters */

NVIC_SetPriority(USART3_IRQn, 0);
NVIC_EnableIRQ(USART3_IRQn);

LL_USART_Disable(USART3);
LL_USART_SetTransferDirection(USART3, LL_USART_DIRECTION_TX);
LL_USART_ConfigCharacter(USART3, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
LL_USART_SetBaudRate(USART3, SystemCoreClock, LL_USART_OVERSAMPLING_16, 115200);

LL_USART_Enable(USART3);

LL_USART_EnableIT_TXE(USART3);

LL_USART_EnableIT_TC(USART3);
}

void USART3_IRQHandler(void)
{
if(LL_USART_IsActiveFlag_TXE(USART3) && LL_USART_IsEnabledIT_TXE(USART3))
{
if(currentState == STATE_SEND_TXBUFFER)
{

LL_USART_TransmitData8(USART3, TxBuffer[TxIndex++]);
if(TxIndex >= TxLen)
{
// close TXE，wait for TC and then transmit GxBuffer
LL_USART_DisableIT_TXE(USART3);
}
}
else if(currentState == STATE_SEND_GXBUFFER)
{
LL_USART_TransmitData8(USART3, GxBuffer[GxIndex++]);
if(GxIndex >= GxLen)
{
LL_USART_DisableIT_TXE(USART3);
currentState = STATE_DONE;
}
}
}

if(LL_USART_IsActiveFlag_TC(USART3) && LL_USART_IsEnabledIT_TC(USART3))
{
LL_USART_ClearFlag_TC(USART3);

if(currentState == STATE_SEND_TXBUFFER)
{
currentState = STATE_SEND_GXBUFFER;
GxIndex = 0;
LL_USART_EnableIT_TXE(USART3);
}

LL_USART_DisableIT_TC(USART3);
}
}
