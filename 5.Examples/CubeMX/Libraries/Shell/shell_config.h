
#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build information related macros
 */
#define TO_STR(x)   #x
#define TOSTRING(x) TO_STR(x)

/**
 * Prompt
 */
#define PROMPT      "uYanki@root: "

/**
 * Max characters on the prompt that the shell will scan and process.
 * Defaults to 32.
 */
#ifndef LINE_BUFSIZE
#define LINE_BUFSIZE 32
#endif

/**
 * Maximum arguments can be half of line buffer.
 */
#define MAX_ARG_COUNT (LINE_BUFSIZE / 2)

/**
 * Maximum commands to be remembered in the history buffer.
 */
// define if user has not defined NUM_HISTORY_ENTRIES
#ifndef NUM_HISTORY_ENTRIES
#define NUM_HISTORY_ENTRIES 10
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

// Key codes
#define END_OF_LINE         '\0'
#define SPACE               ' '
#define TAB                 '\t'
#define NEW_LINE            '\n'
#define CARRIAGE_RETURN     '\r'
#define BACK_SPACE          '\b'
#define DELETE              '\177'
#define ESCAPE              '\33'
#define SQUARE_BRACKET_OPEN '\133'
#define UP_ARROW            '\101'

// Hash Key
#define CMD_HASH            0xb433e5c6

#ifdef __cplusplus
}
#endif

#endif
