#ifndef CYGONCE_HAL_PLATFORM_EXTRAS_H
#define CYGONCE_HAL_PLATFORM_EXTRAS_H

/*=============================================================================
//
//      hal_platform_extras.h
//
//      Platform specific MMU table.
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
// Date:         2001-12-03
// Purpose:      Intel XScale Mainstone platform specific mmu table
// Description: 
// Usage:        #include <cyg/hal/hal_platform_extras.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#if defined(CYG_HAL_STARTUP_ROM)
        .section .mmu_tables, "a"

    mmu_table:
        //  This page table sets up the preferred mapping:
        //
        //  Virtual Address   Physical Address  XCB  Size (MB)  Description
        //  ---------------   ----------------  ---  ---------  -----------
        //     0x00000000       0x00000000      010      64     Boot flash
        //     0x04000000       0x04000000      010      64     App flash
        //     0x08000000       0x08000000      000       1     Board Registers
        //     0x0A000000       0x0A000000      000       2     SRAM
        //     0x10000000       0x10000000      000       1     Ethernet
        //     0x14000000       0x14000000      000      64     Expansion Board
        //     0x20000000       0x20000000      000     256     PCMCIA Slot 0
        //     0x30000000       0x30000000      000     256     PCMCIA Slot 1
        //     0x40000000       0x40000000      000      64     Peripheral MMR
        //     0x44000000       0x44000000      000      64     LCD MMR
        //     0x48000000       0x48000000      000      64     MCU MMR
        //     0x4C000000       0x4C000000      000      64     USB Host MMR
        //     0x58000000       0x58000000      000      64     IM Control
        //     0x5C000000       0x5C000000      000      64     IMem Storage
        //     0xA0000000       0xA0000000      010     256     SDRAM
        //     0xB0000000       0xB0000000      000     256     Uncached SDRAM
        //     0xC0000000       0xC0000000      010       1     Cache flush
	//

	// 512MB ECC DRAM
	// In flash based table: X=1, C=1, B=1, ECC
	.set	__base,0xA00
	.rept	0x200 - 0x000
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr

	// 2048MB ATU Outbound direct
	// X=0, Non-cacheable, Non-bufferable
	.set	__base,0x000
	.rept	0xA00 - 0x200
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 257MB ATU Outbound translation
	// X=0, Non-cacheable, Non-bufferable
	.rept	0xB01 - 0xA00
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Nothing here
	.rept	0xC00 - 0xB01
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// Uncached/unbuffered alias for 512MB ECC DRAM
	.set	__base,0xA00
	.rept	0xE00 - 0xC00
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// Cache flush region.
	// Don't need physical memory, just a cached area.
	.set	__base,0xE00
	.rept	0xE01 - 0xE00
	FL_SECTION_ENTRY __base,1,3,0,0,1,1
	.set	__base,__base+1
	.endr
	
	// Nothing here
	.rept	0xF00 - 0xE01
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// 8MB of FLASH
	// X=0, Cacheable, Non-bufferable
	.rept	0xF08 - 0xF00
	FL_SECTION_ENTRY __base,0,3,0,0,1,0
	.set	__base,__base+1
	.endr	

	// Nothing here
	.rept	0xFE8 - 0xF08
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// PBIU CS1 - CS5
	// X=0, Non-cacheable, Non-bufferable
	.rept	0xFE9 - 0xFE8
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// Nothing here
	.rept	0xFFF - 0xFE9
	FL_SECTION_ENTRY __base,0,0,0,0,0,0
	.set	__base,__base+1
	.endr

	// Verde PMMR
	// X=0, Non-cacheable, Non-bufferable
	.rept	0x1000 - 0xFFF
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr
#endif /* CYG_HAL_STARTUP_ROM */

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
