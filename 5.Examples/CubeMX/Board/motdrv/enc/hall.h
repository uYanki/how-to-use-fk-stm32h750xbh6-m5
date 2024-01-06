#ifndef __HALL_ENC_H__
#define __HALL_ENC_H__

#include "main.h"

#if 0
#define HALL_ANGLE_30 30u
#else
#define HALL_ANGLE_30 65535u / 12
#endif

typedef enum {
    DIR_FWD  = +1,
    DIR_BACK = -1,
} dir_e;

typedef enum {
    HALL_STATE_0 = 0,
    HALL_STATE_1 = 1,
    HALL_STATE_2 = 2,
    HALL_STATE_3 = 3,
    HALL_STATE_4 = 4,
    HALL_STATE_5 = 5,
    HALL_STATE_6 = 6,
    HALL_STATE_7 = 7,
} hall_state_t;

typedef struct {
    float Speed_RPM;

    uint8_t  HallState;
    uint8_t  Direction;
    int16_t  ElecAngle;
    uint16_t EdgeCount;
    uint8_t  Placement;  // Sensor Placement
    uint8_t  PhaseShift;

    // GPIO
    GPIO_TypeDef* HA_Port;
    uint16_t      HA_Pin;
    GPIO_TypeDef* HB_Port;
    uint16_t      HB_Pin;
    GPIO_TypeDef* HC_Port;
    uint16_t      HC_Pin;

} hall_encoder_t;

extern hall_encoder_t HallEnc;

void HallEnc_Init(hall_encoder_t* pEnc);
void HallEnc_Update(hall_encoder_t* pEnc);

#endif
