//=============================================================================
//
//      pcitest.c
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

#include <redboot.h>
#include <cyg/io/pci.h>
#include "test_menu.h"
#include <cyg/hal/hal_ioc80314_pci.h>
#include <cyg/hal/iq80315.h>


// Externs
extern int memTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr);
extern cyg_uint32 hal_pci_physical_memory_base;
extern cyg_uint32 hal_pci_physical_io_base;
extern CYG_ADDRWORD decIn( void);
extern void probe_sata(int slot, bool read_test, bool write_test);
extern void __delay_bsy(volatile cyg_uint8 *bar0);
extern void print_sata_info(cyg_uint16 *buf);

// Prototypes
void pci_test (MENU_ARG arg);
void probe_slot(int slot, cyg_bool test);
void print_info (cyg_pci_device *dev_info, int test_slot);

//
// PCI Bus Test
//
// This test assumes that an Agilent 2929 PCI-X board 
// is installed in both A1 and B2 slots.  The 2929s come
// installed with 1Meg of RAM.
void
pci_test (MENU_ARG arg) {
	cyg_uint32 choice;

	do {
		diag_printf ("\n\n");
		diag_printf ("=======================================\n");
		diag_printf ("|    PCI Test Menu for IQ80315         |\n");
		diag_printf ("=======================================\n");
		diag_printf ("| 1) Test Agilent 2929s - slots A1 & B2|\n");
		diag_printf ("| 2) Test Agilent 2929s - slots B1 & B2|\n");
		diag_printf ("| 3) Probe Slots A1, B1, and B2        |\n");
		diag_printf ("| 4) Probe SATA Disks                  |\n");
		diag_printf ("| 5) Write Test SATA Disks             |\n");
		diag_printf ("| 6) Read from SATA Disks              |\n");
		diag_printf ("---------------------------------------\n");
		choice = decIn();
		switch (choice) {
		case 1:
			probe_slot(__SLOT_A1, true);
			probe_slot(__SLOT_B2, true);
			break;
		case 2:
			probe_slot(__SLOT_B1, true);
			probe_slot(__SLOT_B2, true);
			break;
		case 3:
			probe_slot(__SLOT_A1, false);
			probe_slot(__SLOT_B1, false);
			probe_slot(__SLOT_B2, false);
			break;
		case 4:
			probe_sata(__SATA1, false, false);
			probe_sata(__SATA2, false, false);
			break;
		case 5:
			probe_sata(__SATA1, false, true);
			probe_sata(__SATA2, false, true);
			break;
		case 6:
			probe_sata(__SATA1, true, false);
			probe_sata(__SATA2, true, false);
			break;
		default:
			diag_printf ("Invalid selection\n");
		}
	} while (choice);
}

//Probe Slot will probe SLOT A1, B1, and B2 of the IQ80315
//and it can also be used to run a memtest on an Agilent 2929 (PCI-X)
//exerciser card.
//
//Inputs: Slot : what slot to test
//        test : If test is true, run a memtest on the agilent 2929 card
void probe_slot (int slot, cyg_bool test) {
	cyg_pci_device dev_info;
	cyg_pci_device_id devid;
	cyg_uint32 mem_size, test_slot;
	cyg_uint16 cmd, devfn;
	cyg_uint32 *start, *end;
	int bus;

	if (slot < __SLOT_B1) { // Testing Primary Side
		bus = ((*(cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_PCIX_STAT)) >> 8) & 0xff;
		hal_pci_physical_memory_base = _PCI_MEM_BASE;
		hal_pci_physical_io_base     = _PCI_IO_BASE;
		test_slot = slot;
	}
	else {
		test_slot = slot - __SATA2;   // B1 & B2 are slots 1/2 respectively on Secondary
		bus = ((*(cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PCI_PCIX_STAT)) >> 8) & 0xff;
		hal_pci_physical_memory_base = _PCI_MEM_BASE_SEC;
		hal_pci_physical_io_base     = _PCI_IO_BASE_SEC;
	}
	if (bus == 0xff)
		bus = 0;

	diag_printf ("\nProbing Bus: %d, Device: %d\n", bus, test_slot);

	devfn = (CYG_PCI_DEV_MAKE_DEVFN(test_slot, 0));
	devid = CYG_PCI_DEV_MAKE_ID(bus, devfn);
	cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND,
	&cmd);
	cmd &= ~(3);   // disable Mem and I/O space
	cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND, cmd);
	cyg_pci_get_device_info(devid, &dev_info);

	if (test == false) {
		print_info(&dev_info, slot);
	}
	else {
		if (dev_info.vendor != 0x15bc || dev_info.device != 0x2929) {
			diag_printf("No 2929 Exerciser in PCI-X slot\n");
			print_info(&dev_info, slot);
			return;
		}
		cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
		cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);
		cyg_pci_configure_device(&dev_info);
		diag_printf ("2929 DRAM starts at PCI address %p, CPU address %p\n",
		dev_info.base_address[0] & CYG_PRI_CFG_BAR_MEM_MASK,
		dev_info.base_map[0]);

		// enable memory space and bus master
		cyg_pci_read_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, &cmd);
		cmd |= (CYG_PCI_CFG_COMMAND_MEMORY | CYG_PCI_CFG_COMMAND_MASTER);
		cyg_pci_write_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, cmd);

		start = (cyg_uint32 *)dev_info.base_map[0];
		// 1MB test
		mem_size = 0x100000;
		end = start + mem_size/sizeof(*start) - 1;

		diag_printf("Testing memory from %p to %p.\n", start, end);
		memTest((CYG_ADDRWORD)start, (CYG_ADDRWORD)end);
		diag_printf ("Memory test done.\n");
	}
	// disable memory space and bus master
	cyg_pci_read_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, &cmd);
	cmd &= ~(CYG_PCI_CFG_COMMAND_MEMORY | CYG_PCI_CFG_COMMAND_MASTER);
	cyg_pci_write_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, cmd);

}


// Print information from the dev_info structure
void print_info (cyg_pci_device *dev_info, int test_slot) {
	char slot[10];

	switch (test_slot) {
		case __SLOT_A1:
			strcpy (slot, "A1");
			break;
		case __SLOT_B1:
			strcpy (slot, "B1");
			break;
		case __SLOT_B2:
			strcpy (slot, "B2");
			break;			
		case __SATA1:
			strcpy (slot, "SATA-A");
			break;			
		case __SATA2:
			strcpy (slot, "SATA-B");
			break;			
		default:
			diag_printf ("Invalid Slot Passed in: %d\n", test_slot);
			return;
	}
	if (dev_info->device == 0xffff) {
		diag_printf ("No Device is present in: %s\n\n", slot);
	}
	else {
		diag_printf ("--------------------------------------\n");
		diag_printf ("Device Information for: %s\n", slot);
		diag_printf ("Vendor ID: 0x%04x\n", dev_info->vendor);
		diag_printf ("Device ID: 0x%04x\n", dev_info->device);
		diag_printf ("Command  : 0x%04x\n", dev_info->command);
		diag_printf ("Status   : 0x%04x\n", dev_info->status);
		diag_printf ("Class+Rev: 0x%08x\n", dev_info->class_rev);
		diag_printf ("--------------------------------------\n\n");
	}
}

