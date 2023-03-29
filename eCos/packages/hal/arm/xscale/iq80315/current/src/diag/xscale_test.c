//=============================================================================
//
//      xscale_test.c - Diagnostics
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

/************************************************************************/
/*									*/
/* Modification History							*/
/* --------------------							*/
/* 11oct00, ejb, Created for IQ80310 StrongARM2				*/
/* 18dec00  jwf                                                         */
/* 02feb01  jwf	added tests: _coy_tight_loop, cache_loop, LoopMemTest,  */
/*              special_mem_test written by snc	 		        */
/* 07feb01  jwf added function calls to a variable delay time generator */
/* 09feb01  jwf added function version_info to show version information */
/*              about OS, BOARD, CPLD, 80200 ID, 80312 ID.              */
/* 26may03  ceb added "basic_sanity" and supporting functions for quick */
/*              check of iq80315 platform                               */
/* 05Jan04  ceb SV and IQ boards built off of same tree - added checks  */
/*              for board-specific tests, like CF, SATA not needed on SV*/
/************************************************************************/
//#define SI_TEST

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ioc80314.h>          // Hardware definitions
#include <cyg/hal/iq80315.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf
#include <redboot.h>
#include <cyg/io/pci.h>
#include "test_menu.h"

void hdwr_diag (void);

typedef struct {
		unsigned int product_sku:4;
		unsigned int product_code:4;
		unsigned int board_rework:4;
		unsigned int board_stepping:4;
		unsigned int fpga_revision:8;
} FPGA_CODES;

static void memory_tests (int testNum);
static void rotary_switch (MENU_ARG arg);
static void seven_segment_display (MENU_ARG arg);
static void cache_loop (MENU_ARG arg);
static unsigned int basic_sanity (MENU_ARG arg);
static void loop_basic_sanity ( MENU_ARG arg);
//static void ds2401_test (MENU_ARG arg);
#ifdef SI_TEST
static void si_menu (MENU_ARG arg);
static void write_ddr  (int cpu);
static void read_ddr   (int cpu);
static void write_gmii (int cpu);
static void write_fpga (int cpu);
static void write_fpga16 (int cpu);
static void read_gmii  (int cpu);
static void write_sram (int cpu);
static void read_sram  (int cpu);
static void read_fpga (int cpu);
static void write_phy (unsigned int phy, unsigned int reg, unsigned short value);
#endif // SI_TEST
static unsigned int read_phy (unsigned int phy, unsigned int reg);
static void read_ddr_i2c ( MENU_ARG arg );
static void read_temp_i2c ( MENU_ARG arg );
static unsigned char get_rot_switch (void);
static void get_fpga_codes(FPGA_CODES * p_tc_codes);
//static char send_ds2401_reset (void);
static void eeprom_menu_i2c (MENU_ARG arg );
static void fan_menu (MENU_ARG arg);
// pel
static void	memtest_submenu (MENU_ARG arg);
static int do_pattern_tests(long startAddr, long endAddr);
static int do_pattern_tests2(long startAddr, long endAddr);
static int do_add_data_tests(long startAddr, long endAddr);
static void dump_sdram_regs(void);
static void cache_onoff(void);
	
																	 
extern void rtc_menu (MENU_ARG arg);
extern void test_buzzer (MENU_ARG arg);
extern void pci_test (MENU_ARG arg);
extern int  battery_status(MENU_ARG arg);
extern void battery_test_menu (MENU_ARG arg);
extern void timer_test (MENU_ARG arg);
extern void timer_test2 (MENU_ARG arg);
extern void read_eeprom_i2c (MENU_ARG arg);
extern void write_eeprom_i2c (void);
extern void erase_eeprom_i2c (void);
extern long decIn (void);
extern CYG_ADDRWORD hexIn(void);
extern int memTest (long startAddr, long endAddr);
extern int LoopMemTest (long startAddr, long endAddr);
extern void enet_setup(MENU_ARG arg);
extern void cf_test (MENU_ARG arg);
extern void lcd_menu (MENU_ARG arg);
extern void disable_ecc (int arg, char *argv[]);
extern int check_ecc_errors (void);
extern cyg_int32 read_i2c_byte ( cyg_uint32 dev_id, cyg_uint16 byte );
extern cyg_int32 write_i2c_byte ( cyg_uint32 dev_id, cyg_uint16 byte, cyg_uint8 write_data );

/* Test Menu Table */
static MENU_ITEM testMenu[] =
{
	// pel
	{"Memory Test Sub-Menu", memtest_submenu,0},
//   {"Memory Test", memory_tests, 0},
//   {"Repeating Memory Tests", repeat_mem_test, 0},
//   {"Repeat-On-Fail Memory Test", special_mem_test, 0},
    {"Rotary Switch Test", rotary_switch, 0},
    {"7 Segment LED Tests", seven_segment_display, 0},
    {"IOC80314 Ethernet Configuration", enet_setup, 0},
    {"Battery Status Test", battery_status, 0},
    {"Battery Backup SDRAM Memory Test", battery_test_menu, 0},
    {"Timer Test", timer_test, 0},
#if 0
    {"Timer Test 2", timer_test2, 0},
#endif
    {"PCI Bus test", pci_test, 0},
    {"CPU Cache Loop (No return)", cache_loop, 0},
	{"Read DDR0 and DDR1 Size on I2C", read_ddr_i2c, 0},
	{"Read Temp Sensor(s) on I2C", read_temp_i2c, 0},
	{"Basic Sanity Tests", basic_sanity, 0},
	{"Loop Version of Basic Sanity Tests", loop_basic_sanity, 0},
#ifdef SI_TEST
	{"SI Test Menu", si_menu, 0},
#endif
	{"EEPROM menu", eeprom_menu_i2c, 0},
   	{"RTC menu", rtc_menu, 0},
   	{"Fan menu", fan_menu, 0},
   	{"Test Buzzer", test_buzzer, 0},
   	{"CompactFlash Test", cf_test, 0},
   	{"LCD Test Menu", lcd_menu, 0}
};
																	 
#define NUM_MENU_ITEMS	(sizeof (testMenu) / sizeof (testMenu[0]))
#define MENU_TITLE  "\n IQ80315 Diags"
#define CPU0 0
#define CPU1 1

/* Set Bits for Failure on Loop Testing */
#define FAIL_SDRAM    1
#define FAIL_SRAM     2
#define FAIL_REG_READ 4
#define ALL_PASS      0
/* Data Patterns for SI testing for Writing Memory */
unsigned int data_patterns[] = 
{
(0xaa55aa55),
(0x55aa55aa),
(0xc3c3c3c3),
(0x3c3c3c3c),
(0xf0f0f0f0),
(0x0f0f0f0f),
(0x00000000),
(0xffffffff)
};

