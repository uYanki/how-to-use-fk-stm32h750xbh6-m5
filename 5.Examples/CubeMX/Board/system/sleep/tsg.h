#ifndef __SLEEP_TSG_H__
#define __SLEEP_TSG_H__

void     TSG_Init(void);
void     TSG_DelayUS(uint64_t us);
uint64_t TSG_GetTick(void);
uint64_t TSG_TickToUs(uint64_t nTickStart, uint64_t nTickEnd);

#endif
