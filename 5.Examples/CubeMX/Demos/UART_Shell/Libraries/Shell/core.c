#include "./core.h"
#include "./utils.h"

//-----------------------------------------------------------------------------

#define REG8S(ADDR)            (*(int8_t*)(ADDR))
#define REG8U(ADDR)            (*(uint8_t*)(ADDR))
#define REG16S(ADDR)           (*(int16_t*)(ADDR))
#define REG16U(ADDR)           (*(uint16_t*)(ADDR))
#define REG32S(ADDR)           (*(int32_t*)(ADDR))
#define REG32U(ADDR)           (*(uint32_t*)(ADDR))
#define REG32F(ADDR)           (*(float32_t*)(ADDR))
#define REG64F(ADDR)           (*(float64_t*)(ADDR))

#define INCLOSE(min, val, max) (((min) <= (val)) && ((val) <= (max)))

//-----------------------------------------------------------------------------

typedef enum {
    DIR_PREV,
    DIR_NEXT,
} dir_e;

typedef enum {
    DEL_PREV,  // 光标前
    DEL_CURR,  // 光标处
} del_e;

typedef enum {
    CONV_STATE_NONE,  // 没有转换
    CONV_STATE_HALT,  // 转换部分
    CONV_STATE_FULL,  // 转换全部
} conv_state_e;

typedef enum {
    SIGN_POS,
    SIGN_NEG,
} sign_e;

typedef enum {
    TYPE_NEG_INT,
    TYPE_POS_INT,
    TYPE_FLOAT,
    TYPE_STRING,
} arg_type_e;

//-----------------------------------------------------------------------------

static bool shell_key_scan(shell_t* shell, uint8_t data);

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0
void history_append(shell_t* shell);
void history_swtich(shell_t* shell, dir_e dir);
#endif

static void shell_insert_byte(shell_t* shell, uint8_t data);
static void shell_remove_byte(shell_t* shell, del_e dir);

static bool shell_parse_cmd(shell_t* shell);
arg_type_e  shell_parse_arg(shell_t* shell, char* str, uint32_t* ret);
static void shell_exec(shell_t* shell);

static cmd_t* shell_seek_cmd(shell_t* shell, const char* str);

static int  shell_call_func(shell_t* shell, cmd_t* cmd);
static void shell_show_var(shell_t* shell, cmd_t* cmd);
static void shell_set_var(shell_t* shell, char* name, void* value);

static void shell_show_help(shell_t* shell, int argc, char* argv[]);
static void shell_show_logo(shell_t* shell);
static void shell_show_prompt(shell_t* shell);

static void console_delete_char(shell_t* shell);
static void console_delete_chars(shell_t* shell, uint16_t size);
static void console_clear_screen(shell_t* shell);
static void console_clear_line(shell_t* shell);

static char* shell_get_cmd_name(shell_t* shell, cmd_t* cmd);
static char* shell_get_cmd_desc(shell_t* shell, cmd_t* cmd);

//-----------------------------------------------------------------------------

static int shell_call_func(shell_t* shell, cmd_t* cmd)
{
    if (cmd->attr.bits.type == CMD_TYPE_FUNC_MAIN)
    {
        int ret = cmd->func.cbk(shell, shell->parser.argc, shell->parser.argv);
    }
    else if (cmd->attr.bits.type == CMD_TYPE_FUNC_C)
    {
        uint8_t argc = cmd->attr.bits.ext.argc;

        if (argc != (shell->parser.argc - 1))
        {
            shell_printf(shell, "Too few or too many arguments. Only %d parameters are needed.\r\n", argc);
            goto _error;
        }

        int32_t args[CONFIG_SHELL_PARAMETER_MAX_COUNT] = {0};

        for (uint8_t i = 0; i < argc; i++)
        {
            shell_parse_arg(shell, shell->parser.argv[i + 1], &args[i]);
        }

        switch (argc)
        {
            case 0:
                return cmd->func.cbk(shell);
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 0
            case 1:
                return cmd->func.cbk(shell,
                                     args[0]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 1
            case 2:
                return cmd->func.cbk(shell,
                                     args[0], args[1]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 2
            case 3:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 3
            case 4:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 4
            case 5:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 5
            case 6:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 6
            case 7:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5], args[6]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 7
            case 8:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5], args[6], args[7]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 8
            case 9:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5], args[6], args[7], args[8]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 9
            case 10:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5], args[6], args[7], args[8], args[9]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 10
            case 11:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5], args[6], args[7], args[8], args[9],
                                     args[10]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 11
            case 12:
                return cmd->func.cbk(shell,
                                     args[0], args[1], args[2], args[3], args[4],
                                     args[5], args[6], args[7], args[8], args[9],
                                     args[10], args[11]);
