//==========================================================================
//
//      iq8134x_misc.c
//
//      HAL misc board support code for Intel XScale IQ8134X CRB
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

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
#include <cyg/hal/hal_iop34x.h> // Hardware definitions
#include <cyg/hal/iq8134x.h>       // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#ifdef CYGPKG_IO_PCI
cyg_uint32 cyg_pci_window_real_base = 0;
#endif

// Global for platform_extra header info
char platform_extra[128];

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{
    INTCTL0_WRITE(0);  // mask all irqs
    INTCTL1_WRITE(0);  // mask all irqs
    INTCTL2_WRITE(0);  // mask all irqs
    INTCTL3_WRITE(0);  // mask all irqs
    INTSTR0_WRITE(0);  // everything goes to IRQ
    INTSTR1_WRITE(0);  // everything goes to IRQ
    INTSTR2_WRITE(0);  // everything goes to IRQ
    INTSTR3_WRITE(0);  // everything goes to IRQ

    HAL_CACHE_SET_POLICY(SDRAM_BASE,
                         (SDRAM_BASE + SDRAM_MAX - 
                          ((CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2) << 20)),
                         CACHE_WB_RW_ALLOC);

// RAM startup only - rewrite relevent bits depending on config
#if defined (CYG_HAL_STARTUP_RAM)
    HAL_DCACHE_SYNC();            // Force data out
    HAL_DCACHE_INVALIDATE_ALL();  // Flush TLBs: make new mmu state effective
	HAL_L2_CACHE_SYNC();		  // Clean and invalidates L2
#endif // ! CYG_HAL_STARTUP_ROM - RAM start only
}

