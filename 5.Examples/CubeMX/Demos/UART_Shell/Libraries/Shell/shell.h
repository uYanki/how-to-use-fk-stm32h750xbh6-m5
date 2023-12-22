#ifndef __SHELL_H__
#define __SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "defs.h"
#include "shell_conf.h"
#include "utils.h"

#if defined(__CC_ARM) || defined(__CLANG_ARM) || 1 /* ARM Compiler */
#define SECTION(x) __attribute__((section(x)))
#define CMD_USED   __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__) /* IAR Compiler */
#define SECTION(x) @x
#define CMD_USED   __root
#else
#error "not supported tool chain..."
#endif

#define FONT_COLO_WHITE         "\e[0m"
#define FONT_COLO_RED           "\e[31m"
#define FONT_COLO_GREEN         "\e[32m"
#define FONT_COLO_YELLOW        "\e[33m"
#define FONT_COLO_BLUE          "\e[34m"
#define FONT_COLO_PURPLE        "\e[35m"
#define FONT_COLO_LIGHTBLUE     "\e[36m"

#define shell_printf            printf
#define shell_error(fmt, ...)   shell_printf(FONT_COLO_RED fmt FONT_COLO_WHITE, ##__VA_ARGS__)
#define shell_success(fmt, ...) shell_printf(FONT_COLO_GREEN fmt FONT_COLO_WHITE, ##__VA_ARGS__)
#define shell_info(fmt, ...)    shell_printf(FONT_COLO_LIGHTBLUE fmt FONT_COLO_WHITE, ##__VA_ARGS__)
#define shell_warning(fmt, ...) shell_printf(FONT_COLO_YELLOW fmt FONT_COLO_WHITE, ##__VA_ARGS__)

typedef int (*cmd_cbk_t)(int argc, char** argv);

typedef struct {
    const char* name;
    const char* desc;
    uint32_t    hash;
    cmd_cbk_t   func;
} cmd_t;

#define CMD_EXPORT(name, desc, func) \
    SECTION("CMDS")                  \
    cmd_t _shell_cmd_##func = {#name, desc, 0, &func};

void shell_init(void);
void shell_loop(void);
int  shell_exec(char* cmd_str);

extern int  shell_getchar(void);
extern void shell_putchar(char ch);

#ifdef __cplusplus
}
#endif

#endif