// pel:  Globals for new memtests
long Gmem_start=0;
long Gmem_size=0;
int Gcache=1; // will assume cache is on by default
int numIt=1;
int GPassFail=0;
unsigned int enh_data_patterns[] = {
	 0x00000000,
	 0xffffffff,	  
	 0xffff0000,
	 0x0000ffff,
	 0xff00ff00,
	 0x00ff00ff,
	 0xf0f0f0f0,
	 0x0f0f0f0f,
	 0xaaaa5555,
	 0x5555aaaa,
	 0x5a5a5a5a,
	 0x55aa55aa,
	 0xaa55aa55,
	 0xa5a5a5a5, 
	 0xcccc3333,
	 0x3333cccc,
	 0x33cc33cc,
	 0xcc33cc33,
	 0x3c3c3c3c,
	 0xc3c3c3c3};


void
diag_wait(void)
{
    char buf[1];
    diag_printf ("Press return to continue.\n");
    while (_rb_gets(buf, sizeof(buf), 0) != _GETS_OK)
	;
}

void
hdwr_diag (void)
{
    diag_printf ("Entering Hardware Diagnostics - Disabling Data Cache!\n");

//    cyg_pci_init();

    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();

    menu (testMenu, NUM_MENU_ITEMS, MENU_TITLE, MENU_OPT_NONE);

    diag_printf ("Exiting Hardware Diagnostics!\n\n");
    HAL_DCACHE_ENABLE();
}

static void
cache_loop (MENU_ARG arg)
{
    diag_printf ("Putting Processor in a Tight Loop Forever...\n\n");

    asm ( "0: mov r0,r0\n"
	  "b 0b\n");
}

// pel:  New memory test sub menu allows user to setup parms and them run
// any of the available tests w/o having to type in the parms over and over again
// also allows for looping of one or all of the tests
static void
memtest_submenu (MENU_ARG arg)
{
	int choice;
	do {
		diag_printf ("\n\n************************************************************************\n");
		diag_printf ("WARNING:  SDRAM Tests are destructive, know what ranges you are testing!\n");
		diag_printf ("************************************************************************\n");
		diag_printf ("\nCurrently selected base address (hex): %p\n",Gmem_start);
		diag_printf ("Currently selected size to test (hex): %p\n",Gmem_size);
		diag_printf ("Currently selected # of itterations (hex): %d\n",numIt);
		diag_printf ("Data Cache During Test (1=on, 0=off): %d\n\n",Gcache);
		diag_printf ("============================================\n");
		diag_printf ("|         Memory Test Sub Menu             |\n");
		diag_printf ("============================================\n");
		diag_printf ("|              Parameters                  |\n");
		diag_printf ("| 0) Select Test Base and Range            |\n");
		diag_printf ("| 1) Select # of Itterations               |\n");
		diag_printf ("| 2) Enable or Disable Data Cache          |\n");
		diag_printf ("| 3) Disable ECC                           |\n");
		diag_printf ("|                                          |\n");
		diag_printf ("|----------------Tests---------------------|\n");
		diag_printf ("| 4) Basic Memory Tests                    |\n");
		diag_printf ("| 5) Pattern Memory Tests                  |\n");
		diag_printf ("| 6) Pattern Memory Tests Variation        |\n");
		diag_printf ("| 7) Address as Data Memory Tests          |\n");
		diag_printf ("| 8) Run all Tests Sequentially            |\n");
		diag_printf ("| 9) Return to Main Menu                   |\n");
		diag_printf ("============================================\n");
		GPassFail = 0;
		if (Gmem_start==0)
			diag_printf ("WARNING: Currently selected base address is ZERO!\n");
		if (Gmem_size==0)
			diag_printf ("WARNING: Currently selected size to test is ZERO!\n");

		diag_printf ("Enter Selection: ");

		choice = decIn();
		
		if (choice==0) {
		    diag_printf("\nBase address of memory to test (in hex) (Should be >= 0x%08x) : ", workspace_start);
		    Gmem_start = hexIn();
			if (Gmem_start < 0) 
				return;    /* Control C must have been entered */
		    diag_printf ("Size of memory to test (in hex): (Should be < 0x%08x) :", (workspace_end - workspace_start));
		    Gmem_size = hexIn();
			if (Gmem_size < 0) 
				return;    /* Control C must have been entered */
		} else if (choice==1) {
		    diag_printf("Number of iterations to test (in hex): ");
		    numIt = hexIn();
		} else if (choice==2) {
			diag_printf ("New Data Cache Setting for Tests (0=off, 1=on): ");
		    Gcache = decIn();
		} else if (choice==3) {
			disable_ecc((int)NULL, NULL);
		}
		else {
			if (choice==4 || choice==8) { 
			    diag_printf ("\n** STARTING Basic Memory Tests **\n");
				memory_tests(1);
			}
			if (choice==5 || choice==8) {
			    diag_printf ("\n** STARTING Pattern Memory Tests **\n");
				memory_tests(2);
			}
			if (choice==6 || choice==8) {
			    diag_printf ("\n** STARTING Pattern Memory Tests Variation **\n");
				memory_tests(3);
			}
			if (choice==7 || choice==8) {
			    diag_printf ("\n** STARTING Address as Data Memory Tests **\n");
				memory_tests(4);
			}
		}
	} while ((choice < 9) && (choice >= 0));


}

// ***************************************************************************
// repeat_mem_test - Repeating Memory Tests
//
/*
static void
repeat_mem_test (void)
{
    CYG_ADDRWORD start_addr, mem_size, end_addr;
    char        cache_disable[10];

    diag_printf ("Turn off Data Cache? (y/n): ");
    while (_rb_gets(cache_disable, sizeof(cache_disable), 0) != _GETS_OK)
        ;
    diag_printf ("\nBase address of memory to test (in hex): ");
    start_addr = hexIn();
    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();
    end_addr = start_addr + mem_size - 1;
    diag_printf("\nTesting memory from %p to %p", start_addr, end_addr);
    while (memTest (start_addr, end_addr))
        ;
}
*/

