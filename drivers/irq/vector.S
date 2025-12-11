/**
 * Inizializzazione della base dei vettori e maschere IRQ.
 */

	.globl irq_vector_init
irq_vector_init:
	// Carica VBAR_EL1 con l’indirizzo della tabella dei vettori
	adr	 x0, vectors
	msr	 vbar_el1, x0
	ret

	.globl enable_irq
enable_irq:
	// Abilita IRQ (pulisce il bit I nel DAIF)
	msr	 daifclr, #2
	ret

	.globl disable_irq
disable_irq:
	// Disabilita IRQ (imposta il bit I nel DAIF)
	msr	 daifset, #2
	ret

