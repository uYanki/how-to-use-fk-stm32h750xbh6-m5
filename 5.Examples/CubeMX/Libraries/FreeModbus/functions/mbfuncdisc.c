
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
#define MB_PDU_FUNC_READ_ADDR_OFF    (MB_PDU_DATA_OFF)
#define MB_PDU_FUNC_READ_DISCCNT_OFF (MB_PDU_DATA_OFF + 2)
#define MB_PDU_FUNC_READ_SIZE        (4)
#define MB_PDU_FUNC_READ_DISCCNT_MAX (0x07D0)

/* ----------------------- Static functions ---------------------------------*/
eMBException prveMBError2Exception(eMBErrorCode eErrorCode);

/* ----------------------- Start implementation -----------------------------*/

#if MB_FUNC_READ_COILS_ENABLED > 0

eMBException eMBFuncReadDiscreteInputs(uint8_t* pucFrame, uint16_t* usLen)
{
    uint16_t usRegAddress;
    uint16_t usDiscreteCnt;
    uint8_t  ucNBytes;
    uint8_t* pucFrameCur;

    eMBException eStatus = MB_EX_NONE;
    eMBErrorCode eRegStatus;

    if (*usLen == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN))
    {
        usRegAddress = (uint16_t) (pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t) (pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usRegAddress++;

        usDiscreteCnt = (uint16_t) (pucFrame[MB_PDU_FUNC_READ_DISCCNT_OFF] << 8);
        usDiscreteCnt |= (uint16_t) (pucFrame[MB_PDU_FUNC_READ_DISCCNT_OFF + 1]);

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if ((usDiscreteCnt >= 1) && (usDiscreteCnt < MB_PDU_FUNC_READ_DISCCNT_MAX))
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen      = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_DISCRETE_INPUTS;
            *usLen += 1;

            /* Test if the quantity of coils is a multiple of 8. If not last
             * byte is only partially field with unused coils set to zero. */
            if ((usDiscreteCnt & 0x0007) != 0)
            {
                ucNBytes = (uint8_t) (usDiscreteCnt / 8 + 1);
            }
            else
            {
                ucNBytes = (uint8_t) (usDiscreteCnt / 8);
            }
            *pucFrameCur++ = ucNBytes;
            *usLen += 1;

            eRegStatus = eMBRegDiscreteCB(pucFrameCur, usRegAddress, usDiscreteCnt);

            /* If an error occured convert it into a Modbus exception. */
            if (eRegStatus != MB_ENOERR)
            {
                eStatus = prveMBError2Exception(eRegStatus);
            }
            else
            {
                /* The response contains the function code, the starting address
                 * and the quantity of registers. We reuse the old values in the 
                 * buffer because they are still valid. */
                *usLen += ucNBytes;
                ;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid read coil register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif
