

#include "stepper_motor.h"

#include "math.h"

extern TIM_HandleTypeDef htim1;

void stepper_star(uint8_t motor_num)
{
    HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
}

void stepper_stop(uint8_t motor_num)
{
    HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
}

#if DEMO == 1

/********************************************梯形加减速***********************************************/
speedRampData g_srd             = {STOP, CW, 0, 0, 0, 0, 0}; /* 加减速变量 */
__IO int32_t  g_step_position   = 0;                         /* 当前位置 */
__IO uint8_t  g_motion_sta      = 0;                         /* 是否在运动？0：停止，1：运动 */
__IO uint32_t g_add_pulse_count = 0;                         /* 脉冲个数累计 */

/*
 * @brief       生成梯形运动控制参数
 * @param       step：移动的步数 (正数为顺时针，负数为逆时针).
 * @param       accel  加速度,实际值为accel*0.1*rad/sec^2  10倍并且2个脉冲算一个完整的周期
 * @param       decel  减速度,实际值为decel*0.1*rad/sec^2
 * @param       speed  最大速度,实际值为speed*0.1*rad/sec
 * @retval      无
 */
void create_t_ctrl_param(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed)
{
    __IO uint16_t tim_count; /* 达到最大速度时的步数*/
    __IO uint32_t max_s_lim; /* 必须要开始减速的步数（如果加速没有达到最大速度）*/
    __IO uint32_t accel_lim;
    if (g_motion_sta != STOP) /* 只允许步进电机在停止的时候才继续*/
    {
        return;
    }
    if (step < 0) /* 步数为负数 */
    {
        g_srd.dir = CCW; /* 逆时针方向旋转 */
        ST_DIR(CCW);
        step = -step; /* 获取步数绝对值 */
    }
    else
    {
        g_srd.dir = CW; /* 顺时针方向旋转 */
        ST_DIR(CW);
    }

    if (step == 1) /* 步数为1 */
    {
        g_srd.accel_count = -1;    /* 只移动一步 */
        g_srd.run_state   = DECEL; /* 减速状态. */
        g_srd.step_delay  = 1000;  /* 默认速度 */
    }
    else if (step != 0) /* 如果目标运动步数不为0*/
    {
        /*设置最大速度极限, 计算得到min_delay用于定时器的计数器的值 min_delay = (alpha / t)/ w*/
        g_srd.min_delay = (int32_t)(A_T_x10 / speed);  // 匀速运行时的计数值

        /* 通过计算第一个(c0) 的步进延时来设定加速度，其中accel单位为0.1rad/sec^2
         step_delay = 1/tt * sqrt(2*alpha/accel)
         step_delay = ( tfreq*0.69/10 )*10 * sqrt( (2*alpha*100000) / (accel*10) )/100 */

        g_srd.step_delay = (int32_t)((T1_FREQ_148 * sqrt(A_SQ / accel)) / 10); /* c0 */

        max_s_lim = (uint32_t)(speed * speed / (A_x200 * accel / 10)); /* 计算多少步之后达到最大速度的限制 max_s_lim = speed^2 / (2*alpha*accel) */

        if (max_s_lim == 0) /* 如果达到最大速度小于0.5步，我们将四舍五入为0,但实际我们必须移动至少一步才能达到想要的速度 */
        {
            max_s_lim = 1;
        }
        accel_lim = (uint32_t)(step * decel / (accel + decel)); /* 这里不限制最大速度 计算多少步之后我们必须开始减速 n1 = (n1+n2)decel / (accel + decel) */

        if (accel_lim == 0) /* 不足一步 按一步处理*/
        {
            accel_lim = 1;
        }
        if (accel_lim <= max_s_lim) /* 加速阶段到不了最大速度就得减速。。。使用限制条件我们可以计算出减速阶段步数 */
        {
            g_srd.decel_val = accel_lim - step; /* 减速段的步数 */
        }
        else
        {
            g_srd.decel_val = -(max_s_lim * accel / decel); /* 减速段的步数 */
        }
        if (g_srd.decel_val == 0) /* 不足一步 按一步处理 */
        {
            g_srd.decel_val = -1;
        }
        g_srd.decel_start = step + g_srd.decel_val; /* 计算开始减速时的步数 */

        if (g_srd.step_delay <= g_srd.min_delay) /* 如果一开始c0的速度比匀速段速度还大，就不需要进行加速运动，直接进入匀速 */
        {
            g_srd.step_delay = g_srd.min_delay;
            g_srd.run_state  = RUN;
        }
        else
        {
            g_srd.run_state = ACCEL;
        }
        g_srd.accel_count = 0; /* 复位加减速计数值 */
    }
    g_motion_sta = 1; /* 电机为运动状态 */
    ST_EN(EN_ON);
    tim_count = __HAL_TIM_GET_COUNTER(&htim1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, tim_count + g_srd.step_delay / 2); /* 设置定时器比较值 */
    HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);                                     /* 使能定时器通道 */
}

