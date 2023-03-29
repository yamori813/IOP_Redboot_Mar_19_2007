//==========================================================================
//
//      iop34x_misc.c
//
//      HAL misc board support code for Intel IOP34X
//      I/O Processors
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#define DEBUG_ECC
#undef  DEBUG_IRQ_HANDLER

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_xscale_iop.h>

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
#include <redboot.h>

// Prototypes
externC void plf_hardware_init(void);
extern int hal_pci_cfg_flags;
void l2_cache_control(cyg_bool enable);
void reset_using_wdt(void);

static cyg_uint32 mcu_ISR(cyg_vector_t vector, cyg_addrword_t data);

#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_IMU
static void boot_complete_doorbell(void);
RedBoot_init(boot_complete_doorbell, RedBoot_INIT_LAST);
#endif

#ifdef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
static void disable_l1_caches(void);
RedBoot_init(disable_l1_caches, RedBoot_INIT_FIRST);
#endif

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
#ifndef CYGSEM_HAL_ARM_IQ8134x_DISABLE_L1
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();
#endif

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
	cyg_uint32 sources2, mask2, sources3, mask3;
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
    INTCTL2_READ(mask2);
    IINTSRC2_READ(sources2);
    INTCTL3_READ(mask3);
    IINTSRC3_READ(sources3);

#ifdef DEBUG_IRQ_HANDLER
    diag_printf("mask0 = 0x%08lx\n", mask0);
    diag_printf("mask1 = 0x%08lx\n", mask1);
    diag_printf("mask2 = 0x%08lx\n", mask2);
    diag_printf("mask3 = 0x%08lx\n", mask3);
    diag_printf("sources0 = 0x%08lx\n", sources0);
    diag_printf("sources1 = 0x%08lx\n", sources1);
    diag_printf("sources2 = 0x%08lx\n", sources2);
    diag_printf("sources3 = 0x%08lx\n", sources3);
#endif
    // just the unmasked ones
    sources0 &= mask0;
    sources1 &= mask1;
	sources2 &= mask2;
	sources3 &= mask3;

    if (sources0) {
	HAL_LSBIT_INDEX( index, sources0 );
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources0 returning: %d\n", index);
#endif
	return index;
    }
    else if (sources1) {
		HAL_LSBIT_INDEX( index, sources1 );
        index += CYGNUM_HAL_ISR1_MIN;
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources1 returning: %d\n", index);
#endif
		return index;
    } 
    else if (sources2) {
		HAL_LSBIT_INDEX( index, sources2 );
		index += CYGNUM_HAL_ISR2_MIN;
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources2 returning: %d\n", index);
#endif
    }
    else if (sources3) {
		HAL_LSBIT_INDEX( index, sources3 );
		index += CYGNUM_HAL_ISR3_MIN;
#ifdef DEBUG_IRQ_HANDLER
        diag_printf("sources3 returning: %d\n", index);
#endif
    }
#ifdef DEBUG_IRQ_HANDLER
    diag_printf("returning: CYGNUM_HAL_INTERRUPT_NONE\n");
#endif
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}


// Scrub_ecc :  Called from the MCU_ISR if an ECC error is encountered
//    We have to scrub the memory location that caused an ECC error
#ifdef CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
static void _scrub_ecc(cyg_uint32 p)
{
    cyg_uint32 iacr;
#ifdef DEBUG_ECC
    void (*dp)(const char *fmt, ...) = diag_printf; 
    dp("In _scrub_ecc(0x%08lx)\n", p);
#endif

    // The following ldr/str pair need to be atomic on the bus. Since
    // the XScale core doesn't support atomic RMW, we have to disable
    // arbitration to prevent other bus masters from taking the internal bus
    // between the the ldr and str.
    iacr = *ARB_IBACR;

#ifdef DEBUG_ECC
    dp("ibacr = 0x%08lx\n", iacr);
#endif

	// Disable internal address arbiters so core1 can have atomic access
	// to the bus and MCU
    *ARB_IBACR = (iacr & IBACR_MASK)        |
                IBACR_CORE0(IBACR_DISABLED) |
                IBACR_CORE1(IBACR_ENABLED)  |
                IBACR_SMBUS(IBACR_ENABLED)  |
                IBACR_ATUE (IBACR_DISABLED) |
                IBACR_ATUX (IBACR_DISABLED) |
                IBACR_ADMA (IBACR_DISABLED) |
                IBACR_TDMA (IBACR_DISABLED) |
                IBACR_DDRMCU(IBACR_DISABLED)|
                IBACR_TPMI (IBACR_DISABLED);

#ifdef DEBUG_ECC
    dp("*ARB_IBACR = 0x%08lx\n", *ARB_IBACR);
#endif

    asm volatile ("mcr  p15,0,r1,c7,c10,4\n");    // drain write buffer
    CPWAIT();

    // Do the ECC clean operation
    asm volatile ("ldrb r4, [%0]\n"
		  "strb r4, [%0]\n" : : "r"(p) : "r4");

    // flush the cache line
    asm volatile ("mcr p15, 0, %0, c7, c10, 1" : : "r"(p));

    // Restore normal internal bus arbitration
    *ARB_IBACR = iacr;

#ifdef DEBUG_ECC
    dp("*ARB_IBACR = 0x%08lx\n", *ARB_IBACR);
#endif
}
#endif // CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC

