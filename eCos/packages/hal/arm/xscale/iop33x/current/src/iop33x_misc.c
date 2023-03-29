//==========================================================================
//
//      iop33x_misc.c
//
//      HAL misc board support code for Intel IOP331 and IOP332
//      I/O Processors
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.
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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-07-10
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
extern int hal_pci_cfg_flags;

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
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_MCU_ERR,
                          &mcu_ISR,
                          CYGNUM_HAL_INTERRUPT_MCU_ERR,
                          0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_MCU_ERR);
}

// -------------------------------------------------------------------------
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int
hal_IRQ_handler(void)
{
    cyg_uint32 sources0, mask0, sources1, mask1;
    int index;

#ifdef DEBUG_IRQ_HANDLER
    extern void cyg_hal_plf_comms_init(void);
    cyg_hal_plf_comms_init();
    diag_printf("In %s()\n", __FUNCTION__);
#endif

    INTCTL0_READ(mask0);
    IINTSRC0_READ(sources0);
    INTCTL1_READ(mask1);
    IINTSRC1_READ(sources1);

#ifdef DEBUG_IRQ_HANDLER
    diag_printf("mask0 = 0x%08lx\n", mask0);
    diag_printf("mask1 = 0x%08lx\n", mask1);
    diag_printf("sources0 = 0x%08lx\n", sources0);
    diag_printf("sources1 = 0x%08lx\n", sources1);
#endif
    // just the unmasked ones
    sources0 &= mask0;
    sources1 &= mask1;

    if (sources0) {
	HAL_LSBIT_INDEX( index, sources0 );
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources0 returning: %d\n", index);
#endif
	return index;
    } else if (sources1) {
	HAL_LSBIT_INDEX( index, sources1 );
        index += CYGNUM_HAL_ISR1_MIN;
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources1 returning: %d\n", index);
#endif
	return index;
    }
#ifdef DEBUG_IRQ_HANDLER
    diag_printf("returning: CYGNUM_HAL_INTERRUPT_NONE\n");
#endif
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

#ifdef CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
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
                IACR_AAU(IACR_PRI_OFF)  |
	        IACR_PBI(IACR_PRI_HIGH) |
                IACR_CORE(IACR_PRI_HIGH);
#ifdef DEBUG_ECC
    dp("*ARB_IACR = 0x%08lx\n", *ARB_IACR);
#endif

    asm volatile ("mcr  p15,0,r1,c7,c10,4\n");    // drain write buffer
    CPWAIT();

    // Do the ECC clean operation
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
#endif // CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC

static cyg_uint32
mcu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
#ifdef CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
    cyg_uint32 eccr_reg;
#endif
    cyg_uint32 mcisr_reg;

#ifdef CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
    // Read current state of ECC register
    eccr_reg = *MCU_ECCR;
#endif

    // and the interrupt status
    mcisr_reg = *MCU_MCISR;

#ifdef CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
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

#endif // CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
    
    // Check for Address Region Error
    if(mcisr_reg & 8) {
	// Clear the MCISR
	*MCU_MCISR = 8;
	diag_printf("Address region error during RMW\n");
    }
    
    // Check for IB Discard Timer Expired Error
    if(mcisr_reg & 0x10) {
	// Clear the MCISR
	*MCU_MCISR = 0x10;
	diag_printf("IB Discard Timer Expired error during RMW\n");
    }
    
#ifdef CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
    // Restore ECCR register
    *MCU_ECCR = eccr_reg;
#endif // CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC

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
    if ((vector >= CYGNUM_HAL_ISR0_MIN) && (vector <= CYGNUM_HAL_ISR0_MAX)) {
	int mask;
	INTCTL0_READ(mask);
	mask &= ~(1 << (vector - CYGNUM_HAL_ISR0_MIN));
	INTCTL0_WRITE(mask);
	CPWAIT();
    } else if ((vector >= CYGNUM_HAL_ISR1_MIN) && (vector <= CYGNUM_HAL_ISR1_MAX)) {
	int mask;
	INTCTL1_READ(mask);
	mask &= ~(1 << (vector - CYGNUM_HAL_ISR1_MIN));
	INTCTL1_WRITE(mask);
	CPWAIT();
    }
}

