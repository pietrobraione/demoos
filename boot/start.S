/**
 * Codice di boot: porta i core in EL1, inizializza la BSS e invoca kernel_main.
 * I core non primari vengono fermati in wfe.
 */

	.section ".text.boot"
	.globl _start

#include "../arch/sysregs.h"

_start:
	// Rileva il livello di eccezione corrente (EL)
	mrs	 x0, CurrentEL
	lsr	 x0, x0, #2
	cmp	 x0, #2
	b.ne	already_el1

	// Siamo in EL2: transizione EL2 -> EL1
	mov	 x0, #HCR_VALUE
	msr	 HCR_EL2, x0
	mov	 x0, #SPSR_VALUE
	msr	 SPSR_EL2, x0
	adr	 x0, already_el1
	msr	 ELR_EL2, x0
	eret

already_el1:
	// Configura SCTLR_EL1 con MMU e cache disabilitati
	ldr	 x0, =SCTLR_VALUE_MMU_DISABLED
	msr	 sctlr_el1, x0
	isb

	// Solo il core primario prosegue (basato su MPIDR_EL1)
	mrs	 x5, mpidr_el1
	and	 x5, x5, #3
	cbz	 x5, init_primary_code

wait_for_event:
	wfe
	b		 wait_for_event

init_primary_code:
	// Imposta uno stack provvisorio
	ldr	 x5, =_start
	mov	 sp, x5

	// Pulisce la BSS
	ldr	 x5, =__bss_start
	ldr	 w6, =__bss_size

clear_bss_loop:
	cbz	 w6, kernel_launch
	str	 xzr, [x5], #8
	sub	 w6, w6, #1
	cbnz	w6, clear_bss_loop

kernel_launch:
	// Avvio del kernel
	bl		kernel_main
	b		 wait_for_event

