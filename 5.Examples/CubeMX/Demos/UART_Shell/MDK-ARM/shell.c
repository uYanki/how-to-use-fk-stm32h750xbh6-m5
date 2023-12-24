#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#define CONFIG_SHELL_HISTROY_MAX_COUNT   0
#define CONFIG_SHELL_PARAMETER_MAX_COUNT 20
#define CONFIG_SHELL_PRINTF_BUFSIZE      128
#define CONFIG_SHELL_SCANF_BUFSIZE       128
#define CONFIG_SHELL_CMD_HASH_SEED       0x4544222
#define CONFIG_SHELL_WHILE               0
#define CONFIG_SHELL_DEBUG_MODE_SW       1

typedef float  float32_t;
typedef double float64_t;

#define CMD_EXPORT_FUNC(name, func, desc)
#define CMD_EXPORT_VAR(name, var, desc)
#define CMD_EXPORT_KEY(key, func, desc)

#define CMD_KEY(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | ((d) << 0))

// CMD_KEY('\033', '[', 'A', 0)

CMD_EXPORT_KEY(0x1B5B4100, shellUp, up);
CMD_EXPORT_KEY(0x1B5B4200, shellDown, down);
CMD_EXPORT_KEY(0x1B5B4300, shellRight, right);
CMD_EXPORT_KEY(0x1B5B4400, shellLeft, left);
CMD_EXPORT_KEY(0x09000000, shellTab, tab);
CMD_EXPORT_KEY(0x08000000, shellBackspace, backspace);
CMD_EXPORT_KEY(0x7F000000, shellBackspace, backspace);
CMD_EXPORT_KEY(0x1B5B337E, shellDelete, delete);
CMD_EXPORT_KEY(0x0A000000, shellEnter, enter);  // SHELL_ENTER_LF
CMD_EXPORT_KEY(0x0D000000, shellEnter, enter);  // SHELL_ENTER_CR
CMD_EXPORT_KEY(0x0D0A0000, shellEnter, enter);  // SHELL_ENTER_CRLF
CMD_EXPORT_FUNC(cmds, shellCmds, "list all cmd");
CMD_EXPORT_FUNC(vars, shellVars, "list all var");
CMD_EXPORT_FUNC(keys, shellKeys, "list all key");
CMD_EXPORT_FUNC(clear, shellClear, "clear console");
CMD_EXPORT_FUNC(help, shellHelp, "show command info\nhelp[cmd]");
CMD_EXPORT_FUNC(exec, shellExecute, "execute function undefined");

void shell_key_proc(shell_t* shell, uint8_t data);
//

typedef enum {
    __cmd_type_func_start,
    CMD_TYPE_FUNC_C,
    CMD_TYPE_FUNC_MAIN,
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

typedef int (*cmd_cbk_t)();

typedef struct {
    uint32_t hash;

    union {
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
            // int (*cbk)(shell_t*);
        } key;
    };

    union {
        uint32_t all;
        struct {
            uint8_t    access : 3;
            cmd_type_t type   : 5;
            cmd_rw_t   rw     : 2;
        } bits;
    } attr;

} cmd_t;

typedef struct {
    char* prompt;

    struct {
        cmd_t*   base;
        uint16_t count;
    } cmds;

    struct {
        char*    buffer;
        uint16_t capacity;

        uint16_t length;
        uint16_t cursor;

        char  argc;
        char* argv[CONFIG_SHELL_PARAMETER_MAX_COUNT];

        uint8_t byteshift;  // key
    } parser;

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0
    struct {
        char*    buffer[CONFIG_SHELL_HISTROY_MAX_COUNT];
        uint16_t record;  // writer
        int16_t  offset;  // reader
        uint16_t count;
    } histroy;
#endif

    bool (*getc)(char*);
    void (*putc)(char);
    int (*puts)(char*);

} shell_t;

