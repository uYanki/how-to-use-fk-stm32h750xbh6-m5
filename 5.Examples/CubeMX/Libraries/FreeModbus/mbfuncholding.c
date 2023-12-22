
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_FUNC_READ_ADDR_OFF              (MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_READ_REGCNT_OFF            (MB_PDU_DATA_OFF + 2)
#define MB_PDU_FUNC_READ_SIZE                  (4)
#define MB_PDU_FUNC_READ_REGCNT_MAX            (0x007D)

#define MB_PDU_FUNC_WRITE_ADDR_OFF             (MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_WRITE_VALUE_OFF            (MB_PDU_DATA_OFF + 2)
#define MB_PDU_FUNC_WRITE_SIZE                 (4)

#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF         (MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF       (MB_PDU_DATA_OFF + 2)
#define MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF      (MB_PDU_DATA_OFF + 4)
#define MB_PDU_FUNC_WRITE_MUL_VALUES_OFF       (MB_PDU_DATA_OFF + 5)
#define MB_PDU_FUNC_WRITE_MUL_SIZE_MIN         (5)
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX       (0x0078)

#define MB_PDU_FUNC_READWRITE_READ_ADDR_OFF    (MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF  (MB_PDU_DATA_OFF + 2)
#define MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF   (MB_PDU_DATA_OFF + 4)
#define MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF (MB_PDU_DATA_OFF + 6)
#define MB_PDU_FUNC_READWRITE_BYTECNT_OFF      (MB_PDU_DATA_OFF + 8)
#define MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF (MB_PDU_DATA_OFF + 9)
#define MB_PDU_FUNC_READWRITE_SIZE_MIN         (9)

/* ----------------------- Static functions ---------------------------------*/
eMBException prveMBError2Exception(eMBErrorCode eErrorCode);

/* ----------------------- Start implementation -----------------------------*/

#if MB_FUNC_WRITE_HOLDING_ENABLED > 0

eMBException eMBFuncWriteHoldingRegister(uint8_t* pucFrame, uint16_t* usLen)
{
    uint16_t     usRegAddress;
    eMBException eStatus = MB_EX_NONE;
    eMBErrorCode eRegStatus;

    if (*usLen == (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN))
    {
        usRegAddress = (uint16_t)(pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1]);
        usRegAddress++;

        /* Make callback to update the value. */
        eRegStatus = eMBRegHoldingCB(&pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF], usRegAddress, 1, MB_REG_WRITE);

        /* If an error occured convert it into a Modbus exception. */
        if (eRegStatus != MB_ENOERR)
        {
            eStatus = prveMBError2Exception(eRegStatus);
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
eMBException eMBFuncWriteMultipleHoldingRegister(uint8_t* pucFrame, uint16_t* usLen)
{
    uint16_t usRegAddress;
    uint16_t usRegCount;
    uint8_t  ucRegByteCount;

    eMBException eStatus = MB_EX_NONE;
    eMBErrorCode eRegStatus;

    if (*usLen >= (MB_PDU_FUNC_WRITE_MUL_SIZE_MIN + MB_PDU_SIZE_MIN))
    {
        usRegAddress = (uint16_t)(pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1]);
        usRegAddress++;

        usRegCount = (uint16_t)(pucFrame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF] << 8);
        usRegCount |= (uint16_t)(pucFrame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF + 1]);

        ucRegByteCount = pucFrame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];

        if ((usRegCount >= 1) && (usRegCount <= MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX) && (ucRegByteCount == (uint8_t)(2 * usRegCount)))
        {
            /* Make callback to update the register values. */
            eRegStatus = eMBRegHoldingCB(&pucFrame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF], usRegAddress, usRegCount, MB_REG_WRITE);

            /* If an error occured convert it into a Modbus exception. */
            if (eRegStatus != MB_ENOERR)
            {
                eStatus = prveMBError2Exception(eRegStatus);
            }
            else
            {
                /* The response contains the function code, the starting
                 * address and the quantity of registers. We reuse the
                 * old values in the buffer because they are still valid.
                 */
                *usLen = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
#endif

#if MB_FUNC_READ_HOLDING_ENABLED > 0

eMBException eMBFuncReadHoldingRegister(uint8_t* pucFrame, uint16_t* usLen)
{
    uint16_t usRegAddress;
    uint16_t usRegCount;
    uint8_t* pucFrameCur;

    eMBException eStatus = MB_EX_NONE;
    eMBErrorCode eRegStatus;

    if (*usLen == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN))
    {
        usRegAddress = (uint16_t)(pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usRegAddress++;

        usRegCount = (uint16_t)(pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8);
        usRegCount = (uint16_t)(pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF + 1]);

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception.
         */
        if ((usRegCount >= 1) && (usRegCount <= MB_PDU_FUNC_READ_REGCNT_MAX))
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen      = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_HOLDING_REGISTER;
            *usLen += 1;

            /* Second byte in the response contain the number of bytes. */
            *pucFrameCur++ = (uint8_t)(usRegCount * 2);
            *usLen += 1;

            /* Make callback to fill the buffer. */
            eRegStatus = eMBRegHoldingCB(pucFrameCur, usRegAddress, usRegCount, MB_REG_READ);
            /* If an error occured convert it into a Modbus exception. */
            if (eRegStatus != MB_ENOERR)
            {
                eStatus = prveMBError2Exception(eRegStatus);
            }
            else
            {
                *usLen += usRegCount * 2;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif

#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0

eMBException eMBFuncReadWriteMultipleHoldingRegister(uint8_t* pucFrame, uint16_t* usLen)
{
    uint16_t usRegReadAddress;
    uint16_t usRegReadCount;
    uint16_t usRegWriteAddress;
    uint16_t usRegWriteCount;
    uint8_t  ucRegWriteByteCount;
    uint8_t* pucFrameCur;

    eMBException eStatus = MB_EX_NONE;
    eMBErrorCode eRegStatus;

    if (*usLen >= (MB_PDU_FUNC_READWRITE_SIZE_MIN + MB_PDU_SIZE_MIN))
    {
        usRegReadAddress = (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF] << 8U);
        usRegReadAddress |= (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF + 1]);
        usRegReadAddress++;

        usRegReadCount = (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF] << 8U);
        usRegReadCount |= (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF + 1]);

        usRegWriteAddress = (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF] << 8U);
        usRegWriteAddress |= (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF + 1]);
        usRegWriteAddress++;

        usRegWriteCount = (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF] << 8U);
        usRegWriteCount |= (uint16_t)(pucFrame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF + 1]);

        ucRegWriteByteCount = pucFrame[MB_PDU_FUNC_READWRITE_BYTECNT_OFF];

        if ((usRegReadCount >= 1) && (usRegReadCount <= 0x7D) && (usRegWriteCount >= 1) && (usRegWriteCount <= 0x79) && ((2 * usRegWriteCount) == ucRegWriteByteCount))
        {
            /* Make callback to update the register values. */
            eRegStatus = eMBRegHoldingCB(&pucFrame[MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF], usRegWriteAddress, usRegWriteCount, MB_REG_WRITE);

            if (eRegStatus == MB_ENOERR)
            {
                /* Set the current PDU data pointer to the beginning. */
                pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
                *usLen      = MB_PDU_FUNC_OFF;

                /* First byte contains the function code. */
                *pucFrameCur++ = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
                *usLen += 1;

                /* Second byte in the response contain the number of bytes. */
                *pucFrameCur++ = (uint8_t)(usRegReadCount * 2);
                *usLen += 1;

                /* Make the read callback. */
                eRegStatus = eMBRegHoldingCB(pucFrameCur, usRegReadAddress, usRegReadCount, MB_REG_READ);
                if (eRegStatus == MB_ENOERR)
                {
                    *usLen += 2 * usRegReadCount;
                }
            }
            if (eRegStatus != MB_ENOERR)
            {
                eStatus = prveMBError2Exception(eRegStatus);
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    return eStatus;
}

#endif
