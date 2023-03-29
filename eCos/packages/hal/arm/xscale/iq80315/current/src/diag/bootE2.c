//=============================================================================
//
//      bootE2.c - I2C EEPROM Diagnostics
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
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/************************************************************************/
/*									*/
/* Modification History							*/
/* --------------------							*/
/* 11oct00, ejb, Created for IQ80310 StrongARM2				*/
/* 18dec00  jwf                                                         */
/* 02feb01  jwf	added tests: _coy_tight_loop, cache_loop, LoopMemTest,  */
/*              special_mem_test written by snc	 		        */
/* 07feb01  jwf added function calls to a variable delay time generator */
/* 09feb01  jwf added function version_info to show version information */
/*              about OS, BOARD, CPLD, 80200 ID, 80312 ID.              */
/************************************************************************/


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
#include "test_menu.h"

char	port0_mac[6];
char	port1_mac[6];
struct {
		cyg_uint32 total_data_pairs;
		cyg_uint32 port0_addr_lower;
		cyg_uint32 port0_addr_upper;
		cyg_uint32 port1_addr_lower;
		cyg_uint32 port1_addr_upper;
}eeprom_mac_address;

void		  			read_eeprom_i2c (MENU_ARG arg);
void   					enet_setup (MENU_ARG arg);
void					write_eeprom_i2c (void);
void					erase_eeprom_i2c (void);
cyg_int32		 		read_i2c_byte (cyg_uint32 dev_id, cyg_uint16 byte);
cyg_int32				write_i2c_byte ( cyg_uint32 dev_id, cyg_uint16 byte, cyg_uint8 write_data );
cyg_uint32 				write_mac_address(cyg_uint32 selection, char *write_data);
static cyg_int32		read_mac_addresses(void);
static void				clear_mac_struct(void);
static void				make_room_eeprom (cyg_uint32 port);

extern CYG_ADDRWORD		hexIn(void);
extern CYG_ADDRWORD		decIn(void);
extern int				diag_ishex(char inChar);
extern int				diag_hex2dec(char inChar);

void
read_eeprom_i2c ( MENU_ARG arg ) {
	char buf[1];
	cyg_uint32 counter, device_id = 0x50;
	cyg_uint32 total_cycles = 0;
	cyg_uint32 address = 0, data = 0, a_cycle = 1;
	cyg_int32 read_data;

	for (counter = 0; counter < 2; counter++) {
		read_data = read_i2c_byte(device_id, counter);
		if (read_data < 0) {
			diag_printf ("Error when reading EEPROM...is it installed?\n");
			return;
		}
		total_cycles += (read_data << (8 - (8*counter)));
	}
	if (total_cycles == 0xffff)  {
		for (counter = 0; counter < 8; counter++) {
			read_data = write_i2c_byte(device_id, counter, 0);
			if (read_data < 0) {
				diag_printf ("Failed to Write...is I2C EEPROM installed?\n");
			}
		}
		total_cycles = 0;
	}

	diag_printf ("Total of %d address/data pairs in EEPROM\n", total_cycles);
	diag_printf ("Choose Read Method:\n");
	diag_printf ("1) Address Data Pairs\n");
	diag_printf ("2) Dump All Valid Bytes\n");
	diag_printf ("3) Dump Select Bytes\n");
	counter = decIn();

	switch (counter) {
	case 1:
		diag_printf ("Total of %d address/data pairs in EEPROM\n", total_cycles);
		counter = 8;  /* Start at the first address/data pair */
		while (total_cycles-- > 0)	{
			for (a_cycle=0; a_cycle < 4; a_cycle++) {  /* Address Bytes */
				data = 0;
				address |= ((read_i2c_byte(device_id, counter)) << (24 - 8*a_cycle));
				counter++;
			}
			diag_printf ("Address : 0x%08x\n", address);
			for (a_cycle=0; a_cycle < 4; a_cycle++) {  /* Data  Bytes*/
				address = 0;
				data |= ((read_i2c_byte(device_id, counter)) << (24 - 8*a_cycle));
				counter++;
			}
			diag_printf ("Data    : 0x%08x\n", data);
		}
		break;
	case 2:
		diag_printf ("Dumping %d bytes\n", ((total_cycles*8)+8));
		for (counter = 0; counter < ((total_cycles*8)+8); counter++) {
			data = read_i2c_byte(device_id, counter);
			diag_printf ("Byte 0x%03x : 0x%02x\n", counter, data);
		}
		break;
	case 3:
		while (1) {
			diag_printf ("What Address? (in hex; <Ctrl+C>=exit)>\n");
			counter = hexIn();
			if (counter == (cyg_uint32)_GETS_CTRLC)
				return;
			data = read_i2c_byte(device_id, counter);
			diag_printf ("Byte 0x%03x : 0x%02x\n", counter, data);
		}
	}
	diag_printf ("\nStrike <CR> to exit\n");
	while (_rb_gets(buf, sizeof(buf), 0) != _GETS_OK) ;
}

