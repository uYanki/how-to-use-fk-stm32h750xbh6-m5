#include "hall.h"

// ST HAL 霍尔模式配置 https://zhuanlan.zhihu.com/p/126478231
// 霍尔位置的对应角度 https://zhuanlan.zhihu.com/p/462932301
// 霍尔传感器信号超前于转子π/6，因此，电角度 = 霍尔角度 - π/6。正向旋转时

hall_encoder_t HallEnc = {
    .HallState = 0,
    .Direction = 0,
    .Placement = 120,
    .ElecAngle = 0,
    .EdgeCount = 0,
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
        case HALL_STATE_5:
            pEnc->ElecAngle = HALL_ANGLE_30 * 1;  // 30
            break;
        case HALL_STATE_1:
            pEnc->ElecAngle = HALL_ANGLE_30 * 3;  // 90
            break;
        case HALL_STATE_3:
            pEnc->ElecAngle = HALL_ANGLE_30 * 5;  // 150
            break;
        case HALL_STATE_2:
            pEnc->ElecAngle = HALL_ANGLE_30 * 6;  // 210
            break;
        case HALL_STATE_6:
            pEnc->ElecAngle = HALL_ANGLE_30 * 9;  // 270
            break;
        case HALL_STATE_4:
            pEnc->ElecAngle = HALL_ANGLE_30 * 11;  // 330
            break;
        default:
            break;
    }
}

void HallEnc_Update(hall_encoder_t* pEnc)
{
    uint8_t HallStatePrev = pEnc->HallState;
    uint8_t DirectionPrev = pEnc->Direction;

    HallEnc_Init(pEnc);

    if (pEnc->HallState != HallStatePrev)
    {
        pEnc->EdgeCount++;
    }

    return;

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

// RO u8 hall_table_cw[6]  = {6, 2, 3, 1, 5, 4};  // 顺时针旋转表
// RO u8 hall_table_ccw[6] = {5, 1, 3, 2, 6, 4};  // 逆时针旋转表

// direction_e HallDirChk(u8 prev, u8 cur)
// {
//     static RO u8 pTblCW[12]  = {0x62, 0x23, 0x31, 0x15, 0x54, 0x46, 0x63, 0x21, 0x35, 0x14, 0x56, 0x42};
//     static RO u8 pTblCCW[12] = {0x45, 0x51, 0x13, 0x32, 0x26, 0x64, 0x41, 0x53, 0x12, 0x36, 0x24, 0x65};

//     u8 match = ((prev & 0x0F) << 4) | (cur & 0x0F);

//     for (u8 i = 0; i < ARRAY_SIZE(pTblCW); ++i)
//     {
//         if (match == pTblCW[i])
//         {
//             return DIR_CCW;
//         }
//     }

//     for (u8 i = 0; i < ARRAY_SIZE(pTblCCW); ++i)
//     {
//         if (match == pTblCCW[i])
//         {
//             return DIR_CCW;
//         }
//     }

//     return DIR_NONE;
// }