//==========================================================================
//
//      bulverde_misc.c
//
//      HAL misc board support code for Intel Bulverde Application Processor
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
// Contributors: msalter
// Date:         2002-07-15
// Purpose:      HAL support
// Description:  Implementations of HAL interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

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
#include <cyg/hal/plf_io.h>
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

// Most initialization has already been done before we get here.
// All we do here is set up the interrupt environment.
// FIXME: some of the stuff in hal_platform_setup could be moved here.

externC void plf_hardware_init(void);

void
hal_hardware_init(void)
{
    hal_xscale_core_init();

    // Mask all interrupts except the GPIO2-120 vector
    ICMR_WRITE(1 << CYGNUM_HAL_INTERRUPT_GPIO2_120);
    *INTR_ICLR = 0;   // Clear the interrupt level register
    *INTR_ICCR = 0;   // Clear Interrupt Control Register

    // Let the timer run at a default rate (for delays)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);

    // Set up eCos/ROM interfaces
    hal_if_init();

    // Enable caches
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();

    // Perform any platform specific initializations
    plf_hardware_init();
}

// -------------------------------------------------------------------------
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int
hal_IRQ_handler(void)
{
    cyg_uint32 sources, mask, index;

    // FIXME
    // Currently, we get here by FIQ also. Someday, eCos will properly handle
    // FIQs. Here, the best we can do is give priority to the FIQ sources.
    //
    ICFP_READ(sources);
    if (sources == 0)
	ICIP_READ(sources);

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    // Specific platforms may have additional interrupts. These platform
    // interrupts will need to be passed through one of the CPU interrupts.
    if (sources & (1 << CYGNUM_HAL_INTERRUPT_PLATFORM))
	return hal_plf_IRQ_handler();
#endif

    // Nothing wrong with scanning them in any order we choose...
    // So here we try to make the serial devices steal fewer cycles.
    // So, knowing this is an ARM:
    if (sources & 0xff0000)
        index = 16;
    else if (sources & 0xff00)
        index = 8;
    else if (sources & 0xff)
        index = 0;
    else if (sources & 0xff000000)
        index = 24;
    else
	return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!

    mask = (1 << index);
    do {
        if (mask & sources) {
            if (index == CYGNUM_HAL_INTERRUPT_GPIO2_120) {
		index = 0;
		mask = 1;

                // Special case of GPIO cascade.  Search for lowest set bit
                sources = *GPIO_GEDR0 & 0xfffffffc;
		if (sources) {
		    index = 2;
		    mask = (1 << 2);
		    do {
			if (sources & mask)
			    return index + CYGNUM_HAL_INTERRUPT_GPIO_BASE;
			mask <<= 1;
		    } while (++index <= 31);
		}
                sources = *GPIO_GEDR1;
		if (sources) {
		    do {
			if (sources & (1 << index))
			    return index + CYGNUM_HAL_INTERRUPT_GPIO_BASE + 32;
			mask <<= 1;
		    } while (++index <= 31);
		}
                sources = *GPIO_GEDR2;
		if (sources) {
		    do {
			if (sources & (1 << index))
			    return index + CYGNUM_HAL_INTERRUPT_GPIO_BASE + 64;
			mask <<= 1;
		    } while (++index <= 31);
		}
                sources = *GPIO_GEDR3;
		if (sources) {
		    do {
			if (sources & (1 << index))
			    return index + CYGNUM_HAL_INTERRUPT_GPIO_BASE + 96;
			mask <<= 1;
		    } while (++index <= 24);
		}
		// something odd happened if we get here.
		return CYGNUM_HAL_INTERRUPT_NONE;
            }
            return index;
        }
	index++;
	mask <<= 1;
    } while (index & 7);
    
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}


//
// Interrupt control
//

// Placeholders for the GFER and GRER registers. We keep these here
// to show how the user has configured the GPIO edge detects. The
// actual registers are then used for masking/unmasking the individual
// GPIO edge detect interrupts. Also, GPIO0 and GPIO1 have bits in the
// ICMR, so we use that for those two GPIO regs.
//
// To use a GPIO edge detect interrupt, the user *must* call
// HAL_INTERRUPT_CONFIGURE before calling HAL_INTERRUPT_UNMASK.
//
//
static cyg_uint32 gfer0_conf, gfer1_conf, gfer2_conf, gfer3_conf;
static cyg_uint32 grer0_conf, grer1_conf, grer2_conf, grer3_conf;

