//=============================================================================
//
//      battery.c
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter, dmoseley
// Date:        2001-01-25
// Purpose:
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_io.h>
#include "test_menu.h"


extern void diag_wait(void);
extern cyg_uint32 write_i2c_byte(cyg_uint32 id, cyg_uint16 byte, cyg_uint8 data);
extern cyg_uint32 read_i2c_byte(cyg_uint32 id, cyg_uint16 byte);
void   bbu_check(void);
void   bbu_disable(MENU_ARG arg);
static void battery_test_read(MENU_ARG arg);
static void battery_test_write(MENU_ARG arg);

RedBoot_init(bbu_check, RedBoot_INIT_LAST);

#define TEST_PATTERN 0x55aa55aa

// test battery status
// returns 0 if battery installed, -1 if no battery
int
battery_status(MENU_ARG arg)
{
    unsigned char status;
	int ret = 0;

	// Are we on an IQ80315
	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315")) {
		diag_printf ("No Battery Backup on this board\n");
		return(-1);
	}

    // read battery status port
    status = *(unsigned char *)IQ80315_BATTERY_STATUS_ADDR;

    // examine bit b0 BATT_PRES#
    if (status & IQ80315_BATTERY_PRESENT)
		diag_printf("A battery is installed.\n");
    else {
		diag_printf("No battery installed.\n");
		ret = -1;
    }

	if (status & IQ80315_BATTERY_ENABLED)
		diag_printf ("Battery Backup is Enabled\n");
	else
		diag_printf ("Battery Backup is Disabled\n");

	diag_wait();
	return (ret);
}

// Check if we need to verify RAM is still good
void
bbu_check(void)
{
	int result;
	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315"))
		return;

	result = read_i2c_byte(0x50, 2);
	if (result == 1) { // Were we in BBU mode
		battery_test_read((MENU_ARG)NULL);
		diag_printf ("\n\n===> DISABLING Battery Backup <===\n");
		diag_printf ("   >  Re-enable in Diag Menu  <   \n");
		bbu_disable((MENU_ARG)NULL);
	}
}


#ifdef CYGSEM_HAL_ARM_IQ80315_BATTERY_TEST
// Data to be written to address and read after the board has
// been powered off and powered back on

static void
battery_test_write (MENU_ARG arg)
{
	int counter, value;
	volatile unsigned int *location = (volatile unsigned int*)SDRAM_BATTERY_TEST_ADDR;
	unsigned char status;

    // read battery status port
    status = *(unsigned char *)IQ80315_BATTERY_STATUS_ADDR;
	if (!(status & IQ80315_BATTERY_ENABLED)) {
		diag_printf ("Battery Backup is Disabled\n");
		diag_printf ("Enabling....\n");
		status |= IQ80315_BATTERY_ENABLED;
	    *(unsigned char *)IQ80315_BATTERY_STATUS_ADDR = status;
	}
  

	for (counter = 0; counter< 0x1000; counter++)
	    *location++ = TEST_PATTERN;

    diag_printf("The value '%p' is now written to 4k of DRAM starting at address %p.\n",
	   TEST_PATTERN, SDRAM_BATTERY_TEST_ADDR);
    diag_printf("\nPower the board off, wait 60 seconds and power it back on.\n");
	diag_printf("=========================IMPORTANT============================\n");
	diag_printf("- If you see 'FF' on the LEDs, the SDRAM did not retain values\n");
	diag_printf("- Power Cycle the board to clear BBU testing and consider it a BBU Failure\n");
	diag_printf("=========================IMPORTANT============================\n");

    // BBU_Test will be a "secret" option in the BootEPROM - I can't think of anyplace else to put
    // it that the initial platform setup will have easy access to -- cebruns
	value = write_i2c_byte(0x50, 2, 1);
	if (value < 0) {  // Write 1 to enabled BBU Test in BootE2
		diag_printf ("Failed to Write to E2PROM device\n");
		diag_printf ("E2PROM must be installed to run BBU testing\n");
	}
    diag_wait();
}


static void
battery_test_read (MENU_ARG arg)
{
    cyg_uint32 value;
	int counter;
	int fails = 0;
	volatile unsigned int *location = (volatile unsigned int*)SDRAM_BATTERY_TEST_ADDR;

	for (counter = 0; counter < 0x1000; counter++) {
		value = *location;
		if (value != TEST_PATTERN) {
		    diag_printf ("FAILED : Value read: %p at location: 0x%08x\n", value, (unsigned int)location);
			diag_printf ("Expected: 0x%08x\n", TEST_PATTERN);
			fails++;
			if (fails > 10) {
				diag_printf ("FAILS > 10, ABORTING\n");
				break;
			}
		}
		*location++;
	}
			

    if (fails > 0) {
		diag_printf ("\n-------------------------------\n");
		diag_printf ("- The battery test FAILED.    -\n");
		diag_printf ("- SDRAM did NOT Retain values.-\n");
		diag_printf ("-------------------------------\n");
    }
	else {
		diag_printf ("\n\n++++++++++++++++++++++++++++\n");
		diag_printf ("+ The battery test PASSED. +\n");
		diag_printf ("+ SDRAM Retained values.   +\n");
		diag_printf ("++++++++++++++++++++++++++++\n");
	}
    diag_wait();
}


void
bbu_disable (MENU_ARG arg) {

    // Disable BBU 
    *(unsigned char *)IQ80315_BATTERY_STATUS_ADDR &= ~(IQ80315_BATTERY_ENABLED);
	// Disable E2Prom BBU byte
	write_i2c_byte(0x50, 2, 0);
}


void
battery_test_menu (MENU_ARG arg)
{
	int value;
	// Are we on an IQ80315
	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315")) {
		diag_printf ("No Battery Backup on this board\n");
		return;
	}

	value = read_i2c_byte(0x50, 2);  // Make sure we've disabled BBU if no battery
	if (value < 0) {
		diag_printf ("ERROR: BootE2 device must be installed for BBU testing\n");
		diag_wait();
		return;
	}

	if (battery_status((MENU_ARG)NULL) < 0) { 
		write_i2c_byte(0x50, 2, 0);  // Make sure we've disabled BBU if no battery
		return;
	}

    // Test Menu Table
    static MENU_ITEM batteryMenu[] = {
	{"Write data to SDRAM and enable battery backup", battery_test_write, NULL},
	{"Verify data in SDRAM", battery_test_read, NULL},
	{"Disable battery backup", bbu_disable, NULL},
    };

    unsigned int num_menu_items = (sizeof (batteryMenu) / sizeof (batteryMenu[0]));

    char *menu_title = "\n Battery Backup SDRAM memory test.";
    diag_printf ("\n*************************************************************************\n");
    diag_printf ("* This test will enable you to perform a battery test in 4 steps:       *\n");
    diag_printf ("* You must have the E2PROM installed on the IQ80315 for BBU Testing     *\n");
    diag_printf ("*  1/  Select option 1 to write test pattern,                           *\n");
    diag_printf ("*  2/  Power the board off and wait 60 seconds,                         *\n");
    diag_printf ("*  3/  Power the board back on,                                         *\n");
    diag_printf ("*  4/  RedBoot will automatically verify SDRAM when rebooted            *\n");
    diag_printf ("*************************************************************************");

    menu (batteryMenu, num_menu_items, menu_title, MENU_OPT_NONE);
    diag_printf ("\n");

}
#endif // CYGSEM_HAL_ARM_IQ80315_BATTERY_TEST