void
write_eeprom_i2c (void) {
	cyg_uint32 dev_id = 0x50;
	cyg_uint8 data;
	cyg_uint16 address;
	while (1) {
		diag_printf ("What byte location(hex)(2=exit)>\n");
		address = (hexIn() & 0x7ff);
		diag_printf ("You chose: 0x%x\n", address);
		if (address == 2) 
			return;
		diag_printf ("What data(hex)>\n");
		data = (hexIn() & 0xff);
		write_i2c_byte(dev_id, address, data);
	}
}

void
erase_eeprom_i2c (void) {
	char choice[1];
	cyg_int32 status;
	cyg_uint32 counter;
	cyg_uint32 dev_id = 0x50;
	cyg_uint8 all_zero =0x0, all_ff = 0xff;

	diag_printf ("ERASE EEPROM --- ARE YOU SURE (y/n)\n");
	while (_rb_gets(choice, sizeof(choice), 0) != _GETS_OK)
		;
	if ((choice[0] == 'y') || (choice[0] == 'Y')) {
		for (counter = 0; counter < 8; counter++) {
			status = write_i2c_byte(dev_id, counter, all_zero);
			if (status < 0) {
				diag_printf ("Failed to Write...is I2C EEPROM installed?\n");
				return;
			}
		}
		diag_printf ("Erasing...");
		for (counter = 8; counter < (2*1024); counter++) {
			status = write_i2c_byte(dev_id, counter, all_ff);
			if (!(counter % 100)) {
				diag_printf (".");
			}
			if (status < 0) {
				diag_printf ("Failed to Write...is I2C EEPROM installed?\n");
				return;
			}
		}
	}
}



cyg_int32 read_i2c_byte ( cyg_uint32 dev_id, cyg_uint16 byte ) {
	volatile unsigned int* p_i2c_cntrl1;
	volatile unsigned int* p_i2c_cntrl2;
	volatile unsigned int* p_i2c_rd_data;
	volatile unsigned int* p_i2c_wr_data;
	unsigned int  devcode, page_select;
	unsigned int  ctl_data = 0;

	p_i2c_cntrl1 = (volatile unsigned int*)I2C_CNTRL1;
	p_i2c_cntrl2 = (volatile unsigned int*)I2C_CNTRL2;
	p_i2c_rd_data= (volatile unsigned int*)I2C_RD_DATA;
	p_i2c_wr_data= (volatile unsigned int*)I2C_WR_DATA;
	page_select = ((dev_id & 0x78) >> 3);
	page_select |= (byte & 0x700);
	devcode     = ((dev_id & 0x7)  << 8);
	*p_i2c_cntrl1 = (page_select | devcode | ((byte & 0xff) << 16));
	*p_i2c_cntrl2 = 0x100;  // Read one byte 
	do {
		ctl_data = *p_i2c_cntrl2;
	} while (ctl_data & 0x100);
	if ((ctl_data >> 24) & 0xf)	{
		switch (ctl_data >> 24) {
		case 1:
			diag_printf ("AHB Master Transfer Error\n");
			break;
		case 2:
			diag_printf ("Write Protect Address Violation\n");
			break;
		case 4:
			diag_printf ("I2C Timeout - no device installed\n");
			break;
		case 8:
			diag_printf ("Init-State-Machine-Config-Error - Too many 8-byte reads\n");
			break;
		default:
			diag_printf ("Unknown I2C Error: ctl_data is: 0x%08X\n", ctl_data);
		}
		return (-1);  /* Failed to Read */
	}
	else {
		return(*p_i2c_rd_data & 0xff);
	}
}


