 
#ifndef __STRING_H__
#define __STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Finds the length of the string
 *
 * @param str - string
 */
size_t strlen(const char* str);

/**
 * @brief Compares two strings
 *
 * @param str1 - first string
 * @param str2 - second string
 */
int strcmp(const char* str1, const char* str2);

/**
 * @brief Copies second string to first string
 *
 * @param str1 - first string
 * @param str2 - second string
 */
char* strcpy(char* str1, const char* str2);

/**
 * @brief Copies second buffer to the first buffer
 *
 * @param dest - pointer to the destination buffer
 * @param src - pointer to the source buffer
 * @param size - number of bytes to be copied
 */
void* memcpy(void* dest, const void* src, size_t size);

/**
 * @brief Sets memory buffer to the given value
 *
 * @param dest - pointer to the memory buffer
 * @param val - Value to set. Only the first byte is considered.
 * @param size - number of bytes to set
 */
void* memset(void* dest, int val, size_t size);

/**
 * @brief Converts provided string to integer
 *
 * @param str string
 */
int atoi(const char* str);

/**
 * @brief Converts provided string to hex
 *
 * @param str string
 */
uint32_t atoh(const char* str);

/**
 * @brief Convert to lowercase letters
 */
int lower(int c);

#ifdef __cplusplus
}
#endif

#endif  // __STRING_H__
