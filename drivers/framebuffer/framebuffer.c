/**
 * Driver framebuffer per Raspberry Pi.
 * Configura risoluzione/colore via mailbox e disegna testo con font PSF.
 */

#include <stddef.h>
#include <stdint.h>
#include "framebuffer.h"
#include "../mbox/mbox.h"
#include "../uart/uart.h"

// ==============================
// Parametri framebuffer
// ==============================

#define WIDTH	1024
#define HEIGHT	768
#define DEPTH	32

// ==============================
// Stato framebuffer
// ==============================

static void *framebuffer_address = NULL;
static uint32_t width, height, pitch, isrgb;

// ==============================
// Inizializzazione framebuffer
// ==============================

int framebuffer_init(void)
{
	mbox[0] = 35*4;
	mbox[1] = MBOX_REQUEST;

	// Larghezza/altezza fisica
	mbox[2] = MBOX_TAG_SETPHYWH;
	mbox[3] = 8;	mbox[4] = 8;
	mbox[5] = WIDTH;	mbox[6] = HEIGHT;

	// Larghezza/altezza virtuale
	mbox[7] = MBOX_TAG_SETVIRTWH;
	mbox[8] = 8;	mbox[9] = 8;
	mbox[10] = WIDTH; mbox[11] = HEIGHT;

	// Offset virtuale
	mbox[12] = MBOX_TAG_SETVIRTOFST;
	mbox[13] = 8;	mbox[14] = 8;
	mbox[15] = 0;	mbox[16] = 0;

	// Profondità colore
	mbox[17] = MBOX_TAG_SETDEPTH;
	mbox[18] = 4;	mbox[19] = 4;
	mbox[20] = DEPTH;

	// Ordine pixel
	mbox[21] = MBOX_TAG_SETPIXLORDR;
	mbox[22] = 4;	mbox[23] = 4;
	mbox[24] = 1; // RGB

	// Allocazione framebuffer (allineata a pagina)
	mbox[25] = MBOX_TAG_ALLOCFB;
	mbox[26] = 8;	mbox[27] = 8;
	mbox[28] = 4096; mbox[29] = 0;

	// Pitch
	mbox[30] = MBOX_TAG_GETPITCH;
	mbox[31] = 4;	mbox[32] = 4;
	mbox[33] = 0;

	// Terminatore
	mbox[34] = MBOX_TAG_LAST;

	int ok = (mbox_call(MBOX_CH_PROP) && mbox[20] == DEPTH && mbox[28] != 0);
	if (ok) {
	framebuffer_address = (void *)(uint64_t)(mbox[28] & 0x3FFFFFFF);
	width = mbox[5];
	height = mbox[6];
	pitch = mbox[33];
	isrgb = mbox[24];
	}
	return ok;
}

// ==============================
// Formato PSF (font fisso)
// ==============================

typedef struct {
	unsigned int magic;
	unsigned int version;
	unsigned int headersize;
	unsigned int flags;
	unsigned int numglyph;
	unsigned int bytesperglyph;
	unsigned int height;
	unsigned int width;
	unsigned char glyphs;
} __attribute__((packed)) psf_t;

extern volatile psf_t __font_start;

// ==============================
// Stampa di una stringa con font PSF
// ==============================

void framebuffer_print(int x, int y, char *s)
{
	volatile psf_t *font = &__font_start;

	while (*s) {
	unsigned char *glyph = (unsigned char *) font +
		font->headersize + (*((unsigned char*)s) < font->numglyph ? *s : 0) * font->bytesperglyph;

	int offs = (y * pitch) + (x * 4);
	unsigned int i, j;
	int line, mask, bytesperline = (font->width + 7) / 8;

	if (*s == '\r') {
		x = 0;
	} else if (*s == '\n') {
		x = 0; y += font->height;
	} else {
		for (j = 0; j < font->height; j++) {
		line = offs;
		mask = 1 << (font->width - 1);
		for (i = 0; i < font->width; i++) {
			*((unsigned int*)((uintptr_t)framebuffer_address + line)) = (((int)*glyph) & mask) ? 0xFFFFFF : 0x000000;
			mask >>= 1;
			line += 4;
		}
		glyph += bytesperline;
		offs += pitch;
		}
		x += (font->width + 1);
	}
	s++;
	}
}

