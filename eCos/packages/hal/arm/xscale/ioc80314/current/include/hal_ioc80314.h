/*=============================================================================
//
//      hal_ioc80314.h
//
//      IOC80314 I/O Coprocessor support (register layout, etc)
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
// Author(s):    msalter
// Contributors: msalter, dkranak, cebruns
// Date:         2001-12-03
// Purpose:
// Description:  Verde I/O Processor support.
// Usage:        #include <cyg/hal/hal_ioc80314.h>
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_IOC80314_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_IOC80314_H

#include <pkgconf/system.h>
#include <cyg/hal/hal_xscale.h>
#include <cyg/hal/hal_ioc80314_basic_types.h>
#include <cyg/hal/hal_ioc80314_ciu.h>
#include <cyg/hal/hal_ioc80314_dram_ctlr.h>
#include <cyg/hal/hal_ioc80314_hlp.h>
#include <cyg/hal/hal_ioc80314_i2c.h>
#include <cyg/hal/hal_ioc80314_mpic.h>
#include <cyg/hal/hal_ioc80314_dma.h>

// --------------------------------------------------------------------------
// remove after fixing ISR handlers
#ifndef __ASSEMBLER__
#define INTCTL_READ(val)
#define _INTCTL_WRITE(val)
#define INTSTR_READ(val)
#define _INTSTR_WRITE(val)
#define IINTSRC_READ(val)
#define FINTSRC_READ(val)

static inline void INTCTL_WRITE(cyg_uint32 val) { _INTCTL_WRITE(val); }
static inline void INTSTR_WRITE(cyg_uint32 val) { _INTSTR_WRITE(val); }
void	dual_proc_prep(void);

// For full read/write access, you have to use coprocessor insns.
#define TMR0_READ(val)
#define _TMR0_WRITE(val)
#define TMR1_READ(val)
#define _TMR1_WRITE(val)
#define TCR0_READ(val)
#define _TCR0_WRITE(val)
#define TCR1_READ(val)
#define _TCR1_WRITE(val)
#define TRR0_READ(val)
#define _TRR0_WRITE(val)
#define TRR1_READ(val)
#define _TRR1_WRITE(val)
#define TISR_READ(val)
#define _TISR_WRITE(val)
#define _WDTCR_READ(val)
#define _WDTCR_WRITE(val)

static inline void TMR0_WRITE(cyg_uint32 val) { _TMR0_WRITE(val); }
static inline void TMR1_WRITE(cyg_uint32 val) { _TMR1_WRITE(val); }
static inline void TCR0_WRITE(cyg_uint32 val) { _TCR0_WRITE(val); }
static inline void TCR1_WRITE(cyg_uint32 val) { _TCR1_WRITE(val); }
static inline void TRR0_WRITE(cyg_uint32 val) { _TRR0_WRITE(val); }
static inline void TRR1_WRITE(cyg_uint32 val) { _TRR1_WRITE(val); }
static inline void TISR_WRITE(cyg_uint32 val) { _TISR_WRITE(val); }
#endif   /* not assembler */

// --------------------------------------------------------------------------
#define HAL_80314_CSR_BASE             0xFFFE0000    //Control register base address
#define HAL_80314_CIU_CFG_BAR          (HAL_80314_CSR_BASE + HAL_80314_CIU_BASE + TS_CIU_CFG_BAR)

#define HAL_80314_HLP_BASE             0x00000000    //Host Local  Port
#define HAL_80314_PCI1_BASE            0x00001000    //
#define HAL_80314_PCI2_BASE            0x00002000    //
#define HAL_80314_CIU_BASE             0x00003000    // XScale Gasket
#define HAL_80314_SDRAM_BASE           0x00004000    //
#define HAL_80314_DMA_BASE             0x00005000    //
#define HAL_80314_GIGE_BASE            0x00006000    //
#define HAL_80314_I2C_BASE             0x00007000    //
#define HAL_80314_MPIC_BASE            0x00007400    //  Interrupt contoler
#define HAL_80314_UART1_BASE           0x00007800    //
#define HAL_80314_GPIO_BASE            0x00007AA0    //
#define HAL_80314_UART2_BASE           0x00007C00    //


//----   80314 Device Address Base Vaules
#define HAL_IOC80314_HLP_BASE       (HAL_80314_HLP_BASE     + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_PCI1_BASE      (HAL_80314_PCI1_BASE    + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_PCI2_BASE      (HAL_80314_PCI2_BASE    + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_CIU_BASE       (HAL_80314_CIU_BASE     + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_SDRAM_BASE     (HAL_80314_SDRAM_BASE   + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_DMA_BASE       (HAL_80314_DMA_BASE     + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_GIGE_BASE      (HAL_80314_GIGE_BASE    + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_I2C_BASE       (HAL_80314_I2C_BASE     + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_MPIC_BASE      (HAL_80314_MPIC_BASE    + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_UART1_BASE     (HAL_80314_UART1_BASE   + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_GPIO_BASE      (HAL_80314_GPIO_BASE    + HAL_IOC80314_CSR_BASE)
#define HAL_IOC80314_UART2_BASE     (HAL_80314_UART2_BASE   + HAL_IOC80314_CSR_BASE)

