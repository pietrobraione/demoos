/**
 * Vettori di eccezione e macro di salvataggio/ripristino contesto.
 * Gestisce IRQ in EL1/EL0 e syscall (SVC) in EL0.
 */

#include "entry.h"
#include "../../libs/syscalls.h"
#include "../../arch/sysregs.h"

	// ==============================
	// Macro: gestione entry non valide
	// ==============================
	.macro handle_invalid_entry el, type
		kernel_entry \el
		mov	x0, #\type
		mrs	x1, esr_el1
		mrs	x2, elr_el1
		bl	show_invalid_entry_message
		b	err_hang
	.endm

	.macro ventry label
		.align  7
		b	\label
	.endm

	// ==============================
	// Macro: salvataggio contesto (entry)
	// ==============================
	.macro kernel_entry, el
		sub	 sp, sp, #S_FRAME_SIZE

		stp	 x0,  x1,  [sp, #16 * 0]
		stp	 x2,  x3,  [sp, #16 * 1]
		stp	 x4,  x5,  [sp, #16 * 2]
		stp	 x6,  x7,  [sp, #16 * 3]
		stp	 x8,  x9,  [sp, #16 * 4]
		stp	 x10, x11, [sp, #16 * 5]
		stp	 x12, x13, [sp, #16 * 6]
		stp	 x14, x15, [sp, #16 * 7]
		stp	 x16, x17, [sp, #16 * 8]
		stp	 x18, x19, [sp, #16 * 9]
		stp	 x20, x21, [sp, #16 * 10]
		stp	 x22, x23, [sp, #16 * 11]
		stp	 x24, x25, [sp, #16 * 12]
		stp	 x26, x27, [sp, #16 * 13]
		stp	 x28, x29, [sp, #16 * 14]

		// Stack separato per EL0
		.if \el == 0
			mrs	 x21, sp_el0
		.else
			add	 x21, sp, #S_FRAME_SIZE
		.endif

		mrs	 x22, elr_el1
		mrs	 x23, spsr_el1

		stp	 x30, x21, [sp, #16 * 15]
		stp	 x22, x23, [sp, #16 * 16]
	.endm

	// ==============================
	// Macro: ripristino contesto (exit)
	// ==============================
	.macro kernel_exit, el
		ldp	 x22, x23, [sp, #16 * 16]
		ldp	 x30, x21, [sp, #16 * 15]

		.if \el == 0
			msr	 sp_el0, x21
		.endif

		msr	 elr_el1,  x22
		msr	 spsr_el1, x23

		ldp	 x0,  x1,  [sp, #16 * 0]
		ldp	 x2,  x3,  [sp, #16 * 1]
		ldp	 x4,  x5,  [sp, #16 * 2]
		ldp	 x6,  x7,  [sp, #16 * 3]
		ldp	 x8,  x9,  [sp, #16 * 4]
		ldp	 x10, x11, [sp, #16 * 5]
		ldp	 x12, x13, [sp, #16 * 6]
		ldp	 x14, x15, [sp, #16 * 7]
		ldp	 x16, x17, [sp, #16 * 8]
		ldp	 x18, x19, [sp, #16 * 9]
		ldp	 x20, x21, [sp, #16 * 10]
		ldp	 x22, x23, [sp, #16 * 11]
		ldp	 x24, x25, [sp, #16 * 12]
		ldp	 x26, x27, [sp, #16 * 13]
		ldp	 x28, x29, [sp, #16 * 14]

		add	 sp, sp, #S_FRAME_SIZE
		eret
	.endm

	// ==============================
	// Tabella dei vettori
	// ==============================
	.align  11
	.globl  vectors
vectors:
	ventry  sync_invalid_el1t
	ventry  irq_invalid_el1t
	ventry  fiq_invalid_el1t
	ventry  error_invalid_el1t

	ventry  sync_invalid_el1h
	ventry  el1_irq
	ventry  fiq_invalid_el1h
	ventry  error_invalid_el1h

	ventry  el0_sync
	ventry  el0_irq
	ventry  fiq_invalid_el0_64
	ventry  error_invalid_el0_64

	ventry  sync_invalid_el0_32
	ventry  irq_invalid_el0_32
	ventry  fiq_invalid_el0_32
	ventry  error_invalid_el0_32

	// Entry non valide (log + hang)
sync_invalid_el1t:	handle_invalid_entry  1, SYNC_INVALID_EL1t
irq_invalid_el1t:	handle_invalid_entry  1, IRQ_INVALID_EL1t
fiq_invalid_el1t:	handle_invalid_entry  1, FIQ_INVALID_EL1t
error_invalid_el1t:	handle_invalid_entry  1, ERROR_INVALID_EL1t

sync_invalid_el1h:	handle_invalid_entry  1, SYNC_INVALID_EL1h
fiq_invalid_el1h:	handle_invalid_entry  1, FIQ_INVALID_EL1h
error_invalid_el1h: handle_invalid_entry  1, ERROR_INVALID_EL1h

sync_invalid_el0_64:	handle_invalid_entry  0, SYNC_INVALID_EL0_64
irq_invalid_el0_64:	handle_invalid_entry  0, IRQ_INVALID_EL0_64
fiq_invalid_el0_64:	handle_invalid_entry  0, FIQ_INVALID_EL0_64
error_invalid_el0_64:	handle_invalid_entry  0, ERROR_INVALID_EL0_64

sync_invalid_el0_32:	handle_invalid_entry  0, SYNC_INVALID_EL0_32
irq_invalid_el0_32:	handle_invalid_entry  0, IRQ_INVALID_EL0_32
fiq_invalid_el0_32:	handle_invalid_entry  0, FIQ_INVALID_EL0_32
error_invalid_el0_32:	handle_invalid_entry  0, ERROR_INVALID_EL0_32

	// IRQ in EL1
el1_irq:
	kernel_entry	1
	bl	handle_irq
	kernel_exit	1

	// IRQ in EL0
el0_irq:
	kernel_entry	0
	bl	handle_irq
	kernel_exit	0

	// Eccezioni sincrone in EL0 (SVC/syscall)
el0_sync:
	kernel_entry	0
	mrs	x25, esr_el1	// legge sindrome
	lsr	x24, x25, #ESR_ELx_EC_SHIFT   // estrae EC
	cmp	x24, #ESR_ELx_EC_SVC64	// SVC 64-bit?
	b.eq	el0_svc
	handle_invalid_entry 0, SYNC_ERROR

	// Alias per syscall
sc_nr	.req x25
scno	.req x26
stbl	.req x27

	// Gestione SVC -> syscall
el0_svc:
	adr	stbl, sys_call_table	// tabella syscall
	uxtw	scno, w8	// numero syscall
	mov	sc_nr, #__NR_SYSCALLS
	bl	 enable_irq	// abilita IRQ durante la syscall
	cmp	scno, sc_nr
	b.hs	ni_sys

	ldr	x16, [stbl, scno, lsl #3]	 // handler
	blr	x16	// esegui
	b	ret_from_syscall

ni_sys:
	handle_invalid_entry 0, SYSCALL_ERROR

ret_from_syscall:
	bl	 disable_irq
	str	x0, [sp, #S_X0]	 // salva retval
	kernel_exit	  0

	// Hang in caso di errore
	.globl  err_hang
err_hang:
	b	err_hang

	// Ritorno da fork (spazio utente)
	.globl  ret_from_fork
ret_from_fork:
	bl	 schedule_tail
	cbz	x19, ret_to_user
	mov	x0, x20
	blr	x19

ret_to_user:
	bl	 disable_irq
	kernel_exit	  0

