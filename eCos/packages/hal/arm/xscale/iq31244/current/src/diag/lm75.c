//=============================================================================
//
//      lm75.c
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
// Date:        2003-05-20
// Purpose:     
// Description: Tests for lm75 temperature sensors
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_verde.h>    // Hardware definitions
#include <cyg/hal/iq31244.h>      // Platform specifics
#include <cyg/hal/hal_intr.h>      // Platform specifics
#include "i2c.h"
#include "test_menu.h"

static int
lm75_read16(cyg_uint8 slave, cyg_uint8 reg, cyg_int16 *val)
{
    cyg_uint8 buf[2];

    if (!iq31244_diag_i2c_write(slave, &reg, 1, 0))
	return 0;
    
    if (!iq31244_diag_i2c_read(slave, buf, 2, 1))
	return 0;
    *val = (buf[0] << 8) | buf[1];
    return 1;
}

static int
lm75_write16(cyg_uint8 slave, cyg_uint8 reg, cyg_int16 val)
{
    cyg_uint8 buf[2];
    buf[0] = (val >> 8) & 0xff;
    buf[1] = val & 0xff;

    if (!iq31244_diag_i2c_write(slave, &reg, 1, 0))
	return 0;

    return iq31244_diag_i2c_write_more(buf, 2, 1);
}


static int
lm75_read_temp(cyg_uint8 slave, cyg_int16 *val)
{
    return lm75_read16(slave, 0, val);
}

static int
lm75_read_config(cyg_uint8 slave, cyg_uint8 *val)
{
    cyg_uint8 reg = 1;

    if (!iq31244_diag_i2c_write(slave, &reg, 1, 0))
	return 0;
    
    return iq31244_diag_i2c_read(slave, val, 1, 1);
}

static int
lm75_write_config(cyg_uint8 slave, cyg_uint8 val)
{
    cyg_uint8 reg = 1;

    if (!iq31244_diag_i2c_write(slave, &reg, 1, 0))
	return 0;
    
    return iq31244_diag_i2c_write_more(&val, 1, 1);
}


static int
lm75_read_hyst(cyg_uint8 slave, cyg_int16 *val)
{
    return lm75_read16(slave, 2, val);
}

static int
lm75_write_hyst(cyg_uint8 slave, cyg_int16 val)
{
    return lm75_write16(slave, 2, val);
}

static int
lm75_read_os(cyg_uint8 slave, cyg_int16 *val)
{
    return lm75_read16(slave, 3, val);
}

static int
lm75_write_os(cyg_uint8 slave, cyg_int16 val)
{
    return lm75_write16(slave, 3, val);
}


static void
pr_temp(cyg_int16 temp)
{
    int dec;
    
    temp >>= 7;

    if (temp < 0) {
	diag_printf("-");
	dec = -(temp >> 1);
	if (temp & 1)
	    --dec;
    } else
	dec = temp >> 1;
    diag_printf("%d.%d", dec, (temp & 1)?5:0);
}

static int
atotemp(char *s, cyg_int16 *temp)
{
    int frac, dec, neg = 0;

    while (*s && *s == ' ') ++s;
    if (*s == '-') {
	neg = 0;
	++s;
    }

    dec = frac = 0;
    while (*s && '0' <= *s && *s <= '9')
	dec = (dec * 10) + (*s++ - '0');

    if (*s && *s == '.') {
	++s;
	if (*s) {
	    if ('5' <= *s && *s <= '9')
		frac = 1;
	    ++s;
	}
    }
    while (*s && *s == ' ') ++s;
    if (*s)
	return 0;  // garbage at end

    if (neg)
	dec = -dec + frac;

    dec <<= 1;
    dec |= frac;

    if ((dec & 0xff00) != 0 && (dec & 0xff00) != 0xff00)
	return 0; // out of range

    *temp = dec << 7;
    return 1;
}


