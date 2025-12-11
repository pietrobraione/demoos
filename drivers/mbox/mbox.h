#ifndef __MBOX_H
#define __MBOX_H

/**
 * Mailbox property channel: usato per configurazioni (clock, framebuffer, ecc.).
 */

#include <stdint.h>

// ==============================
// Registri mailbox (offset da MMIO_BASE)
// ==============================

enum {
	MBOX_BASE	= 0xB880,
	MBOX_READ	= (MBOX_BASE + 0x00),
	MBOX_STATUS	= (MBOX_BASE + 0x18),
	MBOX_WRITE	 = (MBOX_BASE + 0x20)
};

#define MBOX_REQUEST	0

// ==============================
// Canali
// ==============================

#define MBOX_CH_POWER	0
#define MBOX_CH_FB		1
#define MBOX_CH_VUART	2
#define MBOX_CH_VCHIQ	3
#define MBOX_CH_LEDS	4
#define MBOX_CH_BTNS	5
#define MBOX_CH_TOUCH	6
#define MBOX_CH_PROP	8

// ==============================
// Tag
// ==============================

#define MBOX_TAG_LAST			0
#define MBOX_TAG_GETSERIAL		0x10004
#define MBOX_TAG_SETCLKRATE		0x38002
#define MBOX_TAG_ALLOCFB		0x40001
#define MBOX_TAG_GETPITCH		0x40008
#define MBOX_TAG_SETPHYWH		0x48003
#define MBOX_TAG_SETVIRTWH		0x48004
#define MBOX_TAG_SETDEPTH		0x48005
#define MBOX_TAG_SETPIXLORDR	0x48006
#define MBOX_TAG_SETVIRTOFST	0x48009

// ==============================
// Buffer allineato (extern)
// ==============================

extern volatile uint32_t mbox[36];

int mbox_call(uint8_t channel);

#endif // __MBOX_H

