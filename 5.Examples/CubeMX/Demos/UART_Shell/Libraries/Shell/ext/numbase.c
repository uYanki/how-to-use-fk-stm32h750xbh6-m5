#include "../core.h"

//-----------------------------------------------------------------------------

int base_conv(shell_t* shell, uint32_t number)
{
    char buffer[34] = {0};

    ltoa(number, buffer, 2);
    shell_printf(shell, "BIN: 0b%s\r\n", buffer);
    shell_printf(shell, "OCT: 0%o\r\n", number);
    shell_printf(shell, "DEC: %d\r\n", number);
    shell_printf(shell, "HEX: 0x%X\r\n", number);
    shell_printf(shell, "BCD: %d\r\n", dec2bcd(number));

    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_C) | CMD_FUNC_ARGC(1), base, base_conv, number base conversion);

//-----------------------------------------------------------------------------

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

    shell_printf(shell, "dec\t hex\t char\t oct\t desc\r\n");
    shell_printf(shell, "===\t ===\t ====\t ===\t ====\r\n");

    for (int i = start; i < (start + count); i++)
    {
        shell_printf(shell, "%d\t %x\t %c\t %o\t %s\r\n", i, i, i <= 32 ? ' ' : i, i, desc[i]);
    }

    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), ascii, print_ascii_table, display ascii table);
