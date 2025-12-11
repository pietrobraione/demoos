#ifndef __SYSREGS_H
#define __SYSREGS_H

/**
 * Definizioni dei system registers AArch64 utilizzati nel boot e nella gestione
 * delle eccezioni (EL1/EL2/EL3).
 *
 * Riferimenti: ARM ARM (AArch64) per SCTLR_EL1, HCR_EL2, SCR_EL3, SPSR_EL3, ESR_EL1.
 */

// ==============================
// SCTLR_EL1 (System Control Register)
// ==============================

#define SCTLR_RESERVED					((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11))
#define SCTLR_EE_LITTLE_ENDIAN			(0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN			(0 << 24)
#define SCTLR_I_CACHE_DISABLED			(0 << 12)
#define SCTLR_D_CACHE_DISABLED			(0 << 2)
#define SCTLR_MMU_DISABLED				(0 << 0)
#define SCTLR_MMU_ENABLED				(1 << 0)

#define SCTLR_VALUE_MMU_DISABLED \
	(SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | \
	 SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// ==============================
// HCR_EL2 (Hypervisor Configuration Register)
// ==============================

#define HCR_RW							(1 << 31)
#define HCR_VALUE						HCR_RW

// ==============================
// SCR_EL3 (Secure Configuration Register)
// ==============================

#define SCR_RESERVED					(3 << 4)
#define SCR_RW							(1 << 10)
#define SCR_NS							(1 << 0)
#define SCR_VALUE						(SCR_RESERVED | SCR_RW | SCR_NS)

// ==============================
// SPSR_EL3 (Saved Program Status Register)
// ==============================

#define SPSR_MASK_ALL					(7 << 6)
#define SPSR_EL1h						(5 << 0)
#define SPSR_VALUE						(SPSR_MASK_ALL | SPSR_EL1h)

// ==============================
// ESR_EL1 (Exception Syndrome Register)
// ==============================

#define ESR_ELx_EC_SHIFT				26
#define ESR_ELx_EC_SVC64				0x15

#endif // __SYSREGS_H