static int
pr_regs(char *name, cyg_uint8 slave)
{
    cyg_uint8 config;
    cyg_int16 temp, hyst, os;

    if (!lm75_read_config(slave, &config)) {
	diag_printf("Error reading %s config register.\n", name);
	return 0;
    }

    if (!lm75_read_temp(slave, &temp)) {
	diag_printf("Error reading %s temp register.\n", name);
	return 0;
    }

    if (!lm75_read_hyst(slave, &hyst)) {
	diag_printf("Error reading %s hyst register.\n", name);
	return 0;
    }

    if (!lm75_read_os(slave, &os)) {
	diag_printf("Error reading %s os register.\n", name);
	return 0;
    }

    diag_printf("%s: config[%02x] temp: ", name, config);
    pr_temp(temp);
    diag_printf("  hyst: ");
    pr_temp(hyst);
    diag_printf("  os: ");
    pr_temp(os);
    diag_printf("\n");

    return 1;
}



static void
lm75_irq_test(cyg_uint8 slave)
{
    cyg_uint32 oldmask, newmask, stat;
    cyg_uint16 temp, newtemp;
    cyg_uint8  config;
    int delay;

    INTCTL_READ(oldmask);
    newmask = oldmask | (1 << CYGNUM_HAL_INTERRUPT_XINT1);
    INTCTL_WRITE(newmask);

    IINTSRC_READ(stat);
    if (stat & (1 << CYGNUM_HAL_INTERRUPT_XINT1)) {
	diag_printf("IRQ already active...failed\n");
	return;
    }

    if (!lm75_read_temp(slave, &temp)) {
	diag_printf("failed (temp read).\n");
	return;
    }
    
    if (!lm75_write_config(slave, 2)) {
	diag_printf("failed (config write).\n");
	return;
    }

    newtemp = ((temp >> 7) - (15 << 1)) << 7;
    if (!lm75_write_hyst(slave, newtemp)) {
	diag_printf("failed (hyst write).\n");
	return;
    }

    newtemp = ((temp >> 7) - (10 << 1)) << 7;
    if (!lm75_write_os(slave, newtemp)) {
	diag_printf("failed (os write).\n");
	return;
    }

    delay = 50;
    while (delay-- > 0) {
	hal_delay_us(100000);
	IINTSRC_READ(stat);
	if (stat & (1 << CYGNUM_HAL_INTERRUPT_XINT1))
	    break;
    }

    if ((stat & (1 << CYGNUM_HAL_INTERRUPT_XINT1)) == 0) {
	diag_printf("failed (IRQ on)\n");
	return;
    }
	
    if (!lm75_read_temp(slave, &newtemp)) {
	diag_printf("failed (temp read 2).\n");
	return;
    }

#if 0
    if (!lm75_read_config(slave, &config)) {
	diag_printf("failed (config read).\n");
	return 0;
    }

    if (!lm75_read_hyst(slave, &newtemp)) {
	diag_printf("failed (temp read 2).\n");
	return;
    }

    if (!lm75_read_os(slave, &newtemp)) {
	diag_printf("failed (temp read 2).\n");
	return;
    }
#endif

    delay = 100;
    while (delay-- > 0) {
	hal_delay_us(100000);
	IINTSRC_READ(stat);
	if ((stat & (1 << CYGNUM_HAL_INTERRUPT_XINT1)) == 0)
	    break;
    }

    if ((stat & (1 << CYGNUM_HAL_INTERRUPT_XINT1)) != 0) {
	diag_printf("failed (IRQ off)\n");
	return;
    }
	
    atotemp("80.0", &newtemp);
    if (!lm75_write_os(slave, newtemp)) {
	diag_printf("failed (os write 2).\n");
	return;
    }

    atotemp("75.0", &newtemp);
    if (!lm75_write_hyst(slave, newtemp)) {
	diag_printf("failed (os write 2).\n");
	return;
    }

    diag_printf("passed\n");
}


void
lm75_test (MENU_ARG arg)
{
    iq31244_diag_i2c_enable();

    diag_printf("\nRegister dump:\n");

    if (!pr_regs("  Sensor 0", 0x48)) {
	iq31244_diag_i2c_disable();
	return;
    }
    if (!pr_regs("  Sensor 1", 0x49)) {
	iq31244_diag_i2c_disable();
	return;
    }

    diag_printf("\nSensor 0 IRQ test...");
    lm75_irq_test(0x48);

    diag_printf("Sensor 1 IRQ test...");
    lm75_irq_test(0x49);

    iq31244_diag_i2c_disable();

    diag_printf("\n");
    diag_wait();
}
