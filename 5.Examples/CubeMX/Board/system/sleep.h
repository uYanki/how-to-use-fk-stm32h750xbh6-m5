#ifndef __DELAY_H__
#define __DELAY_H__

//

#define DELAY_CLKSRC_TSG    0
#define DELAY_CLKSRC_DWT    1
#define DELAY_CLKSRC_TIM    2

#define CONFIG_DELAY_CLKSRC DELAY_CLKSRC_DWT

//

#define BV(n)               (1ul << (n))

#include <stdbool.h>
#include "stm32h7xx_hal.h"

//

typedef enum {

    // DelayBlock
    UNIT1_US = 1ul,
    UNIT1_MS = 1000ul,
    UNIT1_S  = 1000000ul,

    // DelayNonBlock
    UNIT2_US = 0ul,
    UNIT2_MS = 1ul,
    UNIT2_S  = 1000ul,

} DelayUnit_e;

#if CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_DWT
typedef uint32_t tick_t;
#define TICK_MAX (UINT32_MAX)
#elif CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_TSG
typedef uint64_t tick_t;
#define TICK_MAX (UINT64_MAX)
#else
#error "must select a clock source for DelayBlock"
#endif

#include "sleep/dwt.h"
#include "sleep/tsg.h"

#define DelayBlockUS(t) DelayBlock((t) * (UNIT1_US))
#define DelayBlockMS(t) DelayBlock((t) * (UNIT1_MS))
#define DelayBlockS(t)  DelayBlock((t) * (UNIT1_S))

inline void   DelayInit(void);
inline tick_t DelayGetTick(void);
inline void   DelayBlock(tick_t nWaitTime);
inline tick_t DelayCalcDelta(tick_t nStartTick, tick_t nEndTick);

//

// #define DelayNonBlockUS(s, t) DelayNonBlock(s, (t) * (UNIT2_US))
#define DelayNonBlockMS(s, t) DelayNonBlock(s, (t) * (UNIT2_MS))
#define DelayNonBlockS(s, t)  DelayNonBlock(s, (t) * (UNIT2_S))

inline void     DelayNonInit(void);
inline uint32_t DelayNonGetTick(void);
inline bool     DelayNonBlock(uint32_t nStartTick, uint32_t nWaitTime);

//

#endif
