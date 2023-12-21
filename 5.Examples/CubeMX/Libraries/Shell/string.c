#include "string.h"

#include <string.h>
#include <ctype.h>
#include <errno.h>   //for returning error codes to compare with test_strtol
#include <limits.h>  //for INT32_MAX & INT32_MIN
#include <stdbool.h>

size_t _strlen(const char* str)
{
    const char* p = str;

    for (; *p; ++p)
        ;

    return p - str;
}

int _strcmp(const char* str1, const char* str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }

    return *str1 - *str2;
}

char* _strcpy(char* str1, const char* str2)
{
    char* p = str1;

    while ((*str1 = *str2))
    {
        str1++;
        str2++;
    }

    return p;
}

int32_t _strtol(const char* nPtr, char** endPtr, int base)
{
    // checking if the base value is correct
    if ((base < 2 || base > 36) && base != 0)
    {
        errno = EINVAL;
        return 0;
    }

    const char* divider;

    int32_t number = 0;
    int     currentdigit, sign, cutlim;

    enum sign {
        NEGATIVE,
        POSITIVE,
    };

    uint32_t cutoff;

    bool correctconversion = true;

    divider = nPtr;

    // looking for a space if the beggining of the string is moved further
    while (isspace(*divider))
    {
        divider++;
    }

    // detecting the sign, positive by default
    if (*divider == '+')
    {
        sign = POSITIVE;
        divider++;
    }
    else if (*divider == '-')
    {
        sign = NEGATIVE;
        divider++;
    }
    else
    {
        sign = POSITIVE;
    }

    if (*divider == NULL)
    {
        *endPtr = (char*)divider;
        return 0;
    }

    if (*divider < '0' || (*divider > '9' && *divider < 'A') || (*divider > 'z'))
    {
        return 0;
    }

    if ((base == 8) && (*divider == '0'))
    {
        divider++;

        if (*divider == 'o' || *divider == 'O')  // if the input includes 'o', it's skipped
        {
            divider++;
        }
    }
    else if ((base == 16))
    {
        if (*divider == '0')
        {
            divider++;

            if (*divider == 'x' || *divider == 'X')
            {
                divider++;

                if (*divider > 'f' || *divider > 'F')
                {
                    divider--;
                    *endPtr = (char*)divider;

                    return 0;
                }
            }
            else
            {
                divider--;
            }
        }
        // basically the system-detecting algorithm
    }
    else if (base == 0)
    {
        if (*divider == '0')
        {
            divider++;

            if (*divider == 'o' || *divider == 'O')  // oct
            {
                base = 8;
                divider++;

                if (*divider > '7')
                {
                    divider--;
                    *endPtr = (char*)divider;

                    return 0;
                }
            }
            else if (*divider == 'x' || *divider == 'X')  // hex
            {
                base = 16;
                divider++;

                if (*divider > 'f' || *divider > 'F')
                {
                    divider--;
                    *endPtr = (char*)divider;

                    return 0;
                }
            }
            else if (*divider <= '7')  // oct
            {
                base = 8;
            }
            else
            {
                *endPtr = (char*)divider;
                return 0;
            }
        }
        else if (*divider >= '1' && *divider <= '9')  // dec
        {
            base = 10;
        }
    }

    // two conditions just for clarity --> |INT32_MIN| = INT32_MAX + 1
    cutoff = (sign ? INT32_MAX : (uint32_t)INT32_MIN) / (uint32_t)base;
    cutlim = cutoff % (uint32_t)base;

    // looping until the end of the input string
    // searching for convertable characters
    while (*divider != NULL)
    {
        if (isdigit(*divider))
        {
            currentdigit = *divider - '0';  // converting to the actual integer
        }
        else
        {
            if (isalpha(*divider))
            {
                if (islower(*divider) && (*divider - 'a') + 10 < base)
                {
                    currentdigit = (*divider - 'a') + 10;
                }
                else if (!islower(*divider) && (*divider - 'A') + 10 < base)
                {
                    currentdigit = (*divider - 'A') + 10;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if (!correctconversion ||
            number > cutoff ||
            (number == cutoff && (int)currentdigit > cutlim))
        {
            correctconversion = false;
            divider++;
        }
        else
        {  // the actual conversion to decimal
            correctconversion = true;
            number            = (number * base) + currentdigit;
            divider++;
        }
    }

    if (!correctconversion)
    {
        number = sign ? INT32_MAX : INT32_MIN;
        errno  = ERANGE;
    }

    if (sign == NEGATIVE)
    {
        number *= -1;
    }

    if (endPtr != NULL)
    {
        if (isspace(*divider))  // checking if the number is separated
        {
            divider++;  // from the rest of the string
        }
        *endPtr = (char*)divider;
    }

    return number;
}

void* _memcpy(void* dest, const void* src, size_t size)
{
    char* p = (char*)src;

    char* p_src  = (char*)src;
    char* p_dest = (char*)dest;

    while (size--)
    {
        *p_dest++ = *p_src++;
    }

    return p;
}

void* _memset(void* dest, int val, size_t size)
{
    char* p = (char*)dest;

    while (size--)
    {
        *p++ = val;
    }

    return dest;
}

int _atoi(const char* str)
{
    int res = 0;

    while (*str)
    {
        res *= 10;
        res += *str - '0';
        ++str;
    }

    return res;
}

uint32_t _atoh(const char* str)
{
    uint32_t hex = 0;
    uint32_t val = 0;

    while (*str)
    {
        if (*str >= '0' && *str <= '9')
        {
            val = *str - '0';
        }
        else if (*str >= 'a' && *str <= 'f')
        {
            val = *str - 'a' + 10;
        }
        else if (*str >= 'A' && *str <= 'F')
        {
            val = *str - 'A' + 10;
        }
        else
        {
            break;
        }

        ++str;

        // make space for the new nibble on the right
        hex = hex << 4;
        hex |= val;
    }

    return hex;
}

int _lower(int c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return c + ('a' - 'A');
    }

    return c;
}
