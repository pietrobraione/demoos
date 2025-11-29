#include "sd.h"
#include "../../libs/utils.h"
#include "../uart/uart.h"

void enable_card_detect();
void enable_clock_and_command();
void enable_data_pins();
void reset_emmc();
int sd_set_clock(unsigned int);
int sd_execute_command(unsigned int, unsigned int);
int sd_status(unsigned int);
int sd_wait_for_interrupt(unsigned int);
int sd_enable_card();
long sd_get_rca();

long sd_standard_version, ccs;

unsigned long sd_scr[2], sd_rca, sd_error;

int sd_init() {
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

    // I enable the EMMC interrupts
    *EMMC_INT_EN = 0xFFFFFFFF;
    *EMMC_INT_MASK = 0xFFFFFFFF;

    sd_scr[0] = sd_scr[1] = sd_rca = sd_error = 0;

    // I execute the following two commands because this is how the DS
    // standard works

    sd_execute_command(CMD_GO_IDLE, 0);
    if (sd_error) {
        return sd_error;
    }

    sd_execute_command(CMD_SEND_IF_COND, 0x1AA);
    if (sd_error) {
        return sd_error;
    }

    int enable_result = sd_enable_card();
    if (enable_result == SD_ERROR) {
        return SD_ERROR;
    }

    // Asks the SD for its Card Identificator
    sd_execute_command(CMD_ALL_SEND_CID, 0);

    // Then the SD generates a Relative Card Address
    long sd_rca = sd_execute_command(CMD_SEND_REL_ADDR, 0);
    uart_hex(sd_rca >> 32);
    uart_hex(sd_rca);
    uart_puts("\n");
    if (sd_error) {
        return sd_error;
    }

    long response;
    if ((response = sd_set_clock(25000000))) {
        return response;
    }

    // Then I select the SD using its RCA
    sd_execute_command(CMD_CARD_SELECT, sd_rca);
    if (sd_error) {
        return sd_error;
    }

    if (sd_status(SR_DAT_INHIBIT)) {
        return SD_TIMEOUT;
    }

    // This registers tells the CPU to read 1 block of 8 bytes
    *EMMC_BLKSIZECNT = (1 << 16) | 8;
    sd_execute_command(CMD_SEND_SCR, 0);
    if (sd_error) {
        return sd_error;
    }

    if (sd_wait_for_interrupt(INT_READ_RDY)) {
        return SD_TIMEOUT;
    }

    // I read 2 words from EMMC_DATA to sd_scr
    response = 0;
    long counter = 100000;
    while (response < 2 && counter) {
        if (*EMMC_STATUS & SR_READ_AVAILABLE) {
            sd_scr[response++] = *EMMC_DATA;
        } else {
            wait_msec(1);
        }
    }

    if (response != 2) {
        return SD_TIMEOUT;
    }

    if (sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
        sd_execute_command(CMD_SET_BUS_WIDTH, sd_rca | 2);
        if (sd_error) {
            return sd_error;
        }
        *EMMC_CONTROL0 |= C0_HCTL_DWIDTH;
    }

    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;

    return SD_OK;
}

void enable_card_detect() {
    // First, I set the GPIO 47 (card detect) pin to input (000). The GPIO47 is inside
    // group 4 (40-49) in position 7 and each position is 3 bit: GPFSEL4 + 7 * 3
    long r = *GPFSEL4;
    r &= ~(7 << (7 * 3));
    *GPFSEL4 = r;

    // Then I pullup the value of the pin to high to avoid unstable state
    // The GPPUD is the register which stores which type of pull to perform (up or down)
    // I have to write this register to perform a pullup which value is two
    *GPPUD = 2;
    delay(150);
    // Then I need to write which PIN should have the pullup; GPPUDCLK1 controls GPIO fr
    // from 32 to 53, so the offset for the GPIO 47 is 15
    *GPPUDCLK1 = (1 << 15);
    delay(150);
    // Then the pullup is applied and I reset the registers
    *GPPUD = 0;
    *GPPUDCLK1 = 0;

    // The register GPHEN1 saves which PINS (from 21 to 53) will generate an interrupt
    // if their value is maintained to 1; I have to write the GPIO 47
    r = *GPHEN1;
    r |= 1 << 15;
    *GPHEN1 = r;
}

void enable_clock_and_command() {
    // The GPIO48 is the PIN which controls the clock of the SD controller,
    // while the GPIO49 controls the command line to send commands to the SD
    // These PINs are in the 4th group with an offset of 8 and 9

    // As before, I have to set the function registers. In this case the function
    // bits are 111
    long r = *GPFSEL4;
    r |= (7 << (8 * 3)) | (7 << 9 * 3);
    *GPFSEL4 = r;

    // Then I pullup these PINS like in the function above
    *GPPUD = 2;
    delay(150);

    *GPPUDCLK1 = (1 << 16) | (1 << 17);
    delay(150);

    *GPPUD = 0;
    *GPPUDCLK1 = 0;
}

