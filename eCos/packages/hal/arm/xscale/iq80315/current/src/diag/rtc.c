//=============================================================================
//
//      rtc.c : Real Time Clock functions
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
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter, dmoseley
// Date:        2001-01-25
// Purpose:
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ioc80314.h>          // Hardware definitions
#include <cyg/hal/iq80315.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf
#include <redboot.h>
#include <cyg/io/pci.h>
#include "test_menu.h"

extern int				diag_ishex(char inChar);
extern int				diag_hex2dec(char inChar);

typedef	struct {
		cyg_uint32 hundredths:4;
		cyg_uint32 tenths:4;
		cyg_uint32 seconds:4;
		cyg_uint32 tens_seconds:3;
		cyg_uint32 stop:1;
		cyg_uint32 minutes:4;
		cyg_uint32 tens_minutes:3;
		cyg_uint32 reserved0:1;
		cyg_uint32 hours:4;
		cyg_uint32 tens_hours:2;
		cyg_uint32 century:1;
		cyg_uint32 century_enable:1;
		cyg_uint32 day_of_week:3;
		cyg_uint32 reserved1:4;
		cyg_uint32 trec:1;
		cyg_uint32 date:4;
		cyg_uint32 tens_date:2;
		cyg_uint32 reserved2:2;
		cyg_uint32 month:4;
		cyg_uint32 tens_months:1;
		cyg_uint32 reserved3:3;
		cyg_uint32 year:4;
		cyg_uint32 tens_years:4;
} rtc_t;
typedef enum {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday} days_of_week;
typedef enum {January=1, February, March, April, May, June, July, August, September, October, \
		 	  November, December} months_of_year;

void				rtc_menu (MENU_ARG arg);
void				test_buzzer (MENU_ARG arg);
static cyg_int32 	read_rtc_i2c ( rtc_t *my_time );
static void 		write_rtc_i2c (rtc_t *my_time );
static void			display_rtc_i2c (rtc_t *my_time);
static void			print_month(rtc_t *my_time);
static void			get_date(rtc_t *my_time);
static void			get_time(rtc_t *my_time);

extern cyg_int32  read_i2c_byte ( unsigned int dev_id, unsigned int byte );
extern cyg_int32  write_i2c_byte (cyg_uint32 dev_id, cyg_uint16 byte, cyg_uint8 data );
extern cyg_uint32 decIn( void );

static void
display_rtc_i2c (rtc_t *my_time) {
	cyg_int32 status;
	char buf[1];

	diag_printf ("Strike <CR> to exit\n");
	while (_rb_gets(buf, sizeof(buf), 500) != _GETS_OK) {
		status = read_rtc_i2c(my_time);
		if (status < 0)
			return;
		if (my_time->tens_years == 0xF) {
			diag_printf ("No Time Set Yet...\n");
			return;
		}
		print_month(my_time);
		diag_printf ("%1d%1d, ", my_time->tens_date, my_time->date);
		diag_printf ("20%1d%1d", my_time->tens_years, my_time->year);
		diag_printf (", ");
		diag_printf ("%1d%1d:%1d%1d.%1d%1d",  my_time->tens_hours, my_time->hours, \
											my_time->tens_minutes, my_time->minutes, \
											my_time->tens_seconds, my_time->seconds);
		diag_printf ("\r");
	}
}

