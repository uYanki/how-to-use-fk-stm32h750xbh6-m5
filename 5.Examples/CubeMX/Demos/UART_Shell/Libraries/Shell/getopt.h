#ifndef __SHELL_GETOPT_H__
#define __SHELL_GETOPT_H__

#include "./core.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define CMD_REQS_END \
    {                \
        0, 0, 0, 0   \
    }
#define CMD_OPTS_END  \
    {                 \
        0, 0, 0, 0, 0 \
    }

// don't use -1 (EOF)
typedef enum {
    CMD_ERR_NO                     = 0,
    CMD_ERR_ARGUMENT_UNKNOWN       = -100,
    CMD_ERR_ARGUMENT_INVALID_TYPE  = -101,
    CMD_ERR_ARGUMENT_INVALID_VALUE = -102,
    CMD_ERR_ARGUMENT_TOO_FEW       = -103,
    CMD_ERR_ARGUMENT_TOO_MANY      = -104,
} cmd_err_e;

typedef enum {
    //---------------------------------
    // 位置参数, Positional

    ARGUMENT_REQUIRED_TYPE_INTEGER,
    ARGUMENT_REQUIRED_TYPE_FLOAT,
    ARGUMENT_REQUIRED_TYPE_STRING,

    //---------------------------------
    // 可选参数, Optional

    // - without argument
    ARGUMENT_OPTIONAL_TYPE_BOOLEAN,
    // - with argument
    ARGUMENT_OPTIONAL_TYPE_INTEGER,
    ARGUMENT_OPTIONAL_TYPE_FLOAT,
    ARGUMENT_OPTIONAL_TYPE_STRING,

} arg_type_e;

typedef struct {
    int32_t     uid;
    arg_type_e  type;
    const char* name;
    const char* desc;
} arg_req_t;

typedef struct {
    int32_t     uid;
    arg_type_e  type;
    const char* lname;  // long name
    const char  sname;  // short name
    const char* desc;
} arg_opt_t;

typedef struct {
    int          argc;
    const char** argv;

    struct {
        arg_req_t* base;
        int        count;
        int        index;  // current index
    } reqs;

    struct {
        arg_opt_t* base;
        int        index;
        int        count;
    } opts;

    struct {
        int   index;
        char* token;
        union {
            int32_t   i32;
            float32_t f32;
            char*     str;
        } value;
    } curr;  // current

} cmd_ctx_t;

const char* cmd_get_errstr(cmd_err_e err);

bool cmd_ctx_init(shell_t* shell, cmd_ctx_t* ctx, int argc, char** argv, const arg_req_t* reqs, const arg_opt_t* opts);
int  cmd_ctx_next(shell_t* shell, cmd_ctx_t* ctx);

uint8_t cmd_parse_arg(shell_t* shell, const char* str, int32_t* out);

// void cmd_generate_help();

#if defined(__cplusplus)
}
#endif

#endif