// ***************************************************************************
// memory_tests - Basic Memory Tests
// Memory tests can be run one of two ways - with the cache turned OFF to test
// physical memory, or with cache turned ON to test the caching, controlled by
// parm option on mem test submenu
//
static void
memory_tests (int testNum)
{
    long end_addr;
	int i, ecc_check, failed_runs=0;
	char buf[2];

	cache_onoff();
    end_addr = Gmem_start + Gmem_size - 1;

	// pel
	for (i = 0; i < numIt ; i++) {   // Number of iterations to run
		diag_printf ("\nStarting Iteration: %d\n", i+1);
	    diag_printf("Testing memory from %p to %p.\n", Gmem_start, end_addr);

		switch (testNum) {
		case 1:	// basic tests
		    if (memTest(Gmem_start, end_addr)==0)
		    	failed_runs++;
			break;
		case 2: // pattern tests
			if (do_pattern_tests(Gmem_start,Gmem_size)) {
		  		diag_printf ("Failed Pattern Test!\n");
		   		failed_runs++;	   
		  	}
			if (failed_runs > 10) {
				diag_printf ("Exceeded 10 Fails - Aborting\n");
				break;
			}
			break;
		case 3: // pattern tests #2
			if (do_pattern_tests2(Gmem_start,Gmem_size)) {
		  		diag_printf ("Failed Pattern Variation Test!\n");
		   		failed_runs++;	   
		  	}
			if (failed_runs > 10) {
				diag_printf ("Exceeded 10 Fails - Aborting\n");
				break;
			}
			break;

		case 4: // address as data
			if (do_add_data_tests(Gmem_start,Gmem_size)) {
		  		diag_printf ("Failed Pattern Test!\n");
		   		failed_runs++;	   
		  	}
			if (failed_runs > 10) {
				diag_printf ("Exceeded 10 Fails - Aborting\n");
				break;
			}
			break;
 		default:
			break;
		}  // switch
	}  // itterations

    diag_printf ("\nMemory test done.\n\n");
	diag_printf ("Checking for any ECC SingleBit Errors\n");
	ecc_check = check_ecc_errors();
	if (ecc_check < 0) {
		diag_printf ("ECC Disabled...\n");
	}
	else if (ecc_check == 0) {
		diag_printf ("No ECC Errors Found\n");
	}
 	else   /* ecc_check routine will output Failure and where it occurred */
		GPassFail = 1;
	if (failed_runs != 0)
		GPassFail = 1;
	diag_printf ("********** SUMMARY ************\n");
	diag_printf ("Iterations: %d\n", i);
	diag_printf ("Failed Runs:  %d\n", failed_runs);
	diag_printf ("Pass/Fail: %s\n", (GPassFail == 0) ? ("Pass") : ("Fail"));
	diag_printf ("*******************************\n");
	if (GPassFail != 0) {
		diag_printf ("------------------------------\n");
		diag_printf (">>>>> Memory Test Failed <<<<<\n");
		diag_printf ("------------------------------\n");
		diag_printf ("Press return to continue\n");
		    while (_rb_gets(buf, sizeof(buf), 0) != _GETS_OK)
			;
	}

}

// pel:  Used to turn cache on or off just before executing mem tests
static void
cache_onoff(void)
{

	if (Gcache==0)
	{
		diag_printf("Disabling data cache");
	    HAL_DCACHE_SYNC();
    	HAL_DCACHE_DISABLE();
	}
	else
	{
		diag_printf("Enabling data cache");
	    HAL_DCACHE_ENABLE();
	}

}

//
// pel:  Dumps some SDRAM reg addresses on certain failures
//
void dump_sdram_regs(void) {
	unsigned int *p_regs;
//	int i;
//	p_regs = (unsigned int*)FL_SDRAM_REGS;
//	for (i =0; i <= 0x20; i++) {
//		printf ("SDRAM Reg: 0x%08x Value: 0x%08x\n", p_regs, *p_regs);
//		*p_regs++;
//	}
	p_regs = ((unsigned int*)(0x50104000 + 0x160));
	diag_printf ("0x%08x\n", *p_regs++);
	diag_printf ("0x%08x\n", *p_regs++);
	diag_printf ("0x%08x\n", *p_regs++);
	diag_printf ("0x%08x\n", *p_regs++);
	diag_printf ("0x%08x\n", *p_regs++);
	diag_printf ("0x%08x\n", *p_regs++);
}

//
// pel:	Fills memory with each pattern first then starts at the top again and
// verifies that pattern is at all addresses
// and also tries to re-read on failure.  Bails after 10 failures
//
static int
do_pattern_tests(long startAddr, long endAddr)
{
	unsigned int *p_memory;
	unsigned int data;
	unsigned int i;
	int j, fails;
	int failed = 0;

	for (i = 0; i < ((sizeof(enh_data_patterns))/(sizeof(enh_data_patterns[0]))); i++) {
	
		diag_printf ("Filling Pattern: 0x%08x\n", enh_data_patterns[i]);
		p_memory = (unsigned int*)startAddr;
		for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
			*p_memory++ = enh_data_patterns[i];
		}
	   
	   	diag_printf ("Verifying Pattern: 0x%08x\n", enh_data_patterns[i]);
	   	p_memory = (unsigned int*)startAddr;
	   	for (fails = 0, j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
			data = *p_memory;
			if (data != enh_data_patterns[i]) {
				diag_printf ("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, data_patterns[i], data);

				dump_sdram_regs();
				diag_printf("Attemping second read of same address\n");
				data = *p_memory;
				if (data != enh_data_patterns[i])  
					diag_printf ("Second read of same address Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, data_patterns[i], data);
				else
					diag_printf("Second read of same address PASSES\n");
				
				fails++;
				failed = 1;		 
				if (fails > 10) {
					diag_printf ("Exceeded 10 Fails - Aborting\n");
					break;
				}
			}
			*p_memory++;
		}
	}
	return failed;
}

//
// pel:	Variation of other pattern tests in that memory is cleared first, each pattern is
// written and read back immediately before incrementing the address.   Then,
// the whole region is read	sequentially for verification again
//
static int
do_pattern_tests2(long startAddr, long endAddr)
{
	unsigned int *p_memory;
	int j, fails, failed = 0;
	unsigned int i;
	volatile unsigned int data;

	diag_printf ("Clearing SDRAM Test Range\n");
	p_memory = (unsigned int*)startAddr;
 	for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
		*p_memory++ = 0x0;
	}
	for (i = 0, fails=0; i < ((sizeof(enh_data_patterns))/(sizeof(enh_data_patterns[0]))); i++) {
		diag_printf ("Filling Pattern: 0x%08x and immediately verifying\n", enh_data_patterns[i]);
		p_memory = (unsigned int*)startAddr;
		for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
			*p_memory = enh_data_patterns[i];
			data = *p_memory;
			if (data != enh_data_patterns[i]) {
				diag_printf ("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, data_patterns[i], data);
				fails++;
				failed = 1;
				if (fails > 10) {
					diag_printf ("Exceeded 10 Fails - Aborting\n");
					break;
				}
			}
			*p_memory++;
		}
		diag_printf ("Verifying Pattern over full range: 0x%08x\n", enh_data_patterns[i]);
		p_memory = (unsigned int*)startAddr;
		for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
			if (*p_memory != enh_data_patterns[i]) {
				diag_printf ("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, data_patterns[i], *p_memory);
				fails++;
				if (fails > 10) {
					diag_printf ("Exceeded 10 Fails - Aborting\n");
					break;
				}
			}
			else {
				*p_memory = 0x0;
			}
			*p_memory++;
		}
	}
	return failed;

}


