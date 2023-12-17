#include "hall.h"

int8_t HallEnc_ReadSector(void)
{
    // 霍尔真值 => 转子实际角度所在范围(N级指向)

    static const int8_t steps[] = {
        [0b100] = 0,  // 330 ~ 30
        [0b110] = 1,  // 30 ~ 90
        [0b010] = 2,  // 90 ~ 150
        [0b011] = 3,  // 150 ~ 210
        [0b001] = 4,  // 210 ~ 270
        [0b101] = 5,  // 270 ~ 330
        [0b000] = -1,
        [0b111] = -1,
    };

    uint8_t state = (((uint8_t)(HAL_GPIO_ReadPin(ENC_HALL_A_GPIO_Port, ENC_HALL_A_Pin) == GPIO_PIN_RESET) << 2u) |
                     ((uint8_t)(HAL_GPIO_ReadPin(ENC_HALL_B_GPIO_Port, ENC_HALL_B_Pin) == GPIO_PIN_RESET) << 1u) |
                     ((uint8_t)(HAL_GPIO_ReadPin(ENC_HALL_C_GPIO_Port, ENC_HALL_C_Pin) == GPIO_PIN_RESET) << 0u));

    return steps[state];
}