//
// Memory layout - runtime variations of all kinds.
//
externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( hal_dram_size <=  (2048<<20),
                "More than 2GB reported - that can't be right" );
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
         ((cyg_uint8 *)CYGMEM_SECTION_heap1 + CYGMEM_SECTION_heap1_SIZE) ) {
        // hal_dram_size excludes the PCI window on this platform.
        if ( regionend > (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size )
            regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
#endif
    return regionend;
}

/* Check the Product Code in the CPLD to determine what board we're on */
char *hal_platform_board(void)
{
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
    char prodcode = *IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK;

    if (prodcode == IQ8134X_PROD_CODE_IQ8134xSC)
        return "IQ8134x SC";
    else if (prodcode == IQ8134X_PROD_CODE_IQ8134xBC)
        return "IQ8134x BC";
    else if (prodcode == IQ8134X_PROD_CODE_IQ8134xMC)
        return "IQ8134x MC";
    else if (prodcode == IQ8134X_PROD_CODE_IQ8134xMC_MODE2)
	return "IQ8134x MC Mode 2";
    else
        return "Unknown IQ8134 CRB";
#else     
        return "Unknown IQ8134 CRB - PCE1 not used\n";
#endif	
}

/* Return Extra Information about the Platform */
char *hal_platform_extra(void) {
	cyg_uint8 rev_id;

	// Get Ax vs. B0 Silicon
	if(atue_safe())
		rev_id = *ATUE_ATURID;
	else
		rev_id = *ATUX_ATURID;
	
	// Zero-out our platform extra string
	memset(platform_extra, 0, sizeof(platform_extra));

	// Core RBoot is executing on - 7 chars
	if (_RUNNING_ON_CORE1)
		strcpy (platform_extra, "Core1, ");
	else
		strcpy (platform_extra, "Core0, ");

	// Memory Frequency - 10 chars
	if ((*PBIU_ESSTSR0 & MEM_FREQ_400) == MEM_FREQ_400)
		strcat (platform_extra, "DDR2-400\n");
	else								 
		strcat (platform_extra, "DDR2-533\n");

	// Interface Select PCIX# - 12 chars
	if (*PBIU_ESSTSR0 & INTERFACE_SEL_PCIX)
		strcat (platform_extra, "IF_PCIX: 1, ");
	else
		strcat (platform_extra, "IF_PCIX: 0, ");

	// Controller Only - 9 chars
	/* This is not a very interesting setting
	if (*PBIU_ESSTSR0 & CONTROLLER_ONLY)
		strcat (platform_extra, "COnly: 1 ");
	else
		strcat (platform_extra, "COnly: 0 ");
	*/

	if(atue_safe())
	{
		cyg_uint32 link_width;
		strcat (platform_extra, "PCIe");

		link_width = (*ATUE_PCIE_LSTS >> LINK_WIDTH_SHIFT) & LINK_WIDTH_MASK;
		if (link_width == 1)
			strcat (platform_extra, "(x1) ");
		else if (link_width == 2) 
			strcat (platform_extra, "(x2) ");
		else if (link_width == 4) 
			strcat (platform_extra, "(x4) ");
		else if (link_width == 8) 
			strcat (platform_extra, "(x8) ");

		// ATU-e Endpoint - 8 chars
		if (*ATUE_PCSR & PCSR_ATUE_END_POINT_MODE)
		strcat (platform_extra, "EP, ");
		else
			strcat (platform_extra, "RC, ");
		}
	
	if(atux_safe())
	{
		// ATU-X endpoint - 9 chars
		if ((*ATUX_PCSR & PCSR_ATUX_CENTRAL_RESOURCE_MODE) == 0)
			strcat (platform_extra, "PCI-X EP: ");
		else
			strcat (platform_extra, "PCI-X CR: ");
		
		switch (*ATUX_PCSR & PCSR_PCI_MODE) { 
			case PCSR_PCI_MODE:
				if (*ATUX_PCSR & 0x400)
					strcat (platform_extra, "PCI66 ");
				else
					strcat (platform_extra, "PCI33 ");
				break;
			case PCSR_PCIX_66_MODE:
				strcat (platform_extra, "PCIX-66 ");
				break;
			case PCSR_PCIX_100_MODE:
				strcat (platform_extra, "PCIX-100 ");
				break;
			case PCSR_PCIX_133_MODE:
				strcat (platform_extra, "PCIX-133 ");
				break;
			case PCSR_PCIX_266_133_MODE:
				strcat (platform_extra, "PCIX-266 ");
				break;
			default:
				strcat (platform_extra, "??? Speed ");
				break;
		}
	}

	// Stepping Info - 9 chars
	if (rev_id < IQ8134X_REV_B0 )
		strcat (platform_extra, "\nA-step, ");
	else
		if(rev_id < IQ8134X_REV_C0)
			strcat (platform_extra, "\nB-step, ");
		else if (rev_id == IQ8134X_REV_C0)
			strcat (platform_extra, "\nC0-step, ");
		else if (rev_id == IQ8134X_REV_C1)
			strcat (platform_extra, "\nC1-step, ");
		else
			strcat (platform_extra, "\nC-step, ");

	// IB speed - 17 chars
	strcat (platform_extra, "IB Speed: ");
	switch (GET_IB_SPEED_MHZ()) {
		case 400:
			strcat (platform_extra, "400MHz,");
			break;
		case 333:
			strcat (platform_extra, "333MHz,");
			break;
		default:
			strcat (platform_extra, "UNKNOWN"); 
	}
	// Core Speed - 20 chars
	strcat (platform_extra, " Core Speed: ");
	switch (GET_CORE_SPEED_MHZ()) {
		case 600:
			strcat (platform_extra, "600MHz");
			break;
		case 667:
			strcat (platform_extra, "667MHz");
			break;
		case 800:
			strcat (platform_extra, "800MHz");
			break;
		case 933:
			strcat (platform_extra, "933MHz");
			break;
		case 1000:
			strcat (platform_extra, "1000MHz");
			break;
		case 1200:
			strcat (platform_extra, "1200MHz");
			break;
		default:
			strcat (platform_extra, "UNKNOWN");
	}	
	// Total of 91 chars used up to here (COnly not included)

	return((char*)platform_extra);
}

/* Return the proper machine IDs for the ATAG list when booting Linux */
unsigned long hal_platform_machine_type(void)
{
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
    char prodcode = *IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK;

    if (prodcode == IQ8134X_PROD_CODE_IQ8134xSC)
        return IQ8134X_MACH_TYPE_SC;
    else if ((prodcode == IQ8134X_PROD_CODE_IQ8134xMC) || (prodcode == IQ8134X_PROD_CODE_IQ8134xMC_MODE2))
        return IQ8134X_MACH_TYPE_MC;
    else
#endif
		// Default IQ8134x value
        return IQ8134X_MACH_TYPE_SC;
}

// ------------------------------------------------------------------------
// EOF iq8134x_misc.c