void enable_data_pins() {
    // DAT0 is the GPIO50, so it is part of the group 5. I have to apply the function
    // bits, which in this case will be 111
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

void reset_emmc() {
    // I have to reset all the bits of the controller
    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 |= C1_SRST_HC;

    // The raspberry hardware should put the EMMC_CONTROL1 to 0 after a while;
    // so I wait this to happen
    // IMPORTANT: qemu is not implementing the EMMC controller, so the CONTROL1
    // bit will never be written
    // To confirm this thesis, execute the following code:
    /*
    *EMMC_CONTROL1 = 0x20;
    uart_hex(*EMMC_CONTROL1); // Prints 0x20
    uart_puts("\n");

    *EMMC_CONTROL1 = 0x01000000;
    uart_hex(*EMMC_CONTROL1); // Prints 0
    uart_puts("\n");
    */
    // Maybe the bit reset is really fast but this is really sus

    while (*EMMC_CONTROL1 & C1_SRST_HC) {
    }
}

int sd_set_clock(unsigned int frequency) {
    int counter = 100000;

    // Busy wait until EMMC inhibit flags are removed
    while (*EMMC_STATUS & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) {
        counter--;
        wait_msec(1);

        if (counter <= 0) {
            return SD_ERROR;
        }
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
        if (!(x & 0xff000000u)) { x <<= 8;  s -= 8; }
        if (!(x & 0xf0000000u)) { x <<= 4;  s -= 4; }
        if (!(x & 0xc0000000u)) { x <<= 2;  s -= 2; }
        if (!(x & 0x80000000u)) { x <<= 1;  s -= 1; }
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

        if (counter <= 0) {
            return SD_ERROR;
        }
    }

    return SD_OK;
}

int sd_execute_command(unsigned int code, unsigned int arg) {
    int response = 0;
    sd_error = SD_OK;

    // If the code is application specific, first I need to send an APP command
    if (code & CMD_NEED_APP) {
        response = sd_execute_command(CMD_APP_CMD | sd_rca ? CMD_RSPNS_48 : 0, sd_rca);
        if (sd_rca && !response) {
            sd_error = SD_ERROR;
            return SD_ERROR;
        }
        code &= ~CMD_NEED_APP;
    }

    if (sd_status(SR_CMD_INHIBIT)) {
        uart_puts("[ERROR] EMMC is inhibit\n");
    }

    /*
    uart_puts("[DEBUG] Sending command ");
    uart_hex(code);
    uart_puts(" with arg ");
    uart_hex(arg);
    uart_puts("to EMMC\n");
    */

    *EMMC_INTERRUPT = *EMMC_INTERRUPT;
    *EMMC_ARG1 = arg;
    *EMMC_CMDTM = code;

    if (code == CMD_SEND_OP_COND) {
        wait_msec(1000);
    } else if (code == CMD_SEND_IF_COND || code == CMD_APP_CMD) {
        wait_msec(100);
    }

    if ((response = sd_wait_for_interrupt(INT_CMD_DONE))) {
        // uart_puts("[ERROR] Failed to send EMMC command\n");
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
        return response & CMD_RCA_MASK;
    }

    return response & CMD_ERRORS_MASK;
}

int sd_status(unsigned int mask) {
    int counter = 500000;
    while ((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & INT_ERROR_MASK)) {
        counter--;
        wait_msec(1);
    }

    return (counter <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

int sd_wait_for_interrupt(unsigned int mask) {
    unsigned int response, m = mask | INT_ERROR_MASK;
    int counter = 10; // TODO make this 100000 when we'll have EMMC
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

int sd_enable_card() {
    long counter = 6, response = 0;
    while (!(response & ACMD41_CMD_COMPLETE) && counter--) {
        delay(400);
        // This commands tells the SD to start the powerup
        response = sd_execute_command(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        uart_puts("[DEBUG] EMMC executed CMD_SEND_OP_COND and returned ");
        // This bit tells if the SD power-up is completed
        if (response & ACMD41_CMD_COMPLETE) {
            uart_puts("COMPLETE ");
        }
        // This bit tells if the 3.3v tension is supported
        if (response & ACMD41_VOLTAGE) {
            uart_puts("VOLTAGE ");
        }
        // This bit is 1 if the card capacity is enabled
        if (response & ACMD41_CMD_CCS) {
            uart_puts("CSS ");
        }
        uart_hex(response >> 32);
        uart_hex(response);
        uart_puts("\n");

        if (sd_error != SD_TIMEOUT && sd_error != SD_OK) {
            uart_puts("[ERROR] EMMC ACMD41 returned error\n");
            return SD_ERROR;
        }
    }

    if (!(response & ACMD41_CMD_COMPLETE) || !counter) {
        return SD_ERROR;
    }
    if (!(response & ACMD41_VOLTAGE)) {
        return SD_ERROR;
    }
    ccs = 0;
    if (!(response & ACMD41_CMD_CCS)) {
        ccs = SCR_SUPP_CCS;
    }

    return SD_OK;
}

// TODO move from here to util
void wait_msec(unsigned int n) {
    uint32_t system_clock = *(volatile uint32_t*)SYS_TIMER_CLOCK;
    uint32_t target = system_clock + n * 1000;

    while ((int32_t)(*(volatile uint32_t*)SYS_TIMER_CLOCK - target) < 0);
}
