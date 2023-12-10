#ifndef __SLEEP_DWT_H__
#define __SLEEP_DWT_H__

void     DWT_Init(void);
void     DWT_DelayTick(uint32_t nTicks);
void     DWT_DelayUS(uint32_t us);
uint32_t DWT_GetTick(void);
uint32_t DWT_TickToUs(uint32_t nTickStart, uint32_t nTickEnd);

void DWT_PinTglTest(void);

#endif
