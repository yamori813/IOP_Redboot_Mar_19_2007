//==========================================================================
//
//      ioc80314_misc.c
//
//      HAL misc board support code for Intel Verde I/O Coprocessor
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
// Author(s):    msalter
// Contributors: msalter, dkranak, cebruns
// Date:         2001-12-03
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <redboot.h>
#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_stub.h>           // Stub macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_io.h>
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED


extern int memTest (long startAddr, long endAddr);

// Most initialization has already been done before we get here.
// All we do here is set up the interrupt environment.
#ifdef PKG_HAL_HAS_MULT_CPU
volatile long        _curProc;
volatile cyg_uint32* ptr_ciu_base;
#endif
volatile cyg_uint32  currentTimerValue;

externC void plf_hardware_init(void);

static cyg_uint32 mcu_ISR(cyg_vector_t vector, cyg_addrword_t data);
int check_ecc_errors(void);

void
hal_hardware_init(void)
{
#ifdef PKG_HAL_HAS_MULT_CPU
    volatile unsigned long * ciu_base = (unsigned long *)(HAL_IOC80314_CIU_BASE);
#endif
    volatile unsigned long * reset_base = (unsigned long *)SDRAM_UNCACHED_BASE;
    unsigned long            warm_start_instruction;

#ifdef PKG_HAL_HAS_MULT_CPU
    _curProc = (ciu_base[TS_CIU_CFG/4] & TS_CIU_CFG_PID)>>8 ;
    ptr_ciu_base = (cyg_uint32*)(HAL_IOC80314_CIU_BASE);
#endif

    hal_xscale_core_init();

    // Perform any platform specific initializations
    plf_hardware_init();
    cyg_hal_plf_pci_init();
	// Fix warm start instruction
    warm_start_instruction         = reset_base[1];
    reset_base[0]             = warm_start_instruction;
	HAL_DCACHE_SYNC();

    // Let the timer run at a default rate (for delays)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);

    // Set up eCos/ROM interfaces
    hal_if_init();

   // Enable caches
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();

    // attach interrupt handlers for MCU errors
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_MCU_ERR, &mcu_ISR, CYGNUM_HAL_INTERRUPT_MCU_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_MCU_ERR);
}

