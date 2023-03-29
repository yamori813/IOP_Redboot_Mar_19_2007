//=============================================================================
//
//      pcitest.c
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
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter, Drew Moseley
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/io/pci.h>
#include "test_menu.h"

extern int memTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr);

extern cyg_uint32 hal_pci_cfg_flags;
//
// PCI Bus Test
//
// Simple scan of the PCI-X bus and report what is installed.
// If an Agilent2929 Exerciser/Analyzer card is installed, a memory
// test will be run using the Outbound ATU window to access the 2929
// onboard SDRAM.
void
pci_test (MENU_ARG arg)
{
	cyg_pci_device dev_info;
	cyg_pci_device_id devid, devid_2929;
	cyg_uint32 mem_size;
	cyg_uint32 *start, *end;
	int bus = iop34x_pcix_bus_number();
	int found = false;
	int bar_number = -1;
	int i;

	// Scan PCI-X and PCIe bus for devices hanging off of them
	diag_printf ("Scanning PCI Busses....\n");
	devid = CYG_PCI_DEV_MAKE_ID(bus, 0) | CYG_PCI_NULL_DEVFN;

	while(cyg_pci_find_next(devid, &devid))	{
		int i;

		cyg_pci_get_device_info(devid, &dev_info);

		diag_printf ("Device Found!\n");
		diag_printf ("Bus: %d, Device: %d, Function: %d\n", CYG_PCI_DEV_GET_BUS(devid), \
				CYG_PCI_DEV_GET_DEV(CYG_PCI_DEV_GET_DEVFN(devid)), \
				CYG_PCI_DEV_GET_FN(CYG_PCI_DEV_GET_DEVFN(devid)));

		diag_printf ("Vendor: 0x%04x  Device:  0x%04x\n", dev_info.vendor, dev_info.device);
		diag_printf ("Status: 0x%04x  Command: 0x%04x\n", dev_info.status, dev_info.command);
		// See if we found a Bridge
		if ((dev_info.header_type & CYG_PCI_CFG_HEADER_TYPE_MASK) == CYG_PCI_HEADER_BRIDGE) { 
			diag_printf ("Bridge Device\n");
			diag_printf ("  Primary/Sec/Sub Bus: %d/%d/%d\n", \
			    dev_info.header.bridge.pri_bus, dev_info.header.bridge.sec_bus, \
			    dev_info.header.bridge.sub_bus);
			for (i = 0; i < 2; i++) {
				diag_printf ("Bar[%d]: 0x%08x\n", i, (dev_info.base_address[i] & 0xFFFFFFF0));
			}

			diag_printf ("  MemBase        : 0x%04x\n", dev_info.header.bridge.mem_base);
			diag_printf ("  MemLimit       : 0x%04x\n", dev_info.header.bridge.mem_limit);
			if (dev_info.header.bridge.mem_base != 0 )  
				diag_printf ("  ClaimsMem      : [0x%08x to 0x%08x]\n", (dev_info.header.bridge.mem_base << 16), 
						(dev_info.header.bridge.mem_limit << 16) | 0xFFFFF);
			diag_printf ("  IOBase         : 0x%02x\n", (dev_info.header.bridge.io_base & ~0xf));
			diag_printf ("  IOLimit        : 0x%02x\n", (dev_info.header.bridge.io_limit & ~0xf));
			// 32 Bit I/O?
			if ((dev_info.header.bridge.io_base & 0x0f) == 0x01) {
				diag_printf ("  IOBaseUpper16  : 0x%04x\n", dev_info.header.bridge.io_base_upper16);
				diag_printf ("  IOLimitUpper16 : 0x%04x\n", dev_info.header.bridge.io_limit_upper16);
			}
			if (((dev_info.header.bridge.io_base & ~0xf) != 0 ) || (dev_info.header.bridge.io_base_upper16 != 0))
				diag_printf ("  ClaimsIO       : [0x%08x to 0x%08x]\n", \
				    (dev_info.header.bridge.io_base_upper16 << 16) | ((dev_info.header.bridge.io_base & ~0xf) << 8), 
					(dev_info.header.bridge.io_limit_upper16 << 16) | ((dev_info.header.bridge.io_limit & ~0xf) << 8) | 0xFFF);
		}
		// Not a bridge - just dump the BAR registers
		else  {		 
			diag_printf ("Endpoint Device\n");
			for (i = 0; i < 6; i++) {
				diag_printf ("Bar[%d]: 0x%08x\n", i, (dev_info.base_address[i] & 0xFFFFFFF0));
			}
		}
		diag_printf ("\n");

		if (dev_info.vendor == 0x15bc && dev_info.device == 0x2929) {
			diag_printf("Found Agilent 2929 in Slot.\n");
			devid_2929 = dev_info.devid;
			bar_number=0;
			cyg_pci_get_device_info(devid_2929, &dev_info);
			diag_printf ("2929 Mem starts at PCI address %p, CPU address %p, and the size is %p\n",
				dev_info.base_address[bar_number] & CYG_PRI_CFG_BAR_MEM_MASK,
				dev_info.base_map[bar_number], dev_info.base_size[bar_number]);

			start = (cyg_uint32 *)dev_info.base_map[bar_number];
			// 64KB test
			mem_size = 0x10000;
			end = start + mem_size/sizeof(*start) - 1;

			diag_printf("Testing memory from %p to %p.\n", start, end);
			memTest((CYG_ADDRWORD)start, (CYG_ADDRWORD)end);
			diag_printf ("Memory test done.\n");
		}
	}
}

