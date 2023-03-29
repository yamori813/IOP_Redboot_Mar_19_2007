//=============================================================================
//
//      memtest.c
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

/*************************************************************************
* Memtest.c - this file performs an address/address bar memory test.
*
*  Modification History
*  --------------------
*  01sep00 ejb Ported to StrongARM2
*  18dec00 snc
*  02feb01 jwf for snc
*  25jan02 Rewritten for RedBoot by Mark Salter
*/

#include <redboot.h>
#include CYGBLD_HAL_VAR_H

extern int global_verbose;
extern void hal_diag_led(int n);

#define LED_DIAG_PROGRESS(address)   { \
    if (((address) & 0xff) == 0)       \
        led_diag_progress(address);    \
}
static cyg_uint32 diag_time = 0;

// Defines for Multipath Testing
#define DEFAULT_AAU_SOURCES 4
#define NUM_AAU_SOURCES 32
#define CONVERT_TO_MB(in) (in*1024*1024)
#define MP_DEBUG 0
#define DMA0_RUNNING_FAIL 0x00000001
#define DMA1_RUNNING_FAIL 0x00000002
#define DMA0_COMPARE_FAIL 0x00000004
#define DMA1_COMPARE_FAIL 0x00000008
#define MEMCOPY_CRC_FAIL  0x00000010
#define AAU_RUNNING_FAIL  0x00000020
#define AAU_VERIFY_FAIL   0x00000040
#define ECC_ERROR         0x00000080
#define ATU_CHANGED		  0x00000100

typedef struct {
		cyg_uint32 mem_slice;
		cyg_uint32 start;
		cyg_uint32 descriptor_start;
		cyg_uint32 num_dma_engines;
		cyg_uint32 aau_sources;
		cyg_uint32 aau_source_slice;
		cyg_uint32 dma_start_address[NUM_DMA_CHANNELS];
		cyg_uint32 aau_start_address;
		cyg_uint32 memcopy_start_address;
		cyg_uint32 total_mem_mb;
		cyg_uint32 dma_pattern[NUM_DMA_CHANNELS];
		cyg_uint32 memcopy_pattern;
		cyg_uint32 aau_desc_start;
		cyg_uint32 dma_desc_start[NUM_DMA_CHANNELS];
		cyg_uint32 num_loops;
		cyg_bool   atu_enabled;
		cyg_bool   memcopy_enabled;
}mp_setup_t;

struct dma_descriptor  {
	cyg_uint32 nda;
	cyg_uint32 pad;
	cyg_uint32 puad;
	cyg_uint32 lad;
	cyg_uint32 bc;
	cyg_uint32 dc;
};

struct aau_descriptor  {
	cyg_uint32 nda;
	cyg_uint32 sar1;
	cyg_uint32 sar2;
	cyg_uint32 sar3;
	cyg_uint32 sar4;
	cyg_uint32 dar;
	cyg_uint32 bc;
	cyg_uint32 dc;
	cyg_uint32 sar5;
	cyg_uint32 sar6;
	cyg_uint32 sar7;
	cyg_uint32 sar8;
	cyg_uint32 edc0;
	cyg_uint32 sar9;
	cyg_uint32 sar10;
	cyg_uint32 sar11;
	cyg_uint32 sar12;
	cyg_uint32 sar13;
	cyg_uint32 sar14;
	cyg_uint32 sar15;
	cyg_uint32 sar16;
	cyg_uint32 edc1;
	cyg_uint32 sar17;
	cyg_uint32 sar18;
	cyg_uint32 sar19;
	cyg_uint32 sar20;
	cyg_uint32 sar21;
	cyg_uint32 sar22;
	cyg_uint32 sar23;
	cyg_uint32 sar24;
	cyg_uint32 edc2;
	cyg_uint32 sar25;
	cyg_uint32 sar26;
	cyg_uint32 sar27;
	cyg_uint32 sar28;
	cyg_uint32 sar29;
	cyg_uint32 sar30;
	cyg_uint32 sar31;
	cyg_uint32 sar32;
};

static int  print_mp_options(mp_setup_t *mp_options);
static void divy_up_space(mp_setup_t *mp_options);
static int  run_memcopy(mp_setup_t *mp_options);
static int  init_dma_engine(cyg_uint32 channel, mp_setup_t *mp_options);
static int  init_aau_engine(mp_setup_t *mp_options);
static void setup_aau_descriptor (struct aau_descriptor *aau_desc, mp_setup_t *mp_options, \
                                  cyg_uint32 aau_size, cyg_uint32 local_start);
static int  setup_local_memory(mp_setup_t *mp_options);
static void my_memset(cyg_uint32* start, cyg_uint32 data, cyg_uint32 byte_count);
static cyg_int32 check_for_done(mp_setup_t *mp_options);
static int mem_compare(cyg_uint32 *base, cyg_uint32 value, cyg_uint32 byte_count);

extern CYG_ADDRWORD decIn(void);
extern cyg_uint32 cyg_posix_crc32(unsigned char *s, int len);

void
led_diag_progress(cyg_uint32 address)
{
    cyg_uint32 val;

    TMR1_READ(val);
    if (val == 0)
    {
        diag_time++;

        //
        // Toggle the period on the LED Display
        //
        hal_diag_led(0x10);

        //
        // Resetup the timer for a 1/2 second timer
        //

        // disable timer
        TMR1_WRITE(0);

        // clear interrupts
        TISR_WRITE(0x2);

        // set reload/count valdue
        // Based on 266MHz clock for a 1/2 second timer:
        TCR1_WRITE(133000000);

        // let it run
        TMR1_WRITE(TMR_TIMER_ENABLE | TMR_CLK_1);

        TMR1_READ(val);
    }
}

#define FAILED          1
#define PASSED          0

// Do walking one's test
//
static int
onesTest(CYG_ADDRWORD *testAddr)
{
    CYG_ADDRWORD theOne, dataRead;
    int	fail, loopCount = 0; // To keep track of when to print CR
    // Loop for all bits in an address
    for (theOne = 1, fail = 0; theOne && !fail; theOne <<= 1) {

	testAddr[0] = theOne;     // Write test data
	testAddr[1] = ~0L;        // Drive d0-dn hi
	dataRead = *testAddr;     // Read back data

	if (global_verbose == 1)
	    diag_printf("%08x%s", dataRead, (++loopCount % 8) ? "" : "\n");

	if (dataRead != theOne)  // Verify data
	    return FAILED;       // Signal failure
    }
    return PASSED;
}


// Do 32-bit word address test
//
static int
Addr32 (cyg_uint32 *start, cyg_uint32 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint32 *currentAddr; /* Current address being tested */
    cyg_uint32 data;

    for(currentAddr = start; currentAddr <= end; currentAddr++) {
		*currentAddr = (cyg_uint32)(CYG_ADDRWORD)currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
    }

    for (currentAddr = start; currentAddr <= end; currentAddr++) {
	data = *currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
	if (data != (cyg_uint32)(CYG_ADDRWORD)currentAddr) {
	    if (global_verbose == 0)
		diag_printf ("\n\nBad Read, Address = %p, Data Read = 0x%08x\n\n",
			      currentAddr, data);
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    }
    return PASSED;
}


// Do inverse long word address test
//
static int
Bar32 (cyg_uint32 *start, cyg_uint32 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint32 *currentAddr, data;

    for(currentAddr = start; currentAddr <= end; currentAddr++) {
	*currentAddr = ~(CYG_ADDRWORD)currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
    }

    for (currentAddr = start; currentAddr <= end; currentAddr++) {
	data = *currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
	if (data != ((~(CYG_ADDRWORD)currentAddr) & 0xffffffff)) {
	    if (global_verbose == 0)
		diag_printf ("\n\nBad Read, Address = %p, Data Read = 0x%08x\n\n",
			     currentAddr, data);
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    }
    return PASSED;
}

// Do byte address test
//
static int
Addr8 (cyg_uint8 *start, cyg_uint8 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint8 *currentAddr; // Current address being tested

    for (currentAddr = start; currentAddr <= end; currentAddr++) {
	*currentAddr = (cyg_uint8)(CYG_ADDRWORD)currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
    }

    for (currentAddr = start; currentAddr <= end; currentAddr++) {
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
	if (*currentAddr != (cyg_uint8)(CYG_ADDRWORD)currentAddr) {
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    }
    return PASSED;
}

// Do inverse byte address test
//
static int
Bar8 (cyg_uint8 *start, cyg_uint8 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint8 *currentAddr;  // Current address being tested

    for(currentAddr = start; currentAddr <= end; currentAddr++) {
	*currentAddr = ~(CYG_ADDRWORD)currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
    }

    for(currentAddr = start; currentAddr <= end; currentAddr++) {
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
	if (*currentAddr != (~(CYG_ADDRWORD)currentAddr & 0xff)) {
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    }
    return PASSED;
}

// This routine is called if one of the memory tests fails.  It dumps
// the 8 32-bit words before and the 8 after the failure address
//
void
dumpMem (CYG_ADDRWORD badAddr)
{
    cyg_uint32 *addr;
    cyg_uint16 *saddr;
    int i;

    // Print out first line of mem dump
    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr - 32,
		*(cyg_uint32 *)(badAddr - 32),
		*(cyg_uint32 *)(badAddr - 28),
		*(cyg_uint32 *)(badAddr - 24),
		*(cyg_uint32 *)(badAddr - 20));

    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr - 16,
		*(cyg_uint32 *)(badAddr - 16),
		*(cyg_uint32 *)(badAddr - 12),
		*(cyg_uint32 *)(badAddr - 8),
		*(cyg_uint32 *)(badAddr - 4));

    // Print out contents of fault addr
    diag_printf("\n%p: %08x",
		(char *)badAddr, *(cyg_uint32 *)badAddr);

    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr + 4,
		*(cyg_uint32 *)(badAddr + 4),
		*(cyg_uint32 *)(badAddr + 8),
		*(cyg_uint32 *)(badAddr + 12),
		*(cyg_uint32 *)(badAddr + 16));

    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr + 20,
		*(cyg_uint32 *)(badAddr + 20),
		*(cyg_uint32 *)(badAddr + 24),
		*(cyg_uint32 *)(badAddr + 28),
		*(cyg_uint32 *)(badAddr + 32));

    /* DEBUG */
    diag_printf ("\n\nReading back data in 32bit chunks:\n");
    for (addr = (cyg_uint32 *)(badAddr - 16), i = 0; i <= 8; i++, addr++)
	diag_printf ("Address = %p, Data = 0x%08x\n", addr, *addr);
    diag_printf ("\n");

    diag_printf ("Reading back data in 16bit chunks:\n");
    for (saddr = (cyg_uint16 *)(badAddr - 16), i = 0; i <= 16; i++, saddr++)
	diag_printf ("Address = %p, Data = 0x%08x\n", saddr, *saddr);
    diag_printf ("\n");
}

