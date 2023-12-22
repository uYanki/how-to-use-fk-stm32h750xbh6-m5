#include "shell.h"

static cmd_t* cmds_begin;
static cmd_t* cmds_end;

static volatile int  __cmd_exec_status;
static volatile bool __shell_echo = 1;  // 回显

//-----------------------------------------------------------------------------

#ifndef SHELL_NO_HISTORY

static volatile int his_cmds_cnt = 0;
static volatile int his_cmd_cur  = 0;
static char         history[NUM_HISTORY_ENTRIES][LINE_BUFSIZE];

static void handle_up_arrow(char* buff, int* size)
{
    if (his_cmd_cur < (his_cmds_cnt - NUM_HISTORY_ENTRIES) ||
        his_cmd_cur == 0)
    {
        shell_printf("%s", buff);
        return;
    }

    _memset(buff, 0, LINE_BUFSIZE);

    int index = (--his_cmd_cur % NUM_HISTORY_ENTRIES);
    _memcpy(buff, &history[index], LINE_BUFSIZE);
    *size = _strlen(buff);

    shell_printf("%s", buff);
}

static void handle_down_arrow(char* buff, int* size)
{
    _memset(buff, 0, LINE_BUFSIZE);

    *size = 0;

    if (his_cmd_cur == his_cmds_cnt)
    {
        return;
    }

    int index = (++his_cmd_cur % NUM_HISTORY_ENTRIES);
    _memcpy(buff, &history[index], LINE_BUFSIZE);
    *size = _strlen(buff);

    shell_printf("%s", buff);
}

static void add_history(const char* cmd)
{
    // add only if command is not empty
    if (cmd == NULL || *cmd == '\0')
    {
        return;
    }

    int index = his_cmds_cnt % NUM_HISTORY_ENTRIES;
    _memcpy(&history[index], cmd, LINE_BUFSIZE);
    his_cmds_cnt++;
    his_cmd_cur = his_cmds_cnt;
}

static int show_history(int argc, char** argv)
{
    uint32_t end   = his_cmds_cnt - 1;
    uint32_t begin = 0;

    if (his_cmds_cnt > NUM_HISTORY_ENTRIES)
    {
        begin = his_cmds_cnt - NUM_HISTORY_ENTRIES;
    }

    shell_printf("\n");

    for (uint32_t index = begin, i = 0; index <= end; ++index, ++i)
    {
        shell_printf("%2d. %s\n", i, history[index % NUM_HISTORY_ENTRIES]);
    }

    shell_printf("\n");

    return 0;
}

CMD_EXPORT(history, "Show command history", show_history);

#endif  // SHELL_NO_HISTORY

//-----------------------------------------------------------------------------

#ifndef SHELL_NO_TAB_COMPLETE

static int prefix_match(char* sub, int len, const char* str)
{
    if (sub == NULL || str == NULL || len <= 0 || len > _strlen(str))
    {
        return false;
    }

    for (int i = 0; i < len; ++i)
    {
        if (sub[i] != str[i])
        {
            return false;
        }
    }

    return true;
}

static void handle_tab(char* buff, int* size)
{
    if (buff == NULL || size <= 0)
    {
        return;
    }

    int i           = 0;
    int match_count = 0;

    cmd_t* last_match = NULL;

    // loop over all commands
    for (cmd_t* cmd = cmds_begin; cmd < cmds_end; cmd++)
    {
        // if prefix matches, print that as one of the options
        if (prefix_match(buff, *size, cmd->name))
        {
            match_count++;
            last_match = cmd;
            shell_printf("\n%s", cmd->name);
        }
    }

    // if only one match, then that's the command to be executed
    if (match_count == 1)
    {
        _memcpy(buff, last_match->name, LINE_BUFSIZE);
        *size = _strlen(buff);
    }

    // print current line with old/updated command
    if (match_count)
    {
        shell_printf("\n");
        shell_printf(PROMPT);
        shell_printf("%s", buff);
    }
}

#endif  // SHELL_NO_TAB_COMPLETE

//-----------------------------------------------------------------------------

static void delete(void)
{
#if 0
    shell_putchar(BACK_SPACE);
    shell_putchar(SPACE);
    shell_putchar(BACK_SPACE);
#else
    shell_printf("\b \b");
#endif
}

static void clear_prompt(int size)
{
    while (size)
    {
        delete ();
        size--;
    }
}

static int cmd_match(const char* str, const char* cmd)
{
    int c1, c2;

    do {
        c1 = _lower(*str++);
        c2 = _lower(*cmd++);
    } while ((c1 == c2) && c1);

    return c1 - c2;
}

static uint32_t cmd_hash(const char* str)
{
    int      tmp, c = *str;
    uint32_t seed = CMD_HASH;
    uint32_t hash = 0;

    while (*str)
    {
        tmp  = _lower(c);
        hash = (hash ^ seed) + tmp;
        str++;
        c = *str;
    }

    return hash;
}