#endif
#if CONFIG_SHELL_PARAMETER_MAX_COUNT > 12
#Warning "too many parameters"
#endif
            default:
                break;
        }
    }
    else if (cmd->attr.bits.type == CMD_TYPE_KEY)
    {
        cmd->key.cbk(shell);
        return 0;
    }

_error:
    return -1;
}

void shell_log(shell_t* shell, char* buffer)
{
    shell->puts("\033[2K\r");
    shell->puts(buffer);
    shell_show_prompt(shell);

    if (shell->parser.length > 0)
    {
        shell_printf(shell, shell->parser.buffer);

        for (uint16_t i = shell->parser.cursor; i < shell->parser.length; i++)
        {
            shell->puts("\b");
        }
    }
}

static void shell_show_var(shell_t* shell, cmd_t* cmd)
{
    if (cmd->attr.bits.ext.rw != CMD_WO)
    {
        shell_printf(shell, "%s = ", cmd->var.name);

        void* addr = cmd->var.addr;

        switch (cmd->attr.bits.type)
        {
            case CMD_TYPE_VAR_STRING: {
                shell_printf(shell, "\"%s\"", (char*)cmd->var.addr);
                break;
            }
            case CMD_TYPE_VAR_INT8: {
                shell_printf(shell, "%d, 0x%X", REG8S(addr), REG8S(addr));
                break;
            }
            case CMD_TYPE_VAR_UINT8: {
                shell_printf(shell, "%d, 0x%X", REG8U(addr), REG8U(addr));
                break;
            }
            case CMD_TYPE_VAR_INT16: {
                shell_printf(shell, "%d, 0x%X", REG16S(addr), REG16S(addr));
                break;
            }
            case CMD_TYPE_VAR_UINT16: {
                shell_printf(shell, "%d, 0x%X", REG16U(addr), REG16U(addr));
                break;
            }
            case CMD_TYPE_VAR_INT32: {
                shell_printf(shell, "%d, 0x%X", REG32S(addr), REG32S(addr));
                break;
            }
            case CMD_TYPE_VAR_UINT32: {
                shell_printf(shell, "%d, 0x%X", REG32U(addr), REG32U(addr));
                break;
            }
            case CMD_TYPE_VAR_INT64: {
                shell_printf(shell, "%d, 0x%X", REG64S(addr), REG64S(addr));
                break;
            }
            case CMD_TYPE_VAR_UINT64: {
                shell_printf(shell, "%d, 0x%X", REG64U(addr), REG64U(addr));
                break;
            }
            case CMD_TYPE_VAR_FLOAT32: {
                shell_printf(shell, "%f", REG32F(addr));
                break;
            }
            case CMD_TYPE_VAR_FLOAT64: {
                shell_printf(shell, "%lf", REG64F(addr));
                break;
            }
            default: {
                shell->puts("unsupported type");
                break;
            }
        }

        shell->puts("\r\n");
    }
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

void shell_show_cmd(shell_t* shell, cmd_t* cmd)
{
    char* name = shell_get_cmd_name(shell, cmd);
    char* desc = shell_get_cmd_desc(shell, cmd);

    if (name == NULL)
    {
        name = "";
    }

    if (desc == NULL)
    {
        desc = "";
    }

    shell_printf(shell, "  %-20s\t%s\r\n", name, desc);
}

static void shell_set_var(shell_t* shell, char* name, void* value)
{
    cmd_t* cmd = shell_seek_cmd(shell, name);

    if (cmd == NULL)
    {
        shell->puts("Variable does not exist\r\n");
    }
    else
    {
        if (cmd->attr.bits.ext.rw == CMD_RO)
        {
            shell->puts("can't set read only var\r\n");
        }
        else
        {
            switch (cmd->attr.bits.type)
            {
                case CMD_TYPE_VAR_STRING:
                    strcpy((char*)(cmd->var.addr), (const char*)(value));
                    break;
                case CMD_TYPE_VAR_INT8:
                case CMD_TYPE_VAR_UINT8:
                    memcpy(cmd->var.addr, value, 1);
                    break;
                case CMD_TYPE_VAR_INT16:
                case CMD_TYPE_VAR_UINT16:
                    memcpy(cmd->var.addr, value, 2);
                    break;
                case CMD_TYPE_VAR_INT32:
                case CMD_TYPE_VAR_UINT32:
                case CMD_TYPE_VAR_FLOAT32:
                    memcpy(cmd->var.addr, value, 4);
                    break;
                case CMD_TYPE_VAR_INT64:
                case CMD_TYPE_VAR_UINT64:
                case CMD_TYPE_VAR_FLOAT64:
                    memcpy(cmd->var.addr, value, 8);
                    break;
                default:
                    break;
            }
        }
    }
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_FUNC_ARGC(2), setv, shell_set_var, set var)

static void shell_show_logo(shell_t* shell)
{
    // http://patorjk.com/software/taag
    // Font Name: Delta Corps Priest 1

    shell->puts("\r\n");
    shell->puts("███    █▄     ▄████████    ▄█    █▄       ▄████████  ▄█        ▄█       \r\n");
    shell->puts("███    ███   ███    ███   ███    ███     ███    ███ ███       ███       \r\n");
    shell->puts("███    ███   ███    █▀    ███    ███     ███    █▀  ███       ███       \r\n");
    shell->puts("███    ███   ███         ▄███▄▄▄▄███▄▄  ▄███▄▄▄     ███       ███       \r\n");
    shell->puts("███    ███ ▀███████████ ▀▀███▀▀▀▀███▀  ▀▀███▀▀▀     ███       ███       \r\n");
    shell->puts("███    ███          ███   ███    ███     ███    █▄  ███       ███       \r\n");
    shell->puts("███    ███    ▄█    ███   ███    ███     ███    ███ ███▌    ▄ ███▌    ▄ \r\n");
    shell->puts("████████▀   ▄████████▀    ███    █▀      ██████████ █████▄▄██ █████▄▄██ \r\n");
    shell->puts("                                                    ▀         ▀         \r\n");
    shell->puts("\r\n");
}

static void shell_show_prompt(shell_t* shell)
{
    shell_printf(shell, shell->prompt);
}

void shell_init(shell_t* shell, char buffer[], uint16_t capacity)
{
    shell->parser.buffer    = buffer;
    shell->parser.capacity  = capacity;
    shell->parser.length    = 0;
    shell->parser.cursor    = 0;
    shell->parser.byteshift = 24;

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)

    extern const uint32_t CMDS$$Base;
    extern const uint32_t CMDS$$Limit;

    shell->cmds.base  = (cmd_t*)(&CMDS$$Base);
    shell->cmds.count = ((uint32_t)&CMDS$$Limit - (uint32_t)&CMDS$$Base) / sizeof(cmd_t);

    extern const uint32_t KEYS$$Base;
    extern const uint32_t KEYS$$Limit;

    shell->keys.base  = (cmd_t*)(&KEYS$$Base);
    shell->keys.count = ((uint32_t)&KEYS$$Limit - (uint32_t)&KEYS$$Base) / sizeof(cmd_t);

#elif defined(__ICCARM__) || defined(__ICCRX__)

    shell->cmds.base  = (cmd_t*)(__section_begin("CMDS"));
    shell->cmds.count = ((uint32_t)(__section_end("CMDS")) - (uint32_t)(__section_begin("CMDS"))) / sizeof(cmd_t);

    shell->cmds.base  = (cmd_t*)(__section_begin("KEYS"));
    shell->cmds.count = ((uint32_t)(__section_end("KEYS")) - (uint32_t)(__section_begin("KEYS"))) / sizeof(cmd_t);

#elif defined(__GNUC__)

    // shell->cmds.base  = (cmd_t*)(&_shell_command_start);
    // shell->cmds.count = ((uint32_t)(&_shell_command_end) - (uint32_t)(&_shell_command_start)) / sizeof(cmd_t);

#else

#error not supported compiler, please use command table mode.

#endif

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

    shell->history.offset = 0;
    shell->history.count  = 0;
    shell->history.record = 0;

    char* p = buffer;

    for (uint16_t i = 0; i < CONFIG_SHELL_HISTROY_MAX_COUNT; i++)
    {
        p += shell->parser.capacity;
        shell->history.buffer[i] = p;
    }

#endif

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        char* name   = shell_get_cmd_name(shell, &base[i]);
        base[i].hash = (name == NULL) ? 0 : generate_hash(name);
    }

    console_clear_screen(shell);
    shell_show_logo(shell);
    shell_show_prompt(shell);
}

