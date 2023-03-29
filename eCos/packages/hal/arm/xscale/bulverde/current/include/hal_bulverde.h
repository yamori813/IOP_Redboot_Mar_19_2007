/*=============================================================================
//
//      hal_bulverde.h
//
//      Bulverde I/O module support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-07-16
// Purpose:      
// Description:  Bulverde I/O modules support.
// Usage:        #include <cyg/hal/hal_bulverde.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_BULVERDE_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_BULVERDE_H

#include <pkgconf/system.h>
#include <cyg/hal/hal_xscale.h>

// Peripheral MMR Base Addresses
#define DMA_BASE          0x40000000
#define FFUART_BASE       0x40100000
#define BTUART_BASE       0x40200000
#define I2C_BASE          0x40300000
#define I2S_BASE          0x40400000
#define AC97_BASE         0x40500000
#define USBCLIENT_BASE    0x40600000
#define SIR_UART_BASE     0x40700000
#define IRDA_BASE         0x40800000
#define RTC_BASE          0x40900000
#define OSTIMER_BASE      0x40a00000
#define PWM02_BASE        0x40b00000
#define PWM13_BASE        0x40c00000
#define INTR_BASE         0x40d00000
#define GPIO_BASE         0x40e00000
#define POWER_BASE        0x40f00000
#define SSP1_BASE         0x41000000
#define MMC_BASE          0x41100000
#define CLOCK_BASE        0x41300000
#define MSL_BASE          0x41400000
#define KEYPAD_BASE       0x41500000
#define USIM_BASE         0x41600000
#define SSP2_BASE         0x41700000
#define MS_BASE           0x41800000
#define SSP3_BASE         0x41900000

#define LCD_BASE          0x44000000
#define MCU_BASE          0x48000000
#define USBHOST_BASE      0x4c000000
#define INTERNAL_MEM_BASE 0x58000000

// =============================================================================
// RTC Definitions
// =============================================================================
#define RTC_RCNR           REG32(RTC_BASE,0x00)
#define RTC_RTAR           REG32(RTC_BASE,0x04)
#define RTC_RTSR           REG32(RTC_BASE,0x08)
#define RTC_RTTR           REG32(RTC_BASE,0x0c)
#define RTC_RDCR           REG32(RTC_BASE,0x10)
#define RTC_RYCR           REG32(RTC_BASE,0x14)
#define RTC_RDAR1          REG32(RTC_BASE,0x18)
#define RTC_RYAR1          REG32(RTC_BASE,0x1c)
#define RTC_RDAR2          REG32(RTC_BASE,0x20)
#define RTC_RYAR2          REG32(RTC_BASE,0x24)
#define RTC_SWCR           REG32(RTC_BASE,0x28)
#define RTC_SWAR1          REG32(RTC_BASE,0x2c)
#define RTC_SWAR2          REG32(RTC_BASE,0x30)
#define RTC_PICR           REG32(RTC_BASE,0x34)
#define RTC_PIAR           REG32(RTC_BASE,0x38)

// =============================================================================
// OS Timer Definitions
// =============================================================================
#define OSTIMER_OSMR0      REG32(OSTIMER_BASE,0x00)
#define OSTIMER_OSMR1      REG32(OSTIMER_BASE,0x04)
#define OSTIMER_OSMR2      REG32(OSTIMER_BASE,0x08)
#define OSTIMER_OSMR3      REG32(OSTIMER_BASE,0x0C)
#define OSTIMER_OSCR0      REG32(OSTIMER_BASE,0x10)
#define OSTIMER_OSSR       REG32(OSTIMER_BASE,0x14)
#define OSTIMER_OWER       REG32(OSTIMER_BASE,0x18)
#define OSTIMER_OIER       REG32(OSTIMER_BASE,0x1C)
#define OSTIMER_OSCR4      REG32(OSTIMER_BASE,0x40)
#define OSTIMER_OSCR5      REG32(OSTIMER_BASE,0x44)
#define OSTIMER_OSCR6      REG32(OSTIMER_BASE,0x48)
#define OSTIMER_OSCR7      REG32(OSTIMER_BASE,0x4C)
#define OSTIMER_OSCR8      REG32(OSTIMER_BASE,0x50)
#define OSTIMER_OSCR9      REG32(OSTIMER_BASE,0x54)
#define OSTIMER_OSCR10     REG32(OSTIMER_BASE,0x58)
#define OSTIMER_OSCR11     REG32(OSTIMER_BASE,0x5C)
#define OSTIMER_OSMR4      REG32(OSTIMER_BASE,0x80)
#define OSTIMER_OSMR5      REG32(OSTIMER_BASE,0x84)
#define OSTIMER_OSMR6      REG32(OSTIMER_BASE,0x88)
#define OSTIMER_OSMR7      REG32(OSTIMER_BASE,0x8C)
#define OSTIMER_OSMR8      REG32(OSTIMER_BASE,0x90)
#define OSTIMER_OSMR9      REG32(OSTIMER_BASE,0x94)
#define OSTIMER_OSMR10     REG32(OSTIMER_BASE,0x98)
#define OSTIMER_OSMR11     REG32(OSTIMER_BASE,0x9C)
#define OSTIMER_OMCR4      REG32(OSTIMER_BASE,0xC0)
#define OSTIMER_OMCR5      REG32(OSTIMER_BASE,0xC4)
#define OSTIMER_OMCR6      REG32(OSTIMER_BASE,0xC8)
#define OSTIMER_OMCR7      REG32(OSTIMER_BASE,0xCC)
#define OSTIMER_OMCR8      REG32(OSTIMER_BASE,0xD0)
#define OSTIMER_OMCR9      REG32(OSTIMER_BASE,0xD4)
#define OSTIMER_OMCR10     REG32(OSTIMER_BASE,0xD8)
#define OSTIMER_OMCR11     REG32(OSTIMER_BASE,0xDC)

// OMCRx bit definitions
#define OMCR_CRES_DISABLE  (0 << 0)
#define OMCR_CRES_32768HZ  (1 << 0)
#define OMCR_CRES_1KHZ     (2 << 0)
#define OMCR_CRES_1HZ      (3 << 0)
#define OMCR_CRES_1MHZ     (4 << 0)
#define OMCR_CRES_EXT      (5 << 0)
#define OMCR_RESET         (1 << 3)
#define OMCR_NOSYNC        (0 << 4)
#define OMCR_SYNC0         (1 << 4)
#define OMCR_SYNC1         (2 << 4)
#define OMCR_PERIODIC      (1 << 6)
#define OMCR_MATCH_CHAN_X  (1 << 7)

// OSSR bit definitions
#define OSSR_TIMER0  (1<<0)   // Timer match register #0
#define OSSR_TIMER1  (1<<1)
#define OSSR_TIMER2  (1<<2)
#define OSSR_TIMER3  (1<<3)
#define OSSR_TIMER4  (1<<4)
#define OSSR_TIMER5  (1<<5)
#define OSSR_TIMER6  (1<<6)
#define OSSR_TIMER7  (1<<7)
#define OSSR_TIMER8  (1<<8)
#define OSSR_TIMER9  (1<<9)
#define OSSR_TIMER10 (1<<10)
#define OSSR_TIMER11 (1<<11)

// OIER bit definitions
#define OIER_TIMER0  (1<<0)
#define OIER_TIMER1  (1<<1)
#define OIER_TIMER2  (1<<2)
#define OIER_TIMER3  (1<<3)
#define OIER_TIMER4  (1<<4)
#define OIER_TIMER5  (1<<5)
#define OIER_TIMER6  (1<<6)
#define OIER_TIMER7  (1<<7)
#define OIER_TIMER8  (1<<8)
#define OIER_TIMER9  (1<<9)
#define OIER_TIMER10 (1<<10)
#define OIER_TIMER11 (1<<11)

// OWER bit definitions
#define OWER_ENABLE  (1<<0) // write-once! watchdog enable

// =============================================================================
// Interrupt Controller Definitions
// =============================================================================
#define INTR_ICIP       REG32(INTR_BASE,0x00)
#define INTR_ICMR       REG32(INTR_BASE,0x04)
#define INTR_ICLR       REG32(INTR_BASE,0x08)
#define INTR_ICFP       REG32(INTR_BASE,0x0c)
#define INTR_ICPR       REG32(INTR_BASE,0x10)
#define INTR_ICCR       REG32(INTR_BASE,0x14)
#define INTR_ICHP       REG32(INTR_BASE,0x18)
#define INTR_IPR_BASE   REG32(INTR_BASE,0x1c)


#ifndef __ASSEMBLER__
#define ICIP_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c0, 0" : "=r" (val))
#define ICMR_READ(val)   asm volatile ("mrc p6, 0, %0, c1, c0, 0" : "=r" (val))
#define ICLR_READ(val)   asm volatile ("mrc p6, 0, %0, c2, c0, 0" : "=r" (val))
#define ICFP_READ(val)   asm volatile ("mrc p6, 0, %0, c3, c0, 0" : "=r" (val))
#define ICPR_READ(val)   asm volatile ("mrc p6, 0, %0, c4, c0, 0" : "=r" (val))
#define ICHP_READ(val)   asm volatile ("mrc p6, 0, %0, c5, c0, 0" : "=r" (val))

#define _ICMR_WRITE(val) asm volatile ("mcr p6, 0, %0, c1, c0, 0" : : "r" (val))
static inline void ICMR_WRITE(cyg_uint32 val) { _ICMR_WRITE(val); }
#define _ICLR_WRITE(val) asm volatile ("mcr p6, 0, %0, c2, c0, 0" : : "r" (val))
static inline void ICLR_WRITE(cyg_uint32 val) { _ICLR_WRITE(val); }
#endif

// =============================================================================
// GPIO Definitions
// =============================================================================
#define GPIO_GPLR0      REG32(GPIO_BASE,0x00)
#define GPIO_GPLR1      REG32(GPIO_BASE,0x04)
#define GPIO_GPLR2      REG32(GPIO_BASE,0x08)
#define GPIO_GPDR0      REG32(GPIO_BASE,0x0c)
#define GPIO_GPDR1      REG32(GPIO_BASE,0x10)
#define GPIO_GPDR2      REG32(GPIO_BASE,0x14)
#define GPIO_GPSR0      REG32(GPIO_BASE,0x18)
#define GPIO_GPSR1      REG32(GPIO_BASE,0x1c)
#define GPIO_GPSR2      REG32(GPIO_BASE,0x20)
#define GPIO_GPCR0      REG32(GPIO_BASE,0x24)
#define GPIO_GPCR1      REG32(GPIO_BASE,0x28)
#define GPIO_GPCR2      REG32(GPIO_BASE,0x2c)
#define GPIO_GRER0      REG32(GPIO_BASE,0x30)
#define GPIO_GRER1      REG32(GPIO_BASE,0x34)
#define GPIO_GRER2      REG32(GPIO_BASE,0x38)
#define GPIO_GFER0      REG32(GPIO_BASE,0x3c)
#define GPIO_GFER1      REG32(GPIO_BASE,0x40)
#define GPIO_GFER2      REG32(GPIO_BASE,0x44)
#define GPIO_GEDR0      REG32(GPIO_BASE,0x48)
#define GPIO_GEDR1      REG32(GPIO_BASE,0x4c)
#define GPIO_GEDR2      REG32(GPIO_BASE,0x50)
#define GPIO_GAFR0_L    REG32(GPIO_BASE,0x54)
#define GPIO_GAFR0_U    REG32(GPIO_BASE,0x58)
#define GPIO_GAFR1_L    REG32(GPIO_BASE,0x5c)
#define GPIO_GAFR1_U    REG32(GPIO_BASE,0x60)
#define GPIO_GAFR2_L    REG32(GPIO_BASE,0x64)
#define GPIO_GAFR2_U    REG32(GPIO_BASE,0x68)
#define GPIO_GAFR3_L    REG32(GPIO_BASE,0x6c)
#define GPIO_GAFR3_U    REG32(GPIO_BASE,0x70)
#define GPIO_GPLR3      REG32(GPIO_BASE,0x100)
#define GPIO_GPDR3      REG32(GPIO_BASE,0x10c)
#define GPIO_GPSR3      REG32(GPIO_BASE,0x118)
#define GPIO_GPCR3      REG32(GPIO_BASE,0x124)
#define GPIO_GRER3      REG32(GPIO_BASE,0x130)
#define GPIO_GFER3      REG32(GPIO_BASE,0x13c)
#define GPIO_GEDR3      REG32(GPIO_BASE,0x148)

// =============================================================================
// Power Management Definitions
// =============================================================================
#define POWER_PMCR      REG32(POWER_BASE,0x00)
#define POWER_PSSR      REG32(POWER_BASE,0x04)
#define POWER_PSPR      REG32(POWER_BASE,0x08)
#define POWER_PWER      REG32(POWER_BASE,0x0c)
#define POWER_PRER      REG32(POWER_BASE,0x10)
#define POWER_PFER      REG32(POWER_BASE,0x14)
#define POWER_PEDR      REG32(POWER_BASE,0x18)
#define POWER_PCFR      REG32(POWER_BASE,0x1c)
#define POWER_PGSR0     REG32(POWER_BASE,0x20)
#define POWER_PGSR1     REG32(POWER_BASE,0x24)
#define POWER_PGSR2     REG32(POWER_BASE,0x28)
#define POWER_PGSR3     REG32(POWER_BASE,0x2c)
#define POWER_RCSR      REG32(POWER_BASE,0x30)
#define POWER_PSLR      REG32(POWER_BASE,0x34)
#define POWER_PSTR      REG32(POWER_BASE,0x38)
#define POWER_PSNR      REG32(POWER_BASE,0x3c)
#define POWER_PVCR      REG32(POWER_BASE,0x40)
#define POWER_PKWR      REG32(POWER_BASE,0x50)
#define POWER_PKSR      REG32(POWER_BASE,0x54)
#define POWER_PCMD0     REG32(POWER_BASE,0x80)
#define POWER_PCMD1     REG32(POWER_BASE,0x84)
#define POWER_PCMD2     REG32(POWER_BASE,0x88)
#define POWER_PCMD3     REG32(POWER_BASE,0x8c)
#define POWER_PCMD4     REG32(POWER_BASE,0x90)
#define POWER_PCMD5     REG32(POWER_BASE,0x94)
#define POWER_PCMD6     REG32(POWER_BASE,0x98)
#define POWER_PCMD7     REG32(POWER_BASE,0x9c)
#define POWER_PCMD8     REG32(POWER_BASE,0xa0)
#define POWER_PCMD9     REG32(POWER_BASE,0xa4)
#define POWER_PCMD10    REG32(POWER_BASE,0xa8)
#define POWER_PCMD11    REG32(POWER_BASE,0xac)
#define POWER_PCMD12    REG32(POWER_BASE,0xb0)
#define POWER_PCMD13    REG32(POWER_BASE,0xb4)
#define POWER_PCMD14    REG32(POWER_BASE,0xb8)
#define POWER_PCMD15    REG32(POWER_BASE,0xbc)
#define POWER_PCMD16    REG32(POWER_BASE,0xc0)
#define POWER_PCMD17    REG32(POWER_BASE,0xc4)
#define POWER_PCMD18    REG32(POWER_BASE,0xc8)
#define POWER_PCMD19    REG32(POWER_BASE,0xcc)
#define POWER_PCMD20    REG32(POWER_BASE,0xd0)
#define POWER_PCMD21    REG32(POWER_BASE,0xd4)
#define POWER_PCMD22    REG32(POWER_BASE,0xd8)
#define POWER_PCMD23    REG32(POWER_BASE,0xdc)
#define POWER_PCMD24    REG32(POWER_BASE,0xe0)
#define POWER_PCMD25    REG32(POWER_BASE,0xe4)
#define POWER_PCMD26    REG32(POWER_BASE,0xe8)
#define POWER_PCMD27    REG32(POWER_BASE,0xec)
#define POWER_PCMD28    REG32(POWER_BASE,0xf0)
#define POWER_PCMD29    REG32(POWER_BASE,0xf4)
#define POWER_PCMD30    REG32(POWER_BASE,0xf8)
#define POWER_PCMD31    REG32(POWER_BASE,0xfc)

// PMCR bit definitions
#define PMCR_BIDAE              (1 << 0)
#define PMCR_BIDAS              (1 << 1)
#define PMCR_VIDAE              (1 << 2)
#define PMCR_VIDAS              (1 << 3)
#define PMCR_IAS                (1 << 4)
#define PMCR_INTRS              (1 << 5)

// PSSR bit definitions
#define PSSR_SSS                (1 << 0)
#define PSSR_BFS                (1 << 1)
#define PSSR_VFS                (1 << 2)
#define PSSR_STS                (1 << 3)
#define PSSR_PH                 (1 << 4)
#define PSSR_RDH                (1 << 5)

// PWER bit definitions
#define PWER_WE0                (1 << 0)
#define PWER_WE1                (1 << 1)
#define PWER_WE2                (1 << 2)
#define PWER_WE3                (1 << 3)
#define PWER_WE4                (1 << 4)
#define PWER_WE9                (1 << 9)
#define PWER_WE10               (1 << 10)
#define PWER_WE11               (1 << 11)
#define PWER_WE12               (1 << 12)
#define PWER_WE13               (1 << 13)
#define PWER_WE14               (1 << 14)
#define PWER_WE15               (1 << 15)
#define PWER_WEBB                (1 << 25)
#define PWER_WEUSBC             (1 << 26)
#define PWER_WEUSBH0            (1 << 27)
#define PWER_WEUSBH1            (1 << 28)
#define PWER_WEP1               (1 << 30)
#define PWER_WERTC              (1 << 31)

// PRER bit definitions
#define PRER_RE0                (1 << 0)
#define PRER_RE1                (1 << 1)
#define PRER_RE2                (1 << 2)
#define PRER_RE3                (1 << 3)
#define PRER_RE4                (1 << 4)
#define PRER_RE9                (1 << 9)
#define PRER_RE10               (1 << 10)
#define PRER_RE11               (1 << 11)
#define PRER_RE12               (1 << 12)
#define PRER_RE13               (1 << 13)
#define PRER_RE14               (1 << 14)
#define PRER_RE15               (1 << 15)

// PFER bit definitions
#define PFER_FE0                (1 << 0)
#define PFER_FE1                (1 << 1)
#define PFER_FE2                (1 << 2)
#define PFER_FE3                (1 << 3)
#define PFER_FE4                (1 << 4)
#define PFER_FE9                (1 << 9)
#define PFER_FE10               (1 << 10)
#define PFER_FE11               (1 << 11)
#define PFER_FE12               (1 << 12)
#define PFER_FE13               (1 << 13)
#define PFER_FE14               (1 << 14)
#define PFER_FE15               (1 << 15)

// PEDR bit definitions
#define PEDR_ED0                (1 << 0)
#define PEDR_ED1                (1 << 1)
#define PEDR_ED2                (1 << 2)
#define PEDR_ED3                (1 << 3)
#define PEDR_ED4                (1 << 4)
#define PEDR_ED9                (1 << 9)
#define PEDR_ED10               (1 << 10)
#define PEDR_ED11               (1 << 11)
#define PEDR_ED12               (1 << 12)
#define PEDR_ED13               (1 << 13)
#define PEDR_ED14               (1 << 14)
#define PEDR_ED15               (1 << 15)
#define PEDR_EDBB               (1 << 25)
#define PEDR_EDUSBC             (1 << 26)
#define PEDR_EDUSBH0            (1 << 27)
#define PEDR_EDUSBH1            (1 << 28)
#define PEDR_EDP1               (1 << 30)
#define PEDR_EDRTC              (1 << 31)

// PCFR bit definitions
#define PCFR_OPDE               (1 << 0)
#define PCFR_FP                 (1 << 1)
#define PCFR_FS                 (1 << 2)
#define PCFR_GPR_EN             (1 << 4)
#define PCFR_PI2C_EN            (1 << 6)
#define PCFR_DC_EN              (1 << 7)
#define PCFR_FVC                (1 << 10)
#define PCFR_L1_EN              (1 << 11)
#define PCFR_GPROD              (1 << 12)

// RCSR bit definitions
#define RCSR_HWR                (1 << 0)
#define RCSR_WDR                (1 << 1)
#define RCSR_SMR                (1 << 2)
#define RCSR_GPR                (1 << 3)

// =============================================================================
// Memory Controller Definitions
// =============================================================================
#define MCU_MDCNFG      REG32(MCU_BASE,0x00)
#define MCU_MDREFR      REG32(MCU_BASE,0x04)
#define MCU_MSC0        REG32(MCU_BASE,0x08)
#define MCU_MSC1        REG32(MCU_BASE,0x0c)
#define MCU_MSC2        REG32(MCU_BASE,0x10)
#define MCU_MECR        REG32(MCU_BASE,0x14)
#define MCU_SXCNFG      REG32(MCU_BASE,0x1c)
#define MCU_MCMEM0      REG32(MCU_BASE,0x28)
#define MCU_MCMEM1      REG32(MCU_BASE,0x2c)
#define MCU_MCATT0      REG32(MCU_BASE,0x30)
#define MCU_MCATT1      REG32(MCU_BASE,0x34)
#define MCU_MCIO0       REG32(MCU_BASE,0x38)
#define MCU_MCIO1       REG32(MCU_BASE,0x3c)
#define MCU_MDMRS       REG32(MCU_BASE,0x40)
#define MCU_BOOT_DEF    REG32(MCU_BASE,0x44)
#define MCU_ARB_CNTL    REG32(MCU_BASE,0x48)
#define MCU_BSCNTR0     REG32(MCU_BASE,0x4c)
#define MCU_BSCNTR1     REG32(MCU_BASE,0x50)
#define MCU_MDMRSLP     REG32(MCU_BASE,0x58)
#define MCU_BSCNTR2     REG32(MCU_BASE,0x5c)
#define MCU_BSCNTR3     REG32(MCU_BASE,0x60)

// MDCNFG bit definitions
#define MDCNFG_DE0       (1 << 0)   // SDRAM partition 0 enable
#define MDCNFG_DE1       (1 << 1)   // SDRAM partition 1 enable
#define MDCNFG_DWID0_32  (0 << 2)   // Partition 0/1 width
#define MDCNFG_DWID0_16  (1 << 2)
#define MDCNFG_DCAC0_8   (0 << 3)   // Partition 0/1 column bits
#define MDCNFG_DCAC0_9   (1 << 3)
#define MDCNFG_DCAC0_10  (2 << 3)
#define MDCNFG_DCAC0_11  (3 << 3)   // Partition 0/1 row bits
#define MDCNFG_DRAC0_11  (0 << 5)
#define MDCNFG_DRAC0_12  (1 << 5)
#define MDCNFG_DRAC0_13  (2 << 5)
#define MDCNFG_DNB0_2    (0 << 7)   // Partition 0/1 banks
#define MDCNFG_DNB0_4    (1 << 7)
#define MDCNFG_DTC0_0    (0 << 8)   // Partition 0/1 clocks timing
#define MDCNFG_DTC0_1    (1 << 8)
#define MDCNFG_DTC0_2    (2 << 8)
#define MDCNFG_DTC0_3    (3 << 8)
#define MDCNFG_DTC0_3    (3 << 8)
#define MDCNFG_DADDR0    (1 << 10)  // Alternate addressing mode
#define MDCNFG_DLATCH0   (1 << 11)  // Return clock data latch
#define MDCNFG_DSA1110_0 (1 << 12)  // SA1110 addressing compatibility
#define MDCNFG_DE2       (1 << 16)   // SDRAM partition 0 enable
#define MDCNFG_DE3       (1 << 17)   // SDRAM partition 1 enable
#define MDCNFG_DWID2_32  (0 << 18)   // Partition 0/1 width
#define MDCNFG_DWID2_16  (1 << 19)
#define MDCNFG_DCAC2_8   (0 << 19)   // Partition 0/1 column bits
#define MDCNFG_DCAC2_9   (1 << 19)
#define MDCNFG_DCAC2_10  (2 << 19)
#define MDCNFG_DCAC2_11  (3 << 19)   // Partition 0/1 row bits
#define MDCNFG_DRAC2_11  (0 << 21)
#define MDCNFG_DRAC2_12  (1 << 21)
#define MDCNFG_DRAC2_13  (2 << 21)
#define MDCNFG_DNB2_2    (0 << 23)   // Partition 0/1 banks
#define MDCNFG_DNB2_4    (1 << 23)
#define MDCNFG_DTC2_0    (0 << 24)   // Partition 0/1 clocks timing
#define MDCNFG_DTC2_1    (1 << 24)
#define MDCNFG_DTC2_2    (2 << 24)
#define MDCNFG_DTC2_3    (3 << 24)
#define MDCNFG_DTC2_3    (3 << 24)
#define MDCNFG_DADDR2    (1 << 26)  // Alternate addressing mode
#define MDCNFG_DLATCH2   (1 << 27)  // Return clock data latch
#define MDCNFG_DSA1110_2 (1 << 28)  // SA1110 addressing compatibility

// MDMRS bit definitions
#define MDMRS0_SHIFT     7
#define MDMRS2_SHIFT     23

// MDMRSLP bit definitions
#define MDMRSLP0_SHIFT   0
#define MDMRSLP_MDLPEN0  (1 << 15)
#define MDMRSLP2_SHIFT   16
#define MDMRSLP_MDLPEN2  (1 << 31)

// MDREFR bit definitions
#define MDREFR_K0RUN     (1 << 13)
#define MDREFR_K0DB2     (1 << 14)
#define MDREFR_E1PIN     (1 << 15)
#define MDREFR_K1RUN     (1 << 16)
#define MDREFR_K1DB2     (1 << 17)
#define MDREFR_K2RUN     (1 << 18)
#define MDREFR_K2DB2     (1 << 19)
#define MDREFR_APD       (1 << 20)
#define MDREFR_SLFRSH    (1 << 22)
#define MDREFR_K0FREE    (1 << 23)
#define MDREFR_K1FREE    (1 << 24)
#define MDREFR_K2FREE    (1 << 25)
#define MDREFR_K0DB4     (1 << 29)

// SXCNFG bit definitions
#define SXCNFG_SXEN0     (1 << 0)
#define SXCNFG_SXEN1     (1 << 1)
#define SXCNFG_SXCL0_3   (2 << 2)
#define SXCNFG_SXCL0_4   (3 << 2)
#define SXCNFG_SXCL0_5   (4 << 2)
#define SXCNFG_SXCL0_6   (5 << 2)
#define SXCNFG_SXCL0_7   (6 << 2)
#define SXCNFG_SXCL0_8   (7 << 2)
#define SXCNFG_SXCL0_9   (8 << 2)
#define SXCNFG_SXCL0_10  (9 << 2)
#define SXCNFG_SXCL0_11  (10 << 2)
#define SXCNFG_SXCL0_12  (11 << 2)
#define SXCNFG_SXCL0_13  (12 << 2)
#define SXCNFG_SXCL0_14  (13 << 2)
#define SXCNFG_SXCL0_15  (14 << 2)
#define SXCNFG_SXCL0_16  (15 << 2)
#define SXCNFG_SXTP0_8   (2 << 12)
#define SXCNFG_SXTP0_16  (3 << 12)
#define SXCNFG_SXLATCH0  (1 << 14)
#define SXCNFG_SXCLEXT0  (1 << 15)
#define SXCNFG_SXEN2     (1 << 16)
#define SXCNFG_SXEN3     (1 << 17)
#define SXCNFG_SXCL2_3   (2 << 18)
#define SXCNFG_SXCL2_4   (3 << 18)
#define SXCNFG_SXCL2_5   (4 << 18)
#define SXCNFG_SXCL2_6   (5 << 18)
#define SXCNFG_SXCL2_7   (6 << 18)
#define SXCNFG_SXCL2_8   (7 << 18)
#define SXCNFG_SXCL2_9   (8 << 18)
#define SXCNFG_SXCL2_10  (9 << 18)
#define SXCNFG_SXCL2_11  (10 << 18)
#define SXCNFG_SXCL2_12  (11 << 18)
#define SXCNFG_SXCL2_13  (12 << 18)
#define SXCNFG_SXCL2_14  (13 << 18)
#define SXCNFG_SXCL2_15  (14 << 18)
#define SXCNFG_SXCL2_16  (15 << 18)
#define SXCNFG_SXTP2_8   (2 << 28)
#define SXCNFG_SXTP2_16  (3 << 28)
#define SXCNFG_SXLATCH2  (1 << 30)
#define SXCNFG_SXCLEXT2  (1 << 31)

// MSCX bit definitions
#define MSCX_RT_NOBURST  (0 << 0)
#define MSCX_RT_SRAM     (1 << 0)
#define MSCX_RT_BURST4   (2 << 0)
#define MSCX_RT_BURST8   (3 << 0)
#define MSCX_RT_VLIO     (4 << 0)
#define MSCX_RBW_32      (0 << 3)
#define MSCX_RBW_16      (1 << 3)
#define MSCX_RDF_SHIFT   4
#define MSCX_RDN_SHIFT   8
#define MSCX_RRR_SHIFT   12
#define MSCX_RBUFF       (1 << 15)


// =============================================================================
// Clocks Definitions
// =============================================================================
#define CLOCK_CCCR      REG32(CLOCK_BASE,0x00)
#define CLOCK_CKEN      REG32(CLOCK_BASE,0x04)
#define CLOCK_OSCC      REG32(CLOCK_BASE,0x08)
#define CLOCK_CCSR      REG32(CLOCK_BASE,0x0c)

// CCCR bit definitions
#define CCCR_2N_SHIFT   7
#define CCCR_PPDIS      (1 << 30)
#define CCCR_CPDIS      (1 << 31)

// CKEN bit definitions
#define CKEN_PWM02      (1 << 0)
#define CKEN_PWM13      (1 << 1)
#define CKEN_AC97       (1 << 2)
#define CKEN_SSP2       (1 << 3)
#define CKEN_SSP3       (1 << 4)
#define CKEN_STUART     (1 << 5)
#define CKEN_FFUART     (1 << 6)
#define CKEN_BTUART     (1 << 7)
#define CKEN_I2S        (1 << 8)
#define CKEN_OSTIMER    (1 << 9)
#define CKEN_USBH       (1 << 10)
#define CKEN_USBC       (1 << 11)
#define CKEN_MMC        (1 << 12)
#define CKEN_ICP        (1 << 13)
#define CKEN_I2C        (1 << 14)
#define CKEN_POWER_I2C  (1 << 15)
#define CKEN_LCD        (1 << 16)
#define CKEN_MSL        (1 << 17)
#define CKEN_USIM       (1 << 18)
#define CKEN_KEYPAD     (1 << 19)
#define CKEN_MEMSTICK   (1 << 21)
#define CKEN_MCU        (1 << 22)
#define CKEN_SSP1       (1 << 23)

// OSCC bit definitions
#define OSCC_OOK        (1 << 0)
#define OSCC_OON        (1 << 1)
#define OSCC_TOUT_EN    (1 << 2)
#define OSCC_PIO_EN     (1 << 3)
#define OSCC_CRI        (1 << 4)

//--------------------------------------------------------------
#endif // CYGONCE_HAL_ARM_XSCALE_HAL_BULVERDE_H
// EOF hal_bulverde.h
