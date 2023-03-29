/*=============================================================================
//
//      hal_xscale.h
//
//      XScale Core I/O module support.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Contributors: msalter, dmoseley
// Date:         2002-10-18
// Purpose:      
// Description:  XScale core I/O modules support.
// Usage:        #include <cyg/hal/hal_xscale.h>
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_H

#include <pkgconf/system.h>
#include <pkgconf/hal_arm_xscale_core.h>
#include <cyg/hal/hal_hotdebug.h>       // JTAG Hot Debug code macro

#ifdef __ASSEMBLER__
	// Useful CPU macros

	// Delay a bit
	.macro DELAY_FOR cycles, reg0
	ldr	\reg0, =\cycles
	subs	\reg0, \reg0, #1
	subne	pc,  pc, #0xc
	.endm
	
	// wait for coprocessor write complete
	.macro CPWAIT reg
        mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm

	// Enable the BTB
	.macro BTB_INIT reg
	mrc	p15, 0, \reg, c1, c0, 0
#ifdef CYGSEM_HAL_ARM_XSCALE_BTB
	orr	\reg, \reg, #MMU_Control_BTB
#else
	bic	\reg, \reg, #MMU_Control_BTB
#endif
	mcr	p15, 0, \reg, c1, c0, 0
	CPWAIT  \reg
	.endm
#else
static inline void CPWAIT(void) {
    cyg_uint32 tmp;
    asm volatile ("mrc  p15,0,%0,c2,c0,0\n"
		  "mov  %0,%0\n"
		  "sub  pc,pc,#4" : "=r" (tmp));
}
#endif

// Override the default MMU off code. This is intended
// to be included in an inline asm statement.
#define CYGARC_HAL_MMU_OFF(__paddr__)        \
              "   mrc p15,0,r0,c1,c0,0\n"    \
              "   bic r0,r0,#0x05\n"         \
              "   b 99f\n"                   \
              "   .p2align 5\n"              \
              "99:\n"                        \
              "   mcr p15,0,r0,c1,c0,0\n"    \
              "   mrc p15,0,r0,c2,c0,0\n"    \
              "   mov r0,r0\n"	             \
              "   sub pc,pc,#4\n"            \
              "   mov pc," #__paddr__ "\n"

#ifdef __ASSEMBLER__

#define REG8(a,b)  (b)
#define REG16(a,b) (b)
#define REG32(a,b) (b)

#else /* __ASSEMBLER__ */

#define REG8(a,b)  ((volatile unsigned char *)((a)+(b)))
#define REG16(a,b) ((volatile unsigned short *)((a)+(b)))
#define REG32(a,b) ((volatile unsigned int *)((a)+(b)))

extern void hal_xscale_core_init(void);
#endif /* __ASSEMBLER__ */

#ifdef   CYGSEM_HAL_ARM_HOTDEBUG
//       Define hot debug macros
#define  CYGHWR_HAL_ARM_HOTDEBUG         _xscale_hotdebug
#define  CYGHWR_HAL_ARM_HOTDEBUG_ENABLE  _xscale_hotdebug_enable
#if defined(CYG_HAL_STARTUP_ROM)
#define  CYGHWR_HAL_ARM_HOTDEBUG_LOCK_VECTORS                                                     \
         _xscale_hotdebug_clean_data_cache     MemoryCleanArea = DCACHE_FLUSH_AREA              ; \
         _xscale_hotdebug_lock_exceptions_into_icache _have_MMU_TABLE=1 _pMMU_TABLE = (SDRAM_BASE + 0x4000);
#elif defined (CYG_HAL_STARTUP_ROMRAM) // defined(CYG_HAL_STARTUP_ROMRAM)
#define  CYGHWR_HAL_ARM_HOTDEBUG_LOCK_VECTORS                                                     \
         _xscale_hotdebug_clean_data_cache     MemoryCleanArea = DCACHE_FLUSH_AREA              ; \
         _xscale_hotdebug_lock_exceptions_into_icache _have_MMU_TABLE=1 _pMMU_TABLE = (mmu_table + SDRAM_PHYS_BASE ) ;
#else // defined (CYG_HAL_STARTUP_RAM)
#define  CYGHWR_HAL_ARM_HOTDEBUG_LOCK_VECTORS                                                     \
         _xscale_hotdebug_clean_data_cache     MemoryCleanArea = DCACHE_FLUSH_AREA              ; \
         _xscale_hotdebug_lock_exceptions_into_icache _have_MMU_TABLE=0 _pMMU_TABLE = mmu_table ;
#endif

#endif


//--------------------------------------------------------------
#endif // CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_H
// EOF hal_xscale.h
