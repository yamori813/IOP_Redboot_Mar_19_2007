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
// Author(s):      curt.e.bruns@intel.com
// Original data:  msalter
// Contributors:   drew.moseley@intel.com
// Date:           2004-12-10
// Purpose:        Define Interrupt support
// Description:    The interrupt details for XScale CPUs are defined here.
// Usage:
//		   #include <pkgconf/system.h>
//		   #include CYGBLD_HAL_VARIANT_H
//                 #include CYGBLD_HAL_VAR_INTS_H
//
//                 ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_iop34x.h>         // registers
#include <cyg/hal/hal_cache.h>
//
// Values for the vector argument of cyg_drv_interrupt_create() and
// other interrupt related interfaces.
//
#define CYGNUM_HAL_INTERRUPT_NONE         -1
#define CYGNUM_HAL_INTERRUPT_ADMA0_EOT     0
#define CYGNUM_HAL_INTERRUPT_ADMA0_EOC     1
#define CYGNUM_HAL_INTERRUPT_ADMA1_EOT     2
#define CYGNUM_HAL_INTERRUPT_ADMA1_EOC     3
#define CYGNUM_HAL_INTERRUPT_ADMA2_EOT     4
#define CYGNUM_HAL_INTERRUPT_ADMA2_EOC     5
#define CYGNUM_HAL_INTERRUPT_WDT           6
#define CYGNUM_HAL_INTERRUPT_RSVD_0_7      7
#define CYGNUM_HAL_INTERRUPT_TIMER0        8
#define CYGNUM_HAL_INTERRUPT_TIMER1        9
#define CYGNUM_HAL_INTERRUPT_I2C_0         10
#define CYGNUM_HAL_INTERRUPT_I2C_1         11
#define CYGNUM_HAL_INTERRUPT_MU            12
#define CYGNUM_HAL_INTERRUPT_MU_IN_POST_Q  13
#define CYGNUM_HAL_INTERRUPT_ATUE_INBOUND  14
#define CYGNUM_HAL_INTERRUPT_ATU_BIST      15
#define CYGNUM_HAL_INTERRUPT_PMON          16
#define CYGNUM_HAL_INTERRUPT_CORE_PMU      17
#define CYGNUM_HAL_INTERRUPT_CORE_CACHE    18
#define CYGNUM_HAL_INTERRUPT_RSVD_0_19     19
#define CYGNUM_HAL_INTERRUPT_RSVD_0_20     20
#define CYGNUM_HAL_INTERRUPT_RSVD_0_21     21
#define CYGNUM_HAL_INTERRUPT_RSVD_0_22     22
#define CYGNUM_HAL_INTERRUPT_RSVD_0_23     23
#define CYGNUM_HAL_INTERRUPT_XINT0         24
#define CYGNUM_HAL_INTERRUPT_XINT1         25
#define CYGNUM_HAL_INTERRUPT_XINT2         26
#define CYGNUM_HAL_INTERRUPT_XINT3         27
#define CYGNUM_HAL_INTERRUPT_XINT4         28
#define CYGNUM_HAL_INTERRUPT_XINT5         29
#define CYGNUM_HAL_INTERRUPT_XINT6         30
#define CYGNUM_HAL_INTERRUPT_XINT7         31
// INTSRC1
#define CYGNUM_HAL_INTERRUPT_XINT8         32
#define CYGNUM_HAL_INTERRUPT_XINT9         33
#define CYGNUM_HAL_INTERRUPT_XINT10        34
#define CYGNUM_HAL_INTERRUPT_XINT11        35
#define CYGNUM_HAL_INTERRUPT_XINT12        36
#define CYGNUM_HAL_INTERRUPT_XINT13        37
#define CYGNUM_HAL_INTERRUPT_XINT14        38
#define CYGNUM_HAL_INTERRUPT_XINT15        39
#define CYGNUM_HAL_INTERRUPT_RSVD_1_8      40
#define CYGNUM_HAL_INTERRUPT_RSVD_1_9      41
#define CYGNUM_HAL_INTERRUPT_RSVD_1_10     42
#define CYGNUM_HAL_INTERRUPT_RSVD_1_11     43
#define CYGNUM_HAL_INTERRUPT_RSVD_1_12     44
#define CYGNUM_HAL_INTERRUPT_RSVD_1_13     45
#define CYGNUM_HAL_INTERRUPT_RSVD_1_14     46
#define CYGNUM_HAL_INTERRUPT_RSVD_1_15     47
#define CYGNUM_HAL_INTERRUPT_RSVD_1_16     48
#define CYGNUM_HAL_INTERRUPT_RSVD_1_17     49
#define CYGNUM_HAL_INTERRUPT_RSVD_1_18     50
#define CYGNUM_HAL_INTERRUPT_UART0         51
#define CYGNUM_HAL_INTERRUPT_UART1         52
#define CYGNUM_HAL_INTERRUPT_PBUS_UNIT     53
#define CYGNUM_HAL_INTERRUPT_ATUXCR_WRITE  54
#define CYGNUM_HAL_INTERRUPT_ATUX_ERR      55
#define CYGNUM_HAL_INTERRUPT_MCU_ERR       56
#define CYGNUM_HAL_INTERRUPT_ADMA0_ERR     57
#define CYGNUM_HAL_INTERRUPT_ADMA1_ERR     58
#define CYGNUM_HAL_INTERRUPT_ADMA2_ERR     59
#define CYGNUM_HAL_INTERRUPT_RSVD_1_28     60
#define CYGNUM_HAL_INTERRUPT_RSVD_1_29     61
#define CYGNUM_HAL_INTERRUPT_MU_ERR        62
#define CYGNUM_HAL_INTERRUPT_RSVD_1_31     63
// INTSRC2
#define CYGNUM_HAL_INTERRUPT_IPU           64
#define CYGNUM_HAL_INTERRUPT_TPMI0         65
#define CYGNUM_HAL_INTERRUPT_RSVD_2_2      66
#define CYGNUM_HAL_INTERRUPT_TMPI1         67
#define CYGNUM_HAL_INTERRUPT_RSVD_2_4      68
#define CYGNUM_HAL_INTERRUPT_TPMI2         69
#define CYGNUM_HAL_INTERRUPT_RSVD_2_6      70
#define CYGNUM_HAL_INTERRUPT_TPMI3         71
#define CYGNUM_HAL_INTERRUPT_RSVD_2_8      72
#define CYGNUM_HAL_INTERRUPT_RSVD_2_9      73
#define CYGNUM_HAL_INTERRUPT_TDMA0         74
#define CYGNUM_HAL_INTERRUPT_TDMA0_ERR     75
#define CYGNUM_HAL_INTERRUPT_TDMA1         76
#define CYGNUM_HAL_INTERRUPT_TDMA1_ERR     77
#define CYGNUM_HAL_INTERRUPT_FSENG0        78
#define CYGNUM_HAL_INTERRUPT_FSENG0_ERR    79
#define CYGNUM_HAL_INTERRUPT_FSENG1        80
#define CYGNUM_HAL_INTERRUPT_FSENG1_ERR    81
#define CYGNUM_HAL_INTERRUPT_FSENG2        82
#define CYGNUM_HAL_INTERRUPT_FSENG2_ERR    83
#define CYGNUM_HAL_INTERRUPT_FSENG3        84
#define CYGNUM_HAL_INTERRUPT_FSENG3_ERR    85
#define CYGNUM_HAL_INTERRUPT_FSENG4        86
#define CYGNUM_HAL_INTERRUPT_FSENG4_ERR    87
#define CYGNUM_HAL_INTERRUPT_FSENG5        88
#define CYGNUM_HAL_INTERRUPT_FSENG5_ERR    89
#define CYGNUM_HAL_INTERRUPT_FSENG6        90
#define CYGNUM_HAL_INTERRUPT_FSENG6_ERR    91
#define CYGNUM_HAL_INTERRUPT_FSENG7        92
#define CYGNUM_HAL_INTERRUPT_FSENG7_ERR    93
#define CYGNUM_HAL_INTERRUPT_SIB_ERR       94
#define CYGNUM_HAL_INTERRUPT_SRAM_ERR      95
// INTSRC3
#define CYGNUM_HAL_INTERRUPT_I2C_2         96
#define CYGNUM_HAL_INTERRUPT_ATUE_BIST     97
#define CYGNUM_HAL_INTERRUPT_ATUECR_WRITE  98
#define CYGNUM_HAL_INTERRUPT_ATUE_ERR      99
#define CYGNUM_HAL_INTERRUPT_IMU           100
#define CYGNUM_HAL_INTERRUPT_RSVD_3_5      101
#define CYGNUM_HAL_INTERRUPT_RSVD_3_6      102
#define CYGNUM_HAL_INTERRUPT_TPMI0_OUT     103
#define CYGNUM_HAL_INTERRUPT_TPMI1_OUT     104
#define CYGNUM_HAL_INTERRUPT_TPMI2_OUT     105
#define CYGNUM_HAL_INTERRUPT_TPMI3_OUT     106
#define CYGNUM_HAL_INTERRUPT_ATUE_MSG_A    107
#define CYGNUM_HAL_INTERRUPT_ATUE_MSG_B    108
#define CYGNUM_HAL_INTERRUPT_ATUE_MSG_C    109
#define CYGNUM_HAL_INTERRUPT_ATUE_MSG_D    110
#define CYGNUM_HAL_INTERRUPT_MU_MSIX_WR    111
#define CYGNUM_HAL_INTERRUPT_TPMI_MSIX_WR  112
#define CYGNUM_HAL_INTERRUPT_MSI_INBOUND   113
#define CYGNUM_HAL_INTERRUPT_RSVD_3_18     114
#define CYGNUM_HAL_INTERRUPT_RSVD_3_19     115
#define CYGNUM_HAL_INTERRUPT_RSVD_3_20     116
#define CYGNUM_HAL_INTERRUPT_RSVD_3_21     117
#define CYGNUM_HAL_INTERRUPT_RSVD_3_22     118
#define CYGNUM_HAL_INTERRUPT_RSVD_3_23     119
#define CYGNUM_HAL_INTERRUPT_RSVD_3_24     120
#define CYGNUM_HAL_INTERRUPT_RSVD_3_25     121
#define CYGNUM_HAL_INTERRUPT_RSVD_3_26     122
#define CYGNUM_HAL_INTERRUPT_RSVD_3_27     123
#define CYGNUM_HAL_INTERRUPT_RSVD_3_28     124
#define CYGNUM_HAL_INTERRUPT_RSVD_3_29     125
#define CYGNUM_HAL_INTERRUPT_RSVD_3_30     126
#define CYGNUM_HAL_INTERRUPT_HPI           127