/**
 * @brief  定时器比较中断
 * @param  htim：定时器句柄指针
 * @note   无
 * @retval 无
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim)
{
    __IO uint32_t        tim_count        = 0;
    __IO uint32_t        tmp              = 0;
    uint16_t             new_step_delay   = 0; /* 保存新（下）一个延时周期 */
    __IO static uint16_t last_accel_delay = 0; /* 加速过程中最后一次延时（脉冲周期） */
    __IO static uint32_t step_count       = 0; /* 总移动步数计数器*/
    __IO static int32_t  rest             = 0; /* 记录new_step_delay中的余数，提高下一步计算的精度 */
    __IO static uint8_t  i                = 0; /* 定时器使用翻转模式，需要进入两次中断才输出一个完整脉冲 */

    tim_count = __HAL_TIM_GET_COUNTER(&htim1);
    tmp       = tim_count + g_srd.step_delay / 2; /* 整个C值里边是需要翻转两次的所以需要除以2 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, tmp);

    i++;        /* 定时器中断次数计数值 */
    if (i == 2) /* 2次，说明已经输出一个完整脉冲 */
    {
        i = 0;                   /* 清零定时器中断次数计数值 */
        switch (g_srd.run_state) /* 加减速曲线阶段 */
        {
            case STOP:
                step_count = 0; /* 清零步数计数器 */
                rest       = 0; /* 清零余值 */
                /* 关闭通道*/
                HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
                ST_EN(EN_OFF);
                g_motion_sta = 0; /* 电机为停止状态  */
                break;

            case ACCEL:
                g_add_pulse_count++; /* 只用于记录相对位置转动了多少度 */
                step_count++;        /* 步数加1*/
                if (g_srd.dir == CW)
                {
                    g_step_position++; /* 绝对位置加1  记录绝对位置转动多少度*/
                }
                else
                {
                    g_step_position--; /* 绝对位置减1*/
                }
                g_srd.accel_count++;                                                                                 /* 加速计数值加1*/
                new_step_delay = g_srd.step_delay - (((2 * g_srd.step_delay) + rest) / (4 * g_srd.accel_count + 1)); /* 计算新(下)一步脉冲周期(时间间隔) */
                rest           = ((2 * g_srd.step_delay) + rest) % (4 * g_srd.accel_count + 1);                      /* 计算余数，下次计算补上余数，减少误差 */
                if (step_count >= g_srd.decel_start)                                                                 /* 检查是否到了需要减速的步数 */
                {
                    g_srd.accel_count = g_srd.decel_val; /* 加速计数值为减速阶段计数值的初始值 */
                    g_srd.run_state   = DECEL;           /* 下个脉冲进入减速阶段 */
                }
                else if (new_step_delay <= g_srd.min_delay) /* 检查是否到达期望的最大速度 计数值越小速度越快，当你的速度和最大速度相等或更快就进入匀速*/
                {
                    last_accel_delay = new_step_delay;  /* 保存加速过程中最后一次延时（脉冲周期）*/
                    new_step_delay   = g_srd.min_delay; /* 使用min_delay（对应最大速度speed）*/
                    rest             = 0;               /* 清零余值 */
                    g_srd.run_state  = RUN;             /* 设置为匀速运行状态 */
                }
                break;

            case RUN:
                g_add_pulse_count++;
                step_count++; /* 步数加1 */
                if (g_srd.dir == CW)
                {
                    g_step_position++; /* 绝对位置加1 */
                }
                else
                {
                    g_step_position--; /* 绝对位置减1*/
                }
                new_step_delay = g_srd.min_delay;    /* 使用min_delay（对应最大速度speed）*/
                if (step_count >= g_srd.decel_start) /* 需要开始减速 */
                {
                    g_srd.accel_count = g_srd.decel_val;  /* 减速步数做为加速计数值 */
                    new_step_delay    = last_accel_delay; /* 加阶段最后的延时做为减速阶段的起始延时(脉冲周期) */
                    g_srd.run_state   = DECEL;            /* 状态改变为减速 */
                }
                break;

            case DECEL:
                step_count++; /* 步数加1 */
                g_add_pulse_count++;
                if (g_srd.dir == CW)
                {
                    g_step_position++; /* 绝对位置加1 */
                }
                else
                {
                    g_step_position--; /* 绝对位置减1 */
                }
                g_srd.accel_count++;
                new_step_delay = g_srd.step_delay - (((2 * g_srd.step_delay) + rest) / (4 * g_srd.accel_count + 1)); /* 计算新(下)一步脉冲周期(时间间隔) */
                rest           = ((2 * g_srd.step_delay) + rest) % (4 * g_srd.accel_count + 1);                      /* 计算余数，下次计算补上余数，减少误差 */

                /* 检查是否为最后一步 */
                if (g_srd.accel_count >= 0) /* 判断减速步数是否从负值加到0是的话 减速完成 */
                {
                    g_srd.run_state = STOP;
                }
                break;
        }
        g_srd.step_delay = new_step_delay; /* 为下个(新的)延时(脉冲周期)赋值 */
    }
}

