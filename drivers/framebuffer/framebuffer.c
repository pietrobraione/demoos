#include <stddef.h>
#include "framebuffer.h"
#include "../mbox/mbox.h"
#include "../uart/uart.h"

#define WIDTH   1024
#define HEIGHT  768
#define DEPTH   32

void *framebuffer_address = NULL;
uint32_t width, height, pitch, isrgb;

int framebuffer_init()
{
  mbox[0] = 35*4;                  //length: 35*4 bytes
  mbox[1] = MBOX_REQUEST;
  mbox[2] = MBOX_TAG_SETPHYWH;     //set physical width/height
  mbox[3] = 8;
  mbox[4] = 8;
  mbox[5] = WIDTH;                 //width
  mbox[6] = HEIGHT;                //height
  mbox[7] = MBOX_TAG_SETVIRTWH;    //set virtual width/height
  mbox[8] = 8;
  mbox[9] = 8;
  mbox[10] = WIDTH;                //width
  mbox[11] = HEIGHT;               //height
  mbox[12] = MBOX_TAG_SETVIRTOFST; //set virtual offset
  mbox[13] = 8;
  mbox[14] = 8;
  mbox[15] = 0;                    //x ofst
  mbox[16] = 0;                    //y ofst
  mbox[17] = MBOX_TAG_SETDEPTH;    //set color depth
  mbox[18] = 4;
  mbox[19] = 4;
  mbox[20] = DEPTH;                //32 bits
  mbox[21] = MBOX_TAG_SETPIXLORDR; //set pixel order (RGB or BGR)
  mbox[22] = 4;
  mbox[23] = 4;
  mbox[24] = 1;                    //RGB
  mbox[25] = MBOX_TAG_ALLOCFB;     //allocate framebuffer
  mbox[26] = 8;
  mbox[27] = 8;
  mbox[28] = 4096;                 //align to page
  mbox[29] = 0;                    //will be filled with the address of the framebuffer
  mbox[30] = MBOX_TAG_GETPITCH;    //get pitch (number of bytes per line)
  mbox[31] = 4;
  mbox[32] = 4;
  mbox[33] = 0;                    //will be filled with the pitch
  mbox[34] = MBOX_TAG_LAST;

  int ok = (mbox_call(MBOX_CH_PROP) && mbox[20] == DEPTH && mbox[28] != 0);
  if (ok) {
    framebuffer_address = (void *) (uint64_t) (mbox[28] & 0x3FFFFFFF);
    width = mbox[5];
    height = mbox[6];
    pitch = mbox[33];
    isrgb = mbox[24];
  }
  return ok;
}

/* PC Screen Font as used by Linux Console */
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

/**
 * Display a string using fixed size PSF
 */
void framebuffer_print(int x, int y, char *s)
{
  // get our font
  volatile psf_t *font = &__font_start;
  // draw next character if it's not zero
  while (*s) {
    // get the offset of the glyph. Need to adjust this to support unicode table
    unsigned char *glyph = (unsigned char *) font +
      font->headersize + (*((unsigned char*)s) < font->numglyph ? *s : 0) * font->bytesperglyph;
    // calculate the offset on screen
    int offs = (y * pitch) + (x * 4);
    // variables
    unsigned int i, j; 
    int line, mask, bytesperline=(font->width+7)/8;
    // handle carriage return
    if(*s == '\r') {
      x = 0;
    } else
      // new line
      if(*s == '\n') {
	x = 0; y += font->height;
      } else {
	// display a character
	for(j=0;j<font->height;j++){
	  // display one row
	  line=offs;
	  mask=1<<(font->width-1);
	  for(i=0;i<font->width;i++){
	    // if bit set, we use white color, otherwise black
	    *((unsigned int*)(framebuffer_address + line))=((int)*glyph) & mask?0xFFFFFF:0;
	    mask>>=1;
	    line+=4;
	  }
	  // adjust to next line
	  glyph+=bytesperline;
	  offs+=pitch;
	}
	x += (font->width+1);
      }
    // next character
    s++;
  }
}