#define CYGNUM_HAL_VAR_ISR_MAX  127

#define CYGNUM_HAL_ISR_MIN      0
#define CYGNUM_HAL_ISR0_MIN     CYGNUM_HAL_ISR_MIN
#define CYGNUM_HAL_ISR0_MAX     31
#define CYGNUM_HAL_ISR1_MIN     (CYGNUM_HAL_ISR0_MAX+1)
#define CYGNUM_HAL_ISR1_MAX     63
#define CYGNUM_HAL_ISR2_MIN     (CYGNUM_HAL_ISR1_MAX+1)
#define CYGNUM_HAL_ISR2_MAX     95
#define CYGNUM_HAL_ISR3_MIN     (CYGNUM_HAL_ISR2_MAX+1)
#define CYGNUM_HAL_ISR3_MAX     127
#define CYGNUM_HAL_ISR_MAX      CYGNUM_HAL_ISR3_MAX

#define CYGNUM_HAL_ISR_COUNT    (CYGNUM_HAL_ISR_MAX+1)

#define CYGNUM_HAL_INTERRUPT_RTC  CYGNUM_HAL_INTERRUPT_TIMER0
#define CYGNUM_HAL_INTERRUPT_UART CYGNUM_HAL_INTERRUPT_UART0

#endif // CYGONCE_HAL_VAR_INTS_H

extern void reset_using_wdt(void);

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
// Reset. 
#define HAL_PLATFORM_RESET() reset_using_wdt()

// Fallback (never really used)
#define HAL_PLATFORM_RESET_ENTRY 0x00000000

// EOF hal_var_ints.h
