#include "./core.h"
#include "./utils.h"

//-----------------------------------------------------------------------------

typedef enum {
    DIR_PREV,
    DIR_NEXT,
} dir_e;

typedef enum {
    DEL_PREV,  // 光标前
    DEL_CURR,  // 光标处
} del_e;

static bool shell_key_scan(shell_t* shell, uint8_t data);

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0
void history_append(shell_t* shell);
void history_swtich(shell_t* shell, dir_e dir);
#endif

static void shell_insert_byte(shell_t* shell, uint8_t data);
static void shell_remove_byte(shell_t* shell, del_e dir);

static void shell_parse(shell_t* shell);
static void shell_exec(shell_t* shell);

static cmd_t* shell_seek_cmd(shell_t* shell, const char* str);

static int  shell_call_func(shell_t* shell, cmd_t* cmd);
static void shell_show_var(shell_t* shell, cmd_t* cmd);

static void shell_show_help(shell_t* shell, int argc, char* argv[]);

static void console_delete_char(shell_t* shell);
static void console_delete_chars(shell_t* shell, uint16_t size);
static void console_clear_screen(shell_t* shell);
static void console_clear_line(shell_t* shell);

static char* shell_get_cmd_name(shell_t* shell, cmd_t* cmd);
static char* shell_get_cmd_desc(shell_t* shell, cmd_t* cmd);
static char* shell_get_cmd_type(shell_t* shell, cmd_t* cmd);

void shell_remove_args_quotes(shell_t* shell);

//-----------------------------------------------------------------------------

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

static int shell_call_func(shell_t* shell, cmd_t* cmd)
{
    if (cmd->attr.bits.type == CMD_TYPE_FUNC_MAIN)
    {
        shell_remove_args_quotes(shell);
        int ret = cmd->func.cbk(shell, shell->parser.argc, shell->parser.argv);
    }
    else if (cmd->attr.bits.type == CMD_TYPE_FUNC_C)
    {
        uint8_t argc = cmd->attr.bits.argc;

        if (argc != (shell->parser.argc - 1))
        {
            shell_printf(shell, "Too many or too few arguments. Only %d parameters are needed.\n", argc);
            goto _error;
        }

        int32_t args[CONFIG_SHELL_PARAMETER_MAX_COUNT] = {0};

        for (uint8_t i = 0; i < argc; i++)
        {
            args[i] = shell_ext_parse_value(shell, shell->parser.argv[i + 1]);
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

static void shell_show_var(shell_t* shell, cmd_t* cmd)
{
    if (cmd->attr.bits.rw != CMD_WO)
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
    char* type = shell_get_cmd_type(shell, cmd);

    shell->putc('  ');

    if (name != NULL)
    {
        shell->puts(name);
    }

    shell->putc('\t');

    if (type != NULL)
    {
        shell->puts(type);
    }

    shell->putc('\t');

    if (desc != NULL)
    {
        shell->puts(desc);
    }

    shell->putc('\n');
}

void shell_get_var(shell_t* shell, cmd_t* cmd)
{
    // shell->puts("can't get write only var");
    // shell->putc('\n');
    // pass
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

#elif defined(__ICCARM__) || defined(__ICCRX__)

    shell->cmds.base  = (cmd_t*)(__section_begin("CMDS"));
    shell->cmds.count = ((uint32_t)(__section_end("CMDS")) - (uint32_t)(__section_begin("CMDS"))) / sizeof(cmd_t);

#elif defined(__GNUC__)

    shell->cmds.base  = (cmd_t*)(&_shell_command_start);
    shell->cmds.count = ((uint32_t)(&_shell_command_end) - (uint32_t)(&_shell_command_start)) / sizeof(cmd_t);

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

    shell->putc('\n');
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

static void shell_insert_byte(shell_t* shell, uint8_t data)
{
    if (shell->parser.length >= (shell->parser.capacity - 1))
    {
        shell->putc('\n');
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
            shell->putc('\b');
        }

        shell->parser.buffer[shell->parser.length] = '\0';

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

static bool shell_key_scan(shell_t* shell, uint8_t data)
{
    cmd_t* base = shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; ++i)
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

    // reset byteshift
    shell->parser.byteshift = 24;

    return false;
}

static void shell_parse(shell_t* shell)
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

            if (shell->parser.argc == (CONFIG_SHELL_PARAMETER_MAX_COUNT + 1))
            {
                break;
            }

            record = false;
        }
    }

    for (i = shell->parser.argc; i < (CONFIG_SHELL_PARAMETER_MAX_COUNT + 1); i++)
    {
        shell->parser.argv[i] = NULL;
    }
}

static void shell_exec(shell_t* shell)
{
    if (shell->parser.length == 0)
    {
        return;
    }

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0
    history_append(shell);
#endif

    shell_parse(shell);

    if (shell->parser.argc == 0)
    {
        // all chars are whitespace
        return;
    }

    cmd_t* cmd = shell_seek_cmd(shell, shell->parser.argv[0]);

    shell->putc('\n');

    if (cmd == NULL)
    {
        shell->puts("Command not Found: ");
        shell->putc('\"');
        shell->puts(shell->parser.buffer);
        shell->putc('\"');
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

static char* shell_get_cmd_type(shell_t* shell, cmd_t* cmd)
{
    if (__cmd_type_func_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_func_end)
    {
        return "FUNC";
    }
    else if (__cmd_type_var_start < cmd->attr.bits.type && cmd->attr.bits.type < __cmd_type_var_end)
    {
        return "VAR";
    }
    else if (cmd->attr.bits.type == CMD_TYPE_KEY)
    {
        return "KEY";
    }

    return NULL;
}

// CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), exec, shellExecute, "execute function undefined");

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

void shell_printf(shell_t* shell, const char* fmt, ...)
{
    static char buffer[CONFIG_SHELL_PRINTF_BUFSIZE];

    va_list vargs;

    va_start(vargs, fmt);
    uint16_t length = vsnprintf(buffer, sizeof(buffer) - 1, fmt, vargs);
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
        shell->puts(shell->parser.buffer);
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
                shell->putc('\n');
                shell->puts(desc);
            }
        }
    }
}