void shell_loop(void)
{
    int special_key = 0;

    int  s;
    char c;

    static int  count              = 0;
    static char line[LINE_BUFSIZE] = {0};

    while (1)
    {
        s = shell_getchar();

        if (s == -1)
        {
            return;
        }

        c = (char)s;

        if (c == CARRIAGE_RETURN || c == NEW_LINE)
        {
            line[count] = END_OF_LINE;
            shell_putchar(NEW_LINE);
            break;
        }

        if (c == DELETE || c == BACK_SPACE)
        {
            if (!__shell_echo)
            {
                delete ();
                delete ();
            }

            // guard against the count going negative!
            if (count == 0)
            {
                continue;
            }

            count--;

            line[count] = END_OF_LINE;
            delete ();
        }
        else if (c == ESCAPE)
        {
            special_key = 1;
            continue;
        }
        else if (c == SQUARE_BRACKET_OPEN && special_key == 1)
        {
            special_key = 2;
            continue;
        }
        else if ((c == 'C' || c == 'D') && special_key != 0)
        {
            /* Ignore left/right arrow keys */
            special_key = 0;
            continue;
        }
        else if ((c == 'A' || c == 'B') && special_key == 2)
        {
            if (!__shell_echo)
            {
                clear_prompt(count + 4);
            }
            else
            {
                clear_prompt(count);
            }
#ifndef SHELL_NO_HISTORY
            if (c == 'A')
            {
                handle_up_arrow(line, &count);
            }
            else
            {
                handle_down_arrow(line, &count);
            }
#endif  // SHELL_NO_HISTORY
            special_key = 0;
            continue;
        }
#ifndef SHELL_NO_TAB_COMPLETE
        else if (c == TAB)
        {
            handle_tab(line, &count);
            continue;
        }
#endif  // SHELL_NO_TAB_COMPLETE
        else
        {
            line[count] = c;
            count++;
        }
        if (__shell_echo && c != DELETE && c != BACK_SPACE)
        {
            shell_putchar(c);
        }
    }

#ifndef SHELL_NO_HISTORY
    add_history(line);
#endif

    // execute the commands
    shell_exec(line);

    // clear buffer
    count = 0;

    shell_printf(PROMPT);
}

void shell_init(void)
{
#if defined(__CC_ARM) || defined(__CLANG_ARM) || 1 /* ARM C Compiler */

    extern const int CMDS$$Base;
    extern const int CMDS$$Limit;

    cmds_begin = (cmd_t*)&CMDS$$Base;
    cmds_end   = (cmd_t*)&CMDS$$Limit;

#elif defined(__ICCARM__) || defined(__ICCRX__) /* IAR Compiler */

    cmds_begin = __section_begin("CMDS");
    cmds_end   = __section_end("CMDS");

#endif

    for (cmd_t* cmd = cmds_begin; cmd < cmds_end; cmd++)
    {
        cmd->hash = cmd_hash(cmd->name);
    }

    shell_printf(PROMPT);
}

static int shell_parse(char** argv, char* buff, int argument_size)
{
    int argc   = 0;
    int pos    = 0;
    int length = _strlen(buff);

    while (pos <= length)
    {
        if (buff[pos] != '\t' && buff[pos] != SPACE && buff[pos] != END_OF_LINE)
        {
            argv[argc++] = &buff[pos];
        }

        for (; buff[pos] != '\t' && buff[pos] != SPACE && buff[pos] != END_OF_LINE; pos++)
            ;

        if (buff[pos] == '\t' || buff[pos] == SPACE)
        {
            buff[pos] = END_OF_LINE;
        }

        pos++;
    }

    return argc;
}

int shell_exec(char* cmd)
{
    int   argc;
    char* argv[MAX_ARG_COUNT];

    argc = shell_parse(argv, cmd, MAX_ARG_COUNT);

    if (argc > 0)
    {
        int matched = false;

        uint32_t hash = cmd_hash(argv[0]);

        for (cmd_t* cmd = cmds_begin; cmd < cmds_end; cmd++)
        {
            if (hash == cmd->hash)
            {
                if (cmd_match(argv[0], cmd->name) == 0)
                {
                    __cmd_exec_status = cmd->func(argc, &argv[0]);

                    matched = true;

                    break;
                }
            }
        }

        if (matched == false)
        {
            shell_printf("\"%s\": command not found. Use \"help\" to list all command.\n", argv[0]);
            __cmd_exec_status = -1;
        }
    }

    return __cmd_exec_status;
}

//-----------------------------------------------------------------------------

static int version(int argc, char** argv)
{
    shell_printf("Build Time: " __DATE__ " - " __TIME__ "\n");
    return 0;
}

static int clear(int argc, char** argv)
{
    shell_printf("\033[2J\033[H");
    return 0;
}

static int help(int argc, char** argv)
{
    bool verbose = true;

    shell_printf("\n");

    if (argc > 1 && (_strcmp(argv[1], "-l") == 0))
    {
        verbose = false;
    }
    else
    {
        shell_printf("use: help -l for list only.\n\n");
    }

    for (cmd_t* cmd = cmds_begin; cmd < cmds_end; cmd++)
    {
        shell_printf("%s",cmd->name);

        if (verbose)
        {
            shell_printf("\n\t%s",cmd->desc);
        }

        shell_printf("\n");
    }

    shell_printf("\n");

    return 0;
}

static int exec_status(int argc, char** argv)
{
    shell_printf("%d\n", __cmd_exec_status);
    return 0;
}

CMD_EXPORT(version, "Prints details of the build", version)
CMD_EXPORT(help, "Prints all available commands", help)
CMD_EXPORT(status, "Returns exit status of last executed command", exec_status)
CMD_EXPORT(clear, "Clear terminal content", clear)
