//=============================================================================
//
//      cftest.c
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Mark Salter
// Contributors: Mark Salter, dmoseley
// Date:         2003-05-27
// Purpose:     
// Description: Tests for compact flash
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#include <redboot.h>
#include <cyg/hal/hal_verde.h>    // Hardware definitions
#include <cyg/hal/ep80219.h>      // Platform specifics
#include "test_menu.h"

extern int ep80219_cf_read_sectors(cyg_uint32 start, cyg_uint8 count, cyg_uint8 *buf);
extern int ep80219_cf_write_sectors(cyg_uint32 start, cyg_uint8 count, cyg_uint8 *buf);
extern cyg_uint32 ep80219_cf_blocks(void);

#define BLOCK_SAVE_ADDR  0x100000
#define BLOCK_TEMP_ADDR  0x110000
#define BLOCK_DATA_ADDR  0x120000

void
cf_test (MENU_ARG arg)
{
    char *save_buf = (char *)BLOCK_SAVE_ADDR;
    char *temp_buf = (char *)BLOCK_TEMP_ADDR;
    char *data_buf = (char *)BLOCK_DATA_ADDR;
    char *p;
    cyg_uint32 nblocks = ep80219_cf_blocks();
    cyg_uint32 blk;
    int i;

    if (nblocks == 0) {
	diag_printf("CF card not detected.\n");
	return;
    }
    diag_printf("CF card has %d blocks of 512 bytes (%d bytes total).\n",
		nblocks, nblocks * 512);

    diag_printf("Read/Write tests...");

    for (i = 0; i < 512; i++)
	data_buf[i] = i & 0xff;

    for (blk = 1; blk < nblocks; blk <<= 1) {
	if (!ep80219_cf_read_sectors(blk, 1, save_buf)) {
	    diag_printf("failed (save block %d).\n", blk);
	    return;
	}
	if (!ep80219_cf_write_sectors(blk, 1, data_buf)) {
	    diag_printf("failed (write data block %d).\n", blk);
	    return;
	}
	memset(temp_buf, 0, 512);
	if (!ep80219_cf_read_sectors(blk, 1, temp_buf)) {
	    diag_printf("failed (read block %d).\n", blk);
	    return;
	}
	for (i = 0; i < 512; i++, p++) {
	    if (data_buf[i] != temp_buf[i]) {
		diag_printf("failed (blk %d offset %d, expected %02x, got %02x)\n",
			    blk, i, data_buf[i], temp_buf[i]);
		ep80219_cf_write_sectors(blk, 1, save_buf);
		return;
	    }
	}
	if (!ep80219_cf_write_sectors(blk, 1, save_buf)) {
	    diag_printf("failed (write saved block %d).\n", blk);
	    return;
	}
    }
    diag_printf("passed.\n");
}
