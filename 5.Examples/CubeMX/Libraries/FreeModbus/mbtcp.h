
#ifndef _MB_TCP_H
#define _MB_TCP_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/* ----------------------- Defines ------------------------------------------*/
#define MB_TCP_PSEUDO_ADDRESS 255

/* ----------------------- Function prototypes ------------------------------*/
eMBErrorCode eMBTCPDoInit(uint16_t ucTCPPort);
void         eMBTCPStart(void);
void         eMBTCPStop(void);
eMBErrorCode eMBTCPReceive(uint8_t* pucRcvAddress, uint8_t** pucFrame, uint16_t* pusLength);
eMBErrorCode eMBTCPSend(uint8_t _unused, const uint8_t* pucFrame, uint16_t usLength);

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
