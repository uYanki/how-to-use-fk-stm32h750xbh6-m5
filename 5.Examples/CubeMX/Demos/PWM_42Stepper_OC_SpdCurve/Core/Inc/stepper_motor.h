
#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

#include "stm32h7xx_hal.h"
#include "main.h"
#include "malloc.h"
#include "stdbool.h"
#include "stdio.h"

#define STEPPER_ENABLE()    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_SET)
#define STEPPER_DISABLE()   HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET)

#define STEPPER_DIR_CW()    HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_SET)
#define STEPPER_DIR_CCW()   HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_RESET)

#define STEPPER_PWM_START() HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1)
#define STEPPER_PWM_STOP()  HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1)

enum STA {
    STOP = 0, /* 加减速曲线状态：停止*/
    ACCEL,    /* 加减速曲线状态：加速阶段*/
    DECEL,    /* 加减速曲线状态：减速阶段*/
    RUN       /* 加减速曲线状态：匀速阶段*/
};

enum DIR {
    CCW = 0, /* 逆时针 */
    CW       /* 顺时针 */
};

enum EN {
    EN_ON = 0, /* 失能脱机引脚 */
    EN_OFF     /* 使能脱机引脚 使能后电机停止旋转 */
};

#define STEPPER_MOTOR_1 1

#define ST_DIR(x)       x ? STEPPER_DIR_CW() : STEPPER_DIR_CCW()
#define ST_EN(x)        x ? STEPPER_ENABLE() : STEPPER_DISABLE()

/* 外部接口函数*/
void stepper_init(uint16_t arr, uint16_t psc); /* 步进电机接口初始化 */
void stepper_star(uint8_t motor_num);          /* 开启步进电机 */
void stepper_stop(uint8_t motor_num);          /* 关闭步进电机 */

#define DEMO 3

#if DEMO == 1

typedef struct
{
    __IO uint8_t  run_state;   /* 电机旋转状态 */
    __IO uint8_t  dir;         /* 电机旋转方向 */
    __IO int32_t  step_delay;  /* 下个脉冲周期（时间间隔），启动时为加速度 */
    __IO uint32_t decel_start; /* 开始减速位置 */
    __IO int32_t  decel_val;   /* 减速阶段步数 */
    __IO int32_t  min_delay;   /* 速度最快，计数值最小的值(最大速度，即匀速段速度) */
    __IO int32_t  accel_count; /* 加减速阶段计数值 */
} speedRampData;

#define MAX_STEP_ANGLE 0.225               /* 最小步距(1.8/MICRO_STEP) */
#define PAI            3.1415926           /* 圆周率*/
#define FSPR           200                 /* 步进电机单圈步数 */
#define MICRO_STEP     8                   /* 步进电机驱动器细分数 */
#define T1_FREQ        1e6                 /* 频率ft值 */
#define SPR            (FSPR * MICRO_STEP) /* 旋转一圈需要的脉冲数 */

/* 数学常数 */

#define ALPHA          ((float)(2 * PAI / SPR)) /* α = 2*pi/spr */
#define A_T_x10        ((float)(10 * ALPHA * T1_FREQ))
#define T1_FREQ_148    ((float)((T1_FREQ * 0.69) / 10)) /* 0.69为误差修正值 */
#define A_SQ           ((float)(2 * 100000 * ALPHA))
#define A_x200         ((float)(200 * ALPHA)) /* 2*10*10*a/10 */

void create_t_ctrl_param(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed); /* 梯形加减速控制函数 */

//--------------

#elif DEMO == 2

#define T1_FREQ                1e6  // Hz
#define FSPR                   200  /* 步进电机单圈步数 */
#define MICRO_STEP             8
#define SPR                    (FSPR * MICRO_STEP) /* 单圈所需要的脉冲数 */

#define ROUNDPS_2_STEPPS(rpm)  ((rpm) * SPR / 60)                 /* 根据电机转速（r/min），计算电机步速（step/s） */
#define MIDDLEVELOCITY(vo, vt) (((vo) + (vt)) / 2)                /* S型加减速加速段的中点速度  */
#define INCACCEL(vo, v, t)     ((2 * ((v) - (vo))) / pow((t), 2)) /* 加加速度:加速度增加量   V - V0 = 1/2 * J * t^2 */
#define INCACCELSTEP(j, t)     (((j) * pow((t), 3)) / 6.0f)       /* 加加速段的位移量(步数)  S = 1/6 * J * t^3 */
#define ACCEL_TIME(t)          ((t) / 2)                          /* 加加速段和减加速段的时间是相等的 */
#define SPEED_MIN              (T1_FREQ / (65535.0f))             /* 最低频率/速度 */

typedef struct {
    int32_t vo;         /*  初速度 单位 step/s */
    int32_t vt;         /*  末速度 单位 step/s */
    int32_t accel_step; /*  加速段的步数单位 step */
    int32_t decel_step; /*  加速段的步数单位 step */
    float*  accel_tab;  /*  速度表格 单位 step/s 步进电机的脉冲频率 */
    float*  decel_tab;  /*  速度表格 单位 step/s 步进电机的脉冲频率 */
    float*  ptr;        /*  速度指针 */
    int32_t dec_point;  /*  减速点 */
    int32_t step;
    int32_t step_pos;
} speed_calc_t;