#elif DEMO == 2

/****************************************S型加减速运动*****************************************************/
volatile int32_t    g_step_pos     = 0;          /* 当前位置 */
volatile uint16_t   g_toggle_pulse = 0;          /* 脉冲频率控制 */
motor_state_typedef g_motor_sta    = STATE_IDLE; /* 电机状态 */
speed_calc_t        g_calc_t       = {0};

__IO uint32_t g_add_pulse_count = 0; /* 脉冲个数累计 */

/**
 * @brief       速度表计算函数
 * @param       vo,初速度;vt,末速度;time,加速时间
 * @retval      true：成功；false：失败
 */
uint8_t calc_speed(int32_t vo, int32_t vt, float time)
{
    uint8_t is_dec       = false;
    int32_t i            = 0;
    int32_t vm           = 0;    /* 中间点速度 */
    int32_t inc_acc_stp  = 0;    /* 加加速所需的步数 */
    int32_t dec_acc_stp  = 0;    /* 减加速所需的步数 */
    int32_t accel_step   = 0;    /* 加速或减速需要的步数 */
    float   jerk         = 0;    /* 加加速度 */
    float   ti           = 0;    /* 时间间隔 dt */
    float   sum_t        = 0;    /* 时间累加量 */
    float   delta_v      = 0;    /* 速度的增量dv */
    float   ti_cube      = 0;    /* 时间间隔的立方 */
    float*  velocity_tab = NULL; /* 速度表格指针 */

    if (vo > vt)                            /* 初速度比末速度大,做减速运动,数值变化跟加速运动相同 */
    {                                       /* 只是建表的时候注意将速度倒序 */
        is_dec      = true;                 /* 减速段 */
        g_calc_t.vo = ROUNDPS_2_STEPPS(vt); /* 转换单位 起速:step/s */
        g_calc_t.vt = ROUNDPS_2_STEPPS(vo); /* 转换单位 末速:step/s */
    }
    else
    {
        is_dec      = false; /* 加速段 */
        g_calc_t.vo = ROUNDPS_2_STEPPS(vo);
        g_calc_t.vt = ROUNDPS_2_STEPPS(vt);
    }

    time = ACCEL_TIME(time);                /* 得到加加速段的时间 */
                                            // printf("time=%f\r\n",time);
    vm   = (g_calc_t.vo + g_calc_t.vt) / 2; /* 计算中点速度 */

    jerk = fabs(2.0f * (vm - g_calc_t.vo) / (time * time)); /* 根据中点速度计算加加速度 */

    inc_acc_stp = (int32_t)(g_calc_t.vo * time + INCACCELSTEP(jerk, time)); /* 加加速需要的步数 */

    dec_acc_stp = (int32_t)((g_calc_t.vt + g_calc_t.vo) * time - inc_acc_stp); /* 减加速需要的步数 S = vt * time - S1 */

    /* 申请内存空间存放速度表 */
    accel_step = dec_acc_stp + inc_acc_stp; /* 加速需要的步数 */
    if (accel_step % 2 != 0)                /* 由于浮点型数据转换成整形数据带来了误差,所以这里加1 */
    {
        accel_step += 1;
    }
    /* mallo申请内存空间,记得释放 */
    velocity_tab = (float*)(malloc((accel_step + 1) * sizeof(float)));
    if (velocity_tab == NULL)
    {
        printf("内存不足!请修改参数: %d\n", (accel_step + 1) * sizeof(float));
        return false;
    }
    /*
     * 目标的S型速度曲线是对时间的方程,但是在控制电机的时候则是以步进的方式控制,所以这里对V-t曲线做转换
     * 得到V-S曲线,计算得到的速度表是关于步数的速度值.使得步进电机每一步都在控制当中
     */
    /* 计算第一步速度,根据第一步的速度值达到下一步的时间 */
    ti_cube         = 6.0f * 1.0f / jerk;       /* 根据位移和时间的公式S = 1/6 * J * ti^3 第1步的时间:ti^3 = 6 * 1 / jerk */
    ti              = pow(ti_cube, (1 / 3.0f)); /* ti */
    sum_t           = ti;
    delta_v         = 0.5f * jerk * pow(sum_t, 2); /* 第一步的速度 */
    velocity_tab[0] = g_calc_t.vo + delta_v;

    /*****************************************************/
    if (velocity_tab[0] <= SPEED_MIN) /* 以当前定时器频率所能达到的最低速度 */
    {
        velocity_tab[0] = SPEED_MIN;
    }

    /*****************************************************/

    for (i = 1; i < accel_step; i++)
    {
        /* 步进电机的速度就是定时器脉冲输出频率,可以计算出每一步的时间 */
        /* 得到第i-1步的时间 */
        ti = 1.0f / velocity_tab[i - 1]; /* 电机每走一步的时间 ti = 1 / Vn-1 */
        /* 加加速段速度计算 */
        if (i < inc_acc_stp)
        {
            sum_t += ti;                                   /* 从0开始到i的时间累积 */
            delta_v         = 0.5f * jerk * pow(sum_t, 2); /* 速度的变化量: dV = 1/2 * jerk * ti^2 */
            velocity_tab[i] = g_calc_t.vo + delta_v;       /* 得到加加速段每一步对应的速度 */
            /* 当最后一步的时候,时间并不严格等于time,所以这里要稍作处理,作为减加速段的时间 */
            if (i == inc_acc_stp - 1)
            {
                sum_t = fabs(sum_t - time);
            }
        }
        /* 减加速段速度计算 */
        else
        {
            sum_t += ti;                                                /* 时间累计 */
            delta_v         = 0.5f * jerk * pow(fabs(time - sum_t), 2); /* dV = 1/2 * jerk *(T-t)^2 看这个逆向看减加速的图 */
            velocity_tab[i] = g_calc_t.vt - delta_v;                    /* V = vt - delta_v */
            if (velocity_tab[i] >= g_calc_t.vt)
            {
                accel_step = i;
                break;
            }
        }
    }
    if (is_dec == true) /* 减速 */
    {
        float tmp_Speed = 0;
        /* 倒序排序 */
        for (i = 0; i < (accel_step / 2); i++)
        {
            tmp_Speed                        = velocity_tab[i];
            velocity_tab[i]                  = velocity_tab[accel_step - 1 - i]; /* 头尾速度对换 */
            velocity_tab[accel_step - 1 - i] = tmp_Speed;
        }

        g_calc_t.decel_tab  = velocity_tab; /* 减速段速度表 */
        g_calc_t.decel_step = accel_step;   /* 减速段的总步数 */
    }
    else /* 加速 */
    {
        g_calc_t.accel_tab  = velocity_tab; /* 加速段速度表 */
        g_calc_t.accel_step = accel_step;   /* 加速段的总步数 */
    }

    free(velocity_tab);

    return true;
}