// Interrupt service routine for the MCU - used mainly for testing
// ECC.  
static cyg_uint32
mcu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
#ifdef CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
    cyg_uint32 eccr_reg;
#endif
    cyg_uint32 mcisr_reg;

#ifdef CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
	// Read current state of ECC register
	eccr_reg = *MCU_DECCR;
#endif

	// and the interrupt status
	mcisr_reg = *MCU_DMCISR;

#ifdef CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
	// Turn off all ecc error reporting
	*MCU_DECCR = 0xc;

#ifdef DEBUG_ECC
	diag_printf("mcu_ISR entry: ECCR = 0x%X, MCISR = 0x%X\n", eccr_reg, mcisr_reg);
#endif

	// Check for ECC Error 0
	if(mcisr_reg & 1) {

#ifdef DEBUG_ECC
		diag_printf("ELOG0 = 0x%X\n", *MCU_DELOG0);
		diag_printf("ECC Error Detected at Address 0x%X\n",*MCU_DEAR0);
#endif

		// Check for single-bit error
		if(!(*MCU_DELOG0 & 0x00000100)) {
			// call ECC restoration function
			_scrub_ecc(CYGARC_VIRTUAL_ADDRESS(*MCU_DEAR0));

			// Clear the MCISR
			*MCU_DMCISR = 1;
		}
		else {
#ifdef DEBUG_ECC
			diag_printf("Multi-bit or nibble error\n");
#endif
		}
	}

	// Check for ECC Error 1
	if(mcisr_reg & 2) {

#ifdef DEBUG_ECC
		diag_printf("ELOG1 = 0x%X\n",*MCU_DELOG1);
		diag_printf("ECC Error Detected at Address 0x%X\n",*MCU_DEAR1);
#endif

		// Check for single-bit error
		if(!(*MCU_DELOG1 & 0x00000100))  {
			// call ECC restoration function
			_scrub_ecc(CYGARC_VIRTUAL_ADDRESS(*MCU_DEAR1));

			// Clear the MCISR
			*MCU_DMCISR = 2;
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
		*MCU_DMCISR = 4;
		diag_printf("Uncorrectable error during RMW\n");
	}

#endif // CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
    
	// Check for Address Region Error
	if(mcisr_reg & 8) {
		// Clear the MCISR
		*MCU_DMCISR = 8;
		diag_printf("Address region error during RMW\n");
	}

	// Check for Parity Error 
	if(mcisr_reg & 0x100) {
		// Clear the MCISR
		*MCU_DMCISR = 0x100;
		diag_printf("MCU Detected Parity error\n");
	}

	// Check for Parity Error N
	if(mcisr_reg & 0x200) {
		// Clear the MCISR
		*MCU_DMCISR = 0x200;
		diag_printf("MCU Detected Parity error N\n");
	}

#ifdef CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
	// Restore ECCR register
	*MCU_DECCR = eccr_reg;
#endif // CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC

#ifdef DEBUG_ECC
	diag_printf("mcu_ISR exit: MCISR = 0x%X\n", *MCU_DMCISR);
#endif

	return CYG_ISR_HANDLED;
}

//
// Interrupt control
//