typedef enum {
    STATE_ACCEL    = 1, /* 电机加速状态 */
    STATE_AVESPEED = 2, /* 电机匀速状态 */
    STATE_DECEL    = 3, /* 电机减速状态 */
    STATE_STOP     = 0, /* 电机停止状态 */
    STATE_IDLE     = 4, /* 电机空闲状态 */
} motor_state_typedef;

void    stepmotor_move_rel(int32_t vo, int32_t vt, float AcTime, float DeTime, int32_t step); /* S型加减速运动控制函数 */
uint8_t calc_speed(int32_t vo, int32_t vt, float time);                                       /* 计算速度表 */

#elif DEMO == 3  // 未测

/* 直线插补定义 */

#define AXIS_X                    0 /* X轴标号 */
#define AXIS_Y                    1 /* Y轴标号 */
#define LINE                      0

/* X轴电机相关引脚定义 */
#define STEPMOTOR_TIM_CHANNEL1    TIM_CHANNEL_3 /* 定时器8通道3 X轴 */
#define STEPMOTOR_TIM_PULSE_PIN_X GPIO_PIN_7    /* 输出脉冲给X轴步进驱动器 */

#define STEPMOTOR_X_DIR_PORT      GPIOB      /* X轴方向脚 */
#define STEPMOTOR_X_DIR_PIN       GPIO_PIN_2 /* X轴方向脚 */

#define STEPMOTOR_X_ENA_PORT      GPIOF         /* X轴使能脚 */
#define STEPMOTOR_X_ENA_PIN       GPIO_PIN_11   /* X轴使能脚 */

/* Y轴电机相关引脚定义 */
#define STEPMOTOR_TIM_CHANNEL2    TIM_CHANNEL_4 /* 定时器8通道4 Y轴 */
#define STEPMOTOR_TIM_PULSE_PIN_Y GPIO_PIN_9    /* 输出脉冲给Y轴步进驱动器 */

#define STEPMOTOR_Y_DIR_PORT      GPIOH      /* Y轴方向脚 */
#define STEPMOTOR_Y_DIR_PIN       GPIO_PIN_2 /* Y轴方向脚 */

#define STEPMOTOR_Y_ENA_PORT      GPIOH      /* Y轴使能脚 */
#define STEPMOTOR_Y_ENA_PIN       GPIO_PIN_3 /* Y轴使能脚 */

/* ------------------------设置电机方向,参数axis:当前活动轴---------------------------- */
#define ST_LINE_DIR(x, axis)                                                                                                                                                       \
    do {                                                                                                                                                                           \
        x ? HAL_GPIO_WritePin(st_motor[axis].dir_port, st_motor[axis].dir_pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(st_motor[axis].dir_port, st_motor[axis].dir_pin, GPIO_PIN_RESET); \
    } while (0)
/* -----------------------设置电机使能,参数axis:当前活动轴 -----------------------------*/
#define ST_LINE_EN(x, axis)                                                                                                                                                    \
    do {                                                                                                                                                                       \
        x ? HAL_GPIO_WritePin(st_motor[axis].en_port, st_motor[axis].en_pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(st_motor[axis].en_port, st_motor[axis].en_pin, GPIO_PIN_RESET); \
    } while (0)

typedef enum /* 电机状态 */
{
    STATE_STOP = 0,
    STATE_RUN  = 1,
} st_motor_status_def;

typedef struct {
    uint16_t      pulse_pin;     /* 定时器脉冲输出引脚 */
    uint32_t      pulse_channel; /* 定时器脉冲输出通道 */
    uint16_t      en_pin;        /* 电机使能引脚编号 */
    uint16_t      dir_pin;       /* 电机方向引脚编号 */
    GPIO_TypeDef* dir_port;      /* 电机方向引脚端口 */
    GPIO_TypeDef* en_port;       /* 电机使能引脚端口 */
} st_motor_ctr_def;

/*  插补算法类型定义 */
typedef struct {
    __IO uint8_t  moving_mode; /* 运动模式 */
    __IO uint8_t  inter_dir;   /* 插补方向 */
    __IO uint8_t  qua_points;  /* 象限点 */
    __IO uint8_t  x_dir;       /* X轴方向 */
    __IO uint8_t  y_dir;       /* Y轴方向 */
    __IO int32_t  end_x;       /* 终点坐标X */
    __IO int32_t  end_y;       /* 终点坐标Y */
    __IO uint32_t end_pulse;   /* 终点位置总的脉冲数 */
    __IO uint32_t act_axis;    /* 活动轴 */
    __IO int32_t  f_e;         /* 函数方程 */
} inter_pol_def;

void stepper_pwmt_speed(uint16_t speed, uint32_t Channel);             /* 设置速度 */
void line_inpolation(int32_t coordsX, int32_t coordsY, int32_t Speed); /* 实现任意象限直线插补 */

#endif

#endif
