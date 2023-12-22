
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "serial/bsp_uart.h"

/* ----------------------- Platform includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbrtu.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN 4   /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX 256 /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC 2   /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF 0   /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF  1   /*!< Offset of Modbus-PDU in Ser-PDU. */

/* ----------------------- Type definitions ---------------------------------*/
typedef enum {
    STATE_RX_INIT, /*!< Receiver is in initial state. */
    STATE_RX_IDLE, /*!< Receiver is in idle state. */
    STATE_RX_RCV,  /*!< Frame is beeing received. */
    STATE_RX_ERROR /*!< If the frame is invalid. */
} eMBRcvState;

typedef enum {
    STATE_TX_IDLE, /*!< Transmitter is in idle state. */
    STATE_TX_XMIT  /*!< Transmitter is in transfer state. */
} eMBSndState;

/* ----------------------- Static variables ---------------------------------*/
static volatile eMBSndState eSndState;
static volatile eMBRcvState eRcvState;

volatile uint8_t ucRTUBuf[MB_SER_PDU_SIZE_MAX] = {0};

static volatile uint8_t* pucSndBufferCur;
static volatile uint16_t usSndBufferCount;

static volatile uint16_t usRcvBufferPos;

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode eMBRTUInit(uint8_t ucSlaveAddress, uint8_t ucPort, uint32_t ulBaudRate, eMBParity eParity)
{
    eMBErrorCode eStatus = MB_ENOERR;
    uint32_t     usTimerT35_50us;

    (void)ucSlaveAddress;
    ENTER_CRITICAL_SECTION();

    /* Modbus RTU uses 8 Databits. */
    if (xMBPortSerialInit(ucPort, ulBaudRate, 8, eParity) != true)
    {
        eStatus = MB_EPORTERR;
    }
    else
    {
        /* If baudrate > 19200 then we should use the fixed timer values
         * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
         */
        if (ulBaudRate > 19200)
        {
            usTimerT35_50us = 35; /* 1800us. */
        }
        else
        {
            /* The timer reload value for a character is given by:
             *
             * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
             *             = 11 * Ticks_per_1s / Baudrate
             *             = 220000 / Baudrate
             * The reload for t3.5 is 1.5 times this value and similary
             * for t3.5.
             */
            usTimerT35_50us = (7UL * 220000UL) / (2UL * ulBaudRate);
        }
        if (xMBPortTimersInit((uint16_t)usTimerT35_50us) != true)
        {
            eStatus = MB_EPORTERR;
        }
    }
    EXIT_CRITICAL_SECTION();

    return eStatus;
}

void eMBRTUStart(void)
{
    ENTER_CRITICAL_SECTION();
    /* Initially the receiver is in the state STATE_RX_INIT. we start
     * the timer and if no character is received within t3.5 we change
     * to STATE_RX_IDLE. This makes sure that we delay startup of the
     * modbus protocol stack until the bus is free.
     */
    eRcvState = STATE_RX_INIT;
    vMBPortSerialEnable(true, false);
    vMBPortTimersEnable();

    EXIT_CRITICAL_SECTION();
}

void eMBRTUStop(void)
{
    ENTER_CRITICAL_SECTION();
    vMBPortSerialEnable(false, false);
    vMBPortTimersDisable();
    EXIT_CRITICAL_SECTION();
}

eMBErrorCode eMBRTUReceive(uint8_t* pucRcvAddress, uint8_t** pucFrame, uint16_t* pusLength)
{
    bool         xFrameReceived = false;
    eMBErrorCode eStatus        = MB_ENOERR;

    ENTER_CRITICAL_SECTION();
    assert(usRcvBufferPos < MB_SER_PDU_SIZE_MAX);

    /* Length and CRC check */
    if ((usRcvBufferPos >= MB_SER_PDU_SIZE_MIN) && (usMBCRC16((uint8_t*)ucRTUBuf, usRcvBufferPos) == 0))
    {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        *pucRcvAddress = ucRTUBuf[MB_SER_PDU_ADDR_OFF];

        /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
         * size of address field and CRC checksum.
         */
        *pusLength = (uint16_t)(usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC);

        /* Return the start of the Modbus PDU to the caller. */
        *pucFrame      = (uint8_t*)&ucRTUBuf[MB_SER_PDU_PDU_OFF];
        xFrameReceived = true;
    }
    else
    {
        eStatus = MB_EIO;
    }

    EXIT_CRITICAL_SECTION();
    return eStatus;
}

eMBErrorCode eMBRTUSend(uint8_t ucSlaveAddress, const uint8_t* pucFrame, uint16_t usLength)
{
    eMBErrorCode eStatus = MB_ENOERR;
    uint16_t     usCRC16;

    ENTER_CRITICAL_SECTION();

    /* Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
    if (eRcvState == STATE_RX_IDLE)
    {
        /* First byte before the Modbus-PDU is the slave address. */
        pucSndBufferCur  = (uint8_t*)pucFrame - 1;
        usSndBufferCount = 1;

        /* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
        pucSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
        usSndBufferCount += usLength;

        /* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
        usCRC16                      = usMBCRC16((uint8_t*)pucSndBufferCur, usSndBufferCount);
        ucRTUBuf[usSndBufferCount++] = (uint8_t)(usCRC16 & 0xFF);
        ucRTUBuf[usSndBufferCount++] = (uint8_t)(usCRC16 >> 8);

        /* Activate the transmitter. */
        eSndState = STATE_TX_XMIT;
        vMBPortSerialEnable(false, true);

        extern UART_HandleTypeDef huart1;
        Uart_SetWorkDir(UART_DIR_TX);

        __HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);
        HAL_UART_Transmit_DMA(&huart1, (const uint8_t*)ucRTUBuf, usSndBufferCount);
    }
    else
    {
        eStatus = MB_EIO;
    }
    EXIT_CRITICAL_SECTION();
    return eStatus;
}

bool xMBRTUReceiveFSM(void)
{
    bool xTaskNeedSwitch = false;
    return xTaskNeedSwitch;
}

bool xMBRTUTransmitFSM(void)
{
    bool xTaskNeedSwitch = false;
    return xTaskNeedSwitch;
}

bool xMBRTUTimerT35Expired(void)
{
    bool xTaskNeedSwitch = false;
    return xTaskNeedSwitch;
}

//-----------------------------------------------------------------------------

#include "stm32h7xx_hal.h"

// call by USART_TC
void modbus_txcbk(void)
{
    // transmit finish, prepare recvice
    eRcvState = STATE_RX_IDLE;
}

// call by USART_IDLE
void modbus_rxcbk(const uint8_t* buffer, uint16_t length)
{
    // if (INCLOSE(length, MB_SER_PDU_SIZE_MIN, MB_SER_PDU_SIZE_MAX))
    {
        usRcvBufferPos = length;
        memcpy((void*)ucRTUBuf, buffer, length);

        // recvice finish
        eRcvState = STATE_RX_IDLE;
        xMBPortEventPost(EV_FRAME_RECEIVED);
    }
}
