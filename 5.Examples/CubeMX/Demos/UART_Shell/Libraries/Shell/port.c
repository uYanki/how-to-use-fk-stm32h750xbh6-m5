#include "gconf.h"
#include "usart.h"
#include "defs.h"

#include "./port.h"

int shell_puts(char* str)
{
    // HAL_UART_Transmit(&huart1,(uint8_t*) str,strlen(str),0xFF) ;
    return printf("%s", str);
}

bool shell_getc(char* c)
{
    return HAL_UART_Receive(&huart1, (uint8_t*)c, 1, 0xFF) == HAL_OK;
}

void shell_putc(char c)
{
    if (c == '\n')
    {
        char ch = '\r';
        HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFF);
    }

    HAL_UART_Transmit(&huart1, (uint8_t*)&c, 1, 0xFF);
}

static shell_t shell = {
    .prompt = "uYanki@root: ",
    .getc   = shell_getc,
    .putc   = shell_putc,
    .puts   = shell_puts,
};

void shell_setup(void)
{
    static char linebuf[32 * (1 + CONFIG_SHELL_HISTROY_MAX_COUNT)] = {0};
    shell_init(&shell, linebuf, 32);
}

void shell_task(void)
{
    shell_loop(&shell);
}

shell_t* shell_get(void)
{
    return &shell;
}
