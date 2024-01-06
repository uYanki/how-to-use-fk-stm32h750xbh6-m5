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

#endif
