/**
 * Driver SD/eMMC per Raspberry Pi: gestione GPIO, clock, comandi e lettura SCR.
 * Nota: QEMU non implementa il controller eMMC, alcuni percorsi potrebbero non progredire.
 */

#include "sd.h"
#include "../../libs/utils.h"
#include "../uart/uart.h"

// ==============================
// Forward declarations interne
// ==============================

void enable_card_detect(void);
void enable_clock_and_command(void);
void enable_data_pins(void);
void reset_emmc(void);
int	sd_set_clock(unsigned int);
int	sd_execute_command(unsigned int, unsigned int);
int	sd_status(unsigned int);
int	sd_wait_for_interrupt(unsigned int);
int	sd_enable_card(void);
long sd_get_rca(void);
void wait_msec(unsigned int n);

// ==============================
// Stato SD
// ==============================

long sd_standard_version, ccs;
unsigned long sd_scr[2], sd_rca, sd_error;

// ==============================
// Inizializzazione SD
// ==============================

int sd_init(void) {
	enable_card_detect();
	enable_clock_and_command();
	enable_data_pins();

	sd_standard_version = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

	reset_emmc();

	*EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
	wait_msec(10);

	if (sd_set_clock(400000) == SD_ERROR) {
		return SD_ERROR;
	}

	// Abilita gli interrupt del controller eMMC
	*EMMC_INT_EN	= 0xFFFFFFFF;
	*EMMC_INT_MASK= 0xFFFFFFFF;

	sd_scr[0] = sd_scr[1] = sd_rca = sd_error = 0;

	// Sequenza standard di avvio SD
	sd_execute_command(CMD_GO_IDLE, 0);
	if (sd_error) return sd_error;

	sd_execute_command(CMD_SEND_IF_COND, 0x1AA);
	if (sd_error) return sd_error;

	int enable_result = sd_enable_card();
	if (enable_result == SD_ERROR) {
		return SD_ERROR;
	}

	// Legge CID
	sd_execute_command(CMD_ALL_SEND_CID, 0);

	// Ottiene RCA
	long rca = sd_execute_command(CMD_SEND_REL_ADDR, 0);
	uart_hex(rca >> 32);
	uart_hex(rca);
	uart_puts("\n");
	if (sd_error) return sd_error;

	long response;
	if ((response = sd_set_clock(25000000))) {
		return response;
	}

	// Seleziona la card tramite RCA
	sd_execute_command(CMD_CARD_SELECT, sd_rca);
	if (sd_error) return sd_error;

	if (sd_status(SR_DAT_INHIBIT)) {
		return SD_TIMEOUT;
	}

	// Richiede 1 blocco di 8 byte (SCR)
	*EMMC_BLKSIZECNT = (1 << 16) | 8;
	sd_execute_command(CMD_SEND_SCR, 0);
	if (sd_error) return sd_error;

	if (sd_wait_for_interrupt(INT_READ_RDY)) {
		return SD_TIMEOUT;
	}

	// Legge SCR (2 parole)
	response = 0;
	long counter = 100000;
	while (response < 2 && counter) {
		if (*EMMC_STATUS & SR_READ_AVAILABLE) {
			sd_scr[response++] = *EMMC_DATA;
		} else {
			wait_msec(1);
		}
		counter--;
	}

	if (response != 2) return SD_TIMEOUT;

	// Imposta bus width 4 se supportato
	if (sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
		sd_execute_command(CMD_SET_BUS_WIDTH, sd_rca | 2);
		if (sd_error) return sd_error;
		*EMMC_CONTROL0 |= C0_HCTL_DWIDTH;
	}

	sd_scr[0] &= ~SCR_SUPP_CCS;
	sd_scr[0] |= ccs;

	return SD_OK;
}

// ==============================
// Configurazione GPIO: card detect
// ==============================

