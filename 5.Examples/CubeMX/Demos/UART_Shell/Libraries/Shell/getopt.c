#include "./getopt.h"

#include <string.h>
#include <ctype.h>

uint8_t cmd_parse_arg(shell_t* shell, const char* str, int32_t* out)
{
    char* p = str;

    int base = 10;

    int32_t  integer  = 0;
    uint32_t decimal  = 0;
    int32_t  exponent = 0;

    if (*p == '0')
    {
        switch (*++p)
        {
            case 'b':
            case 'B':  // bin int
            {
                p++;
                base = 2;
                goto __int_part;
            }

            case '0' ... '7':  // oct int
            {
                base = 8;
                goto __int_part;
            }

            case 'x':
            case 'X':  // hex int
            {
                p++;
                base = 16;
                goto __int_part;
            }

            case '\0':  // zero
            {
                integer = 0;
                break;
            }

            case 'e':
            case 'E':  // zero
            {
                goto __exp_part;
            }

            case '.':  // float
            {
                goto __deci_part;
            }

            default: {
                goto __error;
            }
        }
    }
    else
    {
    __int_part:

        if (*p == '\0')
        {
            goto __error;
        }

        integer = strtol(p, &p, base);

        switch (*p)
        {
            case 'e':
            case 'E':  // exp
            {
            __exp_part:

                if (p[1])
                {
                    exponent = strtol(++p, &p, base);
                }

                break;
            }

            case '.':  // deci
            {
            __deci_part:

                if (p[1])
                {
                    decimal = strtol(++p, &p, base);

                    if (*p == 'e' || *p == 'E')
                    {
                        exponent = strtol(++p, &p, base);
                    }
                }

                break;
            }
        }

        if (*p != '\0')
        {
            goto __error;
        }
    }

    //

    float32_t res = decimal;

    while (res >= 1)
    {
        res /= 10.f;
    }

    res += integer;

    if (exponent > 0)
    {
        while (exponent--)
        {
            res *= 10.f;
        }
    }
    else if (exponent < 0)
    {
        while (exponent++)
        {
            res /= 10.f;
        }
    }

    if (decimal == 0)  // interger
    {
        // 用 uint32_t 会导致负整数无法传递出去
        *(int32_t*)out = (int32_t)res;
        return 1;
    }
    else  // float
    {
        *(float32_t*)out = (float32_t)res;
        return 2;
    }

__error:
    // string
    *out = (uint32_t)str;
    return 0;
}

//

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>

// clang-format off
#define ARGUMENT_END { ARGUMENT_TYPE_END, 0, 0, 0, 0, 0}
// clang-format on

typedef enum {
    ARGUMENT_TYPE_INTEGER,  // parse as an interger
    ARGUMENT_TYPE_FLOAT,    // parse as a float
    ARGUMENT_TYPE_STRING,   // parse as a string
    ARGUMENT_TYPE_BOOLEAN,  // boolean, flip logical value
    ARGUMENT_TYPE_CUSTOM,   // call the custom parser to parse
    ARGUMENT_TYPE_END,      // the end of argument group
} arg_type_e;

typedef enum {
    ARGUMENT_ERROR_NONE,
    ARGUMENT_ERROR_REQUIRED_TOO_FEW,          // 过少的位置参数
    ARGUMENT_ERROR_REQUIRED_TOO_MANY,         // 过多的位置参数
    ARGUMENT_ERROR_OPTIONAL_UNKNOWN_NAME,     // 未匹配的可选参数名
    ARGUMENT_ERROR_OPTIONAL_MISSING_VALUE,    // 值缺失的可选参数
    ARGUMENT_ERROR_OPTIONAL_UNCORRECT_USAGE,  // 不正确的用法
    ARGUMENT_ERROR_ILLEGAL_VALUE,             // 不合法的值
    ARGUMENT_ERROR_EXPECTED_INTEGER,          // 值应为整数
    ARGUMENT_ERROR_EXPECTED_FLOAT,            // 值应为小数
    ARGUMENT_ERROR_EXPECTED_STRING,           // 值应为字符串
} arg_err_e;

typedef struct {
    arg_type_e type;

    const char* lname;  // long name
    const char  sname;  // short name
    const char* desc;   // description
    void*       value;  // the default or incoming value pointer

    // arg_err_e parser(arg_def_t* argdef, char* valstr);
    arg_err_e (*parser)();  // if type is custom, it will be called
} arg_def_t;