/**
 * @brief       S型加减速运动
 * @param       vo:初速度;vt:末速度;AcTime:加速时间;DeTime:减速时间;step:步数;
 * @retval      无
 */
void stepmotor_move_rel(int32_t vo, int32_t vt, float AcTime, float DeTime, int32_t step)
{
    if (calc_speed(vo, vt, AcTime) == false) /* 计算出加速段的速度和步数 */
    {
        return;
    }
    if (calc_speed(vt, vo, DeTime) == false) /* 计算出减速段的速度和步数 */
    {
        return;
    }

    if (step < 0)
    {
        step = -step;
        ST_DIR(CCW);
    }
    else
    {
        ST_DIR(CW);
    }

    if (step >= (g_calc_t.decel_step + g_calc_t.accel_step)) /* 当总步数大于等于加减速度步数相加时，才可以实现完整的S形加减速 */
    {
        g_calc_t.step      = step;
        g_calc_t.dec_point = g_calc_t.step - g_calc_t.decel_step; /* 开始减速的步数 */
    }
    else /* 步数不足以进行足够的加减速 */
    {
        /* 步数不足不足以运动，要把前面申请的速度表所占内存释放，以便后续可重复申请 */
        free(g_calc_t.accel_tab); /* 释放加速段速度表 */
        free(g_calc_t.decel_tab); /* 释放减速段速度表 */
        printf("步数不足，参数设置错误!\r\n");
        return;
    }
    g_calc_t.step_pos = 0;
    g_motor_sta       = STATE_ACCEL; /* 电机为加速状态 */

    g_calc_t.ptr   = g_calc_t.accel_tab; /* 把加速段的速度表存储到ptr里边 */
    g_toggle_pulse = (uint32_t)(T1_FREQ / (*g_calc_t.ptr));
    g_calc_t.ptr++;
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint16_t)(g_toggle_pulse / 2)); /*  设置定时器比较值 */
    HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);                                   /* 使能定时器通道 */
    ST_EN(EN_ON);
}

