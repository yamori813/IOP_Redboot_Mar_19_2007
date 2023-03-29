//==========================================================================
//
//      iq8134x_cf.c
//
//      HAL compact flash support code for Intel XScale IQ8134x
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
// Author(s):    msalter
// Contributors: msalter, dmoseley, dkranak, cebruns
// Date:         2003-04-16
// Purpose:      Compact Flash support for RedBoot
// Description:  Implementation of RedBoot 'cf' command.
//
// Copyright:    (C) 2003-2005 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>

#ifdef CYGPKG_REDBOOT
#include <redboot.h>

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

#include <cyg/hal/iq8134x.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <fs/ide.h>

#define HAL_IDE_NUM_CONTROLLERS 1

#define SECTOR_SIZE 512

#define IQ8134X_IDE_BASE (IQ8134X_COMPACT_FLASH_ADDR)

// Initialize the IDE controller(s).
#define HAL_IDE_INIT()

#define HAL_IDE_READ_UINT8( __regno, __val )  \
    (__val) = *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + (__regno))
#define HAL_IDE_READ_UINT16( __regno, __val )                                      \
    {                                                                              \
        (__val) = *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + (__regno));           \
        (__val) |= ((*(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + (__regno))) << 8); \
    }
#define HAL_IDE_READ_ALTSTATUS( __val )  \
    (__val) = *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + 14)


#define HAL_IDE_WRITE_UINT8( __regno, __val )  \
    *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + (__regno)) = (__val);
#define HAL_IDE_WRITE_UINT16( __regno, __val )                                 \
    {                                                                                  \
        *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + (__regno)) = (__val) & 0xff;        \
        *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + (__regno)) = ((__val) >> 8) & 0xff; \
    }
#define HAL_IDE_WRITE_CONTROL( __val )  \
    *(volatile cyg_uint8 *)(IQ8134X_IDE_BASE + 14) = (__val)



static inline int
__wait_for_ready(void)
{
    cyg_uint8 status;
	cyg_uint32 timeout = 0x100000;
    do {
	HAL_IDE_READ_UINT8(IDE_REG_STATUS, status);
    } while ((status & IDE_STAT_BSY) && --timeout);
	if (timeout)
		return 1;
	else
		return 0;
}

static inline int
__wait_for_drq(void)
{
    cyg_uint8 status;
	int threshold = 10000000;

    CYGACC_CALL_IF_DELAY_US(10);
    do {
		HAL_IDE_READ_UINT8(IDE_REG_STATUS, status);
		if (status & IDE_STAT_DRQ) 
		    return 1;
    } while (--threshold > 0);

	diag_printf ("Failed - trying again status: 0x%02x\n", status);

	threshold = 10000000;
    CYGACC_CALL_IF_DELAY_US(10);
    do {
		HAL_IDE_READ_UINT8(IDE_REG_STATUS, status);
		if (status & IDE_STAT_DRQ) 
		    return 1;
    } while (--threshold > 0);
	diag_printf ("Failed  AGAIN - status: 0x%02x\n", status);

    return 0;
}

static int
ide_ident(cyg_uint8 *buf)
{
    unsigned int i;
	cyg_uint8 data;

	if (!(__wait_for_ready()))
		return 0;  // Ready Timed Out

	HAL_IDE_READ_UINT8(IDE_REG_STATUS, data);
    HAL_IDE_WRITE_UINT8(IDE_REG_DEVICE, 0);
	
    HAL_IDE_WRITE_UINT8(IDE_REG_COMMAND, 0xEC);
    CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);

    if (!__wait_for_drq()) {
		diag_printf ("<ide_ident> Wait for DRQ  failed \n");
		return 0;
    }

    for (i = 0; i < (SECTOR_SIZE / sizeof(cyg_uint8)); i++, buf++)
	HAL_IDE_READ_UINT8(IDE_REG_DATA, *buf);

    return 1;
}

int
iq8134x_cf_read_sectors(unsigned long start, unsigned long count, cyg_uint8 *buf)
{
	unsigned int new_count;
	int loop;
	int  i, j;
	cyg_uint8 *p;

	do {
		if (!(__wait_for_ready()))
			return 0;  // Ready Timed out
		if (count > 255) {
			new_count = 0;
			count -= 256;
		}
		else {
			new_count = count;
			count = 0;
		}
		HAL_IDE_WRITE_UINT8(IDE_REG_COUNT, new_count);
		HAL_IDE_WRITE_UINT8(IDE_REG_LBALOW, start & 0xff);
		HAL_IDE_WRITE_UINT8(IDE_REG_LBAMID, (start >>  8) & 0xff);
		HAL_IDE_WRITE_UINT8(IDE_REG_LBAHI,  (start >> 16) & 0xff);
		HAL_IDE_WRITE_UINT8(IDE_REG_DEVICE, ((start >> 24) & 0xf) | 0x40);
		HAL_IDE_WRITE_UINT8(IDE_REG_COMMAND, 0x20);

		(new_count == 0) ? (loop = 256) : (loop = new_count);
		for(p = buf, i = 0; i < loop; i++) {
			if (!__wait_for_drq()) {
				diag_printf("%s: NO DRQ\n", __FUNCTION__);
				diag_printf ("count: %d, new_count: %d, loop: %d, i: %d\n", count, new_count, loop, i);
				return 0;
			}

			for (j = 0; j < SECTOR_SIZE; j++, p++, buf++)
				HAL_IDE_READ_UINT8(IDE_REG_DATA, *p);
		}
		start += loop;  // If we're writing >255 sectors, start needs incremented
	} while (count > 0);
	return 1;
}