typedef struct {
    int    argc;    // argument count
    char** argv;    // argument vector
    char*  curstr;  // currently parsed content
} cmd_ctx_t;

#define LOGD(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

static int startswith(const char* str, const char* substr)
{
    const char* p = substr;

    while (*substr)
    {
#if 0
        // case-sensitive
        if (*str++ != *substr++)
#else
        // case-insensitive
        if (tolower(*str++) != tolower(*substr++))
#endif
        {
            return 0;
        }
    }

    // return the length of substr
    return (int)(substr - p);
}

static arg_err_e __parse_value(arg_def_t* argdef, char* valstr)
{
    arg_err_e err = ARGUMENT_ERROR_NONE;

    char* pend;

    switch (argdef->type)
    {
        case ARGUMENT_TYPE_INTEGER: {
            *(int32_t*)(argdef->value) = strtol(valstr, &pend, 0);
            
            if (*pend)
            {
                err = ARGUMENT_ERROR_EXPECTED_INTEGER;
            }
            break;
        }
        case ARGUMENT_TYPE_FLOAT: {
            *(float*)(argdef->value) = strtof(valstr, &pend);

            if (*pend)
            {
                err = ARGUMENT_ERROR_EXPECTED_FLOAT;
            }

            break;
        }
        case ARGUMENT_TYPE_STRING: {
            *(char**)(argdef->value) = valstr;
            break;
        }
        case ARGUMENT_TYPE_BOOLEAN: {
            *(int32_t*)(argdef->value) = !*(int32_t*)(argdef->value);
            break;
        }
        case ARGUMENT_TYPE_CUSTOM: {
            if (argdef->parser)
            {
                err = argdef->parser(argdef, valstr);
            }
            break;
        }
    }

    return err;
}

static arg_err_e get_opt_val(cmd_ctx_t* ctx, arg_def_t* argdef)
{
    arg_err_e err;

    char* valstr;

    switch (argdef->type)
    {
        // without argument
        case ARGUMENT_TYPE_BOOLEAN: break;

        // with argument
        case ARGUMENT_TYPE_INTEGER:
        case ARGUMENT_TYPE_FLOAT:
        case ARGUMENT_TYPE_STRING:
        case ARGUMENT_TYPE_CUSTOM: {
            if (*ctx->curstr)
            {
            }
            else if (ctx->argc > 0)
            {
                ctx->argc--;
                ctx->argv++;
                ctx->curstr = *ctx->argv;
            }
            else
            {
                return ARGUMENT_ERROR_OPTIONAL_MISSING_VALUE;
            }

            valstr = ctx->curstr;

            break;
        }
    }

    return __parse_value(argdef, valstr);
}

static arg_err_e get_req_val(cmd_ctx_t* ctx, arg_def_t* argdef)
{
    if (argdef != NULL && argdef->type != ARGUMENT_TYPE_END)
    {
        return __parse_value(argdef, ctx->curstr);
    }

    return ARGUMENT_ERROR_REQUIRED_TOO_MANY;
}

static arg_err_e __parse_short_option(cmd_ctx_t* ctx, arg_def_t* opts)
{
    if (opts != NULL)
    {
        for (arg_def_t* optdef = opts; optdef->type != ARGUMENT_TYPE_END; optdef++)
        {
            if (*ctx->curstr == optdef->sname)
            {
                ctx->curstr++;

                return get_opt_val(ctx, optdef);
            }
        }
    }

    return ARGUMENT_ERROR_OPTIONAL_UNKNOWN_NAME;
}

static arg_err_e __parse_long_option(cmd_ctx_t* ctx, arg_def_t* opts)
{
    if (opts != NULL)
    {
        for (arg_def_t* optdef = opts; optdef->type != ARGUMENT_TYPE_END; optdef++)
        {
            if (optdef->lname)  // no null
            {
                uint16_t len = startswith(ctx->curstr, optdef->lname);

                if (len > 0)
                {
                    ctx->curstr += len;

                    if (*ctx->curstr == '=')
                    {
                        ctx->curstr++;
                    }

                    return get_opt_val(ctx, optdef);
                }
            }
        }
    }

    return ARGUMENT_ERROR_OPTIONAL_UNKNOWN_NAME;
}

