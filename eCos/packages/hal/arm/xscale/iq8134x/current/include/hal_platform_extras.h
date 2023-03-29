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
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:      Intel XScale IQ8134 CRB platform specific mmu table
// Description: 
// Usage:        #include <cyg/hal/hal_platform_extras.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#if defined(CYG_HAL_STARTUP_ROM) || defined (CYG_HAL_STARTUP_ROMRAM) || defined (CYG_HAL_STARTUP_RAMDUAL)

#include <cyg/hal/iq8134x.h>

#ifndef CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE
#define CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE 0
#endif

        .section .mmu_tables, "a"

    mmu_table:
	// [ROMRAM] Version
        //  Virtual Address   Physical Address  Size (MB)   Description
        //  ---------------   ----------------  ---------   -----------
        //     0x00000000       0x0.0000.0000      2048     SDRAM - 64-bit ECC
        //     0x80000000       0x1.8000.0000       128     ATU-X Outbound Memory Translation Window 
        //     0x88000000       0x2.8800.0000       128     ATUe Outbound Memory Translation Window                
        //     0x90000000       0x0.9000.0000         1     ATU-X Outbound IO Window
        //     0x90100000       0x0.9010.0000         1     ATUe Outbound IO Window 
	//     0x90200000       ----------          254		Unused
        //     0xA0000000       0x0.0000.0000       512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0x0.C000.0000         1     Cache Flush, DCache Lock
        //     0xC0100000       ----------          255     Unused
        //     0xD0000000       ----------          512     Unused (32-bit ECC not enabled)
        //     0xF0000000       0x0.F000.0000        32     Flash (PCE0#)
        //     0xF2000000       0x0.F200.0000         1     PCE1#        
	//     0xF2100000       0x0.F200.0000         1		PCE1# - Cached/Buffered
	//     0xF2200000       ----------           14     Unused
        //     0xF3000000       0x0.F300.0000         1     Compact Flash (Maple Creek Only)
        //     0xF3100000       ----------          204     Unused
        //     0xFFD00000       0x0.FFD0.0000         1     MMR
	//     0xFFE00000       0x0.FFE0.0000         2     Unused 
	//
	// [ROM] Version   
        //  Virtual Address   Physical Address  Size (MB)   Description
        //  ---------------   ----------------  ---------   -----------
        //     0x00000000       0x0.0000.0000      2048     SDRAM - 64-bit ECC
        //     0x80000000       0x1.8000.0000       128     ATU-X Outbound Memory Translation Window 
        //     0x88000000       0x2.8800.0000       128     ATUe Outbound Memory Translation Window                
        //     0x90000000       0x0.9000.0000         1     ATU-X Outbound IO Window
        //     0x90100000       0x0.9010.0000         1     ATUe Outbound IO Window 
	//     0x90200000       ----------          254		Unused
        //     0xA0000000       0x0.0000.0000       512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0x0.C000.0000         1     Cache Flush, DCache Lock
        //     0xC0100000       ----------          255     Unused
        //     0xD0000000       ----------          512     Unused (32-bit ECC not enabled)
        //     0xF0000000       0x0.F000.0000        32     Flash (PCE0#)
        //     0xF2000000       0x0.F200.0000         1     PCE1#        
	//     0xF2100000       0x0.F210.0000        15		Unused
        //     0xF3000000       0x0.F300.0000         1     Compact Flash (Maple Creek Only)
        //     0xF3100000       ----------          204     Unused
        //     0xFFD00000       0x0.FFD0.0000         1     MMR
	//     0xFFE00000       0x0.FFE0.0000         1     SRAM   
	//     0xFFF00000       0x0.FFF0.0000         1     Unused
        //
        //  * - The 64-bit ECC SDRAM cached region will start at Virtual Address 0x00000000
        //      and Physical Address (0x0.0000.0000 + CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2)
        //
        //  1024* - the page tables will setup a region for cached SDRAM at 0,
        //          of the size 2GB - CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2.
        //          Additionally the MCU Base Registers may further restrict the usable
        //          space depending on the actual size of the DIMM installed.
        //          Any unneeded page table entries will be set to unused.
        //
        //  ** - The 64-bit ECC SDRAM uncached region will start at Virtual Address 0xA0000000
        //       and Physical Address (0x0.0000.0000 + CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2)
        //
        //  *** - The 32-bit ECC SDRAM region will start at Virtual Address 0xD0000000
        //        and Physical Address 0x0.0000.0000 but may not exist if
        //        CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE == 0
        //
        //  512* - the page tables will setup a region for 32-bit ECC SDRAM at 0xA0000000,
        //         of the size CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE.
        //         Additionally the MCU Base Registers may further restrict the usable
        //         space depending on the actual size of the DIMM installed.
        //         Any unneeded page table entries will be set to unused.
        //
        //  =====================================================================
        //  EXAMPLE:
        //  If CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE == 0
        //  =====================================================================
        //  Virtual Address   Physical Address  Size (MB)   Description
        //  ---------------   ----------------  ---------   -----------
        //     0x00000000       0x0.0000.0000      2048     SDRAM - 64-bit ECC
        //     0x80000000       0x1.8000.0000       128     ATU-X Outbound Memory Translation Window 
        //     0x88000000       0x2.8800.0000       128     ATUe Outbound Memory Translation Window                
        //     0x90000000       0x0.9000.0000         1     ATU-X Outbound IO Window
        //     0x90100000       0x0.9010.0000         1     ATUe Outbound IO Window 
	//     0x90200000       ----------          254		Unused
        //     0xA0000000       0x0.0000.0000       512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0x0.C000.0000         1     Cache Flush, DCache Lock
        //     0xC0100000       ----------          255     Unused
        //     0xD0000000       ----------          512     Unused (32-bit ECC not enabled)
        //     0xF0000000       0x0.F000.0000        32     Flash (PCE0#)
        //     0xF2000000       0x0.F200.0000         1     PCE1#        
	//     0xF2100000       0x0.F210.0000        15		Unused
        //     0xF3000000       0x0.F300.0000         1     Compact Flash (Maple Creek Only)
        //     0xF3100000       ----------          204     Unused
        //     0xFFD00000       0x0.FFD0.0000         1     MMR
	//     0xFFE00000       0x0.FFE0.0000         1     SRAM  
	//     0xFFF00000       ----------            1     Unused
        //
        //  =====================================================================
        //  EXAMPLE:
        //  If CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE == 32
        //  =====================================================================
        //  Virtual Address   Physical Address  Size (MB)   Description
        //  ---------------   ----------------  ---------   -----------
        //     0x00000000       0x0.0400.0000      1984     SDRAM - 64-bit ECC
        //     0x7C000000       ----------           64     Unused
        //     0x80000000       0x1.8000.0000       128     ATU-X Outbound Memory Translation Window 
        //     0x88000000       0x2.8800.0000       128     ATUe Outbound Memory Translation Window                
        //     0x90000000       0x0.9000.0000         1     ATU-X Outbound IO Window
        //     0x90100000       0x0.9010.0000         1     ATUe Outbound IO Window 
	//     0x90200000       ----------          254		Unused
        //     0xA0000000       0x0.0000.0000       512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0x0.C000.0000         1     Cache Flush, DCache Lock
        //     0xC0100000       ----------          255     Unused
        //     0xD0000000       0x0.0000.0000        32     SDRAM - 32-bit ECC
        //     0xF0000000       0x0.F000.0000        32     Flash (PCE0#)
        //     0xF2000000       0x0.F200.0000         1     PCE1#        
	//     0xF2100000       0x0.F210.0000        15		Unused
        //     0xF3000000       0x0.F300.0000         1     Compact Flash (Maple Creek Only)
        //     0xF3100000       ----------          204     Unused
        //     0xFFD00000       0x0.FFD0.0000         1     MMR
	//     0xFFE00000       0x0.FFE0.0000         1     SRAM  
	//     0xFFF00000       ----------            1     Unused
        //

        //  =====================================================================
        //  EXAMPLE:
        //  If CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE == 256
        //  =====================================================================
        //
        //  Virtual Address   Physical Address  Size (MB)  Description
        //  ---------------   ----------------  ---------  -----------
        //     0x00000000       0x0.2000.0000      1536     SDRAM - 64-bit ECC
        //     0x60000000       ----------          256     Unused
        //     0x80000000       0x1.8000.0000       128     ATU-X Outbound Memory Translation Window 
        //     0x88000000       0x2.8800.0000       128     ATUe Outbound Memory Translation Window                
        //     0x90000000       0x0.9000.0000         1     ATU-X Outbound IO Window
        //     0x90100000       0x0.9010.0000         1     ATUe Outbound IO Window 
	//     0x90200000       ----------          254		Unused
        //     0xA0000000       0x0.A000.0000       512     SDRAM - 64-bit ECC - Uncached Alias
        //     0xC0000000       0x0.C000.0000         1     Cache Flush, DCache Lock
        //     0xC0100000       ----------          255     Unused
        //     0xD0000000       0x0.0000.0000       256     SDRAM - 32-bit ECC
        //     0xF0000000       0x0.F000.0000        32     Flash (PCE0#)
        //     0xF2000000       0x0.F200.0000         1     PCE1#        
	//     0xF2100000       0x0.F210.0000        15		Unused
        //     0xF3000000       0x0.F300.0000         1     Compact Flash (Maple Creek Only)
        //     0xF3100000       ----------          204     Unused
        //     0xFFD00000       0x0.FFD0.0000         1     MMR
	//     0xFFE00000       0x0.FFE0.0000         1     SRAM  
	//     0xFFF00000       ----------            1     Unused
 	// 0x0.0000.0000 - 0x0.8000.0000 (Elastic - up to 2GB)
        // 64-Bit ECC SDRAM
	// Size == SDRAM_MAX_MB - CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE * 2
	// X=0, C=1, B=1, ECC
        //    -- note that this sets the caching policy for SDRAM to Read Allocate
        //       This greatly improves the speed of the ECC scrub.  However for general
        //       usage, read-write allocate performs better so we will change the caching
        //       policy after having copied the page tables to SDRAM.
