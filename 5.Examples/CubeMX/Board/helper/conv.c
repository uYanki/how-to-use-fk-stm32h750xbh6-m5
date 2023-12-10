#include "stm32h7xx_hal.h"

/**
 * @brief big endian to uint16
 */
uint16_t U16BE(uint8_t* buf)
{
    uint16_t n;

    n = buf[0] << 8;
    n |= buf[1];

    return n;
}

/**
 * @brief little endian to uint16
 */
uint16_t U16LE(uint8_t* buf)
{
    uint16_t n;

    n = buf[1] << 8;
    n |= buf[0];

    return n;
}

/**
 * @brief big endian to uint32
 */
uint32_t U32BE(uint8_t* buf)
{
    uint32_t n;

    n = buf[0] << 24;
    n |= buf[1] << 16;
    n |= buf[2] << 8;
    n |= buf[3];

    return n;
}

/**
 * @brief little endian to uint32
 */
uint32_t U32LE(uint8_t* buf)
{
    uint32_t n;

    n = buf[3] << 24;
    n |= buf[2] << 16;
    n |= buf[1] << 8;
    n |= buf[0];

    return n;
}