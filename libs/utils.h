#ifndef _UTILS_H
#define _UTILS_H

#include <stddef.h>

// Returns the exception level of the CPU
extern int get_el();
// Delays the cpu by the given number of cycles
extern void delay(unsigned long cycles);

#endif