// Returns 1 if passed, 0 if failed.
//
int
memTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr)
{
    CYG_ADDRWORD badAddr;  // Addr test failed at

    diag_time = 0;

    diag_printf("\nWalking 1's test: ");
    if (onesTest((CYG_ADDRWORD *)startAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n32-bit address test: ");
    if (Addr32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n32-bit address bar test: ");
    if (Bar32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n8-bit address test: ");
    if (Addr8((cyg_uint8 *)startAddr, (cyg_uint8 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\nByte address bar test: ");
    if (Bar8((cyg_uint8 *)startAddr, (cyg_uint8 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n\nTotal Execution Time: %d s\n", diag_time / 2);
    return 1;

 failed:
    diag_printf("failed");
    if (global_verbose == 0)
    {
	dumpMem(badAddr);
	diag_printf("\n\nTotal Execution Time: %d s\n", diag_time / 2);
    }
    return 0;
}


/* 02/02/01 jwf */
/* Do alternating inverse long word address test */
static int
ABar32(cyg_uint32 *start,		/* Starting address of test */
       cyg_uint32 *end,		/* Ending address */
       CYG_ADDRWORD *badAddr)		/* Failure address */
{
    register cyg_uint32 *currentAddr;	/* Current address being tested */
    int fail = 0;		/* Test hasn't failed yet */
    cyg_uint32 data;

    /* In this test, the contents of each longword address toggles
       between the Address and the Address BAR */
    for(currentAddr = start; currentAddr <= end; currentAddr++) {
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);

	if ((CYG_ADDRWORD)currentAddr & 4) /* Address ending in 0x4 or 0xc */
	    *currentAddr = ~(cyg_uint32)(CYG_ADDRWORD)currentAddr;

	else /* Address ending in 0x0 or 0x8 */
	    *currentAddr = (cyg_uint32)(CYG_ADDRWORD)currentAddr;
    }

    for (currentAddr = start; currentAddr <= end && !fail; currentAddr++) {
	data = *currentAddr;
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);

	switch ((CYG_ADDRWORD)currentAddr & 0xf) {
	  case 0x0:
	  case 0x8:
	    if (data != (cyg_uint32)(CYG_ADDRWORD)currentAddr) {
		fail = 1;
		diag_printf ("\nFailed at Address %p, Expected 0x%08X, Read 0x%08X\n",
			     currentAddr, (cyg_uint32)(CYG_ADDRWORD)currentAddr, data);
	    }
	    break;

	  case 0x4:
	  case 0xc:
	    if (data != ~(cyg_uint32)(CYG_ADDRWORD)currentAddr) {
		fail = 1;
		diag_printf ("\nFailed at Address %p, Expected 0x%08X, Read 0x%08X\n",
			     currentAddr, ~(cyg_uint32)(CYG_ADDRWORD)currentAddr, data);
	    }
	    break;

	  default:
	    fail = 1;
	    diag_printf ("\nFailed at Address %p, Unaligned address\n", currentAddr);
	    break;
	}
    }

    if (fail) {
	*badAddr = (CYG_ADDRWORD)(--currentAddr);
	return FAILED;
    } else
	return PASSED;
}


/* 02/02/01 jwf */
/*
 * Returns 1 if passed, 0 if failed.
 */
int
LoopMemTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr)
{
    CYG_ADDRWORD badAddr;  /* Addr test failed at */
    volatile int junk;

    while (1) {
	diag_printf("\n");

	diag_printf("\n32-bit address test: ");
	if (Addr32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	    break;
	diag_printf("passed");

	diag_printf("\n32-bit address bar test: ");
	if (Bar32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	    break;
	diag_printf("passed");

	diag_printf("\nAlternating Long word, Long word address bar test: ");
	if (ABar32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	    break;
	diag_printf("passed");
    }

    diag_printf("failed at Address %p\n", badAddr);
    diag_printf("Performing Continuous Write/Read/!Write/Read...\n\n");
    while (1) {
	*(volatile int *)badAddr = badAddr;
	junk = *(volatile int *)badAddr;
	*(volatile int *)badAddr = ~badAddr;
	junk = *(volatile int *)badAddr;
    }

    return 1;	// not reached
}


static unsigned int xscale_test_rand_seed;
static void
xscale_test_srand(unsigned int seed)
{
    xscale_test_rand_seed = seed;
}
static int
xscale_test_rand( void )
{
    unsigned int *seed = &xscale_test_rand_seed;
    int retval;

    // This is the code supplied in Knuth Vol 2 section 3.6 p.185 bottom
#define XSCALE_TEST_RAND_MAX  2147483647
#define MM 2147483647    // a Mersenne prime
#define AA 48271         // this does well in the spectral test
#define QQ 44488         // (long)(MM/AA)
#define RR 3399          // MM % AA; it is important that RR<QQ
    *seed = AA*(*seed % QQ) - RR*(unsigned int)(*seed/QQ);
    if (*seed < 0)
        *seed += MM;
    retval = (int)( *seed & XSCALE_TEST_RAND_MAX );
    return retval;
}

// Do 32-bit random data word address test
//
int
RandData32 (cyg_uint32 *start, cyg_uint32 *end)
{
    cyg_uint32 *currentAddr; /* Current address being tested */
    cyg_uint32 tcr0;

    diag_time = 0;

    TCR0_READ(tcr0);
    xscale_test_srand(tcr0);
    for (currentAddr = start; currentAddr <= end; currentAddr++)
    {
        *currentAddr = xscale_test_rand();
        LED_DIAG_PROGRESS((cyg_uint32)currentAddr);
    }
    diag_printf ("*MCU_ELOG0 = 0x%08lx\n", *MCU_ELOG0);
    diag_printf ("*MCU_ELOG1 = 0x%08lx\n", *MCU_ELOG1);
    diag_printf ("*MCU_ECAR0 = 0x%08lx\n", *MCU_ECAR0);
    diag_printf ("*MCU_ECAR1 = 0x%08lx\n", *MCU_ECAR1);
    diag_printf("\n\nTotal Execution Time: %d s\n", diag_time / 2);
    return PASSED;
}

//
// Dump SDRAM Controller Regs
//
void dump_sdram_regs(void) {
    diag_printf("*MCU_SDBR   = 0x%08lx\n", *MCU_SDBR);
    diag_printf("*MCU_SDCR0  = 0x%08lx\n", *MCU_SDCR0);
    diag_printf("*MCU_SDCR1  = 0x%08lx\n", *MCU_SDCR1);
    diag_printf("*MCU_SBR0   = 0x%08lx\n", *MCU_SBR0);
    diag_printf("*MCU_SBR1   = 0x%08lx\n", *MCU_SBR1);
    diag_printf("*MCU_S32SR  = 0x%08lx\n", *MCU_S32SR);
    diag_printf("*MCU_RCVDLY = 0x%08lx\n", *MCU_RCVDLY);
}

//
// Enhanced Memory Diag.
// Patterns
//
unsigned int data_patterns[] = {
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
	 0x55aa55aa,
	 0xaa55aa55,
	 0x5a5a5a5a,
	 0xa5a5a5a5,
	 0xcccc3333,
	 0x3333cccc,
	 0x33cc33cc,
	 0xcc33cc33,
	 0x3c3c3c3c,
	 0xc3c3c3c3};

static int do_enh_pattern_tests(cyg_uint32 *start, cyg_uint32 *end) {
    register cyg_uint32 *p_memory;
    cyg_uint32 data;
    unsigned int i, fails, failed = 0;

    diag_printf ("Starting Pattern Tests\n");
    for (i = 0; i < ((sizeof(data_patterns))/(sizeof(data_patterns[0]))); i++) {
        diag_printf ("Pattern: 0x%08lx", data_patterns[i]);
        diag_printf ("     fill");
        p_memory = start;
        while (p_memory < end) {
            LED_DIAG_PROGRESS((cyg_uint32)p_memory);
            *p_memory++ = data_patterns[i];
        }
        diag_printf ("     verify");
        p_memory = start;
        fails = 0;
        while (p_memory < end) {
            LED_DIAG_PROGRESS((cyg_uint32)p_memory);
            data = *p_memory;
            if (data != data_patterns[i]) {
                diag_printf ("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n",
                             p_memory, data_patterns[i], data);

                dump_sdram_regs();
                diag_printf("Attemping second read of same address\n");
                data = *p_memory;
                if (data != data_patterns[i]) {
                    diag_printf ("Second read of same address Failed: ");
                    diag_printf ("Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n",
                                 p_memory, data_patterns[i], data);
                } else {
                    diag_printf("Second read of same address PASSES\n");
                }

                fails++;
                failed = 1;
                if (fails > 10) {
                    diag_printf ("Exceeded 10 Fails - Aborting\n");
                    break;
                }
            }
            p_memory++;
        }
        diag_printf ("     done\n");
    }
    return failed;
}

//
// Enhanced Memory Diag.
// Writing the address to the location.
//
int do_enh_address_test(cyg_uint32 *start, cyg_uint32 *end) {
    register cyg_uint32 *p_memory;
    cyg_uint32 data;
    int fails, failed = 0;

    p_memory = start;
    diag_printf("Starting Address Test\n");

    while (p_memory < end) {
        LED_DIAG_PROGRESS((cyg_uint32)p_memory);
        *p_memory = (unsigned int)p_memory;
        p_memory++;
    }

    diag_printf("Verifying Addresses\n");
    p_memory = start;
    fails = 0;
    while (p_memory < end) {
        LED_DIAG_PROGRESS((cyg_uint32)p_memory);
        data = *p_memory;
        if (data != (unsigned int)p_memory) {
            diag_printf("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n",
                        p_memory, p_memory, data);

            dump_sdram_regs();
            diag_printf("Attemping second read of same address\n");
            data = *p_memory;
            if (data != (unsigned int)p_memory) {
                diag_printf("Second read of same address failed: ");
                diag_printf("Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n",
                            p_memory, p_memory, data);
            } else {
                diag_printf("Second read of same address PASSES\n");
            }

            fails++;
            failed = 1;
            if (fails > 10) {
                diag_printf("Exceeded 10 Fails - Aborting\n");
                break;
            }
        }
        p_memory++;
    }

    /* Now go from top of memory down to bottom */
    diag_printf ("Going Top to Bottom\n");
    p_memory = (cyg_uint32*)((cyg_uint32)end &~ 0x3);
    while (p_memory >= start) {
        LED_DIAG_PROGRESS((cyg_uint32)p_memory);
        *p_memory = (unsigned int)p_memory;
        p_memory--;
    }

    diag_printf ("Verifying Addresses\n");
    p_memory = (cyg_uint32*)((cyg_uint32)end &~ 0x3);
    fails = 0;
    while (p_memory >= start) {
        data = *p_memory;
        if (data != (unsigned int)p_memory) {
            diag_printf("Memory Failed: Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n",
                        p_memory, p_memory, data);

            dump_sdram_regs();
            diag_printf("Attemping second read of same address\n");
            data = *p_memory;
            if (data != (unsigned int)p_memory) {
                diag_printf("Second read of same address failed: ");
                diag_printf("Address: 0x%08x, Expected: 0x%08x Found: 0x%08x\n",
                            p_memory, p_memory, data);
            } else {
                diag_printf("Second read of same address PASSES\n");
            }

            fails++;
            failed = 1;
            if (fails > 10) {
                diag_printf("Exceeded 10 Fails - Aborting\n");
                break;
            }
        }
        p_memory--;
    }

    return failed;
}

// Do enhanced memory diagnostics
//
#include <cyg/hal/hal_cache.h>
int
EnhMemTest (cyg_uint32 *start, cyg_uint32 *end, unsigned iter)
{
    unsigned int i;
    int failed_patterns = 0;
    int failed_address = 0;
    int failed_patterns_unc = 0;
    int failed_address_unc = 0;

    diag_time = 0;

    for (i = 1; i <= iter; i++) {   // Number of iterations to run
        diag_printf ("Starting Iteration: %d\n", i);
        /* Enable the Data Cache */
        diag_printf ("Enabling Data Cache\n");
        HAL_DCACHE_ENABLE();

        /* Call the Pattern Tests */
        if (do_enh_pattern_tests(start, end)) {
            diag_printf ("Failed Pattern Test!\n");
            failed_patterns++;
        }

        /* Call the Address Test */
        if (do_enh_address_test(start, end)) {
            diag_printf ("Failed Address Test!\n");
            failed_address++;
        }

        /* Now Disable The Data Cache and Run the Tests Again */
        diag_printf ("Disabling Data Cache\n");
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();

        /* Call the Pattern Tests */
        if (do_enh_pattern_tests(start, end)) {
            diag_printf ("Failed Pattern Test!\n");
            failed_patterns++;
        }

        /* Call the Address Test */
        if (do_enh_address_test(start, end)) {
            diag_printf ("Failed Address Test!\n");
            failed_address++;
        }
    }

    diag_printf ("********** SUMMARY ************\n");
    diag_printf ("Iterations: %d\n", i);
    diag_printf ("Failed Patterns w/ Cache Enabled:  %d\n", failed_patterns);
    diag_printf ("Failed Address  w/ Cache Enabled:  %d\n", failed_address);
    diag_printf ("Failed Patterns w/ Cache Disabled: %d\n", failed_patterns_unc);
    diag_printf ("Failed Address  w/ Cache Disabled: %d\n", failed_address_unc);
    diag_printf ("*******************************\n");

    /* Dump the Controller Registers */
    dump_sdram_regs();
    diag_printf("\n\nTotal Execution Time: %d s\n", diag_time / 2);
}

#define _Write_FSR(VAL) asm("mcr\tp15, 0, %0, c5, c0, 0" : : "r" (VAL))
#define _Write_FAR(VAL) asm("mcr\tp15, 0, %0, c6, c0, 0" : : "r" (VAL))
#define _Read_FAR ({ unsigned _val_; asm volatile("mrc\tp15, 0, %0, c6, c0, 0" : "=r" (_val_)); _val_; })
#define _Read_FSR ({ unsigned _val_; asm volatile("mrc\tp15, 0, %0, c5, c0, 0" : "=r" (_val_)); _val_; })

// multiPath - Setup and Execute concurrent SDRAM tests SDRAM via:
//     o AAU
//     o DMA engines
//     o Core Memcopy
// The diag menu will pass in a starting address and a total size of
// available memory that is safe to test.
//
// The source data for each unit comes from the data_patterns[] array.
// The Multipath test will loop through the data_patterns array until
// it uses up all the patterns.
int multiPath(cyg_uint32 start, cyg_uint32 total_mem_mb) {
	char recv_data[20];
	cyg_uint32 loop_counter;
	cyg_int32 res;
	static cyg_bool mp_setup_bool = false;
	static mp_setup_t mp_setup;
	cyg_bool done = false;
	cyg_uint32 result = 0;
	cyg_uint32 iterations = 1;
	// Clear Error Registers before starting test
	*BIU_BIUSR = BIUSR_RC_ERROR_BITS;
	_Write_FAR(0x0);
	_Write_FSR(0x0);

	if (mp_setup_bool == false) {

		// Setup Default Units if it hasn't been configured
		mp_setup.num_dma_engines = NUM_DMA_CHANNELS;
		mp_setup.aau_sources     = DEFAULT_AAU_SOURCES;
		mp_setup.num_loops       = 1;
		mp_setup.memcopy_enabled = true;
		mp_setup_bool            = true;
	}

	// Setup start and total Memory Available to test
	mp_setup.start           = start;
	mp_setup.total_mem_mb    = total_mem_mb;

	if ((*ATU_ATUCMD & ATUCMD_IO_SPACE_ENABLE) ||
		(*ATU_ATUCMD & ATUCMD_MEM_SPACE_ENABLE)) {
		mp_setup.atu_enabled = true;
	}
	else  {
		mp_setup.atu_enabled = false;
	}

	// Figure out how many MB each unit will test
	mp_setup.mem_slice = (total_mem_mb)/(mp_setup.num_dma_engines + 2);

	// Defaults are defined - Divy up the available memory between the units
	divy_up_space(&mp_setup);

#if MP_DEBUG == 1
	diag_printf ("Each Unit (AAU, DMAx, Core) will test: %d MB of memory\n", mp_setup.mem_slice);
#endif

	// Print options and allow user to change
	if (print_mp_options(&mp_setup) < 0)
		return(0);

	if (mp_setup.num_loops > 1)
		diag_printf ("\n   **** Starting Loop: %d of %-3d ****\n", iterations, mp_setup.num_loops);

	// Loop until we get through the test patterns array and user selected loops
	do {
		// See if we're done with our address pattern array
		done = setup_local_memory(&mp_setup);

		// If so, see if we're done with our requested number of loops
		if (done == true) {

#if MP_DEBUG == 1
			cyg_uint32 my_sp;
        	asm volatile("mov\t%0, sp" :   "=r" (my_sp));
			diag_printf ("\n            >>> SP: 0x%08x <<< \n", my_sp);
#endif

        	if (iterations != mp_setup.num_loops) {
				diag_printf ("\n   **** Starting Loop: %d of %-3d ****\n", ++iterations, mp_setup.num_loops);
				diag_printf ("      Press <Ctrl+C> to Exit\n");
    			done = false;
	    		res = _rb_gets(recv_data, 1, 3000);
        		if (res == _GETS_CTRLC) {
					diag_printf ("Ctrl-C hit - exiting...\n");
					return(0);
    	    	}
			}
		}

#if MP_DEBUG > 1
done = true;
#endif

		// Reserved area for descriptors is 1M below starting address
	    mp_setup.descriptor_start = start - SZ_1M;

		// Options are all setup - now initialize descriptors
		for (loop_counter = 0; loop_counter < mp_setup.num_dma_engines; loop_counter++) {
			// Setup DMA Descriptors
			init_dma_engine(loop_counter, &mp_setup);
		}

		// Setup AAU Descriptors and source/destination memory blocks
		if (mp_setup.aau_sources != 0)
			init_aau_engine(&mp_setup);

		/* At this point everything is ready - just need to hit "go" */
		// Launch DMA engines
		for (loop_counter = 0; loop_counter < mp_setup.num_dma_engines; loop_counter++) {
			if (loop_counter == 0)
				*DMA_CCR0 |= CCR_ENABLE;
			else
				*DMA_CCR1 |= CCR_ENABLE;
		}

		// Launch AAU Unit
		if (mp_setup.aau_sources != 0)
			*AAU_ACR |= ACR_ENABLE;

		// Launch Memcopy
		if (mp_setup.memcopy_enabled == true) {
			run_memcopy(&mp_setup);
		}

		// Check for errors from any units and make sure data is valid
		result = check_for_done(&mp_setup);
		if (result != 0) {
			diag_printf ("  ********** FAILURE OCCURRED DURING COMPARE ****************\n");
			diag_printf ("  MCU ECC Registers: \n");
			diag_printf ("     MCISR = 0x%08lx, ECCR  = 0x%08x\n", *MCU_MCISR, *MCU_ECCR);
	    	diag_printf ("     ELOG0 = 0x%08lx, ELOG1 = 0x%08lx\n", *MCU_ELOG0, *MCU_ELOG1);
	    	diag_printf ("     ECAR0 = 0x%08lx, ECAR1 = 0x%08lx\n", *MCU_ECAR0, *MCU_ECAR1);
			diag_printf ("  ATU Registers:\n");
			diag_printf ("     ATUCMD: 0x%04x, ATUSR: 0x%04x\n", *ATU_ATUCMD, *ATU_ATUSR);
			diag_printf ("     ATUISR: 0x%08lx \n", *ATU_ATUISR);
			diag_printf ("  DMA Status registers:\n");
			diag_printf ("     DMA0_CSR: 0x%08x, DMA1_CSR: 0x%08x\n", *DMA_CSR0, *DMA_CSR1);
			diag_printf ("  AAU Status registers:\n");
			diag_printf ("     AAU_ASR: 0x%08x\n", *AAU_ASR);
			diag_printf ("  BIU Registers:       \n");
			diag_printf ("     BIU_BIUSR: 0x%08x\n", *BIU_BIUSR);
			diag_printf ("     BIU_BEAR : 0x%08x\n", *BIU_BEAR);
			diag_printf ("     BIU_BIUCR: 0x%08x\n", *BIU_BIUCR);
			diag_printf ("  ********** FAILURE OCCURRED DURING COMPARE ****************\n");
			done = true;
		}

	}while (done == false);

	return (result);
}

// print_mp_options - Print out the options that have been selected and
//					   allow user to change the options
int print_mp_options(mp_setup_t *mp_options)  {

	cyg_int32 temp_selection, menu_selection;

	menu_selection = 0;
	do {
		diag_printf ("==================================================\n");
		diag_printf ("1  DMA Channels Being Used : %3d\n", mp_options->num_dma_engines);
		diag_printf ("2  Number of AAU Sources   : %3d\n", mp_options->aau_sources);
		diag_printf ("3  Execute Memcopy         : %3s\n", ((mp_options->memcopy_enabled) == true) ?\
															"yes" : "no");
		diag_printf ("4  MB of Memory to Test    : %3d\n", mp_options->total_mem_mb  );
		diag_printf ("5  Number of Loops         : %3d\n", mp_options->num_loops  );
		diag_printf ("6  Toggle ATU Setting      : %8s\n", ((mp_options->atu_enabled) == true) ?\
															"enabled" : "disabled");
		diag_printf ("\n7  Exit test (do not exectute)\n");
		diag_printf ("==================================================\n");
 		diag_printf ("Enter Number or <CR> to execute> ");

		menu_selection = decIn();

		diag_printf ("Menu_selection is: %d\n", menu_selection);
 		switch (menu_selection) {
   		// Change # of DMA Channels Used
   		case 1:
   			do {
   				diag_printf ("\nHow many DMA Channels? (currently: %d)\n", mp_options->num_dma_engines);
   				temp_selection = decIn();
   			}while ((temp_selection > NUM_DMA_CHANNELS) || (temp_selection < 0));
   			mp_options->num_dma_engines = temp_selection;
   			divy_up_space(mp_options);
   			break;

   		// Change # AAU Sources
   		case 2:
   			do {
   				diag_printf ("\nHow many AAU Sources? (currently: %d)\n", mp_options->aau_sources);
   				diag_printf ("Enter 0 to disable AAU unit (valid: 4, 8, 16, 32)\n");
   				temp_selection = decIn();
   			}while ((temp_selection > NUM_AAU_SOURCES) || (temp_selection < 0) || \
   				   ((temp_selection != 4) && (temp_selection != 8) && \
   				    (temp_selection != 16) && (temp_selection != 32) && \
   				    (temp_selection != 0)));
   			if (temp_selection != mp_options->aau_sources )
   				mp_options->aau_sources = temp_selection;
   			divy_up_space(mp_options);
   			break;

 		case 3:
   			if (mp_options->memcopy_enabled == true) {
   				mp_options->memcopy_enabled = false;
				divy_up_space(mp_options);
   			}
   			else  {
   				mp_options->memcopy_enabled = true;
				divy_up_space(mp_options);
   			}
   			break;

   		case 4:
   			do {
   				diag_printf ("MB to test (currently : %d\n", mp_options->total_mem_mb);
   				diag_printf ("(Must be >= 4 and <= %-3d Selection> ", mp_options->total_mem_mb);
   				temp_selection = decIn();
   			} while ((temp_selection < 4) || (temp_selection > mp_options->total_mem_mb));
   			mp_options->total_mem_mb = temp_selection;
   			divy_up_space(mp_options);
   			break;

   		case 5:
   			do {
   				diag_printf ("How many Loops? (currently: %d)\n", mp_options->num_loops);
   				diag_printf ("Selection> ");
   				temp_selection = decIn();
   			} while (temp_selection <= 0);
   			mp_options->num_loops = temp_selection;
   			break;

   		case 6:
   			if (mp_options->atu_enabled == true) {
   				mp_options->atu_enabled = false;
   				*ATU_ATUCMD &= ~ATUCMD_IO_SPACE_ENABLE;
   				*ATU_ATUCMD &= ~ATUCMD_MEM_SPACE_ENABLE;
   			}

   			else  {
   				mp_options->atu_enabled = true;
   				*ATU_ATUCMD |= ATUCMD_IO_SPACE_ENABLE;
   				*ATU_ATUCMD |= ATUCMD_MEM_SPACE_ENABLE;
   			}
   			break;

		case 7:
			return(-1);

   		default:
			diag_printf ("Unknown Option: %c\n", menu_selection);
   		}

	} while (menu_selection != 0);
	return (0);
}

// divy_up_space : Number of units (AAU or DMA) has changed - make sure mem_slice is
//                 changed accordingly
void divy_up_space(mp_setup_t *mp_options) {

	cyg_uint32 units = 0;
	cyg_uint32 loop_counter;
	cyg_uint32 local_start = mp_options->start;

	// If Memcopy Enabled - add it as a unit
	if (mp_options->memcopy_enabled)
		units++;

	// If AAU unit is enabled (i.e. #sources not 0) - count it as a source
	if (mp_options->aau_sources != 0)
		units++;

	// Add in the Num of DMA engines in use
	units += mp_options->num_dma_engines;

	if (units == 0) {
		diag_printf ("*** All Units disabled - not a good idea - enabling Memcopy\n");
		CYGACC_CALL_IF_DELAY_US(2000*1000);  // delay for 2S
		mp_options->memcopy_enabled = true;
		units++;
	}

	mp_options->mem_slice = (mp_options->total_mem_mb / units );
	diag_printf ("\nChange in Units - New mem slice for the %d units is : %d MB\n", units, mp_options->mem_slice);

	// Set the starting address for the DMA Units
	for (loop_counter = 0; loop_counter < (mp_options->num_dma_engines); loop_counter++) {
#if MP_DEBUG == 1
		diag_printf ("   New DMA[%d] starting address: 0x%08x\n", loop_counter, local_start);
#endif

		mp_options->dma_start_address[loop_counter] = local_start;
		local_start += (CONVERT_TO_MB(mp_options->mem_slice));
	}

	// If AAU unit is enabled (i.e. #sources not 0) - count it as a source
	if (mp_options->aau_sources != 0) {
#if MP_DEBUG == 1
		diag_printf ("   New AAU     starting address: 0x%08x\n", local_start);
#endif
		mp_options->aau_start_address = local_start;
		local_start += (CONVERT_TO_MB(mp_options->mem_slice));
	}

	// Set memcopy starting address
	if (mp_options->memcopy_enabled == true) {
		mp_options->memcopy_start_address = local_start;
#if MP_DEBUG == 1
		diag_printf ("   New MemCopy starting address: 0x%08x\n", local_start);
#endif
	}
}

// init_dma_engine : Setup descriptors in memory and update the memory descriptor pointer
int init_dma_engine(cyg_uint32 channel, mp_setup_t *mp_options) {

	cyg_uint32 byte_count = (CONVERT_TO_MB(mp_options->mem_slice))/2;
	cyg_uint32 local_start, local_end;
	volatile  cyg_uint32 *dma_ndar;
	volatile  cyg_uint32 *dma_ccr;
	volatile  cyg_uint32 *dma_csr;
	struct    dma_descriptor *dma_desc;
	cyg_uint32 dma_size = SZ_8M;
	local_start = mp_options->dma_start_address[channel];
	local_end   = local_start + byte_count;

#if MP_DEBUG == 1
	diag_printf ("Allocating Descriptors for DMA[%d] for MemBlock 0x%08x\n", \
				channel, mp_options->dma_start_address[channel]);
	diag_printf ("Local Start: 0x%08x, Local End: 0x%08x, Slice: %d(MB), ByteCount: %d\n", \
				local_start, local_end, mp_options->mem_slice, byte_count);
#endif

	if (channel == 0) {
		dma_ndar = DMA_NDAR0;
		dma_ccr  = DMA_CCR0;
		dma_csr  = DMA_CSR0;
	}
	else {
		dma_ndar = DMA_NDAR1;
		dma_ccr  = DMA_CCR1;
		dma_csr  = DMA_CSR1;
	}

	// If DMA engine is running - return a FAIL
	if ((*dma_csr & CSR_ACTIVE) || (*dma_csr & CSR_ERROR_MASK)) {
		diag_printf ("\nChannel%d already active or error triggered --- aborting setup\n", \
					channel);
		diag_printf ("Channel%d CSR: 0x%08x\n", channel, *dma_csr);
		return(-1);
	}
	else
		// Disable channel to update descriptors
		*dma_ccr &= ~CCR_ENABLE;

	// Align Descriptor to 32-byte boundary
	mp_options->descriptor_start += 31;
	mp_options->descriptor_start &= ~(0x1F);
	mp_options->dma_desc_start[channel] = mp_options->descriptor_start;

	// Set descriptor head to descriptor_start address
#if MP_DEBUG == 1
	diag_printf ("Setting NDAR (0x%08x) to 0x%08x\n", (int)dma_ndar, \
			(int)mp_options->descriptor_start);
	diag_printf ("DMA_CCR (0x%08x) is set to: 0x%08x\n", (int)dma_ccr, *dma_ccr);
#endif

	*dma_ndar = (mp_options->descriptor_start);

	// Make descriptors to transfer in "DMA_SIZE" blocks
	while (byte_count > dma_size) {
		dma_desc = (struct dma_descriptor*)(mp_options->descriptor_start);
		memset((void*)dma_desc, 0, sizeof(struct dma_descriptor));
		dma_desc->bc   = dma_size;
		dma_desc->pad  = local_start;
		dma_desc->puad = 0x0;
		dma_desc->lad  = local_end;
		dma_desc->dc   = (DCR_MEM_TO_MEM | DCR_MRB_COMMAND);
		byte_count    -= dma_size;
		local_start   += dma_size;
		local_end	  += dma_size;
		// Setup next descriptor
		mp_options->descriptor_start += sizeof(struct dma_descriptor);
		mp_options->descriptor_start += 31;
		mp_options->descriptor_start &= ~(0x1F);
		dma_desc->nda = mp_options->descriptor_start;
		HAL_DCACHE_FLUSH( dma_desc, sizeof(struct dma_descriptor));
#if MP_DEBUG == 1
		diag_printf ("\nChannel:   %8d\n", channel);
		diag_printf ("======DMA Descriptor at: 0x%08x ======\n", (int)dma_desc);
		diag_printf ("NDA:     0x%08x\n", dma_desc->nda);
		diag_printf ("PAD:     0x%08x\n", dma_desc->pad);
		diag_printf ("PUAD:    0x%08x\n", dma_desc->puad);
		diag_printf ("LAD:     0x%08x\n", dma_desc->lad);
		diag_printf ("BC:      0x%08x\n", dma_desc->bc);
		diag_printf ("DC:      0x%08x\n", dma_desc->dc);
		diag_printf ("======End DMA Descriptor =============\n");
#endif
	}

	// Less than DMA_Size now - finish off the residual byte count
	if (byte_count > 0) {
		dma_desc = (struct dma_descriptor*)(mp_options->descriptor_start);
		memset((void*)dma_desc, 0, sizeof(struct dma_descriptor));
		dma_desc->bc   = byte_count;
		dma_desc->pad  = local_start;
		dma_desc->puad = 0x0;
		dma_desc->lad  = local_end;
		dma_desc->dc   = (DCR_MEM_TO_MEM | DCR_MRB_COMMAND);

		// Setup next descriptor
		mp_options->descriptor_start += sizeof(struct dma_descriptor);
		mp_options->descriptor_start += 31;
		mp_options->descriptor_start &= ~(0x1F);
		dma_desc->nda = 0;
		HAL_DCACHE_FLUSH( dma_desc, sizeof(struct dma_descriptor));

#if MP_DEBUG == 1
		diag_printf ("\nChannel:   %8d\n", channel);
		diag_printf ("======DMA Descriptor at: 0x%08x ======\n", (int)dma_desc);
		diag_printf ("NDA:     0x%08x\n", dma_desc->nda);
		diag_printf ("PAD:     0x%08x\n", dma_desc->pad);
		diag_printf ("PUAD:    0x%08x\n", dma_desc->puad);
		diag_printf ("LAD:     0x%08x\n", dma_desc->lad);
		diag_printf ("BC:      0x%08x\n", dma_desc->bc);
		diag_printf ("DC:      0x%08x\n", dma_desc->dc);
		diag_printf ("======End DMA Descriptor =============\n");
#endif
	}
	return (0);
}

// init_dma_engine : Setup descriptors in memory and update the memory descriptor pointer
int init_aau_engine(mp_setup_t *mp_options) {
	cyg_uint32 byte_count = (CONVERT_TO_MB(mp_options->mem_slice));
	volatile  cyg_uint32 *aau_asr  = AAU_ASR;
	struct    aau_descriptor *aau_desc;
	cyg_uint32 local_start = mp_options->aau_start_address;
	cyg_uint32 bc_per_source;
	cyg_uint32 aau_size = SZ_1M;

#if MP_DEBUG == 1
	diag_printf ("Allocating Descriptors for %d AAU Sources for MemBlock 0x%08x.\n", \
		mp_options->aau_sources, local_start);
	diag_printf ("Local Start: 0x%08x, Slice: %d(MB), ByteCount : %d\n", \
		local_start, mp_options->mem_slice, byte_count);
	diag_printf ("Before alignment, descriptor_start is: 0x%08x\n", mp_options->descriptor_start);
#endif

	// If AAU is running - return a FAIL
	if ((*aau_asr & ASR_ACTIVE) || (*aau_asr & ASR_ERROR_MASK)) {
		diag_printf ("AAU already active or size invalid --- aborting setup\n");
		diag_printf ("ASR Reporting: 0x%08x\n", *aau_asr);
		return(-1);
	}

	// Align Descriptor to 256-byte boundary
	mp_options->descriptor_start += 255;
	mp_options->descriptor_start &= ~(0xFF);
	mp_options->aau_desc_start = mp_options->descriptor_start;

	*AAU_ACR &= ~(ACR_ENABLE);
    asm volatile ("mcr  p15,0,r1,c7,c10,4\n");    // drain write buffer
    CPWAIT();

	// Set descriptor head to descriptor_start address
	*AAU_ANDAR = mp_options->descriptor_start;

#if MP_DEBUG == 1
	// Disable Channel while setting up
	diag_printf ("AAU_ACR   (0x%08x) is: 0x%08x\n", (int)AAU_ACR, *AAU_ACR);
	diag_printf ("AAU_ANDAR (0x%08x) is: 0x%08x\n", (int)AAU_ANDAR, *AAU_ANDAR);
#endif

	// Make descriptors to transfer in "AAU Size" blocks while we have data to xfer
	// Add one for the destination of the XOR as well.
	while (byte_count > (aau_size * (mp_options->aau_sources))) {
		aau_desc = (struct aau_descriptor*)(mp_options->descriptor_start);
		memset((void*)aau_desc, 0, sizeof(struct aau_descriptor));
		setup_aau_descriptor(aau_desc, mp_options, aau_size, local_start);
		byte_count -= (aau_size * (mp_options->aau_sources));
		local_start += (aau_size * (mp_options->aau_sources));

		// Setup next descriptor
		mp_options->descriptor_start += sizeof(struct aau_descriptor);
		mp_options->descriptor_start += 255;
		mp_options->descriptor_start &= ~(0xFF);
		aau_desc->nda = mp_options->descriptor_start;

#if MP_DEBUG == 1
		diag_printf ("NDA :    0x%08x\n", aau_desc->nda);
		diag_printf ("====== End of AAU Descriptor    ======\n", (int)aau_desc);
#endif
		HAL_DCACHE_FLUSH( aau_desc, sizeof(struct aau_descriptor));
	}

	// Less than AAU_Size now - finish off the residual byte count
	if (byte_count > 0) {
		// Convert reamining byte count for this engine into a slice each AAU
		// source can use.
		bc_per_source = (byte_count / (mp_options->aau_sources));
		aau_desc = (struct aau_descriptor*)(mp_options->descriptor_start);
		memset((void*)aau_desc, 0, sizeof(struct aau_descriptor));
		setup_aau_descriptor(aau_desc, mp_options, bc_per_source, local_start);
		// Setup next descriptor
		aau_desc->nda = 0;
		HAL_DCACHE_FLUSH( aau_desc, sizeof(struct aau_descriptor));

#if MP_DEBUG == 1
		diag_printf ("NDA :    0x%08x\n", aau_desc->nda);
		diag_printf ("====== End of AAU Descriptor    ======\n", (int)aau_desc);
#endif

	}
	return (0);
}

// run_memcopy : Call C-Lib memcpy function to do Core Copy
int run_memcopy(mp_setup_t *mp_options) {
	cyg_uint32 byte_count = CONVERT_TO_MB(mp_options->mem_slice);
	cyg_uint32 dest = mp_options->memcopy_start_address + (byte_count/2);

#if MP_DEBUG == 1
	diag_printf ("\nMemCopy from 0x%08x to 0x%08x with size: 0x%08x\n", \
				mp_options->memcopy_start_address, (int)dest, (byte_count / 2));
#endif

	memcpy((void*)dest, (void*)mp_options->memcopy_start_address, (byte_count / 2));
	return (0);
}

// setup_aau_descriptor : fill out an AAU descriptor based on a pointer
//                        to the descriptor, byte count and transfer size
void setup_aau_descriptor (struct aau_descriptor *aau_desc, mp_setup_t *mp_options, \
                           cyg_uint32 aau_size, cyg_uint32 local_start) {

	cyg_uint32 *source_ptr;
	cyg_uint32 source_counter;



	aau_desc->bc   = aau_size;
	aau_desc->dc   = (ADCR_DWE | ADCR_BLOCK1_COMMAND(ADCR_DIRECTFILL) | \
					  ADCR_BLOCK2_COMMAND(ADCR_XOR_COMMAND) |
					  ADCR_BLOCK3_COMMAND(ADCR_XOR_COMMAND) |
					  ADCR_BLOCK4_COMMAND(ADCR_XOR_COMMAND));

	// Setup first 4 source addresses
	source_ptr = &(aau_desc->sar1);
	for (source_counter = 0; source_counter < 4; source_counter++) {
		*source_ptr = local_start;
		*source_ptr++;
		local_start += aau_size;
	}

	if (mp_options->aau_sources >= 8) {
	aau_desc->dc   |=(ADCR_BLOCK5_COMMAND(ADCR_XOR_COMMAND) | \
					  ADCR_BLOCK6_COMMAND(ADCR_XOR_COMMAND) | \
					  ADCR_BLOCK7_COMMAND(ADCR_XOR_COMMAND) | \
					  ADCR_BLOCK8_COMMAND(ADCR_XOR_COMMAND) | \
					  ADCR_EXTENDED_8);

		// Setup sources 5-8
		source_ptr = &aau_desc->sar5;
		for (source_counter = 0; source_counter < 4; source_counter++) {
			*source_ptr = local_start;
			*source_ptr++;
			local_start += aau_size;
		}
	}

    if (mp_options->aau_sources >= 16) {
		aau_desc->dc   |=(ADCR_EXTENDED_16);
		aau_desc->edc0 = (EDCR0_BLOCK9_COMMAND(ADCR_XOR_COMMAND) | \
						  EDCR0_BLOCK10_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR0_BLOCK11_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR0_BLOCK12_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR0_BLOCK13_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR0_BLOCK14_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR0_BLOCK15_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR0_BLOCK16_COMMAND(ADCR_XOR_COMMAND));

		// Setup sources 9-16
		source_ptr = &aau_desc->sar9;
		for (source_counter = 0; source_counter < 8; source_counter++) {
			*source_ptr = local_start;
			*source_ptr++;
			local_start += aau_size;
		}
    }

    if (mp_options->aau_sources == 32) {
		aau_desc->dc   |=(ADCR_EXTENDED_32);
		aau_desc->edc1 = (EDCR1_BLOCK17_COMMAND(ADCR_XOR_COMMAND) | \
						  EDCR1_BLOCK18_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR1_BLOCK19_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR1_BLOCK20_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR1_BLOCK21_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR1_BLOCK22_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR1_BLOCK23_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR1_BLOCK24_COMMAND(ADCR_XOR_COMMAND));
		aau_desc->edc2 = (EDCR2_BLOCK25_COMMAND(ADCR_XOR_COMMAND) | \
						  EDCR2_BLOCK26_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR2_BLOCK27_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR2_BLOCK28_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR2_BLOCK29_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR2_BLOCK30_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR2_BLOCK31_COMMAND(ADCR_XOR_COMMAND)| \
						  EDCR2_BLOCK32_COMMAND(ADCR_XOR_COMMAND));

		// Setup sources 17-24
		source_ptr = &aau_desc->sar17;
		for (source_counter = 0; source_counter < 8; source_counter++) {
			*source_ptr = local_start;
			*source_ptr++;
			local_start += aau_size;
		}

		// Setup sources 24-32
		source_ptr = &aau_desc->sar25;
		for (source_counter = 0; source_counter < 8; source_counter++) {
			*source_ptr = local_start;
			*source_ptr++;
			local_start += aau_size;
		}
    }

	// Destination Address will be SAR1
	aau_desc->dar = aau_desc->sar1;

#if MP_DEBUG == 1
	diag_printf ("======AAU Descriptor at: 0x%08x ======\n", (int)aau_desc);
	diag_printf ("SAR1:    0x%08x\n", aau_desc->sar1);
	diag_printf ("SAR2:    0x%08x\n", aau_desc->sar2);
	diag_printf ("SAR3:    0x%08x\n", aau_desc->sar3);
	diag_printf ("SAR4:    0x%08x\n", aau_desc->sar4);
	diag_printf ("DAR:     0x%08x\n", aau_desc->dar);
	diag_printf ("BC:      0x%08x\n", aau_desc->bc );
	diag_printf ("DC:      0x%08x\n", aau_desc->dc );
	if (mp_options->aau_sources >= 8) {
		diag_printf ("SAR5:    0x%08x\n", aau_desc->sar5);
		diag_printf ("SAR6:    0x%08x\n", aau_desc->sar6);
		diag_printf ("SAR7:    0x%08x\n", aau_desc->sar7);
		diag_printf ("SAR8:    0x%08x\n", aau_desc->sar8);
		if (mp_options->aau_sources >= 16) {
			diag_printf ("EDC0:    0x%08x\n", aau_desc->edc0);
			diag_printf ("SAR9:    0x%08x\n", aau_desc->sar9);
			diag_printf ("SAR10:   0x%08x\n", aau_desc->sar10);
			diag_printf ("SAR11:   0x%08x\n", aau_desc->sar11);
			diag_printf ("SAR12:   0x%08x\n", aau_desc->sar12);
			diag_printf ("SAR13:   0x%08x\n", aau_desc->sar13);
			diag_printf ("SAR14:   0x%08x\n", aau_desc->sar14);
			diag_printf ("SAR15:   0x%08x\n", aau_desc->sar15);
			diag_printf ("SAR16:   0x%08x\n", aau_desc->sar16);
			if (mp_options->aau_sources == 32) {
				diag_printf ("EDC1:    0x%08x\n", aau_desc->edc1);
				diag_printf ("SAR17:   0x%08x\n", aau_desc->sar17);
				diag_printf ("SAR18:   0x%08x\n", aau_desc->sar18);
				diag_printf ("SAR19:   0x%08x\n", aau_desc->sar19);
				diag_printf ("SAR20:   0x%08x\n", aau_desc->sar20);
				diag_printf ("SAR21:   0x%08x\n", aau_desc->sar21);
				diag_printf ("SAR22:   0x%08x\n", aau_desc->sar22);
				diag_printf ("SAR23:   0x%08x\n", aau_desc->sar23);
				diag_printf ("SAR24:   0x%08x\n", aau_desc->sar24);
				diag_printf ("EDC2:    0x%08x\n", aau_desc->edc2);
				diag_printf ("SAR25:   0x%08x\n", aau_desc->sar25);
				diag_printf ("SAR26:   0x%08x\n", aau_desc->sar26);
				diag_printf ("SAR27:   0x%08x\n", aau_desc->sar27);
				diag_printf ("SAR28:   0x%08x\n", aau_desc->sar28);
				diag_printf ("SAR29:   0x%08x\n", aau_desc->sar29);
				diag_printf ("SAR30:   0x%08x\n", aau_desc->sar30);
				diag_printf ("SAR31:   0x%08x\n", aau_desc->sar31);
				diag_printf ("SAR32:   0x%08x\n", aau_desc->sar32);
			}
		}
	}
#endif
}

// setup_local_memory : Use the data_patterns array to fill the local memory
//                      for the data to be transferred via DMA, AAU, and memcopy.
//                      Save the filled value for quick compare when transfers
//                      are complete.
int setup_local_memory(mp_setup_t *mp_options) {

	cyg_uint32 j;
	cyg_uint32 *dma0_memory = 0;
	cyg_uint32 *dma1_memory = 0;
	cyg_uint32 *aau_memory  = 0;
	cyg_uint32 *memcopy_memory = 0;
	cyg_bool   done = false;
	cyg_uint32 byte_count = (CONVERT_TO_MB(mp_options->mem_slice));
	static cyg_uint32 i = 0;

	j = ((sizeof(data_patterns))/sizeof(data_patterns[0]));

	diag_printf ("\n     Each Unit Testing :  %-3d MB of Memory\n", \
			mp_options->mem_slice);

	// Setup DMA Source Memory
	if (mp_options->num_dma_engines >= 1) {
		dma0_memory = (cyg_uint32*)mp_options->dma_start_address[0];
		my_memset(dma0_memory, (int)data_patterns[i++], (byte_count/2));
		// Save data_pattern for verify sequence
		mp_options->dma_pattern[0] = data_patterns[i-1];
		diag_printf ("     Setting DMA0 memory: 0x%08x to 0x%08x\n", \
				(int)dma0_memory, data_patterns[i-1]);
		// Roll over the data_patterns if needed for filling out memory
		if (i >= j) {
			i = 0;
			done = true;
		}
	}

	if (mp_options->num_dma_engines == 2) {
		dma1_memory = (cyg_uint32*)mp_options->dma_start_address[1];
		my_memset(dma1_memory, (int)data_patterns[i++], (byte_count/2));
		// Save data_pattern for verify sequence
		mp_options->dma_pattern[1] = data_patterns[i-1];
		diag_printf ("     Setting DMA1 memory: 0x%08x to 0x%08x\n", \
				(int)dma1_memory, data_patterns[i-1]);
		// Roll over the data_patterns if needed for filling out memory
		if (i >= j) {
			i = 0;
			done = true;
		}
	}

	// Setup AAU Sources
	if (mp_options->aau_sources != 0) {
		aau_memory = (cyg_uint32*)mp_options->aau_start_address;
		my_memset(aau_memory, data_patterns[i++], byte_count);
		diag_printf ("     Setting AAU memory:  0x%08x to 0x%08x\n", \
				(int)aau_memory, data_patterns[i-1], byte_count);

		// Roll over the data_patterns if needed for filling out memory
		if (i >= j) {
			i = 0;
			done = true;
		}
	}

	// Setup Memcopy Source
	if (mp_options->memcopy_enabled == true ) {
		memcopy_memory = (cyg_uint32*)mp_options->memcopy_start_address;
		my_memset((void*)memcopy_memory, (int)data_patterns[i++], ((byte_count)/2));
		// Save data_pattern for verify sequence
		mp_options->memcopy_pattern = data_patterns[i-1];

	 	diag_printf ("     Setting Memcopy mem: 0x%08x to 0x%08x\n", \
				(int)memcopy_memory, data_patterns[i-1]);
#if MP_DEBUG == 1
		diag_printf ("\nBased on slice: 0x%08x, Dma0: 0x%08x, Dma1: 0x%08x\n",
				byte_count, (int)dma0_memory, (int)dma1_memory);
		diag_printf ("AAU Source setting: 0x%08x\n", (int)aau_memory);
		diag_printf ("Memcopy source    : 0x%08x\n", (int)memcopy_memory);
#endif

		// Roll over the data_patterns if needed for filling out memory
		if (i >= j) {
			i = 0;
			done = true;
		}
	}
	return(done);
}

// Memset function for 32-bit values.  RedBoot memset only does 8-bit data :
//    which is probably a bug, and I'll put it on the "to-do" list to look at
//    - cebruns
void my_memset(cyg_uint32* start, cyg_uint32 data, cyg_uint32 byte_count) {

	cyg_uint32 *p_memory = start;
	while ((int)p_memory < ((int)start + byte_count)) {
		*p_memory++ = data;
	}

	HAL_DCACHE_FLUSH( start, byte_count );
}

// check_for_done : Check all engines to make sure they complete successfuly
//      and then verify that the data was transferred correctly.  Finally, check
//      the ECC status unit to check if any errors occurred.
cyg_int32 check_for_done(mp_setup_t *mp_options) {

	cyg_uint32 return_value = 0;
	cyg_uint32 byte_count = CONVERT_TO_MB(mp_options->mem_slice);
	struct dma_descriptor *dma_desc = 0;
	struct aau_descriptor *aau_desc = 0;

#if MP_DEBUG == 1
	diag_printf ("Checking if all our engines have finished successfully\n");
	diag_printf ("Byte_count verify of: 0x%08x\n", byte_count);
#endif

	// If ATU is supposed to be disabled, verify it hasn't been changed
	if (mp_options->atu_enabled == false) {
		if ((*ATU_ATUCMD & ATUCMD_IO_SPACE_ENABLE) ||
			(*ATU_ATUCMD & ATUCMD_MEM_SPACE_ENABLE)) {
				diag_printf (" >>> ATU Has Been Enabled without our Consent! <<<\n");
				diag_printf ("    >> Memory could have changed <<\n");
				return_value |= ATU_CHANGED;
		}
	}

	// Verify DMA Engine Results
	if (mp_options->num_dma_engines > 0) {
		diag_printf ("Verifying DMA Results\n");
		while (*DMA_CSR0 & CSR_ACTIVE) {
			diag_printf ("DMA0 Still running.  DMA_CSR0: 0x%08x\n", *DMA_CSR0);
		}
		if (*DMA_CSR0 & CSR_ERROR_MASK) {
			diag_printf ("DMA0 Had error: 0x%08x\n", *DMA_CSR0);
			return_value |= DMA0_RUNNING_FAIL;;
		}
		else  {
			// Check the destination data matches the source data by parsing
			// through the descriptor chain and calling mem_compare
			HAL_DCACHE_INVALIDATE( mp_options->dma_start_address[0] + (byte_count / 2), \
				(byte_count / 2));
			diag_printf ("\nDMA0 Done: \n");
			dma_desc = (struct dma_descriptor*)mp_options->dma_desc_start[0];
			do {
				if (mem_compare((cyg_uint32*)dma_desc->lad, mp_options->dma_pattern[0], \
					(cyg_uint32)dma_desc->bc) != 0) {
					diag_printf ("***** DMA0 MemCompare Failed\n");
					return_value |= DMA0_COMPARE_FAIL;
				}

				// Verify next descriptor in chain
				dma_desc = (struct dma_descriptor*)dma_desc->nda;
			} while ((dma_desc != 0) && (!(return_value & DMA0_COMPARE_FAIL)));
		}

		// Check the destination data matches the source data by parsing
		// through the descriptor chain and calling mem_compare
		if (mp_options->num_dma_engines > 1) {
			while (*DMA_CSR1 & CSR_ACTIVE) {
				diag_printf ("DMA1 Still running.  DMA_CSR1: 0x%08x\n", *DMA_CSR1);
			}
			if (*DMA_CSR1 & CSR_ERROR_MASK) {
				diag_printf ("DMA1 Had error: 0x%08x\n", *DMA_CSR1);
				return_value |= DMA1_RUNNING_FAIL;
			}
			else  {
				HAL_DCACHE_INVALIDATE( mp_options->dma_start_address[1] + (byte_count / 2), \
					(byte_count / 2));
				diag_printf ("\nDMA1 Done: \n");
				dma_desc = (struct dma_descriptor*)mp_options->dma_desc_start[1];
				do {
					if (mem_compare((cyg_uint32*)dma_desc->lad, mp_options->dma_pattern[1], \
						(cyg_uint32)dma_desc->bc) != 0) {
						diag_printf ("***** DMA1 MemCompare Failed\n");
						return_value |= DMA1_COMPARE_FAIL;
					}

					// Verify next descriptor in chain
					dma_desc = (struct dma_descriptor*)dma_desc->nda;
				} while ((dma_desc != 0) && (!(return_value & DMA1_COMPARE_FAIL)));
			}
		}
	}



	// Verify AAU Results

	if (mp_options->aau_sources != 0) {
		diag_printf ("Verifying AAU Results\n");
		while (*AAU_ASR & ASR_ACTIVE) {
			diag_printf ("AAU Still running.  AAU_ASR: 0x%08x\n", *AAU_ASR);
		}
		if (*AAU_ASR & ASR_ERROR_MASK) {
			diag_printf ("AAU Error.  AAU_ASR: 0x%08x\n", *AAU_ASR);
			return_value |= AAU_RUNNING_FAIL;
		}
		else  {
			diag_printf ("\nAAU  Done:\n");
			// Invalidate Mem space AAU Unit used and Verify Results
			aau_desc = (struct aau_descriptor*)mp_options->aau_desc_start;
			do {
				HAL_DCACHE_INVALIDATE( aau_desc->dar, aau_desc->bc);
				if (mem_compare((cyg_uint32*)aau_desc->dar, 0x0, \
					(cyg_uint32)aau_desc->bc) != 0) {
						diag_printf ("***** AAU MemCompare Failed\n");
						return_value |= AAU_VERIFY_FAIL;
				}
				aau_desc = (struct aau_descriptor*)aau_desc->nda;
			} while ((aau_desc != 0) && (!(return_value & AAU_VERIFY_FAIL)));
		}
	}

	// Verify Memcpy outcome
	if (mp_options->memcopy_enabled == true ) {
		diag_printf ("\nMCopy Done:\n");
		if (mem_compare((cyg_uint32*)(mp_options->memcopy_start_address + (byte_count / 2)), \
					*((cyg_uint32*)mp_options->memcopy_start_address), (byte_count / 2)) != 0) {
			diag_printf ("  **** Failed MemCompare for MemCopy\n");
			return_value |= MEMCOPY_CRC_FAIL;
		}
	}

	if ((*MCU_MCISR & 0x7) != 0x0) {
		diag_printf ("Encountered an ECC Error!!!\n");
		diag_printf("MCISR = 0x%08lx, ECCR  = 0x%08lx\n", *MCU_MCISR, *MCU_ECCR);
	    diag_printf("ELOG0 = 0x%08lx, ELOG1 = 0x%08lx\n", *MCU_ELOG0, *MCU_ELOG1);
	    diag_printf("ECAR0 = 0x%08lx, ECAR1 = 0x%08lx\n", *MCU_ECAR0, *MCU_ECAR1);
		return_value |= ECC_ERROR;
	}
	else  {
		diag_printf ("\nNo Ecc Errors Reported (yet): \n");
		diag_printf("   MCISR = 0x%08x,  ECCR  = 0x%08x\n", *MCU_MCISR, *MCU_ECCR);
	    diag_printf("   ELOG0 = 0x%08lx, ELOG1 = 0x%08lx\n", *MCU_ELOG0, *MCU_ELOG1);
	    diag_printf("   ECAR0 = 0x%08lx, ECAR1 = 0x%08lx\n", *MCU_ECAR0, *MCU_ECAR1);
		diag_printf ("ATU Registers:\n");
		diag_printf ("  ATUCMD: 0x%04x, ATUSR: 0x%04x\n", *ATU_ATUCMD, *ATU_ATUSR);
		diag_printf ("  ATUISR: 0x%08lx \n", *ATU_ATUISR);
		diag_printf ("BIU Registers:       \n");
		diag_printf ("     BIU_BIUSR: 0x%08x\n", *BIU_BIUSR);
		diag_printf ("     BIU_BEAR : 0x%08x\n", *BIU_BEAR);
		diag_printf ("     BIU_BIUCR: 0x%08x\n", *BIU_BIUCR);
		diag_printf ("FSR Registers:       \n");
		diag_printf ("     FSR:     : 0x%08x\n", _Read_FSR);
		diag_printf ("     FAR      : 0x%08x\n", _Read_FAR);
	}
	return (return_value);
}

// Mem_compare : Verify 'base' to 'base+byte_count' is equal to 'value'
int mem_compare(cyg_uint32 *base, cyg_uint32 value, cyg_uint32 byte_count) {

	int i;
	int num_fails = 0;

	diag_printf ("      Verifying : 0x%08x to 0x%08x.  Comparing: 0x%08x...", \
			(int)base, (int)(base+(byte_count/4)), value);

	for (i = 0; i < byte_count; i+=4) {
		if (*base != value) {
			diag_printf ("\n   FAILED TO COMPARE at: 0x%08x\n", (int)base);
			diag_printf ("     Expected value: 0x%08x\n", value);
			diag_printf ("     Found    value: 0x%08x\n", *base);
			diag_printf ("     << Delaying for 1s, invalidating cache, then re-reading >>\n");
			CYGACC_CALL_IF_DELAY_US(1000*1000);  // Delay for 1 sec
			HAL_DCACHE_INVALIDATE( base, 4 );
			if (*base != value) {
				diag_printf ("   SECOND COMPARE FAILED at: 0x%08x\n", (int)base);
				diag_printf ("     Expected value: 0x%08x\n", value);
				diag_printf ("     Found    value: 0x%08x\n", *base);
			}
			else  {
				diag_printf ("   SECOND COMPARE PASSED at: 0x%08x\n", (int)base);
				diag_printf ("       STILL COUNTING AS A FAIL\n");
			}
			num_fails++;
		}
		if (num_fails > 10) {
			diag_printf ("Fails > 10 --- aborting\n");
			return (-1);
		}
		*base++;
	}
	if (num_fails == 0) {
		diag_printf  ("PASSED!\n");
		return (0);
	}
	else
		return (-1);
}