cyg_int32 write_i2c_byte ( cyg_uint32 dev_id, cyg_uint16 byte, cyg_uint8 write_data ) {
	volatile unsigned int* p_i2c_cntrl1;
	volatile unsigned int* p_i2c_cntrl2;
	volatile unsigned int* p_i2c_rd_data;
	volatile unsigned int* p_i2c_wr_data;
	unsigned int  devcode, page_select;
	unsigned int  ctl_data = 0;

	p_i2c_cntrl1 = (volatile unsigned int*)I2C_CNTRL1;
	p_i2c_cntrl2 = (volatile unsigned int*)I2C_CNTRL2;
	p_i2c_rd_data= (volatile unsigned int*)I2C_RD_DATA;
	p_i2c_wr_data= (volatile unsigned int*)I2C_WR_DATA;
	page_select = ((dev_id & 0x78) >> 3);
	page_select |= (byte & 0x700);
	devcode     = ((dev_id & 0x7)  << 8);

	*p_i2c_cntrl1 = (I2C_WRITE | page_select | devcode | ((byte & 0xff)<< 16));
	*p_i2c_wr_data = write_data;
	*p_i2c_cntrl2 = 0x100;  // Write one byte
	do {
		ctl_data = *p_i2c_cntrl2;
	} while (ctl_data & 0x100);
	if ((ctl_data >> 24) & 0xf)	{
		switch (ctl_data >> 24) {
		case 1:
			diag_printf ("AHB Master Transfer Error\n");
			break;
		case 2:
			diag_printf ("Write Protect Address Violation\n");
			break;
		case 4:
			diag_printf ("I2C Timeout - no device installed\n");
			break;
		case 8:
			diag_printf ("Init-State-Machine-Config-Error - Too many 8-byte reads\n");
			break;
		default:
			diag_printf ("Unknown I2C Error: ctl_data is: 0x%08X\n", ctl_data);
		}
		return(-1);  /* Failed to write */
	}
	else {
		if (dev_id == 0x50) // Writing to EEPROM - 5mS cycle time
			CYGACC_CALL_IF_DELAY_US(5000); // 5mS cycle time for writes
		return(0);
	}
}