void shell_loop(shell_t* shell)
{
    char ch;

    if (shell->getc(&ch))
    {
        if (shell_key_scan(shell, ch))
        {
            return;
        }

        shell_insert_byte(shell, ch);
    }
}

static void shell_insert_byte(shell_t* shell, uint8_t data)
{
    if (shell->parser.length >= (shell->parser.capacity - 1))
    {
        shell->puts("\r\nWarning: Command is too long\r\n");
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
        shell_printf(shell, "%c", data);
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

        shell->puts(&shell->parser.buffer[shell->parser.cursor - 1]);

        for (i = shell->parser.length - shell->parser.cursor; i > 0; i--)
        {
            shell->puts("\b");
        }

        // replace mode...
    }
}

static void shell_remove_byte(shell_t* shell, del_e dir)
{
    if ((shell->parser.cursor == shell->parser.length && dir == DEL_CURR) ||
        (shell->parser.cursor == 0 && dir == DEL_PREV))
    {
        return;
    }

    if (shell->parser.cursor == shell->parser.length)
    {
        shell->parser.length--;
        shell->parser.cursor--;
        shell->parser.buffer[shell->parser.length] = '\0';
        console_delete_char(shell);
    }
    else
    {
        uint16_t i;

        // inser mode
        uint8_t offset = (dir == DEL_CURR) ? 1 : 0;

        // replace mode
        // uint8_t offset = (dir == DEL_PREV) ? 1 : 0;

        for (i = offset + shell->parser.cursor; i < shell->parser.length; i++)
        {
            // move forward
            shell->parser.buffer[i - 1] = shell->parser.buffer[i];
        }

        shell->parser.length--;

        if (!offset)
        {
            shell->parser.cursor--;
            shell->puts("\b");
        }

        shell->parser.buffer[shell->parser.length] = '\0';

        shell->puts(&shell->parser.buffer[shell->parser.cursor]);

        // the last char
        shell->puts(" ");

        for (i = shell->parser.length - shell->parser.cursor + 1; i > 0; i--)
        {
            shell->puts("\b");
        }
    }
}

