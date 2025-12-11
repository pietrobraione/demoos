#ifndef __UTILS_H
#define __UTILS_H

/**
 * Funzioni di utilità assembly: livello di eccezione corrente,
 * delay busy-wait e attesa millisecondi.
 */

int get_el(void);
void delay(unsigned long cycles);
void wait_msec(unsigned int ms);

#endif // __UTILS_H

