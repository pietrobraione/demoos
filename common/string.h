#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

/**
 * string.h defines some functions to manipulate strings
 */

extern size_t strlen(const char* str);
extern char* strcat(char* dest, const char* src);
extern char* strcpy(char* dest, const char* src);
extern void strsplit(const char* src, char delimiter, char* dest1, char* dest2);
extern int strcmp(const char* src1, const char* src2, int size);

#endif