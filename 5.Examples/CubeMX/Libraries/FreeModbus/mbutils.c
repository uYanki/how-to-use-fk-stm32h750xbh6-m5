
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

/* ----------------------- Platform includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbproto.h"

/* ----------------------- Defines ------------------------------------------*/
#define BITS_uint8_t 8U

/* ----------------------- Start implementation -----------------------------*/
void xMBUtilSetBits(uint8_t* ucByteBuf, uint16_t usBitOffset, uint8_t ucNBits, uint8_t ucValue)
{
    uint16_t usWordBuf;
    uint16_t usMask;
    uint16_t usByteOffset;
    uint16_t usNPreBits;
    uint16_t usValue = ucValue;

    assert(ucNBits <= 8);
    assert((size_t)BITS_uint8_t == sizeof(uint8_t) * 8);

    /* Calculate byte offset for first byte containing the bit values starting
     * at usBitOffset. */
    usByteOffset = (uint16_t)((usBitOffset) / BITS_uint8_t);

    /* How many bits precede our bits to set. */
    usNPreBits = (uint16_t)(usBitOffset - usByteOffset * BITS_uint8_t);

    /* Move bit field into position over bits to set */
    usValue <<= usNPreBits;

    /* Prepare a mask for setting the new bits. */
    usMask = (uint16_t)((1 << (uint16_t)ucNBits) - 1);
    usMask <<= usBitOffset - usByteOffset * BITS_uint8_t;

    /* copy bits into temporary storage. */
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << BITS_uint8_t;

    /* Zero out bit field bits and then or value bits into them. */
    usWordBuf = (uint16_t)((usWordBuf & (~usMask)) | usValue);

    /* move bits back into storage */
    ucByteBuf[usByteOffset]     = (uint8_t)(usWordBuf & 0xFF);
    ucByteBuf[usByteOffset + 1] = (uint8_t)(usWordBuf >> BITS_uint8_t);
}

uint8_t xMBUtilGetBits(uint8_t* ucByteBuf, uint16_t usBitOffset, uint8_t ucNBits)
{
    uint16_t usWordBuf;
    uint16_t usMask;
    uint16_t usByteOffset;
    uint16_t usNPreBits;

    /* Calculate byte offset for first byte containing the bit values starting
     * at usBitOffset. */
    usByteOffset = (uint16_t)((usBitOffset) / BITS_uint8_t);

    /* How many bits precede our bits to set. */
    usNPreBits = (uint16_t)(usBitOffset - usByteOffset * BITS_uint8_t);

    /* Prepare a mask for setting the new bits. */
    usMask = (uint16_t)((1 << (uint16_t)ucNBits) - 1);

    /* copy bits into temporary storage. */
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << BITS_uint8_t;

    /* throw away unneeded bits. */
    usWordBuf >>= usNPreBits;

    /* mask away bits above the requested bitfield. */
    usWordBuf &= usMask;

    return (uint8_t)usWordBuf;
}

eMBException prveMBError2Exception(eMBErrorCode eErrorCode)
{
    eMBException eStatus;

    if (eErrorCode & 0xF0)
    {
        return eErrorCode & 0x0F;
    }

    switch (eErrorCode)
    {
        case MB_ENOERR:
            eStatus = MB_EX_NONE;
            break;

        case MB_ENOREG:
            eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
            break;

        case MB_ETIMEDOUT:
            eStatus = MB_EX_SLAVE_BUSY;
            break;

        default:
            eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
            break;
    }

    return eStatus;
}