static cyg_int32
read_mac_addresses( void ) {
	cyg_int32 counter, counter1, counter2, read_data, device_id = 0x50;
	cyg_uint32 status, total_cycles = 0;
	cyg_uint32 address = 0, data = 0, a_cycle = 1, found = 0;

	for (counter1 = 1; counter1 >= 0; counter1--) {
		read_data = read_i2c_byte(device_id, counter1);
		if (read_data < 0) {
			diag_printf ("Error when reading EEPROM...is it installed?\n");
			return(-1);
		}
		total_cycles += (read_data << (8 - counter1*8));
	}
	eeprom_mac_address.total_data_pairs = total_cycles;
	if (total_cycles == 0xffff)  {
		for (counter = 0; counter < 8; counter++) {
			status = write_i2c_byte(device_id, counter, 0);
			if (status < 0) {
				diag_printf ("Failed to Write...is I2C EEPROM installed?\n");
				return(-1);
			}
		}
		total_cycles = 0;
	}
	diag_printf ("Total Data Pairs in EEPROM: 0x%0x\n", total_cycles);

	/* First Try to Find MAC Address in EEPROM */
	counter1 = 8;  /* Start at the first address/data pair */
	while (total_cycles-- > 0)	{
		for (a_cycle=0; a_cycle < 4; a_cycle++) {  /* Address Bytes */
			data = 0;
			address |= ((read_i2c_byte(device_id, counter1)) << (24 - 8*a_cycle));
			counter1++;
		}
		diag_printf ("Address: 0x%08x\n", address);
		if (address == 0xFFFE6040) {
			diag_printf ("Found PORT0 Upper Address in EEPROM at location: 0x%x\n", counter1);
			eeprom_mac_address.port0_addr_upper = (counter1);
			found = 1;
		}
		else if (address == 0xFFFE6044) {
			diag_printf ("Found PORT0 Lower Address in EEPROM at location: 0x%x\n", counter1);
			eeprom_mac_address.port0_addr_lower = (counter1);
			found = 2;
		}
		else if (address == 0xFFFE6440) {
			diag_printf ("Found PORT1 Upper Address in EEPROM at location: 0x%x\n", counter1);
			eeprom_mac_address.port1_addr_upper = (counter1);
			found = 3;
		}
		else if (address == 0xFFFE6444) {
			diag_printf ("Found PORT1 Lower Address in EEPROM at location: 0x%x\n", counter1);
			eeprom_mac_address.port1_addr_lower = (counter1);
			found = 4;
		}
		/* Now Get Data part of Add/Data Pair */
		for (a_cycle=0; a_cycle < 4; a_cycle++) {
			address = 0;
			data |= ((read_i2c_byte(device_id, counter1)) << (24 - 8*a_cycle));
			counter1++;
		}
		diag_printf ("Data: 0x%08x\n", data);
		if (found == 1) {
			for (counter2 = 0; counter2 < 4; counter2++) {
				port0_mac[counter2] = ((data >> (24 - counter2 * 8)) & 0xff);
			}
			found = 0;
		}
		else if (found == 2) {
			port0_mac[4] = (data >> 24) & 0xff;
			port0_mac[5] = (data >> 16) & 0xff;
			found = 0;
		}
		else if (found == 3) {
			for (counter2 = 0; counter2 < 4; counter2++)
				port1_mac[counter2] = (data >> ((24 - counter2 * 8) & 0xff));
			found = 0;
		}
		else if (found == 4) {
			port1_mac[4] = (data >> 24) & 0xff;
			port1_mac[5] = (data >> 16) & 0xff;
			found = 0;
		}
	}
	if (eeprom_mac_address.port0_addr_upper) {
		diag_printf ("Port0 MAC Address from EEPROM: ");
		for (counter1 = 5; counter1 > 0; counter1--)
			diag_printf ("%02x:", port0_mac[counter1]);
		diag_printf ("%02x\n", port0_mac[counter1]);
	}
	if (eeprom_mac_address.port1_addr_upper) {
		diag_printf ("Port1 MAC Address from EEPROM: ");
		for (counter1 = 5; counter1 > 0; counter1--)
			diag_printf ("%02x:", port1_mac[counter1]);
		diag_printf ("%02x\n", port1_mac[counter1]);
	}
	return(0);
}


/* Write_mac_address */
/* Routine to write MAC addresses into the I2C Boot EEPROM */
unsigned int
write_mac_address(cyg_uint32 port, char *mac_data) {
	cyg_int32 dev_id = 0x50, counter;
	cyg_uint16 addr_lower =0;
	cyg_uint16 addr_upper =0;


	for (counter = 0; counter < 6; counter++)
		diag_printf ("Mac[%d] : 0x%02x\n", counter, mac_data[counter]);

	switch (port) {
		case 0:			 
			addr_lower = eeprom_mac_address.port0_addr_lower;
			addr_upper = eeprom_mac_address.port0_addr_upper;
			break;
		case 1:
			addr_lower = eeprom_mac_address.port1_addr_lower;
			addr_upper = eeprom_mac_address.port1_addr_upper;
			break;
		default:
			diag_printf ("<write_mac_address> Port : %d invalid\n", port);
	}
	for (counter = 5; counter >= 2; counter--) {
		write_i2c_byte(dev_id, addr_upper++, mac_data[counter]);
	}
	for (counter = 1; counter >= 0; counter--) {
		write_i2c_byte(dev_id, addr_lower++, mac_data[counter]);
	}
	return(0);
}
		