/**
 * @brief  定时器比较中断
 * @param  htim：定时器句柄指针
 * @note   无
 * @retval 无
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim)
{
    volatile uint32_t       Tim_Count = 0;
    volatile uint32_t       tmp       = 0;
    volatile float          Tim_Pulse = 0;
    volatile static uint8_t i         = 0;

    {
        i++;        /* 定时器中断次数计数值 */
        if (i == 2) /* 2次，说明已经输出一个完整脉冲 */
        {
            i = 0;        /* 清零定时器中断次数计数值 */
            g_step_pos++; /* 当前位置 */
            if ((g_motor_sta != STATE_IDLE) && (g_motor_sta != STATE_STOP))
            {
                g_calc_t.step_pos++;
            }
            switch (g_motor_sta)
            {
                case STATE_ACCEL:
                    g_add_pulse_count++;
                    Tim_Pulse = T1_FREQ / (*g_calc_t.ptr);        /* 由速度表得到每一步的定时器计数值 */
                    g_calc_t.ptr++;                               /* 取速度表的下一位 */
                    g_toggle_pulse = (uint16_t)(Tim_Pulse / 2);   /* 翻转模式C需要除以2 */
                    if (g_calc_t.step_pos >= g_calc_t.accel_step) /* 当大于加速段步数就进入匀速 */
                    {
                        free(g_calc_t.accel_tab); /* 运动完要释放内存 */
                        g_motor_sta = STATE_AVESPEED;
                    }
                    break;
                case STATE_DECEL:
                    g_add_pulse_count++;
                    Tim_Pulse = T1_FREQ / (*g_calc_t.ptr); /* 由速度表得到每一步的定时器计数值 */
                    g_calc_t.ptr++;
                    g_toggle_pulse = (uint16_t)(Tim_Pulse / 2);
                    if (g_calc_t.step_pos >= g_calc_t.step)
                    {
                        free(g_calc_t.decel_tab); /* 运动完要释放内存 */
                        g_motor_sta = STATE_STOP;
                    }
                    break;
                case STATE_AVESPEED:
                    g_add_pulse_count++;
                    Tim_Pulse      = T1_FREQ / g_calc_t.vt;
                    g_toggle_pulse = (uint16_t)(Tim_Pulse / 2);
                    if (g_calc_t.step_pos >= g_calc_t.dec_point)
                    {
                        g_calc_t.ptr = g_calc_t.decel_tab; /* 将减速段的速度表赋值给ptr */
                        g_motor_sta  = STATE_DECEL;
                    }
                    break;
                case STATE_STOP:
                    HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1); /* 开启对应PWM通道 */
                    g_motor_sta = STATE_IDLE;
                    break;
                case STATE_IDLE:
                    break;
            }
        }
        /*  设置比较值 */
        Tim_Count = __HAL_TIM_GET_COUNTER(&htim1);
        tmp       = 0xFFFF & (Tim_Count + g_toggle_pulse);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, tmp);
    }
}