void            shell_printf(shell_t* shell, const char* fmt, ...);
static uint32_t generate_hash(const char* str);
void            cmd_show(shell_t* shell, cmd_t* cmd);
void            shell_get_var(shell_t* shell, cmd_t* cmd);
void            shell_set_var(shell_t* shell, cmd_t* cmd, void* value);
void            shell_show_prompt(shell_t* shell);
void            shell_list_cmds(shell_t* shell);
void            shell_list_var(shell_t* shell);
void            shell_list_all(shell_t* shell);
void            shell_init(shell_t* shell, char buffer[], uint16_t capacity);
void            shell_loop(shell_t* shell);
void            cmd_run(shell_t* shell, cmd_t* cmd);
void            shell_parse(shell_t* shell);
cmd_t*          shell_seek_cmd(shell_t* shell, const char* cmd, uint16_t len);
void            shell_exec(shell_t* shell);
void            console_clear_line(shell_t* shell);
void            shell_show_help();
void            console_delete_char(shell_t* shell);
void            console_delete_chars(shell_t* shell, uint16_t size);
void            console_clear_screen(shell_t* shell);
void            console_letf_arrow(shell_t* shell);
void            console_right_arrow(shell_t* shell);
void            console_tab(shell_t* shell);
void            console_backspace(shell_t* shell);
void            console_enter(shell_t* shell);
void            history_append(shell_t* shell);
void            history_swtich(shell_t* shell, char dir);
void            shell_remove_args_quotes(shell_t* shell);

static char shellExtParseChar(char* string)
{
    char* p     = string + 1;
    char  value = 0;

    if (*p == '\\')
    {
        switch (*(p + 1))
        {
            case 'b':
                value = '\b';
                break;
            case 'r':
                value = '\r';
                break;
            case 'n':
                value = '\n';
                break;
            case 't':
                value = '\t';
                break;
            case '0':
                value = 0;
                break;
            default:
                value = *(p + 1);
                break;
        }
    }
    else
    {
        value = *p;
    }
    return value;
}
static char* shellExtParseString(char* string)
{
    char*          p     = string;
    unsigned short index = 0;

    if (*string == '\"')
    {
        p = ++string;
    }

    while (*p)
    {
        if (*p == '\\')
        {
            *(string + index) = shellExtParseChar(p - 1);
            p++;
        }
        else if (*p == '\"')
        {
            *(string + index) = 0;
        }
        else
        {
            *(string + index) = *p;
        }
        p++;
        index++;
    }
    *(string + index) = 0;
    return string;
}

int32_t shell_ext_parse_value(shell_t* shell, char* argv)
{
    switch (*argv)
    {
        case '$':  // var
        {
            break;
        }

        case '-':
        case '+':
        case '0' ... '9':  // num
        {
            return atoi(argv);
        }

        default:
            return (int)shellExtParseString(argv);
    }
}

int shell_ext_call(shell_t* shell, cmd_t* cmd)
{
    int32_t args[CONFIG_SHELL_PARAMETER_MAX_COUNT] = {0};

    uint32_t argc = shell->parser.argc - 1;

    for (uint16_t i = 0; i < argc; i++)
    {
        args[i] = shell_ext_parse_value(shell, shell->parser.argv[i + 1]);
    }

    switch (argc)
    {
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 0
        case 0:
            return cmd->func.cbk();
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 1
        case 1:
            return cmd->func.cbk(args[0]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 2
        case 2:
            return cmd->func.cbk(args[0], args[1]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 3
        case 3:
            return cmd->func.cbk(args[0], args[1], args[2]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 4
        case 4:
            return cmd->func.cbk(args[0], args[1], args[2], args[3]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 5
        case 5:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 6
        case 6:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 7
        case 7:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5], args[6]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 8
        case 8:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5], args[6], args[7]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 9
        case 9:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5], args[6], args[7], args[8]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 10
        case 10:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5], args[6], args[7], args[8], args[9]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 11
        case 11:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5], args[6], args[7], args[8], args[9],
                                 args[10]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 12
        case 12:
            return cmd->func.cbk(args[0], args[1], args[2], args[3], args[4],
                                 args[5], args[6], args[7], args[8], args[9],
                                 args[10], args[11]);
#endif
        default:
            break;
    }

    return -1;
}