void
hal_interrupt_mask(int vector)
{
    cyg_uint32 val, mask;

    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_MASK(vector);
	return;
    }
#endif
    if (vector < CYGNUM_HAL_ISR_MIN)
	return;

    if (vector < CYGNUM_HAL_INTERRUPT_GPIO2) {
	ICMR_READ(val);
	ICMR_WRITE(val & ~(1 << vector));
	return;
    }

    // Must be GPIO 2 - 120

    if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(31)) {
	mask = 1 << (2 + (vector - CYGNUM_HAL_INTERRUPT_GPIO(2)));
	if (gfer0_conf & mask)
	    *GPIO_GFER0 &= ~mask;
	if (grer0_conf & mask)
	    *GPIO_GRER0 &= ~mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(63)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(32));
	if (gfer1_conf & mask)
	    *GPIO_GFER1 &= ~mask;
	if (grer1_conf & mask)
	    *GPIO_GRER1 &= ~mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(95)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(64));
	if (gfer2_conf & mask)
	    *GPIO_GFER2 &= ~mask;
	if (grer2_conf & mask)
	    *GPIO_GRER2 &= ~mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(120)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(96));
	if (gfer3_conf & mask)
	    *GPIO_GFER3 &= ~mask;
	if (grer3_conf & mask)
	    *GPIO_GRER3 &= ~mask;
    }
}

void
hal_interrupt_unmask(int vector)
{
    cyg_uint32 val, mask;

//    HEX_DISPLAY(vector | 0x10000000);

    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_UNMASK(vector);
	return;
    }
#endif

    if (vector < CYGNUM_HAL_INTERRUPT_GPIO2) {
	ICMR_READ(val);
	ICMR_WRITE(val | (1 << vector));
	return;
    }

    // GPIO 2 - 120

    if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(31)) {
	mask = 1 << (2 + (vector - CYGNUM_HAL_INTERRUPT_GPIO(2)));
	if (gfer0_conf & mask)
	    *GPIO_GFER0 |= mask;
	if (grer0_conf & mask)
	    *GPIO_GRER0 |= mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(63)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(32));
	if (gfer1_conf & mask)
	    *GPIO_GFER1 |= mask;
	if (grer1_conf & mask)
	    *GPIO_GRER1 |= mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(95)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(64));
	if (gfer2_conf & mask)
	    *GPIO_GFER2 |= mask;
	if (grer2_conf & mask)
	    *GPIO_GRER2 |= mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(120)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(96));
	if (gfer3_conf & mask)
	    *GPIO_GFER3 |= mask;
	if (grer3_conf & mask)
	    *GPIO_GRER3 |= mask;
    }
}

void
hal_interrupt_acknowledge(int vector)
{
    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_ACKNOWLEDGE(vector);
	return;
    }
#endif

    // The only edge triggered interrupts are the GPIO interrupts.

    if (vector == CYGNUM_HAL_INTERRUPT_GPIO0
	|| vector == CYGNUM_HAL_INTERRUPT_GPIO1) {
	*GPIO_GEDR0 = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO0);
	return;
    }

    if (vector < CYGNUM_HAL_INTERRUPT_GPIO2)
	return;

    if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(31))
	*GPIO_GEDR0 = 1 << (2 + (vector - CYGNUM_HAL_INTERRUPT_GPIO(2)));
    else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(63))
	*GPIO_GEDR1 = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(32));
    else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(95))
	*GPIO_GEDR2 = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(64));
    else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(120))
	*GPIO_GEDR3 = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(96));
}

// This function is used to configure the GPIO interrupts (and possibly
// some platform-specific interrupts).  All of the GPIO pins can potentially
// generate an interrupt.

// If 'level' is true, then both edges are enabled if 'up' is
// true, otherwise they will be disabled. That is:
//
//    level  up  edges
//    -----  --  -----
//      0     0  None
//      0     1  Rising
//      1     0  Falling
//      1     1  Both
//
void
hal_interrupt_configure(int vector, int level, int up)
{
    cyg_uint32 mask;

    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_CONFIGURE(vector, level, up);
	return;
    }
