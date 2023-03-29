#ifndef CYGONCE_HAL_ARM_XSCALE_MAINSTONE_MAINSTONE_H
#define CYGONCE_HAL_ARM_XSCALE_MAINSTONE_MAINSTONE_H

/*=============================================================================
//
//      Mainstone.h
//
//      Platform specific support (register layout, etc)
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
// Date:         2002-07-15
// Purpose:      Intel Mainstone specific support routines
// Description: 
// Usage:        #include <cyg/hal/mainstone.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_mainstone.h>
#include <cyg/hal/hal_bulverde.h>  // IO defines

#define MAINSTONE_BOOT_FLASH_PHYS_BASE     0x00000000   // CS0
#define MAINSTONE_BOOT_FLASH_BASE          0xf0000000

#define MAINSTONE_APP_FLASH_PHYS_BASE      0x04000000   // CS1
#define MAINSTONE_APP_FLASH_BASE           0xf2000000

#define MAINSTONE_REGS_PHYS_BASE           0x08000000   // CS2
#define MAINSTONE_REGS_BASE                0x08000000

#define MAINSTONE_SRAM_PHYS_BASE           0x0a000000   // CS2
#define MAINSTONE_SRAM_BASE                0x0a000000

#define MAINSTONE_ETHERNET_PHYS_BASE       0x10000000   // CS4
#define MAINSTONE_ETHERNET_BASE            0x10000000

#define MAINSTONE_EXPANSION_PHYS_BASE      0x14000000   // CS5
#define MAINSTONE_EXPANSION_BASE           0x14000000

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_UNCACHED_BASE                0xa0000000

#define DCACHE_FLUSH_PHYS_BASE             0xa8000000
#define DCACHE_FLUSH_AREA                  0xa8000000

#define SDRAM_PHYS_BASE                    0xa0000000
#define SDRAM_BASE                         0x00000000
#define SDRAM_SIZE                         0x04000000  // 64MB

// Board registers.
#define MAINSTONE_LEDDAT1     REG32(MAINSTONE_REGS_BASE,0x10)
#define MAINSTONE_LEDDAT2     REG32(MAINSTONE_REGS_BASE,0x14)
#define MAINSTONE_LEDCTRL     REG32(MAINSTONE_REGS_BASE,0x40)
#define MAINSTONE_GPSWR       REG32(MAINSTONE_REGS_BASE,0x60)
#define MAINSTONE_MSCWR1      REG32(MAINSTONE_REGS_BASE,0x80)
#define MAINSTONE_MSCWR2      REG32(MAINSTONE_REGS_BASE,0x84)
#define MAINSTONE_MSCRD       REG32(MAINSTONE_REGS_BASE,0x90)
#define MAINSTONE_INTMSKENA   REG32(MAINSTONE_REGS_BASE,0xc0)
#define MAINSTONE_INTSETCLR   REG32(MAINSTONE_REGS_BASE,0xd0)
#define MAINSTONE_PCMCIA0     REG32(MAINSTONE_REGS_BASE,0xe0)
#define MAINSTONE_PCMCIA1     REG32(MAINSTONE_REGS_BASE,0xe4)


// -------- MCU CSx values --------
//

// Boot Flash
#define CS0_CONFIG  (MSCX_RBUFF | (2 << MSCX_RRR_SHIFT) | (7 << MSCX_RDN_SHIFT) \
	                  | (10 << MSCX_RDF_SHIFT) | MSCX_RBW_32 | MSCX_RT_BURST8)
// Application Flash
#define CS1_CONFIG  ((3 << MSCX_RRR_SHIFT) | (9 << MSCX_RDN_SHIFT) \
	                  | (15 << MSCX_RDF_SHIFT) | MSCX_RBW_32 | MSCX_RT_BURST4)
// Baseboard Regs / SRAM
#define CS2_CONFIG  (MSCX_RBUFF | (2 << MSCX_RRR_SHIFT) | (6 << MSCX_RDN_SHIFT) \
	                  | (9 << MSCX_RDF_SHIFT) | MSCX_RBW_32 | MSCX_RT_SRAM)
// Not used
#define CS3_CONFIG  0

// Ethernet
#define CS4_CONFIG  (MSCX_RBUFF | (3 << MSCX_RRR_SHIFT) | (8 << MSCX_RDN_SHIFT) \
	                  | (11 << MSCX_RDF_SHIFT) | MSCX_RBW_32 | MSCX_RT_VLIO)
// Not used
#define CS5_CONFIG  0
	
// -------- misc. MCU values --------
//

#define MDREFR_CONFIG  0x00000018       // (CLK_MEM cycles - 31) / 32
#define MECR_CONFIG    0x00000001
#define MCMEM0_CONFIG  0x00010204
#define MCMEM1_CONFIG  0x00010204
#define MCATT0_CONFIG  0x00010504
#define MCATT1_CONFIG  0x00010504
#define MCIO0_CONFIG   0x0000C108
#define MCIO1_CONFIG   0x0001C108
#define FLYCNFG_CONFIG 0x00000000
#define MDMRSLP_CONFIG ((0x4008 << MDRSLP0_SHIFT) | MDLPEN0)
#define SXCNFG_CONFIG  0x40044004
#define MDCNFG_CONFIG  0x00000AC9
#define MDMRS_CONFIG   0x00000000


// ------------------------------------------------------------------------
// 8 Digit Hex Display
//
// The board has two 4 hex digit display controlled by three board registers.
// Each digit has a decimal point and may be individually blanked.

#ifdef __ASSEMBLER__
        // Display hex digits in 'value' not masked by 'mask'.
	.macro HEX_DISPLAY reg0, reg1, value
	mov     \reg0,#MAINSTONE_REGS_BASE
	ldr     \reg1,=\value
	str     \reg1, [\reg0, #MAINSTONE_LEDDAT1]
#if 0
	// delay
        ldr     \reg0, =0x780000
    0:
        subs    \reg0, \reg0, #1
        bne     0b
#endif
	.endm
#else
static inline void HEX_DISPLAY(int value)
{
    *MAINSTONE_LEDDAT1 = value;
}
#endif // __ASSEMBLER__

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_MAINSTONE_MAINSTONE_H
// EOF mainstone.h
