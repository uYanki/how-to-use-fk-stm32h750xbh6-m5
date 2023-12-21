

#include "shell.h"
#include "i2c.h"
#include "shell_conf.h"
#include "string.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

#define I2C_TIMEOUT     0xFF

#if 0
#define I2C_ADDR_MIN 0x00
#define I2C_ADDR_MAX 0x7F
#else
#define I2C_ADDR_MIN 0x08
#define I2C_ADDR_MAX 0x77
#endif

typedef enum {
    DATA_SIZE_BYTE,
    DATA_SIZE_WORD,
    DATA_SIZE_DWORD,
} data_size_e;

static int parse_i2c_address(const char* str, uint8_t* addr)
{
    char* end;

    *addr = _strtol(str, &end, 0);

    if (*end || !*str)
    {
        shell_error("Error: Device address is not a number!\n");
        return -1;
    }

    if (*addr < I2C_ADDR_MIN || *addr > I2C_ADDR_MAX)
    {
        shell_error("Error: Device address out of range (0x%02X-0x%02X)!\n",
                    I2C_ADDR_MIN, I2C_ADDR_MAX);
        return -2;
    }

    return 0;
}

static int parse_reg_address(const char* str)
{
    char* end;

    int addr = _strtol(str, &end, 0);

    if (*end || !*str)
    {
        shell_error("Error: Register address is not a number!\n");
        return -1;
    }

    return addr;
}

static int parse_reg_value(const char* str)
{
    char* end;

    int val = _strtol(str, &end, 0);

    if (*end || !*str)
    {
        shell_error("Error: Register value is not a number!\n");
        return -1;
    }

    return val;
}

int i2c_scan(int argc, char* argv[])  // ok
{
    uint8_t addr = 0, step, cnt = 0;

    shell_printf("i2cdev 7-bit address detector:\n");
    shell_printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    while (addr < I2C_ADDR_MIN)
    {
        shell_printf("   ");
    }

    while (addr <= I2C_ADDR_MAX)
    {
        if ((addr & 0x0F) == 0)
        {
            shell_printf("%02X: ", addr);
        }

        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 5, HAL_MAX_DELAY) == HAL_OK)
        {
            shell_printf("%02X ", addr);
            ++cnt;
        }
        else
        {
            shell_printf("-- ");
        }

        ++addr;

        if ((addr & 0x0F) == 0)
        {
            shell_printf("\n");
        }
    }

    shell_printf(">> %d devices detected in this scan\n", cnt);

    return cnt;
}

int i2c_dump(int argc, char* argv[])
{
    if (argc < 2)
    {
        // clang-format off

        shell_info("Usage: %s ADDR [MODE]\n"
                     " ADDR is an integer (" TOSTRING(I2C_ADDR_MIN) " - " TOSTRING(I2C_ADDR_MAX) ")\n"
                     " MODE is one of:\n"
                     " - b (byte, default)\n"
                     " - w (word)\n"
                     " - dw (double word)\n",
                     argv[0]);

        // clang-format on

        return -1;
    }

    uint8_t dev_addr;

    if (parse_i2c_address(argv[1], &dev_addr) < 0)
    {
        return -1;
    }
    else
    {
        dev_addr <<= 1;
    }

    data_size_e size = DATA_SIZE_BYTE;

    if (argc > 2)
    {
        if (_strcmp(argv[2], "b") == 0) { size = DATA_SIZE_BYTE; }
        else if (_strcmp(argv[2], "w") == 0) { size = DATA_SIZE_WORD; }
        else if (_strcmp(argv[2], "dw") == 0) { size = DATA_SIZE_DWORD; }
    }

    uint16_t i, j;
    uint8_t  block[256];
    uint16_t first = 0x00, last = ARRAY_SIZE(block);  // TODO, for reg skip

    HAL_StatusTypeDef res;

    shell_printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef\n");

    for (i = 0; i < 256; i += 16)
    {
        if (i / 16 < first / 16)
        {
            continue;
        }
        if (i / 16 > last / 16)
        {
            break;
        }

        shell_printf("%02X: ", i);

        for (j = 0; j < 16; j++)
        {
            uint16_t reg = i + j;

            // skip unwanted registers
            if (reg < first || reg > last)
            {
                switch (size)
                {
                    case DATA_SIZE_DWORD: {
                        shell_printf("   ");
                        j++;
                        shell_printf("   ");
                        j++;
                    }
                    case DATA_SIZE_WORD: {
                        shell_printf("   ");
                        j++;
                    }
                    case DATA_SIZE_BYTE: {
                        shell_printf("   ");
                    }
                }

                continue;
            }

            uint8_t* buf = &block[reg];

            switch (size)  //  read register
            {
                case DATA_SIZE_BYTE: {
                    res = HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg, I2C_MEMADD_SIZE_8BIT, buf, 1, I2C_TIMEOUT);
                    break;
                }
                case DATA_SIZE_WORD: {
                    res = HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg, I2C_MEMADD_SIZE_8BIT, buf, 2, I2C_TIMEOUT);
                    break;
                }
                case DATA_SIZE_DWORD: {
                    res = HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg, I2C_MEMADD_SIZE_8BIT, buf, 4, I2C_TIMEOUT);
                    break;
                }
            }

            if (res == HAL_OK)  // display hex buffer
            {
                switch (size)
                {
                    case DATA_SIZE_DWORD: {
                        shell_printf("%02X ", *buf++);
                        shell_printf("%02X ", *buf++);
                        j += 2;
                    }
                    case DATA_SIZE_WORD: {
                        shell_printf("%02X ", *buf++);
                        j++;
                    }
                    case DATA_SIZE_BYTE: {
                        shell_printf("%02X ", *buf++);
                    }
                }
            }
            else
            {
                switch (size)
                {
                    case DATA_SIZE_DWORD: {
                        shell_printf("   ");
                        shell_printf("   ");
                        j += 2;
                    }
                    case DATA_SIZE_WORD: {
                        shell_printf("   ");
                        j++;
                    }
                    case DATA_SIZE_BYTE: {
                        shell_printf("   ");
                    }
                }
            }
        }

        shell_printf("   ");

        for (j = 0; j < 16; j++)  // display ascii buffer
        {
            uint16_t reg = i + j;

            // Skip unwanted registers
            if (reg < first || reg > last)
            {
                shell_printf(" ");
                continue;
            }

            if (res == HAL_OK)
            {
                switch (block[reg])
                {
                    case 0x00:
                    case 0xFF:
                        shell_printf(".");
                        break;

                    case 0x01 ... 0x1F:  // 1~31
                    case 0x7F ... 0xFE:  // 127~254
                        shell_printf(".");
                        break;

                    case 0x20 ... 0x7E:  // 32~126
                        shell_printf("%c", block[reg]);
                        break;

                    default:
                        break;
                }
            }
            else
            {
                shell_printf("X");
            }
        }

        shell_printf("\n");
    }

    return 0;
}

