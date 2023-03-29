//==========================================================================
//
//      flash_lock_block.c
//
//      Flash programming
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
// Contributors: gthomas. dmoseley, cebruns
// Date:         2001-02-14
// Purpose:
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

#include "strata.h"

#include <cyg/hal/hal_cache.h>

//
// CAUTION!  This code must be copied to RAM before execution.  Therefore,
// it must not contain any code which might be position dependent!
//
int flash_lock_block(volatile flash_t *block)
        __attribute__ ((section (".2ram.flash_lock_block")));



int flash_lock_block(volatile flash_t *block)
{
    volatile flash_t *ROM;
    flash_t stat;
    int timeout = 5000000;
    int cache_on;
	unsigned int temp, blocks_to_lock = 1;

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V(CYGNUM_FLASH_BASE_MASK & (unsigned int)block);
    block = FLASH_P2V(block);

    HAL_DCACHE_IS_ENABLED(cache_on);
    if (cache_on) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }
    CYGHWR_FLASH_WRT_ON();
    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

	// Need to check if we are trying to lock Parameter blocks
	// which have smaller block sizes than what we reported up
	// to RedBoot
	if (strata_query_data_g.num_regions > 1) {
		if (strata_query_data_g.top_parameters == 1) {
			if (block >= FLASH_P2V(CYGNUM_FLASH_BASE + \
			    ((_si(strata_query_data_g.q_data.region1_blocks)+1) * \
			      strata_query_data_g.large_block))) {
				// We're locking parameter blocks - figure out how many 
				for (temp = strata_query_data_g.small_block; \
				     temp < strata_query_data_g.large_block; 
						temp += strata_query_data_g.small_block ) {
					blocks_to_lock++;
				}
			}
		}
		// Parameters are in the bottom
		else  {
			if (block < FLASH_P2V(CYGNUM_FLASH_BASE + \
			    ((_si(strata_query_data_g.q_data.region1_blocks)+1) * \
			      strata_query_data_g.small_block))) {
				// We're locking parameter blocks - figure out how many 
				for (temp = strata_query_data_g.small_block; \
				     temp < strata_query_data_g.large_block; 
						temp += strata_query_data_g.small_block ) {
					blocks_to_lock++;
				}
			}
		}
	}

    // Set lock bit
	do { 
    block[0] = FLASH_Set_Lock;
    block[0] = FLASH_Set_Lock_Confirm;  // Confirmation
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }
	blocks_to_lock--;
	block = FLASH_P2V((unsigned int)block + strata_query_data_g.small_block);
	} while (blocks_to_lock > 0);

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;
    CYGHWR_FLASH_WRT_RESTORE();
    if (cache_on) {
        HAL_DCACHE_ENABLE();
    }

    return stat;
}