#endif

    if (vector == CYGNUM_HAL_INTERRUPT_GPIO0 
	|| vector == CYGNUM_HAL_INTERRUPT_GPIO1) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO0);
	if (up)
	    *GPIO_GRER0 |= mask;
	else
	    *GPIO_GRER0 &= ~mask;

	if (level)
	    *GPIO_GFER0 |= mask;
	else
	    *GPIO_GFER0 &= ~mask;
	return;
    }

    if (vector < CYGNUM_HAL_INTERRUPT_GPIO2)
	return;

    if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(31)) {
	mask = 1 << (2 + (vector - CYGNUM_HAL_INTERRUPT_GPIO(2)));
	if (up)
	    grer0_conf |= mask;
	else
	    grer0_conf &= ~mask;
	if (level)
	    gfer0_conf |= mask;
	else
	    gfer0_conf &= ~mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(63)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(32));
	if (up)
	    grer1_conf |= mask;
	else
	    grer1_conf &= ~mask;
	if (level)
	    gfer1_conf |= mask;
	else
	    gfer1_conf &= ~mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(95)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(64));
	if (up)
	    grer2_conf |= mask;
	else
	    grer2_conf &= ~mask;
	if (level)
	    gfer2_conf |= mask;
	else
	    gfer2_conf &= ~mask;
    } else if (vector <= CYGNUM_HAL_INTERRUPT_GPIO(120)) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO(96));
	if (up)
	    grer3_conf |= mask;
	else
	    grer3_conf &= ~mask;
	if (level)
	    gfer3_conf |= mask;
	else
	    gfer3_conf &= ~mask;
    }
}

void
hal_interrupt_set_level(int vector, int level)
{
    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_SET_LEVEL(vector);
	return;
    }
#endif
}


/*------------------------------------------------------------------------*/
// RTC Support

static cyg_uint32 _period;

// OSTimer runs at 3.25MHz
#define CLOCK_MULTIPLIER   325
#define CLOCK_DIVISOR      100

void
hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 tmr_period;
    _period = period;

    tmr_period = (period * CLOCK_MULTIPLIER) / CLOCK_DIVISOR;
    
    // Load match value
    *OSTIMER_OSMR0 = tmr_period;

    // Start the counter
    *OSTIMER_OSCR0 = 0;

    // Clear any pending interrupt
    *OSTIMER_OSSR = OSSR_TIMER0;

    // Enable timer 0 interrupt    
    *OSTIMER_OIER |= OIER_TIMER0;

    // That's all.
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
#define MIN_TICKS (32)
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

// Define this if you want to ensure that the clock is perfect (i.e. does
// not drift).  One reason to leave it turned off is that it costs some
// us per system clock interrupt for this maintenance.
#undef COMPENSATE_FOR_CLOCK_DRIFT

void
hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
#ifdef COMPENSATE_FOR_CLOCK_DRIFT
    cyg_uint32 next = *OSTIMER_OSMR0 + period;    // Next interrupt time
    *OSTIMER_OSSR = OSSR_TIMER0;                  // Clear any pending interrupt
    *OSTIMER_OSMR0 = next;                        // Load new match value
    {
        cyg_uint32 ctr = *OSTIMER_OSCR0;
        _period = next - ctr;
        if ((_period - 1) >= period) {           // Adjust for missed interrupts
            *OSTIMER_OSMR0 = ctr + period;
            *OSTIMER_OSSR = OSSR_TIMER0;         // Clear pending interrupt
            _period = period;
        }
    }
#else
    *OSTIMER_OSMR0 = *OSTIMER_OSCR0 + period;    // Load new match value
    *OSTIMER_OSSR = OSSR_TIMER0;                 // Clear any pending interrupt
#endif
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)
void
hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 val;

    val = *OSTIMER_OSCR0 - *OSTIMER_OSMR0;
    val = (val * CLOCK_DIVISOR) / CLOCK_MULTIPLIER;
    
    *pvalue = _period - val;
}

// Delay for some usecs.
void
hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 ticks, start;

    ticks = (3 * usecs) + (2 * usecs/10) + (5 * usecs/100);

    start = *OSTIMER_OSCR0;
    while ((cyg_int32)(*OSTIMER_OSCR0 - start - ticks) < 0);
}

/*------------------------------------------------------------------------*/
// EOF bulverde_misc.c