/* Write the address of a memory location into that memory location */
/* Write the entire SDRAM and then Verify */
static int do_add_data_tests(long startAddr, long endAddr)
{
	unsigned int *p_memory;
	unsigned int data;
	int j, failed = 0;
	int fails = 0;

	p_memory = (unsigned int*)startAddr;
 	for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
		*p_memory++ = (unsigned int)p_memory;
	}
	diag_printf ("Verifying Data as Addresses\n");
	p_memory = (unsigned int*)startAddr;
	for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
		data = *p_memory;
		if (data != (unsigned int)p_memory) {
			diag_printf ("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, p_memory, data);

				dump_sdram_regs();
				diag_printf("Attemping second read of same address\n");
				data = *p_memory;
				if (data != (unsigned int)p_memory) 
					diag_printf ("Second read of same address failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, p_memory, data);
				else
					diag_printf("Second read of same address PASSES\n");
			fails++;
			failed = 1;
			if (fails > 10) {
				diag_printf ("Exceeded 10 Fails - Aborting\n");
				break;
			}
		}
		*p_memory++;
	}
	/* Now go from top of memory down to bottom */
	diag_printf ("Now Verifying from Top to Bottom\n");
	p_memory = (unsigned int*)(startAddr + endAddr - 4);
	for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
		*p_memory-- = (unsigned int)p_memory;
	}
	diag_printf ("Verifying Data as Addresses\n");
	p_memory = (unsigned int*)(startAddr + endAddr - 4);
	fails=0;
	for (j = (startAddr/4); j < (startAddr/4 + endAddr/4); j++) {
		data = *p_memory;
		if (data != (unsigned int)p_memory) {
			diag_printf ("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n", p_memory, p_memory, data);
			fails++;
			failed = 1;
			if (fails > 10) {
				diag_printf ("Exceeded 10 Fails - Aborting\n");
				break;
			}
		}
		*p_memory--;
	}
	return failed;

}

// ****************************************************************************
// special_mem_test - Repeat-On-Fail Memory Test
//
// Memory tests can be run one of two ways - with the cache turned OFF to test
// physical memory, or with cache turned ON to test the caching
//
/*
static void
special_mem_test (MENU_ARG arg)
{
    long	end_addr;

    end_addr = Gmem_start + Gmem_size - 1;
    diag_printf("\nTesting memory from %p to %p.\n", Gmem_start, end_addr);

    LoopMemTest(Gmem_start, end_addr);

}
*/

static unsigned char led_data[] = {
    DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3, DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
    DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B, DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F,
    DISPLAY_PERIOD, DISPLAY_OFF
};

// sequential test for LSD and MSD 7 segment Leds
//
void
seven_segment_display (MENU_ARG arg)
{
    unsigned char SevSegDecode;
    int DisplaySequence;
    int SelectLed;

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_OFF;

    SelectLed=0; /* initialize 7 segment LED selection */

    for (SelectLed = 0; SelectLed < 2; SelectLed++) {
	/* run test data sequence for a 7 segment LED */
	for (DisplaySequence = 0; DisplaySequence <= 17; ++DisplaySequence ) {
	    /* fetch 7 segment decode byte */
	    SevSegDecode = led_data[DisplaySequence];

	    /* display test data on selected 7 segment LED */
	    /* the test data sequence for a 7 segment led will be seen as:*/
	    /* 0 1 2 3 4 5 6 7 8 9 A b C d e F . */
	    if (SelectLed)
		*(volatile unsigned char *) DISPLAY_LEFT = SevSegDecode;
	    else
		*(volatile unsigned char *) DISPLAY_RIGHT = SevSegDecode;

	    CYGACC_CALL_IF_DELAY_US((cyg_int32)4*100000);
	}
    }
}


// tests rotary switch status, S1 positions 0-3, a 2 bit output code
static void
rotary_switch (MENU_ARG arg)
{
    char recv_data[1];
    const unsigned char MAX_SWITCH_SAMPLES = 9;
    unsigned char RotarySwitch[MAX_SWITCH_SAMPLES]; // multiple samples of a 4 bit switch code
    unsigned char index;		// index for Rotary Switch array
    unsigned char debounce;		// keeps tally of equal rotary switch data reads in a loop
    unsigned char SevSegDecode;		// holds decode data for a 7 segment LED display

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_OFF;
    diag_printf("\n\nThe 7-Segment LSD LED shows the Rotary Switch position selected, i.e., 0-F.");
    diag_printf("\n\nSlowly dial the Rotary Switch through each position 0-F and confirm reading.");

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
    while (_rb_gets(recv_data, sizeof(recv_data), 50) != _GETS_OK) {
	do {
	    for(index = 0; index <= MAX_SWITCH_SAMPLES; index++) {
        RotarySwitch[index] = *(volatile unsigned char *) IQ80315_ROTARY_SWITCH_ADDR;
		RotarySwitch[index] &= 0x0f;
	    }
	    debounce = 0;
	    for(index = 1; index <= MAX_SWITCH_SAMPLES; index++) {
		if (RotarySwitch[0] == RotarySwitch[index])
		    debounce++;	// keep tally of equal rotary switch code samples
	    }
	} while (debounce < (MAX_SWITCH_SAMPLES - 1));

	// decipher state of rotary switch position
	if (RotarySwitch[0] > 16)
	    RotarySwitch[0] = 16;
	
	RotarySwitch[0] = (~(RotarySwitch[0]) & 0xf);

	SevSegDecode =  led_data[RotarySwitch[0]];
	// display the rotary switch position on the 7 segment LSD LED as: 0, 1, 2, 3, etc.
	*(volatile unsigned char *)DISPLAY_RIGHT = SevSegDecode;
    }

} // end rotary_switch()




