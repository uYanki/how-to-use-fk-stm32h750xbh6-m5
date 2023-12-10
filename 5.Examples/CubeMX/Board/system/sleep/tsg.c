#include "../sleep.h"

/// @note: the TSG clock frequency is half of the system clock

#define TSG_REG_BASE  0x5C005000
#define TSG_REG_CNTCR *(vu32*)(TSG_REG_BASE + 0x00)
#define TSG_REG_CNTCV *(vu64*)(TSG_REG_BASE + 0x08)

void TSG_Init(void)
{
    // disable TSG
    TSG_REG_CNTCR = 0;

    // clear counter
    TSG_REG_CNTCV = 0ull;

    // enable TSG
    TSG_REG_CNTCR = BV(0);
}

// 64-bit timestamp
uint64_t TSG_GetTick(void)
{
    return TSG_REG_CNTCV;
}

void TSG_DelayUS(uint64_t us)
{
    uint64_t nTickStart, nTickEnd, nTickDelta;

    nTickStart = TSG_REG_CNTCV;

    // us to tick
    nTickDelta = us * (CONFIG_SYSCLK / 2e6);

    nTickEnd = nTickStart + nTickDelta;

    if (nTickEnd > nTickStart)
    {
        while (TSG_REG_CNTCV < nTickEnd)
            ;
    }
    else  // counter overflow
    {
        while (TSG_REG_CNTCV >= nTickEnd)
            ;
        while (TSG_REG_CNTCV < nTickEnd)
            ;
    }
}

uint64_t TSG_TickToUs(uint64_t nTickStart, uint64_t nTickEnd)
{
    uint64_t nTickDelta;

    if (nTickEnd > nTickStart)
    {
        nTickDelta = nTickEnd - nTickStart;
    }
    else
    {
        nTickDelta = UINT64_MAX - (nTickStart - nTickEnd) + 1;
    }

    return nTickDelta / (CONFIG_SYSCLK / 2e6);
}