int
iq8134x_cf_write_sectors(unsigned long start, unsigned long count, cyg_uint8 *buf)
{
	int  i, j;
	unsigned int new_count;
	int loop;
	cyg_uint8 *p;
	do {
		if (!(__wait_for_ready()))
			return 0;  // Ready Timed Out
		if (count > 255) {  /* We have 256 or more sectors to write - need to batch them */
			new_count = 0;
			count -= 256;
		}
		else {              /* Less than 256 sectors coming - we can do it all at once */
			new_count = count;
			count = 0;	   
		}
		HAL_IDE_WRITE_UINT8(IDE_REG_COUNT, new_count);   // # of sectors (0==256)
		HAL_IDE_WRITE_UINT8(IDE_REG_LBALOW, start & 0xff);
		HAL_IDE_WRITE_UINT8(IDE_REG_LBAMID, (start >>  8) & 0xff);
		HAL_IDE_WRITE_UINT8(IDE_REG_LBAHI,  (start >> 16) & 0xff);
		HAL_IDE_WRITE_UINT8(IDE_REG_DEVICE,	((start >> 24) & 0xf) | 0x40);
		HAL_IDE_WRITE_UINT8(IDE_REG_COMMAND, 0x30);

		(new_count == 0) ? (loop = 256) : (loop = new_count);
		for(p = buf, i = 0; i < loop; i++) {
			if (!__wait_for_drq()) {
				diag_printf ("count: %d, new_count: %d, loop: %d, i: %d\n", count, new_count, loop, i);
				diag_printf("%s: NO DRQ\n", __FUNCTION__);
				return 0;
			}
			for (j = 0; j < SECTOR_SIZE; j++, p++, buf++)
				HAL_IDE_WRITE_UINT8(IDE_REG_DATA, *p);

		}
		start += loop;
	} while (count > 0);
	return 1;
}


// Define table boundaries
CYG_HAL_TABLE_BEGIN( __CF_cmds_TAB__, CF_cmds);
CYG_HAL_TABLE_END( __CF_cmds_TAB_END__, CF_cmds);

extern struct cmd __CF_cmds_TAB__[], __CF_cmds_TAB_END__;

static void
cf_usage(char *why)
{
    diag_printf("*** invalid 'cf' command: %s\n", why);
    cmd_usage(__CF_cmds_TAB__, &__CF_cmds_TAB_END__, "cf ");
}

static int
cf_present(void)
{
    cyg_uint8 val;

	if (!(__wait_for_ready()))
		return 0;  // Ready Timed Out
    // This is reminiscent of a memory test. We write a value
    // to a certain location (device register), then write a
    // different value somewhere else so that the first value
    // is not hanging on the bus, then we read back the first
    // value to see if the write was succesful.
    //
    HAL_IDE_WRITE_UINT8(IDE_REG_DEVICE, 0xA0);
    HAL_IDE_WRITE_UINT8(IDE_REG_FEATURES, 0);
    CYGACC_CALL_IF_DELAY_US(50000);
    HAL_IDE_READ_UINT8(IDE_REG_DEVICE, val);
    return (val == 0xA0);
}

static cyg_uint32 cf_lba_blocks;

static int
cf_ident(void)
{
    cyg_uint32 buf[SECTOR_SIZE/sizeof(cyg_uint32)];

    if (ide_ident((cyg_uint8 *)buf) <= 0)
		return 0;

    if (*(cyg_uint16 *)buf != 0x848a) {
		diag_printf ("<cf_ident> Buf != 0x848a\n");
		diag_printf ("It's : 0x%04x\n", *(cyg_uint16*)buf);
		return 0;
    }

    cf_lba_blocks = *(cyg_uint32 *)((char *)buf + IDE_DEVID_LBA_CAPACITY);

    return 1;
}

static void
cf_info(int argc, char *argv[])
{
    if (!cf_ident()) {
	diag_printf("Unable to identify CF card.\n");
	return;
    }
    diag_printf("CF card with %d blocks of 512 bytes. Total size: %d bytes.\n",
		cf_lba_blocks, cf_lba_blocks * 512);
}

