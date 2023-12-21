
#include "shell.h"
#include "string.h"

/*
 * Memory manipulation utilities.
 */
#ifndef SHELL_NO_UTILS

#define REG32(addr) (*((volatile uint32_t*)((uintptr_t)(addr))))

int r32(int argc, char* argv[])
{
    if (argc < 2)
    {
        shell_printf("Usage: %s <address> (in hex)\n", argv[0]);
        return -1;
    }

    uint32_t addr = _atoh(argv[1]);
    uint32_t data;

    data = REG32(addr);

    shell_printf("0x%x: 0x%x\n", addr, data);
    return 0;
}

int w32(int argc, char* argv[])
{
    if (argc < 3)
    {
        shell_printf("Usage: %s <address> (in hex) <value> (in hex)\n", argv[0]);
        return -1;
    }
    uint32_t addr = _atoh(argv[1]);
    uint32_t data = _atoh(argv[2]);
    REG32(addr)   = data;

    return 0;
}

int read_mem(int argc, char* argv[])
{
    if (argc < 3)
    {
        shell_printf("Usage: %s <address> (in hex) <num_words> (in decimal)\n", argv[0]);
        return -1;
    }

    uint32_t addr   = _atoh(argv[1]);
    uint32_t length = _atoi(argv[2]);

    for (uint32_t i = 0, data = 0; i < length; i++)
    {
        data = REG32(addr);
        shell_printf("0x%x: 0x%x\n", addr, data);

        addr += 4;
    }

    return 0;
}

#ifndef SHELL_NO_BIT_UTILS

int w32_bit(int argc, char* argv[])
{
    if (argc < 4)
    {
        goto usage;
    }

    uint32_t addr    = _atoh(argv[1]);
    uint32_t bit_loc = _atoi(argv[2]);
    uint32_t bit_val = _atoi(argv[3]);

    if (bit_loc > 31)
    {
        shell_printf("Error: Invalid bit location argument\n");
        goto usage;
    }

    if (bit_val != 0 && bit_val != 1)
    {
        shell_printf("Error: Invalid bit value argument\n");
        goto usage;
    }

    uint32_t data = REG32(addr);
    data          = data | (bit_val << bit_loc);
    data          = data & ~(!bit_val << bit_loc);
    REG32(addr)   = data;

    return 0;
usage:
    shell_printf("Usage: %s <address_in_hex(32-bits)> <bit_location_in_decimal(0 to 31)> <bit_value(0 or 1)>\n", argv[0]);
    return -1;
}

int r32_bit(int argc, char* argv[])
{
    if (argc < 3)
    {
        goto usage;
    }

    uint32_t addr    = _atoh(argv[1]);
    uint32_t bit_loc = _atoi(argv[2]);

    if (bit_loc > 31)
    {
        shell_printf("Error: Invalid bit location argument\n");
        goto usage;
    }

    uint32_t data    = REG32(addr);
    uint32_t bit_val = (data >> bit_loc) & 0x1;
    shell_printf("Bit %u @ 0x%08x: %u\n", bit_loc, addr, bit_val);
    return 0;

usage:
    shell_printf("Usage: %s <address_in_hex(32-bits)> <bit_location_in_decimal(0 to 31)\n", argv[0]);
    return -1;
}

CMD_EXPORT(wb, "write a bit to memory location", w32_bit);
CMD_EXPORT(rb, "read a bit to memory location", r32_bit);
#endif  // SHELL_NO_BIT_UTILS

CMD_EXPORT(r32, "reads a 32 bit memory location", r32);
CMD_EXPORT(w32, "writes a 32 bit value to a memory location", w32);
CMD_EXPORT(read, "Reads number of bytes from memory", read_mem);
#endif  // SHELL_NO_UTILS
