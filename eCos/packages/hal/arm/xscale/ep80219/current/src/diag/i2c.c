//=============================================================================
//
//      i2c.c
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Mark Salter
// Contributors: Mark Salter, dmoseley
// Date:         2003-05-20
// Purpose:     
// Description:  I2C support for diags.
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#include <redboot.h>
#include <cyg/hal/hal_verde.h>    // Hardware definitions
#include <cyg/hal/ep80219.h>      // Platform specifics
#include <cyg/hal/hal_xscale_iop.h> // IOP Common definitions
#include "i2c.h"

// I2C 0
#define _ICR_P0   I2C_ICR0
#define _ISR_P0   I2C_ISR0
#define _ISAR_P0  I2C_ISAR0
#define _IDBR_P0  I2C_IDBR0

// I2C 1
#define _ICR   I2C_ICR1
#define _ISR   I2C_ISR1
#define _ISAR  I2C_ISAR1
#define _IDBR  I2C_IDBR1

void
ep80219_diag_i2c_enable(void)
{
    *_ICR = ICR_I2C_RESET;
    *_ISR = 0x7ff;
    *_ICR = 0;
    *_ISAR = I2C_DEVID;
    *_ICR = ICR_GENERAL_CALL_DISABLE | ICR_I2C_ENABLE | ICR_SCL_ENABLE;
}

void
ep80219_diag_i2c_disable(void)
{
    *_ICR &= ~(ICR_I2C_ENABLE | ICR_SCL_ENABLE);
}


static int
wait_for_empty(void)
{
    int delay = 100000;  // 1 second
    cyg_uint8 sts;

    while (delay-- > 0) {
	if ((sts = (*_ISR & (ISR_TX_EMPTY | ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))) != 0) {
	    *_ISR = sts; // clear it
	    if (sts & (ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))
		return 0;
	    return 1;
	}
	hal_delay_us(10);
    }
    return 0;
}

static int
wait_for_full(void)
{
    int delay = 100000;  // 1 second
    cyg_uint8 sts;

    while (delay-- > 0) {
	if ((sts = (*_ISR & (ISR_RX_FULL | ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))) != 0) {
	    *_ISR = sts; // clear it
	    if (sts & (ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))
		return 0;
	    return 1;
	}
	hal_delay_us(10);
    }
    return 0;
}


#define ICR_START_TRANSFER    (ICR_GENERAL_CALL_DISABLE | ICR_I2C_ENABLE | ICR_SCL_ENABLE | ICR_START | ICR_TRANSFER)
#define ICR_CONTINUE_TRANSFER (ICR_GENERAL_CALL_DISABLE | ICR_I2C_ENABLE | ICR_SCL_ENABLE | ICR_TRANSFER)
#define ICR_STOP_TRANSFER     (ICR_GENERAL_CALL_DISABLE | ICR_I2C_ENABLE | ICR_SCL_ENABLE | ICR_STOP | ICR_ACK | ICR_TRANSFER)

int
ep80219_diag_i2c_read(cyg_uint8 slave, cyg_uint8 *buf, int nbytes, int eot)
{
    *_IDBR = (slave << 1) | IDBR_RWMODE;
    *_ICR = ICR_START_TRANSFER;
    if (!wait_for_empty())
	return 0;

    while (--nbytes >= 0) {
	if (nbytes == 0 && eot)
	    *_ICR = ICR_STOP_TRANSFER;
	else
	    *_ICR = ICR_CONTINUE_TRANSFER;
	if (!wait_for_full())
	    return 0;
	*buf++ = *_IDBR;
    }
    return 1;
}


int
ep80219_diag_i2c_read_more(cyg_uint8 *buf, int nbytes, int eot)
{
    while (--nbytes >= 0) {
	if (nbytes == 0 && eot)
	    *_ICR = ICR_STOP_TRANSFER;
	else
	    *_ICR = ICR_CONTINUE_TRANSFER;
	if (!wait_for_full())
	    return 0;
	*buf++ = *_IDBR;
    }
    return 1;
}


int
ep80219_diag_i2c_write(cyg_uint8 slave, cyg_uint8 *buf, int nbytes, int eot)
{
    *_IDBR = slave << 1;
    *_ICR = ICR_START_TRANSFER;
    if (!wait_for_empty())
	return 0;

    while (--nbytes >= 0) {
	*_IDBR = *buf++;
	if (nbytes == 0 && eot)
	    *_ICR = ICR_STOP_TRANSFER;
	else
	    *_ICR = ICR_CONTINUE_TRANSFER;
	if (!wait_for_empty())
	    return 0;
    }
    return 1;
}