void
hal_interrupt_mask(int vector)
{
	int mask;
    if ((vector >= CYGNUM_HAL_ISR0_MIN) && (vector <= CYGNUM_HAL_ISR0_MAX)) {
		INTCTL0_READ(mask);
		mask &= ~(1 << (vector - CYGNUM_HAL_ISR0_MIN));
		INTCTL0_WRITE(mask);
		CPWAIT();
    } 
    else if ((vector >= CYGNUM_HAL_ISR1_MIN) && (vector <= CYGNUM_HAL_ISR1_MAX)) {
		INTCTL1_READ(mask);
		mask &= ~(1 << (vector - CYGNUM_HAL_ISR1_MIN));
		INTCTL1_WRITE(mask);
		CPWAIT();
    }
    else if ((vector >= CYGNUM_HAL_ISR2_MIN) && (vector <= CYGNUM_HAL_ISR2_MAX)) {
		INTCTL2_READ(mask);
		mask &= ~(1 << (vector - CYGNUM_HAL_ISR2_MIN));
		INTCTL2_WRITE(mask);
		CPWAIT();
    }
    else if ((vector >= CYGNUM_HAL_ISR3_MIN) && (vector <= CYGNUM_HAL_ISR3_MAX)) {
		INTCTL3_READ(mask);
		mask &= ~(1 << (vector - CYGNUM_HAL_ISR3_MIN));
		INTCTL3_WRITE(mask);
		CPWAIT();
    }
}

