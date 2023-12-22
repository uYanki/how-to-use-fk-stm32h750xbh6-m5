
#ifndef _MB_ASCII_H
#define _MB_ASCII_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

#if MB_ASCII_ENABLED > 0
eMBErrorCode    eMBASCIIInit( uint8_t slaveAddress, uint8_t ucPort,
                              uint32_t ulBaudRate, eMBParity eParity );
void            eMBASCIIStart( void );
void            eMBASCIIStop( void );

eMBErrorCode    eMBASCIIReceive( uint8_t * pucRcvAddress, uint8_t ** pucFrame,
                                 uint16_t * pusLength );
eMBErrorCode    eMBASCIISend( uint8_t slaveAddress, const uint8_t * pucFrame,
                              uint16_t usLength );
BOOL            xMBASCIIReceiveFSM( void );
BOOL            xMBASCIITransmitFSM( void );
BOOL            xMBASCIITimerT1SExpired( void );
#endif

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
