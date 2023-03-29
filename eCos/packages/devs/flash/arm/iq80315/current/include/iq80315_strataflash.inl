#ifndef CYGONCE_DEVS_FLASH_IQ80315_STRATAFLASH_INL
#define CYGONCE_DEVS_FLASH_IQ80315_STRATAFLASH_INL
//==========================================================================
//
//      iq80315_strataflash.inl
//
//      Flash programming - device constants, etc.
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
// Contributors: gthomas, dkranak, cebruns
// Date:         2001-12-07
// Purpose:
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

// The iq80315 system has one 16-bit device.
#include <pkgconf/system.h>             // System-wide configuration info
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_ioc80314.h>

#define CYGNUM_FLASH_DEVICES      (1)
#define CYGNUM_FLASH_BASE         HAL_IOC80314_FLASH_BASE

#if defined(CYGPKG_HAL_ARM_XSCALE_IQ80315_BOARD_FLAVOR_IQ80315)
#define CYGNUM_FLASH_BASE_MASK    (0xFF800000u) // 8Mb devices
#elif defined(CYGPKG_HAL_ARM_XSCALE_IQ80315_BOARD_FLAVOR_XCARD)
#define CYGNUM_FLASH_BASE_MASK    (0xFF000000u) // 16Mb devices
#else
error (YOU MUST DEFINE A BOARD FLAVOR IN THE CDL FILE)
#endif

#define CYGNUM_FLASH_WIDTH        (8)
#define CYGNUM_FLASH_BLANK        (1)

#define CYGHWR_FLASH_WRT_ON()                                \
        CYG_MACRO_START                                      \
        currentB0Ctrl0 = *ptrB0Ctrl0;                        \
        *ptrB0Ctrl0 = currentB0Ctrl0 |  TS_HLP_Bx_CTRL0_FWE; \
		while (!(*ptrB0Ctrl0 & TS_HLP_Bx_CTRL0_FWE))         \
		      ;                                              \
        CYG_MACRO_END

#define CYGHWR_FLASH_WRT_RESTORE()                           \
        CYG_MACRO_START                                      \
        *ptrB0Ctrl0 = currentB0Ctrl0;                        \
        CYG_MACRO_END

static volatile cyg_uint32* ptrB0Ctrl0 = 
        (cyg_uint32*)(HAL_IOC80314_CSR_BASE + HAL_80314_HLP_BASE + TS_HLP_B0_CTRL0);
static cyg_uint32 currentB0Ctrl0;

#endif  // CYGONCE_DEVS_FLASH_IQ80315_STRATAFLASH_INL
// ------------------------------------------------------------------------
// EOF iq80315_strataflash.inl
