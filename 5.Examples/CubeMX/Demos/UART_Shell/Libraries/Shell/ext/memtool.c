#include "../core.h"
#include "../getopt.h"

// hexdump

#define DEFAULT_DUMP_LEN             368
#define DEFAULT_DUMP_SZ              4
#define DUMP_BYTE_PER_LINE           16

#define ROUND_DOWN(__value, __align) ((__value) & ~((__align)-1))
#define ROUND_UP(__value, __align)   (((__value) + (__align)-1) & ~((__align)-1))

static int debug = 0;

static int memdump(shell_t* shell, const void* map, uint32_t addr, uint32_t len, uint32_t size)
{
    uint8_t *ptr_read, *ptr_disp;
    uint32_t start, end, line, byte;

    start = ROUND_DOWN(addr, DUMP_BYTE_PER_LINE);
    end   = ROUND_UP(addr + len, DUMP_BYTE_PER_LINE);
    len   = end - start;

    ptr_disp = (uint8_t*)addr;
    ptr_read = (uint8_t*)map;

    switch (size)
    {
        case 1:
            shell->puts("address    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789abcdef\r\n");
            shell->puts("=============================================================================\r\n");
            break;

        case 2:
            shell->puts("address      0    2    4    6    8    A    C    E    0123456789abcdef\r\n");
            shell->puts("=====================================================================\r\n");
            break;

        case 4:
            shell->puts("address          0        4        8        C    0123456789abcdef\r\n");
            shell->puts("=================================================================\r\n");
            break;

        default:
            break;
    }

    for (line = 0; line < len; line += DUMP_BYTE_PER_LINE)
    {
        shell_printf(shell, "%08X: ", ptr_disp);

        ptr_disp += line;
        ptr_read += line;

        for (byte = 0; byte < DUMP_BYTE_PER_LINE; byte += size)
        {
            switch (size)
            {
                case 1:
                    shell_printf(shell, "%02X ", *(__IO uint8_t*)(ptr_read + byte));
                    break;
                case 2:
                    shell_printf(shell, "%04X ", *(__IO uint16_t*)(ptr_read + byte));
                    break;
                case 4:
                    shell_printf(shell, "%08X ", *(__IO uint32_t*)(ptr_read + byte));
                    break;
                default:
                    break;
            }
        }

        shell->puts("   ");

        for (byte = 0; byte < DUMP_BYTE_PER_LINE; byte++)
        {
            char ch = *(__IO uint8_t*)(ptr_read + byte);

            switch (ch)
            {
                case 0x00:
                case 0xFF:
                    shell->puts(".");
                    break;

                case 0x01 ... 0x1F:  // 1~31
                case 0x7F ... 0xFE:  // 127~254
                    shell->puts(".");
                    break;

                case 0x20 ... 0x7E:  // 32~126
                    shell_printf(shell, "%c", ch);
                    break;

                default:
                    break;
            }
        }

        shell->puts("\r\n");
    }
}

static int hexdump(shell_t* shell, int argc, char** argv)
{
    enum {
        ARG_REQ_ADDR,
        ARG_OPT_SIZE,
        ARG_OPT_LEN,
        ARG_OPT_FORCE,
        ARG_OPT_HELP,
    };

    arg_req_t req_list[] = {
        {ARG_REQ_ADDR, ARGUMENT_REQUIRED_TYPE_INTEGER, "address", "memory address"},
        CMD_REQS_END,
    };

    arg_opt_t opt_list[] = {
        {ARG_OPT_SIZE, ARGUMENT_OPTIONAL_TYPE_INTEGER, "size",  's', "the size of unit, default is dword (1:byte, 2:word, 4:dword)"},
        {ARG_OPT_LEN,  ARGUMENT_OPTIONAL_TYPE_INTEGER, "len",   'l', "the length of region, default is 368 bytes"                  },
        {ARG_OPT_HELP, ARGUMENT_OPTIONAL_TYPE_BOOLEAN, "force", 'f', "force access (without address check)"                        },
        {ARG_OPT_HELP, ARGUMENT_OPTIONAL_TYPE_BOOLEAN, "help",  'h', "show help message and exit"                                  },
        CMD_OPTS_END,
    };

    cmd_ctx_t ctx;

    cmd_ctx_init(shell, &ctx, argc, argv, req_list, opt_list);

    //-------------------------------------------------------------------------

    uint32_t addr  = 0;
    uint8_t  size  = 4;
    uint32_t len   = 368;
    uint32_t val   = 0;
    bool     force = false;

    int opt;

    while ((opt = cmd_ctx_next(shell, &ctx)) != -1)
    {
        switch (opt)
        {
            case ARG_REQ_ADDR: {
                addr = ctx.curr.value.i32;
                break;
            }
            case ARG_OPT_SIZE: {
                size = ctx.curr.value.i32;
                switch (size)
                {
                    case 1:
                    case 2:
                    case 4: {
                        break;
                    }
                    default: {
                        shell_error(shell, "Invalid size\r\n");
                        return CMD_ERR_ARGUMENT_INVALID_VALUE;
                    }
                }
                break;
            }
            case ARG_OPT_LEN: {
                len = ctx.curr.value.i32;
                if (len == 0)
                {
                    shell_error(shell, "Invalid length\r\n");
                    return CMD_ERR_ARGUMENT_INVALID_VALUE;
                }
                break;
            }
            case ARG_OPT_FORCE: {
                force = true;
                break;
            }
            case ARG_OPT_HELP: {
                // print_help_string(shell, &ctx);
                return CMD_ERR_NO;
            }
            case CMD_ERR_ARGUMENT_UNKNOWN:
            case CMD_ERR_ARGUMENT_INVALID_TYPE:
            case CMD_ERR_ARGUMENT_INVALID_VALUE:
            case CMD_ERR_ARGUMENT_TOO_FEW:
            case CMD_ERR_ARGUMENT_TOO_MANY: {
                shell_error(shell, "%s\r\n", cmd_get_errstr(opt));
                return opt;
            }
            default: {
                break;
            }
        }
    }

    if (ctx.reqs.index < ctx.reqs.count)
    {
        opt = CMD_ERR_ARGUMENT_TOO_FEW;
        shell_error(shell, "%s\r\n", cmd_get_errstr(opt));
        return opt;
    }

    if (!force)
    {
        // chek address
    }

    //-------------------------------------------------------------------------

    memdump(shell, addr, addr, len, size);

    return CMD_ERR_NO;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), hexdump, hexdump, dump memory region);
