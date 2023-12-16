

#ifndef __STEPPER_TIM_H
#define __STEPPER_TIM_H


#define ATIM_TIMX_PWM                          TIM1
#define ATIM_TIMX_PWM_CH1                      TIM_CHANNEL_1                                   


extern TIM_HandleTypeDef htim1;                                                       
extern TIM_OC_InitTypeDef g_atimx_oc_chy_handle;                                        


void atim_timx_oc_chy_init(uint16_t arr, uint16_t psc);                                      

#endif



