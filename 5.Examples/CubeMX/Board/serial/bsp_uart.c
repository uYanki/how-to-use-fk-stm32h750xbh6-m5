#include "gconf.h"
#include "usart.h"
#include "bsp_uart.h"

void Uart_SetWorkDir(uart_dir_e dir)
{
    switch (dir)
    {
        case UART_DIR_TX: {
#if CONFIG_UART_MODE == UART_MODE_RS485
            DelayBlockUS(CONFIG_RS485_SWITCH_TIME_US * 10);
            HAL_GPIO_WritePin(RS485_RTS_GPIO_Port, RS485_RTS_Pin, GPIO_PIN_RESET);
            DelayBlockUS(CONFIG_RS485_SWITCH_TIME_US);
#endif
            break;
        }
        case UART_DIR_RX: {
#if CONFIG_UART_MODE == UART_MODE_RS485
            DelayBlockUS(CONFIG_RS485_SWITCH_TIME_US);
            HAL_GPIO_WritePin(RS485_RTS_GPIO_Port, RS485_RTS_Pin, GPIO_PIN_SET);
            DelayBlockUS(CONFIG_RS485_SWITCH_TIME_US);
#endif
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// redirect printf

#if CONFIG_REDIRECT_PRINTF_SW

int fputc(int ch, FILE* f)
{
#if 1
    if (ch == '\n')
    {
        char ch = '\r';
        HAL_UART_Transmit(CONFIG_REDIRECT_UART_PORT, (uint8_t*)&ch, 1, 0xFF);
    }
#endif

    HAL_UART_Transmit(CONFIG_REDIRECT_UART_PORT, (uint8_t*)&ch, 1, 0xFF);
    return (ch);
}

#endif

#if defined(__CC_ARM)  // AC5

#pragma import(__use_no_semihosting)

struct __FILE {
    int handle;
};

FILE __stdout;

int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

void _sys_exit(int x)
{
    x = x;
}

#elif defined(__GNUC__)  // AC6

__asm(".global __use_no_semihosting\n\t");

void _sys_exit(int x)
{
    x = x;
}

//__use_no_semihosting was requested, but _ttywrch was

void _ttywrch(int ch)
{
    ch = ch;
}

FILE __stdout;

#endif

#ifndef NDEBUG

#include <stdlib.h>
#include <stdio.h>

int fputs(const char* str, FILE* stream)
{
    printf("%s", str);
    return 0;

    // if error
    // return EOF;
}

__attribute__((weak, noreturn)) void __aeabi_assert(const char* expr, const char* file, int line)
{
    char str[12], *p;

    fputs("*** assertion failed: ", stderr);
    fputs(expr, stderr);
    fputs(", file ", stderr);
    fputs(file, stderr);
    fputs(", line ", stderr);

    p    = str + sizeof(str);
    *--p = '\0';
    *--p = '\n';
    while (line > 0)
    {
        *--p = '0' + (line % 10);
        line /= 10;
    }
    fputs(p, stderr);

    abort();
}

__attribute__((weak)) void abort(void)
{
    for (;;)
        ;
}

#endif
