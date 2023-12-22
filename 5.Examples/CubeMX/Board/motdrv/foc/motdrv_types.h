#ifndef __MOTDRV_TYPES_H__
#define __MOTDRV_TYPES_H__

#include "stm32h7xx_hal.h"
#include "stdbool.h"

typedef float float32_t;

typedef enum {
    SECTOR_1,
    SECTOR_2,
    SECTOR_3,
    SECTOR_4,
    SECTOR_5,
    SECTOR_6,
} sector_e;

typedef enum {
    MOTDRV_CTRLMODE_DQ,
    MOTDRV_CTRLMODE_ALPHA_BETA,
    MOTDRV_CTRLMODE_PHASE,
    MOTDRV_CTRLMODE_SVPWM,
} MotDrv_CtrlMode_e;

#define M_PI          3.1415926f  ///< π = 3.14159265358979323846
#define M_PI_X2       6.2831852f  ///< 2π
#define M_PI_X100     314         ///< 100π
#define M_RAD2DGE     57.295779f  ///< 1 radians to 57.295780 degress
#define M_SQRT2       1.414213f   ///< sqrt(2)
#define M_SQRT3       1.732050f   ///< sqrt(3)
#define M_INVSQRT2    0.707106f   ///< 1/sqrt(2)
#define M_INVSQRT3    0.577350f   ///< 1/sqrt(3)
#define M_INVSQRT3_X2 1.154700f   ///< 2/sqrt(3)

typedef struct {
    uint8_t   PolePairs;  // 极对数
    float32_t Umdc;       // 母线电压
    uint16_t  CarryFreq;  // 载波频率(8k有噪音,16k&24k没有)

    uint16_t DutyMax;
} MotorInfo_t;

// clang-format off

typedef struct {

    // float32_t CurPhA;
    // float32_t CurPhB;
    // float32_t CurPhC;

    // float32_t CurPhA;
    // float32_t CurPhB;
    // float32_t CurPhC;

    float32_t theta;  // rotating angle (electrical angle)
    float32_t sin;    // sine(theta)
    float32_t cos;    // cosine(theta)

    float32_t d;  // rotating d-axis stator variable
    float32_t q;  // rotating q-axis stator variable

    float32_t alpha;  // stationary d-axis stator
    float32_t beta;   // stationary q-axis stator

    float32_t phase_a;  // phase-a stator
    float32_t phase_b;  // phase-b stator
    float32_t phase_c;  // phase-c stator


    float32_t phase_min;  // phase-min
    float32_t phase_mid;  // phase-mid
    float32_t phase_max;  // phase-max

    uint8_t sector;  // Space vector sector

    float32_t Ta;  // phase-a switching function
    float32_t Tb;  // phase-b switching function
    float32_t Tc;  // phase-c switching function

    float32_t Umdc;

} foc_t;

// clang-format on

void park(foc_t* v);
void ipark(foc_t* v);
void clarke(foc_t* v);
void iclarke(foc_t* v);
void ph_order(foc_t* v);
void svpwm(foc_t* v);
void zero_inject(foc_t* v);

#endif
