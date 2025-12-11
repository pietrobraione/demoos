#ifndef __ENTRY_H
#define __ENTRY_H

/**
 * Definizioni per frame di stack salvato e codici di entry non valide.
 * Utilizzato da entry.S per salvataggio/ripristino contesto e diagnosi.
 */

// ==============================
// Layout del frame salvato
// ==============================

#define S_FRAME_SIZE			272 // dimensione totale del frame
#define S_X0					0	// offset di x0 nel frame

// ==============================
// Codici di entry non valida (logging)
// ==============================

#define SYNC_INVALID_EL1t		0
#define IRQ_INVALID_EL1t		1
#define FIQ_INVALID_EL1t		2
#define ERROR_INVALID_EL1t		3

#define SYNC_INVALID_EL1h		4
#define IRQ_INVALID_EL1h		5
#define FIQ_INVALID_EL1h		6
#define ERROR_INVALID_EL1h		7

#define SYNC_INVALID_EL0_64		8
#define IRQ_INVALID_EL0_64		9
#define FIQ_INVALID_EL0_64		10
#define ERROR_INVALID_EL0_64	11

#define SYNC_INVALID_EL0_32		12
#define IRQ_INVALID_EL0_32		13
#define FIQ_INVALID_EL0_32		14
#define ERROR_INVALID_EL0_32	15

#define SYNC_ERROR				16
#define SYSCALL_ERROR			17

#ifndef __ASSEMBLER__
void ret_from_fork(void);
#endif

#endif // __ENTRY_H

