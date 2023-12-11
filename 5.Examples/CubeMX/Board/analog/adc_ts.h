#ifndef __ADC_TS_H__
#define __ADC_TS_H__

#include "stm32h7xx_hal.h"

#define AD2VOL(ADConv) (3.3 * (uint32_t)ADConv / 65535)

float TS_ConvTemp(uint16_t ADConv);

#endif