static bool shell_key_scan(shell_t* shell, uint8_t data)
{
    cmd_t* base = shell->keys.base;

    for (uint16_t i = 0; i < shell->keys.count; ++i)
    {
        if (base[i].attr.bits.type == CMD_TYPE_KEY)
        {
            uint8_t cur_key = (base[i].key.value >> shell->parser.byteshift) & 0xFF;

            if (cur_key != data)
            {
                continue;
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
                }
            }
            else
            {
                // current key is last
                matched = true;
            }

            if (matched)
            {
                // reset byteshift
                shell->parser.byteshift = 24;

                // call key handler
                shell_call_func(shell, &base[i]);
            }

            return true;
        }
    }

    return false;
}

static void shell_exec(shell_t* shell)
{
    shell->puts("\r\n");

    if (shell->parser.length == 0)
    {
        return;
    }

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0
    history_append(shell);
#endif

    if (shell_parse_cmd(shell) == false)
    {
        return;
    }

    cmd_t* cmd = shell_seek_cmd(shell, shell->parser.argv[0]);

    if (cmd == NULL)
    {
        shell->puts("Command not Found: \"");
        shell->puts(shell->parser.buffer);
        shell->puts("\"\r\n");
    }
    else
    {
        if (__cmd_type_func_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_func_end)
        {
            shell_call_func(shell, cmd);
        }
        else if (__cmd_type_var_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_var_end)
        {
            shell_show_var(shell, cmd);
        }
    }

    shell->parser.length = shell->parser.cursor = 0;
}

static cmd_t* shell_seek_cmd(shell_t* shell, const char* str)
{
    cmd_t* base = shell->cmds.base;

    uint32_t hash = generate_hash(str);

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        if (hash == base[i].hash)
        {
            char* name = shell_get_cmd_name(shell, &base[i]);

            if (name != NULL && strcmp(str, name) == 0)
            {
                return &base[i];
            }
        }
    }

    return NULL;
}

static char* shell_get_cmd_desc(shell_t* shell, cmd_t* cmd)
{
    if (__cmd_type_func_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_func_end)
    {
        return cmd->func.desc;
    }
    else if (__cmd_type_var_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_var_end)
    {
        return cmd->var.desc;
    }
    else if (cmd->attr.bits.type == CMD_TYPE_KEY)
    {
        return cmd->key.desc;
    }

    return NULL;
}

static char* shell_get_cmd_name(shell_t* shell, cmd_t* cmd)
{
    if (__cmd_type_func_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_func_end)
    {
        return cmd->func.name;
    }
    else if (__cmd_type_var_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_var_end)
    {
        return cmd->var.name;
    }
    else if (cmd->attr.bits.type == CMD_TYPE_KEY)
    {
        return NULL;
    }

    return NULL;
}

