//=============================================================================
//
//      xscale_test.c - Cyclone Diagnostics
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
/************************************************************************/


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
#include <cyg/hal/hal_iop34x.h> // Hardware definitions
#include <cyg/hal/iq8134x.h>       // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf
#include <redboot.h>
#include <cyg/io/pci.h>

#include "test_menu.h"

int  global_verbose=1;

extern CYG_ADDRWORD hexIn(void);

extern int memTest (long startAddr, long endAddr);
extern int EnhMemTest (long startAddr, long endAddr, unsigned iter);
extern int LoopMemTest (long startAddr, long endAddr);
extern int RandData32 (long startAddr, long endAddr);
extern CYG_ADDRWORD decIn(void);
extern int multiPath (cyg_uint32 startAddr, cyg_uint32 endAddr);
extern int nvram_test(CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr);
void hdwr_diag (void);

#ifdef CYGPKG_DEVS_ETH_INTEL_I82544 
extern void enet_setup (MENU_ARG arg);
#endif 
#ifdef CYGPKG_DEVS_ETH_SMSC_LAN91CXX
extern void smsc_setup (MENU_ARG arg);
#endif

static void memory_tests (MENU_ARG arg);
static void random_write_memory_tests (MENU_ARG arg);
static void repeat_mem_test (MENU_ARG arg);
static void special_mem_test (MENU_ARG arg);
static void enhanced_memory_tests (MENU_ARG arg);
static void multipath_memory_test (MENU_ARG arg);
static void rotary_switch (MENU_ARG arg);
static void seven_segment_display (MENU_ARG arg);
static void cache_loop (MENU_ARG arg);
static void exec_all(MENU_ARG arg);
static void dump_atus(MENU_ARG arg);
static void diag_dump_region(unsigned long addr, unsigned long size);
static void reset_pcix (MENU_ARG arg);
static void nvram_setup (MENU_ARG arg);

#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
static void cache_thrash_test (MENU_ARG arg);
static void toggle_cache (MENU_ARG arg);
static int dcache_thrash (void);
static int icache_thrash (void);
// Create function pointer for our SWI block
void (*fp1)(void);
#endif

extern void pci_test (MENU_ARG arg);
extern void battery_status(MENU_ARG arg);
extern void battery_test_menu (MENU_ARG arg);
extern void timer_test (MENU_ARG arg);
extern void configure_atux(int);
extern void l2_cache_control(cyg_bool t);
extern void cf_test(MENU_ARG arg);

/*Auto Diagnostic Menu Table*/
static AUTO_DIAG_LIST autodiag[] =
{
    {memory_tests},
};

/* Test Menu Table */
static MENU_ITEM testMenu[] =
{
    {"Memory Tests", memory_tests, 0},
    {"Random-write Memory Tests", random_write_memory_tests, 0},
    {"Repeating Memory Tests", repeat_mem_test, 0},
    {"Repeat-On-Fail Memory Test", special_mem_test, 0},
    {"Enhanced Memory Tests", enhanced_memory_tests, 0},
    {"Rotary Switch S1 Test", rotary_switch, 0},
    {"7 Segment LED Tests", seven_segment_display, 0},
#ifdef CYGPKG_DEVS_ETH_INTEL_I82544 
    {"i82545 Ethernet Configuration", enet_setup, 0},
#endif
#ifdef CYGPKG_DEVS_ETH_SMSC_LAN91CXX
    {"SMSC 91C111 EEPROM Configuration", smsc_setup, 0},
#endif
#ifdef CYGSEM_HAL_ARM_IQ8134X_BATTERY_TEST
    {"Battery Status Test", battery_status, 0},
    {"Battery Backup SDRAM Memory Test", battery_test_menu, 0},
#endif
    {"Timer Test", timer_test, 0},
    {"PCI Bus test", pci_test, 0},
    {"CPU Cache Loop (No return)", cache_loop, 0},
    {"Batch Memory Tests", exec_all, 0},
    {"Multi-Path Memory Test", multipath_memory_test, 0},
#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
/*    {"Cache Thrash Test", cache_thrash_test, 0}, */
    {"Toggle Cache Settings", toggle_cache, 0},
#endif
    {"NVRAM Test", nvram_setup, 0	},
    {"CompactFlash Test", cf_test, 0},

};

