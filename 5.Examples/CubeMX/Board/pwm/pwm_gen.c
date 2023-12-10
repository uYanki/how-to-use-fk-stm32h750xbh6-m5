#include "pwm_gen.h"

/**
 * @brief pwm configuration generate
 *
 * @param [in]  clk tim clock input
 * @param [in]  frq target frequency
 * @param [in]  duty duty cycle, 0(0%) ~ 10000 (100%)
 * @param [out] psc prescale
 * @param [out] prd period
 * @param [out] pls pulse
 *
 * @return none
 */
void PWM_ParaGen(uint32_t clk, uint32_t frq, uint16_t duty, __IO uint32_t* psc, __IO uint32_t* prd, __IO uint32_t* pls)
{
    // frq = clk / psc / prd

    if (frq < 100)
    {
        *psc = 1e4;
    }
    else if (frq < 3000)
    {
        *psc = 1e2;
    }
    else
    {
        *psc = 1;
    }

    *prd = clk / (*psc) / frq;
    *pls = (*prd) * duty / 10000;

    *psc -= 1;  // prescale
    *prd -= 1;  // period
    *pls -= 1;  // pulse
}

/**
 * @brief generate pwm
 * @note TIM mode: TIM_OCMODE_PWM1 or TIM_OCMODE_PWM2
 */
void PWM_Out(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t frq, uint16_t duty)
{
    uint32_t clk;  // tim clkin

    TIM_TypeDef*   TIMx = htim->Instance;
    __IO uint32_t* TIM_CCR;

#if 0
    // The frequency of clocks mounted on different buses may vary
    if (TIMx == TIM1)
#endif
    {
        clk = SystemCoreClock / 2;
    }

    switch (channel)
    {
        case TIM_CHANNEL_1:
            TIM_CCR = &TIMx->CCR1;
            break;
        case TIM_CHANNEL_2:
            TIM_CCR = &TIMx->CCR2;
            break;
        case TIM_CHANNEL_3:
            TIM_CCR = &TIMx->CCR3;
            break;
        case TIM_CHANNEL_4:
            TIM_CCR = &TIMx->CCR4;
            break;
        case TIM_CHANNEL_5:
            TIM_CCR = &TIMx->CCR5;
            break;
        case TIM_CHANNEL_6:
            TIM_CCR = &TIMx->CCR6;
            break;
        default:
            return;
    }

    HAL_TIM_PWM_Stop(htim, channel);
    PWM_ParaGen(clk, frq, duty, &TIMx->PSC, &TIMx->ARR, TIM_CCR);
    HAL_TIM_PWM_Start(htim, channel);
}