static conv_state_e scan_int(char* str, uint8_t base, uint32_t* ret, char** pend)
{
    char* pcur = str;

    uint32_t num = 0;
    uint8_t  addon;

    if (*pcur)
    {
        for (; *pcur; ++pcur)
        {
            if (base < 10)
            {
                if (INCLOSE('0', *pcur, '0' - 1 + base))
                {
                    addon = *pcur - '0';
                }
                else
                {
                    break;
                }
            }
            else
            {
                if (INCLOSE('0', *pcur, '9'))
                {
                    addon = *pcur - '0';
                }
                else if (INCLOSE('a', *pcur, 'a' - 10 + base))
                {
                    addon = *pcur - 'a' + 10;
                }
                else if (INCLOSE('A', *pcur, 'A' - 10 + base))
                {
                    addon = *pcur - 'A' + 10;
                }
                else
                {
                    break;
                }
            }

            num *= (uint32_t)base;
            num += addon;
        }

        *ret = num;

        if (pend != NULL)
        {
            // return the pointer where it stop
            *pend = pcur;
        }

        return *pcur ? CONV_STATE_HALT : CONV_STATE_FULL;
    }

    return CONV_STATE_NONE;
}

arg_type_e shell_parse_arg(shell_t* shell, char* str, uint32_t* ret)
{
    char* p = str;

    uint32_t integer  = 0;  // 整数
    uint32_t decimal  = 0;  // 小数
    uint32_t exponent = 0;  // 指数

    sign_e int_sign = SIGN_POS;
    sign_e exp_sign = SIGN_POS;

    switch (*p)
    {
        case '0': {
            switch (*++p)
            {
                case 'b':
                case 'B':  // bin
                {
                    if (scan_int(++p, 2, &integer, NULL) == CONV_STATE_FULL)
                    {
                        goto _as_int;
                    }

                    break;
                }

                case 'x':
                case 'X':  // hex
                {
                    if (scan_int(++p, 16, &integer, NULL) == CONV_STATE_FULL)
                    {
                        goto _as_int;
                    }
                    break;
                }

                // float
                case '.':
                    goto _chk_flt_pt;
                case 'e':
                case 'E':
                    goto _chk_flt_exp;

                case '0' ... '7':  // oct
                {
                    if (scan_int(p, 8, &integer, NULL) == CONV_STATE_FULL)
                    {
                        goto _as_int;
                    }

                    break;
                }

                default:
                    break;
            }

            goto _as_str;
        }

        case '-':
            int_sign = SIGN_NEG;
        case '+':
        case '1' ... '9': {
            if (scan_int(p, 10, &integer, &p) == CONV_STATE_FULL)
            {
                goto _as_int;
            }

            if (*p == '.')
            {
            _chk_flt_pt:
                if (scan_int(++p, 10, &decimal, &p) == CONV_STATE_FULL)
                {
                    goto _as_flt;
                }
            }

            if (*p == 'e' || *p == 'E')
            {
            _chk_flt_exp:
                switch (*++p)
                {
                    case '-':
                        exp_sign = SIGN_NEG;
                    case '+':
                        ++p;
                    case '0' ... '9': {
                        if (scan_int(p, 10, &exponent, NULL) == CONV_STATE_FULL)
                        {
                            if (exp_sign == SIGN_NEG)
                            {
                                exponent = -exponent;
                            }

                            goto _as_flt;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            goto _as_str;
        }

        case '$': {
            if (*++p)
            {
                // var scan
                goto _as_var;
            }

            goto _as_str;
        }

        default:
            goto _as_str;
    }

    //-------------------------------------------------------------------------

    float flt;

_as_str:

    // format it
    *ret = (uint32_t)str;
    return TYPE_STRING;

_as_int:

    if (int_sign == SIGN_POS)
    {
        *ret = integer;
        return TYPE_POS_INT;
    }
    else
    {
        *ret = -integer;
        return TYPE_NEG_INT;
    }

_as_flt:

    flt = decimal;

    while (flt > 1)
    {
        flt /= 10;
    }

    flt += integer;

    if (int_sign == SIGN_NEG)
    {
        flt = -flt;
    }

    if (exp_sign == SIGN_POS)
    {
        while (exponent--)
        {
            flt *= 10;

            if (flt == 0.0f)
            {
                break;
            }
        }
    }
    else
    {
        while (exponent--)
        {
            flt /= 10;

            if (flt == 0.0f)
            {
                break;
            }
        }
    }

    *ret = *(uint32_t*)&flt;

    return TYPE_FLOAT;

_as_var:
    return TYPE_NEG_INT;
}

static bool shell_parse_cmd(shell_t* shell)
{
    struct {
        uint8_t inArg   : 1;
        uint8_t inQuote : 1;
        uint8_t done    : 1;
    } state = {0};

    uint16_t i = 0;

    char*    inbuf = shell->parser.buffer;
    uint16_t len   = shell->parser.length;

    uint16_t argc = 0;
    char**   argv = shell->parser.argv;

    for (; !state.done & (argc <= CONFIG_SHELL_PARAMETER_MAX_COUNT); ++i)
    {
        switch (inbuf[i])
        {
            case '\0':  // end
            {
                if (state.inQuote)
                {
                    goto __error;
                }
                state.done = 1;
                break;
            }

            case '"':  // str
            {
                if (state.inQuote)
                {
                    if (state.inArg)
                    {
                        // end of string
                        state.inQuote = 0;
                        goto __end_arg;
                    }
                    else
                    {
                        goto __error;
                    }
                }
                else if (!state.inArg)
                {
                    // beginning of string arg
                    state.inArg = state.inQuote = 1;
                    argv[argc++]                = &inbuf[i + 1];
                }

                break;
            }

            case ' ': {
                if (state.inArg)
                {
                    if (!state.inQuote)
                    {
                    __end_arg:
                        state.inArg = 0;
                        inbuf[i]    = '\0';  // end of arg
                    }
                }
                break;
            }

            case '\\': {
                switch (inbuf[i + 1])
                {
                    case '"':
                        // next char isn't a delimiter
                        inbuf[i] = '"';
                        break;
                    case ' ':
                        // next char isn't a delimiter
                        inbuf[i] = ' ';
                        break;
                    case 'a':
                        inbuf[i] = '\a';
                        break;
                    case 'b':
                        inbuf[i] = '\b';
                        break;
                    case 'r':
                        inbuf[i] = '\r';
                        break;
                    case 'n':
                        inbuf[i] = '\n';
                        break;
                    case 't':
                        inbuf[i] = '\t';
                        break;
                    case '\\':
                        inbuf[i] = '\\';
                        break;
                    case '0': {
                        inbuf[i--] = '\0';
                        continue;
                    }
                    case '\0':
                    default: {
                        continue;
                    }
                }

                strcpy(&inbuf[i + 1], &inbuf[i + 2]);

                break;
            }
            default: {
                if (!state.inArg)
                {
                    // beginning of arg
                    state.inArg  = 1;
                    argv[argc++] = &inbuf[i];
                }
                break;
            }
        }
    }

    if (state.inQuote)
    {
        goto __error;
    }

    shell->parser.argc = argc;

    return true;

__error:

    shell->parser.argc = 0;  // ERR_SYNTAX

    for (uint16_t i = shell->parser.argc; i <= CONFIG_SHELL_PARAMETER_MAX_COUNT; i++)
    {
        shell->parser.argv[i] = NULL;
    }

    shell_printf(shell, "syntax error\r\n");

    return false;
}

// CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), exec, shellExecute, execute function undefined);

//-----------------------------------------------------------------------------
// shell format inout

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
            shell_printf(shell, "%c", buffer[index]);

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

void shell_printf(shell_t* shell, const char* fmt, ...)
{
    static char buffer[CONFIG_SHELL_PRINTF_BUFSIZE];

    va_list vargs;

    va_start(vargs, fmt);
    uint16_t length = vsnprintf(buffer, CONFIG_SHELL_PRINTF_BUFSIZE, fmt, vargs);
    va_end(vargs);

    shell->puts(buffer);
}

//-----------------------------------------------------------------------------
// shell history

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

void history_append(shell_t* shell)
{
    shell->history.offset = 0;

    if (shell->history.count > 0)
    {
        if (strcmp(shell->history.buffer[(CONFIG_SHELL_HISTROY_MAX_COUNT + shell->history.record - 1) % CONFIG_SHELL_HISTROY_MAX_COUNT],
                   shell->parser.buffer) == 0)
        {
            // same as the last record
            return;
        }
    }

    strcpy(shell->history.buffer[shell->history.record++], shell->parser.buffer);

    if (shell->history.count < CONFIG_SHELL_HISTROY_MAX_COUNT)
    {
        shell->history.count++;
    }

    if (shell->history.record == CONFIG_SHELL_HISTROY_MAX_COUNT)
    {
        shell->history.record = 0;
    }
}

void history_swtich(shell_t* shell, dir_e dir)
{
    if (dir == DIR_PREV)
    {
        if (shell->history.offset == -shell->history.count)
        {
            return;
        }

        shell->history.offset--;
    }
    else if (dir == DIR_NEXT)
    {
        if (shell->history.offset == 0)
        {
            return;
        }

        shell->history.offset++;
    }
    else
    {
        return;
    }

    console_clear_line(shell);

    if (shell->history.offset == 0)
    {
        shell->parser.cursor = shell->parser.length = 0;
    }
    else
    {
        strcpy(shell->parser.buffer, shell->history.buffer[(CONFIG_SHELL_HISTROY_MAX_COUNT + shell->history.record + shell->history.offset) % CONFIG_SHELL_HISTROY_MAX_COUNT]);
        shell->parser.cursor = shell->parser.length = strlen(shell->parser.buffer);
        shell_printf(shell, shell->parser.buffer);
    }
}

#endif

//-----------------------------------------------------------------------------
// shell helper

void shell_list_all(shell_t* shell)
{
    shell_list_cmds(shell);
    shell_list_vars(shell);
}

static void shell_show_help(shell_t* shell, int argc, char* argv[])
{
    if (argc == 1)
    {
        shell_list_all(shell);
    }
    else if (argc == 2)
    {
        cmd_t* cmd = shell_seek_cmd(shell, argv[1]);

        if (cmd != NULL)
        {
            char* desc = shell_get_cmd_desc(shell, cmd);

            if (desc != NULL)
            {
                shell->puts(desc);
                shell->puts("\r\n");
            }
        }
    }
}

void shell_list_cmds(shell_t* shell)
{
    shell->puts("Command List:\r\n");

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        if (__cmd_type_func_start < base[i].attr.bits.type && base[i].attr.bits.type < __cmd_type_func_end)
        {
            shell_show_cmd(shell, &base[i]);
        }
    }
}

void shell_list_vars(shell_t* shell)
{
    shell->puts("Variable List:\r\n");

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        if (__cmd_type_var_start < base[i].attr.bits.type && base[i].attr.bits.type < __cmd_type_var_end)
        {
            shell_show_cmd(shell, &base[i]);
        }
    }
}

void shell_show_history(shell_t* shell)
{
    for (uint16_t i = 0; i < shell->history.count; ++i)
    {
        shell->puts(shell->history.buffer[i]);
        shell->puts("\r\n");
    }
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), cmds, shell_list_cmds, list all cmd)
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), vars, shell_list_vars, list all var)
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), help, shell_show_help, show command info);
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), history, shell_show_history, show history)