#define NUM_MENU_ITEMS	(sizeof (testMenu) / sizeof (testMenu[0]))
#define MENU_TITLE	"\n  IQ8134 CRB Hardware Tests"


void reset_pcix (MENU_ARG arg) {

		*ATUX_PCSR |= (PCSR_ATUX_CR_BUS_RESET);
		hal_delay_us(1000*1000);
		*ATUX_PCSR &= ~(PCSR_ATUX_CR_BUS_RESET);
		// Wait for Reset to be deasserted on PCI-X bus
		while (*ATUX_PCSR & PCSR_ATUX_CR_BUS_RESET)
			;
		hal_delay_us(3000*1000);
		// Reconfigure the bus after reset
		configure_atux(0);

}

void dump_atus(MENU_ARG arg) {
	cyg_uint32 *atue_p;
	cyg_uint32 *atux_p;
	cyg_uint32 enabled = 0;

	atue_p = (cyg_uint32*)(ATUE_BASE);
	atux_p = (cyg_uint32*)(ATUX_BASE);

	while (enabled == 0) {
		if (*ATUE_ATUCMD & ATUCMD_BUS_MASTER_ENABLE) {
			diag_printf ("WE ARE NOW ENABLED!!!\n");
			diag_printf ("WE ARE NOW ENABLED!!!\n");
			diag_printf ("WE ARE NOW ENABLED!!!\n");
			diag_printf ("WE ARE NOW ENABLED!!!\n");
			diag_printf ("WE ARE NOW ENABLED!!!\n");
			enabled = 1;
		}
		// Dump ATU-X Base Registers
		diag_printf ("\n\n------- ATUX Registers : %s -------\n", (enabled) ? "ATUE ENABLED" : "ATUE DISABLED");
		diag_dump_region((int)atux_p, 128);
		diag_printf ("------------------------------\n");

		diag_printf ("\n\n======= ATUE Registers : %s =======\n", (enabled) ? "ATUE ENABLED" : "ATUE DISABLED");
		diag_dump_region((int)atue_p, 128);
		diag_printf ("================================\n");

		HAL_DELAY_US(1000*1000);  // delay 1S
	}

}

void diag_dump_region(unsigned long addr, unsigned long size)
{
    unsigned long cur = addr;
    while (cur < addr + size) {
        diag_printf("0x%08lx: %08lx %08lx %08lx %08lx\n",
               cur,
               *(unsigned long *)(cur),      *(unsigned long *)(cur + 4),
               *(unsigned long *)(cur + 8),  *(unsigned long *)(cur + 12));
        cur += 16;
    }
    diag_printf("\n");
}

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
    diag_printf ("Entering Hardware Diagnostics - Disabling Data and L2 Cache!\n");

#ifndef CYGSEM_HAL_ARM_BOARD_YORK_CREEK
    cyg_pci_init();
#endif
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
	HAL_L2_CACHE_SYNC();
	HAL_L2_CACHE_DISABLE();

    menu (testMenu, NUM_MENU_ITEMS, MENU_TITLE, MENU_OPT_NONE);

    diag_printf ("Exiting Hardware Diagnostics!\n\n");
#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
    HAL_DCACHE_ENABLE();
#endif
	HAL_L2_CACHE_ENABLE();
}

static void
cache_loop (MENU_ARG arg)
{
    diag_printf ("Putting Processor in a Tight Loop Forever...\n\n");

    asm ( "0: mov r0,r0\n"
	  "b 0b\n");
}

// ***************************************************************************
// exec_all - Execute some pre-selected tests sequentially.
//
// This test executes some pre-selected tests sequentially with minimum user
// intervention. The verbose output is limited to a simple "PASS" and "FAIL"
// for each test.
//
static void exec_all(MENU_ARG arg)
{

    global_verbose=0;

#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
	HAL_DCACHE_ENABLE();
#endif
	HAL_L2_CACHE_ENABLE();

    unsigned int auto_index, menu_index;

    for(auto_index=0; auto_index < (sizeof(autodiag)/sizeof(AUTO_DIAG_LIST)); auto_index++)
    {
          for(menu_index=0; menu_index< (sizeof(testMenu)/sizeof(MENU_ITEM)); menu_index++)
          {

                if(autodiag[auto_index].actionRoutine == testMenu[menu_index].actionRoutine)
                {
                   (*testMenu[menu_index].actionRoutine) ((void *)menu_index);
                }
          }
    }

    global_verbose=1;
	HAL_DCACHE_SYNC();
	HAL_DCACHE_DISABLE();
	HAL_L2_CACHE_SYNC();
	HAL_L2_CACHE_DISABLE();
}