int i2c_get(int argc, char* argv[])  // TODO
{
    if (argc < 3)
    {
        // clang-format off

        shell_info("Usage: %s ADDR \n"
                     " ADDR is an integer (" TOSTRING(I2C_ADDR_MIN) " - " TOSTRING(I2C_ADDR_MAX) ")\n" 
                     " REG is an integer (0x0000 - 0xFFFF)\n",
                     argv[0]);

        // clang-format on

        return -1;
    }

    //

    uint8_t dev_addr;

    if (parse_i2c_address(argv[1], &dev_addr) < 0)
    {
        return -1;
    }
    else
    {
        dev_addr <<= 1;
    }

    //

    int reg_addr = parse_reg_address(argv[2]);

    if (reg_addr < 0)
    {
        return -1;
    }

    //

    uint8_t reg_value;

    HAL_StatusTypeDef res;

    res = HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg_addr, (reg_addr <= 0xFF) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, &reg_value, 1, I2C_TIMEOUT);

    shell_printf("0x%02X\n", reg_value);

    return res;
}

int i2c_set(int argc, char* argv[])  // TODO
{
    if (argc < 4)
    {
        // clang-format off

        shell_info("Usage: %s ADDR REG VAL\n"
                     " ADDR is an integer (" TOSTRING(I2C_ADDR_MIN) " - " TOSTRING(I2C_ADDR_MAX) ")\n"
                     " REG is an integer (0x0000 - 0xFFFF)\n",
                     argv[0]);

        // clang-format on

        return -1;
    }

    //

    uint8_t dev_addr;

    if (parse_i2c_address(argv[1], &dev_addr) < 0)
    {
        return -1;
    }
    else
    {
        dev_addr <<= 1;
    }

    //

    int reg_addr = parse_reg_address(argv[2]);

    if (reg_addr < 0)
    {
        return -1;
    }

    //

    int reg_value = parse_reg_value(argv[3]);

    if (reg_value < 0)
    {
        return -1;
    }

    //

    HAL_StatusTypeDef res;

    res = HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg_addr, (reg_addr <= 0xFF) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, (uint8_t*)&reg_value, 1, I2C_TIMEOUT);
    shell_printf("0x%02X\n", reg_value);

    return res;
}

CMD_EXPORT(i2cscan, "scan i2c device address", i2c_scan);
CMD_EXPORT(i2cdump, "dump i2c device register", i2c_dump);
CMD_EXPORT(i2cset, "set register value", i2c_set);
CMD_EXPORT(i2cget, "get register value", i2c_get);
