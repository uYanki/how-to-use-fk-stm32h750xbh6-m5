#include "motdrv.h"

extern TIM_HandleTypeDef htim1;

void PWM_Start(void)
{
    uint16_t duty = ParaTbl.u16PwmDutyMax * 0.5;
    PWM_SetDuty(duty, duty, duty);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

void PWM_Stop(void)
{
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
}

void PWM_SetDuty(uint16_t DutyA, uint16_t DutyB, uint16_t DutyC)
{
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, DutyA);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, DutyB);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, DutyC);
}
