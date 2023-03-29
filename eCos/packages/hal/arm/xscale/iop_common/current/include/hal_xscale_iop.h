/*=============================================================================
//
//      hal_xscale_iop.h
//
//      XScale I/O Processor Core support.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Red Hat, Inc.
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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2004-05-07
// Purpose:      
// Description:  XScale I/O Processor core support.
// Usage:        #include <cyg/hal/hal_xscale_iop.h>
//
// Copyright:    (C) 2003-2005 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_IOP_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_IOP_H

#include <pkgconf/system.h>
#include <pkgconf/hal_arm_xscale_iop.h>
#include CYGBLD_HAL_VARIANT_H
#include CYGBLD_HAL_VAR_H

// --------------------------------------------------------------------------
// MCU Register definitions
#define MCU_ECCR_SINGLE_BIT_REPORTING   0x1
#define MCU_ECCR_MULTI_BIT_REPORTING    0x2
#define MCU_ECCR_SINGLE_BIT_CORRECTION  0x4

// --------------------------------------------------------------------------
// I2C Register definitions
#define I2C_ICR				0x00
#define I2C_ISR				0x04
#define I2C_ISAR			0x08
#define I2C_IDBR			0x0c
#define I2C_ICCR			0x10
#define I2C_IBMR			0x14
#define I2C_IMBR			0x18

#ifdef I2C_BASE0
#define I2C_ICR0			REG32(I2C_BASE0,I2C_ICR)
#define I2C_ISR0			REG32(I2C_BASE0,I2C_ISR)
#define I2C_ISAR0			REG32(I2C_BASE0,I2C_ISAR)
#define I2C_IDBR0			REG32(I2C_BASE0,I2C_IDBR)
#define I2C_ICCR0			REG32(I2C_BASE0,I2C_ICCR)
#define I2C_IBMR0			REG32(I2C_BASE0,I2C_IBMR)
#define I2C_IMBR0			REG32(I2C_BASE0,I2C_IMBR)
#endif

#ifdef I2C_BASE1
#define I2C_ICR1			REG32(I2C_BASE1,I2C_ICR)
#define I2C_ISR1			REG32(I2C_BASE1,I2C_ISR)
#define I2C_ISAR1			REG32(I2C_BASE1,I2C_ISAR)
#define I2C_IDBR1			REG32(I2C_BASE1,I2C_IDBR)
#define I2C_ICCR1			REG32(I2C_BASE1,I2C_ICCR)
#define I2C_IBMR1			REG32(I2C_BASE1,I2C_IBMR)
#define I2C_IMBR1			REG32(I2C_BASE1,I2C_IMBR)
#endif

#ifdef I2C_BASE2
#define I2C_ICR2			REG32(I2C_BASE2,I2C_ICR)
#define I2C_ISR2			REG32(I2C_BASE2,I2C_ISR)
#define I2C_ISAR2			REG32(I2C_BASE2,I2C_ISAR)
#define I2C_IDBR2			REG32(I2C_BASE2,I2C_IDBR)
#define I2C_ICCR2			REG32(I2C_BASE2,I2C_ICCR)
#define I2C_IBMR2			REG32(I2C_BASE2,I2C_IBMR)
#define I2C_IMBR2			REG32(I2C_BASE2,I2C_IMBR)
#endif
/* The SDRAM SPD may not always be on I2C0, so a SDRAM
   base and offset are defined to indicate which I2C the
   SPD is on.  If I2C_BASE_SDRAM is not defined, default
   to I2C0 - cebruns */