#define GPIO_DATA_OUT_ADDR         ( 0x1 )     //Byte address
#define GPIO_DATA_IN_ADDR          ( 0x3 )     //Byte address
#define GPIO_CNTRL_DIRECTION_ADDR  ( 0x7 )     //Byte address
#define GPIO_CNTRL_EN_ADDR         ( 0x4 )     //Byte address

//--- Global Timers
#define GTM0    0
#define GTM1    1
#define GTM2    2
#define GTM3    3

// ------------------------------------------------------------------------
// Assembler macro's used for boot initialization
#ifndef __ASSEMBLER__
#define GTCCR_READ(_x_, _tvalue_)   (_tvalue_ = (*((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x30))) & TS_MPIC_GTCCR_COUNT)
#define GTCCR_WRITE(_x_, _tvalue_)  (*((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x30)) = _tvalue_)
#define GTBCR_READ(_x_, _tvalue_)   (_tvalue_ = *((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x34 )))
#define GTBCR_WRITE(_x_, _tvalue_)  (*((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x34)) = _tvalue_)
#define GTVPR_READ(_x_, _tvalue_)   (_tvalue_ = *((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x38 )))
#define GTVPR_WRITE(_x_, _tvalue_)  (*((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x38)) = _tvalue_)
#define GTDR_READ(_x_, _tvalue_)    (_tvalue_ = *((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x3C(TS_MPIC_GTDR(_x_)))))
#define GTDR_WRITE(_x_, _tvalue_)   (*((volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + 0x3C)) = _tvalue_)
#define GTAD(_y_)                   (  (volatile unsigned int*)(HAL_IOC80314_MPIC_BASE  + TS_MPIC_GTBCR(_y_)))

#ifdef PKG_HAL_HAS_MULT_CPU
__externC  volatile long        _curProc;
__externC  volatile cyg_uint32* ptr_ciu_base;
#endif
__externC  volatile cyg_uint32  currentTimerValue;
#ifdef CYGPKG_IO_PCI
__externC  void cyg_hal_ioc_pci_init(void);
#endif

/* SFN Switch Fabric Network Port numbers */
typedef enum {
               PORT_PBI,             // 0  Peripheral Bus Interface
               PORT_PCI1,            // 1
               PORT_PCI2,            // 2
               PORT_XSCALE,          // 3  CIU and SRAM
               PORT_SDRAM,           // 4
               PORT_DMA,             // 5
               PORT_GIGE             // 6
             } SF_PORT_NUM;
#define PORT_SRAM   PORT_XSCALE

#define CIU_LUT_ENTRIES  32

#endif // not __ASSEMBLER__

#ifdef PKG_HAL_HAS_MULT_CPU
#define HAL_FLASH_WRITE_ON()                                                                              \
          CYG_MACRO_START                                                                                 \
          currentTimerValue = ptr_ciu_base[((_curProc == 0) ? TS_CIU_TIMER1/4 : TS_CIU_TIMER0/4)] ;  \
          ptr_ciu_base[((_curProc == 0) ? TS_CIU_TIMER1/4 : TS_CIU_TIMER0/4)]  = 0;                  \
	      CYG_MACRO_END

#define HAL_FLASH_WRITE_OFF()                                                                             \
          CYG_MACRO_START                                                                                 \
          ptr_ciu_base[((_curProc == 0) ? TS_CIU_TIMER1/4 : TS_CIU_TIMER0/4)]  = currentTimerValue;  \
  	      CYG_MACRO_END
#else
#define HAL_FLASH_WRITE_ON()
#define HAL_FLASH_WRITE_OFF()
#endif

#define SERIAL_REC_BUF  256               // size of serial receive buffer

// C ASM Macros
#ifndef REGISTER
#if defined(__cplusplus)
#define REGISTER
#else
#define REGISTER register
#endif
#endif

#ifndef _Read_CCLKCFG
#define _Read_CCLKCFG()                                       \
({ REGISTER unsigned _val_;                                   \
   asm volatile("mrc\tp14, 0, %0, c6, c0, 0" : "=r" (_val_)); \
   _val_ & 0xF;                                               \
})
#endif

#ifndef _Read_ID
#define _Read_ID()                                            \
({ REGISTER unsigned _val_;                                   \
   asm volatile("mrc\tp15, 0, %0, c0, c0, 0" : "=r" (_val_)); \
   _val_;                                                     \
})
#endif

#ifndef _Read_CIU_CTRL
#define _Read_CIU_CTRL()                                                              \
({ REGISTER unsigned _val_;                                                           \
   asm volatile("ldr\t%0, =HAL_IOC80314_CIU_BASE+TS_CIU_CFG;  \n\t"                   \
                "ldr\t%0, [%0];                                               \n\t"   \
                "ands\t%0, %0, #TS_CIU_CFG_PID;                               \n\t"   \
                          : "=r" (_val_));                                            \
                _val_ ;                                                               \
})
#endif


#endif // CYGONCE_HAL_ARM_XSCALE_HAL_IOC80314_H

// EOF hal_ioc80314.h