// -------------------------------------------------------------------------
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int
hal_IRQ_handler(void)
{
    cyg_uint32 sources = 0;
	cyg_uint32 mask = 0;
    int index;

    INTCTL_READ(mask);
    IINTSRC_READ(sources);

    sources &= mask; // just the unmasked ones

    if (sources) {
	HAL_LSBIT_INDEX( index, sources );
	return index;
    }
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

static inline void
_scrub_ecc(unsigned p)
{
    asm volatile ("ldrb r4, [%0]\n"
		  "strb r4, [%0]\n" : : "r"(p) : "r4");
}


static cyg_uint32
mcu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{

	/* IOC80314 doesn't have interrupts enabled */
	return -1;

}


//
// Interrupt control
//

void
hal_interrupt_mask(int vector)
{
    if (vector <= CYGNUM_HAL_INTERRUPT_HPI) {
	int mask;

	INTCTL_READ(mask);
	mask &= ~(1 << vector);
	INTCTL_WRITE(mask);
	CPWAIT();
    }
}

void
hal_interrupt_unmask(int vector)
{
    if (vector <= CYGNUM_HAL_INTERRUPT_HPI) {
	int mask;

	INTCTL_READ(mask);
	mask |= (1 << vector);
	INTCTL_WRITE(mask);
	CPWAIT();
    }
}

void
hal_interrupt_acknowledge(int vector)
{
    // If this is a timer interrupt, write a 1 to the appropriate bit
    // in the TISR register.
    if( vector == CYGNUM_HAL_INTERRUPT_TIMER0 ||
	vector == CYGNUM_HAL_INTERRUPT_TIMER1 )
    {
	TISR_WRITE(1<<(vector-CYGNUM_HAL_INTERRUPT_TIMER0));
    }
}


void hal_interrupt_configure(int vector, int level, int up)
{
}

void hal_interrupt_set_level(int vector, int level)
{
}


/*------------------------------------------------------------------------*/
// RTC Support


#define CLOCK_MULTIPLIER  1            // Prescale = 0 clock runs at SFN speed.
cyg_uint32 _TICKS_PER_USEC[] = {(CYGNUM_HAL_SFN_SPEED/1000000), (CYGNUM_HAL_PROCESSOR_SPEED/1000000)};

static cyg_uint32 _period = 0;
static cyg_uint32 clocks_per_roll_over;

void
hal_clock_initialize(cyg_uint32 period)
{
    volatile int gtccr = (volatile int)GTAD(0);
    cyg_uint32 bc_val, cc_val;

    // disable timer
    GTBCR_WRITE(GTM0, (TS_MPIC_GTBCR_CI + 0));  // CI => Count Inhibit
    GTVPR_WRITE(GTM0,  TS_MPIC_GTVPR_M);       // Disable interrupts
    GTCCR_WRITE(GTM0,  0);                     // Zero counter
    gtccr++;

    _period = period;
    clocks_per_roll_over = _period * CLOCK_MULTIPLIER * _TICKS_PER_USEC[_TICKS_INDEX];

    // clear interrupts

    // set reload/count value
    GTBCR_WRITE(GTM0, clocks_per_roll_over);      // CI == 0 enables counter

    GTBCR_READ(GTM0, bc_val);           // Read Just for debug.
    GTCCR_READ(GTM0, cc_val);
}


// Dynamically set the timer interrupt rate.
// Not for eCos application use at all, just special GPROF code in RedBoot.

void
hal_clock_reinitialize(          int *pfreq,    /* inout */
                        unsigned int *pperiod,  /* inout */
                        unsigned int old_hz )   /* in */
{
    unsigned int newp = 0, period, i = 0;
    int hz;
    int do_set_hw;

// Arbitrary choice somewhat - so the CPU can make
// progress with the clock set like this, we hope.
#define MIN_TICKS (1)                    //(2000)
#define MAX_TICKS  N/A: 32-bit counter

    if ( ! pfreq || ! pperiod )
        return; // we cannot even report a problem!

    hz = *pfreq;
    period = *pperiod;

// Requested HZ:
// 0         => tell me the current value (no change, implemented in caller)
// - 1       => tell me the slowest (no change)
// - 2       => tell me the default (no change, implemented in caller)
// -nnn      => tell me what you would choose for nnn (no change)
// MIN_INT   => tell me the fastest (no change)
//
// 1         => tell me the slowest (sets the clock)
// MAX_INT   => tell me the fastest (sets the clock)

    do_set_hw = (hz > 0);
    if ( hz < 0 )
        hz = -hz;

    // Be paranoid about bad args, and very defensive about underflows
    if ( 0 < hz && 0 < period && 0 < old_hz ) {

        newp = period * old_hz / (unsigned)hz;

        if ( newp < MIN_TICKS ) {
            newp = MIN_TICKS;
            // recalculate to get the exact delay for this integral hz
            // and hunt hz down to an acceptable value if necessary
            i = period * old_hz / newp;
            if ( i ) do {
                newp = period * old_hz / i;
                i--;
            } while (newp < MIN_TICKS && i);
        }
        // So long as period * old_hz fits in 32 bits, there is no need to
        // worry about overflow; hz >= 1 in the initial divide.  If the
        // clock cannot do a whole second (period * old_hz >= 2^32), we
        // will get overflow here, and random returned HZ values.

        // Recalculate the actual value installed.
        i = period * old_hz / newp;
    }

    *pfreq = i;
    *pperiod = newp;

    if ( do_set_hw ) {
        hal_clock_initialize( newp );
    }
}

// This routine is called during a clock interrupt.
void
hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)
void
hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 timer_val;

    GTCCR_READ(GTM0, timer_val);

    // Translate timer value back into microseconds

    timer_val /= (CLOCK_MULTIPLIER * _TICKS_PER_USEC[_TICKS_INDEX]);

    *pvalue = _period - timer_val;
}

// Delay for some usecs and get input if chanel != NULL.
cyg_bool
hal_delay_us_getc(cyg_int32 delay, void* __ch_data, cyg_uint8* ch)
{
    volatile cyg_uint32    now, prev;
             cyg_uint64    usecsclock, delayclock, diff;

    if ( _period == 0) {
      hal_clock_initialize(1);
    }
    if (delay == 0) {
      if (__ch_data) {
         return cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
         }
      else
        {
        return FALSE;
        }
    }
    diff = usecsclock = 0;
    GTCCR_READ(GTM0, prev);             // Counter counts down.
    delayclock = (delay * _TICKS_PER_USEC[_TICKS_INDEX]);
    while (delayclock > usecsclock) {
       GTCCR_READ(GTM0, now);
      if (__ch_data) {
         if (cyg_hal_plf_serial_getc_nonblock(__ch_data, ch)) {
           return TRUE;
         }
      }
       if (prev < now)
           diff = (cyg_uint64)(prev + (clocks_per_roll_over - now));
       else
           diff =(cyg_uint64) (prev - now);

       prev = now;

       usecsclock += diff;
    }                                   // end While

  return FALSE;
}

