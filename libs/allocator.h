#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#define PAGE_SHIFT 12                   // Page dimension (2^12)
#define TABLE_SHIFT 9                   // Number of entry of the page table (2^9)
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define N_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PROCESS_SIZE 4096

#include <stddef.h>
#include "scheduler.h"
#include "mm.h"

unsigned long get_free_page();
int free_page(unsigned long p);
void map_page(struct PCB* process, unsigned long virtual_address, unsigned long page);
int map_sector(struct PCB* process, unsigned long start_virtual_address, unsigned long end_virtual_address, unsigned long page_physical_address, unsigned long flags);

int copy_virtual_memory(struct PCB* destination_process);
unsigned long allocate_kernel_page();
unsigned long allocate_user_page(struct PCB* process, unsigned long virtual_address);

void set_pgd(unsigned long pgd);

unsigned long user_to_kernel_address(unsigned long user_virtual_address);

#endif
