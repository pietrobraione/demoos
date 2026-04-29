#ifndef __FRAMEBUFFER_H
#define __FRAMEBUFFER_H

/**
 * Driver framebuffer: inizializzazione tramite mailbox e stampa con font PSF.
 */

int framebuffer_init(void);
void framebuffer_print(int x, int y, char *s);

#endif // __FRAMEBUFFER_H

