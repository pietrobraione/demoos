/**
 * Funzioni assembly per operazioni di memoria (memzero, memcpy).
 */

.globl memzero
memzero:
	str		xzr, [x0], #8		// Scrive zero
	subs	x1, x1, #8			// Decrementa contatore
	b.gt	memzero
	ret

.globl memcpy
memcpy:
	ldr	x3, [x1], #8		 // Legge da src
	str	x3, [x0], #8		 // Scrive in dst
	subs	x2, x2, #8
	b.gt	memcpy
	ret