static void shell_echo(shell_t* shell, int argc, char* argv[])
{
    if (argc >= 2)
    {
        shell_printf(shell, argv[1]);
        shell_printf(shell, "\r\n");
    }
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), echo, shell_echo, echo)

//-----------------------------------------------------------------------------
// console key handler

/*
 *  Key Binding   Editor Action
 *     Ctrl A     Move cursor to start of the line
 *     Ctrl B     Move left one character
 *     Ctrl D     Delete a single character at the cursor position
 *     Ctrl E     Move cursor to end of current line
 *     Ctrl F     Move right one character
 *     Ctrl H     Delete character, left
 *     Ctrl K     Delete to the end of the line
 *     Ctrl U     Delete the entire line
 */

static void console_delete_char(shell_t* shell)
{
    shell->puts("\b \b");
}

static void console_delete_chars(shell_t* shell, uint16_t size)
{
    while (size--)
    {
        console_delete_char(shell);
    }
}

static void console_clear_line(shell_t* shell)
{
    // delete the entire line
    shell->puts("\33[2K\r");
    shell->parser.buffer[shell->parser.length = shell->parser.cursor = 0] = '\0';
    shell_show_prompt(shell);
}
CMD_EXPORT_KEY(0, 0x15000000, console_clear_line, clear line);  // Ctrl+U

