/**
 * Implementazione allocatore di pagine.
 * Mantiene un array di bit per tracciare le pagine libere/occupate.
 */

#include "allocator.h"

// ==============================
// Stato memoria
// ==============================

static unsigned short memory_pages[N_PAGES] = {0};

// ==============================
// Allocazione pagina libera
// ==============================

unsigned long get_free_page(void) {
	for (int i = 0; i < N_PAGES; i++) {
		if (memory_pages[i] == 0) {
			memory_pages[i] = 1;
			return LOW_MEMORY + i * PAGE_SIZE;
		}
	}
	return 0; // Nessuna pagina disponibile
}

// ==============================
// Liberazione pagina
// ==============================

void free_page(unsigned long p) {
	memory_pages[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
}