#elif DEMO == 3

/**
 * @brief       修改定时器频率进而设置步进电机速度
 * @param       speed   : 定时器的重装载值
 * @param       Channel : 定时器通道
 * @retval      无
 */
void stepper_pwmt_speed(uint16_t speed, uint32_t Channel)
{
    __HAL_TIM_SetAutoreload(&htim1, speed);
    __HAL_TIM_SetCompare(&htim1, Channel, __HAL_TIM_GET_AUTORELOAD(&htim1) >> 1);
}

/*****************************************************直线插补实验*****************************************************/

inter_pol_def g_pol_par = {0}; /* 直线插补参数值 */

const st_motor_ctr_def st_motor[2] =
    {
        {STEPMOTOR_TIM_PULSE_PIN_X,
         STEPMOTOR_TIM_CHANNEL1,
         STEPMOTOR_X_ENA_PIN,
         STEPMOTOR_X_DIR_PIN,
         STEPMOTOR_X_DIR_PORT,
         STEPMOTOR_X_ENA_PORT},
        {STEPMOTOR_TIM_PULSE_PIN_Y,
         STEPMOTOR_TIM_CHANNEL2,
         STEPMOTOR_Y_ENA_PIN,
         STEPMOTOR_Y_DIR_PIN,
         STEPMOTOR_Y_DIR_PORT,
         STEPMOTOR_Y_ENA_PORT},
};
__IO st_motor_status_def g_motor_sta = STATE_STOP; /* 步进电机运动状态 */

/**
 * @brief       步进电机驱动器定时器初始化
 * @param       无
 * @retval      无
 */
void stepmotor_init(void)
{
    __HAL_TIM_MOE_ENABLE(&htim1);              /* 主输出使能 */
    __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE); /* 清除更新中断标志位 */
}

/**
 * @brief       直线增量插补函数实现直线插补功能,两个步进电机分别向X轴和Y轴步进IncX,IncY步
 * @param       IncX    ：终点X轴坐标
 * @param       IncY    ：终点Y轴坐标
 * @param       Speed   ：进给速度
 * @retval      无
 */
void line_incmove(uint32_t IncX, uint32_t IncY, uint32_t Speed)
{
    /* 偏差方程置零 */
    g_pol_par.f_e = 0;

    /* 计算起点到终点坐标对应的脉冲数位置*/
    g_pol_par.end_x     = IncX;
    g_pol_par.end_y     = IncY;
    g_pol_par.end_pulse = g_pol_par.end_y + g_pol_par.end_x;

    /* 根据终点判断在直线上的进给方向,减少偏差 */
    if (g_pol_par.end_y > g_pol_par.end_x)
    {
        g_pol_par.act_axis = AXIS_Y; /* 第一步进给Y轴 */
        g_pol_par.f_e      = g_pol_par.f_e + g_pol_par.end_x;
    }
    else
    {
        g_pol_par.act_axis = AXIS_X; /* 第一步进给X轴 */
        g_pol_par.f_e      = g_pol_par.f_e - g_pol_par.end_y;
    }
    /* 设置通道的比较值 */
    __HAL_TIM_SET_COMPARE(&htim1, st_motor[AXIS_X].pulse_channel, Speed);
    __HAL_TIM_SET_COMPARE(&htim1, st_motor[AXIS_Y].pulse_channel, Speed);
    __HAL_TIM_SET_AUTORELOAD(&htim1, Speed * 2); /* ARR设置为比较值2倍，这样输出的波形就是50%的占空比 */

    TIM_CCxChannelCmd(TIM8, st_motor[g_pol_par.act_axis].pulse_channel, TIM_CCx_ENABLE); /* 使能通道输出 */
    HAL_TIM_Base_Start_IT(&htim1);                                                       /* 使能定时器以及开启更新中断 */
    g_motor_sta = STATE_RUN;                                                             /* 标记电机正在运动 */
}
/**
 * @brief       实现任意象限直线插补
 * @param       coordsX    ：终点X轴坐标
 * @param       coordsY    ：终点Y轴坐标
 * @param       Speed      ：进给速度
 * @retval      无
 */
