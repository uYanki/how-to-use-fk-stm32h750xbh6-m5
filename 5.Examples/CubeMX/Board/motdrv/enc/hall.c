#include "hall.h"

hall_encoder_t HallEnc = {
    .HallState = 0,
    .Direction = 0,
    .Placement = 120,
    .ElecAngle = 0,
    .HA_Port   = ENC_HALL_A_GPIO_Port,
    .HA_Pin    = ENC_HALL_A_Pin,
    .HB_Port   = ENC_HALL_B_GPIO_Port,
    .HB_Pin    = ENC_HALL_B_Pin,
    .HC_Port   = ENC_HALL_C_GPIO_Port,
    .HC_Pin    = ENC_HALL_C_Pin,
};

void HallEnc_ReadState(hall_encoder_t* pEnc)
{
    uint8_t HallStateCur = 0, HallStatePrev;

    do {
        HallStatePrev = HallStateCur;
        HallStateCur  = 0;

        switch (pEnc->Placement)
        {
            case 60:  // DEGREES_60
            {
                HallStateCur |= ((HAL_GPIO_ReadPin(pEnc->HB_Port, pEnc->HB_Pin) == GPIO_PIN_SET) ^ 1) << 2;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HC_Port, pEnc->HC_Pin) == GPIO_PIN_SET) << 1;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HA_Port, pEnc->HA_Pin) == GPIO_PIN_SET);
                break;
            }
            case 120:  // DEGREES_120
            {
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HC_Port, pEnc->HC_Pin) == GPIO_PIN_SET) << 2;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HB_Port, pEnc->HB_Pin) == GPIO_PIN_SET) << 1;
                HallStateCur |= (HAL_GPIO_ReadPin(pEnc->HA_Port, pEnc->HA_Pin) == GPIO_PIN_SET);
                break;
            }
        }

    } while (HallStateCur != HallStatePrev);

    pEnc->HallState = HallStateCur;
}

void HallEnc_Init(hall_encoder_t* pEnc)
{
    HallEnc_ReadState(pEnc);

    switch (pEnc->HallState)
    {
        case HALL_STATE_2:
            pEnc->ElecAngle = 270;
            break;
        case HALL_STATE_6:
            pEnc->ElecAngle = 330;
            break;
        case HALL_STATE_4:
            pEnc->ElecAngle = 30;
            break;
        case HALL_STATE_5:
            pEnc->ElecAngle = 90;
            break;
        case HALL_STATE_1:
            pEnc->ElecAngle = 150;
            break;
        case HALL_STATE_3:
            pEnc->ElecAngle = 210;
            break;
        default:
            break;
    }

   // pEnc->ElecAngle -= 30;
}

void HallEnc_Update(hall_encoder_t* pEnc)
{
    uint8_t HallStatePrev = pEnc->HallState;
    uint8_t DirectionPrev = pEnc->Direction;

    HallEnc_ReadState(pEnc);

    switch (pEnc->HallState)  // 配速度
    {
        case HALL_STATE_2: {
            switch (HallStatePrev)
            {
                case HALL_STATE_3: {
                    pEnc->Direction = DIR_FWD;
                    pEnc->ElecAngle = 240;
                    break;
                }
                case HALL_STATE_6: {
                    pEnc->Direction = DIR_BACK;
                    pEnc->ElecAngle = 300;
                    break;
                }
            }
            break;
        }
        case HALL_STATE_6: {
            switch (HallStatePrev)
            {
                case HALL_STATE_2: {
                    pEnc->Direction = DIR_FWD;
                    pEnc->ElecAngle = 300;
                    break;
                }
                case HALL_STATE_4: {
                    pEnc->Direction = DIR_BACK;
                    pEnc->ElecAngle = 0;
                    break;
                }
            }
            break;
        }
        case HALL_STATE_4: {
            switch (HallStatePrev)
            {
                case HALL_STATE_6: {
                    pEnc->Direction = DIR_FWD;
                    pEnc->ElecAngle = 0;
                    break;
                }
                case HALL_STATE_5: {
                    pEnc->Direction = DIR_BACK;
                    pEnc->ElecAngle = 60;
                    break;
                }
            }
            break;
        }
        case HALL_STATE_5: {
            switch (HallStatePrev)
            {
                case HALL_STATE_4: {
                    pEnc->Direction = DIR_FWD;
                    pEnc->ElecAngle = 60;
                    break;
                }
                case HALL_STATE_1: {
                    pEnc->Direction = DIR_BACK;
                    pEnc->ElecAngle = 120;
                    break;
                }
            }
            break;
        }
        case HALL_STATE_1: {
            switch (HallStatePrev)
            {
                case HALL_STATE_5: {
                    pEnc->Direction = DIR_FWD;
                    pEnc->ElecAngle = 120;
                    break;
                }
                case HALL_STATE_3: {
                    pEnc->Direction = DIR_BACK;
                    pEnc->ElecAngle = 180;
                    break;
                }
            }
            break;
        };
        case HALL_STATE_3: {
            switch (HallStatePrev)
            {
                case HALL_STATE_1: {
                    pEnc->Direction = DIR_FWD;
                    pEnc->ElecAngle = 180;
                    break;
                }
                case HALL_STATE_2: {
                    pEnc->Direction = DIR_BACK;
                    pEnc->ElecAngle = 240;
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    if (pEnc->Direction != DirectionPrev)
    {
        /* Setting BufferFilled to 0 will prevent to compute the average speed based
         on the SpeedPeriod buffer values */
        // pEnc->BufferFilled = 0;
        // pEnc->SpeedFIFOIdx = 0;
    }
}