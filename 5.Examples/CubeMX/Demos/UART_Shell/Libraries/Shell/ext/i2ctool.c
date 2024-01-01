#include "../core.h"

#include "i2c.h"
#include "./utils.h"

#define __TO_STRING(x) #x
#define TO_STRING(x)   __TO_STRING(x)

#define I2C_TIMEOUT    0xFF

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

static int __i2c_devaddr_parse(shell_t* shell, const char* argv, /* OUT */ uint8_t* dev_addr)
{
    uint32_t addr;

    if (cmd_parse_arg(shell, argv, &addr) != TYPE_POS_INT)
    {
        shell_error(shell, "Error: Device address is not a positive number!\r\n");
        return -1;
    }

    if (addr < I2C_ADDR_MIN || addr > I2C_ADDR_MAX)
    {
        shell_error(shell, "Error: Device address out of range (0x%02X-0x%02X)!\r\n",
                    I2C_ADDR_MIN, I2C_ADDR_MAX);
        return -2;
    }

    addr <<= 1;

    *dev_addr = addr;

    return 0;
}

static int __i2c_regaddr_parse(shell_t* shell, const char* argv, /* OUT */ uint16_t* reg_addr)
{
    uint32_t addr;

    if (cmd_parse_arg(shell, argv, &addr) != TYPE_POS_INT)
    {
        shell_error(shell, "Error: Register address is not a positive number!\r\n");
        return -1;
    }

    *reg_addr = addr;

    return 0;
}

static int __i2c_regvalue_parse(shell_t* shell, const char* argv, /* OUT */ uint8_t* reg_value)
{
    uint32_t value;

    switch (cmd_parse_arg(shell, argv, &value))
    {
        case TYPE_NEG_INT:
        case TYPE_POS_INT:
            break;
        default:
            shell_error(shell, "Error: Register value is not a number!\r\n");
            return -1;
    }

    *reg_value = value;

    return 0;
}

static int i2c_scan(shell_t* shell, int argc, char* argv[])  // ok
{
    uint8_t addr = 0, step, cnt = 0;

    shell->puts("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

    while (addr <= I2C_ADDR_MAX)
    {
        if ((addr & 0x0F) == 0)
        {
            shell_printf(shell, "\r\n%02X: ", addr);
        }

        if (addr < I2C_ADDR_MIN)
        {
            shell->puts("   ");
        }
        else
        {
            if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 5, I2C_TIMEOUT) == HAL_OK)
            {
                shell_printf(shell, "%02X ", addr);
                ++cnt;
            }
            else
            {
                shell->puts("-- ");
            }
        }

        ++addr;
    }

    shell_printf(shell, "\r\n>> %d devices detected in this scan\r\n", cnt);

    return cnt;
}

static int i2c_dump(shell_t* shell, int argc, char* argv[])
{
    int ret = 0;

    //-------------------------------------------

    if (argc < 2)
    {
        goto __usage;
    }

    //-------------------------------------------

    uint8_t dev_addr;

    if ((ret = __i2c_devaddr_parse(shell, argv[1], &dev_addr)) < 0)
    {
        goto __exit;
    }

    //-------------------------------------------

    data_size_e size = DATA_SIZE_BYTE;

    if (argc > 2)
    {
        if (_strcmp(argv[2], "b") == 0) { size = DATA_SIZE_BYTE; }
        else if (_strcmp(argv[2], "w") == 0) { size = DATA_SIZE_WORD; }
        else if (_strcmp(argv[2], "dw") == 0) { size = DATA_SIZE_DWORD; }
        else
        {
            ret = -1;
            goto __exit;
        }
    }

    //-------------------------------------------

    uint16_t i, j;
    uint8_t  block[256];
    uint16_t first = 0x00, last = ARRAY_SIZE(block);  // TODO, for reg skip

    HAL_StatusTypeDef res;

    shell->puts("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f    0123456789abcdef\r\n");

    for (i = 0; i < 256; i += 16)  // TODO: 改为单次读全部寄存器
    {
        if (i / 16 < first / 16)
        {
            continue;
        }
        if (i / 16 > last / 16)
        {
            break;
        }

        shell_printf(shell, "%02X: ", i);

        for (j = 0; j < 16; j++)
        {
            uint16_t reg = i + j;

            // skip unwanted registers
            if (reg < first || reg > last)
            {
                switch (size)
                {
                    case DATA_SIZE_DWORD: {
                        shell->puts("   ");
                        j++;
                        shell->puts("   ");
                        j++;
                    }
                    case DATA_SIZE_WORD: {
                        shell->puts("   ");
                        j++;
                    }
                    case DATA_SIZE_BYTE: {
                        shell->puts("   ");
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
                        shell_printf(shell, "%02X ", *buf++);
                        shell_printf(shell, "%02X ", *buf++);
                        j += 2;
                    }
                    case DATA_SIZE_WORD: {
                        shell_printf(shell, "%02X ", *buf++);
                        j++;
                    }
                    case DATA_SIZE_BYTE: {
                        shell_printf(shell, "%02X ", *buf++);
                    }
                }
            }
            else
            {
                switch (size)
                {
                    case DATA_SIZE_DWORD: {
                        shell->puts("   ");
                        shell->puts("   ");
                        j += 2;
                    }
                    case DATA_SIZE_WORD: {
                        shell->puts("   ");
                        j++;
                    }
                    case DATA_SIZE_BYTE: {
                        shell->puts("   ");
                    }
                }
            }
        }

        shell->puts("   ");

        for (j = 0; j < 16; j++)  // display ascii buffer
        {
            uint16_t reg = i + j;

            // Skip unwanted registers
            if (reg < first || reg > last)
            {
                shell->puts(" ");
                continue;
            }

            if (res == HAL_OK)
            {
                switch (block[reg])
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
                        shell_printf(shell, "%c", block[reg]);
                        break;

                    default:
                        break;
                }
            }
            else
            {
                shell->puts("X");
            }
        }

        shell->puts("\r\n");
    }