void shell_printf(shell_t* shell, const char* fmt, ...)
{
    static char buffer[CONFIG_SHELL_PRINTF_BUFSIZE];

    va_list vargs;

    va_start(vargs, fmt);
    uint16_t length = vsnprintf(buffer, sizeof(buffer) - 1, fmt, vargs);
    va_end(vargs);

    shell->puts(buffer);
}

static uint32_t generate_hash(const char* str)
{
    uint32_t seed = CONFIG_SHELL_CMD_HASH_SEED;
    uint32_t hash = 0;

    while (*str)
    {
        hash ^= seed;
        hash += *str++;
    }

    return hash;
}

void cmd_show(shell_t* shell, cmd_t* cmd)
{
    char *name, *desc, *type;

    switch (cmd->attr.bits.type)
    {
        case CMD_TYPE_FUNC_C:
            name = (char*)cmd->func.name;
            desc = (char*)cmd->func.desc;
            type = (char*)"CMD";
            break;
        case CMD_TYPE_VAR_STRING:
        case CMD_TYPE_VAR_INT8:
        case CMD_TYPE_VAR_INT16:
        case CMD_TYPE_VAR_INT32:
        case CMD_TYPE_VAR_INT64:
        case CMD_TYPE_VAR_UINT8:
        case CMD_TYPE_VAR_UINT16:
        case CMD_TYPE_VAR_UINT32:
        case CMD_TYPE_VAR_UINT64:
        case CMD_TYPE_VAR_FLOAT32:
        case CMD_TYPE_VAR_FLOAT64:
            name = (char*)cmd->var.name;
            desc = (char*)cmd->var.desc;
            type = (char*)"VAR";
            break;
    }

    int len = shell->puts(name);
    shell->putc('\t');
    shell->puts(type);
    shell->putc('\t');
    shell->puts(desc);
    shell->putc('\n');
}

