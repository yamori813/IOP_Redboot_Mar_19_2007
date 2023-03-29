//==========================================================================
//
//      flash_query.c
//
//      Flash programming - query device
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
// Author(s):    gthomas, hmt
// Contributors: gthomas, dmoseley, cebruns
// Date:         2001-02-14
// Purpose:
// Description:
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include "strata.h"

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

//
// CAUTION!  This code must be copied to RAM before execution.  Therefore,
// it must not contain any code which might be position dependent!
//
int
flash_query(unsigned char *data, unsigned char *ext_data)  __attribute__ ((section (".2ram.flash_query")));



#define CNT 20*1000*10  // Approx 20ms

// Modified to grab Standard and Extended Query table from
// StrataFlash 
int
flash_query(unsigned char *data, unsigned char *ext_data)
{
    volatile flash_t *ROM;
    int i, cnt, j = 0;
    int cache_on;
    struct FLASH_query *qp = (struct FLASH_query *)data;


    HAL_DCACHE_IS_ENABLED(cache_on);
    if (cache_on) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }
    CYGHWR_FLASH_WRT_ON();
    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( CYGNUM_FLASH_BASE );
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
    // BootBlock flash does not support full Read_Query - we have do a
    // table oriented thing above, after getting just two bytes of results:
    ROM[0] = FLASH_Read_ID;
    i = 2;
#else
    // StrataFlash supports the full Read_Query op:
    ROM[0] = FLASH_Read_Query;
    i = sizeof(struct FLASH_query);
	j = sizeof(struct FLASH_ext_query);
#endif // Not CYGOPT_FLASH_IS_BOOTBLOCK

    for (cnt = CNT;  cnt > 0;  cnt--) ;
    for ( /* i */;  i > 0;  i-- ) {
        // It is very deliberate that data is chars NOT flash_t:
        // The info comes out in bytes regardless of device.
        *data++ = (unsigned char) (*ROM++);
#ifndef CYGOPT_FLASH_IS_BOOTBLOCK
# if  8 == CYGNUM_FLASH_WIDTH
	// strata flash with 'byte-enable' contains the configuration data
	// at even addresses
	++ROM;
# endif
#endif
    }
	// According to this BOOTBLOCK define, there is no ext_query, so
	// we don't use it if BOOTLBOCK is defined.
	// For other Flashes, we need to get offset to Extended Query Table for Strata
#ifndef CYGOPT_FLASH_IS_BOOTBLOCK
	ROM = FLASH_P2V (CYGNUM_FLASH_BASE) + _si(qp->ex_query_table);
    for ( /* j */;  j > 0;  j-- ) {
        // It is very deliberate that data is chars NOT flash_t:
        // The info comes out in bytes regardless of device.
        *ext_data++ = (unsigned char)(*ROM++);
#if  8 == CYGNUM_FLASH_WIDTH
	// strata flash with 'byte-enable' contains the configuration data
	// at even addresses
	++ROM;
#endif
    }
#endif

    ROM[0] = FLASH_Reset;

    if (cache_on) {
        HAL_DCACHE_ENABLE();
    }
    CYGHWR_FLASH_WRT_RESTORE();
    return 0;
}
