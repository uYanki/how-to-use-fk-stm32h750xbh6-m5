#ifndef __PWM_GEN_H__
#define __PWM_GEN_H__

#include "stm32h7xx_hal.h"

void PWM_Out(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t frq, uint16_t duty);

#endif