// Use for basic checkout before more diags become available
//
static unsigned int
basic_sanity (MENU_ARG arg)
{
	unsigned int status;
	unsigned int return_status = 0;
	FPGA_CODES tc_codes;

	get_fpga_codes(&tc_codes);

	diag_printf ("\n>>>>Basic Sanity Checkout for %s<<<<<\n", HAL_PLATFORM_BOARD);
	diag_printf ("Product Code: SKU=0x%x, Code=0x%x\n",  tc_codes.product_sku, tc_codes.product_code);
	diag_printf ("Board Rework: 0x%0x, Board Stepping: 0x%0x\n", tc_codes.board_rework, tc_codes.board_stepping);
	diag_printf ("FPGA FW Revision: 0x%0x\n", tc_codes.fpga_revision);
	diag_printf ("Rotary Switch is at position: 0x%0x\n", (unsigned int)get_rot_switch());
	diag_printf ("Starting Tests::::\n\n");
    diag_printf ("Memory Test on SDRAM\n");	
    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_1;
	
	diag_printf ("Testing from 0x%08x to 0x%08x\n", workspace_start, workspace_end);
	status = memTest ( (long)workspace_start, (long)workspace_end); 
	if (status == 0) {
		diag_printf ("\n\n********Failed SDRAM Test\n");
	    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_F;
    	*(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_1;
		if (arg != (MENU_ARG)0xff) {
			for (;;) {
			}
		}
		else
			return_status |= FAIL_SDRAM; /* Log SDRAM failure for looping version */
	}
	else {
		diag_printf ("\nSDRAM Test Passed\n");
	    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_P;
	    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_1;
	}
	diag_printf ("\nMemory Test on SRAM\n");
	diag_printf ("Testing from 0x%08x to 0x%08x\n", HAL_IOC80314_SRAM_BASE, (HAL_IOC80314_SRAM_SIZE+HAL_IOC80314_SRAM_BASE-1));
	status = memTest (HAL_IOC80314_SRAM_BASE, (HAL_IOC80314_SRAM_BASE+HAL_IOC80314_SRAM_SIZE-1));
    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_0;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_2;
	if (status == 0) {
		diag_printf ("\n\n\n\n********Failed SRAM Test\n");
	    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_F;
    	*(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_2;
		if (arg != (MENU_ARG)0xff) {
			for (;;) {
			}
		}
		else
			return_status |= FAIL_SRAM;
	}
	else {
		diag_printf ("\nSRAM Test Passed\n");
	    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_P;
	    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_2;
	}
	diag_printf ("\nSeven Seg Display Tests\n");
    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_0;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_3;
	seven_segment_display( 0 );

	diag_printf ("\nReading PHY 0 Identifier\n");
    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_0;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_4;
		status = read_phy (0, 2);
	if (status != 0x141) {
		diag_printf ("\n\n********Failed PHY READ\n");
		diag_printf ("Phy0 ID incorrect: Should be 0x141, but read as: 0x%x\n", status);
		if (arg != (MENU_ARG)0xff) {
			for (;;) {
			}
		}
	}
	else {
		diag_printf ("Phy0 ID correct: 0x%x\n", status);
    	*(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_P;
	    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_4;
	}

	diag_printf ("\nReading PHY 1 Identifier\n");
    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_0;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_5;
	status = read_phy (1, 2);
	if (status != 0x141) {
		diag_printf ("\n\n********Failed PHY READ\n");
		diag_printf ("Phy1 ID incorrect: Should be 0x141, but read as: 0x%x\n", status);
		if (arg != (MENU_ARG)0xff) {
			for (;;) {
			}
		}
	}
	else {
		diag_printf ("Phy1 ID correct: 0x%x\n", status);
    	*(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_P;
	    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_5;
	}
	
	diag_printf ("Reading Back SRAM BAR Register\n");
	status = *(unsigned int*)0x50103010;
	if (status != 0x50000001) {
		diag_printf ("\n\n********Failed Internal Register Read of IOC80314\n");
		if (arg != (MENU_ARG)0xff) {
			for (;;) {
			}
		}
		else
			return_status |= FAIL_REG_READ;
	}
	return (return_status);
}

/* Loop Basic Sanity - Continuously Call Basic Sanity Tests */
/*                     and keep track of fails/passes for   */
/*                     environmental testing - cebruns      */
static void
loop_basic_sanity (MENU_ARG arg) {
	char buf[2];
	unsigned int loop_counter = 0;
	unsigned int sram_fail = 0;
	unsigned int sdram_fail = 0;
	unsigned int fail_reg   = 0;
	unsigned int all_pass = 0;
	unsigned int status;

    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();

	diag_printf ("Starting BASIC SANITY LOOP TEST\n");
	diag_printf ("Strike <CR> to End Testing\n");
	while (_rb_gets(buf, sizeof(buf), 5000) != _GETS_OK) {
		loop_counter++;
		status = basic_sanity ((MENU_ARG)0xff);
		if (status & 0x1)
			sdram_fail++;
		if (status & 0x2)
			sram_fail++;
		if (status & 0x4)
			fail_reg++;
		if (!status)
			all_pass++;
		diag_printf ("==================================\n");
		diag_printf ("|     Looping Basic Sanity       |\n");
		diag_printf ("==================================\n");
		diag_printf ("Total Loops Executed: %u\n", loop_counter);
		diag_printf ("Total SDRAM Fails   : %u\n", sdram_fail);
		diag_printf ("Total SRAM  Fails   : %u\n", sram_fail);
		diag_printf ("Total Reg_Read Fails: %u\n", fail_reg);
		diag_printf ("\nStrike <CR> *NOW* to End Testing\n");

	}
}

unsigned int
read_phy (unsigned int phy, unsigned int reg) {
	volatile unsigned int *MacConfig1 = (volatile unsigned int*)0x50106000;
	volatile unsigned int *MiiConfig  = (volatile unsigned int*)0x50106020;
	volatile unsigned int *MiiCommand = (volatile unsigned int*)0x50106024;
	volatile unsigned int *MiiAddress = (volatile unsigned int*)0x50106028;
	volatile unsigned int *MiiStatus  = (volatile unsigned int*)0x50106030;
	volatile unsigned int *MiiIndicat = (volatile unsigned int*)0x50106034;
	int counter; 

    *(unsigned int*)MacConfig1 = 0x80000000;
    *(unsigned int*)MiiConfig   = 0x80000000;
    for (counter = 0; counter < 0x5000; counter++)
                ;
    *(unsigned int*)MacConfig1 = 0x00000000;
    *(unsigned int*)MiiConfig   = 0x00000007;

	*MiiAddress = ((phy << 8) | (reg));
	*MiiCommand = 0x0;
	*MiiCommand = 0x1;
	while (*MiiIndicat & 0x1) { // Wait for Busy to clear
	}
	*MiiCommand = 0x0;
	return (*MiiStatus & 0xffff);
}


/*
static void
ds2401_test (MENU_ARG arg) {
	diag_printf ("Sending Reset/Presence code to DS2401\n");
	diag_printf ("Data returned from presence is: 0x%x\n",  send_ds2401_reset());
}
*/

static void
get_fpga_codes (FPGA_CODES *p_tc_codes) {
	unsigned char data;
	data = *(volatile unsigned char*)IQ80315_PRODUCT_CODE_ADDR;
	p_tc_codes->product_sku = (data & 0xf);
	p_tc_codes->product_code = ((data & 0xf0) >> 4);

	data = *(volatile unsigned char*)IQ80315_BOARD_REVISION_ADDR;
	p_tc_codes->board_rework   = (data & 0xf);
	p_tc_codes->board_stepping = ((data & 0xf0) >> 4);

	data = *(volatile unsigned char*)IQ80315_FPGA_REVISION_ADDR;
	p_tc_codes->fpga_revision = data;
}


static unsigned char
get_rot_switch(void) {
	return((~(*(unsigned char*)IQ80315_ROTARY_SWITCH_ADDR))& 0xf);
}


/* Two 1-wire protocol devices attached to FPGA at 0x4e8e0000 */
/* Bit 0 of 0x4e8e0000 is data */
/* Bit 1 of 0x4e8e0000 is bus control: 0: tri-state, 1: enabled */
/* Use tri-state for reads/releasing bus, and 1 for driving bus */
/* Have to use SEARCH_ROM command to select/de-select ESN devices */
/*
static char
send_ds2401_reset (void) {
	unsigned char count, read_cmd, write_data, write_one, write_zero, tristate, read_data, search_rom;
	unsigned char* esn = (unsigned char*)IQ80315_ESERIAL_NUMBER_ADDR;
	struct {
		unsigned int family:8;
		unsigned int serial_low:32;
		unsigned int serial_high:16;
		unsigned int crc:8;
	} ds2401 ;
	write_one = 0x3;
	write_zero = 0x2;
	tristate = 0x0;
	read_cmd = 0x33;
	search_rom = 0xf0;

	// Send Reset Pulse 0x0 and enable drive
	*esn = write_zero;	
	CYGACC_CALL_IF_DELAY_US(600);
	// Release Pulse and tristate bus
	*esn = tristate;
	CYGACC_CALL_IF_DELAY_US(100);


	// Send Search ROM Command (0xF0)
	for (count = 0; count < 8; count++) {
		write_data = ((search_rom >> count) & 0x1);
		if (write_data) {
			*esn = write_zero;
			CYGACC_CALL_IF_DELAY_US(7);
			*esn = tristate;
			CYGACC_CALL_IF_DELAY_US(60);
		}
		else {
			*esn = write_zero;
			CYGACC_CALL_IF_DELAY_US(70);
			*esn = tristate;
			CYGACC_CALL_IF_DELAY_US(40);
		}
	}
	ds2401.family = 0;
	// Read out first bits from ESN devices 
	for (count = 0; count < 2; count++) {
		*esn = write_zero;
		CYGACC_CALL_IF_DELAY_US(1);
		*esn = tristate;
		CYGACC_CALL_IF_DELAY_US(12);
		ds2401.family |= (*esn << count);
	}
	diag_printf ("SCAN ROM returned with: 0x%0x\n", ds2401.family);
	return(0);

	// Start Reading Family Data
	for (count = 0; count < 8; count++) {
		*esn = write_zero;
		CYGACC_CALL_IF_DELAY_US(1);
		*esn = tristate;
		CYGACC_CALL_IF_DELAY_US(12);
		ds2401.family |= (*esn << count);
	}
	diag_printf ("Family Value is: 0x%0x\n", ds2401.family);
	// Start Reading Serial Low Data
	for (count = 0; count < 32; count++) {
		*esn = write_zero;
		CYGACC_CALL_IF_DELAY_US(1);
		*esn = tristate;
		CYGACC_CALL_IF_DELAY_US(12);
		ds2401.serial_low |= (*esn << count);
	}
	// Start Reading Serial High Data 
	for (count = 0; count < 16; count++) {
		*esn = write_zero;
		CYGACC_CALL_IF_DELAY_US(1);
		*esn = tristate;
		CYGACC_CALL_IF_DELAY_US(12);
		ds2401.serial_high |= (*esn << count);
	}
	diag_printf ("Serial Value is: 0x%0x%x\n", ds2401.serial_high, ds2401.serial_low);
	return (0x0);
}
*/

#ifdef SI_TEST
// Use for SI testing - setup looping writes/reads to various interfaces
void
si_menu (MENU_ARG arg)
{
	FPGA_CODES tc_codes;
	int choice;
	int show_menu = 1;

	get_fpga_codes(&tc_codes);
	do {
		if (show_menu == 1) {
			diag_printf ("\n\n");
			diag_printf ("Product Code: SKU=0x%x, Code=0x%x\n",  tc_codes.product_sku, tc_codes.product_code);
			diag_printf ("Board Rework: 0x%0x, Board Stepping: 0x%0x\n", tc_codes.board_rework, tc_codes.board_stepping);
			diag_printf ("FPGA FW Revision: 0x%0x\n", tc_codes.fpga_revision);
			diag_printf ("Rotary Switch is at position: 0x%0x\n", (unsigned int)get_rot_switch());
			diag_printf ("========================================================\n");
			diag_printf ("|              SI Menu for IQ80315                     |\n");
			diag_printf ("|      - These tests are run from CPU0 only -          |\n");
			diag_printf ("========================================================\n");
			diag_printf ("| 1) Write to DDR                                      |\n");
			diag_printf ("| 2) Write to SRAM                                     |\n");
			diag_printf ("| 3) Write to GMII                                     |\n");
			diag_printf ("| 4) Write to FPGA                                     |\n");
			diag_printf ("| 5) Reserved for future use...                        |\n");
			diag_printf ("| 6) Reserved for future use...                        |\n");
			diag_printf ("| 7) Read from DDR                                     |\n");
			diag_printf ("| 8) Read from SRAM                                    |\n");
			diag_printf ("| 9) Read from GMII                                    |\n");
			diag_printf ("|10) Read from FPGA                                    |\n");
			diag_printf ("|11) Reserved for future use...                        |\n");
			diag_printf ("|12) Reserved for future use...                        |\n");
			diag_printf ("|13) _EXIT_                                            |\n");
			diag_printf ("========================================================\n");
		}
		diag_printf ("Enter Selection (0 to show menu) >\n");
		show_menu = 0;
		choice = decIn();
		switch (choice) {
		case 0:
			show_menu = 1;
			break;
		case 1:
			diag_printf ("Executing Write to DDR          \n");
			write_ddr(CPU0);
			break;
		case 2:
			diag_printf ("Executing Write to SRAM          \n");
			write_sram(CPU0);
			break;
		case 3:
			diag_printf ("Executing Write to GMII          \n");
			write_gmii(CPU0);
			break;
		case 4:
			diag_printf ("Executing Write to FPGA          \n");
			write_fpga(CPU0);
			break;
		case 5:
			diag_printf ("Executing Write to PCI-Primary          \n");
			diag_printf ("No implemented yet...\n");
			/* Insert function here */
			break;
		case 6:
			diag_printf ("Executing Write to PCI-Secondary          \n");
			diag_printf ("No implemented yet...\n");
			/* Insert function here */
			break;
		case 7:
			diag_printf ("Executing Read from DDR \n");
			read_ddr(CPU0);
			break;
		case 8:
			diag_printf ("Executing Read from SRAM \n");
			read_sram(CPU0);
			break;
		case 9:
			diag_printf ("Executing Read from GMII \n");
			read_gmii(CPU0);
			break;
		case 10:
			diag_printf ("Executing Read from FPGA \n");
			read_fpga(CPU0);
			break;
		case 11:
			diag_printf ("Executing Read from PCI-Primary \n");
			diag_printf ("No implemented yet...\n");
			/* Insert function here */
			break;
		case 12:
			diag_printf ("Executing Read from PCI-Secondary \n");
			diag_printf ("No implemented yet...\n");
			/* Insert function here */
			break;
		default:
			diag_printf ("Buh-bye\n");
			break;
		}
	}
	while ((choice < 13) && (choice >= 0));

}

static void
write_ddr(int cpu) {
	char buf[1];
	unsigned int counter1, counter2;
	unsigned int* p_memory = (unsigned int*)0x80000;
	unsigned int* p_data;

	while (_rb_gets(buf, sizeof(buf), 2000) != _GETS_OK) {
		p_data = data_patterns;
		for (counter1 = 0; counter1 < (sizeof(data_patterns)/(sizeof(data_patterns[0]))); counter1++, *p_data++) {
			diag_printf ("Writing 0x%08X\n", *p_data);
			for (counter2 = 0; counter2 < 0x2000000; counter2++) {
				*p_memory++ = *p_data;
			}
			p_memory = (unsigned int*)0x80000;
		}
		diag_printf( "\n\nStrike <CR> *NOW* to exit this test.\n" );
	}
}

static void
write_sram(int cpu) {
	char buf[1];
	unsigned int counter1, counter2;
	unsigned int* p_memory = (unsigned int*)SRAM_BASE;
	unsigned int* p_data;

	while (_rb_gets(buf, sizeof(buf), 5) != _GETS_OK) {
		p_data = data_patterns;
		for (counter1 = 0; counter1 < (sizeof(data_patterns)/(sizeof(data_patterns[0]))); counter1++, *p_data++) {
			diag_printf ("Writing 0x%08X\n", *p_data);
			for (counter2 = 0; counter2 < (IQ80315_SRAM_SIZE / 4); counter2++) {
				*p_memory++ = *p_data;
			}
			p_memory = (unsigned int*)SRAM_BASE;
		}
	}
}

static void
read_ddr(int cpu) {
	char buf[1];
	int counter1;
	unsigned int* p_memory = (unsigned int*)0x80000;
	unsigned int* p_data =   data_patterns;

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
	/* Load Memory with known data */
	for (counter1 = 0; counter1 < 0x2000000; counter1++) {
		*p_memory++ = *p_data;
	}	
	/* Start Reading Data back */
    while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		for (p_memory = (unsigned int*)0x80000, counter1 = 0; counter1 < 0x2000000; counter1++) {
			if (*p_memory != *p_data) {
				diag_printf ("Failed on Read at location: 0x%0x\n", p_memory);
				diag_printf ("Expected: 0x%08X\n", *p_data);
				diag_printf ("Received: 0x%08X\n", *p_memory);
			}
			*p_memory++;
		}
   	}
}

static void
read_sram(int cpu) {
	char buf[1];
	int counter1;
	unsigned int* p_memory = (unsigned int*)SRAM_BASE;
	unsigned int* p_data =   data_patterns;

	diag_printf( "\n\nStrike <CR> to exit this test.\n" );
	/* Load Memory with known data */
	for (counter1 = 0; counter1 < (IQ80315_SRAM_SIZE / 4); counter1++) {
		*p_memory++ = *p_data;
	}

	/* Start Reading Data back */
	while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		for (p_memory = (unsigned int*)SRAM_BASE, counter1 = 0; counter1 < (IQ80315_SRAM_SIZE / 4); counter1++) {
			if (*p_memory != *p_data) {
				diag_printf ("Failed on Read at location: 0x%08X\n", p_memory);
				diag_printf ("Expected: 0x%08X\n", *p_data);
				diag_printf ("Received: 0x%08X\n", *p_memory);
			}
			*p_memory++;
		}
	}
}

static void
read_gmii(int cpu) {
	char buf[1];
	int counter1;
	unsigned int data;

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
	/* Read registers 0-9 of PHY */
    while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		for (counter1 = 0; counter1 < 10; counter1++) {
			data = read_phy(0, counter1);
			data = read_phy(1, counter1);
		}
	}
}

static void
write_gmii (int cpu) {
	char buf[1];

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
	/* Continuously Write 0x0 to PHY Interrupt Enable register */
    while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		write_phy(0, 18, 0x0);
		write_phy(1, 18, 0x0);
	}
}

