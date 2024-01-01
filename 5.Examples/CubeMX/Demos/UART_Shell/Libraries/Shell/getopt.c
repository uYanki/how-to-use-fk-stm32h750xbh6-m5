#include "./getopt.h"

#include <string.h>
#include <ctype.h>

static int str_starts_with(const char* str, const char* substr)
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

static bool cmd_opt_has_arg(arg_opt_t* opt)
{
    switch (opt->type)
    {
        default:
        case ARGUMENT_OPTIONAL_TYPE_BOOLEAN: {
            return false;
        }
        case ARGUMENT_OPTIONAL_TYPE_INTEGER:
        case ARGUMENT_OPTIONAL_TYPE_FLOAT:
        case ARGUMENT_OPTIONAL_TYPE_STRING: {
            return true;
        }
    }
}

bool cmd_ctx_init(shell_t* shell, cmd_ctx_t* ctx, int argc, char** argv, const arg_req_t* reqs, const arg_opt_t* opts)
{
    ctx->argc = argc - 1;
    ctx->argv = argv + 1;

    ctx->reqs.base  = reqs;
    ctx->reqs.index = 0;
    ctx->reqs.count = 0;

    ctx->opts.base  = opts;
    ctx->opts.index = 0;
    ctx->opts.count = 0;

    ctx->curr.index = 0;
    ctx->curr.token = 0;

    for (const arg_req_t* req = reqs; req->name; req++)
    {
        if (req->name[0] == '-')
        {
            return false;
        }

        ctx->reqs.count++;
    }

    for (const arg_opt_t* opt = opts; opt->lname || opt->sname; opt++)
    {
        if ((opt->lname[0] == '-') ||  // don't startwith '-'
            (opt->sname == '-') ||
            isdigit(opt->sname))  // not 0~9, the argument maybe negative number
        {
            return false;
        }

        ctx->opts.count++;
    }

    return true;
}