void
hal_interrupt_unmask(int vector)
{
	int mask;
	if ((vector >= CYGNUM_HAL_ISR0_MIN) && (vector <= CYGNUM_HAL_ISR0_MAX)) {
		INTCTL0_READ(mask);
		mask |= (1 << (vector - CYGNUM_HAL_ISR0_MIN));
		INTCTL0_WRITE(mask);
		CPWAIT();
	}
	else if ((vector >= CYGNUM_HAL_ISR1_MIN) && (vector <= CYGNUM_HAL_ISR1_MAX)) {
		INTCTL1_READ(mask);
		mask |= (1 << (vector - CYGNUM_HAL_ISR1_MIN));
		INTCTL1_WRITE(mask);
		CPWAIT();
	}
	else if ((vector >= CYGNUM_HAL_ISR2_MIN) && (vector <= CYGNUM_HAL_ISR2_MAX)) {
		INTCTL2_READ(mask);
		mask |= (1 << (vector - CYGNUM_HAL_ISR2_MIN));
		INTCTL2_WRITE(mask);
		CPWAIT();
	}
	else if ((vector >= CYGNUM_HAL_ISR3_MIN) && (vector <= CYGNUM_HAL_ISR3_MAX)) {
		INTCTL3_READ(mask);
		mask |= (1 << (vector - CYGNUM_HAL_ISR3_MIN));
		INTCTL3_WRITE(mask);
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

#define CLOCK_MULTIPLIER 400

void
hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 val;
    cyg_uint32 tmr_period;
    _period = period;

    //tmr_period = period * CLOCK_MULTIPLIER;
	tmr_period = period * GET_IB_SPEED_MHZ();
    
    // disable Timer0
    TMR0_WRITE(0);

    // clear interrupt on Timer0
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
    
    //timer_val /= CLOCK_MULTIPLIER;
	timer_val /=  GET_IB_SPEED_MHZ();
    
    *pvalue = _period - timer_val;
}

// Delay for some usecs.
void
hal_delay_us(cyg_int32 delay)
{
//#define _TICKS_PER_USEC CLOCK_MULTIPLIER
#define _TICKS_PER_USEC GET_IB_SPEED_MHZ();
	cyg_uint32 now, prev, diff, usecs, ticks;
	//cyg_uint32 tmr_period = _period * CLOCK_MULTIPLIER;
	cyg_uint32 tmr_period = _period * GET_IB_SPEED_MHZ();

	ticks = GET_IB_SPEED_MHZ();
	diff = usecs = 0;
	TCR0_READ(prev);

	while ((cyg_uint32)delay > usecs) {
		TCR0_READ(now);

		if (prev < now)
			diff += (prev + (tmr_period - now));
		else
			diff += (prev - now);

		prev = now;

		if (diff >= ticks ) {
			usecs += (diff / ticks);
			diff %= ticks;
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

// Preamble to check for a stuck condition on the I2C bus before
// resetting the ICRx register.  I2C0 is reserved for the transport
// core, so it is not checked.
void hal_platform_i2c_bus_reset_preamble(void)
{
    // Devices on the I2C bus may be in a hung state. This can occur
    // if a reset happens during an I2C transaction.  Without a power
    // cycle, the device will never release the bus.

    // We can manually toggle the IMBCRx[2:1] to force the device to
    // release the bus.
    //
    int i;

	// Try just setting GPOD to 0 and returning - cebruns
	*GPIO_GPOD = 0x0; 
	return;

	// Check I2C1 for a stuck condition (i.e. One or both lines
	// pulled low).
	if ((*I2C_IBMR1 & 0x3) != 0x3) {
    	hal_platform_lowlevel_serial_debug_init(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD);
	    hal_platform_lowlevel_serial_debug_puts("I2C1 locked ");
	    hal_platform_lowlevel_serial_debug_puts("()\r\n");
		*I2C_IMBR1 = 1;
	    for (i = 0; i < 32; i++) {
	    	*I2C_IMBR1 |= 2;    I2C_BUS_CLEAR_DELAY(0x100);
	    	*I2C_IMBR1 &= ~(2); I2C_BUS_CLEAR_DELAY(0x100);
	    }
		*I2C_IMBR1 = 0;
	    // Reset I2C Unit for Bus1
	    *I2C_ICR1 = ICR_I2C_RESET;
	    *I2C_ISR1 = 0x7FF;
	    *I2C_ICR1 = 0;
	}
	// Check I2C2 for a stuck condition (i.e. One or both lines
	// pulled low).  This bus holds the SDRAM SPD, so use those
	// definitions.
	if ((*I2C_SDRAM_IBMR & 0x3) != 0x3) {
    	hal_platform_lowlevel_serial_debug_init(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD);
	    hal_platform_lowlevel_serial_debug_puts("I2C2 locked ");
	    hal_platform_lowlevel_serial_debug_puts("()\r\n");
		*I2C_SDRAM_IMBR = 1;
	    for (i = 0; i < 32; i++) {
	    	*I2C_SDRAM_IMBR |= 2;    I2C_BUS_CLEAR_DELAY(0x100);
	    	*I2C_SDRAM_IMBR &= ~(2); I2C_BUS_CLEAR_DELAY(0x100);
	    }
		*I2C_SDRAM_IMBR = 0;
		// (I2C Bus2 block is reset in xscale_iop.c)
	}
}

/*------------------------------------------------------------------------*/
// Extra Memory Listing
void hal_platform_memory_info(void)
{
#if defined(CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE) && \
    !defined(CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE_0)
    diag_printf("RAM: %p-%p, 32-bit ECC\n", 
                (void*)SDRAM_32BIT_ECC_BASE, 
                (void*)(SDRAM_32BIT_ECC_BASE + (CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE << 20)));
#endif
}
/*------------------------------------------------------------------------*/
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_IMU

// Set timeout to 10mSec
#define DOORBELL_TIMEOUT 1000

// Boot_complete_doorbell : Ring the doorbell of the transport core to tell
// it that we have finished booting our firmware.  Also, wait for the 
// transport core's boot-complete doorbell to know that the system is ready.
static void boot_complete_doorbell(void) {
	cyg_uint32 timeout = 0;

	*IMU_DBAR |= BOOT_COMPLETE_DOORBELL;
	// Wait 10mSec for FW Boot doorbell
	while (((*IMU_DBCR & BOOT_COMPLETE_DOORBELL) == 0) &&
	         (timeout < DOORBELL_TIMEOUT)) {
	   HAL_DELAY_US(10);
	   timeout++;
	}
	if (timeout == DOORBELL_TIMEOUT) { 
		diag_printf (" --- Never received boot_complete_doorbell from Transport Core!\n");
		HEX_DISPLAY_QUICK(D, F);
	}
}
#endif

RedBoot_cmd("l2", 
            "Manage L2 cache", 
            "[ON | OFF]",
            do_l2
    );

RedBoot_cmd("inv_cache", "Invalidate DCache/L2 Range", "<base> [length]",
			do_cache_inv
	);

RedBoot_cmd("flush_cache", "Flush/Invalidate DCache/L2 Range", "<base> [length]",
			do_cache_flush
	);

// Function to control L2 behavior.  Disable/Enable by default, but
// also an option to invalidate (not flush) a range of L2.
void
do_l2(int argc, char *argv[])
{
    unsigned long oldints;
    static int l2_on = 1;
	int temp;
    int base, length;
    bool base_set, length_set;
    struct option_info opts[2];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base, (bool *)&base_set, "Base");

    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "Length");

	if (L2_CACHE_ENABLED) { 
	    if (argc == 2) {
	        if (strcasecmp(argv[1], "on") == 0) {
	            HAL_DISABLE_INTERRUPTS(oldints);
				l2_cache_control(true);
	            HAL_RESTORE_INTERRUPTS(oldints);
				l2_on = 1;
	        } 
	        else if (strcasecmp(argv[1], "off") == 0) {
	            HAL_DISABLE_INTERRUPTS(oldints);
				l2_cache_control(false);  // Disable L2 Caching in PT entries
	            HAL_L2_CACHE_SYNC();  // Cleans and Invalidates all L2 lines
	            HAL_RESTORE_INTERRUPTS(oldints);
				l2_on = 0;
	        }
	        else {
	            diag_printf("Invalid cache mode: %s\n", argv[1]);
	        }
	    } else if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
	    	return;
	    }
		else  {
			if (!base_set)
				diag_printf("L2 Cache : %s\n", l2_on?"On":"Off");
			else  {
				if (l2_on == 1) { 
					if (!length_set)
						length = 32;
					if (length > (512*1024)) { 
						diag_printf ("Length can't be > 512k - setting to 512k\n");
						length = (512*1024);
					}
		            HAL_DISABLE_INTERRUPTS(oldints);
					diag_printf ("Invalidating: 0x%08x to 0x%08x\n", \
							base, base+length-1);
					HAL_L2_CACHE_INVALIDATE(base, length);
	            	HAL_RESTORE_INTERRUPTS(oldints); 
				}
				else  {
					diag_printf ("L2 disabled - can't invalidate\n");
				}
			}
	    }
	}
	else  {
		diag_printf ("L2 Must be enabled by RedBoot before MMU enabled...\n");
		diag_printf ("Change CDL option 'CYGSEM_HAL_ENABLE_L2_CACHE' to 1 and rebuild to enable L2\n");
	}
}

// Function to invalidate L1 and L2 Data Cache.  
void
do_cache_inv(int argc, char *argv[])  {
    unsigned long oldints;
    int base, length;
    bool base_set, length_set;
    struct option_info opts[2];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base, (bool *)&base_set, "Base");

    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "Length");
    
    if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
		return;
    }

	if (!base_set) { 
		diag_printf("Need to have a base to invalidate\n");
		return;
	}
	if (!length_set) 
		length = 32;
	HAL_DISABLE_INTERRUPTS(oldints);
	diag_printf ("Invalidating: 0x%08x to 0x%08x\n", \
		base, base+length-1);
	HAL_DCACHE_INVALIDATE(base, length);
	HAL_L2_CACHE_INVALIDATE(base, length);
	HAL_RESTORE_INTERRUPTS(oldints); 
}