// ***************************************************************************
// random_write_memory_tests - Random Write Memory Tests
//
// This test writes pseudo-random values to memory looking for ECC failures
//
static void
random_write_memory_tests (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, end_addr;
    long mem_size;

    diag_printf("Base address of memory to test (in hex): ");
    start_addr = hexIn();

    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();

    end_addr = start_addr + mem_size - 1;

    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);
    RandData32(start_addr, end_addr);
    diag_printf ("\nMemory test done.\n");
}

// ***************************************************************************
// enhanced_memory_tests - Enhanced Memory Tests
//
// This test performs more intensive memory testing.
//
static void
enhanced_memory_tests (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, end_addr;
    long mem_size;
    unsigned iter;

	if (global_verbose == 1) {
    	diag_printf("Base address of memory to test (in hex): ");
    	start_addr = hexIn();

	    diag_printf ("\nSize of memory to test (in hex): ");
	    mem_size = hexIn();

	    diag_printf ("\nNumber of iterations (in decimal): ");
	    iter = decIn();

    	end_addr = start_addr + mem_size - 1;
	}
	else  {
		start_addr = (CYG_ADDRWORD)workspace_start;
        end_addr  =(CYG_ADDRWORD)workspace_end;
		iter = 1;
	}

    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);
    EnhMemTest(start_addr, end_addr, iter);
    diag_printf ("\nMemory test done.\n");
}

#define MP_DEBUG 0
// ***************************************************************************
// Multi-path memory test
//
// This test performs concurrent memcopy, DMA, and XOR execution
//
static void
multipath_memory_test (MENU_ARG arg)
{
    cyg_uint32 start_addr, end_addr;
	cyg_uint32 testing_space, testing_space_mb;

#if MP_DEBUG == 1
	diag_printf ("Workspace_Start is: 0x%08x\n", workspace_start);
	diag_printf ("Workspace_End   is: 0x%08x\n", workspace_end  );
#endif
	// Align start of RAM to 1M offset and create 1M for Descriptors
	start_addr = (cyg_uint32)(workspace_start + SZ_2M - 1);
	start_addr &= (cyg_uint32)(~(SZ_1M - 1));

	// Give 1M for stack space and then round-down workspace_end to 1M alignment
	end_addr   = (cyg_uint32)(workspace_end - SZ_1M);
	end_addr   &= (cyg_uint32)(PBLR_SZ_1M);

	if (start_addr > end_addr) {
		diag_printf ("Memory Problem!  Start_Addr: 0x%08x > End_Addr: 0x%08x\n",
					start_addr, end_addr);
		return;
	}
	testing_space = end_addr - start_addr;
	testing_space_mb = (testing_space/(1024*1024));

#if MP_DEBUG == 1
	// Hardcode a 4MB region for testing when debugging
	testing_space_mb = 4;
	diag_printf("\nTesting memory from %p to %p.\n", start_addr, \
				(start_addr + (testing_space_mb*1024*1024)-1));
#endif

	if (multiPath(start_addr, testing_space_mb) == 0)
		diag_printf ("\n     ++++ Passed MultiPath Test +++++\n");
	else
		diag_printf ("\n     ---- Failed MultiPath Test -----\n");

	CYGACC_CALL_IF_DELAY_US(1000*2000);  // Delay for 2s

}


// ***************************************************************************
// memory_tests - Basic Memory Tests
//
// Memory tests can be run one of two ways - with the cache turned OFF to test
// physical memory, or with cache turned ON to test the caching
//
static void
memory_tests (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, end_addr;
    long mem_size;

    if(global_verbose==1)
    {
        diag_printf("Base address of memory to test (in hex): ");
        start_addr = hexIn();

        diag_printf ("\nSize of memory to test (in hex): ");
        mem_size = hexIn();

        end_addr = start_addr + mem_size - 1;
    }
    else
    {
        start_addr=(CYG_ADDRWORD)workspace_start;
        end_addr  =(CYG_ADDRWORD)workspace_end;
    }

    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);
    memTest(start_addr, end_addr);
    diag_printf ("\nMemory test done.\n");
}