void
hal_interrupt_unmask(int vector)
{
    if ((vector >= CYGNUM_HAL_ISR0_MIN) && (vector <= CYGNUM_HAL_ISR0_MAX)) {
	int mask;
	INTCTL0_READ(mask);
	mask |= (1 << (vector - CYGNUM_HAL_ISR0_MIN));
	INTCTL0_WRITE(mask);
	CPWAIT();
    } else if ((vector >= CYGNUM_HAL_ISR1_MIN) && (vector <= CYGNUM_HAL_ISR1_MAX)) {
	int mask;
	INTCTL1_READ(mask);
	mask |= (1 << (vector - CYGNUM_HAL_ISR1_MIN));
	INTCTL1_WRITE(mask);
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
static cyg_uint32 CLOCK_MULTIPLIER;

void
hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 val;
    cyg_uint32 tmr_period;
    cyg_uint32 dev_id;

    // Set the CLOCK_MULTIPLIER based on the device stepping
    asm volatile("mrc\tp15, 0, %0, c0, c0, 0" : "=r" (dev_id));
    if ((dev_id == IOP331_DEVICE_ID_A_1) ||
        (dev_id == IOP331_DEVICE_ID_B_0) ||
        (dev_id == IOP331_DEVICE_ID_C_0) ||
        (dev_id == IOP331_DEVICE_ID_C_1) ||
        (dev_id == IOP332_DEVICE_ID_A_1)  ||
        (dev_id == IOP332_DEVICE_ID_A_2)  ||
        (dev_id == IOP332_DEVICE_ID_B_0)  ||
        (dev_id == IOP332_DEVICE_ID_C_0)  ||
        (dev_id == IOP332_DEVICE_ID_C_1)) {
        // For IOP332 and IOP331 A, B, or C steps
        CLOCK_MULTIPLIER = 266;
    } else {
        // For IOP333 and IOP331 D steps and above
        CLOCK_MULTIPLIER = 333;
    }

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
    TMR0_WRITE(TMR_TIMER_ENABLE | TMR_AUTO_RELOAD_ENABLE | TMR_CLK_1);

    TMR0_READ(val);
}


// Dynamically set the timer interrupt rate.
// Not for eCos application use at all, just special GPROF code in RedBoot.
#warning Move hal_clock_reinitialize out of variant code.
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

    while ((cyg_uint32)delay > usecs) {
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
// i2c Bus reset

#define I2C_BUS_CLEAR_DELAY(w)                          \
{                                                       \
    register unsigned long _val_ = (w);                 \
    if (_val_)  {                                       \
       asm volatile (                                   \
           "0:  subs %0,%0,#1  \n"                      \
           "    bne  0b        \n"                      \
           "1:                 \n" : "+r" (_val_) );    \
    }                                                   \
}

void hal_platform_i2c_bus_reset_preamble(void)
{
    cyg_uint32 dev_id;
    asm volatile("mrc\tp15, 0, %0, c0, c0, 0" : "=r" (dev_id));

    //
    // Devices on the I2C bus may be in a hung state. This can occur
    // if a reset happens during an I2C transaction.  Without a power
    // cycle, the device will never release the bus.
    //
    // Some A and B step boards have been modified with a FET
    // connecting GPIO Pin 6 to the I2C clock line.  C step parts have
    // GPIO pins 10 and 11 internally connected to the I2C clock
    // lines.
    //
    // We can manually toggle the GPIO Pin to force the device to
    // release the bus.
    //
    if ((dev_id == IOP331_DEVICE_ID_A_1)  ||
        (dev_id == IOP331_DEVICE_ID_B_0)  ||
        (dev_id == IOP332_DEVICE_ID_A_1)  ||
        (dev_id == IOP332_DEVICE_ID_A_2)  ||
        (dev_id == IOP332_DEVICE_ID_B_0))
    {
        //
        // For boards without the modification, this is not a problem
        // since we are just toggling some lines that aren't connected
        // anywhere.
        //
            
        // Save these registers for restoration later on
        unsigned long orig_gpoe = *GPIO_GPOE;
        unsigned long orig_gpod = *GPIO_GPOD;
        unsigned long orig_ier  = *UART1_IER;
        int i;
            
        // Select the GPIO functionality for the pins that are
        // multiplexed w/ the UART
        *UART1_IER = 0;
            
        // Setup GPIO 6 as an output
        *GPIO_GPOE &=~ (1 << 6);
            
        for (i = 0; i < 32; i++) {
            *GPIO_GPOD = 0;     I2C_BUS_CLEAR_DELAY(0x100);
            *GPIO_GPOD = ~0;    I2C_BUS_CLEAR_DELAY(0x100);
        }

        // Restore these registers
        *GPIO_GPOE = orig_gpoe;
        *GPIO_GPOD = orig_gpod;
        *UART1_IER  = orig_ier;
    } else {
        // Save these registers for restoration later on
        unsigned long orig_gpod = *GPIO_GPOD;
        int i;
            
        for (i = 0; i < 32; i++) {
            *GPIO_GPOD = (1<<10)|(1<<11);     I2C_BUS_CLEAR_DELAY(0x100);
            *GPIO_GPOD = 0;                   I2C_BUS_CLEAR_DELAY(0x100);
        }

        // Restore these registers
        *GPIO_GPOD = orig_gpod;
    }
}

/*------------------------------------------------------------------------*/
// Extra Memory Listing
void hal_platform_memory_info(void)
{
#if defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE) && \
    !defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE_0)
    diag_printf("RAM: %p-%p, 32-bit ECC\n", 
                (void*)SDRAM_32BIT_ECC_BASE, 
                (void*)(SDRAM_32BIT_ECC_BASE + (CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE << 20)));
#endif
}
/*------------------------------------------------------------------------*/
// EOF iop33x_misc.c