void shell_list_cmds(shell_t* shell)
{
    shell->putc('\n');
    shell->puts("Command List:");
    shell->putc('\n');
    shell->putc('\n');

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
    shell->putc('\n');
    shell->puts("Variable List:");
    shell->putc('\n');
    shell->putc('\n');

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        if (__cmd_type_var_start < base[i].attr.bits.type && base[i].attr.bits.type < __cmd_type_var_end)
        {
            shell_show_cmd(shell, &base[i]);
        }
    }
}

void shell_list_keys(shell_t* shell)
{
    shell->putc('\n');
    shell->puts("Key List:");
    shell->putc('\n\n');

    cmd_t* base = (cmd_t*)shell->cmds.base;

    for (uint16_t i = 0; i < shell->cmds.count; i++)
    {
        if (base[i].attr.bits.type == CMD_TYPE_KEY)
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
        shell->putc('\n');
    }
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), cmds, shell_list_cmds, "list all cmd")
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), vars, shell_list_vars, "list all var")
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), keys, shell_list_keys, "list all key")
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), help, shell_show_help, "show command info");
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C), history, shell_show_history, "show history")

//-----------------------------------------------------------------------------
// console helper

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

static void console_clear_screen(shell_t* shell)
{
    shell->puts("\e[2J\e[1H");
}

static void console_clear_line(shell_t* shell)
{
    uint16_t n = shell->parser.length - shell->parser.cursor;

    while (n--)
    {
        shell->putc(' ');
    }

    console_delete_chars(shell, shell->parser.length);
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_ARGC(0), cls, console_clear_screen, "clear console");
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_ARGC(0), clear, console_clear_screen, "clear console");

//-----------------------------------------------------------------------------
// console key handler