static void
write_fpga (int cpu) {
	char buf[1];
	unsigned char data = 0xa5;
	unsigned char* p_memory;
	unsigned int counter1;

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
    while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		for (p_memory = (unsigned char*)0x4e600000, counter1 = 0; counter1 < 0x80000; counter1++) {
			*p_memory++ = data;
		}
    }
}

static void
write_fpga16 (int cpu) {
	char buf[1];
	unsigned short data = 0xff00;
	unsigned short* p_memory;
	unsigned int counter1;

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
    while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		for (p_memory = (unsigned short*)0x4e600000, counter1 = 0; counter1 < 0x40000; counter1++) {
			*p_memory++ = data;
		}
    }
}

static void
read_fpga (int cpu) {
	char buf[1];
	unsigned char data = 0xa5;
	unsigned char* p_memory = (unsigned char*)0x4e600000;
	unsigned int counter1;

    diag_printf( "\n\nStrike <CR> to exit this test.\n" );
	/* Fill Registers with known data */
	for (counter1 = 0; counter1 < 0x80000; counter1++) {
		*p_memory++ = data;
	}
    while (_rb_gets(buf, sizeof(buf), 50) != _GETS_OK) {
		for (p_memory = (unsigned char*)0x4e600000, counter1 =0; counter1 < 0x80000; counter1++) {
			if (*p_memory != data) {
				diag_printf ("Failed : Data doesn't match\n");
				diag_printf ("Expected: 0x%08X\n", data);
				diag_printf ("Received: 0x%08X\n", *p_memory);				
			}
		}
    }
}


