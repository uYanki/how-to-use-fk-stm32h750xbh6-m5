
#include "mb.h"
#include "mbport.h"
#include "mbutils.h"

#include "paratbl.h"

// register_group_t
typedef struct {
    uint16_t  u16Offset;
    uint16_t  u16Count;
    uint16_t* u16Buffer;
} reg_grp_t;

static const reg_grp_t m_holding[] = {
    {   0,  sizeof(ParaTable_t) / sizeof(uint16_t), (uint16_t*) &ParaTbl},
    {1000, sizeof(DebugTable_t) / sizeof(uint16_t),  (uint16_t*) &DbgTbl},
};

//-----------------------------------------------------------------------------
//

eMBErrorCode eMBRegInputCB(uint8_t* pu8Buffer, uint16_t u16Address, uint16_t u16Count)
{
    return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(uint8_t* pu8Buffer, uint16_t u16Address, uint16_t u16Count, eMBRegisterMode eMode)
{
    eMBErrorCode eStatus = MB_ENOREG;

    uint8_t u8GrpIdx;

    u16Address--;

    for (u8GrpIdx = 0; u8GrpIdx < ARRAY_SIZE(m_holding); ++u8GrpIdx)
    {
        if ((m_holding[u8GrpIdx].u16Offset <= u16Address) && ((u16Address + u16Count) <= (m_holding[u8GrpIdx].u16Offset + m_holding[u8GrpIdx].u16Count)))
        {
            uint16_t  u16RegIdx  = u16Address - m_holding[u8GrpIdx].u16Offset;
            uint16_t* pu16RegBuf = &(m_holding[u8GrpIdx].u16Buffer[u16RegIdx]);

            switch (eMode)
            {
                case MB_REG_READ :
                {
                    while (u16Count > 0)
                    {
                        *pu8Buffer++ = (uint8_t) (*pu16RegBuf >> 8);
                        *pu8Buffer++ = (uint8_t) (*pu16RegBuf & 0xFF);
                        pu16RegBuf++;
                        u16Count--;
                    }
                    break;
                }
                case MB_REG_WRITE :
                {
                    while (u16Count > 0)
                    {
                        *pu16RegBuf = *pu8Buffer++ << 8;
                        *pu16RegBuf |= *pu8Buffer++;
                        pu16RegBuf++;
                        u16Count--;
                    }
                }
            }

            eStatus = MB_ENOERR;
        }
    }

    return eStatus;
}

eMBErrorCode eMBRegCoilsCB(uint8_t* pu8Buffer, uint16_t u16Address, uint16_t u16NCoils, eMBRegisterMode eMode)
{
    return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(uint8_t* pu8Buffer, uint16_t u16Address, uint16_t u16NDiscrete)
{
    return MB_ENOREG;
}