bool errcbk(cmd_ctx_t* ctx, arg_err_e err)
{
    switch (err)
    {
        case ARGUMENT_ERROR_NONE: return false;
        case ARGUMENT_ERROR_REQUIRED_TOO_FEW: LOGE("[wrn] too few arguments\n"); return false;
        case ARGUMENT_ERROR_REQUIRED_TOO_MANY: LOGE("[wrn] too many arguments: %s\n", ctx->curstr); return false;
        case ARGUMENT_ERROR_OPTIONAL_UNKNOWN_NAME: LOGE("[wrn] unknown option name: %s\n", ctx->curstr); return false;
        case ARGUMENT_ERROR_OPTIONAL_MISSING_VALUE: LOGE("[err] option requires a value\n"); return true;
        case ARGUMENT_ERROR_OPTIONAL_UNCORRECT_USAGE: LOGE("[err] uncorrect option usage: %s\n", ctx->curstr); return true;
        case ARGUMENT_ERROR_ILLEGAL_VALUE: LOGE("[err] illegal value: %s\n", ctx->curstr); return true;
        case ARGUMENT_ERROR_EXPECTED_INTEGER: LOGE("[err] expects an integer: %s\n", ctx->curstr); return true;
        case ARGUMENT_ERROR_EXPECTED_FLOAT: LOGE("[err] expects a float: %s\n", ctx->curstr); return true;
        case ARGUMENT_ERROR_EXPECTED_STRING: LOGE("[err] expects a string: %s\n", ctx->curstr); return true;
        default: return false;
    }
}

bool doarg(int argc, char** argv, arg_def_t* reqs, arg_def_t* opts)
{
    if (argc < 2 || argv == NULL)
    {
        return false;
    }

    cmd_ctx_t ctx = {
        .argc = argc - 1,
        .argv = argv + 1,
    };

    arg_def_t* reqdef = reqs;

    arg_err_e err = ARGUMENT_ERROR_NONE;

    for (; ctx.argc; ctx.argv++, ctx.argc--)
    {
        ctx.curstr = *ctx.argv;

        if (ctx.curstr[0] == '-')  // optional
        {
            switch (ctx.curstr[1])
            {
                case '\0':  // only '-'
                {
                    err = ARGUMENT_ERROR_OPTIONAL_UNCORRECT_USAGE;
                    break;
                }

                case '-':  //
                {
                    if (ctx.curstr[2] != '\0')  // long
                    {
                        ctx.curstr += 2;
                        err = __parse_long_option(&ctx, opts);
                    }
                    else  // only '--'
                    {
                        err = ARGUMENT_ERROR_OPTIONAL_UNCORRECT_USAGE;
                    }

                    break;
                }

                default:  // short
                {
                    ctx.curstr++;
                    err = __parse_short_option(&ctx, opts);
                    break;
                }
            }
        }
        else  // required
        {
            err = get_req_val(&ctx, reqdef);

            if (err == ARGUMENT_ERROR_NONE)
            {
                reqdef++;
            }
        }

        // true: stop parser
        // false: skip this error
        if (errcbk(&ctx, err))
        {
            return false;
        }
    }

    if (reqdef && reqdef->type != ARGUMENT_TYPE_END)
    {
        return !errcbk(NULL, ARGUMENT_ERROR_REQUIRED_TOO_FEW);
    }

    return true;
}

