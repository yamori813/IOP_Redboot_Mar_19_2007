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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-08-04
// Purpose:      Intel XScale IQ80331/IQ80332 CRB platform specific support routines
// Description: 
//     This file contains low-level initialization code called by hal_platform_setup.h
//     Since we may be using "alternative" memory (ie different stack and heap than
//     we were compiled for), we cannot use any global data or C Library calls.
// Usage:
//     Only called by "hal_platform_setup.h"
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include <cyg/hal/iq8033x.h>             // Platform specific hardware definitions
#include <cyg/hal/hal_cache.h>

extern void mcu_initialization(cyg_uint32 *sdram_size, 
                               cyg_bool *sdram_installed,
                               cyg_bool *ecc_cap);
extern void minimal_ATU_setup(cyg_bool sdram_installed,
                              cyg_uint32 sdram_size,
                              cyg_uint32 *sdram_pci_size,
                              cyg_uint32 *config_flags);
extern void mcu_memory_scrub(cyg_uint32 base, cyg_uint32 size);
extern void mcu_enable_ecc(void);

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
    mcu_initialization(sdram_size, &sdram_installed, &ecc_enable);

    HEX_DISPLAY_QUICK(2, 3);
    minimal_ATU_setup(sdram_installed, *sdram_size, sdram_pci_size, config_flags);

    HEX_DISPLAY_QUICK(2, A);
    if (sdram_installed && ecc_enable)
    {
        mcu_memory_scrub(SDRAM_BASE,
                         *sdram_size);
#if defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE) && \
    !defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE_0)
        mcu_memory_scrub(SDRAM_32BIT_ECC_BASE,
                         CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE << 20);
#endif
    }

    if (ecc_enable)
        mcu_enable_ecc();

    HEX_DISPLAY_QUICK(2, B);
    return;
}
