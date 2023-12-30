#ifndef __SHELL_CORE_H__
#define __SHELL_CORE_H__

#include "./conf.h"

#include "defs.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#if defined(__CC_ARM) || defined(__CLANG_ARM) || defined(__GNUC__)
#define __SECTION(x) __attribute__((section(x)))
#define __USED       __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __SECTION(x) @x
#define __USED       __root
#else
#error "unsupported toolchain"
#endif

typedef enum {
    __cmd_type_func_start,
    CMD_TYPE_FUNC_C,     // int cbk(shell_t* shell, ...)
    CMD_TYPE_FUNC_MAIN,  // int cbk(shell_t* shell, int argc, char* argv[])
    __cmd_type_func_end,

    __cmd_type_var_start,
    CMD_TYPE_VAR_STRING,   // string
    CMD_TYPE_VAR_INT8,     // s8
    CMD_TYPE_VAR_INT16,    // s16
    CMD_TYPE_VAR_INT32,    // s32
    CMD_TYPE_VAR_INT64,    // s64
    CMD_TYPE_VAR_UINT8,    // u8
    CMD_TYPE_VAR_UINT16,   // u16
    CMD_TYPE_VAR_UINT32,   // u32
    CMD_TYPE_VAR_UINT64,   // u64
    CMD_TYPE_VAR_FLOAT32,  // f32
    CMD_TYPE_VAR_FLOAT64,  // f64
    __cmd_type_var_end,

    CMD_TYPE_KEY,
} cmd_type_t;

typedef enum {
    CMD_RO = 0,  // read only
    CMD_WO = 1,  // write only
    CMD_RW = 2,  // read & write
} cmd_rw_t;

#define FONT_COLO_WHITE         "\e[0m"
#define FONT_COLO_RED           "\e[31m"
#define FONT_COLO_GREEN         "\e[32m"
#define FONT_COLO_YELLOW        "\e[33m"
#define FONT_COLO_BLUE          "\e[34m"
#define FONT_COLO_PURPLE        "\e[35m"
#define FONT_COLO_LIGHTBLUE     "\e[36m"

// #define shell_printf            printf
#define shell_error(fmt, ...)   shell_printf(shell_get(), FONT_COLO_RED fmt FONT_COLO_WHITE, ##__VA_ARGS__)
#define shell_success(fmt, ...) shell_printf(shell_get(), FONT_COLO_GREEN fmt FONT_COLO_WHITE, ##__VA_ARGS__)
#define shell_info(fmt, ...)    shell_printf(shell_get(), FONT_COLO_LIGHTBLUE fmt FONT_COLO_WHITE, ##__VA_ARGS__)
#define shell_warning(fmt, ...) shell_printf(shell_get(), FONT_COLO_YELLOW fmt FONT_COLO_WHITE, ##__VA_ARGS__)

//

#define B_LEN_CMD_ATTR_ACCESS   3
#define B_LEN_CMD_ATTR_TYPE     5
#define B_LEN_CMD_ATTR_EXT      8

#define B_STB_CMD_ATTR_ACCESS   0
#define B_STB_CMD_ATTR_TYPE     (B_STB_CMD_ATTR_ACCESS + B_LEN_CMD_ATTR_ACCESS)
#define B_STB_CMD_ATTR_EXT      (B_STB_CMD_ATTR_TYPE + B_LEN_CMD_ATTR_TYPE)

typedef struct cmd   cmd_t;
typedef struct shell shell_t;

typedef int (*cmd_cbk_t)();

struct cmd {
    volatile uint32_t hash;

    const union {
        struct {
            const char* name;
            const char* desc;
            cmd_cbk_t   cbk;
        } func;

        struct {
            const char* name;
            const char* desc;
            void*       addr;
        } var;

        struct {
            uint32_t    value;
            const char* desc;
            cmd_cbk_t   cbk;
        } key;
    };

    const union {
        uint32_t all;
        struct {
            uint32_t   access : B_LEN_CMD_ATTR_ACCESS;
            cmd_type_t type : B_LEN_CMD_ATTR_TYPE;
            union {
                uint8_t  argc : B_LEN_CMD_ATTR_EXT;  // func
                cmd_rw_t rw : B_LEN_CMD_ATTR_EXT;    // var
            } ext;
        } bits;
    } attr;
};

struct shell {
    char* prompt;

    struct {
        cmd_t*   base;
        uint16_t count;
    } cmds;

    struct {
        cmd_t*   base;
        uint16_t count;
    } keys;  // hotkey

    struct {
        char*    buffer;
        uint16_t capacity;

        uint16_t length;
        uint16_t cursor;

        uint8_t argc;
        char*   argv[CONFIG_SHELL_PARAMETER_MAX_COUNT + 1];
        // [0]: cmd name
        // [1...]: args

        uint8_t byteshift;  // key
    } parser;

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

    struct {
        char*    buffer[CONFIG_SHELL_HISTROY_MAX_COUNT];
        uint16_t record;  // writer
        int16_t  offset;  // reader
        uint16_t count;
    } history;

#endif

    bool (*getc)(char*);
    int (*puts)(char*);
};

#define CMD_ACCESS(n)    ((uint32_t)(n) << B_STB_CMD_ATTR_ACCESS)
#define CMD_TYPE(n)      ((uint32_t)(n) << B_STB_CMD_ATTR_TYPE)
#define CMD_FUNC_ARGC(n) ((uint32_t)(n) << B_LEN_CMD_ATTR_EXT)
#define CMD_VAR_RW(n)    ((uint32_t)(n) << B_LEN_CMD_ATTR_EXT)

#define CMD_EXPORT_FUNC(_attr, _name, _cbk, _desc)  \
    static const char _cmd_name_##_name[] = #_name; \
    static const char _cmd_desc_##_name[] = #_desc; \
    __SECTION("CMDS")                               \
    __USED static cmd_t _cmd_##_name = {            \
        .attr.all  = _attr,                         \
        .func.name = _cmd_name_##_name,             \
        .func.cbk  = (cmd_cbk_t)_cbk,               \
        .func.desc = _cmd_desc_##_name,             \
    };

#define CMD_EXPORT_VAR(_attr, _name, _addr, _desc)  \
    static const char _cmd_name_##_name[] = #_name; \
    static const char _cmd_desc_##_name[] = #_desc; \
    __SECTION("CMDS")                               \
    __USED static cmd_t _cmd_##_name = {            \
        .attr.all = _attr,                          \
        .var.name = _cmd_name_##_name,              \
        .var.addr = (void*)_addr,                   \
        .var.desc = _cmd_desc_##_name,              \
    };

#define CMD_EXPORT_KEY(_attr, _key, _cbk, _desc)     \
    static const char _cmd_desc_##_key[] = #_desc;   \
    __SECTION("KEYS")                                \
    __USED static cmd_t _cmd_##_key = {              \
        .attr.all  = _attr | CMD_TYPE(CMD_TYPE_KEY), \
        .key.value = _key,                           \
        .key.cbk   = (cmd_cbk_t)_cbk,                \
        .key.desc  = _cmd_desc_##_key,               \
    };

void shell_init(shell_t* shell, char buffer[], uint16_t capacity);
void shell_loop(shell_t* shell);

void shell_printf(shell_t* shell, const char* fmt, ...);

void shell_list_cmds(shell_t* shell);
void shell_list_vars(shell_t* shell);
void shell_list_all(shell_t* shell);

void shell_log(shell_t* shell, char* buffer);

#endif
