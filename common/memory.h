#ifndef _MEMORY_H
#define _MEMORY_H

#include <stddef.h>

/**
 * memory.h defines some functions to manage memory
 */

// Clears the content of a memory address
void memzero(unsigned long src, unsigned long n);
// Compares the content of two memory addresses until the given dimension; returns 0 if the content is the same
int memcmp(const void *src1, const void *src2, size_t n);
// Sets the given memory address value
void memset(void *dest, int c, size_t count);
// Copies the memory content from a source address to a destination address
void memcpy(void *dest, void *src, size_t count);

#endif
