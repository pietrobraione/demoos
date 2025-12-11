/**
 * Funzioni assembly di utilità.
 */

.globl get_el
get_el:
	// Restituisce livello di eccezione corrente (EL)
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret

.globl delay
delay:
	// Busy-wait decrementando contatore
	subs x0, x0, #1
	bne	delay
	ret

