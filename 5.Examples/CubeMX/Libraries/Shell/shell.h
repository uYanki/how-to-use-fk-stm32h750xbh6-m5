
#ifndef __SHELL_H__
#define __SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#if defined(__CC_ARM) || defined(__CLANG_ARM) || 1 /* ARM Compiler */
#define SECTION(x) __attribute__((section(x)))
#define CMD_USED   __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__) /* IAR Compiler */
#define SECTION(x) @x
#define CMD_USED   __root
#else
#error "not supported tool chain..."
#endif

#define shell_printf printf

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