static void console_clear_screen(shell_t* shell)
{
    shell->puts("\e[2J\e[1H");
}
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_FUNC_ARGC(0), cls, console_clear_screen, clear console);
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_FUNC_ARGC(0), clear, console_clear_screen, clear console);

static void console_cursor_delete_to_end(shell_t* shell, uint16_t distance)
{
    // Delete to the end of the line

    if (shell->parser.cursor < shell->parser.length)
    {
        uint16_t i;

        for (i = shell->parser.cursor; i < shell->parser.length; i++)
        {
            shell->puts(" ");
        }
        for (i = shell->parser.cursor; i < shell->parser.length; i++)
        {
            shell->puts("\b");
        }

        shell->parser.buffer[shell->parser.length = shell->parser.cursor] = '\0';
    }
}
CMD_EXPORT_KEY(0, 0x0B000000, console_cursor_delete_to_end, delete to the end of the line);  // Ctrl+K

static void console_key_tab(shell_t* shell)
{
    if (shell->parser.length == 0)
    {
        shell->puts("\r\n");
        shell_list_all(shell);
        shell_show_prompt(shell);
    }
    else  // length > 0
    {
        uint16_t nLastMacthedIndex = 0;
        uint16_t nMatchedCount     = 0;

        cmd_t* base = (cmd_t*)shell->cmds.base;

        for (uint16_t i = 0; i < shell->cmds.count; i++)
        {
            char* name = shell_get_cmd_name(shell, &base[i]);

            if (name != NULL)
            {
                if (strncmp(name, shell->parser.buffer, shell->parser.length) == 0)
                {
                    if (++nMatchedCount == 1)
                    {
                        shell->puts("\r\n");
                    }

                    nLastMacthedIndex = i;

                    shell_show_cmd(shell, &base[i]);
                }
            }
        }

        if (nMatchedCount == 0)
        {
            return;
        }
        else if (nMatchedCount == 1)
        {
            strcpy(shell->parser.buffer, shell_get_cmd_name(shell, &base[nLastMacthedIndex]));
            shell->parser.cursor = shell->parser.length = strlen(shell->parser.buffer);
        }

        shell_show_prompt(shell);
        shell->puts(shell->parser.buffer);
    }
}
CMD_EXPORT_KEY(0, 0x09000000, console_key_tab, tab);  // '\t'

