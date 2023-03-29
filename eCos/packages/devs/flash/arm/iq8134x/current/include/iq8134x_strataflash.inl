#ifndef CYGONCE_DEVS_FLASH_IQ8134X_STRATAFLASH_INL
#define CYGONCE_DEVS_FLASH_IQ8134X_STRATAFLASH_INL
//==========================================================================
//
//      iq8134x_strataflash.inl
//
//      Flash programming - device constants, etc.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

// The IQ8134 CRBS have one device that can be accessed
// in either 8-bit or 16-bit.

#define CYGNUM_FLASH_DEVICES 	(1)
#define CYGNUM_FLASH_BASE 	IQ8134X_FLASH_ADDR
#define CYGNUM_FLASH_BASE_MASK  (((*IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK) == IQ8134X_PROD_CODE_IQ8134xMC) ? \
                                          (0xFE000000u) : 0xFF800000u)
#define CYGNUM_FLASH_WIDTH 	CYGNUM_HAL_ARM_IQ8134X_FLASH_WIDTH
#define CYGNUM_FLASH_BLANK      (1)

#include <pkgconf/system.h>             // System-wide configuration info
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_iop34x.h>  // IO Processor defines

#define CYGHWR_FLASH_WRT_ON()                                           \
        CYG_MACRO_START                                                 \
        HAL_CACHE_SET_POLICY(IQ8134X_FLASH_ADDR,                        \
                             (IQ8134X_FLASH_ADDR + FLASH_MAX),          \
                             STALL);                                    \
		GET_FAC_SEMAPHORES();                                           \
        CYG_MACRO_END

#define CYGHWR_FLASH_WRT_RESTORE()                                      \
        CYG_MACRO_START                                                 \
        HAL_CACHE_SET_POLICY(IQ8134X_FLASH_ADDR,                        \
                             (IQ8134X_FLASH_ADDR + FLASH_MAX),          \
                             CACHE_WB_RW_ALLOC);                        \
		RELEASE_FAC_SEMAPHORES();                                       \
        CYG_MACRO_END

static volatile cyg_uint32* ptrPBAR0 = (cyg_uint32*)(PBIU_PBAR0);
static          cyg_uint32  currentPBAR0;


#endif  // CYGONCE_DEVS_FLASH_IQ8134X_STRATAFLASH_INL
// ------------------------------------------------------------------------
// EOF iq8134x_strataflash.inl