void generate_help(arg_def_t* reqs, arg_def_t* opts)
{
    LOGD("\nRequired\n\n");

    for (arg_def_t* arg = reqs; arg->type != ARGUMENT_TYPE_END; ++arg)
    {
        LOGD("\t");

        char* typestr;

        switch (arg->type)
        {
            case ARGUMENT_TYPE_INTEGER: typestr = "int"; break;
            case ARGUMENT_TYPE_FLOAT: typestr = "float"; break;
            case ARGUMENT_TYPE_STRING: typestr = "string"; break;
            case ARGUMENT_TYPE_BOOLEAN: typestr = "bool"; break;
            case ARGUMENT_TYPE_CUSTOM: typestr = "custom"; break;
            default: typestr = "unknown"; break;
        }

        (arg->lname) ? LOGD("--%-12s ", arg->lname) : LOGD("  %-*s ", 12, "");
        (arg->sname) ? LOGD("-%c ", arg->sname) : LOGD("  ");

        if (arg->desc)
        {
            LOGD("\t%-30s", arg->desc);
        }

        if (arg->value)
        {
            LOGD("\t");

            switch (arg->type)
            {
                case ARGUMENT_TYPE_INTEGER: LOGD("%d", *(int32_t*)arg->value); break;
                case ARGUMENT_TYPE_FLOAT: LOGD("%f", *(float*)arg->value); break;
                case ARGUMENT_TYPE_STRING: LOGD("%s", *(char**)arg->value); break;
                case ARGUMENT_TYPE_BOOLEAN: LOGD("%d", *(int32_t*)arg->value); break;
                case ARGUMENT_TYPE_CUSTOM: LOGD("%s", "custom"); break;
                default: LOGD("%s", "unknown"); break;
            }
        }

        LOGD("\n");
    }

    LOGD("\nOptional\n\n");

    for (arg_def_t* arg = opts; arg->type != ARGUMENT_TYPE_END; ++arg)
    {
        LOGD("\t");

        char* typestr;

        switch (arg->type)
        {
            case ARGUMENT_TYPE_INTEGER: typestr = "int"; break;
            case ARGUMENT_TYPE_FLOAT: typestr = "float"; break;
            case ARGUMENT_TYPE_STRING: typestr = "string"; break;
            case ARGUMENT_TYPE_BOOLEAN: typestr = "bool"; break;
            case ARGUMENT_TYPE_CUSTOM: typestr = "custom"; break;
            default: typestr = "unknown"; break;
        }

        (arg->lname) ? LOGD("--%-12s ", arg->lname) : LOGD("  %-*s ", 12, "");
        (arg->sname) ? LOGD("-%c ", arg->sname) : LOGD("  ");

        if (arg->desc)
        {
            LOGD("\t%-30s\t", arg->desc);
        }

        if (arg->value)
        {
            LOGD("\t");

            switch (arg->type)
            {
                case ARGUMENT_TYPE_INTEGER: LOGD("%d", *(int32_t*)arg->value); break;
                case ARGUMENT_TYPE_FLOAT: LOGD("%f", *(float*)arg->value); break;
                case ARGUMENT_TYPE_STRING: LOGD("%s", *(char**)arg->value); break;
                case ARGUMENT_TYPE_BOOLEAN: LOGD("%d", *(int32_t*)arg->value); break;
                case ARGUMENT_TYPE_CUSTOM: LOGD("%s", "custom"); break;
                default: LOGD("%s", "unknown"); break;
            }
        }

        LOGD("\n");
    }
}

int main()
{
    int     address = 0;
    int     i = 123, j = 456, k = 789;
    char *  s = "\0", *h = "hello world";
    int32_t x = 1, y = 0, z = 1;
    float   a = 0.33, b = 0.24545, c = 0.333;

    arg_def_t reqs[] = {
        {ARGUMENT_TYPE_INTEGER, "address", 0, "desc", &address, 0},
        ARGUMENT_END,
    };

    arg_def_t opts[] = {

        {ARGUMENT_TYPE_INTEGER, "integer0", 'i', "default 0",     &i, 0},
        {ARGUMENT_TYPE_INTEGER, "integer1", 'j', "default 1000",  &j, 0},
        {ARGUMENT_TYPE_INTEGER, "integer2", 'k', "default 2000",  &k, 0},

        {ARGUMENT_TYPE_BOOLEAN, "boolean0", 'x', "default no",    &x, 0},
        {ARGUMENT_TYPE_BOOLEAN, "boolean1", 'y', "default yes",   &y, 0},
        {ARGUMENT_TYPE_BOOLEAN, "boolean2", 'z', "default no ",   &z, 0},

        {ARGUMENT_TYPE_STRING,  "string0",  's', "default null",  &s, 0},
        {ARGUMENT_TYPE_STRING,  "string1",  'h', "hello world ",  &h, 0},

        {ARGUMENT_TYPE_FLOAT,   "float0",   'a', "default 1.234", &a, 0},
        {ARGUMENT_TYPE_FLOAT,   "float1",   'b', "default 1.234", &b, 0},
        {ARGUMENT_TYPE_FLOAT,   "float2",   'c', "default 1.234", &c, 0},

        ARGUMENT_END,
    };

    generate_help(reqs, opts);

    char* argv[] = {"tst", "-i222", "-k", "3333", "-x", "-y", "-sbbc sb", "--float0=", "122.22e3", "0x800000"};

    int argc = sizeof(argv) / sizeof(*argv);
    doarg(argc, argv, reqs, opts);

    LOGD("----------------------------------------\n");
    generate_help(reqs, opts);

    return 0;
}
