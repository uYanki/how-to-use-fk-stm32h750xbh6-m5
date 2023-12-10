#ifndef __DELAY_H__
#define __DELAY_H__

//

#define DELAY_CLKSRC_NONE   0
#define DELAY_CLKSRC_TSG    1
#define DELAY_CLKSRC_DWT    2
#define DELAY_CLKSRC_TIM    3

#define CONFIG_DELAY_CLKSRC DELAY_CLKSRC_TSG

//

#include "stm32h7xx_hal.h"

typedef volatile unsigned short int     vu16;
typedef volatile unsigned long int      vu32;
typedef volatile unsigned long long int vu64;

#define BV(n) (1ul << (n))

//

#include "sleep/dwt.h"
#include "sleep/tsg.h"

//

#define DelayBlockMS(t) DelayBlockUS((t) * 1000)
#define DelayBlockS(t)  DelayBlockMS((t) * 1000000)

#if CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_DWT

#define DelayInit()     DWT_Init()
#define DelayGetTick(t) DWT_GetTick()
#define DelayBlockUS(t) DWT_DelayUS((t))

#elif CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_TSG

#define DelayInit()     TSG_Init()
#define DelayGetTick(t) TSG_GetTick()
#define DelayBlockUS(t) TSG_DelayUS((t))

#elif CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_TIM

#else

#define DelayInit()
#define DelayGetTick(t)
#define DelayBlockUS(t)

#endif

//

#endif