static void
write_phy (unsigned int phy, unsigned int reg, unsigned short value) {
	volatile unsigned int *MacConfig1 = (unsigned int*)0x50106000;
	volatile unsigned int *MiiCommand = (unsigned int*)0x50106024;
	volatile unsigned int *MiiAddress = (unsigned int*)0x50106028;
	volatile unsigned int *MiiControl = (unsigned int*)0x5010602C;
	volatile unsigned int *MiiIndicat = (unsigned int*)0x50106034;

	*MacConfig1 = 0x0;
	*MiiAddress = ((phy << 8) | (reg));
	*MiiCommand = 0x0;
	*MiiControl = value;        // Causes write cycle to PHY
	while (*MiiIndicat & 0x1) { // Wait for Busy to clear
	}
}

#endif // SI_TEST

static void
read_ddr_i2c ( MENU_ARG arg ) {
	volatile unsigned int* p_i2c_cntrl1;
	volatile unsigned int* p_i2c_cntrl2;
	volatile unsigned int* p_i2c_rd_data;
	volatile unsigned int* p_i2c_wr_data;
	unsigned int  devcode, dev_id, page_select, counter;
	unsigned int  reg_id, ctl_data = 0;
	unsigned int  rows, density, fail;
	char buf[1];

	rows = density = 0;

	p_i2c_cntrl1 = (volatile unsigned int*)SDRAM_I2C_CNTRL1;
	p_i2c_cntrl2 = (volatile unsigned int*)SDRAM_I2C_CNTRL2;
	p_i2c_rd_data= (volatile unsigned int*)SDRAM_I2C_RD_DATA;
	p_i2c_wr_data= (volatile unsigned int*)SDRAM_I2C_WR_DATA;

	for (dev_id = 0x50; dev_id < 0x52; dev_id++) {
		page_select = ((dev_id & 0x78) >> 3);
		devcode     = ((dev_id & 0x7)  << 8);
		fail = 0;
		reg_id = SDRAM_SPD_MODULE_ROWS;
		for (counter = 0; (counter < 2 && fail == 0); counter++, reg_id = SDRAM_SPD_ROW_DENSITY) {
			*p_i2c_cntrl1 = (page_select | devcode | (reg_id << 16));
			*p_i2c_cntrl2 = 0x100;
			do {
				ctl_data = *p_i2c_cntrl2;
			} while (ctl_data & 0x100);
			if ((ctl_data >> 24) & 0xf)	{
				switch (ctl_data >> 24) {
				case 1:
					diag_printf ("AHB Master Transfer Error on DDR_%d\n", (dev_id & 0x1));
					fail = 1;
					break;
				case 2:
					diag_printf ("Write Protect Address Violation on DDR_%d\n", (dev_id & 0x1));
					fail = 1;
					return;
				case 4:
					diag_printf ("I2C Timeout - no device installed on DDR_%d\n", (dev_id & 0x1));
					fail = 1;
					break;
				case 8:
					diag_printf ("Init-State-Machine-Config-Error - on DDR_%d\n", (dev_id & 0x1));
					fail = 1;
					break;
				default:
					diag_printf ("Unknown I2C Error: ctl_data is: 0x%08X\n", ctl_data);
					fail = 1;
				}
			}
			else 
				(counter == 0) ? (rows = (*p_i2c_rd_data & 0xff)) : (density = (*p_i2c_rd_data &0xff));
		}
		if (!fail) {
			if (density == 1)   // Check for 1G/2G Density
				density = 0x100;
			else if (density == 2)
				density = 0x200;
			diag_printf ("%dM installed in DDR_%d\n", (rows*(density << 2)), (dev_id & 0x1));
		}

	}
	diag_printf ("\nStrike <CR> to exit\n");
	while (_rb_gets(buf, sizeof(buf), 0) != _GETS_OK)
		;
}

