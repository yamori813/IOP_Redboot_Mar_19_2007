/*=============================================================================
//
//      hal_platform_setup.c
//
//      Platform specific support for HAL (C code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// Purpose:      Intel XScale IQ8134 CRB platform specific support routines
// Description: 
//     This file contains low-level initialization code called by hal_platform_setup.h
//     Since we may be using "alternative" memory (ie different stack and heap than
//     we were compiled for), we cannot use any global data or C Library calls.
// Usage:
//     Only called by "hal_platform_setup.h"
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include <cyg/hal/iq8134x.h>             // Platform specific hardware definitions
#include <cyg/hal/hal_cache.h>


#define REDBOOT_OWNS_ATUX
#define REDBOOT_OWNS_ATUE
#define REDBOOT_OWNS_MCU



extern void mcu_initialization(cyg_uint32 *sdram_size, 
                               cyg_bool *sdram_installed,
                               cyg_bool *ecc_cap);
extern void minimal_ATU_setup(cyg_bool sdram_installed,
                              cyg_uint32 sdram_size,
                              cyg_uint32 *sdram_pci_size,
                              cyg_uint32 *config_flags,
                              cyg_bool   configuring_atue);
extern void mcu_memory_scrub(cyg_uint32 base, cyg_uint32 size);
extern void mcu_enable_ecc(void);




// The following functions allow the user to determine if an ATU is safe to access
// First we much check the extrernal strap register to see which ATU is guaranteed
// to be safe.  We then check the device ID from that ATU to see if it is safe to 
// read from the other ATU.
inline int atux_safe()
{
#ifdef REDBOOT_OWNS_ATUX
	volatile unsigned int * ESSTSR0 = (unsigned int *)0xffd82188;
	int id;
	
	if(*ESSTSR0 & (1<<15))
	{ //Strapped for PCIe, check to see if we are SI or DI
		volatile unsigned  int * ATUEID = (unsigned int *)0xffdc8000;
		id = (*ATUEID >> 20);
		id = id & 0xF;
		if((id==7) || (id==8) || (id==0xC))
			return 1;
		else
			return 0;
	}
	else // Strapped for PCI-X, safe if we are SI or DI
		return 1;
#else
	return 0;
#endif
}

inline int atue_safe()
{
#ifdef REDBOOT_OWNS_ATUE
	volatile unsigned int * ESSTSR0 = (unsigned int*)0xffd82188;
	int id;	
	
	if(*ESSTSR0 & (1<<15))
	{	// Strapped for PCI-E, safe if we are SI or DI	
		return 1;
	}
	else
	{//Strapped for PCIX, check to see if we are SI or DI  
		volatile unsigned int * ATUXID = (unsigned int *)0xffdc8000;
		id = (*ATUXID >> 20);
		id = id & 0xF;
		if((id==7) || (id==8) || (id==0xC))
			return 1;
		else
			return 0;
	}
#else
	return 0;
#endif
}


//
// Low-level platform initialization
//   This routine may perform any hardware setup necessary.
//   There will be stack space and only minimal heap so no global
//   variables or C library calls allowed.
//
void hal_platform_setup(cyg_uint32 *sdram_size, cyg_uint32 *sdram_pci_size, cyg_uint32 *config_flags)
{
    cyg_bool sdram_installed, ecc_enable;

    hal_platform_lowlevel_serial_debug_init(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD);
    hal_platform_lowlevel_serial_debug_puts("In hal_platform_setup()\r\n");

    HEX_DISPLAY_QUICK(0, 5);

#ifdef REDBOOT_OWNS_MCU
    mcu_initialization(sdram_size, &sdram_installed, &ecc_enable);
#else   // Hardcode for use with Greg Tucker's common boot code 
	*sdram_size = SZ_64M;
	sdram_installed = true;
#endif
    HEX_DISPLAY_QUICK(2, 3);

	// Initialize Config Flags
	*config_flags = 0;  

	// Need to decide if ATU-E needs configured (i.e. If we're in a host)
	// Check ATU-E Root Complex - if we're an endpoint, we should configure our ATU
	// For ATU-E, PCSR Bit 13 is "1" if it is an Endpoint
	// Setup ATUe Interface
	if (atue_safe())
	{
		if (!(*ATUE_PCSR & PCSR_ATUE_END_POINT_MODE))
			*config_flags |= IOP34X_PCI_MASTER_ATUE;
		minimal_ATU_setup(sdram_installed, *sdram_size, sdram_pci_size, config_flags, true);
	}

	// Need to decide if ATU-X needs configured (i.e. If we're in a host)
	// Check ATU-X End Point - if we're an endpoint, we should configure our ATU
	// For ATU-X PCSR bit 25 is "1" if it's the central resource (!endpoint)
	if(atux_safe())
	{
		if (*ATUX_PCSR & PCSR_ATUX_CENTRAL_RESOURCE_MODE)  
			*config_flags |= IOP34X_PCI_MASTER_ATUX;
   		minimal_ATU_setup(sdram_installed, *sdram_size, sdram_pci_size, config_flags, false);
	}

	
    HEX_DISPLAY_QUICK(2, A);

    if (sdram_installed)
    {
		// If we have SDRAM, we need to scrub it.
		// If sdram_size is zero, then the 32-bit ECC region and its associated hole in memory
		// takes up all our space, so we don't scrub SDRAM_BASE.

#ifdef REDBOOT_OWNS_MCU
		if (*sdram_size != 0)
			mcu_memory_scrub(SDRAM_BASE,*sdram_size);

#if CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE > 0
		mcu_memory_scrub(SDRAM_32BIT_ECC_BASE,CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE << 20);
#endif
		if (ecc_enable)
			mcu_enable_ecc();
#endif

#ifdef CYGSEM_HAL_SCRUB_SRAM
		mcu_memory_scrub(SRAM_BASE, SRAM_SIZE);
#endif
    }

#if CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
    *sdram_size = HAL_DCACHE_LOCKABLE_SIZE;
    sdram_installed = false;
#endif

    HEX_DISPLAY_QUICK(2, B);
    return;
}
