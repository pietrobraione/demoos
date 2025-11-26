#include "scheduler.h"

.globl cpu_switch_to_process
cpu_switch_to_process:
	// Salvo in X10 il puntatore alla zona dei registri nella struct
    mov    x10, #CPU_CONTEXT_OFFSET_IN_PCB
    add    x8, x0, x10    // x8 contiene il puntatore alla struct registri
    mov    x9, sp
    // Salvo tutti i registri nella struttura del vecchio processo
    stp    x19, x20, [x8], #16
    stp    x21, x22, [x8], #16
    stp    x23, x24, [x8], #16
    stp    x25, x26, [x8], #16
    stp    x27, x28, [x8], #16
    stp    x29, x9, [x8], #16
    str    x30, [x8]
    // Ora faccio puntare x8 alla struct registri del nuovo processo
    add    x8, x1, x10
    // Carico i registri dalla struct del nuovo processo
    ldp    x19, x20, [x8], #16
    ldp    x21, x22, [x8], #16
    ldp    x23, x24, [x8], #16
    ldp    x25, x26, [x8], #16
    ldp    x27, x28, [x8], #16
    ldp    x29, x9, [x8], #16
    // Salvo in x30 il puntatore alla nuova struct
    ldr    x30, [x8]
    mov    sp, x9
    // Ritorna il puntatore del nuovo processo
    ret
