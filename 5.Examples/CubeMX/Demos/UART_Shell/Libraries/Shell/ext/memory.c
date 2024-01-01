#include "../core.h"
#include "../getopt.h"

// hexdump

// int aos_log_hexdump(const char* tag, char* buffer, int len)
// {
//     int i;

//     if (aos_ulog_init == false)
//     {
//         return -1;
//     }

//     aos_cust_output_func("[%s]\n", tag);
//     aos_cust_output_func("0x0000: ");
//     for (i = 0; i < len; i++)
//     {
//         aos_cust_output_func("0x%02x ", buffer[i]);

//         if (i % 8 == 7)
//         {
//             aos_cust_output_func("\n");
//             aos_cust_output_func("0x%04x: ", i + 1);
//         }
//     }

//     aos_cust_output_func("\n");
//     return 0;
// }

static void print_help_string(getopt_context_t* ctx)
{
    char buffer[2048];
    printf("%s\n", getopt_create_help_string(ctx, buffer, sizeof(buffer)));
}

#define DEFAULT_DUMP_LEN             368
#define DEFAULT_DUMP_SZ              4
#define DUMP_BYTE_PER_LINE           16

#define ROUND_DOWN(__value, __align) ((__value) & ~((__align)-1))
#define ROUND_UP(__value, __align)   (((__value) + (__align)-1) & ~((__align)-1))
#define LOGV(f, args...)             fprintf(stdout, f, ##args)
#define LOGD(f, args...)                       \
    do {                                       \
        if (debug) fprintf(stderr, f, ##args); \
    } while (0)
#define LOGE(f, args...)   fprintf(stderr, "Error: " f, ##args)
#define OUTPUT(f, args...) fprintf(stdout, f, ##args)

static int debug;

static const char* optstr = "a:s:l:w:dh";

static void usage(void)
{
    LOGV("Usage: hexdumpmem [options] -a|--address <memory address (hex) to dump>\n");
    LOGV("   or: hexdumpmem -w <fill pattern (hex)> [options] -a|--address <memory address (hex) to fill>\n");
    LOGV("Options:\n");
    LOGV("  -s, --size <size>         Select the size of dump/fill unit\n");
    LOGV("  -l, --len <length>        Select the length of the dump/fill region\n");
    LOGV("  -d, --debug               Enable the debug log\n");
    LOGV("  -h, --help                Show help message and exit\n");
}

static uint8_t readb(const void* addr)
{
    return *(volatile uint8_t*)addr;
}

static void writeb(uint8_t val, void* addr)
{
    *(volatile uint8_t*)addr = val;
}

static uint16_t readw(const void* addr)
{
    return *(volatile uint16_t*)addr;
}

static void writew(uint16_t val, void* addr)
{
    *(volatile uint16_t*)addr = val;
}

static uint32_t readl(const void* addr)
{
    return *(volatile uint32_t*)addr;
}

static void writel(uint32_t val, void* addr)
{
    *(volatile uint32_t*)addr = val;
}

static int memdump(const void* map, uint32_t addr, uint32_t len, uint32_t size)
{
    uint8_t* ptr = (uint8_t*)map;
    uint32_t start, end, line, byte;

    // if (!map)
    // {
    //     LOGE("Invalid map pointer\n");
    //     return -1;
    // }
    if (!len)
    {
        LOGE("Invalid length\n");
        return -1;
    }
    if (size != 1 && size != 2 && size != 4)
    {
        LOGE("Invalid size\n");
        return -1;
    }

    start = ROUND_DOWN(addr, DUMP_BYTE_PER_LINE);
    end   = ROUND_UP(addr + len, DUMP_BYTE_PER_LINE);
    LOGD("Dump memory 0x%08X ~ 0x%08X\n", start, end);

    LOGD("Before adjust, ptr = %p\n", ptr);
    ptr += start - ROUND_DOWN(addr, 8);
    LOGD("After adjust, ptr = %p\n", ptr);

    switch (size)
    {
        case 1:
            OUTPUT("%-8s   %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X\n",
                   "addr", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
            OUTPUT("==========================================================\n");
            OUTPUT("");
            break;

        case 2:
            OUTPUT("%-8s   %4X %4X %4X %4X %4X %4X %4X %4X\n",
                   "addr", 0, 2, 4, 6, 8, 10, 12, 14);
            OUTPUT("==================================================\n");
            break;

        case 4:
            OUTPUT("%-8s   %8X %8X %8X %8X\n", "addr", 0, 4, 8, 12);
            OUTPUT("==============================================\n");
            break;

        default:
            break;
    }

    for (line = 0; line < (end - start); line += DUMP_BYTE_PER_LINE)
    {
        OUTPUT("%08Xh: ", start + line);
        for (byte = 0; byte < DUMP_BYTE_PER_LINE; byte += size)
        {
            switch (size)
            {
                case 1:
                    OUTPUT("%02X ", readb(ptr + line + byte));
                    break;

                case 2:
                    OUTPUT("%04X ", readw(ptr + line + byte));
                    break;

                case 4:
                    OUTPUT("%08X ", readl(ptr + line + byte));
                    break;
                default:
                    break;
            }
        }
        OUTPUT("\n");
    }

    return 0;
}

static int memfill(void* map, uint32_t addr, uint32_t len, uint32_t size, uint32_t val)
{
    uint8_t* ptr = (uint8_t*)map;
    uint32_t byte;

    if (!map)
    {
        LOGE("Invalid map pointer\n");
        return -1;
    }
    if (!len)
    {
        LOGE("Invalid length\n");
        return -1;
    }
    if (size != 1 && size != 2 && size != 4)
    {
        LOGE("Invalid size\n");
        return -1;
    }
    if (addr & (size - 1))
    {
        LOGE("address and size are not aligned\n");
        return -1;
    }

    LOGD("Before adjust, map = %p\n", map);
    ptr += addr - ROUND_DOWN(addr, 8);
    LOGD("After adjust, map = %p\n", map);

    for (byte = 0; byte < len; byte += size)
    {
        switch (size)
        {
            case 1:
                LOGD("Write 0x%02X to *(%p)\n",
                     (uint8_t)val, ptr + byte);
                writeb(val, ptr + byte);
                break;

            case 2:
                LOGD("Write 0x%02X to *(%p)\n",
                     (uint16_t)val, ptr + byte);
                writew(val, ptr + byte);
                break;

            case 4:
                LOGD("Write 0x%02X to *(%p)\n",
                     (uint32_t)val, ptr + byte);
                writel(val, ptr + byte);
                break;

            default:
                break;
        }
    }

    return 0;
}

int cmd_memfill(shell_t* shell, int argc, char** argv)
{
    static const getopt_option_t memfill_opts[] = {
        {"address", 'a', GETOPT_OPTION_TYPE_REQUIRED_INT32, NULL, 0x0, "memory address to fill",        NULL                     },
        {"size",    's', GETOPT_OPTION_TYPE_REQUIRED_INT32, NULL, 0x0, "the size of fill unit",         "1:byte, 2:word, 4:dword"},
        {"len",     'l', GETOPT_OPTION_TYPE_REQUIRED_INT32, NULL, 0x0, "the length of the fill region", NULL                     },
        {"write",   'w', GETOPT_OPTION_TYPE_REQUIRED_INT32, NULL, 0x0, "show help message and exit",    NULL                     },
        {"debug",   'd', GETOPT_OPTION_TYPE_OPTIONAL,       NULL, 0x0, "enable the debug log",          NULL                     },
        {"help",    'h', GETOPT_OPTION_TYPE_NO_ARG,         NULL, 0x0, "show help message and exit",    NULL                     },
        GETOPT_OPTIONS_END
    };
}

static int cmd_memdump22(shell_t* shell, int argc, char** argv)
{
    static const getopt_option_t opts_list[] = {
        {"address", 'a', GETOPT_OPTION_TYPE_REQUIRED_INT32, NULL, 0x0, "memory address to dump",        NULL                               },
        {"size",    's', GETOPT_OPTION_TYPE_OPTIONAL_INT32, NULL, 0x0, "the size of dump unit",         "1:byte (default), 2:word, 4:dword"},
        {"len",     'l', GETOPT_OPTION_TYPE_OPTIONAL_INT32, NULL, 0x0, "the length of the dump region", NULL                               },
        {"debug",   'd', GETOPT_OPTION_TYPE_NO_ARG,         NULL, 0x0, "enable the debug log",          NULL                               },
        {"help",    'h', GETOPT_OPTION_TYPE_NO_ARG,         NULL, 0x0, "show help message and exit",    NULL                               },
        GETOPT_OPTIONS_END
    };

    getopt_context_t ctx;

    if (getopt_create_context(&ctx, argc, argv, opts_list) < 0)
    {
        printf("error while creating getopt ctx, bad options-list?");
        return 1;
    }

    int opt;

    uint32_t addr = 0;
    uint32_t len = DEFAULT_DUMP_LEN, size = DEFAULT_DUMP_SZ;

    while ((opt = getopt_next(&ctx)) != -1)
    {
        switch (opt)
        {
            case '+':
                printf("got argument without flag: %s\n", ctx.current_opt_arg);
                break;
            case '?':
                printf("unknown flag: %s\n", ctx.current_opt_arg);
                break;
            case '!':
                printf("invalid use of flag: %s\n", ctx.current_opt_arg);
                break;
            case 'a':
                addr = ctx.current_value.i32;
                break;
            case 's':
                size = ctx.current_value.i32;
                break;
            case 'l':
                len = ctx.current_value.i32;
                break;
            case 'd':  //
                break;
            case 'h':
                // print_help_string(&ctx);
                char buffer[2048];
                getopt_create_help_string(&ctx, buffer, sizeof(buffer));
                shell->puts(buffer);
                shell->puts("\r\n");
                return 0;
            default:
                break;
        }
    }

    int loptidx, c, fd, err = 0;

    uint32_t wr = 0, val;
    void*    map;

    // LOGV("Memory region 0x%08x ~ 0x%08x\n", addr, addr + len - 1);
    // LOGD("addr = 0x%08x, len = 0x%08x, size = %d\n", addr, len, size);
    // LOGD("wr = %d, val = 0x%08x\n", wr, val);

    // LOGD("mmap 0x%08x ~ 0x%08x\n",
    //      ROUND_DOWN(addr, 8),
    //      ROUND_DOWN(addr, 8) + ROUND_UP(len, 8));
    // map = mmap(NULL, ROUND_UP(len, 8), ROUND_DOWN(addr, 8));

    // err = memdump(map, addr, len, size);
    // err = memfill(map, addr, len, size, val);

    // munmap(map, ROUND_UP(len, 8));

exit:
    return err;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), memdump22, cmd_memdump22, show memory region);
