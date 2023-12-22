

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/


/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0

/* ----------------------- Static variables ---------------------------------*/
static uint8_t  ucMBSlaveID[MB_FUNC_OTHER_REP_SLAVEID_BUF];
static uint16_t usMBSlaveIDLen;

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode eMBSetSlaveID(uint8_t ucSlaveID, bool xIsRunning, const uint8_t* pucAdditional, uint16_t usAdditionalLen)
{
    eMBErrorCode eStatus = MB_ENOERR;

    /* the first byte and second byte in the buffer is reserved for
     * the parameter ucSlaveID and the running flag. The rest of
     * the buffer is available for additional data. */
    if (usAdditionalLen + 2 < MB_FUNC_OTHER_REP_SLAVEID_BUF)
    {
        usMBSlaveIDLen                = 0;
        ucMBSlaveID[usMBSlaveIDLen++] = ucSlaveID;
        ucMBSlaveID[usMBSlaveIDLen++] = (uint8_t) (xIsRunning ? 0xFF : 0x00);
        if (usAdditionalLen > 0)
        {
            memcpy(&ucMBSlaveID[usMBSlaveIDLen], pucAdditional, (size_t) usAdditionalLen);
            usMBSlaveIDLen += usAdditionalLen;
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }
    return eStatus;
}

eMBException eMBFuncReportSlaveID(uint8_t* pucFrame, uint16_t* usLen)
{
    memcpy(&pucFrame[MB_PDU_DATA_OFF], &ucMBSlaveID[0], (size_t) usMBSlaveIDLen);
    *usLen = (uint16_t) (MB_PDU_DATA_OFF + usMBSlaveIDLen);
    return MB_EX_NONE;
}

#endif