// Function to Flush L1 Data Cache and L2 Cache into SDRAM.  
void
do_cache_flush(int argc, char *argv[])  {
    unsigned long oldints;
    int base, length;
    bool base_set, length_set;
    struct option_info opts[2];

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base, (bool *)&base_set, "Base");

    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "Length");
    
    if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
		return;
    }

	if (!base_set) { 
		diag_printf("Need to have a base to Flush\n");
		return;
	}
	if (!length_set) 
		length = 32;
	HAL_DISABLE_INTERRUPTS(oldints);
	diag_printf ("Flushing/Invalidating Data in L1 and L2 from: 0x%08x to 0x%08x\n",base, base+length-1);
	HAL_DCACHE_FLUSH (base, length);
	HAL_L2_CACHE_FLUSH (base, length);
	HAL_RESTORE_INTERRUPTS(oldints); 
}

// L2 Cache cannot simply be disabled once enabled.  Mod'ing PT entries 
// to Outer-uncacheable is the only way to "disable".  
// Inner is L1, Outer is L2.  PT Entry fields for Inner/Outer Cacheable:
// TEX    == 1x1b   : Outer Cacheable
// TEX    == 1x0b   : Outer Uncacheable
// TEX CB == 0x1 11b : Inner WB/RA, Outer WB/WA
// TEX CB == 0x1 00b : Inner/Outer Uncacheable, but writes may coalesce
void l2_cache_control(cyg_bool enable) {
	cyg_uint32 *pt_entry;
	cyg_uint32 ttb_base_va, ttb_base_pa;
	cyg_uint32 *second_level;
	cyg_uint32 counter;
	ttb_base_pa = (_Read_Translation_Table_Base);
	ttb_base_va = CYGARC_VIRTUAL_ADDRESS(ttb_base_pa);
	pt_entry = (cyg_uint32*)((cyg_uint32)ttb_base_va & 0xFFFFC000);

	//	diag_printf ("TTB_Base: 0x%08x, First PT_Entry: 0x%08x\n", ttb_base, *pt_entry);
	if (enable == false) { 
		// Don't allow table walks to load descriptors into L2
		ttb_base_pa &= ~(OUTER_CACHEABLE_TABLE_WALKS);
		//		diag_printf ("Disabling OC Table Walks: 0x%08x\n", ttb_base);
		_Write_Translation_Table_Base(ttb_base_pa);
		HAL_L2_CACHE_SYNC();      
		DATA_MEMORY_BARRIER();
	}

	// Go through all 4096 PT entries and either enable or disable L2 cache-ability
	for (counter = 0; counter < 4096; counter++, pt_entry++) {
		// If we want to "disable" L2 caching, we clear the X-bit in TEX field
		// where appropriate.
		if (enable == false) {
			if ((*pt_entry & 0x3) == 0x2) {/* Section Descriptor */
				// if TEX == 1x1b, clear X-bit
				if (((*pt_entry >> TEX_OFFSET_SECTION) & TEX_OUTER_CACHEABLE) ==\
					TEX_OUTER_CACHEABLE)  									    
					*pt_entry &= ~(X_BIT_SECTION);
				// else if TEXCB == 0xx11b, clear the X-bit and set the T-bit
				else if ((((*pt_entry >> CB_OFFSET) & 0x3) == 0x3) &&   \
					(((*pt_entry >> TEX_OFFSET_SECTION) & 0x4) != 0x4)) {
					*pt_entry &= ~(X_BIT_SECTION);
					*pt_entry |=  (T_BIT_SECTION);
				}
			}
			else {
				second_level = (unsigned int*)(*pt_entry & 0xFFFFFC00);
				if ((*second_level & 0x3) == 0x3) {  /* Tiny Page */
					// If TEX == 1x1b, clear X-bit
					if (((*second_level >> TEX_OFFSET_TINY) & TEX_OUTER_CACHEABLE) == \
						TEX_OUTER_CACHEABLE)
						*second_level &= ~(X_BIT_TINY);
					// else if TEXCB == 0xx11b, clear X-bit and set the T-bit
					else if ((((*second_level >> CB_OFFSET) & 0x3) == 0x3) &&   \
						(((*second_level >> TEX_OFFSET_TINY) & 0x4) != 0x4)) {
						*second_level &= ~(X_BIT_TINY);
						*second_level |=  (T_BIT_TINY);
					}
				}
				else if ((*second_level & 0x3) == 1) { /* Large Page */
					// If TEX == 1x1b, clear X-bit
					if (((*second_level >> TEX_OFFSET_LARGE) & TEX_OUTER_CACHEABLE) == \
						TEX_OUTER_CACHEABLE)
						*second_level &= ~(X_BIT_LARGE);
					// else if TEXCB == 0xx11b, clear X-bit and set the T-bit
					else if ((((*second_level >> CB_OFFSET) & 0x3) == 0x3) &&   \
						(((*second_level >> TEX_OFFSET_LARGE) & 0x4) != 0x4)) {
						*second_level &= ~(X_BIT_LARGE);
						*second_level |=  (T_BIT_LARGE);
					}
				}
			}
		}
		// Else we're enabling the L2, so set TEX field as appropriate
		// Only need to look for TEXCB == 1x011b and change to 1x111b
		else  {
			if ((*pt_entry & 0x3) == 0x2) {/* Section Descriptor */
				// if TEXCB == 1x011b, Set the X-bit
				if ((((*pt_entry >> CB_OFFSET) & 0x3) == 0x3) &&   \
					(((*pt_entry >> TEX_OFFSET_SECTION) & 0x4) == 0x4)) {
					*pt_entry |=  (X_BIT_SECTION);
				}
			}
			else {
				second_level = (unsigned int*)(*pt_entry & 0xFFFFFC00);
				if ((*second_level & 0x3) == 0x3) {  /* Tiny Page */
					// if TEXCB == 1x011b, Set the X-bit
					if ((((*second_level >> CB_OFFSET) & 0x3) == 0x3) &&   \
						(((*second_level >> TEX_OFFSET_TINY) & 0x4) == 0x4)) {
						*second_level |=  (X_BIT_TINY);
					}
				}
				else if ((*second_level & 0x3) == 1) { /* Large Page */
					// if TEXCB == 1x011b, Set the X-bit
					if ((((*second_level >> CB_OFFSET) & 0x3) == 0x3) &&   \
						(((*second_level >> TEX_OFFSET_LARGE) & 0x4) == 0x4)) {
						*second_level |=  (X_BIT_LARGE);
					}
				}
			}
		}
	}
	HAL_DCACHE_SYNC();
	HAL_DCACHE_INVALIDATE_ALL();  // Also flushes I+D TLBs
	DATA_WRITE_BARRIER();
	if (enable == false)
		HAL_L2_CACHE_SYNC();      // Make sure any modified PT Entries get into SDRAM
	else  {
		ttb_base_pa |= OUTER_CACHEABLE_TABLE_WALKS;
		//		diag_printf ("Enabing OC in TTB_Base: 0x%08x\n", ttb_base);
		_Write_Translation_Table_Base(ttb_base_pa);
	}
}