void enable_card_detect(void) {
	// GPIO47 input: gruppo 4 (40-49), posizione 7 (3 bit ciascuno)
	long r = *GPFSEL4;
	r &= ~(7 << (7 * 3));
	*GPFSEL4 = r;

	// Pull-up per stabilizzare stato
	*GPPUD = 2;
	delay(150);
	*GPPUDCLK1 = (1 << 15);
	delay(150);
	*GPPUD = 0;
	*GPPUDCLK1 = 0;

	// High detect: abilita interrupt quando il pin resta alto
	r = *GPHEN1;
	r |= 1 << 15;
	*GPHEN1 = r;
}

// ==============================
// Configurazione GPIO: clock e command
// ==============================

void enable_clock_and_command(void) {
	// GPIO48 -> clock, GPIO49 -> command (gruppo 4, posizioni 8 e 9)
	long r = *GPFSEL4;
	r |= (7 << (8 * 3)) | (7 << (9 * 3));
	*GPFSEL4 = r;

	*GPPUD = 2;
	delay(150);
	*GPPUDCLK1 = (1 << 16) | (1 << 17);
	delay(150);
	*GPPUD = 0;
	*GPPUDCLK1 = 0;
}

// ==============================
// Configurazione GPIO: data pins (DAT0..DAT3)
// ==============================

void enable_data_pins(void) {
	long r = *GPFSEL5;
	r |= (7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3));
	*GPFSEL5 = r;

	*GPPUD = 2;
	delay(150);
	*GPPUDCLK1 |= (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
	delay(150);
	*GPPUD = 0;
	*GPPUDCLK1 = 0;
}

// ==============================
// Reset controller eMMC
// ==============================

void reset_emmc(void) {
	*EMMC_CONTROL0 = 0;
	*EMMC_CONTROL1 |= C1_SRST_HC;

	// Attende reset (su QEMU potrebbe non azzerarsi)
	while (*EMMC_CONTROL1 & C1_SRST_HC) { }
}

// ==============================
// Clock SD/eMMC
// ==============================

