
#include "string.h"

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

int strcmp(const char* str1, const char* str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

char* strcpy(char* str1, const char* str2)
{
    while (*str2)
    {
        *(str1++) = *(str2++);
    }
    *str1 = '\0';
    return 0;
}

void* memcpy(void* dest, const void* src, size_t size)
{
    size_t i;
    char*  src_char  = (char*)src;
    char*  dest_char = (char*)dest;
    for (i = 0; i < size; i++)
    {
        dest_char[i] = src_char[i];
    }
    return 0;
}

void* memset(void* dest, int val, size_t size)
{
    size_t i;
    char*  dest_char = (char*)dest;
    for (i = 0; i < size; i++)
    {
        dest_char[i] = val;
    }

    return 0;
}

int atoi(const char* str)
{
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i)
    {
        res = res * 10 + str[i] - '0';
    }
    return res;
}

uint32_t atoh(const char* str)
{
    uint32_t hex   = 0;
    uint32_t value = 0;

    for (uint32_t i = 0; str[i] != '\0'; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            value = str[i] - '0';
        }
        else if (str[i] >= 'a' && str[i] <= 'f')
        {
            value = str[i] - 'a' + 10;
        }
        else if (str[i] >= 'A' && str[i] <= 'F')
        {
            value = str[i] - 'A' + 10;
        }
        else
        {
            continue;
        }

        // make space for the new nibble on the right
        hex = hex << 4;
        hex |= value;
    }

    return hex;
}

int lower(int c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return c + ('a' - 'A');
    }

    return c;
}