int
ep80219_diag_i2c_write_more(cyg_uint8 *buf, int nbytes, int eot)
{
    while (--nbytes >= 0) {
	*_IDBR = *buf++;
	if (nbytes == 0 && eot)
	    *_ICR = ICR_STOP_TRANSFER;
	else
	    *_ICR = ICR_CONTINUE_TRANSFER;
	if (!wait_for_empty())
	    return 0;
    }
    return 1;
}


void
ep80219_diag_i2c_enable_p0(void)
{
    *_ICR_P0 = ICR_I2C_RESET;
    *_ISR_P0 = 0x7ff;
    *_ICR_P0 = 0;
    *_ISAR_P0 = I2C_DEVID;
    *_ICR_P0 = ICR_GENERAL_CALL_DISABLE | ICR_I2C_ENABLE | ICR_SCL_ENABLE;
}

void
ep80219_diag_i2c_disable_p0(void)
{
    *_ICR_P0 &= ~(ICR_I2C_ENABLE | ICR_SCL_ENABLE);
}


static int
wait_for_empty_p0(void)
{
    int delay = 100000;  // 1 second
    cyg_uint8 sts;

    while (delay-- > 0) {
	if ((sts = (*_ISR_P0 & (ISR_TX_EMPTY | ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))) != 0) {
	    *_ISR_P0 = sts; // clear it
	    if (sts & (ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))
		return 0;
	    return 1;
	}
	hal_delay_us(10);
    }
    return 0;
}

static int
wait_for_full_p0(void)
{
    int delay = 100000;  // 1 second
    cyg_uint8 sts;

    while (delay-- > 0) {
	if ((sts = (*_ISR_P0 & (ISR_RX_FULL | ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))) != 0) {
	    *_ISR_P0 = sts; // clear it
	    if (sts & (ISR_ARB_LOSS_DETECTED | ISR_BUS_ERROR_DETECTED))
		return 0;
	    return 1;
	}
	hal_delay_us(10);
    }
    return 0;
}

int
ep80219_diag_i2c_read_p0(cyg_uint8 slave, cyg_uint8 *buf, int nbytes, int eot)
{
    *_IDBR_P0 = (slave << 1) | IDBR_RWMODE;
    *_ICR_P0 = ICR_START_TRANSFER;
    if (!wait_for_empty_p0())
	return 0;

    while (--nbytes >= 0) {
	if (nbytes == 0 && eot)
	    *_ICR_P0 = ICR_STOP_TRANSFER;
	else
	    *_ICR_P0 = ICR_CONTINUE_TRANSFER;
	if (!wait_for_full_p0())
	    return 0;
	*buf++ = *_IDBR_P0;
    }
    return 1;
}


int
ep80219_diag_i2c_read_more_p0(cyg_uint8 *buf, int nbytes, int eot)
{
    while (--nbytes >= 0) {
	if (nbytes == 0 && eot)
	    *_ICR_P0 = ICR_STOP_TRANSFER;
	else
	    *_ICR_P0 = ICR_CONTINUE_TRANSFER;
	if (!wait_for_full_p0())
	    return 0;
	*buf++ = *_IDBR_P0;
    }
    return 1;
}


int
ep80219_diag_i2c_write_p0(cyg_uint8 slave, cyg_uint8 *buf, int nbytes, int eot)
{
    *_IDBR_P0 = slave << 1;
    *_ICR_P0 = ICR_START_TRANSFER;
    if (!wait_for_empty_p0())
	return 0;

    while (--nbytes >= 0) {
	*_IDBR_P0 = *buf++;
	if (nbytes == 0 && eot)
	    *_ICR_P0 = ICR_STOP_TRANSFER;
	else
	    *_ICR_P0 = ICR_CONTINUE_TRANSFER;
	if (!wait_for_empty_p0())
	    return 0;
    }
    return 1;
}

int
ep80219_diag_i2c_write_more_p0(cyg_uint8 *buf, int nbytes, int eot)
{
    while (--nbytes >= 0) {
	*_IDBR_P0 = *buf++;
	if (nbytes == 0 && eot)
	    *_ICR_P0 = ICR_STOP_TRANSFER;
	else
	    *_ICR_P0 = ICR_CONTINUE_TRANSFER;
	if (!wait_for_empty_p0())
	    return 0;
    }
    return 1;
}