void shell_show_var(shell_t* shell, cmd_t* cmd)
{
    if (cmd->attr.bits.rw == CMD_WO)
    {
        shell->puts("can't get write only var");
        shell->putc('\n');
    }
    else
    {
        shell->puts((char*)cmd->var.name);
        shell->puts(" = ");

        switch (cmd->attr.bits.type)
        {
            case CMD_TYPE_VAR_STRING: {
                shell->putc('\"');
                shell->puts((char*)cmd->var.addr);
                shell->putc('\"');
            }
            break;
            case CMD_TYPE_VAR_INT8: {
                int8_t v = *(int8_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_UINT8: {
                uint8_t v = *(uint8_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_INT16: {
                int16_t v = *(int16_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_UINT16: {
                uint16_t v = *(uint16_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_INT32: {
                int32_t v = *(int32_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_UINT32: {
                uint32_t v = *(uint32_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_INT64: {
                int64_t v = *(int64_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_UINT64: {
                uint64_t v = *(uint64_t*)(cmd->var.addr);
                shell_printf(shell, "%11d, 0x%X", v, v);
                break;
            }
            case CMD_TYPE_VAR_FLOAT32: {
                float32_t v = *(float32_t*)(cmd->var.addr);
                shell_printf(shell, "%.6f", v);
                break;
            }
            case CMD_TYPE_VAR_FLOAT64: {
                float64_t v = *(float64_t*)(cmd->var.addr);
                shell_printf(shell, "%.12f", v);
                break;
            }
            default: {
                shell->puts("unsupported type");
                break;
            }
        }

        shell->putc('\n');
    }
}

void shell_set_var(shell_t* shell, cmd_t* cmd, void* value)
{
    if (cmd->attr.bits.rw == CMD_RO)
    {
        shell->puts("can't set read only var");
        shell->putc('\n');
    }
    else
    {
        switch (cmd->attr.bits.type)
        {
            case CMD_TYPE_VAR_STRING:
                strcpy((char*)(cmd->var.addr), (char*)(value));
                break;
            case CMD_TYPE_VAR_INT8:
                *(int8_t*)(cmd->var.addr) = *(int8_t*)value;
                break;
            case CMD_TYPE_VAR_UINT8:
                *(uint8_t*)(cmd->var.addr) = *(uint8_t*)value;
                break;
            case CMD_TYPE_VAR_INT16:
                *(int16_t*)(cmd->var.addr) = *(int16_t*)value;
                break;
            case CMD_TYPE_VAR_UINT16:
                *(uint16_t*)(cmd->var.addr) = *(uint16_t*)value;
                break;
            case CMD_TYPE_VAR_INT32:
                *(uint32_t*)(cmd->var.addr) = *(int32_t*)value;
                break;
            case CMD_TYPE_VAR_UINT32:
                *(uint32_t*)(cmd->var.addr) = *(uint32_t*)value;
                break;
            case CMD_TYPE_VAR_INT64:
                *(int64_t*)(cmd->var.addr) = *(int64_t*)value;
                break;
            case CMD_TYPE_VAR_UINT64:
                *(uint64_t*)(cmd->var.addr) = *(uint64_t*)value;
                break;
            case CMD_TYPE_VAR_FLOAT32:
                *(float32_t*)(cmd->var.addr) = *(float32_t*)value;
                break;
            case CMD_TYPE_VAR_FLOAT64:
                *(float64_t*)(cmd->var.addr) = *(float64_t*)value;
                break;
            default:
                break;
        }
    }
}

void shell_show_prompt(shell_t* shell)
{
    shell->puts(shell->prompt);
}

void shell_list_cmds(shell_t* shell)
{
    shell->puts("Command List:");
    shell->putc('\n');

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        if (base->attr.bits.type == CMD_TYPE_FUNC_C ||
            base->attr.bits.type == CMD_TYPE_FUNC_MAIN)
        {
            cmd_show(shell, &base[i]);
        }
    }
}

void shell_list_var(shell_t* shell)
{
    shell->puts("Variable List:");
    shell->putc('\n');

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        switch (base->attr.bits.type)
        {
            case CMD_TYPE_VAR_STRING:
            case CMD_TYPE_VAR_INT8:
            case CMD_TYPE_VAR_INT16:
            case CMD_TYPE_VAR_INT32:
            case CMD_TYPE_VAR_INT64:
            case CMD_TYPE_VAR_UINT8:
            case CMD_TYPE_VAR_UINT16:
            case CMD_TYPE_VAR_UINT32:
            case CMD_TYPE_VAR_UINT64:
            case CMD_TYPE_VAR_FLOAT32:
            case CMD_TYPE_VAR_FLOAT64:
                cmd_show(shell, &base[i]);
                break;

            default:
                break;
        }
    }
}

void shell_list_all(shell_t* shell)
{
    shell_list_cmds(shell);
    shell_list_var(shell);
}

void shell_init(shell_t* shell, char buffer[], uint16_t capacity)
{
    shell->parser.buffer   = buffer;
    shell->parser.capacity = capacity;
    shell->parser.length   = 0;
    shell->parser.cursor   = 0;

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const uint32_t CMDS$$Base;
    extern const uint32_t CMDS$$Limit;
    shell->cmds.base  = (cmd_t*)(&CMDS$$Base);
    shell->cmds.count = (&CMDS$$Limit - &CMDS$$Base) / sizeof(cmd_t);
#elif defined(__ICCARM__) || defined(__ICCRX__)
    // shell->cmds.base  = (cmd_t*)(__section_begin("CMDS"));
    // shell->cmds.count = ((uint32_t)(__section_end("CMDS")) - (uint32_t)(__section_begin("CMDS"))) / sizeof(cmd_t);
#elif defined(__GNUC__)
    // shell->cmds.base  = (cmd_t*)(&_shell_command_start);
    // shell->cmds.count = ((uint32_t)(&_shell_command_end) - (uint32_t)(&_shell_command_start)) / sizeof(cmd_t);
#else
#error not supported compiler, please use command table mode
#endif

#if SHELL_HISTORY_MAX_NUMBER > 0

#endif

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        base[i].hash = generate_hash(base[i].func.desc);
    }

    shell->putc('\n');
    shell_show_prompt(shell);
}

void shell_loop(shell_t* shell)
{
    char ch;

    if (shell->getc(ch))
    {
        if (shell_key_proc(shell, ch) == false)
        {
            shell_insert_byte(shell, ch);
        }
    }
}

void shell_parse(shell_t* shell)
{
    uint16_t i;

    shell->parser.argc = 0;

    bool quotes = false;  // macth string
    bool record = true;

    for (char* p = shell->parser.buffer; *p != '\0'; ++p)
    {
        if (*p == ' ')
        {
            // not string
            if (quotes == false)
            {
                *p = '\0';

                // new arg
                record = true;
            }

            continue;
        }
        else if (*p == '\"')
        {
            quotes = !quotes;
        }

        if (record == true)
        {
            shell->parser.argv[shell->parser.argc++] = p;

            if (shell->parser.argc == CONFIG_SHELL_PARAMETER_MAX_COUNT)
            {
                break;
            }

            record = false;
        }
    }

    for (i = shell->parser.argc; i < CONFIG_SHELL_PARAMETER_MAX_COUNT; i++)
    {
        shell->parser.argv[i] = NULL;
    }
}

cmd_t* shell_seek_cmd(shell_t* shell, const char* cmd, uint16_t len)
{
    cmd_t* base = (cmd_t*)shell->cmds.base;

    if (len > 0)
    {
        for (uint16_t i = 0; i < shell->cmds.count; i++)
        {
            if (strncmp(cmd, base[i].func.name, len) == 0)
            {
                return &base[i];
            }
        }
    }
    else
    {
        uint32_t hash = generate_hash(cmd);

        for (uint16_t i = 0; i < shell->cmds.count; i++)
        {
            // if (hash == base[i].hash)
            {
                if (strcmp(cmd, base[i].func.name) == 0)
                {
                    return &base[i];
                }
            }
        }
    }

    return NULL;
}

void cmd_exec(shell_t* shell, cmd_t* cmd)
{
    if (cmd->attr.bits.type == CMD_TYPE_FUNC_C)
    {
        int ret = shell_ext_call(shell, cmd);
    }
    else if (cmd->attr.bits.type == CMD_TYPE_FUNC_MAIN)
    {
        shell_remove_args_quotes(shell);
        int ret = cmd->func.cbk(shell->parser.argc, shell->parser.argv);
    }
    else if (__cmd_type_var_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_var_end)
    {
        shell_show_var(shell, cmd);
    }
}

void shell_remove_args_quotes(shell_t* shell)
{
    for (uint16_t i = 0; i < shell->parser.argc; ++i)
    {
        char* p = shell->parser.argv[i];

        if (*p == '\"')
        {
            *p++ = '\0';

            shell->parser.argv[i] = p;
        }

        uint16_t len = strlen(p);

        if (p[len - 1] == '\"')
        {
            p[len - 1] = '\0';
        }
    }
}

void disp_args(int argc, char* argv[])
{
    for (uint16_t i = 0; i < argc; i++)
    {
        printf("%2d. %s\n", i, argv[i]);
    }
}

void shell_key_proc(shell_t* shell, uint8_t data)
{
    cmd_t* base = shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; ++i)
    {
        if (base[i].attr.bits.type == CMD_TYPE_KEY)
        {
            uint8_t cur_key = ((base[i].key.value >> shell->parser.byteshift) & 0xFF);

            if (cur_key != data)
            {
                return false;
            }

            bool matched = false;

            if (shell->parser.byteshift > 0)
            {
                shell->parser.byteshift -= 8;

                uint8_t next_key = (base[i].key.value >> (shell->parser.byteshift)) & 0xFF;

                if (next_key == 0x00)
                {
                    // no next key
                    matched = true;

                    shell->parser.byteshift = 24;
                }
            }
            else
            {
                // current key is last
                matched = true;
            }

            if (matched)
            {
                base[i].key.cbk(shell);
            }

            return true;
        }
    }

    return false;
}

void shell_exec(shell_t* shell)
{
    if (shell->parser.length == 0)
    {
        return;
    }

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0
    history_append(shell);
#endif

    shell_parse(shell);

    shell->parser.length = shell->parser.cursor = 0;

#if CONFIG_SHELL_DEBUG_MODE_SW

    // display argv

    shell->putc('\n');
    shell->puts(shell->parser.buffer);
    shell->putc('\n');

    for (uint16_t i = 0; i < shell->parser.argc; i++)
    {
        shell_printf(shell, "%2d. %s\n", i, shell->parser.argv[i]);
    }

    shell->putc('\n');

#endif

    if (shell->parser.argc == 0)
    {
        // all chars are whitespace
        return;
    }

    cmd_t* cmd = shell_seek_cmd(shell, (const char*)(shell->parser.argv[0]), 0);

    if (cmd == NULL)
    {
        shell->puts("Command not Found");
        shell->putc('\n');
    }
    else
    {
        cmd_exec(shell, cmd);
    }
}

void console_clear_line(shell_t* shell)
{
    uint16_t n = shell->parser.length - shell->parser.cursor;

    while (n--)
    {
        shell->putc(' ');
    }

    console_delete_chars(shell, shell->parser.length);
}

void shell_show_help() {}

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

void history_append(shell_t* shell)
{
    shell->histroy.offset = 0;

    if (shell->histroy.count > 0)
    {
        if (strcmp(shell->histroy.buffer[(CONFIG_SHELL_HISTROY_MAX_COUNT + shell->histroy.record - 1) % CONFIG_SHELL_HISTROY_MAX_COUNT],
                   shell->parser.buffer) == 0)
        {
            // same as the last record
            return;
        }
    }

    if (*shell->parser.buffer != '\0')
    {
        strcpy(shell->histroy.buffer[shell->histroy.record++], shell->parser.buffer);

        if (shell->histroy.count < CONFIG_SHELL_HISTROY_MAX_COUNT)
        {
            shell->histroy.count++;
        }

        if (shell->histroy.record == CONFIG_SHELL_HISTROY_MAX_COUNT)
        {
            shell->histroy.record = 0;
        }
    }
}

void history_swtich(shell_t* shell, char dir)
{
    if (dir < 0)  // prev
    {
        if (shell->histroy.offset == -shell->histroy.count)
        {
            return;
        }

        shell->histroy.offset--;
    }
    else if (dir > 0)  // next
    {
        if (shell->histroy.offset == 0)
        {
            return;
        }

        shell->histroy.offset++;
    }
    else
    {
        return;
    }

    console_clear_line(shell);

    if (shell->histroy.offset == 0)
    {
        shell->parser.cursor = shell->parser.length = 0;
    }
    else
    {
        strcpy(shell->parser.buffer, shell->histroy.buffer[(CONFIG_SHELL_HISTROY_MAX_COUNT + shell->histroy.record + shell->histroy.offset) % CONFIG_SHELL_HISTROY_MAX_COUNT]);
        shell->parser.cursor = shell->parser.length = strlen(shell->parser.buffer);
        shell->puts(shell->parser.buffer);
    }
}

#endif

#if CONFIG_SHELL_SCANF_BUFSIZE > 0

void shell_scanf(shell_t* shell, char* fmt, ...)
{
    char buffer[CONFIG_SHELL_SCANF_BUFSIZE];

    va_list  vargs;
    uint16_t index = 0;

    while (index < sizeof(buffer))
    {
        if (shell->getc(&buffer[index]))
        {
            shell->putc(buffer[index]);

            if (buffer[index] == '\r' ||
                buffer[index] == '\n')
            {
                break;
            }

            index++;
        }
    }

    if (index == sizeof(buffer))
    {
        index--;
    }

    buffer[index] = '\0';

    va_start(vargs, fmt);
    vsscanf(buffer, fmt, vargs);
    va_end(vargs);
}

#endif

void console_delete_char(shell_t* shell)
{
    shell->puts("\b \b");
}

void console_delete_chars(shell_t* shell, uint16_t size)
{
    while (size--)
    {
        console_delete_char(shell);
    }
}

void console_clear_screen(shell_t* shell)  // clear console
{
    shell->puts("\e[2J\e[1H");
}

void console_letf_arrow(shell_t* shell)
{
    if (shell->parser.cursor > 0)
    {
        shell->parser.cursor--;
        shell->putc('\b');
    }
}

void console_right_arrow(shell_t* shell)
{
    if (shell->parser.cursor < shell->parser.length)
    {
        shell->parser.cursor++;
        shell->putc(shell->parser.buffer[shell->parser.cursor]);
    }
}

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

void console_up_arrow(shell_t* shell)
{
    history_swtich(shell, -1);
}

void console_down_arrow(shell_t* shell)
{
    history_swtich(shell, 1);
}

#endif

void shell_insert_byte(shell_t* shell, uint8_t data)
{
    if (shell->parser.length >= (shell->parser.capacity - 1))
    {
        shell->puts("Warning: Command is too long");
        shell->putc('\n');
        shell_show_prompt(shell);
        shell->puts(shell->parser.buffer);
        return;
    }

    if (shell->parser.cursor == shell->parser.length)
    {
        // clang-format off
        shell->parser.buffer[shell->parser.cursor++] = data;
        shell->parser.length = shell->parser.cursor;
        shell->parser.buffer[shell->parser.length] = '\0';
        // clang-format on
        shell->putc(data);
    }
    else if (shell->parser.cursor < shell->parser.length)
    {
        // insert mode

        uint16_t i;

        for (i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            // move back
            shell->parser.buffer[shell->parser.cursor + i] =
                shell->parser.buffer[shell->parser.cursor + i - 1];
        }

        shell->parser.buffer[shell->parser.cursor++] = data;
        shell->parser.buffer[++shell->parser.length] = 0;

        for (i = shell->parser.cursor - 1; i < shell->parser.length; i++)
        {
            shell->putc(shell->parser.buffer[i]);
        }
        for (i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            shell->putc('\b');
        }

        // replace mode...
    }
}

/**
 * @param dir 1: before cursor, -1: after cursor
 */
void shell_remove_byte(shell_t* shell, int8_t dir)
{
    if ((shell->parser.cursor == 0 && dir > 0) ||                   // forward
        (shell->parser.cursor == shell->parser.length && dir < 0))  // backward
    {
        return;
    }

    if (shell->parser.cursor == shell->parser.length && dir > 0)
    {
        shell->parser.buffer[shell->parser.length = --shell->parser.cursor] = 0;
        console_delete_char(shell);
    }
    else
    {
        uint16_t i;

        uint8_t offset = (dir == -1) ? 1 : 0;

        for (i = offset; i < shell->parser.length - shell->parser.cursor; i++)
        {
            // move forward
            shell->parser.buffer[shell->parser.cursor + i - 1] =
                shell->parser.buffer[shell->parser.cursor + i];
        }

        shell->parser.length--;

        if (!offset)
        {
            shell->parser.cursor--;
            shell->putc('\b');
        }

        shell->parser.buffer[shell->parser.length] = 0;

        for (i = shell->parser.cursor; i < shell->parser.length; i++)
        {
            shell->putc(shell->parser.buffer[i]);
        }

        shell->putc(' ');

        for (i = shell->parser.length - shell->parser.cursor + 1; i > 0; i--)
        {
            shell->putc('\b');
        }
    }
}

void console_tab(shell_t* shell) {}

void console_backspace(shell_t* shell) {}

void console_enter(shell_t* shell)
{
    shell_exec(shell);
    shell_show_prompt(shell);
}

void setv()
{
    char a = 'D';

    // 458965760 = 0x1B5B4300
    // 0x1B000000 = \e << 24 = 27 << 24
    // 0x005B0000 =  [ << 16 = 91 << 16
    // 0x00004100 =  A <<  8 = 65 <<  8
    // 0x00004200 =  B <<  8 = 66 <<  8
    // 0x00004300 =  C <<  8 = 67 <<  8
    // 0x00004400 =  D <<  8 = 68 <<  8
}

void getv() {}

// 执行未定义函数
void call() {}  // execute function undefined
void list() {}  // list all cmd

// export cmds, list all cmd
// export vars, list all var

void ret() {}  // return value of last command

//---------------------------------------------------

#if 1

int shell_puts(char* str)
{
    return printf(str);
}

bool shell_getc(char* c)
{
    *c = getchar();
    // return false when timeout
    return true;
}

void shell_putc(char c)
{
    putchar(c);
}

uint16_t tstVar1;
uint32_t tstVar2;
uint32_t tstVar3;

int tstFun1(int argc, char* argv[])
{
    printf("-> fun1\n");
    disp_args(argc, argv);
    return 0;
}

int tstFun2(int a, int b, char* str)
{
    printf("-> fun2: %d, %d, %s\n", a, b, str);
    // printf("-> fun2: %d, %d\n", a, b);
    return 0;
}

cmd_t cmds[10] = {
    {

     .func = {
            .name = "fun1",
            .desc = "fun1 desc",
            .cbk  = (int (*)()) & tstFun1,
        },
     .attr.bits = {
            .type = CMD_TYPE_FUNC_MAIN,
            .rw   = CMD_RW,
        },
     },
    {
     .func = {
            .name = "fun2",
            .desc = "fun2 desc",
            .cbk  = (int (*)()) & tstFun2,
        },
     .attr.bits = {
            .type = CMD_TYPE_FUNC_C,
            .rw   = CMD_RW,
        },
     },
    {
     .var = {
            .name = "var1",
            .desc = "var1 desc",
            .addr = &tstVar1,
        },
     .attr.bits = {
            .type = CMD_TYPE_FUNC_C,
            .rw   = CMD_RW,
        },
     },
    {
     .var = {
            .name = "var2",
            .desc = "var2 desc",
            .addr = &tstVar2,
        },
     .attr.bits = {
            .type = CMD_TYPE_FUNC_C,
            .rw   = CMD_RO,
        },
     },
    {
     .var = {
            .name = "var3",
            .desc = "var3 desc",
            .addr = &tstVar3,
        },
     .attr.bits = {
            .type = CMD_TYPE_FUNC_C,
            .rw   = CMD_WO,
        },
     }
};

void shell_run(shell_t* shell, const char* cmd)
{
    strcpy(shell->parser.buffer, cmd);
    shell->parser.cursor = shell->parser.length = strlen(shell->parser.buffer);
    shell_exec(shell);
}

int main()
{
    shell_t shell = {
        .prompt = "uYanki@root: ",
        .cmds   = {
                   .base  = cmds,
                   .count = 5,
                   },
        .getc = shell_getc,
        .putc = shell_putc,
        .puts = shell_puts,
    };

    char linebuf[128];

    shell_init(&shell, linebuf, 128);

    // shell_run(&shell, "setv var1 1212 \"1222\"");
    // shell_run(&shell, "getv var1");

    shell_run(&shell, "fun2 1 22 \"2122432\"");
    // shell_run(&shell, "fun2");

    while (CONFIG_SHELL_WHILE)
    {
        shell_loop(&shell);
    }

    return 0;
}

#endif