static void console_key_tab(shell_t* shell)
{
    if (shell->parser.length == 0)
    {
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
                        shell->putc('\n');
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
CMD_EXPORT_KEY(0, 0x09000000, console_key_tab, "tab");  // '\t'

static void console_key_delete(shell_t* shell)
{
    shell_remove_byte(shell, DEL_CURR);
}
CMD_EXPORT_KEY(0, 0x1B5B337E, console_key_delete, "delete");

static void console_key_backspace(shell_t* shell)
{
    shell_remove_byte(shell, DEL_PREV);
}
CMD_EXPORT_KEY(0, 0x08000000, console_key_backspace, "backspace");
CMD_EXPORT_KEY(0, 0x7F000000, console_key_backspace, "backspace");

static void console_key_enter(shell_t* shell)
{
    shell_exec(shell);
    shell->putc('\n');
    shell_show_prompt(shell);
}
CMD_EXPORT_KEY(0, 0x0A000000, console_key_enter, "enter");  // SHELL_ENTER_LF, '\r'
CMD_EXPORT_KEY(0, 0x0D000000, console_key_enter, "enter");  // SHELL_ENTER_CR, '\n'
CMD_EXPORT_KEY(0, 0x0D0A0000, console_key_enter, "enter");  // SHELL_ENTER_CRLF, '\r\n'

static void console_key_letf_arrow(shell_t* shell)
{
    if (shell->parser.cursor > 0)
    {
        shell->parser.cursor--;
        shell->putc('\b');
    }
}
CMD_EXPORT_KEY(0, 0x1B5B4400, console_key_letf_arrow, "left");  // '\e[D'

static void console_key_right_arrow(shell_t* shell)
{
    if (shell->parser.cursor < shell->parser.length)
    {
        shell->putc(shell->parser.buffer[shell->parser.cursor++]);
    }
}
CMD_EXPORT_KEY(0, 0x1B5B4300, console_key_right_arrow, "right");  // '\e[C'

#if CONFIG_SHELL_HISTROY_MAX_COUNT > 0

static void console_key_up_arrow(shell_t* shell)
{
    history_swtich(shell, DIR_PREV);
}
CMD_EXPORT_KEY(0, 0x1B5B4100, console_key_up_arrow, "up");  // '\e[A'

static void console_key_down_arrow(shell_t* shell)
{
    history_swtich(shell, DIR_NEXT);
}
CMD_EXPORT_KEY(0, 0x1B5B4200, console_key_down_arrow, "down");  // '\e[B'

#endif

//

int base_conv(shell_t* shell, uint32_t number)
{
    char buffer[34] = {0};

    ltoa(number, buffer, 2);
    shell_printf(shell, "BIN: 0b%s\n", buffer);
    shell_printf(shell, "OCT: 0%o\n", number);
    shell_printf(shell, "DEC: %d\n", number);
    shell_printf(shell, "HEX: 0x%X\n", number);
    shell_printf(shell, "BCD: %d\n", dec2bcd(number));

    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_ARGC(1), base, base_conv, "number base conversion");

//

int print_ascii_table(shell_t* shell, int argc, char* argv[])
{
    /* prints out a basic ASCII table */
    /* dec, hex, char, octal, description */

    static const char desc[128][15] = {"null", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI", "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US", "space", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "~", "]", "^", "_", "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "DEL"};

    uint8_t start;
    uint8_t count = 0;

    if (argc == 1)
    {
        start = 0;
        count = 128;
    }
    else if (argc == 2)
    {
        int n = atoi(argv[1]);

        if (0 <= n && n <= 127)
        {
            start = n;
            count = 1;
        }
    }

    if (count == 0)
    {
        return -1;
    }

    shell->puts("dec\t hex\t char\t oct\t desc\n");
    shell->puts("===\t ===\t ====\t ===\t ====\n");

    for (int i = start; i < (start + count); i++)
    {
        shell_printf(shell, "%d\t %x\t %c\t %o\t %s\r\n", i, i, i <= 32 ? ' ' : i, i, desc[i]);
    }

    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), ascii, print_ascii_table, "display ascii table");
