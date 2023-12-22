#ifndef __GLOBAL_CONF_H__
#define __GLOBAL_CONF_H__

#define UART_MODE_NORMAL          0
#define UART_MODE_RS485           1

#define CONFIG_UART_MODE          UART_MODE_NORMAL

// 串口重定向
#define CONFIG_REDIRECT_UART_PORT &huart1
#define CONFIG_REDIRECT_PRINTF_SW 1

// 串口缓冲区大小
#define CONFIG_UART_TXBUF_SIZE    256
#define CONFIG_UART_RXBUF_SIZE    256

#if CONFIG_UART_MODE == UART_MODE_RS485
// RS485 换向时间
#define CONFIG_RS485_SWITCH_TIME_US 200
#endif

#endif
