#ifndef __MM_H
#define __MM_H

#define VA_START            0xffff000000000000

#define PHYS_MEMORY_SIZE    0x40000000

#define PAGE_MASK           0xfffffffffffff000
#define PAGE_SHIFT          12
#define TABLE_SHIFT         9
#define SECTION_SHIFT       (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE           (1 << PAGE_SHIFT)
#define SECTION_SIZE        (1 << SECTION_SHIFT)

#define PGD_SIZE            (3 * PAGE_SIZE)

#define PGD_SHIFT			(PAGE_SHIFT + 3 * TABLE_SHIFT)
#define PUD_SHIFT			(PAGE_SHIFT + 2 * TABLE_SHIFT)
#define PMD_SHIFT			(PAGE_SHIFT + TABLE_SHIFT)

#define LOW_MEMORY          (2 * SECTION_SIZE)   // Kernel memory
#define HIGH_MEMORY         0x3F000000          // IO registers memory

#define PTRS_PER_TABLE	    (1 << TABLE_SHIFT)


#define PAGE_SHIFT 12                   // Page dimension (2^12)
#define TABLE_SHIFT 9                   // Number of entry of the page table (2^9)
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define N_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PROCESS_SIZE 4096

#include "scheduler.h"

#ifndef __ASSEMBLER__

#include <stddef.h>
unsigned long get_free_page();
int free_page(unsigned long p);
void map_page(struct PCB* process, unsigned long virtual_address, unsigned long page);
int map_sector(struct PCB* process, unsigned long start_virtual_address, unsigned long end_virtual_address, unsigned long page_physical_address, unsigned long flags);

int copy_virtual_memory(struct PCB* destination_process);
unsigned long allocate_kernel_page();
unsigned long allocate_user_page(struct PCB* process, unsigned long virtual_address);

void set_pgd(unsigned long pgd);

#endif

#endif
