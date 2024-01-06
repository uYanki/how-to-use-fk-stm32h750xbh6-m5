#include "sleep.h"

//

#include "sleep/dwt.c"
#include "sleep/tsg.c"
#include "sleep/tim.c"

void DelayInit(void)
{
#if CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_DWT
    DWT_Init();
#elif CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_TSG
    TSG_Init();
#endif
}

void DelayBlock(tick_t nWaitTime)
{
#if CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_DWT
    DWT_DelayUS(nWaitTime);
#elif CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_TSG
    TSG_DelayUS(nWaitTime);
#endif
}

tick_t DelayGetTick(void)
{
#if CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_DWT
    return DWT_GetTick();
#elif CONFIG_DELAY_CLKSRC == DELAY_CLKSRC_TSG
    return TSG_GetTick();
#endif
}

tick_t DelayCalcDelta(tick_t nStartTick, tick_t nEndTick)
{
    if (nEndTick >= nStartTick)
    {
        return nEndTick - nStartTick;
    }
    else
    {
        return TICK_MAX - nStartTick + nEndTick;
    }
}

//

void DelayNonInit(void) {}

uint32_t DelayNonGetTick(void)
{
    return HAL_GetTick();
}

bool DelayNonBlock(uint32_t nStartTick, uint32_t nWaitTime)
{
    uint32_t nDeltaTick;
    uint32_t nEndTick;
	
	if( nWaitTime ==0 )return  true;

    nEndTick = DelayNonGetTick();

    if (nEndTick >= nStartTick)
    {
        nDeltaTick = nEndTick - nStartTick;
    }
    else
    {
        nDeltaTick = TICK_MAX - nStartTick + nEndTick;
    }

    return nDeltaTick >= nWaitTime;

    // return DelayCalcDelta(nStartTick, DelayGetTick()) >= nWaitTime;
}