// Delay for some usecs.
void
hal_delay_us(cyg_int32 delay)
{
hal_delay_us_getc(delay, NULL, NULL);
}


// Disable ECC
// Pretty self-explanatory - clears SDRAM ECC checking, CIU ECC Checking and then the 80200
// ECC
//
void disable_ecc (int arg, char *argv[]) {
	unsigned int sd_ecc_status;
	unsigned int ciu_ecc_status;
	sd_ecc_status = *(volatile unsigned int*)(SD_ECC_STATUS);
	ciu_ecc_status = *(volatile unsigned int*)(CIU_CFG);
	if ((((sd_ecc_status & (SD_ECC_ENABLE)) == 0) && ((ciu_ecc_status & (CIU_ECC_ENABLE)) == 0)) && \
		   ((ciu_ecc_status & (CIU_PARITY_ENABLE)) == 0))
			diag_printf ("ECC Already Disabled\n");
	else {
		diag_printf ("Disabling XScale/CIU/SDRAM ECC and SRAM Parity....");
		*(volatile unsigned int*)(CIU_CFG) &= ~(CIU_ECC_ENABLE + CIU_PARITY_ENABLE);// Clear CIU ECC and SRAM Parity
		*(volatile unsigned int*)(SD_ECC_STATUS) &= ~(SD_ECC_ENABLE); // Clear SDRAM ECC
		/* Clear 80200 ECC */
		asm volatile ( 
		"mrc 13, 0, r0, cr0, cr1, 0;" \
		"bic r0, r0, #8;"  \
		"mcr 13, 0, r0, cr0, cr1, 0;" \
		: \
		: \
		: "r0" \
		);
		diag_printf ("Disabled\n");
	}

}

// Check ECC Error
// Code to check CIU and SDRAM ECC_STATUS/ADDRESS registers to see if an ECC Error is logged
// Since RedBoot has interrupts disabled - we'll just poll this routine after a memtest to
// check for SBE that might have been corrected without our knowledge.  MBE will lock us up,
// so no way to verify those w/o interrupts.
// Return Values:
//  0 : No Errors found
// -1 : ECC Disabled
//  1 : SBE Found in SDRAM
//  2 : SBE Found in CIU
//  3 : SBE Found in SDRAM + CIU
int check_ecc_errors (void) {
	unsigned int sd_ecc_status;
	unsigned int ciu_cfg_status;
	unsigned int ciu_ecc_status;
	int ret_value = 0;
	sd_ecc_status  = *(volatile unsigned int*)(SD_ECC_STATUS);
	ciu_cfg_status = *(volatile unsigned int*)(CIU_CFG);
	if (((sd_ecc_status & (SD_ECC_ENABLE)) == 0) && ((ciu_cfg_status & (CIU_ECC_ENABLE)) == 0))
		ret_value = -1;
	else {
		ciu_ecc_status = *(volatile unsigned int*)(CIU_ECC_STATUS);
		if ((ciu_ecc_status & (CIU_ECC_ERROR_MASK)) != 0) {
			diag_printf ("CIU SingleBit ECC Error Found at 0x%08x\n", *(volatile unsigned int*)(CIU_ECC_ERROR_AD));
			ret_value |= 2;
		}
		if ((sd_ecc_status & (SD_ECC_ERROR_MASK)) != 0)  {
			diag_printf ("SDRAM SingleBit ECC Error Found at 0x%08x\n", *(volatile unsigned int*)(SD_ECC_ADDR1));
			ret_value |= 1;
		}
	}
	return(ret_value);
}



RedBoot_cmd("ecc",
            "Disable ECC",
            ": disables CIU+SDRAM ECC, and SRAM Parity - reset to enable",
            disable_ecc
    );

/*
void
flush_dcache_area(int argc, char *argv[])
{
    struct option_info opts[2];
    unsigned long base, len;
	bool* base_set, len_set;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base, (bool *)&base_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&len, (bool *)&len_set, "length");

    if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
        return;
    }
	diag_printf ("Flushing : 0x%08x for %d bytes\n", base, len);
	HAL_DCACHE_STORE(base, len);
}


RedBoot_cmd("flush",
            "Flush address length of data cache",
            " [-b] base address to flush [-l] length in bytes of flush",
            flush_dcache_area
    );
*/

/*------------------------------------------------------------------------*/
// EOF ioc80314_misc.c

