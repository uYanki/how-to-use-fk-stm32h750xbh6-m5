#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include "main.h"

#define LED_SET(level) HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, level)
#define LED_ON()       HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)
#define LED_OFF()      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET)
#define LED_TGL()      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin)

#endif
