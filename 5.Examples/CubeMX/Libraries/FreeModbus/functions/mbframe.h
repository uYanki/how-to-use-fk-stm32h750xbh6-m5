

#ifndef _MB_FRAME_H
#define _MB_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Constants which defines the format of a modbus frame. The example is
 * shown for a Modbus RTU/ASCII frame. Note that the Modbus PDU is not
 * dependent on the underlying transport.
 *
 * <code>
 * <------------------------ MODBUS SERIAL LINE PDU (1) ------------------->
 *              <----------- MODBUS PDU (1') ---------------->
 *  +-----------+---------------+----------------------------+-------------+
 *  | Address   | Function Code | Data                       | CRC/LRC     |
 *  +-----------+---------------+----------------------------+-------------+
 *  |           |               |                                   |
 * (2)        (3/2')           (3')                                (4)
 *
 * (1)  ... MB_SER_PDU_SIZE_MAX = 256
 * (2)  ... MB_SER_PDU_ADDR_OFF = 0
 * (3)  ... MB_SER_PDU_PDU_OFF  = 1
 * (4)  ... MB_SER_PDU_SIZE_CRC = 2
 *
 * (1') ... MB_PDU_SIZE_MAX     = 253
 * (2') ... MB_PDU_FUNC_OFF     = 0
 * (3') ... MB_PDU_DATA_OFF     = 1
 * </code>
 */

/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_SIZE_MAX 253 /*!< Maximum size of a PDU. */
#define MB_PDU_SIZE_MIN 1   /*!< Function Code */
#define MB_PDU_FUNC_OFF 0   /*!< Offset of function code in PDU. */
#define MB_PDU_DATA_OFF 1   /*!< Offset for response data in PDU. */

/* ----------------------- Prototypes  0-------------------------------------*/
typedef void (*pvMBFrameStart)(void);

typedef void (*pvMBFrameStop)(void);

typedef eMBErrorCode (*peMBFrameReceive)(uint8_t* pucRcvAddress, uint8_t** pucFrame, uint16_t* pusLength);

typedef eMBErrorCode (*peMBFrameSend)(uint8_t slaveAddress, const uint8_t* pucFrame, uint16_t usLength);

typedef void (*pvMBFrameClose)(void);

#ifdef __cplusplus
}
#endif
#endif