// get_fac_semaphores
// Checks WriteProtection on the Flash via a SW mechanism.  In 8134x we have TSR registers
// which are defined as semaphores to control access to the Flash when running with more than
// one core.  The macro GET_FAC_SEMAPHORES will be called an will only return if both
// semaphores are obtained.  This means we will have access to the Flash until we release
// the semaphores.
// Changed the function to use the macro, because it handles single core parts - ajbrooks
void get_fac_semaphores(void *addr, void* len) {
	GET_FAC_SEMAPHORES();
/*	volatile cyg_uint8  *fac0_p = IMU_TSR_FAC0;								
	volatile cyg_uint8  *fac1_p = IMU_TSR_FAC1;								
	volatile cyg_uint8  fac0, fac1;
	cyg_uint32 timeout, coreid;                                                     
	// Determine which core we will be waiting on to release the FACx
	CIDR_READ(coreid);                                                      
	if (!coreid)                                                            
		coreid = 2;            
	do { 
		// Create different timeouts depending on the coreid                                             
		timeout = coreid * 1000000;
		do {																   	
			fac0 = *fac0_p;    													
			fac1 = *fac1_p;													   	
		} while (((fac0 == coreid) || (fac1 == coreid)) && (--timeout != 0));
		// Check if we bailed because of a timeout.
		// Release FACs and try again if so
		if (!timeout) {
			diag_printf ("Other core must have semaphore...trying again\n"); 
			RELEASE_FAC_SEMAPHORES();
		}
	} while (!timeout);
*/
}

// release_fac_semaphores
// Releases WriteProtection on the Flash via a SW mechanism.
void release_fac_semaphores(void *addr, void *len) {
	RELEASE_FAC_SEMAPHORES();
}


// reset_using_wdt : Cause an Internal Bus Reset by allowing the WDT to expire
//                   by not ever petting the dog.  This will reset BOTH CORES!
void reset_using_wdt(void)  {
    cyg_int32 ctrl, tmp;                                                  

	//Setup WDT to start counting from 0xFFFF_FFFF
	WDTCR_WRITE(0x1E1E1E1E);
	WDTCR_WRITE(0xE1E1E1E1);
	//Set IB Reset Bit and Test Mode bit.
	//Test Mode Bit allows us to modify the WDTCR to a smaller value (for a quicker reset)
	WDTSR_WRITE(0x80000001);
	WDTCR_WRITE(0x1000);											   
	// Wait for IB reset to take place
	for (;;);															   
}

// Prior to L1 cache parity issue being resolved, disable the L1 Caches
static void disable_l1_caches(void) {
	HAL_DCACHE_SYNC();
	HAL_DCACHE_DISABLE();
	HAL_ICACHE_SYNC();
	HAL_ICACHE_DISABLE();
}

// EOF iop34x_misc.c

