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
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/hal_iop33x.h> // Hardware definitions
#include <cyg/hal/iq8033x.h>       // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf
#include <redboot.h>
#include <cyg/io/pci.h>

#include "test_menu.h"

int  global_verbose=1;

extern CYG_ADDRWORD hexIn(void);
extern CYG_ADDRWORD decIn(void);

extern int memTest (long startAddr, long endAddr);
extern int EnhMemTest (long startAddr, long endAddr, unsigned iter);
extern int LoopMemTest (long startAddr, long endAddr);
extern int RandData32 (long startAddr, long endAddr);
extern int multiPath (cyg_uint32 startAddr, cyg_uint32 endAddr);

void hdwr_diag (void);

#ifdef CYGPKG_IO_ETH_DRIVERS
extern void enet_setup (MENU_ARG arg);
#endif // CYGPKG_IO_ETH_DRIVERS

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
extern void pci_test (MENU_ARG arg);
extern void battery_status(MENU_ARG arg);
extern void battery_test_menu (MENU_ARG arg);
extern void timer_test (MENU_ARG arg);

/*Auto Diagnostic Menu Table*/
static AUTO_DIAG_LIST autodiag[] =
{
//    {battery_status},
    {timer_test},
    {memory_tests},
//    {enet_setup},
//    {seven_segment_display},
};

/* Test Menu Table */
static MENU_ITEM testMenu[] =
{
    {"Memory Tests", memory_tests, 0},
    {"Random-write Memory Tests", random_write_memory_tests, 0},
    {"Repeating Memory Tests", repeat_mem_test, 0},
    {"Repeat-On-Fail Memory Test", special_mem_test, 0},
    {"Enhanced Memory Tests", enhanced_memory_tests, 0},
    {"Multi-Path Memory Test", multipath_memory_test, 0},
    {"Rotary Switch S1 Test", rotary_switch, 0},
    {"7 Segment LED Tests", seven_segment_display, 0},
#ifdef CYGPKG_IO_ETH_DRIVERS
    {"i82545 Ethernet Configuration", enet_setup, 0},
#endif // CYGPKG_IO_ETH_DRIVERS
    {"Battery Status Test", battery_status, 0},
#ifdef CYGSEM_HAL_ARM_IQ8033X_BATTERY_TEST
    {"Battery Backup SDRAM Memory Test", battery_test_menu, 0},
#endif
    {"Timer Test", timer_test, 0},
    {"PCI Bus test", pci_test, 0},
    {"CPU Cache Loop (No return)", cache_loop, 0},
    {"Execute Selected Tests", exec_all, 0}
};

#define NUM_MENU_ITEMS	(sizeof (testMenu) / sizeof (testMenu[0]))
#define MENU_TITLE	"\n  IQ80331/IQ80332 CRB Hardware Tests"

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

    cyg_pci_init(); 

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

    int auto_index, menu_index;

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

    diag_printf("Base address of memory to test (in hex): ");
    start_addr = hexIn();

    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();

    diag_printf ("\nNumber of iterations (in decimal): ");
    iter = decIn();

    end_addr = start_addr + mem_size - 1;

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
	diag_printf("\nTesting memory from %p to %p.\n", start_addr, \
				(start_addr + (selected_space*1024*1024)-1));
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
        start_addr=(void*)workspace_start;
        end_addr=(void*)workspace_end;
    }

    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);
    memTest(start_addr, end_addr);
    diag_printf ("\nMemory test done.\n");
}


// ***************************************************************************
// repeat_mem_test - Repeating Memory Tests                       
//
static void
repeat_mem_test (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, mem_size, end_addr;
    char	cache_disable[10];

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

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_OFF;
    diag_printf("\n\nThe 7-Segment LSD LED shows the Rotary Switch position selected, i.e., 0-F.");
    diag_printf("\n\nSlowly dial the Rotary Switch through each position 0-F and confirm reading.");

    diag_printf( "\n\nStrike <CR> to exit this test." );
    while (_rb_gets(recv_data, sizeof(recv_data), 50) != _GETS_OK) {
	do {
	    for(index = 0; index <= MAX_SWITCH_SAMPLES; index++) {
		RotarySwitch[index] = *(volatile unsigned char *) IQ8033X_ROTARY_SWITCH_ADDR;
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




