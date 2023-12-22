#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include <stdio.h>

#include "stm32h7xx_hal.h"

typedef enum {
    UART_DIR_TX,
    UART_DIR_RX,
} uart_dir_e;

void Uart_SetWorkDir(uart_dir_e dir);

#endif
