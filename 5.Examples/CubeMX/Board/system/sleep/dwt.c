#include "../sleep.h"

// Debug Exception and Monitor Control Register
#define DEM_REG_CR           *(__IO uint32_t*)0xE000EDFC
#define DEM_MSK_CR_TRCENA    BV(24)

// Debug Watchpoint and Trace (DWT) unit
#define DWT_REG_BASE         0xE0001000
// Control Register
#define DWT_REG_CR           *(__IO uint32_t*)(DWT_REG_BASE + 0x00)
#define DWT_MSK_CR_CYCCNTENA BV(0)
// Cycle Count register
#define DWT_REG_CYCCNT       *(__IO uint32_t*)(DWT_REG_BASE + 0x04)

void DWT_Init(void)
{
    DEM_REG_CR |= DEM_MSK_CR_TRCENA;

    // clear counter
    DWT_REG_CYCCNT = 0u;

    // enable DWT
    DWT_REG_CR |= DWT_MSK_CR_CYCCNTENA;
}

// 32-bit timestamp
uint32_t DWT_GetTick(void)
{
    return DWT_REG_CYCCNT;
}

void DWT_DelayUS(uint32_t us)
{
    // us to tick
    DWT_DelayTick(us * (SystemCoreClock / 1e6));
}

void DWT_DelayTick(uint32_t nTicks)
{
    uint32_t nTickStart = DWT_REG_CYCCNT;
    uint32_t nTickEnd   = nTickStart + nTicks;

    if (nTickEnd > nTickStart)
    {
        while (DWT_REG_CYCCNT < nTickEnd)
            ;
    }
    else  // counter overflow
    {
        while (DWT_REG_CYCCNT >= nTickEnd)
            ;
        while (DWT_REG_CYCCNT < nTickEnd)
            ;
    }
}

uint32_t DWT_TickToUs(uint32_t nTickStart, uint32_t nTickEnd)
{
    uint32_t nTickDelta;

    if (nTickEnd > nTickStart)
    {
        nTickDelta = nTickEnd - nTickStart;
    }
    else
    {
        nTickDelta = UINT32_MAX - (nTickStart - nTickEnd) + 1;
    }

    return nTickDelta / (SystemCoreClock / 1e6);
}

/**
 * @brief Pin Toggle Test
 */
void DWT_PinTglTest(void)
{
#if 0  // 示波器看多少个 ticks 对应 1us, 用于实现精确微秒延迟

#define DeltaTicks 135

    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    while (1)
    {
        GPIOA->BSRRL = GPIO_Pin_11;
        DWT_DelayTick(DeltaTicks);
        GPIOA->BSRRH = GPIO_Pin_11;
        DWT_DelayTick(DeltaTicks);
    }

#endif
}
