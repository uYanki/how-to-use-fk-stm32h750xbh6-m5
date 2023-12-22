#include "gconf.h"
#include "usart.h"

extern UART_HandleTypeDef huart1;

int shell_getchar(void)
{
    char ch;

    if (HAL_UART_Receive(&huart1, (uint8_t*)&ch, 1, 0xFF) == HAL_OK)
    {
        return ch;
    }

    return -1;
}

void shell_putchar(char ch)
{
    if (ch == '\n')
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r", 1, 0xFF);
    }

    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFF);
}