// ***************************************************************************
// NVRAM Test - Setup start and end address to test NVRAM
//
//
static void
nvram_setup (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, end_addr;
    long mem_size;

    char prodcode = *IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK;

	if (prodcode == IQ8134X_PROD_CODE_IQ8134xMC) {
		diag_printf ("Sorry - No NVRAM on Maple Creek!\n");
	}
	else   {
		diag_printf("Base address of memory to test (in hex): ");
		start_addr = hexIn();

		diag_printf ("\nSize of memory to test (in hex): ");
		mem_size = hexIn();

		end_addr = start_addr + mem_size - 1;

		diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);
		nvram_test(start_addr, end_addr);
		diag_printf ("\nMemory test done.\n");
	}
}



// ***************************************************************************
// repeat_mem_test - Repeating Memory Tests
//
static void
repeat_mem_test (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, mem_size, end_addr;
    char	cache_disable[10];
	cyg_uint32 count = 1;

    diag_printf ("Turn off Data Cache? (y/n): ");
    while (_rb_gets(cache_disable, sizeof(cache_disable), 0) != _GETS_OK)
	;

	if (cache_disable[0] == 'y')
		HAL_DCACHE_DISABLE();
#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
	else
		HAL_DCACHE_ENABLE();
#endif

    diag_printf ("\nBase address of memory to test (in hex): ");
    start_addr = hexIn();
    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();
    end_addr = start_addr + mem_size - 1;
    diag_printf("\nTesting memory from %p to %p", start_addr, end_addr);
    while (memTest (start_addr, end_addr)) {
		count++;
    }
	diag_printf ("Failed on count: %d\n", count);
}


