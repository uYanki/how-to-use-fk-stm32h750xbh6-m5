

#ifndef _MB_PORT_H
#define _MB_PORT_H

#include <stdbool.h>
#include "stm32h7xx_hal.h"

#define INLINE inline

#define ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------- Type definitions ---------------------------------*/

typedef enum {
    EV_READY,          /*!< Startup finished. */
    EV_FRAME_RECEIVED, /*!< Frame received. */
    EV_EXECUTE,        /*!< Execute function. */
    EV_FRAME_SENT      /*!< Frame sent. */
} eMBEventType;

/*! \ingroup modbus
 * \brief Parity used for characters in serial mode.
 *
 * The parity which should be applied to the characters sent over the serial
 * link. Please note that this values are actually passed to the porting
 * layer and therefore not all parity modes might be available.
 */
typedef enum {
    MB_PAR_NONE, /*!< No parity. */
    MB_PAR_ODD,  /*!< Odd parity. */
    MB_PAR_EVEN  /*!< Even parity. */
} eMBParity;

/* ----------------------- Supporting functions -----------------------------*/
bool xMBPortEventInit(void);
bool xMBPortEventPost(eMBEventType eEvent);
bool xMBPortEventGet(/*@out@ */ eMBEventType* eEvent);

/* ----------------------- Serial port functions ----------------------------*/

bool xMBPortSerialInit(uint8_t ucPort, uint32_t ulBaudRate, uint8_t ucDataBits, eMBParity eParity);
void vMBPortClose(void);
void xMBPortSerialClose(void);
void vMBPortSerialEnable(bool xRxEnable, bool xTxEnable);
bool xMBPortSerialGetByte(int8_t* pucByte);
bool xMBPortSerialPutByte(int8_t ucByte);

/* ----------------------- Timers functions ---------------------------------*/
bool xMBPortTimersInit(uint16_t usTimeOut50us);
void xMBPortTimersClose(void);
void vMBPortTimersEnable(void);
void vMBPortTimersDisable(void);
void vMBPortTimersDelay(uint16_t usTimeOutMS);

/* ----------------------- Callback for the protocol stack ------------------*/

/*!
 * \brief Callback function for the porting layer when a new byte is
 *   available.
 *
 * Depending upon the mode this callback function is used by the RTU or
 * ASCII transmission layers. In any case a call to xMBPortSerialGetByte()
 * must immediately return a new character.
 *
 * \return <code>true</code> if a event was posted to the queue because
 *   a new byte was received. The port implementation should wake up the
 *   tasks which are currently blocked on the eventqueue.
 */
extern bool (*pxMBFrameCBByteReceived)(void);

extern bool (*pxMBFrameCBTransmitterEmpty)(void);

extern bool (*pxMBPortCBTimerExpired)(void);

/* ----------------------- TCP port functions -------------------------------*/
bool xMBTCPPortInit(uint16_t usTCPPort);

void vMBTCPPortClose(void);

void vMBTCPPortDisable(void);

bool xMBTCPPortGetRequest(uint8_t** ppucMBTCPFrame, uint16_t* usTCPLength);

bool xMBTCPPortSendResponse(const uint8_t* pucMBTCPFrame, uint16_t usTCPLength);

#ifdef __cplusplus
}
#endif
#endif
