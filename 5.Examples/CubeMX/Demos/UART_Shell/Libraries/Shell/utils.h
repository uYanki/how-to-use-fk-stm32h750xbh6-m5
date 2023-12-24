#ifndef __SHELL_UTILS_H__
#define __SHELL_UTILS_H__

#include <stdint.h>

void     ltoa(long num, char* str, int radix);
void     itoa(int num, char* str, int radix);
uint32_t dec2bcd(uint16_t dec);

#endif
