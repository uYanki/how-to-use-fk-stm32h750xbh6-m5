
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
size_t _strlen(const char* str);

/**
 * @brief Compares two strings
 *
 * @param str1 - first string
 * @param str2 - second string
 */
int _strcmp(const char* str1, const char* str2);

/**
 * @brief Copies second string to first string
 *
 * @param str1 - first string
 * @param str2 - second string
 */
char* _strcpy(char* str1, const char* str2);

/**
 * @brief Copies second buffer to the first buffer
 *
 * @param dest - pointer to the destination buffer
 * @param src - pointer to the source buffer
 * @param size - number of bytes to be copied
 */
void* _memcpy(void* dest, const void* src, size_t size);

/**
 * @brief Sets memory buffer to the given value
 *
 * @param dest - pointer to the memory buffer
 * @param val - Value to set. Only the first byte is considered.
 * @param size - number of bytes to set
 */
void* _memset(void* dest, int val, size_t size);

/**
 * @brief Converts provided string to integer
 *
 * @param str string
 */
int _atoi(const char* str);

/**
 * @brief Converts provided string to hex
 *
 * @param str string
 */
uint32_t _atoh(const char* str);

/**
 * @brief Convert to lowercase letters
 */
int _lower(int c);

int32_t _strtol(const char* nPtr, char** endPtr, int base);

#ifdef __cplusplus
}
#endif

#endif  // __STRING_H__