// ****************************************************************************
// special_mem_test - Repeat-On-Fail Memory Test
//
// Memory tests can be run one of two ways - with the cache turned OFF to test
// physical memory, or with cache turned ON to test the caching
//
static void
special_mem_test (MENU_ARG arg)
{
    long	start_addr;
    long	mem_size;
    long	end_addr;

    diag_printf ("Base address of memory to test (in hex): ");
    start_addr = hexIn();
    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();

    end_addr = start_addr + mem_size - 1;
    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);

    LoopMemTest(start_addr, end_addr);

    diag_printf ("\n\nMemory test done.\n");
    diag_wait();
}

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

	diag_printf ("Testing 7 Segment Displays.\n");

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
    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_S;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_S;
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

    char prodcode = *IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK;

	if ((prodcode == IQ8134X_PROD_CODE_IQ8134xMC) || (prodcode == IQ8134X_PROD_CODE_IQ8134xMC_MODE2)) {
		diag_printf ("Sorry - No Rotary Switch on Maple Creek!\n");
		return;
	}

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_OFF;
    diag_printf("\n\nThe 7-Segment LSD LED shows the Rotary Switch position selected, i.e., 0-F.");
    diag_printf("\n\nSlowly dial the Rotary Switch through each position 0-F and confirm reading.");

    diag_printf( "\n\nStrike <CR> to exit this test." );
    while (_rb_gets(recv_data, sizeof(recv_data), 50) != _GETS_OK) {
	do {
	    for(index = 0; index <= MAX_SWITCH_SAMPLES; index++) {
		RotarySwitch[index] = *(volatile unsigned char *) IQ8134X_ROTARY_SWITCH_ADDR;
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
	SevSegDecode = led_data[RotarySwitch[0]];

	// display the rotary switch position on the 7 segment LSD LED as: 0, 1, 2, 3, etc.
	*(volatile unsigned char *)DISPLAY_RIGHT = SevSegDecode;
    }

} // end rotary_switch()

#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
// Either Turn all caches on or off
static void toggle_cache (MENU_ARG arg) {
	int dcache_on;

	HAL_DCACHE_IS_ENABLED(dcache_on);

	if (dcache_on) {
		HAL_DCACHE_SYNC();
		HAL_DCACHE_DISABLE();
		HAL_ICACHE_SYNC();
		HAL_ICACHE_DISABLE();
		HAL_L2_CACHE_SYNC();
		HAL_L2_CACHE_DISABLE();
		dcache_on = 0;
	}
	else  {
#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
		HAL_DCACHE_ENABLE();
		HAL_ICACHE_ENABLE();
#endif
		HAL_L2_CACHE_ENABLE();
		dcache_on = 1;
	}

	diag_printf ("All Caches are now: %s\n", (dcache_on) ? "On" : "Off");
}

#define NEW_SET (0x20/4)
#define NEW_WAY (0x2000/4)
#define LIMIT 100000

// ***************************************************************************
// Cache_Trash_Test - Lots of cache reads/writes
//
static void
cache_thrash_test (MENU_ARG arg)
{
	diag_printf ("Enabling Data and Instruction Cache\n");
#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
	HAL_DCACHE_ENABLE();
	HAL_ICACHE_ENABLE();
#endif
	HAL_L2_CACHE_ENABLE();

 	_Write_FSR(0);
	_Write_FAR(0);

	if (dcache_thrash() == 0) {
		diag_printf ("\n\n    +++++  PASSED DCACHE THRASH TEST +++++\n\n");
	}
	else  {
		diag_printf ("\n\n    -----  FAILED DCACHE THRASH TEST -----\n\n");
		// Hang if batch mem test
		if (global_verbose == 0) {
			while (1)
				;
		}
		else
			return;
	}

	if (icache_thrash() == 0) {
		diag_printf ("\n\n    +++++  PASSED ICACHE THRASH TEST +++++\n\n");
	}
	else  {
		diag_printf ("\n\n    -----  FAILED ICACHE THRASH TEST -----\n\n");
		// Hang if batch mem test
		if (global_verbose == 0) {
			while (1)
				;
		}
		else
			return;
	}
	// If we're here - We passed both Cache Thrash Tests
	diag_printf ("\n\n    +++++  PASSED CACHE THRASH TESTS +++++\n\n");

}


// Thrash the Datacache by doing lots of writes/reads
// to the dcache with different data to change the parity
int dcache_thrash(void) {
	CYG_ADDRWORD start_addr;
	unsigned int data[4];
	volatile unsigned int data2;
	int i, j, loop, big_loop, count = 0;
	volatile unsigned int *address;
	volatile unsigned int *test_address = address;

	// Start on a 32k boundary
	start_addr = (CYG_ADDRWORD)(workspace_start + SZ_64K);
	start_addr &= ~(SZ_32K-1);
	address = (volatile unsigned int *)start_addr;

	diag_printf ("Start Address is: 0x%08x\n", start_addr);
	diag_printf ("Trying to thrash the Data cache.....\n");

	data[0] = 0;

	// Big Loop 0 = Count Data; Big Loop 1 = Address as Data;
	// Big Loop 2 = Targetted Parity
	for (big_loop = 0; big_loop < 3; big_loop++ ) {
		if (big_loop == 1) {
			diag_printf ("\nDone with Incrementing Data\n");
			diag_printf ("\nUsing Address as Data.....\n");
			data[0] = (int)start_addr;
		}
		else if (big_loop == 2) {
			diag_printf ("\nDone with Address as Data\n");
			diag_printf ("\nUsing Targetted Parity Data....\n");
		}
		else
			diag_printf ("\nUsing Incrementing Data....\n");

		count = 0;
		// LIMIT Loops to check
		while (count < LIMIT) {
			if (big_loop == 1){
				data[1] = data[0] + 4;
				data[2] = data[0] + 8;
				data[3] = data[0] + 0xc;
			}
			else  if (big_loop == 2) {
				switch (count) {
					case 0:
						data[0] = data[1] = data[2] = data[3] = 0x3;
						break;
					case 1:
						data[0] = data[1] = data[2] = 0x3;
						data[3] = 0x2;
						break;
					case 2:
						data[0] = 0x3;
						data[1] = data[2] = data[3] = 0x2;
						break;
					case 3:
						data[0] = 0x2;
						data[1] = data[2] = data[3] = 0x3;
						break;
					case 4:
						data[0] = data[1] = data[2] = 0x2;
						data[3] = 0x3;
						break;
					case 5:
						data[0] = data[1] = data[2] = data[3] = 0x2;
						break;
					default:
						data[0] = data[1] = data[2] = data[3] = 0x2;
						diag_printf ("Count is %d, All targetted parity done...\n", count);
						count = LIMIT+1;
				}
			}
			else {
				data[1] = data[0] + 1;
				data[2] = data[0] + 2;
				data[3] = data[0] + 3;
			}

			// As addresses increment, don't go outside the DIMM Limit
			if ((int)address > (int)(workspace_end - SZ_1M)) {
				//diag_printf ("Address is getting too big: 0x%08x\n", (int)address);
				test_address = address = (volatile unsigned int*)start_addr;
			}

			// Status printout every 1000 loops
			if ((count % 1000) == 0) {
				diag_printf ("Loop : %6d out of: %6d\r", count, LIMIT);
/*				diag_printf ("Addresses coming up: 0x%08x 0x%08x 0x%08x 0x%08x\n", \
						(int)(address), (int)(address+1), (int)(address+2),\
						(int)(address+3));
				diag_printf ("Storing Data: 0x%08x 0x%08x 0x%08x 0x%08x\n", \
					data[0], data[1], data[2], data[3]);
*/
			}

			// PLD the cachelines - Load each set for each way
			for (i = 0, test_address = address; \
				i < 4; i++) {
				for (j = 0; j < 256; j++, test_address+= NEW_SET) {
					asm volatile ("pld [%0]" : : "r" (test_address));
				}
			}

			// Cover each word in each cacheline
			for (loop = 0; loop < 8; loop++) {
				// Write data into each set for each way
				for (i = 0, test_address = address + loop;\
				    	i < 4; i++) {
					for (j = 0; j < 256; j++, test_address+= NEW_SET) {
						//diag_printf ("Writing 0x%08x to address: 0x%08x ...\n", data[i], (int)test_address);
						*test_address = data[i];
					}
				}
				// Read  data from each set for each way
				for (i = 0, test_address = address + loop; \
					i < 4; i++) {
					for (j = 0; j < 256; j++, test_address+= NEW_SET) {
						data2 = *test_address;
						//diag_printf ("Comparing Data at address: 0x%08x\n", (int)test_address);
						if (data2 != data[i]) {
							int fsr, falr;
							diag_printf ("Failed to compare at: 0x%08x\n", (int)(test_address));
							diag_printf ("Expected: 0x%08x, Found: 0x%08x\n", data[i], data2);
							fsr = _Read_FSR;
							falr = _Read_FAR;
							diag_printf ("FSR: 0x%08x, FAR: 0x%08x\n", fsr, falr);
							return (-1);
						}
					}
				}
			} // For 0-8 loop

			// Increment by 32k
			address += (8*1024);

			if (big_loop == 1)
				data[0] = (int)(address);
			else
				data[0]++;
			count++;
		}
	} // Big_loop
	return (0);  // Pass
}

#define SWI_ADDRESS           0x28
#define SWI_IMMED_MASK        0xFFFFFF
#define SWI_INSTRUCTION_BASE  0xEF000000

int g_swi_count;
int g_pass_fail;

// Temporary SWI Handler used for ICache Thrash
void my_swi_handler(void) {
	volatile unsigned int* my_lr, came_from;

	asm volatile ("mov %0, lr" : "=r" (my_lr));

	//diag_printf ("Entered SWI and swi count is: %d\n", g_swi_count);
	// Just starting if swi_count == -1
	if (g_swi_count == -1) {
		g_swi_count = 0;
	}
	else
		g_swi_count++;

	//diag_printf ("LR and value is 0x%08x : 0x%08x\n", (int)my_lr, *my_lr);
	came_from = (*(my_lr - 1)) & SWI_IMMED_MASK;
	if (came_from != g_swi_count)  {
		if (g_pass_fail == 0) {
			diag_printf ("\n\nERROR: LR and SWI_COUNT don't match!\n");
			diag_printf ("SWI_COUNT: 0x%08x, CALLED_SWI: 0x%08x, Address: 0x%08x\n", \
				g_swi_count, came_from, (int)my_lr);
			g_pass_fail = 1;
		}
	}
	else  {
		//diag_printf ("SWI COUNT AND LR MATCH!\n");
		if (g_swi_count == 8191) {
			//diag_printf ("At the final count for this 32k block\n");
			g_swi_count = -1;
		}
	}

}

// Thrash the Instruction Cache by doing SWI instructions
// throughout the available mem space
int icache_thrash(void) {
	CYG_ADDRWORD start_addr;
	int i, big_loop, count = 0;
	volatile unsigned int *address, orig_swi_handler;
	volatile unsigned int *test_address = address;

	// Init the global swi_count to 0xFFFF.FFFF
	g_swi_count = -1;
    g_pass_fail = 0;

	// Start on a 32k boundary
	start_addr = (CYG_ADDRWORD)(workspace_start + SZ_64K);
	start_addr &= ~(SZ_32K-1);
	address = (volatile unsigned int*)start_addr;

	diag_printf ("Start Address is: 0x%08x\n", start_addr);
	diag_printf ("Trying to thrash the Instruction cache.....\n");

	// Get orig_swi_handler offset so we can take over SWIs
	orig_swi_handler = *(volatile unsigned int*)SWI_ADDRESS;
	*(volatile unsigned int*)SWI_ADDRESS = (int)&my_swi_handler;

	// Big Loop to iterate through all memory patterns
	for (big_loop = 0; big_loop < 1; big_loop++ ) {
		count = 0;
		// LIMIT Loops to check
		while (count < LIMIT) {

			// As addresses increment, don't go outside the DIMM Limit
			if ((int)address > (int)(workspace_end - SZ_1M)) {
				//diag_printf ("Address is getting too big: 0x%08x\n", (int)address);
				test_address = address = (volatile unsigned int*)start_addr;
			}
			else
				test_address = address;

			// Status printout every 1000 runs - disabled to try and speed up test
			if ((count % 1000) == 0) {
				diag_printf ("Loop : %d out of: %d\r", count, LIMIT);
/*				diag_printf ("Addresses coming up: 0x%08x 0x%08x 0x%08x 0x%08x\n", \
						(int)(address), (int)(address+1), (int)(address+2),\
						(int)(address+3)); */
			}

			// Function pointer points to our SWI Block - cast the address as our fcn_pointer
			fp1 = (void (*)(void))test_address;

			// 8k * 4 bytes = 32k of ICache to load
			for (i = 0; i < 8192; i++) {
				*test_address++ = (SWI_INSTRUCTION_BASE | i);
			}

			//LDMFD SP!, {r0, lr} instruction takes place after the last SWI
			//so we can return to the instruction after the function ptr call
			*test_address++ = 0xE8BD4001;  // LDMFD SP!, {r0, lr}
			*test_address++ = 0xE1A0F000;  // MOV   PC, R0

			HAL_DCACHE_FLUSH(address, 33*1024);

			// Push our R0, LR and PC+0xC onto Stack
			asm volatile ("stmfd sp!, { r0 }\n"
						  "mov	r0, pc\n"
						  "add  r0, r0, #0x10\n"
						  "stmfd  sp!, { r0, lr }\n"
			              : : : "r0" );

			// Jump to SWIs
			(*fp1)();

			// Will return here once done - need to get R0 off stack
			asm volatile ("ldmfd sp!, {r0}");

			// Check if we passed or failed
			if (g_pass_fail == 1) {
				int fsr, falr;
				fsr = _Read_FSR;
				falr = _Read_FAR;
				diag_printf ("\n\nFAILED SWI COMPARE at count: %d, address block: 0x%08x\n", \
						count, (int)address);
				diag_printf ("FSR: 0x%08x, FAR: 0x%08x\n", fsr, falr);
				diag_printf (" Time to Bail! \n");

				// Restore RedBoot SWI Handler
				*(volatile unsigned int*)SWI_ADDRESS = orig_swi_handler;
				return (-1);
			}
			// Increment by 32k
			address += (8*1024);
			count++;
		}
	} // Big_loop
	diag_printf ("\nDone with Loop\n");

	// Restore RedBoot SWI Handler
	*(volatile unsigned int*)SWI_ADDRESS = orig_swi_handler;

	return (0);  // Pass
}

#endif // L1_DISABLE
