#include "mbox.h"
#include "../../libs/mmio.h"

#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

/* mailbox message buffer */
volatile uint32_t __attribute__((aligned(16))) mbox[36];

#define MBOX_MAIL_CHANNEL_MASK 0xF

int mbox_call(uint8_t channel)
{
  uint64_t r = (((uint64_t)(&mbox) & ~MBOX_MAIL_CHANNEL_MASK) | (channel & MBOX_MAIL_CHANNEL_MASK));
  // wait until we can talk to the VC
  while ( mmio_read(MBOX_STATUS) & MBOX_FULL ) { }
  // send our message to property channel and wait for the response
  mmio_write(MBOX_WRITE, r);
  while ( (mmio_read(MBOX_STATUS) & MBOX_EMPTY) || mmio_read(MBOX_READ) != r ) { }
  return (mbox[1] == MBOX_RESPONSE);
}
