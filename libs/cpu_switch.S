/**
 * Context switch: salva registri del processo corrente e carica quelli del nuovo.
 */

#include "scheduler.h"

.globl cpu_switch_to_process
cpu_switch_to_process:
	// Calcola offset della cpu_context nella PCB
	mov	x10, #CPU_CONTEXT_OFFSET_IN_PCB
	add	x8, x0, x10	// x8 -> cpu_context del processo corrente
	mov	x9, sp

	// Salva registri callee-saved
	stp	x19, x20, [x8], #16
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	x30, [x8]

	// Carica registri del nuovo processo
	add	x8, x1, x10
	ldp	x19, x20, [x8], #16
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x9, [x8], #16
	ldr	x30, [x8]

	mov	sp, x9
	ret

