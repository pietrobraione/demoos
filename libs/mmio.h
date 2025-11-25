#ifndef __MMIO_H
 #define __MMIO_H

 #include <stdint.h>

 #define MMIO_BASE 0x3F000000

 static inline void mmio_write(uint32_t reg, uint32_t data)
 {
     *(volatile uint32_t*)(MMIO_BASE + reg) = data;
 }

 static inline uint32_t mmio_read(uint32_t reg)
 {
     return *(volatile uint32_t*)(MMIO_BASE + reg);
 }

 enum {
     GPIO_BASE   = 0x200000,
     GPPUD       = (GPIO_BASE + 0x94),
     GPPUDCLK0   = (GPIO_BASE + 0x98),
 };

 #define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
 #define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
 #define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
 #define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
 #define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
 #define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
 #define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
 #define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
 #define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
 #define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
 #define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
 #define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
 #define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
 #define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
 #define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
 #define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
 #define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
 #define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

 #define EMMC_ARG2           ((volatile unsigned int*)(MMIO_BASE+0x00300000))
 #define EMMC_BLKSIZECNT     ((volatile unsigned int*)(MMIO_BASE+0x00300004))
 #define EMMC_ARG1           ((volatile unsigned int*)(MMIO_BASE+0x00300008))
 #define EMMC_CMDTM          ((volatile unsigned int*)(MMIO_BASE+0x0030000C))
 #define EMMC_RESP0          ((volatile unsigned int*)(MMIO_BASE+0x00300010))
 #define EMMC_RESP1          ((volatile unsigned int*)(MMIO_BASE+0x00300014))
 #define EMMC_RESP2          ((volatile unsigned int*)(MMIO_BASE+0x00300018))
 #define EMMC_RESP3          ((volatile unsigned int*)(MMIO_BASE+0x0030001C))
 #define EMMC_DATA           ((volatile unsigned int*)(MMIO_BASE+0x00300020))
 #define EMMC_STATUS         ((volatile unsigned int*)(MMIO_BASE+0x00300024))
 #define EMMC_CONTROL0       ((volatile unsigned int*)(MMIO_BASE+0x00300028))
 #define EMMC_CONTROL1       ((volatile unsigned int*)(MMIO_BASE+0x0030002C))
 #define EMMC_INTERRUPT      ((volatile unsigned int*)(MMIO_BASE+0x00300030))
 #define EMMC_INT_MASK       ((volatile unsigned int*)(MMIO_BASE+0x00300034))
 #define EMMC_INT_EN         ((volatile unsigned int*)(MMIO_BASE+0x00300038))
 #define EMMC_CONTROL2       ((volatile unsigned int*)(MMIO_BASE+0x0030003C))
 #define EMMC_SLOTISR_VER    ((volatile unsigned int*)(MMIO_BASE+0x003000FC))

 #endif
