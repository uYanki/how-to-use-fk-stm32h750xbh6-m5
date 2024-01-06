
#include "mb.h"
#include "mbport.h"
#include "mbrtu.h"

#include "usart.h"
#include "serial/bsp_uart.h"

#if __DCACHE_PRESENT == 1U
ALIGN_32BYTES(static uint8_t mUartRxFrame[CONFIG_UART_RXBUF_SIZE]) = {0};
#else
static uint8_t mUartRxFrame[CONFIG_UART_RXBUF_SIZE] = {0};
#endif

void vMBPortSerialEnable(bool xRxEnable, bool xTxEnable)
{
    if (xRxEnable)
    {
        // RS485_SetRxDir();
    }

    if (xTxEnable)
    {
        // RS485_SetTxDir();
    }
}

bool xMBPortSerialInit(uint8_t ucPort, uint32_t ulBaudRate, uint8_t ucDataBits, eMBParity eParity)
{
    // ucPort is set by eMBInit

    UART_HandleTypeDef* huart;

    if (ucPort == 1)
    {
        // PB6(TX) & PB7(RX)
        huart = &huart1;
    }
    else
    {
        return false;
    }

    huart->Init.BaudRate = ulBaudRate;

    if (eParity == MB_PAR_NONE)
    {
        huart->Init.Parity = UART_PARITY_NONE;

        switch (ucDataBits)
        {
            case 7:
                return false;
            case 8:
                huart->Init.WordLength = UART_WORDLENGTH_8B;
                break;
        }
    }
    else
    {
        // 在 STM32 中, 数据位包含了校验位

        switch (ucDataBits)
        {
            case 7:
                huart->Init.WordLength = UART_WORDLENGTH_8B;
                break;
            case 8:
                huart->Init.WordLength = UART_WORDLENGTH_9B;
                break;
        }

        switch (eParity)
        {
            case MB_PAR_EVEN:
                huart->Init.Parity = UART_PARITY_EVEN;
                break;
            case MB_PAR_ODD:
                huart->Init.Parity = UART_PARITY_ODD;
                break;
            default:
                return false;
        }
    }

    HAL_UART_Init(huart);

    return true;
}

extern void modbus_rxcbk(const uint8_t* buffer, uint16_t length);
extern void modbus_txcbk(void);

// call in main()
void Modbus_StartReceive(void)
{
    // 空闲中断断帧
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    HAL_UART_Receive_DMA(&huart1, mUartRxFrame, ARRAY_SIZE(mUartRxFrame));
}

// call in USARTx_IRQHandler()
void HAL_UART_IdleCpltCallback(UART_HandleTypeDef* huart)
{
    extern DMA_HandleTypeDef hdma_usart1_rx;

    if (huart->Instance == USART1)
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
        {
            __HAL_UART_CLEAR_IDLEFLAG(huart);

            HAL_UART_DMAStop(huart);

            uint16_t length = ARRAY_SIZE(mUartRxFrame) - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);

            if (length > 0)
            {
                // 使用 DCache + DMA 就必需 32位对齐
                SCB_InvalidateDCache_by_Addr((uint32_t*)&mUartRxFrame[0], ARRAY_SIZE(mUartRxFrame) / 4);

                mUartRxFrame[length] = '\0';
                modbus_rxcbk(mUartRxFrame, length);
            }

            HAL_UART_Receive_DMA(huart, mUartRxFrame, ARRAY_SIZE(mUartRxFrame));
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1)
    {
        __HAL_UART_DISABLE_IT(huart, UART_IT_TC);

        modbus_txcbk();

        Uart_SetWorkDir(UART_DIR_RX);
    }
}