static cyg_int32
read_rtc_i2c ( rtc_t *my_time ) {
	cyg_int32 data;
	cyg_uint32 dev_id = 0x68;

	data =  read_i2c_byte (dev_id, 0xc); /* Get Halt Bit */
	if (data < 0) 
		return (data);
	my_time->hundredths 	= (read_i2c_byte (dev_id, 0) & 0x0f);
	my_time->tenths     	= ((read_i2c_byte (dev_id, 0) & 0xf0) >> 4);
	my_time->seconds    	= (read_i2c_byte (dev_id, 1) & 0x0f);
	my_time->tens_seconds 	= ((read_i2c_byte (dev_id, 1) & 0x70) >> 4);
	my_time->minutes		= (read_i2c_byte (dev_id, 2) & 0x0f);
	my_time->tens_minutes	= ((read_i2c_byte (dev_id, 2) & 0x70) >> 4);
	my_time->hours			= (read_i2c_byte (dev_id, 3) & 0x0f);
	my_time->tens_hours		= ((read_i2c_byte (dev_id, 3) & 0x30) >> 4);
	my_time->day_of_week	= (read_i2c_byte (dev_id, 4) & 0x07);
	my_time->date			= (read_i2c_byte (dev_id, 5) & 0x0f);
	my_time->tens_date		= ((read_i2c_byte (dev_id, 5) & 0x30) >> 4);;
	my_time->month			= (read_i2c_byte (dev_id, 6) & 0x0f);
	my_time->tens_months	= ((read_i2c_byte (dev_id, 6) & 0x10) >> 4);
	my_time->year			= (read_i2c_byte (dev_id, 7) & 0x0f);
	my_time->tens_years		= ((read_i2c_byte (dev_id, 7) & 0xf0) >> 4);
	data &= ~(0x40);
	write_i2c_byte (dev_id, 0xc, data);  /* Let Clock Run */

	return(0);
}

static void
write_rtc_i2c ( rtc_t *my_time ) {
	cyg_uint32 data, dev_id = 0x68;
	cyg_uint8 minutes, hours, years, months, date;

	get_date(my_time);
	get_time(my_time);

	data = read_i2c_byte (dev_id, 0xc);
	data |= (0x40); // Set Halt update bit
	write_i2c_byte (dev_id, 0xc, data);
	minutes = ((my_time->tens_minutes << 4) | my_time->minutes);
	hours   = ((my_time->tens_hours   << 4) | my_time->hours);
	years   = ((my_time->tens_years   << 4) | my_time->year);
	months  = ((my_time->tens_months  << 4) | my_time->month);
	date    = ((my_time->tens_date    << 4) | my_time->date);
	write_i2c_byte (dev_id, 0x0, 0x0);	
	write_i2c_byte (dev_id, 0x1, 0x00);	/*Seconds*/
	write_i2c_byte (dev_id, 0x2, minutes);	/*Minutes+10s minutes*/
	write_i2c_byte (dev_id, 0x3, hours);	/* Hours + 10s hours */
	write_i2c_byte (dev_id, 0x5, date);	/* Date + 10s date */
	write_i2c_byte (dev_id, 0x6, months);	/* Month + 10s Month */
	write_i2c_byte (dev_id, 0x7, years);	/* Year + 10s year */
	data = read_i2c_byte (dev_id, 0xc);
	data &= ~(0x40); // clear Halt update bit
	write_i2c_byte (dev_id, 0xc, data);

}

void
rtc_menu (MENU_ARG arg) {
	cyg_uint32 choice;
	rtc_t my_time;
	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315")) {
		diag_printf ("No RTC on this board\n");
		return;
	}
	do {
		diag_printf ("\n\n");
		diag_printf ("=======================================\n");
		diag_printf ("|  Real Time Clock Menu for IQ80315    |\n");
		diag_printf ("=======================================\n");
		diag_printf ("| 1) Display Current Time              |\n");
		diag_printf ("| 2) Set Time                          |\n");
		diag_printf ("---------------------------------------\n");
		choice = decIn();
		switch (choice) {
		case 1:
			display_rtc_i2c(&my_time);
			break;
		case 2:
			write_rtc_i2c(&my_time);
			break;
		default:
			diag_printf ("Invalid selection\n");
		}
	} while (choice);
}

