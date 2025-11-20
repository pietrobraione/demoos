    .section ".text.boot"

    .globl _start

#include "../arch/sysregs.h"

_start:
    // Controlla il livello di eccezione (EL)
    // Il controllo viene effettuato prima dell'inizializzazione core,
	// cosi tutti i core vengono portati allo stesso ivello
    mrs x0, CurrentEL
    lsr x0, x0, #2
    cmp x0, #2
    b.ne already_el1

	// Se siamo qui vuol dire che siamo in EL2,
    // passiamo da EL2 --> EL1
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

clear_bss_loop:
    cbz w6, kernel_launch
    str xzr, [x5], #8
    sub     w6, w6, #1
    cbnz    w6, clear_bss_loop

kernel_launch:
    bl  kernel_main
    b   wait_for_event
