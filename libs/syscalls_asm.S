/**
 * Wrapper assembly per le system call lato utente.
 * Ogni funzione imposta il numero di syscall in w8 e genera un'eccezione SVC.
 */

#include "syscalls.h"

.globl call_syscall_write
call_syscall_write:
	// Numero syscall in w8
	mov w8, #SYSCALL_WRITE_NUMBER
	// Genera eccezione sincrona
	svc #0
	ret

.globl call_syscall_clone
call_syscall_clone:
	// Salva argomenti del nuovo thread (funzione, argomento, stack)
	mov x10, x0	 // Funzione
	mov x11, x1	 // Argomento
	mov x12, x2	 // Stack

	// Invoca handler clone
	mov x0, x2
	mov w8, #SYSCALL_CLONE_NUMBER
	svc #0
	cmp x0, #0
	// Se ritorna 0 siamo nel nuovo thread
	beq thread_start
	ret

thread_start:
	mov x29, 0		// azzera frame pointer
	mov x0, x11		// argomento
	blr x10			// chiama funzione

	// Al termine invoca exit
	mov w8, #SYSCALL_EXIT_NUMBER
	svc #0

.globl call_syscall_malloc
call_syscall_malloc:
	mov w8, #SYSCALL_MALLOC_NUMBER
	svc #0
	ret

.globl call_syscall_exit
call_syscall_exit:
	mov w8, #SYSCALL_EXIT_NUMBER
	svc #0
	ret

