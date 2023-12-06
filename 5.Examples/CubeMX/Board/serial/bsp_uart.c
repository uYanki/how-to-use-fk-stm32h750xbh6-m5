#include "bsp_uart.h"

//-----------------------------------------------------------------------------
// redirect printf

extern UART_HandleTypeDef huart1;

int fputc(int ch, FILE* f)
{
#if 1
    if (ch == '\n')
    {
        char ch = '\r';
        HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFF);
    }
#endif

    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFF);
    return (ch);
}

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
