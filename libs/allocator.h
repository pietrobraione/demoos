#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#define PAGE_SHIFT 12                   // Page dimension (2^12)
#define TABLE_SHIFT 9                   // Number of entry of the page table (2^9)
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (2 * SECTION_SIZE)   // Kernel memory
#define HIGH_MEMORY 0x3F000000          // IO registers memory

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define N_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PROCESS_SIZE 4096

unsigned long get_free_page();
void free_page(unsigned long p);

#endif