int cmd_ctx_next(shell_t* shell, cmd_ctx_t* ctx)
{
    if (ctx->curr.index == ctx->argc)
    {
        return -1;  // EOF
    }

    const char* token = ctx->curr.token = ctx->argv[ctx->curr.index++];

    //-------------------------------------------------------------------------
    // positional arguments

    if ((token[0] != '-') ||
        (token[0] == '-' && isdigit(token[1])))
    {
        if (ctx->reqs.index < ctx->reqs.count)
        {
            int32_t value = 0;

            switch (ctx->reqs.base[ctx->reqs.index].type)
            {
                case ARGUMENT_REQUIRED_TYPE_INTEGER:

                    switch (cmd_parse_arg(shell, token, &value))
                    {
                        case 0:  // string
                            return CMD_ERR_ARGUMENT_INVALID_VALUE;
                        case 1:  // int
                            ctx->curr.value.i32 = *(uint32_t*)&value;
                            break;
                        case 2:  // float
                            ctx->curr.value.i32 = *(float32_t*)&value;
                            break;
                    }
                    break;

                case ARGUMENT_REQUIRED_TYPE_FLOAT:

                    switch (cmd_parse_arg(shell, token, &value))
                    {
                        case 0:  // string
                            return CMD_ERR_ARGUMENT_INVALID_VALUE;
                        case 1:  // int
                            ctx->curr.value.f32 = *(uint32_t*)&value;
                            break;
                        case 2:  // float
                            ctx->curr.value.f32 = *(float32_t*)&value;
                            break;
                    }
                    break;

                case ARGUMENT_REQUIRED_TYPE_STRING:
                    ctx->curr.value.str = token;
                    break;
            }

            return ctx->reqs.base[ctx->reqs.index++].uid;
        }

        // too many positional argument
        return CMD_ERR_ARGUMENT_TOO_MANY;
    }

    //-------------------------------------------------------------------------
    // optional arguments

    arg_opt_t* found_opt = NULL;
    char*      found_arg = NULL;

    // short opt
    if (token[1] != '\0' &&
        token[1] != '-' &&
        token[2] == '\0')
    {
        for (const arg_opt_t* opt = ctx->opts.base; true; opt++)
        {
            if (opt->sname == 0)
            {
                if (opt->lname == NULL)
                {
                    break;  // the end
                }
                continue;
            }

            if (opt->sname == token[1])  // found opt
            {
                if (cmd_opt_has_arg(found_opt = opt))
                {
                    // check if next argv isn't opt
                    if ((ctx->curr.index < ctx->argc) &&
                        (ctx->argv[ctx->curr.index][0] != '-'))
                    {
                        found_arg = ctx->argv[ctx->curr.index++];
                    }
                }
                break;  // jump out the loop
            }
        }
    }
    // long opt
    else if (token[1] == '-' &&
             token[2] != '\0')
    {
        token += 2;

        for (const arg_opt_t* opt = ctx->opts.base; true; opt++)
        {
            if (opt->lname == NULL)
            {
                if (opt->sname == 0)
                {
                    break;  // the end
                }
                continue;
            }

            int len = str_starts_with(token, opt->lname);

            if (len > 0)
            {
                const char* p = token + len;

                switch (p[0])
                {
                    case '\0': {
                        break;
                    }
                    case '=': {
                        if (p[1] != '\0')
                        {
                            found_arg = &p[1];
                        }
                        break;
                    }
                    default: {
                        // not found, matched for example
                        // --test but it was --testing
                        continue;
                    }
                }

                if (cmd_opt_has_arg(found_opt = opt))
                {
                    // check if next argv isn't opt
                    if ((ctx->curr.index < ctx->argc) &&
                        (ctx->argv[ctx->curr.index][0] != '-'))
                    {
                        found_arg = ctx->argv[ctx->curr.index++];
                    }
                }

                break;  // jump out the loop
            }
        }
    }
    // malformed opt: "-", "-xyz" or "--"
    else
    {
        // error usage
        return CMD_ERR_ARGUMENT_UNKNOWN;
    }

    if (found_opt == NULL)
    {
        // unknown opt
        return CMD_ERR_ARGUMENT_UNKNOWN;
    }

    if (found_arg == NULL)
    {
        switch (found_opt->type)
        {
            default:
            case ARGUMENT_OPTIONAL_TYPE_BOOLEAN: {
                break;
            }
            case ARGUMENT_OPTIONAL_TYPE_INTEGER:
            case ARGUMENT_OPTIONAL_TYPE_FLOAT:
            case ARGUMENT_OPTIONAL_TYPE_STRING: {
                // missing optional argument
                return CMD_ERR_ARGUMENT_TOO_FEW;
            }
        }
    }
    else  // found_arg != NULL
    {
        char* end = NULL;

        switch (found_opt->type)
        {
            default:
            case ARGUMENT_OPTIONAL_TYPE_BOOLEAN: {
                return CMD_ERR_ARGUMENT_TOO_MANY;  // error usage
            }
            case ARGUMENT_OPTIONAL_TYPE_INTEGER: {
                ctx->curr.value.i32 = strtol(found_arg, &end, 0);
                break;
            }
            case ARGUMENT_OPTIONAL_TYPE_FLOAT: {
                ctx->curr.value.f32 = strtof(found_arg, &end);
                break;
            }
            case ARGUMENT_OPTIONAL_TYPE_STRING: {
                ctx->curr.value.str = found_arg;
                break;
            }
        }

        if (end != NULL && *end != '\0')
        {
            return CMD_ERR_ARGUMENT_INVALID_VALUE;
        }
    }

    ctx->opts.index++;

    return found_opt->uid;
}

const char* cmd_get_errstr(cmd_err_e err)
{
    switch (err)
    {
        case CMD_ERR_NO:
            return "no error";
        case CMD_ERR_ARGUMENT_UNKNOWN:
            return "error: unknown argument name";
        case CMD_ERR_ARGUMENT_INVALID_TYPE:
            return "error: invalid argument type";
        case CMD_ERR_ARGUMENT_INVALID_VALUE:
            return "error: invalid argument value";
        case CMD_ERR_ARGUMENT_TOO_FEW:
            return "error: too few arguments";
        case CMD_ERR_ARGUMENT_TOO_MANY:
            return "error: too many arguments";
        default:
            return "";
    }
}

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