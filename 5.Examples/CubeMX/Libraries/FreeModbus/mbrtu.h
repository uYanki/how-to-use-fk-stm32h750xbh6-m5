
#ifndef _MB_RTU_H
#define _MB_RTU_H

#ifdef __cplusplus
extern "C" {
#endif

eMBErrorCode eMBRTUInit(uint8_t slaveAddress, uint8_t ucPort, uint32_t ulBaudRate, eMBParity eParity);
void         eMBRTUStart(void);
void         eMBRTUStop(void);
eMBErrorCode eMBRTUReceive(uint8_t* pucRcvAddress, uint8_t** pucFrame, uint16_t* pusLength);
eMBErrorCode eMBRTUSend(uint8_t slaveAddress, const uint8_t* pucFrame, uint16_t usLength);
bool         xMBRTUReceiveFSM(void);
bool         xMBRTUTransmitFSM(void);
bool         xMBRTUTimerT15Expired(void);
bool         xMBRTUTimerT35Expired(void);

#ifdef __cplusplus
}
#endif
#endif