static void
cf_write(int argc, char *argv[])
{
    unsigned long nsects, sector_addr;
    CYG_ADDRESS mem_addr;
    bool mem_addr_set = false;
    bool sector_addr_set = false;
    bool nsects_set = false;
    struct option_info opts[3];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void *)&mem_addr, (bool *)&mem_addr_set, "memory base address");
    init_opts(&opts[1], 's', true, OPTION_ARG_TYPE_NUM, 
              (void *)&sector_addr, (bool *)&sector_addr_set, "starting sector");
    init_opts(&opts[2], 'n', true, OPTION_ARG_TYPE_NUM, 
              (void *)&nsects, (bool *)&nsects_set, "data length [in 512 byte sectors]");
    if (!scan_opts(argc, argv, 2, opts, 3, 0, 0, 0)) {
        cf_usage("invalid arguments");
        return;
    }

    if (!mem_addr_set || !sector_addr_set || !nsects_set) {
        cf_usage("required parameter missing");
        return;
    }

    if (!cf_ident()) {
	diag_printf("Unable to identify CF card.\n");
	return;
    }

    if ((mem_addr < (CYG_ADDRESS)ram_start) ||
        ((mem_addr+(nsects * SECTOR_SIZE)) >= (CYG_ADDRESS)ram_end)) {
        diag_printf("** ERROR: RAM address: %p is invalid\n", (void *)mem_addr);
        diag_printf("   valid range is %p-%p\n", (void *)ram_start, (void *)ram_end);
	return;
    }

    if ((sector_addr + nsects) >= cf_lba_blocks) {
        diag_printf("** ERROR: Attempting to write past end of device.\n");
	return;
    }

    if (!iq8134x_cf_write_sectors(sector_addr, nsects, (cyg_uint8 *)mem_addr)) {
        diag_printf("** ERROR: Writing CF device.\n");
    }
}

static void
cf_read(int argc, char *argv[])
{
    unsigned long nsects, sector_addr;
    CYG_ADDRESS mem_addr;
    bool mem_addr_set = false;
    bool sector_addr_set = false;
    bool nsects_set = false;
    struct option_info opts[3];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void *)&mem_addr, (bool *)&mem_addr_set, "memory base address");
    init_opts(&opts[1], 's', true, OPTION_ARG_TYPE_NUM, 
              (void *)&sector_addr, (bool *)&sector_addr_set, "starting sector");
    init_opts(&opts[2], 'n', true, OPTION_ARG_TYPE_NUM, 
              (void *)&nsects, (bool *)&nsects_set, "data length [in 512 byte sectors]");
    if (!scan_opts(argc, argv, 2, opts, 3, 0, 0, 0)) {
        cf_usage("invalid arguments");
        return;
    }

    if (!mem_addr_set || !sector_addr_set || !nsects_set) {
        cf_usage("required parameter missing");
        return;
    }

    if (!cf_ident()) {
	diag_printf("Unable to identify CF card.\n");
	return;
    }

    if ((mem_addr < (CYG_ADDRESS)ram_start) ||
        ((mem_addr+(nsects * SECTOR_SIZE)) >= (CYG_ADDRESS)ram_end)) {
        diag_printf("** ERROR: RAM address: %p is invalid\n", (void *)mem_addr);
        diag_printf("   valid range is %p-%p\n", (void *)ram_start, (void *)ram_end);
	return;
    }

    if ((sector_addr + nsects) >= cf_lba_blocks) {
        diag_printf("** ERROR: Attempting to read past end of device.\n");
	return;
    }

    if (!iq8134x_cf_read_sectors(sector_addr, nsects, (cyg_uint8 *)mem_addr)) {
        diag_printf("** ERROR: Reading CF device.\n");
    }
}

// Raw compact flash access functions
local_cmd_entry("info",
                "Print information about compact flash card",
                "",
                cf_info,
                CF_cmds
    );

local_cmd_entry("write",
                "Write data to compact flash sectors",
                "-s <sector_number> -b <mem_base> -n <nsectors>",
                cf_write,
                CF_cmds
    );												     

local_cmd_entry("read",
                "Read data from compact flash sectors",
                "-s <sector_number> -b <mem_base> -n <nsectors>",
                cf_read,
                CF_cmds
    );

static void
do_cf(int argc, char *argv[])
{
    struct cmd *cmd;

    if (argc < 2) {
        cf_usage("too few arguments");
        return;
    }
    if (!cf_present()) {
	diag_printf("No CF card installed!\n");
	return;
    }
    if ((cmd = cmd_search(__CF_cmds_TAB__, &__CF_cmds_TAB_END__, 
                          argv[1])) != (struct cmd *)0) {
        (cmd->fun)(argc, argv);
        return;
    }
    cf_usage("unrecognized command");
}


// CLI function
static cmd_fun do_cf;
RedBoot_nested_cmd("cf", 
            "Manage compact flash devices", 
            "{cmds}",
            do_cf,
            __CF_cmds_TAB__, &__CF_cmds_TAB_END__
    );




#define CYGHWR_REDBOOT_ARM_BSD_CMDLINE_ADDRESS 0x400

unsigned long
iq8134x_cf_blocks(void)
{
    if (!cf_ident())
	return 0;
    return cf_lba_blocks;
}

static int
cf_dev_read(unsigned long start_sector, int nsects, void *p)
{
    if (!iq8134x_cf_read_sectors(start_sector, nsects, p))
	return 0;
    return nsects;
}

#endif // CYGPKG_REDBOOT

// ------------------------------------------------------------------------
// EOF iq8134x_cf.c


