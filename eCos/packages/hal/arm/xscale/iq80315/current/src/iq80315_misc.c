//==========================================================================
//
//      iq80315_misc.c
//
//      HAL misc board support code for Intel XScale IQ80315
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter, dmoseley, dkranak, cebruns
// Date:         2001-12-03
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <redboot.h>
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ioc80314.h>          // Hardware definitions
#include <cyg/hal/iq80315.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#ifdef CYGPKG_IO_PCI
#include <cyg/hal/hal_ioc80314_pci.h>
cyg_uint32 cyg_pci_window_real_base = 0;
#endif

cyg_uint32 hal_current_cpu_id = -1;
char stepping[64];

void check_ram_size(void);
void check_pci_busses (void);

RedBoot_init(check_ram_size, RedBoot_INIT_LAST);
RedBoot_init(check_pci_busses, RedBoot_INIT_LAST);
//
// Platform specific initialization
//
void
plf_hardware_init(void)
{

// RAM startup only - rewrite relevent bits depending on config
#ifndef CYG_HAL_STARTUP_ROM
    HAL_DCACHE_SYNC();            // Force data out
    HAL_DCACHE_INVALIDATE_ALL();  // Flush TLBs: make new mmu state effective
#endif // ! CYG_HAL_STARTUP_ROM - RAM start only
}

//
// Memory layout - runtime variations of all kinds.
//
externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( hal_dram_size <=  (4096<<20),
                "More than 4GB reported - that can't be right" );
    CYG_ASSERT( 0 == (hal_dram_size & 0xfffff),
                "hal_dram_size not whole Mb" );
    // is it the "normal" end of the DRAM region? If so, it should be
    // replaced by the real size
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE) ) {
        regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
    // Also, we must check for the top of the heap having moved.  This is
    // because the heap does not abut the top of memory.
#ifdef CYGMEM_SECTION_heap1
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_SECTION_heap1 + CYGMEM_SECTION_heap1_SIZE )) {
        // hal_dram_size excludes the PCI window on this platform.
        if ( regionend > (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size )
            regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
#endif

    return regionend;
}


// Check_ram_size - Called during RedBoot's init procedure and it is only
//                  used to verify if >512Meg of SDRAM is installed.  If not
//                  BAR4 is cleared because we don't need the extra memory
//                  Window through the switch fabric.  RedBoot is unaware of
//                  any memory above 512Meg, so an O/S or app has to window
//                  this memory in and out of the memory map as needed.
void check_ram_size (void) {
	cyg_uint32 dimm0_size, dimm1_size, dimm2_size, total_dram_size;
	// Read SDRAM registers.  If total memory installed is <=512Meg, we can disable BAR4 - no window is needed
	// Mask Registers are 36-bit, so take the value, invert it and add one to get the sizeof DIMM right-shifted by 16
	dimm0_size = *(cyg_uint32*)(SD_BANK0_MASK);
	if (dimm0_size != 0) {
		dimm0_size |= 0xFFF00000;   // Fill reserved fields with F before inverting
		dimm0_size = ~(dimm0_size);
		dimm0_size += 1;
	}
	dimm1_size = *(cyg_uint32*)(SD_BANK1_MASK);
	if (dimm1_size != 0) {
		dimm1_size |= 0xFFF00000;   // Fill reserved fields with F before inverting
		dimm1_size = ~(dimm1_size);
		dimm1_size += 1;
	}
	dimm2_size = *(cyg_uint32*)(SD_BANK2_MASK);
	if (dimm2_size != 0) {
		dimm2_size |= 0xFFF00000;   // Fill reserved fields with F before inverting
		dimm2_size = ~(dimm2_size);
		dimm2_size += 1;
	}
		  
	total_dram_size = dimm0_size + dimm1_size + dimm2_size;

	/* Check if total_dram_size is <= 512Meg (remember Mask specifies a 36-bit address, so only need to shift by 4) */
	if (total_dram_size <= (512 << 4)) {   
		*(unsigned int*)(HAL_IOC80314_CIU_BASE + TS_CIU_SF_SIZES) &= ~(0xff);  // Clear Bar4 Sizes
                while (*(unsigned int*)(HAL_IOC80314_CIU_BASE + TS_CIU_SF_SIZES) & 0xff) ;
		*(unsigned int*)(HAL_IOC80314_CIU_BASE + TS_CIU_SF_BAR4) = 0x0;  // No Bar4 needed if memory < 512
	}		
}

// check_pci_busses - Called during RedBoot's init procedure and it is 
//                  used to verify if both busses are running in PCI-X Mode.
//                  If so, nothing needs to be done.  If one of the busses
//                  is in PCI mode, the 1k buffers need to be disabled on
//                  both busses, so the buffer size will be set to 256bytes.
void check_pci_busses (void) {
	cyg_uint32 ltimer1, ltimer2;
	ltimer1 = ((*(volatile cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_MISC0)) & 0xFF00) >> 8;
	ltimer2 = ((*(volatile cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PCI_MISC0)) & 0xFF00) >> 8;
	// Check if either bus is in PCI Mode - if so, have to disable 1k buffers and make both busses 256 bytes buffers
	if ((ltimer1 == 0) || (ltimer2 == 0)) {
        *(volatile cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_MISC_CSR) &= ~(0x4);
        *(volatile cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PCI_MISC_CSR) &= ~(0x4);
	}
	// Whether we're PCI-X or PCI, set LTIMER to 0x80
	*(volatile cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_MISC0) = PCI_PCI1_LTIMER;
	*(volatile cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PCI_MISC0) = PCI_PCI2_LTIMER;

}


// __get_stepping_info - used to determine which version of IOC80314 is on the board.
// This is called when RedBoot is printing out the boot header.
char* __get_stepping_info(void) {
	if (*(cyg_uint8*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_CLASS) == 0x0)
		strcpy(stepping, "IOC80314 : A0");
	else if (*(cyg_uint8*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_CLASS) == 0x1)
		strcpy(stepping, "IOC80314 : B0");
	else if (*(cyg_uint8*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_CLASS) == 0x2)
		strcpy(stepping, "IOC80314 : B1");
	else
		strcpy(stepping, "IOC80314 : ERR");

	return stepping;
}

// ------------------------------------------------------------------------
// EOF iq80315_misc.c
