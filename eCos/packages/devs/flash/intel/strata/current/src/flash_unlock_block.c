//==========================================================================
//
//      flash_unlock_block.c
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

int flash_unlock_block(volatile flash_t *block, int block_size, int blocks)
        __attribute__ ((section (".2ram.flash_unlock_block")));


//
// The difficulty with this operation is that the hardware does not support
// unlocking single blocks.  However, the logical layer would like this to
// be the case, so this routine emulates it.  The hardware can clear all of
// the locks in the device at once.  This routine will use that approach and
// then reset the regions which are known to be locked.
//

#define MAX_FLASH_BLOCKS 256

int
flash_unlock_block(volatile flash_t *block, int block_size, int blocks)
{
    volatile flash_t *ROM;
    flash_t stat;
    int timeout = 5000000;
    int cache_on;
	unsigned int temp, blocks_to_unlock = 1;
#ifndef CYGOPT_FLASH_IS_SYNCHRONOUS
    int i;
    volatile flash_t *bp, *bpv;
    unsigned char is_locked[MAX_FLASH_BLOCKS];
#endif

    HAL_DCACHE_IS_ENABLED(cache_on);
    if (cache_on) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }
    CYGHWR_FLASH_WRT_ON();
    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( CYGNUM_FLASH_BASE_MASK & (unsigned int)block );
    block = FLASH_P2V(block);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

	// If each block has a block lock bit - clear it here
	if ((_si(strata_query_data_g.ext_q_data.optional_feature) & INDIVIDUAL_BLOCK_LOCKS) != 0) { 
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
						blocks_to_unlock++;
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
						blocks_to_unlock++;
					}
				}
			}
		}
		do { 
		    // Clear lock bit
		    block[0] = FLASH_Clear_Locks;
		    block[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
		    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
		        if (--timeout == 0) break;
	    	}
			blocks_to_unlock--;
			block = FLASH_P2V((unsigned int)block + strata_query_data_g.small_block);
		} while (blocks_to_unlock > 0);
	}
	// Else all the Block Lock bits can be cleared at once with a single command
	// and we will re-lock the blocks that were not selected for unlocking
	else  {
	    // Get current block lock state.  This needs to access each block on
	    // the device so currently locked blocks can be re-locked.
	    bp = ROM;
	    for (i = 0;  i < blocks;  i++) {
	        bpv = FLASH_P2V( bp );
	        *bpv = FLASH_Read_Query;
	        if (bpv == block) {
	            is_locked[i] = 0;
	        } else {
#if 8 == CYGNUM_FLASH_WIDTH
            is_locked[i] = (bpv[4] & LOCK_MASK);
#else
            is_locked[i] = (bpv[2] & LOCK_MASK);
# endif
        }
        bp += block_size / sizeof(*bp);
    }

    // Clears all lock bits
    ROM[0] = FLASH_Clear_Locks;
    ROM[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
    timeout = 5000000;
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

   // Restore the lock state
   bp = ROM;
    for (i = 0;  i < blocks;  i++) {
        bpv = FLASH_P2V( bp );
        if (is_locked[i]) {
            *bpv = FLASH_Set_Lock;
            *bpv = FLASH_Set_Lock_Confirm;  // Confirmation
            timeout = 5000000;
            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
                if (--timeout == 0) break;
            }
        }
        bp += block_size / sizeof(*bp);
    }
  //#endif CYGOPT_FLASH_IS_SYNCHRONOUS
	        
	    // Clears all lock bits
	    ROM[0] = FLASH_Clear_Locks;
	    ROM[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
	    timeout = 5000000;
	    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
	        if (--timeout == 0) break;
	    }

	    // Restore the lock state
	    bp = ROM;
	    for (i = 0;  i < blocks;  i++) {
	        bpv = FLASH_P2V( bp );
	        if (is_locked[i]) {
	            *bpv = FLASH_Set_Lock;
	            *bpv = FLASH_Set_Lock_Confirm;  // Confirmation
	            timeout = 5000000;
	            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
	                if (--timeout == 0) break;
	            }
	        }
	        bp += block_size / sizeof(*bp);
	    }
	}


    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;
    CYGHWR_FLASH_WRT_RESTORE();
    if (cache_on) {
        HAL_DCACHE_ENABLE();
    }

    return stat;
}