static void
read_temp_i2c ( MENU_ARG arg ) {
	volatile unsigned int* p_i2c_cntrl1;
	volatile unsigned int* p_i2c_cntrl2;
	volatile unsigned int* p_i2c_rd_data;
	volatile unsigned int* p_i2c_wr_data;
	unsigned int  dev_id, devcode, page_select, counter;
	unsigned int  reg_id = 0, ctl_data = 0;
	char neg = ' ';
	char buf[1];

	p_i2c_cntrl1 = (volatile unsigned int*)I2C_CNTRL1;
	p_i2c_cntrl2 = (volatile unsigned int*)I2C_CNTRL2;
	p_i2c_rd_data= (volatile unsigned int*)I2C_RD_DATA;
	p_i2c_wr_data= (volatile unsigned int*)I2C_WR_DATA;

	diag_printf ("\nStrike <CR> to exit\n");
	while (_rb_gets(buf, sizeof(buf), 500) != _GETS_OK) {
		for (counter = 0; counter < 2; counter++) {
			(counter) ? (dev_id = 0x49) : (dev_id = 0x48);
			if (strcmp (HAL_PLATFORM_BOARD, "IQ80315")) {  // We're a TC, so DevId is different (of course)
				counter = 1;
				dev_id = 0x4a;
			}
			page_select = ((dev_id & 0x78) >> 3);
			devcode     = ((dev_id & 0x7)  << 8);
			*p_i2c_cntrl1 = (page_select | devcode | (reg_id << 16));

			*p_i2c_cntrl2 = 0x101;  // Read two bytes for temperature
			do {
				ctl_data = *p_i2c_cntrl2;
			} while (ctl_data & 0x100);

			if ((ctl_data >> 24) & 0xf)	{
				switch (ctl_data >> 24) {
				case 1:
					diag_printf ("AHB Master Transfer Error\n");
					break;
				case 2:
					diag_printf ("Write Protect Address Violation\n");
					break;
				case 4:
					diag_printf ("I2C Timeout - no device installed\n");
					break;
				case 8:
					diag_printf ("Init-State-Machine-Config-Error - Too many 8-byte reads\n");
					break;
				default:
					diag_printf ("Unknown I2C Error: ctl_data is: 0x%08X\n", ctl_data);
				}
			}
			else {
				ctl_data = ((*p_i2c_rd_data >> 4) & 0xfff);
				if (ctl_data & 0x800) { /* Below Zero */
					ctl_data = (~ctl_data) + 1;
					ctl_data &= 0xfff;
					neg = '-';
				}
				else
					neg = ' ';
				ctl_data *= 0.0625;
					diag_printf ("Temperature of Device: 0x%x is: %c%3uC ", dev_id, neg, ctl_data);
					if (counter) {
						diag_printf ("\r");
				}
			}
		}
	}
}


static void
eeprom_menu_i2c (MENU_ARG arg) {
	cyg_uint32 choice;
	do {
		diag_printf ("\n\n");
		diag_printf ("=======================================\n");
		diag_printf ("|  I2C EEPROM menu for %s         |\n", HAL_PLATFORM_BOARD);
		diag_printf ("=======================================\n");
		diag_printf ("| 1) Read  EEPROM                      |\n");
		diag_printf ("| 2) Write EEPROM                      |\n");
		diag_printf ("| 3) Erase EEPROM                      |\n");
		diag_printf ("---------------------------------------\n");
		choice = hexIn();
		switch (choice) {
		case 1:
			read_eeprom_i2c(arg);
			break;
		case 2:
			write_eeprom_i2c();
			break;
		case 3:
			erase_eeprom_i2c();
			break;
		case (_GETS_CTRLC):
			return;
		default:
			diag_printf ("Invalid selection\n");
		}
	} while (choice);

}

static void
fan_menu (MENU_ARG arg) {
	cyg_uint32 choice;
	cyg_uint32 front_id = 0x4b;
	cyg_uint32 rear_id  = 0x1f;
	cyg_uint32 data_front, data_back;

	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315")) {
		diag_printf ("No Fans on this board\n");
		return;
	}

	data_front = read_i2c_byte (front_id, 0x2);
	if (data_front < 0)
		return;
	data_back = read_i2c_byte (rear_id, 0x2);
	if (data_back < 0)
		return;
	do {
		diag_printf ("\n\n");
		diag_printf ("=======================================\n");
		diag_printf ("|  I2C Case Fan menu for IQ80315       |\n");
		diag_printf ("=======================================\n");
		diag_printf ("| 1) Turn Front Case Fans On           |\n");
		diag_printf ("| 2) Turn Front Case Fans Off          |\n");
		diag_printf ("| 3) Turn Rear  Case Fans On           |\n");
		diag_printf ("| 4) Turn Rear  Case Fans Off          |\n");
		diag_printf ("---------------------------------------\n");
		choice = decIn();
		switch (choice) {
		case 1:
			data_front &= ~(0x30);
			write_i2c_byte (front_id, 0x2, data_front);
			break;
		case 2:
			data_front &= ~(0x20);
			data_front |= 0x10;
			write_i2c_byte (front_id, 0x2, data_front);			
			break;
		case 3:
			data_back &= ~(0x30);
			write_i2c_byte (rear_id, 0x2, data_back);
			break;
		case 4:
			data_back &= ~(0x20);
			data_back |= 0x10;
			write_i2c_byte (rear_id, 0x2, data_back);
			break;
		default:
			diag_printf ("Invalid selection\n");
		}
	} while (choice);

}


