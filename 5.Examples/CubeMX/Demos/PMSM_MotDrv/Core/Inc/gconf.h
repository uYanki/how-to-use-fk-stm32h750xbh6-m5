#ifndef __GLOBAL_CONF_H__
#define __GLOBAL_CONF_H__

//-----------------------------------------
// 软件版本号

#define CONFIG_VERSION_MAJOR 5  // Major
#define CONFIG_VERSION_MINOR 0  // Minor
#define CONFIG_VERSION_PATCH 1  // Patch

#define PROJECT_VERSION      ((CONFIG_VERSION_MAJOR * 10000) + (CONFIG_VERSION_MINOR * 100) + (CONFIG_VERSION_PATCH))

//-----------------------------------------
//

// 串口类型
#define UART_MODE_NORMAL 0
#define UART_MODE_RS485  1
#define UART_MODE_RS232  2
#define UART_MODE_RS422  3
#define CONFIG_UART_MODE UART_MODE_NORMAL

// 使能重定向
#define CONFIG_REDIRECT_UART_PORT &huart1
#define CONFIG_REDIRECT_PRINTF_SW 1

// 缓冲区大小
#define CONFIG_UART_TXBUF_SIZE 256
#define CONFIG_UART_RXBUF_SIZE 256

#if CONFIG_UART_MODE == UART_MODE_RS485
// RS485 换向时间
#define CONFIG_RS485_SWITCH_TIME_US 200
#endif

//-----------------------------------------
// 编码器

#define ENC_INC             0
#define ENC_ABS             1
#define ENC_ROT             2
#define ENC_HALL            3

#define CONFIG_ENCODER_TYPE ENC_HALL

#endif
