#ifndef CYGONCE_HAL_VAR_INTS_H
#define CYGONCE_HAL_VAR_INTS_H
//==========================================================================
//
//      hal_var_ints.h
//
//      HAL Interrupt and clock support
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
// Date:         2001-12-03
// Purpose:      Define Interrupt support
// Description:  The interrupt details for XScale CPUs are defined here.
// Usage:
//		 #include <pkgconf/system.h>
//		 #include CYGBLD_HAL_VARIANT_H
//               #include CYGBLD_HAL_VAR_INTS_H
//
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_bulverde.h>         // registers

#define CYGNUM_HAL_INTERRUPT_NONE         -1
#define CYGNUM_HAL_INTERRUPT_SSP3          0
#define CYGNUM_HAL_INTERRUPT_MSL           1
#define CYGNUM_HAL_INTERRUPT_USB_HOST2     2
#define CYGNUM_HAL_INTERRUPT_USB_HOST1     3
#define CYGNUM_HAL_INTERRUPT_KEYPAD        4
#define CYGNUM_HAL_INTERRUPT_MEMSTICK      5
#define CYGNUM_HAL_INTERRUPT_POWER_I2C     6
#define CYGNUM_HAL_INTERRUPT_TIMERX        7
#define CYGNUM_HAL_INTERRUPT_GPIO0         8
#define CYGNUM_HAL_INTERRUPT_GPIO1         9
#define CYGNUM_HAL_INTERRUPT_GPIO2_120     10
#define CYGNUM_HAL_INTERRUPT_USB_CLIENT    11
#define CYGNUM_HAL_INTERRUPT_CORE_PMU      12
#define CYGNUM_HAL_INTERRUPT_I2S           13
#define CYGNUM_HAL_INTERRUPT_AC97          14
#define CYGNUM_HAL_INTERRUPT_USIM          15
#define CYGNUM_HAL_INTERRUPT_SSP2          16
#define CYGNUM_HAL_INTERRUPT_LCD           17
#define CYGNUM_HAL_INTERRUPT_I2C           18
#define CYGNUM_HAL_INTERRUPT_ICP           19
#define CYGNUM_HAL_INTERRUPT_STUART        20
#define CYGNUM_HAL_INTERRUPT_BTUART        21
#define CYGNUM_HAL_INTERRUPT_FFUART        22
#define CYGNUM_HAL_INTERRUPT_FLASH         23
#define CYGNUM_HAL_INTERRUPT_SSP1          24
#define CYGNUM_HAL_INTERRUPT_DMA           25
#define CYGNUM_HAL_INTERRUPT_TIMER0        26
#define CYGNUM_HAL_INTERRUPT_TIMER1        27
#define CYGNUM_HAL_INTERRUPT_TIMER2        28
#define CYGNUM_HAL_INTERRUPT_TIMER3        29
#define CYGNUM_HAL_INTERRUPT_HZ            30
#define CYGNUM_HAL_INTERRUPT_ALARM         31

// GPIO bits 120..2 can generate interrupts as well, but they all
// end up clumped into interrupt signal #10.  Using the macros
// below allow for handling of these separately.
#define CYGNUM_HAL_INTERRUPT_GPIO_BASE     32
#define CYGNUM_HAL_INTERRUPT_GPIO(x)       (CYGNUM_HAL_INTERRUPT_GPIO_BASE + x)
#define CYGNUM_HAL_INTERRUPT_GPIO2         CYGNUM_HAL_INTERRUPT_GPIO(2)
#define CYGNUM_HAL_INTERRUPT_GPIO120       CYGNUM_HAL_INTERRUPT_GPIO(120)

#define CYGNUM_HAL_VAR_ISR_MAX  CYGNUM_HAL_INTERRUPT_GPIO120

#define CYGNUM_HAL_ISR_MIN      0
#define CYGNUM_HAL_ISR_MAX      CYGNUM_HAL_VAR_ISR_MAX

#define CYGNUM_HAL_ISR_COUNT    (CYGNUM_HAL_ISR_MAX+1)

#define CYGNUM_HAL_INTERRUPT_RTC  CYGNUM_HAL_INTERRUPT_TIMER0

// ------------------------------------------------------------------------
// Dynamically set the timer interrupt rate.
// Not for application use at all.

externC void
hal_clock_reinitialize(          int *pfreq,    /* inout */
                        unsigned int *pperiod,  /* inout */
                        unsigned int old_hz );  /* in */

#define HAL_CLOCK_REINITIALIZE( _freq, _period, _old_hz ) \
        hal_clock_reinitialize( &_freq, &_period, _old_hz )

//----------------------------------------------------------------------------
// Reset. (Needs to be defined by platform code)
// #define HAL_PLATFORM_RESET()

// Fallback (never really used)
#define HAL_PLATFORM_RESET_ENTRY 0x00000000

#endif // CYGONCE_HAL_VAR_INTS_H
// EOF hal_var_ints.h
