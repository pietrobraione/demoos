#ifndef __MBOX_H
#define __MBOX_H

#include <stdint.h>

enum {
    // The offsets for Mailbox registers
    MBOX_BASE    = 0xB880,
    MBOX_READ    = (MBOX_BASE + 0x00),
    MBOX_STATUS  = (MBOX_BASE + 0x18),
    MBOX_WRITE   = (MBOX_BASE + 0x20)
};

#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_LAST           0
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_ALLOCFB        0x40001
#define MBOX_TAG_GETPITCH       0x40008
#define MBOX_TAG_SETPHYWH       0x48003
#define MBOX_TAG_SETVIRTWH      0x48004
#define MBOX_TAG_SETDEPTH       0x48005
#define MBOX_TAG_SETPIXLORDR    0x48006
#define MBOX_TAG_SETVIRTOFST    0x48009

/* a properly aligned buffer */
extern volatile uint32_t mbox[36];

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(uint8_t channel);

#endif