static void
print_month(rtc_t *my_time) {
	months_of_year month;
	month = my_time->month + (my_time->tens_months*10);

	switch (month) {
	case 1:
		diag_printf ("January ");
		break;
	case 2:
		diag_printf ("February ");
		break;
	case 3:
		diag_printf ("March ");
		break;
	case 4:
		diag_printf ("April ");
		break;
	case 5:
		diag_printf ("May ");
		break;
	case 6:
		diag_printf ("June ");
		break;
	case 7:
		diag_printf ("July ");
		break;
	case 8:
		diag_printf ("August ");
		break;
	case 9:
		diag_printf ("September ");
		break;
	case 10:
		diag_printf ("October ");
		break;
	case 11:
		diag_printf ("November ");
		break;
	case 12:
		diag_printf ("December ");
		break;
	default:
		diag_printf ("NoMonth ");
	}
}

static void
get_date ( rtc_t *my_time )
{
	char input[12], date[3], *p;
	int got;

	do {
		got = 0;
		diag_printf ("Enter Date (mm/dd/yy): ");
		while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
			;

		p = input;
		while (*p && *p == ' ');

		if (p[0] == '\0' || p[1] == '\0')
			continue;

		for (; got < 3; got++) {
			if (!diag_ishex(p[0]) || !diag_ishex(p[1]))
				break;
			date[got] = (diag_hex2dec(p[0]) * 16) + diag_hex2dec(p[1]);
			p += 2;
			if (*p == '/' || *p == ':' || *p == ' ' || *p == '-')
				p++;
		}
	} while (got != 3);

	my_time->tens_months = ((date[0] & 0x10) >> 4);
	my_time->month = (date[0] & 0xf);
	my_time->tens_date = ((date[1] & 0x30) >> 4);
	my_time->date = (date[1] & 0xf);
	my_time->tens_years = ((date[2] & 0xf0) >> 4);
	my_time->year = (date[2] & 0xf);	
}

static void
get_time ( rtc_t *my_time )
{
	char input[12], time[2], *p;
	int got;

	do {
		got = 0;
		diag_printf ("Enter Time (24hr Format) (hh:mm): ");
		while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
			;

		p = input;
		while (*p && *p == ' ');

		if (p[0] == '\0' || p[1] == '\0')
			continue;

		for (; got < 2; got++) {
			if (!diag_ishex(p[0]) || !diag_ishex(p[1]))
				break;
			time[got] = (diag_hex2dec(p[0]) * 16) + diag_hex2dec(p[1]);
			p += 2;
			if (*p == '/' || *p == ':' || *p == ' ' || *p == '-')
				p++;
		}
	} while (got != 2);																	   

	my_time->tens_hours = ((time[0] & 0xf0) >> 4);
	my_time->hours = (time[0] & 0xf);
	my_time->tens_minutes = ((time[1] & 0xf0) >> 4);
	my_time->minutes = (time[1] & 0xf);
}

void
test_buzzer (MENU_ARG arg) {
	cyg_uint32 dev_id = 0x68;
	cyg_uint8  buzz_freq;
	cyg_uint32 data;

	if (strcmp(HAL_PLATFORM_BOARD, "IQ80315")) {
		diag_printf ("No Buzzer on this board\n");
		return;
	}

	data = read_i2c_byte (dev_id, 0x1);
	data &= ~(0x80); // Clear ST bit
	write_i2c_byte (dev_id, 0x1, data);

	diag_printf ("Testing Buzzer...from 32kHz to 512Hz\n");
	data = read_i2c_byte (dev_id, 0xa);  // Get SQE
	if (data < 0)
		return;
	data |= 0x40;   // Set SQE bit
	write_i2c_byte(dev_id, 0xa, data);

	for (buzz_freq = 0; buzz_freq < 0x7; buzz_freq++) {
		write_i2c_byte(dev_id, 0x13, (buzz_freq << 4));
	    CYGACC_CALL_IF_DELAY_US(250000);
	}
	data &= ~(0x40); // Clear SQE bit
	write_i2c_byte(dev_id, 0xa, data);
}
