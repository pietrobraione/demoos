#include "allocator.h"

static unsigned short memory_pages[N_PAGES] = {0};

unsigned long get_free_page() {
    for (int i = 0; i < N_PAGES; i++) {
        if (memory_pages[i] == 0) {
            memory_pages[i] = 1;
            return LOW_MEMORY + i * PAGE_SIZE;
        }
    }
    return 0;
}
