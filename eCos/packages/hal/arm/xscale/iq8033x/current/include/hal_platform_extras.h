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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-07-10
// Purpose:      Intel XScale IQ80331/IQ80332 CRB platform specific mmu table
// Description: 
// Usage:        #include <cyg/hal/hal_platform_extras.h>
//     Only used by "vectors.S"         
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#if defined(CYG_HAL_STARTUP_ROM)

#include <cyg/hal/iq8033x.h>

#ifndef CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE
#define CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE 0
#endif

        .section .mmu_tables, "a"

        // Note that configuring SDRAM at Physical Address 0
        // requires disabling the ATU direct outbound window

    mmu_table:
        //  Virtual Address   Physical Address  Size (MB)  Description
        //  ---------------   ----------------  ---------  -----------
        //     0x00000000                *        2048*    SDRAM - 64-bit ECC
        //     0x80000000       0x80000000         128     ATU Outbound Memory Translation Windows
        //     0x88000000       ----------         128     Unused
        //     0x90000000       0x90000000           1     ATU Outbound I/O Translation Window
        //     0x90100000       ----------         255     Unused
        //     0xA0000000               **         512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0xC0000000           8     Flash (PCE0#)
        //     0xC0800000       ----------         224     Unused
        //     0xCE800000       0xCE800000           1     PCE1# - Uncached
        //     0xCE900000       ----------          23     Unused
        //     0xD0000000              ***         512*    SDRAM - 32-bit ECC
        //     0xF0000000       0xF0000000           1     Cache Flush
        //     0xF0100000       ----------         254     Unused
        //     0xFFF00000       0xFFF00000           1     PMMR
        //
        //
        //  * - The 64-bit ECC SDRAM cached region will start at Virtual Address 0x00000000
        //      and Physical Address CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2
        //
        //  2048* - the page tables will setup a region for cached SDRAM at 0,
        //          of the size 2GB - CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2.
        //          Additionally the MCU Base Registers may further restrict the usable
        //          space depending on the actual size of the DIMM installed.
        //          Any unneeded page table entries will be set to unused.
        //
        //  ** - The 64-bit ECC SDRAM uncached region will start at Virtual Address 0xA0000000
        //       and Physical Address CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2
        //
        //  *** - The 32-bit ECC SDRAM region will start at Virtual Address 0xD0000000
        //        and Physical Address 0 but may not exist if
        //        CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE == 0
        //
        //  512* - the page tables will setup a region for 32-bit ECC SDRAM at 0xA0000000,
        //         of the size CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE.
        //         Additionally the MCU Base Registers may further restrict the usable
        //         space depending on the actual size of the DIMM installed.
        //         Any unneeded page table entries will be set to unused.
        //
        //  =====================================================================
        //  EXAMPLE:
        //  If CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE == 0
        //  =====================================================================
        //
        //  Virtual Address   Physical Address  Size (MB)  Description
        //  ---------------   ----------------  ---------  -----------
        //     0x00000000       0x00000000        2048     SDRAM - 64-bit ECC
        //     0x80000000       0x80000000         128     ATU Outbound Memory Translation Windows
        //     0x88000000       ----------         128     Unused
        //     0x90000000       0x90000000           1     ATU Outbound I/O Translation Window
        //     0x90100000       ----------         255     Unused
        //     0xA0000000       0x00000000         512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0xC0000000           8     Flash (PCE0#)
        //     0xC0800000       ----------         224     Unused
        //     0xCE800000       0xCE800000           1     PCE1# - Uncached
        //     0xCE900000       ----------          23     Unused
        //     0xD0000000       ----------         512     Unused
        //     0xF0000000       0xF0000000           1     Cache Flush
        //     0xF0100000       ----------         254     Unused
        //     0xFFF00000       0xFFF00000           1     PMMR
        //
        //  =====================================================================
        //  EXAMPLE:
        //  If CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE == 32
        //  =====================================================================
        //
        //  Virtual Address   Physical Address  Size (MB)  Description
        //  ---------------   ----------------  ---------  -----------
        //     0x00000000       0x04000000        1984     SDRAM - 64-bit ECC
        //     0x7C000000       ----------          64     Unused
        //     0x80000000       0x80000000         128     ATU Outbound Memory Translation Windows
        //     0x88000000       ----------         128     Unused
        //     0x90000000       0x90000000           1     ATU Outbound I/O Translation Window
        //     0x90100000       ----------         255     Unused
        //     0xA0000000       0x04000000         512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0xC0000000           8     Flash (PCE0#)
        //     0xC0800000       ----------         224     Unused
        //     0xCE800000       0xCE800000           1     PCE1# - Uncached
        //     0xCE900000       ----------          23     Unused
        //     0xD0000000       0x00000000          32     SDRAM - 32-bit ECC
        //     0xD2000000       ----------         480     Unused
        //     0xF0000000       0xF0000000           1     Cache Flush
        //     0xF0100000       ----------         254     Unused
        //     0xFFF00000       0xFFF00000           1     PMMR
        //
        //  =====================================================================
        //  EXAMPLE:
        //  If CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE == 512
        //  =====================================================================
        //
        //  Virtual Address   Physical Address  Size (MB)  Description
        //  ---------------   ----------------  ---------  -----------
        //     0x00000000       0x40000000        1024     SDRAM - 64-bit ECC
        //     0x40000000       ----------        1024     Unused
        //     0x80000000       0x80000000         128     ATU Outbound Memory Translation Windows
        //     0x88000000       ----------         128     Unused
        //     0x90000000       0x90000000           1     ATU Outbound I/O Translation Window
        //     0x90100000       ----------         255     Unused
        //     0xA0000000       0x40000000         512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0xC0000000           8     Flash (PCE0#)
        //     0xC0800000       ----------         224     Unused
        //     0xCE800000       0xCE800000           1     PCE1# - Uncached
        //     0xCE900000       ----------          23     Unused
        //     0xD0000000       0x00000000         512     SDRAM - 32-bit ECC
        //     0xF0000000       0xF0000000           1     Cache Flush
        //     0xF0100000       ----------         254     Unused
        //     0xFFF00000       0xFFF00000           1     PMMR

        // 64-Bit ECC SDRAM
	// Size == SDRAM_MAX_MB - CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE * 2
	// X=0, C=1, B=1, ECC
        //    -- note that this sets the caching policy for SDRAM to Read Allocate
        //       This greatly improves the speed of the ECC scrub.  However for general
        //       usage, read-write allocate performs better so we will change the caching
        //       policy after having copied the page tables to SDRAM.
        .set	__base,((SDRAM_PHYS_BASE >> 20) + CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2)
        .rept	SDRAM_MAX_MB - (CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2)
	FL_SECTION_ENTRY __base,0,3,1,0,1,1
	.set	__base,__base+1
	.endr

	// Hole to 2GB -- Unused
        // Size == CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2
        .rept	2048 - (SDRAM_MAX_MB - CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2)
	FL_UNMAPPED_ENTRY
	.endr

	// 128MB ATU Outbound Memory Translation Windows
	// X=0, C=0, B=0. Non-ECC
	.set	__base,0x800
	.rept	128
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 128MB Unused
	.rept	128
	FL_UNMAPPED_ENTRY
	.endr

	// 1MB ATU Outbound I/O Translation Window
	// X=0, C=0, B=0. Non-ECC
	.set	__base,0x900
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 255MB Unused
	.rept	255
	FL_UNMAPPED_ENTRY
	.endr

        // 64-Bit ECC SDRAM - Uncached/Unbuffered Alias
	// Size == 512MB
	// X=0, C=0, B=0, ECC
        .set	__base,((SDRAM_PHYS_BASE >> 20) + CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2)
        .rept	512
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// 8MB FLASH(PCE0#)
	// X=1, C=1, B=1
        .set	__base,(PCE0_PHYS_BASE >> 20)
	.rept	8
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr	

	// 224MB Unused
	.rept	224
	FL_UNMAPPED_ENTRY
	.endr

	// 1MB PCE1#
	// X=0, C=0, B=0
        .set	__base,(PCE1_PHYS_BASE >> 20)
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 23MB Unused
	.rept	23
	FL_UNMAPPED_ENTRY
	.endr

        // 32-Bit ECC SDRAM - Uncached/Unbuffered Alias
	// Size == CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE
	// X=0, C=0, B=0, ECC
        .set	__base,(SDRAM_PHYS_BASE >> 20)
	.rept	CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// Hole between 32-bit ECC SDRAM and Cache Flush Region
	.rept	512 - CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE
	FL_UNMAPPED_ENTRY
	.endr

	// 1MB Cache flush region.
	// X=1, C=1, B=1, Non-ECC
	.set	__base,0xF00
	.rept	1
	FL_SECTION_ENTRY __base,1,3,0,0,1,1
	.set	__base,__base+1
	.endr

	// 254MB Unused
	.rept	254
	FL_UNMAPPED_ENTRY
	.endr
	
	// 1MB PMMR
	// X=0, C=0, B=0
	.set	__base,0xFFF
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

#endif /* CYG_HAL_STARTUP_ROM */

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
