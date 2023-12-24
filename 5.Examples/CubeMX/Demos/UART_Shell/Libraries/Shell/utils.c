#include "utils.h"

void ltoa(long num, char* str, int radix)
{
    int           i = 0;
    int           j = 0;
    long          sum;
    unsigned long num1     = num;  // 如果是负数求补码，必须将他的绝对值放在无符号位中在进行求反码
    char          str1[33] = {0};
    if (num < 0)
    {
        // 负数补码
        num  = -num;
        num1 = ~num;
        num1 += 1;
    }
    if (num == 0)
    {
        str1[i] = '0';

        i++;
    }
    while (num1 != 0)
    {
        // 进制运算
        sum     = num1 % radix;
        str1[i] = (sum > 9) ? (sum - 10) + 'a' : sum + '0';
        num1    = num1 / radix;
        i++;
    }
    i--;

    for (i; i >= 0; i--)
    {
        // 逆序输出
        str[i] = str1[j];
        j++;
    }
}

void itoa(int num, char* str, int radix)
{
    int  i = 0;
    int  j = 0;
    int  sum;
    int  num1     = num;  // 如果是负数求补码，必须将他的绝对值放在无符号位中在进行求反码
    char str1[33] = {0};

    if (num < 0)
    {
        // 求负数补码
        num  = -num;
        num1 = ~num;
        num1 += 1;
    }

    if (num == 0)
    {
        str1[i] = '0';

        i++;
    }
    while (num1 != 0)
    {
        // 进制运算
        sum     = num1 % radix;
        str1[i] = (sum > 9) ? (sum - 10) + 'a' : sum + '0';
        num1    = num1 / radix;
        i++;
    }

    i--;

    for (i; i >= 0; i--)
    {
        // 逆序输出
        str[i] = str1[j];
        j++;
    }
}

uint32_t dec2bcd(uint16_t dec)
{
    // Binary-Coded Decimal

    uint32_t result = 0;
    int      shift  = 0;

    while (dec)
    {
        result += (dec % 10) << shift;
        dec = dec / 10;
        shift += 4;
    }

    return result;
}