__exit:

    return ret;

__usage:

    // clang-format off
    shell_info(shell,
                "Usage: %s DEV [MODE]\r\n"
                " - DEV: device address, an integer (" TO_STRING(I2C_ADDR_MIN) " - " TO_STRING(I2C_ADDR_MAX) ")\r\n"
                " - MODE: data size, is one of:\r\n"
                "   - b (byte, default)\r\n"
                "   - w (word)\r\n"
                "   - dw (double word)\r\n",
                argv[0]);
    // clang-format on

    return 0;
}

static int i2c_get(shell_t* shell, int argc, char* argv[])
{
    int ret = 0;

    if (argc < 3)
    {
        goto __usage;
    }

    uint8_t  dev_addr;
    uint16_t reg_addr;
    uint8_t  reg_value;

    if (((ret = __i2c_devaddr_parse(shell, argv[1], &dev_addr)) < 0) ||
        ((ret = __i2c_regaddr_parse(shell, argv[1], &reg_addr)) < 0))
    {
        goto __exit;
    }

    if ((ret = __i2c_regaddr_parse(shell, argv[1], &reg_addr)) < 0)
    {
        goto __exit;
    }

    HAL_StatusTypeDef res = HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg_addr, (reg_addr <= 0xFF) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, &reg_value, 1, I2C_TIMEOUT);

    if (res != HAL_OK)
    {
        ret = -1;
        shell_printf(shell, "fail to access 0x%02X\r\n", reg_addr);
        goto __exit;
    }

    shell_printf(shell, "0x%02X\r\n", reg_value);

__exit:

    return ret;

    //-------------------------------------------

__usage:

    // clang-format off
    shell_info(shell,
                "Usage: %s DEV REG\r\n"
                " - DEV: device address, an integer (" TO_STRING(I2C_ADDR_MIN) " - " TO_STRING(I2C_ADDR_MAX) ")\r\n" 
                " - REG: register address, an integer (0x0000 - 0xFFFF)\r\n",
                argv[0]);
    // clang-format on

    return 0;
}

static int i2c_set(shell_t* shell, int argc, char* argv[])
{
    int ret = 0;

    if (argc < 4)
    {
        goto __usage;
    }

    uint8_t  dev_addr;
    uint16_t reg_addr;
    uint8_t  reg_value;

    if (((ret = __i2c_devaddr_parse(shell, argv[1], &dev_addr)) < 0) ||
        ((ret = __i2c_regaddr_parse(shell, argv[2], &reg_addr)) < 0) ||
        ((ret = __i2c_regaddr_parse(shell, argv[3], &reg_value)) < 0))
    {
        goto __exit;
    }

    HAL_StatusTypeDef res = HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg_addr, (reg_addr <= 0xFF) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT, (uint8_t*)&reg_value, 1, I2C_TIMEOUT);

    if (res != HAL_OK)
    {
        ret = -1;
        shell_printf(shell, "fail to access 0x%02X\r\n", reg_addr);
        goto __exit;
    }

    shell_printf(shell, "0x%02X\r\n", reg_value);

__exit:

    return ret;

    //-------------------------------------------

__usage:

    // clang-format off
    shell_info(shell,
                "Usage: %s DEV REG VAL\r\n"
                " - DEV: device address, an integer (" TO_STRING(I2C_ADDR_MIN) " - " TO_STRING(I2C_ADDR_MAX) ")\r\n" 
                " - REG: register address, an integer (0x0000 - 0xFFFF)\r\n",
                " - VAL: register new value, an integer (0x00 - 0xFF)\r\n",
                argv[0]);
    // clang-format on

    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), i2cscan, i2c_scan, scan i2c device address);
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), i2cdump, i2c_dump, dump i2c device register);
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), i2cset, i2c_set, set register value);
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), i2cget, i2c_get, get register value);