void line_inpolation(int32_t coordsX, int32_t coordsY, int32_t Speed)
{
    if (g_motor_sta != STATE_STOP) /* 当前电机正在运转 */
    {
        return;
    }
    /* 其他象限的直线跟第一象限是一样,只是电机运动方向不一样 */
    g_pol_par.moving_mode = LINE;
    if (coordsX < 0) /* 当x轴小于0时，电机方向设为反向*/
    {
        g_pol_par.x_dir = CCW;
        coordsX         = -coordsX; /* 取绝对值 */
        ST_LINE_DIR(CCW, AXIS_X);
    }
    else
    {
        g_pol_par.x_dir = CW;
        ST_LINE_DIR(CW, AXIS_X);
    }
    if (coordsY < 0) /* 当y轴小于0时，电机方向设为反向*/
    {
        g_pol_par.y_dir = CCW;
        coordsY         = -coordsY; /* 取绝对值 */
        ST_LINE_DIR(CCW, AXIS_Y);
    }
    else
    {
        g_pol_par.y_dir = CW;
        ST_LINE_DIR(CW, AXIS_Y);
    }
    line_incmove(coordsX, coordsY, Speed);
}

/**
 * @brief       定时器中断回调函数
 * @param       htim ： 定时器句柄
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    __IO uint32_t axis = 0;                  /* 进给轴 */
    axis               = g_pol_par.act_axis; /* 当前进给轴 */

    /* 判断是否到达终点或者还没开始运动 */
    if (g_pol_par.end_pulse == 0)
    {
        return;
    }
    /* 根据进给方向 更新坐标值 */

    if (g_pol_par.moving_mode == LINE)
    {
        if (g_pol_par.f_e > 0) /* 偏差方程 > 0 ,说明当前位置位于直线上方,应向X轴进给 */
        {
            g_pol_par.act_axis = AXIS_X;
            g_pol_par.f_e      = g_pol_par.f_e - g_pol_par.end_y; /* 第一象限的X轴进给时,偏差计算 */
        }
        else if (g_pol_par.f_e < 0) /* 偏差方程 < 0 ,说明当前位置位于直线下方,应向Y轴进给 */
        {
            g_pol_par.act_axis = AXIS_Y;
            g_pol_par.f_e      = g_pol_par.f_e + g_pol_par.end_x; /* 第一象限的Y轴进给时,偏差计算 */
        }
        /* 偏差为0的时候,判断x,y轴终点的大小决定进给方向 */
        else if (g_pol_par.f_e == 0) /* 偏差方程 = 0 ,说明当前位置位于直线,应判断终点坐标再进给 */
        {
            if (g_pol_par.end_y > g_pol_par.end_x) /* 当Y轴更长的话，应向Y轴进给 */
            {
                g_pol_par.act_axis = AXIS_Y;
                g_pol_par.f_e      = g_pol_par.f_e + g_pol_par.end_x; /* 第一象限的Y轴进给时,偏差计算 */
            }
            else
            {
                g_pol_par.act_axis = AXIS_X;
                g_pol_par.f_e      = g_pol_par.f_e - g_pol_par.end_y;
            }
        }
    }
    /* 判断是否需要跟换进给轴 */
    if (axis != g_pol_par.act_axis)
    {
        TIM_CCxChannelCmd(TIM8, st_motor[axis].pulse_channel, TIM_CCx_DISABLE);
        TIM_CCxChannelCmd(TIM8, st_motor[g_pol_par.act_axis].pulse_channel, TIM_CCx_ENABLE);
    }
    /* 终点判别:总步长 */
    g_pol_par.end_pulse--;
    if (g_pol_par.end_pulse == 0)
    {
        g_motor_sta = STATE_STOP;                                                             /* 到达终点 */
        TIM_CCxChannelCmd(TIM8, st_motor[g_pol_par.act_axis].pulse_channel, TIM_CCx_DISABLE); /* 关闭当前轴输出 */
        HAL_TIM_Base_Stop_IT(&htim1);                                                         /* 停止定时器 */
    }
}

#endif
