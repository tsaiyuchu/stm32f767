18. stm32f7xx_it.c cubemx生成code後會加上此程式碼，請將IRQHandler (中斷處理函數) 負責處理 I2C 事件中斷 (EV) 和錯誤中斷 (ER)函式放置此處
#include "main.h"
#include "stm32f7xx_it.h"
/* USER CODE BEGIN PM */
extern uint8_t master_tx_data[];
extern uint8_t slave_rx_data[];
extern volatile uint8_t tx_index;
extern volatile uint8_t rx_index;
extern volatile uint8_t transfer_complete;

/**
@brief This function handles I2C2 event interrupt.
*/

void I2C2_EV_IRQHandler(void)
{
if (LL_I2C_IsActiveFlag_TXIS(I2C2))
{
LL_I2C_TransmitData8(I2C2, master_tx_data[tx_index++]);
}

if (LL_I2C_IsActiveFlag_TC(I2C2))
{
LL_I2C_GenerateStopCondition(I2C2);
LL_I2C_DisableIT_TX(I2C2);
}
}

/**
@brief This function handles I2C2 error interrupt.
*/

void I2C2_ER_IRQHandler(void)
{
if (LL_I2C_IsActiveFlag_BERR(I2C2)) { LL_I2C_ClearFlag_BERR(I2C2); }
if (LL_I2C_IsActiveFlag_ARLO(I2C2)) { LL_I2C_ClearFlag_ARLO(I2C2); }
if (LL_I2C_IsActiveFlag_OVR(I2C2)) { LL_I2C_ClearFlag_OVR(I2C2); }
if (LL_I2C_IsActiveFlag_NACK(I2C2)) { LL_I2C_ClearFlag_NACK(I2C2); }
}

/**
@brief This function handles I2C4 event interrupt.
*/

void I2C4_EV_IRQHandler(void)
{
/* Slave Address Matched */
if (LL_I2C_IsActiveFlag_ADDR(I2C4))
{
LL_I2C_ClearFlag_ADDR(I2C4);
LL_I2C_AcknowledgeNextData(I2C4, LL_I2C_ACK);
}
if (LL_I2C_IsActiveFlag_RXNE(I2C4))
{
slave_rx_data[rx_index++] = LL_I2C_ReceiveData8(I2C4);
}

/* STOP Condition */
if (LL_I2C_IsActiveFlag_STOP(I2C4))
{
LL_I2C_ClearFlag_STOP(I2C4);
LL_I2C_DisableIT_RX(I2C4);
transfer_complete = 1;
}
}

/**
@brief This function handles I2C4 error interrupt.
*/

void I2C4_ER_IRQHandler(void)
{
if (LL_I2C_IsActiveFlag_BERR(I2C4)) { LL_I2C_ClearFlag_BERR(I2C4); }
if (LL_I2C_IsActiveFlag_ARLO(I2C4)) { LL_I2C_ClearFlag_ARLO(I2C4); }
if (LL_I2C_IsActiveFlag_OVR(I2C4)) { LL_I2C_ClearFlag_OVR(I2C4); }
}
