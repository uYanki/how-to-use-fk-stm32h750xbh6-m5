
#include "mb.h"
#include "mbport.h"
#include "mbrtu.h"

#include "main.h"
#include "usart.h"
#include "serial/bsp_uart.h"

// 开启DCache后, 是否要8字节对齐??
static uint8_t mUartRxFrame[CONFIG_UART_RXBUF_SIZE] = {0};

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

    if (ucPort == 1)
    {
        extern UART_HandleTypeDef huart1;

        huart1.Init.BaudRate = ulBaudRate;

        if (eParity == MB_PAR_NONE)
        {
            huart1.Init.Parity = UART_PARITY_NONE;

            switch (ucDataBits)
            {
                case 7:
                    return false;
                case 8:
                    huart1.Init.WordLength = UART_WORDLENGTH_8B;
                    break;
            }
        }
        else
        {
            // 在 STM32 中, 数据位包含了校验位

            switch (ucDataBits)
            {
                case 7:
                    huart1.Init.WordLength = UART_WORDLENGTH_8B;
                    break;
                case 8:
                    huart1.Init.WordLength = UART_WORDLENGTH_9B;
                    break;
            }

            switch (eParity)
            {
                case MB_PAR_EVEN:
                    huart1.Init.Parity = UART_PARITY_EVEN;
                    break;
                case MB_PAR_ODD:
                    huart1.Init.Parity = UART_PARITY_ODD;
                    break;
                default:
                    return false;
            }
        }

        HAL_UART_Init(&huart1);
    }
    else
    {
        return false;
    }

    return true;
}

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
                mUartRxFrame[length] = '\0';

                extern void modbus_rxcbk(const uint8_t* buffer, uint16_t length);
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

        extern void modbus_txcbk(void);
        modbus_txcbk();

        Uart_SetWorkDir(UART_DIR_RX);
    }
}