// Get MAC address from user.
// Acceptable formats are:
//   nn.nn.nn.nn.nn.nn
//   nn:nn:nn:nn:nn:nn
//   nn-nn-nn-nn-nn-nn
//   nn nn nn nn nn nn
//   nnnnnnnnnnnn
static void
get_mac_address( char *addr_buf )
{
    char input[40], mac[6], *p;
    int got, i;

    do {
	got = 0;
	diag_printf ("\nCurrent MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		     addr_buf[5], addr_buf[4], addr_buf[3],
		     addr_buf[2], addr_buf[1], addr_buf[0]);
	diag_printf ("Enter desired MAC address: ");
	while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
	    ;

	p = input;
	while (*p == ' ')
		*p++;

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

static void
make_room_eeprom (cyg_uint32 port) {
	cyg_uint32 eeprom_address, eeprom_total_bytes;
	cyg_uint32 done = 0, dev_id = 0x50;
	cyg_int32  counter;

	eeprom_address = 0;
	diag_printf ("Checking if MAC address already in EEPROM\n");
	eeprom_total_bytes = (eeprom_mac_address.total_data_pairs + 1)*8;
	while (!done) {
		switch (port) {
		case 0:
			if (eeprom_mac_address.port0_addr_lower == 0) {
				diag_printf ("Port0 MAC Lower not in EEPROM\n");
				diag_printf ("Making room...\n");
				eeprom_address = 0xFFFE6044;
				eeprom_mac_address.port0_addr_lower = (eeprom_total_bytes + 4);
			}
			else if (eeprom_mac_address.port0_addr_upper == 0) {
				diag_printf ("Port0 MAC Upper not in EEPROM\n");
				diag_printf ("Making room...\n");
				eeprom_address = 0xFFFE6040;
				eeprom_mac_address.port0_addr_upper = (eeprom_total_bytes + 4);
			}
			else {
				eeprom_address = 0;
				done = 1;
			}
			break;
		case 1:
			if (eeprom_mac_address.port1_addr_lower == 0) {
				diag_printf ("Port1 MAC Lower not in EEPROM\n");
				diag_printf ("Making room...\n");
				eeprom_address = 0xFFFE6444;
				eeprom_mac_address.port1_addr_lower = (eeprom_total_bytes + 4);
			}
			else if (eeprom_mac_address.port1_addr_upper == 0) {
				diag_printf ("Port1 MAC Upper not in EEPROM\n");
				diag_printf ("Making room...\n");
				eeprom_address = 0xFFFE6440;
				eeprom_mac_address.port1_addr_upper = (eeprom_total_bytes + 4);
			}
			else {
				eeprom_address = 0;
				done = 1;
			}
			break;
		}
		if (!done) {
			for (counter = 3; counter >= 0; counter--) {
				write_i2c_byte(dev_id, eeprom_total_bytes++, ((eeprom_address >> (counter*8)) & 0xff));
			}
			eeprom_total_bytes += 4; /* Make room for data */
			eeprom_mac_address.total_data_pairs++;
			write_i2c_byte(dev_id, 0x1, (eeprom_mac_address.total_data_pairs & 0xff));
			write_i2c_byte(dev_id, 0x0, ((eeprom_mac_address.total_data_pairs >> 8) & 0xff));
		}
	}
}

static void
clear_mac_struct (void) {
	eeprom_mac_address.port0_addr_lower = 0;
	eeprom_mac_address.port0_addr_upper = 0;
	eeprom_mac_address.port1_addr_lower = 0;
	eeprom_mac_address.port1_addr_upper = 0;
	eeprom_mac_address.total_data_pairs = 0;
}

/* enet_setup */
/* Main function to Read/Write the MAC Addresses of the IOC80314 EEPROM */
void
enet_setup (MENU_ARG arg) {
	cyg_uint32 selection;
	cyg_int32  status;
	char *mac_data, buf[1];

	diag_printf ("IOC80314 Ethernet Configuration\n");
	clear_mac_struct();
	status = read_mac_addresses();
	if (status < 0) 
		return;
	diag_printf ("1) Set MAC Address for Port0\n");
	diag_printf ("2) Set MAC Address for Port1\n");

	selection = decIn();
	switch (selection) {
		case 1:
		case 2:
			(selection == 1) ? (mac_data = port0_mac) : (mac_data = port1_mac);
			get_mac_address(mac_data);
			make_room_eeprom(selection -1);
			write_mac_address((selection -1), mac_data);
			diag_printf ("*******************************************\n");
			diag_printf ("Must Reset Board for changes to take effect\n");
			diag_printf ("*******************************************\n");
			diag_printf ("\nStrike <CR> to exit\n");
			while (_rb_gets(buf, sizeof(buf), 0) != _GETS_OK) ;
			break;
		default:
			diag_printf ("Invalid Selection Made\n");
	}
	return;
}

