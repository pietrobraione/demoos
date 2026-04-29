    .section ".text.boot"

    .globl _start

#include "../arch/sysregs.h"
#include "../arch/peripherals/base.h"
#include "../arch/mmu.h"
#include "../libs/mm.h"

_start:
    // Checks the exception level of the CPU to make it EL1
    mrs x0, CurrentEL
    lsr x0, x0, #2
    cmp x0, #2
    b.ne already_el1

    // If it's not already EL1, I change it
	mov x0, #HCR_VALUE
	msr HCR_EL2, x0
	mov x0, #SPSR_VALUE
	msr SPSR_EL2, x0
	adr x0, already_el1
	msr ELR_EL2, x0
	eret

already_el1:
    ldr	x0, =SCTLR_VALUE_MMU_DISABLED
	msr	sctlr_el1, x0
    isb

    mrs     x5, mpidr_el1
    and x5, x5, #3
    cbz x5, init_primary_code

wait_for_event:
    wfe
    b   wait_for_event

init_primary_code:
    ldr     x5, =_start
    mov sp, x5
    ldr x5, =__bss_start
    ldr w6, =__bss_size

clear_bss:
    and x0, x0, x0
    adr x0, __bss_start
    adr x1, __bss_end
    sub x1, x1, x0
    bl memzero

el1_entry:
    bl __create_page_tables

    // I change the SP to point to the first virtual address of the kernel
    mov     x0, #VA_START
    add     sp, x0, #LOW_MEMORY

    // I store the pgd filled previously in its register
    adrp    x0, pgd
    msr     ttbr1_el1, x0
    msr     ttbr0_el1, x0

    // Then I store some configuration registers from the MMU
    ldr     x0, =(TCR_VALUE)
    msr     tcr_el1, x0

    ldr     x0, =(MAIR_VALUE)
    msr     mair_el1, x0

    // I store the physical address of the kernel main before activating the
    // MMU so I keep the physical address and not the virtual one
    ldr     x2, =kernel_main

enable_mmu:
    // I enable the MMU
    mov     x0, #SCTLR_MMU_ENABLED
    msr     sctlr_el1, x0

    // Then I jump to the kernel_main using x2, in which there is the physical address of the function
    br      x2

    // This macro is a wrapper which calls all the macros to create directly the
    // entry both in PGD and in PUD
    .macro  create_pgd_entry, tbl, virt, tmp1, tmp2
    create_table_entry \tbl, \virt, PGD_SHIFT, \tmp1, \tmp2
    create_table_entry \tbl, \virt, PUD_SHIFT, \tmp1, \tmp2
    .endm

    // This macro writes the entry which points to the next table, in the given
    // table for the given virtual address
    .macro  create_table_entry, tbl, virt, shift, tmp1, tmp2
    // First I extract the index in the table inside tmp1
    lsr     \tmp1, \virt, #\shift
    and     \tmp1, \tmp1, #PTRS_PER_TABLE - 1
    // I calculate the address of the next page table (current table + 1 page)
    add     \tmp2, \tbl, #PAGE_SIZE
    // I put the last 2 bits to 1:
    // - First bit is the validity bit
    // - Second bit is the one which says that the entry points to another table
    //   and not to a memory address
    orr     \tmp2, \tmp2, #MM_TYPE_PAGE_TABLE
    // Then I write the entry in the table
    str     \tmp2, [\tbl, \tmp1, lsl #3]
    // And I change the tbl to point to the next table
    add     \tbl, \tbl, #PAGE_SIZE
    .endm

    // This macro maps the given interval of virtual addresses in the PMD table
    .macro  create_block_map, tbl, phys, start, end, flags, tmp1
    // First I extract the first and last table indexes
    lsr     \start, \start, #SECTION_SHIFT
    and     \start, \start, #PTRS_PER_TABLE - 1
    lsr     \end,   \end,   #SECTION_SHIFT
    and     \end,   \end,   #PTRS_PER_TABLE - 1
    // Then I calculate the descriptor: I remove the first 21 bits from right by
    // performing a double shift, then I or this with the flags and I obtain the
    // descriptor
    lsr     \phys, \phys, #SECTION_SHIFT
    mov     \tmp1, #\flags
    orr     \phys, \tmp1, \phys, lsl #SECTION_SHIFT

    // Then I loop to write an entry for each sector in the given interval
    9999:   str     \phys, [\tbl, \start, lsl #3]
            add     \start, \start, #1
            add     \phys, \phys, #SECTION_SIZE
            cmp     \start, \end
            b.ls    9999b
    .endm

__create_page_tables:
    // I save the return address because I'll call other procedures
    mov     x29, x30

    // I clear the memory where the PGD will be stored
    adrp    x0, pgd
    mov     x1, #PGD_SIZE
    bl memzero

    // The kernel will have the entire 1GB space completely mapped;
    // it only needs PGD, PUD and PMD; PMD will directly point to the phisical
    // address, because it can cover the whole 1GB space

    // I create the entry for the PGD and PUD for the first virtual address
    adrp    x0, pgd
    mov     x1, #VA_START
    create_pgd_entry x0, x1, x2, x3

    // First I map the entire memory of the kernel

    // In x0 there will be the PTE address, because it has been incremented
    // during the create_pgd_entry
    // In x1 there will be the physical address to map
    mov x1, xzr
    // In x2 there will be first virtual address to map
    mov x2, #VA_START
    // In x3 there will be the last virtual address to map
    // (until DEVICE_BASE, which is the first address for MMIO)
    ldr x3, =(VA_START + DEVICE_BASE - SECTION_SIZE)
    // Then I can call the macro passing also the flags
    create_block_map x0, x1, x2, x3, MMU_FLAGS, x4

    // Then I write the memory section for the IO devices with a different flag
    // In x1 I put the first address of the devices addresses
    mov x1, #DEVICE_BASE
    // In x2 I put the first virtual address
    ldr x2, =(VA_START + DEVICE_BASE)
    // In x2 I put the last virtual address
    ldr x3, =(VA_START + PHYS_MEMORY_SIZE - SECTION_SIZE)
    // Then I can call the macro passing also the flags
    create_block_map x0, x1, x2, x3, MMU_DEVICE_FLAGS, x4

    /*
    This is an identity mapping which was used when the kernel was compiled with low virtual 
    addresses. I don't want to remove it because maybe it will be useful. 

    // Since the kernel is put in the low memory, I create the identity map
    // for the low addresses to make the kernel able to map the 0x80000 addresses
    adrp    x0, pgd
    mov     x1, xzr
    create_pgd_entry x0, x1, x2, x3

    mov x1, xzr
    mov x2, xzr
    mov x3, #0x200000
    create_block_map x0, x1, x2, x3, MMU_FLAGS, x4
    */

    // I restore the original return address and return to el1_entry
    mov x30, x29
    ret

kernel_launch:
    bl  kernel_main
    b   wait_for_event
