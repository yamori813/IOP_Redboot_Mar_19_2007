//=============================================================================
//
//      smsc.c - Diagnostic support for smsc 91c111 NIC
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
// Author(s):    Adam Brooks
// Contributors: Mark Salter
// Date:         2006-11-06
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include "test_menu.h"

#ifdef CYGPKG_DEVS_ETH_SMSC_LAN91CXX
extern int diag_ishex(char theChar);
extern int diag_hex2dec(char hex);
extern CYG_ADDRWORD decIn(void);


// Get MAC address from user.
// Acceptable formats are:
//   nn.nn.nn.nn.nn.nn
//   nn:nn:nn:nn:nn:nn
//   nn-nn-nn-nn-nn-nn
//   nn nn nn nn nn nn
//   nnnnnnnnnnnn
static void
get_mac_address(char *addr_buf)
{
    char input[40], mac[6], *p;
    int got, i;

    do {
	got = 0;
//	diag_printf ("\nCurrent MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
//		     addr_buf[0], addr_buf[1], addr_buf[2],
//		     addr_buf[3], addr_buf[4], addr_buf[5]);
	diag_printf ("Enter desired MAC address: ");
	while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
	    ;

	p = input;
	while (*p && *p == ' ');

	if (p[0] == '\0' || p[1] == '\0')
	    continue;

	for (; got < 6; got++) {
	    if (!diag_ishex(p[0]) || !diag_ishex(p[1]))
		break;
	    mac[got] = (diag_hex2dec(p[0]) * 16) + diag_hex2dec(p[1]);
	    p += 2;
	    if (*p == '.' || *p == ':' || *p == ' ' || *p == '-')
		p++;
	}
    } while (got != 6);

    for (i = 0; i < 6; i++)
	*addr_buf++ = mac[i];
}


void
smsc_setup (MENU_ARG arg)
{
	cyg_uint32 count;
	cyg_uint8 eeprom_data[6];

	volatile cyg_uint16 * BANKSEL = 0xf20a061c;
	volatile cyg_uint16 * POINTER = 0xf20a060c;
	volatile cyg_uint16 * GP = 0xf20a0614;
	volatile cyg_uint16 * CONTROL = 0xf20a0618;
		
	
	
	get_mac_address((char *)eeprom_data);
	
	for(count = 0; count < 3; count++)
	{
		*BANKSEL = 0x3302;
		*POINTER = (0x20 + count);
		*BANKSEL = 0x3301;
		*GP = ((eeprom_data[(count*2)]) | (eeprom_data[((count*2)+1)] << 8));
		*CONTROL = 0x1215;
		while(*CONTROL & 0x3);
	}
	diag_printf("Value written\n");
}

#endif // CYGPKG_DEVS_ETH_SMSC_LAN91CXX
