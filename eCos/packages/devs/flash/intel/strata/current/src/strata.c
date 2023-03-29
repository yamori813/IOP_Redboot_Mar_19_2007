//==========================================================================
//
//      strata.c
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
// Contributors: gthomas, dmoseley, cebruns
// Date:         2001-02-14
// Purpose:
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

#include "strata.h"


extern void diag_dump_buf(void *buf, CYG_ADDRWORD len);
extern int strncmp(const char *s1, const char *s2, size_t len);
extern void *memcpy( void *, const void *, size_t );

struct FLASH_query_data strata_query_data_g;
		     
int
flash_hwr_init(void)
{
	struct FLASH_query *qp;
	int stat, buffer_size, num_regions = 0, region_size = 0;
	unsigned int size;
	unsigned int limit_val = 0xFFFFFFFF;
	
	flash_dev_query((unsigned char *)&(strata_query_data_g.q_data), (unsigned char *)&(strata_query_data_g.ext_q_data));

 
	strata_query_data_g.num_regions = strata_query_data_g.q_data.num_regions;
	qp = &(strata_query_data_g.q_data);
	if ( (qp->manuf_code == FLASH_Intel_code)
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
         // device types go as follows: 0x90 for 16-bits, 0xD0 for 8-bits,
         // plus 0 or 1 for -T (Top Boot) or -B (Bottom Boot)
         //     [FIXME: whatever that means :FIXME]
         // [I think it means the boot blocks are top/bottom of addr space]
         // plus the following size codes:
         //    0: 16Mbit    2:  8Mbit    4:  4Mbit
         //    6: 32Mbit    8: 64Mbit
#if 16 == CYGNUM_FLASH_WIDTH
         && (0x90 == (0xF0 & qp->device_code)) // 16-bit devices
#elif 8 == CYGNUM_FLASH_WIDTH
         && (0xD0 == (0xF0 & qp->device_code)) // 8-bit devices
#else
         && 0
#error Only understand 16 and 8-bit bootblock flash types
#endif
        ) {
        int lookup[] = { 16, 8, 4, 32, 64 };
#define BLOCKSIZE (0x10000)
        region_size = BLOCKSIZE;
        num_regions = qp->device_code & 0x0F;
        num_regions >>= 1;
        if ( num_regions > 4 )
            goto flash_type_unknown;
        num_regions = lookup[num_regions];
        num_regions *= 1024 * 1024;     // to bits
        num_regions /= 8;               // to bytes
        num_regions /= BLOCKSIZE;       // to blocks
        buffer_size = 0;
#else // CYGOPT_FLASH_IS_BOOTBLOCK
         && (strncmp(qp->id, "QRY", 3) == 0)) {
		// If we have parameter blocks, use the highest block_size as what we pass up
		// to the RedBoot Flash driver.  We'll deal with dis-similar block sizes in 
		// the Flash functions themselves.
		if ((qp->num_regions) >= 1) {
			// If Top Blocks are smaller or only Single Region
			//    use lower block sizes for Flash Description
			if ((_si(qp->region1_size) > _si(qp->region2_size)) || (qp->num_regions == 1)) {
				strata_query_data_g.top_parameters = 1;
		        num_regions = _si(qp->region1_blocks)+1;
		        strata_query_data_g.large_block = region_size = _si(qp->region1_size)*256;
				if (qp->num_regions > 1) {
					// Add the Parameter blocks in Main Block sizes
					strata_query_data_g.small_block = (_si(qp->region2_size)*256);
			 		num_regions += ((_si(qp->region2_blocks)+1)* \
			 		                 (strata_query_data_g.small_block))/region_size;
				}
			}
			else  {
				// Bottom blocks are smaller - use Upper block size for Flash Description
				strata_query_data_g.top_parameters = 0;
		        num_regions = _si(qp->region2_blocks)+1;
		        strata_query_data_g.large_block = region_size = _si(qp->region2_size)*256;
				// Add the Parameter blocks in Main Block sizes
				strata_query_data_g.small_block = (_si(qp->region1_size)*256);
		 		num_regions += ((_si(qp->region1_blocks)+1)* \
		 		               (strata_query_data_g.small_block))/region_size;
			}
		}
        if (_si(qp->buffer_size)) {
            buffer_size = CYGNUM_FLASH_DEVICES << _si(qp->buffer_size);
        } else {
            buffer_size = 0;
        }
#endif // Not CYGOPT_FLASH_IS_BOOTBLOCK
		// Record largest block for later checks during erase/lock/unlock/program
        flash_info.block_size = region_size*CYGNUM_FLASH_DEVICES;
        flash_info.buffer_size = buffer_size;
        flash_info.blocks = num_regions;
        flash_info.start = (void *)CYGNUM_FLASH_BASE;
        flash_info.end = (void *)(CYGNUM_FLASH_BASE +
                        (num_regions*region_size*CYGNUM_FLASH_DEVICES));
#ifdef CYGNUM_FLASH_BASE_MASK
#ifdef CYGPKG_HAL_ARM_XSCALE_IOP34X 
#define IOP_MAX_PBI 0x2000000
	size = (unsigned int)flash_info.end - (unsigned int)flash_info.start;
	if (size > IOP_MAX_PBI)
	{	
            	unsigned int x;
		x = (unsigned int)IOP_MAX_PBI + (unsigned int)flash_info.start;
                (*flash_info.pf)("\nFLASH: Oversized device!  End addr %p changed to %p\n",flash_info.end, (void *)x);
                flash_info.end = (void *)x;
                // Also adjust the block count else unlock crashes!
                x = ((cyg_uint8 *)flash_info.end - (cyg_uint8 *)flash_info.start)
                    / flash_info.block_size;
                flash_info.blocks = x;
		size = IOP_MAX_PBI;
	}
	limit_val = 0xFFFFFFFF;
	while (size /= 2)
		limit_val = limit_val << 1;
	HAL_WRITE_UINT32(PBIU_PBLR0,limit_val);
        //(*flash_info.pf)("\nFLASH: end %p  limitval=%x\n",flash_info.end,limit_val);
#else
        // Then this gives us a maximum size for the (visible) device.
        // This is to cope with oversize devices fitted, with some high
        // address lines ignored.
        if ( ((unsigned int)flash_info.start & CYGNUM_FLASH_BASE_MASK) !=
             (((unsigned int)flash_info.end - 1) & CYGNUM_FLASH_BASE_MASK ) ) {
            // then the size of the device appears to span >1 device-worth!
            unsigned int x;
            x = (~(CYGNUM_FLASH_BASE_MASK)) + 1; // expected device size
            x += (unsigned int)flash_info.start;
            if ( x < (unsigned int)flash_info.end ) { // 2nd sanity check
                (*flash_info.pf)("\nFLASH: Oversized device!  End addr %p changed to %p\n",
                       flash_info.end, (void *)x );
                flash_info.end = (void *)x;
                // Also adjust the block count else unlock crashes!
                x = ((cyg_uint8 *)flash_info.end - (cyg_uint8 *)flash_info.start)
                    / flash_info.block_size;
                flash_info.blocks = x;
            }
        }
#endif
#endif // CYGNUM_FLASH_BASE_MASK
        return FLASH_ERR_OK;
    }
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
 flash_type_unknown:
#endif
    (*flash_info.pf)("Can't identify FLASH, sorry, man %x, dev %x, id [%4s] stat %x\n",
           qp->manuf_code, qp->device_code, qp->id, stat );
    diag_dump_buf(qp, sizeof(strata_query_data_g.q_data));
    return FLASH_ERR_HWR;
}

// Map a hardware status to a package error
int
flash_hwr_map_error(int err)
{
    if (err & FLASH_ErrorMask) {
        (*flash_info.pf)("Err = %x\n", err);
		if (err & FLASH_BlockLocked)
	        (*flash_info.pf)("\nBlock is Locked! ");
        if (err & FLASH_ErrorProgram) { 
	        (*flash_info.pf)("Cannot Program\n");
            return FLASH_ERR_PROGRAM;
        }
        else if (err & FLASH_ErrorErase) {
	        (*flash_info.pf)("Cannot Erase\n");
            return FLASH_ERR_ERASE;
        }
        else  {
	        (*flash_info.pf)("Unknown Error\n");
            return FLASH_ERR_HWR;  // FIXME
        }
    } else {
        return FLASH_ERR_OK;
    }
}

// See if a range of FLASH addresses overlaps currently running code
bool
flash_code_overlaps(void *start, void *end)
{
    extern char _stext[], _etext[];

    return ((((unsigned long)&_stext >= (unsigned long)start) &&
             ((unsigned long)&_stext < (unsigned long)end)) ||
            (((unsigned long)&_etext >= (unsigned long)start) &&
             ((unsigned long)&_etext < (unsigned long)end)));
}

// EOF strata.c

