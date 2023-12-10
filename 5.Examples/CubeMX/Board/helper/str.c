int strlen(char* str)
{
    char* p = str;

    while (*p++)
        ;

    return (int)(p - str);
}

void strcpy(char* dest, char* src)
{
    do {
        *dest++ = *src;
    } while (*src++);
}

int strcmp(char* s1, char* s2)
{
    while (
        (*s1 != 0) &&
        (*s2 != 0) &&
        (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

void memset(char* dest, char data, int len)
{
    while (len--)
    {
        *dest++ = data;
    }
}