#if defined (CYG_HAL_STARTUP_RAMDUAL)
        .set	__base,(CYGHWR_HAL_SDRAM_SPLIT_BASE >> 20)
        .rept	SDRAM_MAX_MB
		FL_SECTION_ENTRY __base,1,3,1,0,1,1
		.set	__base,__base+1
		.endr
#else
        .set	__base,((SDRAM_PHYS_BASE >> 20) + CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2)
        .rept	SDRAM_MAX_MB - (CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2)
		FL_SECTION_ENTRY __base,1,3,1,0,1,1
		.set	__base,__base+1
		.endr

	// 0x0.xxxx.xxxx - 0x0.8000.0000 - Set based on ECC32 Region
    // Size == CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2
        .rept	2048 - (SDRAM_MAX_MB - CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2)
	FL_UNMAPPED_ENTRY
	.endr
#endif

	// 0x0.8000.0000 - 0x0.8800.0000 (128MB)
	// 128MB ATU-X Outbound Memory Translation Windows - Super Section Descriptors
	// X=0, C=0, B=0. Non-ECC
	.set	__base,0x80
	.set    __upper_base,0x1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	// 0x0.8800.0000 - 0x0.9000.0000 (128MB)
	// 128MB ATUe Outbound Memory Translation Windows - Super Section Descriptors
	// X=0, C=0, B=0. Non-ECC
	.set	__base,0x00
	.set    __upper_base,0x2
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	.set	__base,__base+1
	.rept	16
	FL_SUPERSECTION_ENTRY __base,__upper_base,0,3,0,0,0,0
	.endr

	// 0x0.9000.0000 - 0x0.9010.0000 (1MB)
	// 1MB ATU-X Outbound I/O Translation Window
	// X=0, C=0, B=0. Non-ECC
	.set	__base,0x900
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 0x0.9010.0000 - 0x0.9020.0000 (1MB)
	// 1MB ATUe Outbound I/O Translation Window
	// X=0, C=0, B=0. Non-ECC
	.set	__base,0x901
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 0x0.9020.0000 - 0x0.A000.0000 (254MB)
	// 254MB Unused
	.rept 	254
	FL_UNMAPPED_ENTRY
	.endr

	// 0x0.A000.0000 - 0x0.C000.0000 (512MB)
    // 64-Bit ECC SDRAM - Uncached/Unbuffered Alias
	// Size == 512MB
	// X=0, C=0, B=0, ECC
	.set	__base,((SDRAM_PHYS_BASE >> 20) + CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2)
    .rept	512
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// 0x0.C000.0000 - 0x0.C010.0000         
	// 1MB Cache flush and Cache Locking Region
	// X=1, C=1, B=1, Non-ECC
	.set	__base,0xC00
	.rept	1
	FL_SECTION_ENTRY __base,1,3,0,0,1,1
	.set	__base,__base+1
	.endr

	// 0x0.C010.0000 - 0x0.D000.0000 - Unused
	.rept	255
	FL_UNMAPPED_ENTRY
	.endr	

	// 0x0.D000.0000 - 0x0.F000.0000 (Elastic 0 to 512MB)
    // 32-Bit ECC SDRAM - Uncached/Unbuffered Alias
	// Size == CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE
	// X=0, C=0, B=0, ECC
    .set	__base,(SDRAM_PHYS_BASE >> 20)
	.rept	CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE
	FL_SECTION_ENTRY __base,0,3,1,0,0,0
	.set	__base,__base+1
	.endr

	// 0x0.D000.0000 - 0x0.F000.0000 (Elastic - 0 to 512MB)
	// Hole between 32-bit ECC SDRAM and Cache Flush Region
	.rept	512 - CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE
	FL_UNMAPPED_ENTRY
	.endr

	// 0x0.F000.0000 - 0x0.F200.0000 (32MB) 
	// 32MB FLASH(PCE0#)
	// X=1, C=1, B=1
	.set	__base,(PCE0_PHYS_BASE >> 20)
	.rept	32
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr	

	// 0x0.F200.0000 - 0x0.F210.0000 (1MB)  
	// 1MB PCE1#
	// X=0, C=0, B=0
    .set	__base,(PCE1_PHYS_BASE >> 20)
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 0x0.F210.0000 - 0x0.F220.0000 (1MB)  
	// 1MB PCE1# - Cacheable
	// X=0, C=1, B=1
    .set	__base,(PCE1_PHYS_BASE >> 20)
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,1,1
	.set	__base,__base+1
	.endr

	// 0x0.F210.0000 - 0x0.F300.0000 (14MB) 
	.rept	14
	FL_UNMAPPED_ENTRY
	.endr

	// 0x0.F300.0000 - 0x0.F310.0000 (1MB)  
	// 1MB PCE1# - Compact Flash Address
	// X=0, C=0, B=0
    .set	__base,0xF30
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	// 0x0.F310.0000 - 0x0.FFD0.0000 (204MB)
	// 204MB Unused
	.rept	204
	FL_UNMAPPED_ENTRY
	.endr
	
	// 0x0.FFD0.0000 - 0x0.FFE0.0000 (1MB)
	// 1MB PMMR
	// X=0, C=0, B=0
	.set	__base,0xFFD
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

#ifdef CYGSEM_HAL_SCRUB_SRAM
	// 0x0.FFE0.0000 - 0x0.FFF0.0000 (1MB)
	// 1MB Internal SRAM
	// X=1, C=1, B=1
	.set	__base,0xFFE
	.rept	1
	FL_SECTION_ENTRY __base,1,3,0,0,1,1
	.set	__base,__base+1
	.endr

#if 0
  /* Eng MMRs - should not be accessible by RedBoot, but
   for debug we can put a PT descriptor in place */
	// 1MB PMMR for FSEng
	// X=0, C=0, B=0
	.set	__base,0xFFF
	.rept	1
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr
#else
	// 0x0.FFE0.0000 - 0x1.0000.0000 (1MB)
	// 1MB Unused
	.rept	1
	FL_UNMAPPED_ENTRY
	.endr
#endif
#else
	// 0x0.FFE0.0000 - 0x1.0000.0000 (2MB)
	// 2MB Unused
	.rept	2
	FL_UNMAPPED_ENTRY
	.endr
#endif  // Scrub SRAM

#endif /* CYG_HAL_STARTUP_ROM */

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