static void console_key_delete(shell_t* shell)
{
    // delete a single character at the cursor position
    shell_remove_byte(shell, DEL_CURR);
}
CMD_EXPORT_KEY(0, 0x1B5B337E, console_key_delete, delete);  // Delete
CMD_EXPORT_KEY(0, 0x04000000, console_key_delete, delete);  // Ctrl+D

static void console_key_backspace(shell_t* shell)
{
    shell_remove_byte(shell, DEL_PREV);
}
CMD_EXPORT_KEY(0, 0x08000000, console_key_backspace, backspace);  // Backspace
CMD_EXPORT_KEY(0, 0x7F000000, console_key_backspace, backspace);  // Ctrl+H

static void console_key_enter(shell_t* shell)
{
    shell_exec(shell);
    shell_show_prompt(shell);
}
CMD_EXPORT_KEY(0, 0x0A000000, console_key_enter, enter);  // ENTER_LF, '\r'
CMD_EXPORT_KEY(0, 0x0D000000, console_key_enter, enter);  // ENTER_CR, '\n'
CMD_EXPORT_KEY(0, 0x0D0A0000, console_key_enter, enter);  // ENTER_CRLF, '\r\n'

static void console_key_letf_arrow(shell_t* shell)
{
    if (shell->parser.cursor > 0)
    {
        shell->parser.cursor--;
        shell->puts("\b");
    }
}
CMD_EXPORT_KEY(0, 0x1B5B4400, console_key_letf_arrow, left);  // '\e[D'
CMD_EXPORT_KEY(0, 0x02000000, console_key_letf_arrow, left);  // Ctrl+B

static void console_key_right_arrow(shell_t* shell)
{
    if (shell->parser.cursor < shell->parser.length)
    {
        shell_printf(shell, "%c", shell->parser.buffer[shell->parser.cursor++]);
    }
}
CMD_EXPORT_KEY(0, 0x1B5B4300, console_key_right_arrow, right);  // '\e[C'
CMD_EXPORT_KEY(0, 0x06000000, console_key_right_arrow, right);  // Ctrl+F

static void console_cursor_move_begin(shell_t* shell)
{
    // Move cursor to start of the line
    while (shell->parser.cursor)
    {
        shell->puts("\b");
        shell->parser.cursor--;
    }
}
CMD_EXPORT_KEY(0, 0x01000000, console_cursor_move_begin, move cursor to start of the line);  // Ctrl+A

static void console_cursor_move_end(shell_t* shell, uint16_t distance)
{
    // Move cursor to end of the line
    if (shell->parser.cursor < shell->parser.length)
    {
        shell->puts(&shell->parser.buffer[shell->parser.cursor]);
        shell->parser.cursor = shell->parser.length;
    }
}
CMD_EXPORT_KEY(0, 0x05000000, console_cursor_move_end, move cursor to end of the line);  // Ctrl+E

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

static void console_key_up_arrow(shell_t* shell)
{
    history_swtich(shell, DIR_PREV);
}
CMD_EXPORT_KEY(0, 0x1B5B4100, console_key_up_arrow, up);  // '\e[A'

static void console_key_down_arrow(shell_t* shell)
{
    history_swtich(shell, DIR_NEXT);
}
CMD_EXPORT_KEY(0, 0x1B5B4200, console_key_down_arrow, down);  // '\e[B'

#endif

//