int sd_set_clock(unsigned int frequency) {
	int counter = 100000;

	// Attende che gli inhibit si azzerino
	while (*EMMC_STATUS & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) {
		counter--;
		wait_msec(1);
		if (counter <= 0) return SD_ERROR;
	}

	*EMMC_CONTROL1 &= ~C1_CLK_EN;
	wait_msec(10);

	unsigned int clock = 41666666 / frequency;
	unsigned int x, s = 32;

	x = clock - 1;
	if (!x) {
		s = 0;
	} else {
		if (!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
		if (!(x & 0xff000000u)) { x <<= 8;	s -= 8; }
		if (!(x & 0xf0000000u)) { x <<= 4;	s -= 4; }
		if (!(x & 0xc0000000u)) { x <<= 2;	s -= 2; }
		if (!(x & 0x80000000u)) { x <<= 1;	s -= 1; }
		if (s > 0) { s--; }
		if (s > 7) { s = 7; }
	}

	unsigned int d;
	if (sd_standard_version > HOST_SPEC_V2) {
		d = clock;
	} else {
		d = (1 << s);
	}

	if (d <= 2) {
		d = 2;
		s = 0;
	}

	unsigned int h = 0;
	if (sd_standard_version > HOST_SPEC_V2) {
		h = (d & 0x300) >> 2;
	}
	d = (((d & 0x0FF) << 8) | h);

	*EMMC_CONTROL1 = (*EMMC_CONTROL1 & 0xffff003f) | d;
	wait_msec(10);

	counter = 10000;
	while (!(*EMMC_CONTROL1 & C1_CLK_STABLE)) {
		wait_msec(10);
		counter--;
		if (counter <= 0) return SD_ERROR;
	}

	return SD_OK;
}

// ==============================
// Invio comando al controller
// ==============================

int sd_execute_command(unsigned int code, unsigned int arg) {
	int response = 0;
	sd_error = SD_OK;

	// Comandi applicativi (preceduti da APP_CMD)
	if (code & CMD_NEED_APP) {
		response = sd_execute_command(CMD_APP_CMD | sd_rca ? CMD_RSPNS_48 : 0, sd_rca);
		if (sd_rca && !response) {
			sd_error = SD_ERROR;
			return SD_ERROR;
		}
		code &= ~CMD_NEED_APP;
	}

	if (sd_status(SR_CMD_INHIBIT)) {
		uart_puts("[ERROR] EMMC inhibit\n");
	}

	*EMMC_INTERRUPT = *EMMC_INTERRUPT;
	*EMMC_ARG1 = arg;
	*EMMC_CMDTM = code;

	if (code == CMD_SEND_OP_COND) {
		wait_msec(1000);
	} else if (code == CMD_SEND_IF_COND || code == CMD_APP_CMD) {
		wait_msec(100);
	}

	if ((response = sd_wait_for_interrupt(INT_CMD_DONE))) {
		sd_error = response;
		return -1;
	}

	response = *EMMC_RESP0;
	if (code == CMD_GO_IDLE || code == CMD_APP_CMD) {
		return 0;
	} else if (code == (CMD_APP_CMD | CMD_RSPNS_48)) {
		return response & SR_APP_CMD;
	} else if (code == CMD_SEND_OP_COND) {
		return response;
	} else if (code == CMD_SEND_IF_COND) {
		return response == arg ? SD_OK : SD_ERROR;
	} else if (code == CMD_ALL_SEND_CID) {
		response |= *EMMC_RESP3;
		response |= *EMMC_RESP2;
		response |= *EMMC_RESP1;
		return response;
	} else if (code == CMD_SEND_REL_ADDR) {
		sd_error = (((response & 0x1fff))|((response & 0x2000)<<6)|((response & 0x4000)<<8)|((response & 0x8000)<<8)) & CMD_ERRORS_MASK;
		sd_rca = response & CMD_RCA_MASK;
		return sd_rca;
	}

	return response & CMD_ERRORS_MASK;
}

// ==============================
// Attesa stato controller
// ==============================

int sd_status(unsigned int mask) {
	int counter = 500000;
	while ((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & INT_ERROR_MASK)) {
		counter--;
		wait_msec(1);
	}
	return (counter <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

// ==============================
// Attesa interrupt specifico
// ==============================

int sd_wait_for_interrupt(unsigned int mask) {
	unsigned int response, m = mask | INT_ERROR_MASK;
	int counter = 100000; // su hardware reale aumentare
	while (!(*EMMC_INTERRUPT & m) && counter--) {
		wait_msec(1);
	}

	response = *EMMC_INTERRUPT;
	if (counter <= 0 || (response & INT_CMD_TIMEOUT) || (response & INT_DATA_TIMEOUT)) {
		*EMMC_INTERRUPT = response;
		return SD_ERROR;
	} else if (response & INT_ERROR_MASK) {
		*EMMC_INTERRUPT = mask;
		return SD_ERROR;
	}

	*EMMC_INTERRUPT = mask;
	return 0;
}

// ==============================
// Abilitazione card (ACMD41)
// ==============================

int sd_enable_card(void) {
	long counter = 6, response = 0;
	while (!(response & ACMD41_CMD_COMPLETE) && counter--) {
		delay(400);
		response = sd_execute_command(CMD_SEND_OP_COND, ACMD41_ARG_HC);

		if (sd_error != SD_TIMEOUT && sd_error != SD_OK) {
			uart_puts("[ERROR] EMMC ACMD41\n");
			return SD_ERROR;
		}
	}

	if (!(response & ACMD41_CMD_COMPLETE) || !counter) return SD_ERROR;
	if (!(response & ACMD41_VOLTAGE)) return SD_ERROR;

	ccs = 0;
	if (!(response & ACMD41_CMD_CCS)) {
		ccs = SCR_SUPP_CCS;
	}
	return SD_OK;
}

// ==============================
// Attesa millisecondi (system timer)
// ==============================

void wait_msec(unsigned int n) {
	uint32_t system_clock = *(volatile uint32_t*)SYS_TIMER_CLOCK;
	uint32_t target = system_clock + n * 1000;
	while ((int32_t)(*(volatile uint32_t*)SYS_TIMER_CLOCK - target) < 0);
}

