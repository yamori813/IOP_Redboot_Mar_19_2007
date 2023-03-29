//==========================================================================
//
//      xscale_iop.c
//
//      HAL common code for Intel XScale I/O Processors.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2004-09-22
// Purpose:      XScale IOP Common Code
// Description:  
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_xscale.h>
#include <cyg/hal/hal_xscale_iop.h>
#include <cyg/hal/var_io.h>

#ifdef CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_I2C_API
#ifndef I2C_BASE_SDRAM
#define I2C_BASE_SDRAM I2C_BASE0
#endif 
/*------------------------------------------------------------------------*/
// I2C Support

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

// Reset the I2C device.  Currently only resets the I2C bus
// which has the SDRAM SPD on it.
void
i2c_reset(void)
{
#ifdef PLATFORM_I2C_RESET_PREAMBLE
    PLATFORM_I2C_RESET_PREAMBLE();
#endif

    // Reset I2C Unit which has SDRAM SPD
    *I2C_SDRAM_ICR = ICR_I2C_RESET;
    *I2C_SDRAM_ISR = 0x7FF;
    *I2C_SDRAM_ICR = 0;
}

// Initialize the I2C device
void
i2c_init(void)
{
    // Initialize my Slave address
	*I2C_SDRAM_ISAR = I2C_DEVID;

    //  Enable I2C Interface Unit - status will be polled
    *I2C_SDRAM_ICR = (ICR_GENERAL_CALL_DISABLE | \
                 ICR_I2C_ENABLE | \
                 ICR_SCL_ENABLE);

}

// Initialize the I2C device
void
i2c_init_fast(void)
{
    // Initialize my Slave address
    *I2C_SDRAM_ISAR = I2C_DEVID;

    //  Enable I2C Interface Unit - status will be polled
    *I2C_SDRAM_ICR = (ICR_FAST_MODE | ICR_GENERAL_CALL_DISABLE | \
                 ICR_I2C_ENABLE | \
                 ICR_SCL_ENABLE);

}

int
i2c_read_bytes(unsigned char i2c_addr, unsigned char *data, int num_bytes)
{
    unsigned int icr_shadow, timeout;

    if (num_bytes == 0)
        return 0;

    icr_shadow = *I2C_SDRAM_ICR;

    // Address of device w/ START and TRANSFER and !STOP and !ACK and Read Mode
    *I2C_SDRAM_IDBR = (i2c_addr | IDBR_RWMODE);
    icr_shadow |= (ICR_START | ICR_TRANSFER);
    icr_shadow &= ~(ICR_STOP | ICR_ACK);
    *I2C_SDRAM_ICR = icr_shadow;
    timeout = I2C_TIMOUT;
    while ((*I2C_SDRAM_ISR & ISR_TX_EMPTY) == 0)
        if (--timeout == 0)
            return -1;

    // Clear the ISR bits
    *I2C_SDRAM_ISR = 0x7ff;

    while (num_bytes--)
    {
        if (num_bytes == 0) {
            // Initiate read of device w/ !START and TRANSFER and STOP and ACK
            icr_shadow |= (ICR_TRANSFER | ICR_STOP | ICR_ACK);
            icr_shadow &= ~(ICR_START);
        } else {
            // Initiate read of device w/ !START and TRANSFER and !STOP and !ACK
            icr_shadow |= (ICR_TRANSFER);
            icr_shadow &= ~(ICR_START | ICR_STOP | ICR_ACK);
        }
        *I2C_SDRAM_ICR = icr_shadow;

        timeout = I2C_TIMOUT;
        while ((*I2C_SDRAM_ISR & ISR_RX_FULL) == 0)
            if (--timeout == 0)
                return -1;

        // Clear the ISR bits
        *I2C_SDRAM_ISR = 0x7ff;

        // Read the data
    *data++ = *I2C_SDRAM_IDBR;
    }

    // Clear the Stop Bit
    icr_shadow &= ~(ICR_TRANSFER | ICR_STOP | ICR_ACK | ICR_START);
    *I2C_SDRAM_ICR = icr_shadow;

    return 0;
}

int
i2c_write_bytes(unsigned char i2c_addr, unsigned char *data, int num_bytes)
{
    unsigned int icr_shadow, timeout;

    if (num_bytes == 0)
        return 0;

    icr_shadow = *I2C_SDRAM_ICR;

    // Address of device w/ START and TRANSFER and !STOP and !ACK and Write Mode
    *I2C_SDRAM_IDBR = (i2c_addr & ~IDBR_RWMODE);
    icr_shadow |= (ICR_START | ICR_TRANSFER);
    icr_shadow &= ~(ICR_STOP | ICR_ACK);
    *I2C_SDRAM_ICR = icr_shadow;
    timeout = I2C_TIMOUT;
    while ((*I2C_SDRAM_ISR & ISR_TX_EMPTY) == 0)
        if (--timeout == 0)
            return -1;

    // Clear the ISR bits
    *I2C_SDRAM_ISR = 0x7ff;

    while (num_bytes--)
    {
        if (num_bytes == 0) {
            // Initiate write to device w/ !START and TRANSFER and STOP and !ACK
            icr_shadow |= (ICR_TRANSFER | ICR_STOP);
            icr_shadow &= ~(ICR_START | ICR_ACK);
        } else {
            // Initiate write to device w/ !START and TRANSFER and !STOP and !ACK
            icr_shadow |= (ICR_TRANSFER);
            icr_shadow &= ~(ICR_START | ICR_STOP | ICR_ACK);
        }

        *I2C_SDRAM_IDBR = *data++;
        *I2C_SDRAM_ICR = icr_shadow;
        timeout = I2C_TIMOUT;
        while (((*I2C_SDRAM_ISR) & ISR_TX_EMPTY) == 0)
            if (--timeout == 0)
                return -1;
        // Clear the ISR bits
    *I2C_SDRAM_ISR = 0x7ff;
    }


    // Clear the Stop Bit
    icr_shadow &= ~(ICR_TRANSFER | ICR_STOP | ICR_ACK | ICR_START);
    *I2C_SDRAM_ICR = icr_shadow;


    return 0;
}

#endif // CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_I2C_API

/*------------------------------------------------------------------------*/