#ifdef I2C_BASE_SDRAM
#define I2C_SDRAM_ICR		REG32(I2C_BASE_SDRAM,I2C_ICR)
#define I2C_SDRAM_ISR		REG32(I2C_BASE_SDRAM,I2C_ISR)
#define I2C_SDRAM_ISAR		REG32(I2C_BASE_SDRAM,I2C_ISAR)
#define I2C_SDRAM_IDBR		REG32(I2C_BASE_SDRAM,I2C_IDBR)
#define I2C_SDRAM_ICCR		REG32(I2C_BASE_SDRAM,I2C_ICCR)
#define I2C_SDRAM_IBMR		REG32(I2C_BASE_SDRAM,I2C_IBMR)
#define I2C_SDRAM_IMBR 		REG32(I2C_BASE_SDRAM,I2C_IMBR)
#else
#ifdef I2C_BASE0
#define I2C_SDRAM_ICR		REG32(I2C_BASE0,I2C_ICR)
#define I2C_SDRAM_ISR		REG32(I2C_BASE0,I2C_ISR)
#define I2C_SDRAM_ISAR		REG32(I2C_BASE0,I2C_ISAR)
#define I2C_SDRAM_IDBR		REG32(I2C_BASE0,I2C_IDBR)
#define I2C_SDRAM_ICCR		REG32(I2C_BASE0,I2C_ICCR)
#define I2C_SDRAM_IBMR		REG32(I2C_BASE0,I2C_IBMR)
#define I2C_SDRAM_IMBR 		REG32(I2C_BASE0,I2C_IMBR)
#endif // I2C_BASE0
#endif // I2C_BASE_SDRAM
// ICR Bit Definitions
#define ICR_START			0x0001
#define ICR_STOP			0x0002
#define ICR_ACK				0x0004
#define ICR_TRANSFER			0x0008
#define ICR_MASTER_ABORT		0x0010
#define ICR_SCL_ENABLE			0x0020
#define ICR_I2C_ENABLE			0x0040
#define ICR_GENERAL_CALL_DISABLE	0x0080
#define ICR_TX_EMPTY_INTERRUPT_ENABLE	0x0100
#define ICR_RX_FULL_INTERRUPT_ENABLE	0x0200
#define ICR_BUS_ERR_INTERUPT_ENABLE	0x0400
#define ICR_SLV_STOP_INTERRUPT_ENABLE	0x0800
#define ICR_ARB_LOSS_INTERRUPT_ENABLE	0x1000
#define ICR_SLV_ADDR_INTERRUPT_ENABLE	0x2000
#define ICR_I2C_RESET			0x4000
#define ICR_FAST_MODE			0x8000

// ISR Bit Definitions
#define ISR_RWMODE			0x0001
#define ISR_ACK				0x0002
#define ISR_I2C_BUSY			0x0004
#define ISR_I2C_BUS_BUSY		0x0008
#define ISR_SLV_STOP_DETECTED		0x0010
#define ISR_ARB_LOSS_DETECTED		0x0020
#define ISR_TX_EMPTY			0x0040
#define ISR_RX_FULL			0x0080
#define ISR_GENERAL_CALL_DETECTED	0x0100
#define ISR_SLV_ADDRESS_DETECTED	0x0200
#define ISR_BUS_ERROR_DETECTED		0x0400

// ISAR Bit Definitions
#define ISAR_SLV_ADDRESS_MASK		0x7f

// IDBR Bit Definitions
#define IDBR_DATA_MASK			0x000000ff
#define IDBR_RWMODE			0x1

#ifdef CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_I2C_API

#ifndef __ASSEMBLER__
extern void i2c_reset(void);
extern void i2c_init(void);
extern int i2c_read_bytes(unsigned char, unsigned char *, int);
extern int i2c_write_bytes(unsigned char, unsigned char *, int);
#endif // __ASSEMBLER__

#endif // CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_I2C_API

//--------------------------------------------------------------
// IOP memory layout for Linux
#define CYGHWR_REDBOOT_LINUX_ATAG_MEM(_p_)                                                      \
    CYG_MACRO_START                                                                             \
    /* Next ATAG_MEM. */                                                                        \
    _p_->hdr.size = (sizeof(struct tag_mem32) + sizeof(struct tag_header))/sizeof(long);        \
    _p_->hdr.tag = ATAG_MEM;                                                                    \
    if (CYGNUM_HAL_ARM_XSCALE_IOP_LINUX_RAM_MAX == 0) {                                         \
       _p_->u.mem.size  = hal_dram_size;                                                        \
    } else {                                                                                    \
       _p_->u.mem.size  = (hal_dram_size < CYGNUM_HAL_ARM_XSCALE_IOP_LINUX_RAM_MAX) ?           \
                          hal_dram_size : CYGNUM_HAL_ARM_XSCALE_IOP_LINUX_RAM_MAX;              \
    }                                                                                           \
    _p_->u.mem.start = CYGARC_PHYSICAL_ADDRESS(SDRAM_BASE);                                     \
    CYG_MACRO_END

#ifndef __ASSEMBLER__
extern unsigned int hal_platform_linux_exec_capable(void);
#define HAL_PLATFORM_LINUX_EXEC_CAPABLE() hal_platform_linux_exec_capable()
#endif

//--------------------------------------------------------------
#endif // CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_IOP_H
// EOF hal_xscale.h
