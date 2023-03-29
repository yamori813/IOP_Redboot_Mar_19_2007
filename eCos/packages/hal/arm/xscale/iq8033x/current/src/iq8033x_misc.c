//==========================================================================
//
//      iq8033x_misc.c
//
//      HAL misc board support code for Intel XScale iq80331/iq80332 CRB
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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-07-10
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
// Copyright:    (C) 2003-2004 Intel Corporation.
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
#include <cyg/hal/hal_iop33x.h> // Hardware definitions
#include <cyg/hal/iq8033x.h>       // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#ifdef CYGPKG_IO_PCI
cyg_uint32 cyg_pci_window_real_base = 0;
#endif

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{
    INTCTL0_WRITE(0);  // mask all irqs
    INTSTR0_WRITE(0);  // everything goes to IRQ
    INTCTL1_WRITE(0);  // mask all irqs
    INTSTR1_WRITE(0);  // everything goes to IRQ
    *PIRSR = 0x0f;     // route XINT0-XINT3 to the XScale core

    HAL_CACHE_SET_POLICY(SDRAM_BASE,
                         (SDRAM_BASE + SDRAM_MAX - 
                          ((CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2) << 20)),
                         CACHE_WB_RW_ALLOC);
    
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

char *hal_platform_board(void)
{
    char prodcode = *IQ8033X_PRODUCT_CODE & IQ8033X_PROD_CODE_MASK;

    if (prodcode == IQ8033X_PROD_CODE_IQ80332)
    {
	if (*ATU_ATUDID == 0x374)
        	return "IQ80333";
	else
        	return "IQ80332";
    }
    else
    {
	if (prodcode == IQ8033X_PROD_CODE_IQ80331)
        	return "IQ80331";
    	else
        	return "Unknown IQ80331/IQ80332 CRB";
    }
}

unsigned long hal_platform_machine_type(void)
{
    char prodcode = *IQ8033X_PRODUCT_CODE & IQ8033X_PROD_CODE_MASK;

    if (prodcode == IQ8033X_PROD_CODE_IQ80332)
        return 629;
    else if (prodcode == IQ8033X_PROD_CODE_IQ80331)
        return 385;
    else
        // Default to IQ80331
        return 385;
}

// ------------------------------------------------------------------------
// EOF iq8033x_misc.c
