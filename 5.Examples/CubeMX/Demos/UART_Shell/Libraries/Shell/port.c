#include "gconf.h"
#include "usart.h"
#include "defs.h"

#include "./port.h"

int shell_puts(char* str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 0xFF);
}

bool shell_getc(char* c)
{
    return HAL_UART_Receive(&huart1, (uint8_t*)c, 1, 0xFF) == HAL_OK;
}

static shell_t shell = {
    .prompt = "uYanki@root: ",
    .getc   = shell_getc,
    .puts   = shell_puts,
};

void shell_setup(void)
{
#define LINE_BUFSIZE 64
    static char linebuf[LINE_BUFSIZE * (1 + CONFIG_SHELL_HISTROY_MAX_COUNT)] = {0};
    shell_init(&shell, linebuf, LINE_BUFSIZE);
#undef LINE_BUFSIZE
}

void shell_task(void)
{
    shell_loop(&shell);
}

shell_t* shell_get(void)
{
    return &shell;
}
