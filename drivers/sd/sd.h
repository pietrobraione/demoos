#ifndef __SD_H
#define __SD_H

/**
 * Driver SD/eMMC: inizializzazione, clock, comandi e lettura SCR.
 */

#include "../../libs/mmio.h"

// ==============================
// Versione host
// ==============================

#define HOST_SPEC_NUM		0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3		2
#define HOST_SPEC_V2		1
#define HOST_SPEC_V1		0

// ==============================
// CONTROL register
// ==============================

#define C0_SPI_MODE_EN	0x00100000
#define C0_HCTL_HS_EN	0x00000004
#define C0_HCTL_DWITDH	0x00000002

#define C1_SRST_DATA	0x04000000
#define C1_SRST_CMD		0x02000000
#define C1_SRST_HC		0x01000000
#define C1_TOUNIT_DIS	0x000f0000
#define C1_TOUNIT_MAX	0x000e0000
#define C1_CLK_GENSEL	0x00000020
#define C1_CLK_EN		0x00000004
#define C1_CLK_STABLE	0x00000002
#define C1_CLK_INTLEN	0x00000001

// ==============================
// Errori/return
// ==============================

#define SD_OK 		0
#define SD_TIMEOUT	-1
#define SD_ERROR 	-2

// ==============================
// System timer per wait
// ==============================

#define SYS_TIMER_BASE	0x3F003000
#define SYS_TIMER_CLOCK	(SYS_TIMER_BASE + 0x04)

// ==============================
// STATUS
// ==============================

#define SR_READ_AVAILABLE	0x00000800
#define SR_DAT_INHIBIT		0x00000002
#define SR_CMD_INHIBIT		0x00000001
#define SR_APP_CMD			0x00000020

// ==============================
// INTERRUPT
// ==============================

#define INT_DATA_TIMEOUT	0x00100000
#define INT_CMD_TIMEOUT		0x00010000
#define INT_READ_RDY		0x00000020
#define INT_CMD_DONE		0x00000001

#define INT_ERROR_MASK		0x017E8000

// ==============================
// Flag comandi
// ==============================

#define CMD_NEED_APP	0x80000000
#define CMD_RSPNS_48	0x00020000
#define CMD_ERRORS_MASK	0xfff9c004
#define CMD_RCA_MASK	0xffff0000

// ==============================
// Comandi SD/eMMC
// ==============================

#define CMD_GO_IDLE		 	0x00000000
#define CMD_ALL_SEND_CID	0x02010000
#define CMD_SEND_REL_ADDR	0x03020000
#define CMD_CARD_SELECT	 	0x07030000
#define CMD_SEND_IF_COND	0x08020000
#define CMD_STOP_TRANS		0x0C030000
#define CMD_READ_SINGLE	 	0x11220010
#define CMD_READ_MULTI		0x12220032
#define CMD_SET_BLOCKCNT	0x17020000
#define CMD_APP_CMD		 	0x37000000
#define CMD_SET_BUS_WIDTH	(0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND	(0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR		(0x33220010|CMD_NEED_APP)

// ==============================
// ACMD41/voltaggi
// ==============================

#define ACMD41_VOLTAGE		0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS		0x40000000
#define ACMD41_ARG_HC		0x51ff8000

// ==============================
// SCR
// ==============================

#define SCR_SD_BUS_WIDTH_4	0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
#define SCR_SUPP_CCS		0x00000001

// ==============================
// CONTROL (PL011 duplicate fix)
// ==============================

#define C0_HCTL_DWIDTH		0x00000002

int sd_init(void);

#endif // __SD_H

