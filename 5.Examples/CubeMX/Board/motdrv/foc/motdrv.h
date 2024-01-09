#ifndef __MOTDRV_H__
#define __MOTDRV_H__

#include "stm32h7xx_hal.h"
#include "stdbool.h"

#include "pwmctrl.h"
#include "paratbl.h"

typedef enum {
    SECTOR_1,
    SECTOR_2,
    SECTOR_3,
    SECTOR_4,
    SECTOR_5,
    SECTOR_6,
} sector_e;

typedef struct {
    float PP;
    float Rs;
    float Ls;
    float Ld;
    float Lq;
    float Kemf;
    float FluxM;
} MotorPara_t;

#define M_PI          3.1415926f  ///< π = 3.14159265358979323846
#define M_PI_X2       6.2831852f  ///< 2π
#define M_PI_X100     314         ///< 100π
#define M_RAD2DGE     57.295779f  ///< 1 radians to 57.295780 degress
#define M_SQRT2       1.414213f   ///< sqrt(2)
#define M_SQRT3       1.732050f   ///< sqrt(3)
#define M_INVSQRT2    0.707106f   ///< 1/sqrt(2)
#define M_INVSQRT3    0.577350f   ///< 1/sqrt(3)
#define M_INVSQRT3_X2 1.154700f   ///< 2/sqrt(3)

#define max(a, b)     ((a) > (b) ? (a) : (b))
#define min(a, b)     ((a) < (b) ? (a) : (b))

//-----------------------------------

typedef enum {
    CTRL_MODE_TRQ,
    CTRL_MODE_SPD,
    CTRL_MODE_POS,
} ctrl_mode_e;

typedef enum {
    // openloop
    ELEC_ANG_SRC_NONE,
    // closeloop
    ELEC_ANG_SRC_HALL,
    ELEC_ANG_SRC_INC,
    ELEC_ANG_SRC_ABS,
} elec_ang_src_e;

typedef enum {
    FOC_MODE_VQ_VD,
    FOC_MODE_VAPLHA_VBETA,
    FOC_MODE_PWM_DUTY,
    FOC_MODE_PHASE,
} foc_mode_e;

typedef struct {
    int16_t  sin;
    int16_t  cos;
    uint16_t angle;
} sincos_t;

typedef struct {
    float32_t sin;  // sine(theta)
    float32_t cos;  // cosine(theta)

    float32_t d;  // rotating d-axis stator variable
    float32_t q;  // rotating q-axis stator variable

    float32_t alpha;  // stationary d-axis stator
    float32_t beta;   // stationary q-axis stator

} park_t, ipark_t;

typedef struct {
    float32_t alpha;  // stationary d-axis stator
    float32_t beta;   // stationary q-axis stator

    float32_t phase_a;  // mA, phase-a stator
    float32_t phase_b;  // mA, phase-b stator
    float32_t phase_c;  // mA, phase-c stator

} clarke_t, iclarke_t;

typedef struct {
    float32_t phase_a;  // mA, phase-a stator
    float32_t phase_b;  // mA, phase-b stator
    float32_t phase_c;  // mA, phase-c stator

    float32_t phase_min;
    float32_t phase_mid;
    float32_t phase_max;

    float32_t Ta;  // phase-a switching function, ton
    float32_t Tb;  // phase-b switching function, ton
    float32_t Tc;  // phase-c switching function, ton

    uint8_t sector;  // Space vector sector

    uint16_t period;

} svpwm_t;

void spdloop(void);
void curloop(void);

void ifoc(void);
void ofoc(void);

//

#define BEMF_DIR_STANDSTILL    2
#define BEMF_DIR_CW            1
#define BEMF_DIR_CCW           0

#define BEMF_DETECT_LIMIT_TIME 600  // unit: 1ms, it define the maximum time to detect BEMF's speed and direction

typedef struct {
    uint16_t u16BEMFSpeed;                    // motor BEMF's speed
    uint8_t  u8BEMFDirectionFlag;             // motor BEMF's direction
    uint16_t u16BEMF1msCounter;               // for BEMF's speed counter
    uint16_t u16BEMFDetectionTime;            // BEMF's checking time define
    uint16_t u16BEMFPhaseABMiddlePoint12bit;  // phase A,B's BEMF midpoint voltage
    uint16_t u16BEMFComparatorHystersis;      // unit:12bit ADC,digital comparator's hysteresys
    uint16_t u16BEMFStandstillThresholdVolt;  // unit:12bit ADC,define the standstill if BEMFA,B difference voltage under this limitation
    uint8_t  bBEMFResultValidFlag;            // if 1, means the result of BEMF checking is valid
    uint8_t  bBEMFMotorIsRotatingFlag;        // check the motor is rotating or not, 1:rotating now, 0:standstill
    uint8_t  u8BEMFPoleNumber;                // motor pole number;
} BEMF_Speed_Struct;

void BEMF_Speed_Detect(BEMF_Speed_Struct* Get_BEMF_Speed, u16 BEMFA_ADC_CHANNEL, u16 BEMFB_ADC_CHANNEL);

//----define the maximum difference voltage between BEMF A,B, if under this threshold voltage, the motor is in standstill status before startup---
// 它定义了数字比较器的磁滞电压
// 它定义了如果BEMFA，B在该限制下的电压差为静止
#ifdef ENABLE_ROTOR_IPD1_FUNCTION
#define BEMF_COMPARATOR_HYSTERESIS        100  // unit: mV,  it define the digital comparator's hysteresis voltage
#define BEMF_STANDSTILL_THRESHOLD_VOLTAGE 200  // unit: mV,  it define the standstill if BEMFA,B difference voltage under this limitation
#else
#define BEMF_COMPARATOR_HYSTERESIS        30   // unit: mV,  it define the digital comparator's hysteresis voltage
#define BEMF_STANDSTILL_THRESHOLD_VOLTAGE 200  // unit: mV,  it define the standstill if BEMFA,B difference voltage under this limitation
#endif


#endif
