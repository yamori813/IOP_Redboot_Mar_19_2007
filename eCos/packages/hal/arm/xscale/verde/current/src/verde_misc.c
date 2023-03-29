//==========================================================================
//
//      verde_misc.c
//
//      HAL misc board support code for Intel Verde I/O Coprocessor
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
// Contributors: msalter, dmoseley
// Date:         2001-12-03
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

//#define DEBUG_ECC
//#define DEBUG_IRQ_HANDLER

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

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
#include <cyg/hal/hal_xscale_iop.h>
#include <cyg/hal/plf_io.h>
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

// Most initialization has already been done before we get here.
// All we do here is set up the interrupt environment.
// FIXME: some of the stuff in hal_platform_setup could be moved here.

externC void plf_hardware_init(void);

static cyg_uint32 mcu_ISR(cyg_vector_t vector, cyg_addrword_t data);

void
hal_hardware_init(void)
{
    hal_xscale_core_init();

    // Perform any platform specific initializations
    plf_hardware_init();

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
    cyg_uint32 sources, mask;
    int index;

#ifdef DEBUG_IRQ_HANDLER
    extern void cyg_hal_plf_comms_init(void);
    cyg_hal_plf_comms_init();
    diag_printf("In %s()\n", __FUNCTION__);
#endif

    INTCTL_READ(mask);
    IINTSRC_READ(sources);

#ifdef DEBUG_IRQ_HANDLER
    diag_printf("mask = 0x%08lx\n", mask);
    diag_printf("sources = 0x%08lx\n", sources);
#endif

    sources &= mask; // just the unmasked ones

    if (sources) {
	HAL_LSBIT_INDEX( index, sources );
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources returning: %d\n", index);
#endif
	return index;
    }
#ifdef DEBUG_IRQ_HANDLER
    diag_printf("returning: CYGNUM_HAL_INTERRUPT_NONE\n");
#endif
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

#ifdef CYGSEM_HAL_ARM_VERDE_ENABLE_ECC
static void _scrub_ecc(cyg_uint32 p)
{
    cyg_uint32 iacr;
#ifdef DEBUG_ECC
    void (*dp)(const char *fmt, ...) = diag_printf; 
    dp("In _scrub_ecc(0x%08lx)\n", p);
#endif

    // The following ldr/str pair need to be atomic on the bus. Since
    // the XScale core doesn't support atomic RMW, we have to disable
    // arbitration to prevent other bus masters from taking the bus
    // between the the ldr and str.

    iacr = *ARB_IACR;
#ifdef DEBUG_ECC
    dp("iacr = 0x%08lx\n", iacr);
#endif

    // Disable internal bus arbitration for everything except the CPU and PBI
    *ARB_IACR = (iacr & IACR_RSV_MASK)  |
                IACR_ATU(IACR_PRI_OFF)  |
                IACR_DMA0(IACR_PRI_OFF) |
  	        IACR_DMA1(IACR_PRI_OFF) |
#ifdef CYGPKG_HAL_ARM_XSCALE_VERDE_IOP_FLAVOR_IOP321
                IACR_AAU(IACR_PRI_OFF)  |
#endif // CYGPKG_HAL_ARM_XSCALE_VERDE_IOP_FLAVOR_IOP321
	        IACR_PBI(IACR_PRI_HIGH) |
                IACR_CORE(IACR_PRI_HIGH);
#ifdef DEBUG_ECC
    dp("*ARB_IACR = 0x%08lx\n", *ARB_IACR);
#endif

    // drain write buffer
    asm volatile ("mcr  p15,0,r1,c7,c10,4\n");
    CPWAIT();

    asm volatile ("ldrb r4, [%0]\n"
		  "strb r4, [%0]\n" : : "r"(p) : "r4");

    // flush the cache line
    asm volatile ("mcr p15, 0, %0, c7, c10, 1" : : "r"(p));

    // Restore normal internal bus arbitration priorities
    *ARB_IACR = iacr;
#ifdef DEBUG_ECC
    dp("*ARB_IACR = 0x%08lx\n", *ARB_IACR);
#endif
}
#endif // CYGSEM_HAL_ARM_VERDE_ENABLE_ECC

static cyg_uint32
mcu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
#ifdef CYGSEM_HAL_ARM_VERDE_ENABLE_ECC
    cyg_uint32 eccr_reg;
#endif
    cyg_uint32 mcisr_reg;

#ifdef CYGSEM_HAL_ARM_VERDE_ENABLE_ECC
    // Read current state of ECC register
    eccr_reg = *MCU_ECCR;
#endif

    // and the interrupt status
    mcisr_reg = *MCU_MCISR;

#ifdef CYGSEM_HAL_ARM_VERDE_ENABLE_ECC
    // Turn off all ecc error reporting
    *MCU_ECCR = 0xc;

#ifdef DEBUG_ECC
    diag_printf("mcu_ISR entry: ECCR = 0x%X, MCISR = 0x%X\n", eccr_reg, mcisr_reg);
#endif

    // Check for ECC Error 0
    if(mcisr_reg & 1) {
	
#ifdef DEBUG_ECC
	diag_printf("ELOG0 = 0x%X\n", *MCU_ELOG0);
	diag_printf("ECC Error Detected at Address 0x%X\n",*MCU_ECAR0);
#endif
	
	// Check for single-bit error
        if(!(*MCU_ELOG0 & 0x00000100)) {
	    // call ECC restoration function
	    _scrub_ecc(CYGARC_VIRTUAL_ADDRESS(*MCU_ECAR0));

	    // Clear the MCISR
	    *MCU_MCISR = 1;
        } else {
#ifdef DEBUG_ECC
            diag_printf("Multi-bit or nibble error\n");
#endif
	}
    }

    // Check for ECC Error 1
    if(mcisr_reg & 2) {

#ifdef DEBUG_ECC
	diag_printf("ELOG1 = 0x%X\n",*MCU_ELOG1);
	diag_printf("ECC Error Detected at Address 0x%X\n",*MCU_ECAR1);	
#endif
        
	// Check for single-bit error
        if(!(*MCU_ELOG1 & 0x00000100))  {
	    // call ECC restoration function
	    _scrub_ecc(CYGARC_VIRTUAL_ADDRESS(*MCU_ECAR1));
 
	    // Clear the MCISR
	    *MCU_MCISR = 2;
	}
	else {
#ifdef DEBUG_ECC
            diag_printf("Multi-bit or nibble error\n");
#endif
	}
    }

    // Check for ECC Error N
    if(mcisr_reg & 4) {
	// Clear the MCISR
	*MCU_MCISR = 4;
	diag_printf("Uncorrectable error during RMW\n");
    }

#endif // CYGSEM_HAL_ARM_VERDE_ENABLE_ECC
    
#ifdef CYGSEM_HAL_ARM_VERDE_ENABLE_ECC
    // Restore ECCR register
    *MCU_ECCR = eccr_reg;
#endif // CYGSEM_HAL_ARM_VERDE_ENABLE_ECC

#ifdef DEBUG_ECC
    diag_printf("mcu_ISR exit: MCISR = 0x%X\n", *MCU_MCISR);
#endif

    return CYG_ISR_HANDLED;
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

static cyg_uint32 _period;

#define CLOCK_MULTIPLIER 200

void
hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 val;
    cyg_uint32 tmr_period;
    _period = period;

    tmr_period = period * CLOCK_MULTIPLIER;
    
    // disable timer
    TMR0_WRITE(0);

    // clear interrupts
    TISR_WRITE(1);

    // set reload/count value
    TRR0_WRITE(tmr_period);
    TCR0_WRITE(tmr_period);

    // let it run
    TMR0_WRITE(TMR_ENABLE | TMR_RELOAD | TMR_CLK_1);

    TMR0_READ(val);
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
#define MIN_TICKS (2000)
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

    TCR0_READ(timer_val);

    // Translate timer value back into microseconds
    
    timer_val /= CLOCK_MULTIPLIER;
    
    *pvalue = _period - timer_val;
}

// Delay for some usecs.
void
hal_delay_us(cyg_int32 delay)
{
#define _TICKS_PER_USEC CLOCK_MULTIPLIER
    cyg_uint32 now, prev, diff, usecs;
    cyg_uint32 tmr_period = _period * CLOCK_MULTIPLIER;

    diff = usecs = 0;
    TCR0_READ(prev);

    while (delay > usecs) {
	TCR0_READ(now);

	if (prev < now)
	    diff += (prev + (tmr_period - now));
	else
	    diff += (prev - now);

	prev = now;

	if (diff >= _TICKS_PER_USEC) {
	    usecs += (diff / _TICKS_PER_USEC);
	    diff %= _TICKS_PER_USEC;
	}
    }
}

/*------------------------------------------------------------------------*/
// EOF verde_misc.c

