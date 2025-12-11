#ifndef __ALLOCATOR_H
#define __ALLOCATOR_H

/**
 * Allocatore di pagine: gestisce la memoria fisica del kernel.
 * Fornisce funzioni per allocare/liberare pagine e azzerare blocchi.
 */

// ==============================
// Costanti di paging
// ==============================

#define PAGE_SHIFT		12					// Dimensione pagina (2^12 = 4096 byte)
#define TABLE_SHIFT		9					// Numero entry per tabella (2^9)
#define SECTION_SHIFT	(PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE		(1 << PAGE_SHIFT)
#define SECTION_SIZE	(1 << SECTION_SHIFT)

#define LOW_MEMORY		(2 * SECTION_SIZE)	 // Memoria riservata al kernel
#define HIGH_MEMORY		0x3F000000			 // Memoria periferiche (IO registers)

#define PAGING_MEMORY	(HIGH_MEMORY - LOW_MEMORY)
#define N_PAGES		 	(PAGING_MEMORY / PAGE_SIZE)

#define PROCESS_SIZE	4096				 // Dimensione stack processo

unsigned long get_free_page(void);
void free_page(unsigned long p);
void memzero(unsigned long src, unsigned long n);

#endif // __ALLOCATOR_H

