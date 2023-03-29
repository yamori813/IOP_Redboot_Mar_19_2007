//==========================================================================
//
//      sata.c
//
//      HAL SATA routines for IQ80315
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter, dmoseley
// Date:         2003-08-10
// Purpose:      SATA diag for IQ80315
// Description:  IDE routines to be used with SATA diagnostic
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>

#ifdef CYGPKG_REDBOOT
#include <redboot.h>
#include <cyg/hal/iq80315.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <fs/ide.h>
#include <cyg/io/pci.h>
#include <cyg/hal/hal_ioc80314_pci.h>

// DPA Mode Register Indices
#define DPA_REG_DATA      0
#define DPA_REG_ERROR     4
#define DPA_REG_FEATURES  6
#define DPA_REG_COUNT     8
#define DPA_REG_LBALOW    0x0C
#define DPA_REG_LBAMID    0x10
#define DPA_REG_LBAHI     0x14
#define DPA_REG_DEVICE    0x18
#define DPA_REG_STATUS    0x1C
#define DPA_REG_COMMAND   0x1D


typedef struct  {
	bool port[4];
	bool found_at_least_one;
	bool dpa_mode;
	cyg_pci_device *dev_info;
	cyg_uint32 device;
}sata_t;

// Globals
static cyg_uint32 primary_cmd_base;
static cyg_uint32 secondary_cmd_base;

// Prototypes
void probe_sata (int slot, bool read_test, bool write_test);
int sata_id(int device, cyg_uint16 *buf, sata_t *drives);
int select_device(int device, sata_t *drives);
void reset_sata_ide(cyg_pci_device dev_info);
void reset_sata_dpa(cyg_pci_device dev_info);
void __delay_bsy(volatile cyg_uint8 *bar0);
void print_sata_info(cyg_uint16 *buf);
void write_test_drive (sata_t *drives, bool write_test);
void find_fail(cyg_uint16 *buf, cyg_uint16 *rbuf);

static void print_data(cyg_uint32* buf);
static inline void __wait_for_ready(int device, sata_t *drives);
static inline int  __wait_for_drq(int device, sata_t *drives);
static int read_sectors(int device, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf, sata_t *drives);
static int write_sectors(int device, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf, sata_t *drives);
static void flush_cache(int device, sata_t *drives);

// Externs
extern cyg_uint32 decIn(void);
extern void print_info (cyg_pci_device *dev_info, int test_slot);

// Initialize the IDE controller(s).
#define HAL_IDE_INIT()

#define HAL_IDE_READ_UINT8( sec,  __regno, __val )  \
	(sec == 1) ? ((__val) = *(volatile cyg_uint8 *)(secondary_cmd_base + (__regno))) : \
			((__val) = *(volatile cyg_uint8 *)(primary_cmd_base + (__regno)));

#define HAL_IDE_READ_UINT16( sec, __regno, __val )                                      \
	(sec == 1) ? ((__val) = *(volatile cyg_uint16 *)(secondary_cmd_base + (__regno/2))) : \
			((__val) = *(volatile cyg_uint16 *)(primary_cmd_base + (__regno/2)));

#define HAL_IDE_READ_ALTSTATUS( __val )  \
    (__val) = *(volatile cyg_uint8 *)(primary_cmd_base + 14)


#define HAL_IDE_WRITE_UINT8( sec, __regno, __val )  \
	(sec == 1) ? (*(volatile cyg_uint8 *)(secondary_cmd_base + (__regno)) = (__val)) : \
			(*(volatile cyg_uint8 *)(primary_cmd_base + (__regno)) = (__val));

#define HAL_IDE_WRITE_UINT16( sec, __regno, __val )                                 \
	(sec == 1) ? (*(volatile cyg_uint16 *)(secondary_cmd_base + (__regno/2)) = (__val)) : \
			(*(volatile cyg_uint16 *)(primary_cmd_base + (__regno/2)) = (__val));

#define HAL_IDE_WRITE_CONTROL( __val )  \
    *(volatile cyg_uint8 *)(primary_cmd_base + 14) = (__val)

// Macros for writing to SATA registers in DPA mode
#define HAL_DPA_READ_UINT8( port,  __regno, __val )  \
			(__val) = *(volatile cyg_uint8 *)(primary_cmd_base + (0x200 + port * 0x200) + (__regno));

#define HAL_DPA_READ_UINT16( port, __regno, __val )                                      \
			(__val) = *(volatile cyg_uint16 *)(primary_cmd_base + (0x200 + port * 0x200) + (__regno/2));

#define HAL_DPA_WRITE_UINT8( port, __regno, __val )  \
			*(volatile cyg_uint8 *)(primary_cmd_base + (0x200 + port * 0x200) + (__regno)) = (__val);

#define HAL_DPA_WRITE_UINT16( port, __regno, __val )                                 \
			(*(volatile cyg_uint16 *)(primary_cmd_base + (0x200 + port * 0x200) + (__regno/2)) = (__val));


static inline void
__wait_for_ready(int device, sata_t *drives)
{
    cyg_uint8 status;
	if (drives->dpa_mode == false) {
	    do {
			HAL_IDE_READ_UINT8(device, IDE_REG_STATUS, status);
	    } while (status & (IDE_STAT_BSY));
	}
	else {
	    do {
			HAL_DPA_READ_UINT8(device, DPA_REG_STATUS, status);
	    } while (status & (IDE_STAT_BSY));
	}
}

static inline int
__wait_for_drq(int device, sata_t *drives)
{
	cyg_uint8 status;
	cyg_uint32 timeout = 0xC000000;

	CYGACC_CALL_IF_DELAY_US(10);
	do {
		if (drives->dpa_mode == false) {
			HAL_IDE_READ_UINT8(device, IDE_REG_STATUS, status);
		}
		else {
			HAL_DPA_READ_UINT8(device, DPA_REG_STATUS, status);
		}
		timeout--;
		if (status & IDE_STAT_DRQ)
			return 0;
	} while ((status & IDE_STAT_BSY) && (timeout != 0));
	if (timeout == 0)
		diag_printf ("TIMED OUT!\n");
	else
		diag_printf ("<wait_for_drq> exiting - but didn't timeout\n");
	return 1;  // FAIL - no DRQ
}

// Return 1 for FAIL, 0 for PASS
int
sata_id(int device, cyg_uint16 *buf, sata_t *drives)
{
	unsigned int i;
	if (drives->dpa_mode == false) {
		(device > 1) ? (device = 1) : (device = 0);
		HAL_IDE_WRITE_UINT8(device, SUISCR, 0x1);  // Set Single Sector
		CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);

		HAL_IDE_WRITE_UINT8(device, IDE_REG_COMMAND, IDENTIFY_DEVICE);
		CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);

	}
	else {  // DPA Mode
		HAL_DPA_WRITE_UINT8(device, DPA_REG_COUNT, 0x1);  // Set Single Sector
		CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
		HAL_DPA_WRITE_UINT8(device, DPA_REG_COMMAND, IDENTIFY_DEVICE);
		CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
	}
	if (__wait_for_drq(device, drives)) {
		diag_printf ("%s wait_for_drq never returned\n", __FUNCTION__);
		return 1;
	}

	for (i=0; i < (SECTOR_SIZE/2); i++, buf++) {
		if (drives->dpa_mode == false) {
			HAL_IDE_READ_UINT16(device, IDE_REG_DATA, *buf);
		}
		else {
			HAL_DPA_READ_UINT16(device, DPA_REG_DATA, *buf);
		}
	}
	return 0;
}

static void
flush_cache(int device, sata_t *drives)
{
	if (drives->dpa_mode == false) {
		(device > 1) ? (device = 1) : (device = 0);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_COMMAND, FLUSH_CACHE);
	}
	else  {  // DPA Mode
		HAL_DPA_WRITE_UINT8(device, DPA_REG_COMMAND, FLUSH_CACHE);
	}
	CYGACC_CALL_IF_DELAY_US((cyg_uint32)50000);
	__wait_for_ready(device, drives);
}


static int
read_sectors(int device, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf, sata_t *drives)
{
	int  i, j;
	cyg_uint8 data;
	cyg_uint16 *p;

	if (drives->dpa_mode == false) {
		diag_printf ("Device input as: %d\n", device);
		(device > 1) ? (device = 1) : (device = 0);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_COUNT, count);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_LBALOW, start & 0xff);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_LBAMID, (start >>  8) & 0xff);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_LBAHI,  (start >> 16) & 0xff);
		HAL_IDE_READ_UINT8(device, IDE_REG_DEVICE,	data);
		data |= (((start >> 24) & 0xf) | 0x40);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_DEVICE,	data);
		HAL_IDE_WRITE_UINT8(device, IDE_REG_COMMAND, 0x20);
	}
	else {
		HAL_DPA_WRITE_UINT8(device, DPA_REG_COUNT, count);
		HAL_DPA_WRITE_UINT8(device, DPA_REG_LBALOW, start & 0xff);
		HAL_DPA_WRITE_UINT8(device, DPA_REG_LBAMID, (start >>  8) & 0xff);
		HAL_DPA_WRITE_UINT8(device, DPA_REG_LBAHI,  (start >> 16) & 0xff);
		HAL_DPA_READ_UINT8(device, DPA_REG_DEVICE,	data);
		data |= (((start >> 24) & 0xf) | 0x40);
		HAL_DPA_WRITE_UINT8(device, DPA_REG_DEVICE,	data);
		HAL_DPA_WRITE_UINT8(device, DPA_REG_COMMAND, 0x20);
	}


	for(p = buf, i = 0; i < count; i++) {
		if (__wait_for_drq(device, drives)) {
			diag_printf("%s: NO DRQ\n", __FUNCTION__);
			return 1;
		}
		for (j = 0; j < (SECTOR_SIZE/2); j++, p++) {
			if (drives->dpa_mode == false) {
				HAL_IDE_READ_UINT16(device, IDE_REG_DATA, *p);
			}
			else {
				HAL_DPA_READ_UINT16(device, DPA_REG_DATA, *p);
			}
		}
	}
	return 0;
}


static int
write_sectors(int device, cyg_uint32 start, cyg_uint8 count, cyg_uint16 *buf, sata_t *drives)
{
    int  i, j;
	cyg_uint8  data;
    cyg_uint16 *p;

	if (drives->dpa_mode == false) {
		(device > 1) ? (device = 1) : (device = 0);
	    HAL_IDE_WRITE_UINT8(device, IDE_REG_COUNT, count);
	    HAL_IDE_WRITE_UINT8(device, IDE_REG_LBALOW, start & 0xff);
	    HAL_IDE_WRITE_UINT8(device, IDE_REG_LBAMID, (start >>  8) & 0xff);
	    HAL_IDE_WRITE_UINT8(device, IDE_REG_LBAHI,  (start >> 16) & 0xff);
	    HAL_IDE_READ_UINT8(device, IDE_REG_DEVICE,	data);
		data |= (((start >> 24) & 0xf) | 0x40);
	    HAL_IDE_WRITE_UINT8(device, IDE_REG_DEVICE,	data);
	    HAL_IDE_WRITE_UINT8(device, IDE_REG_COMMAND, 0x30);
	}
	else {
	    HAL_DPA_WRITE_UINT8(device, DPA_REG_COUNT, count);   
		HAL_DPA_WRITE_UINT8(device, DPA_REG_LBALOW, start & 0xff);
	    HAL_DPA_WRITE_UINT8(device, DPA_REG_LBAMID, (start >>  8) & 0xff);
	    HAL_DPA_WRITE_UINT8(device, DPA_REG_LBAHI,  (start >> 16) & 0xff);
	    HAL_DPA_READ_UINT8(device, DPA_REG_DEVICE,	data);
		data |= (((start >> 24) & 0xf) | 0x40);
	    HAL_DPA_WRITE_UINT8(device, DPA_REG_DEVICE,	data);
	    HAL_DPA_WRITE_UINT8(device, DPA_REG_COMMAND, 0x30);
	}

    for(p = buf, i = 0; i < count; i++) {

        if (__wait_for_drq(device, drives)) {
            diag_printf("%s: NO DRQ\n", __FUNCTION__);
            return 1;
        }

        for (j = 0; j < (SECTOR_SIZE/2); j++, p++)	{
			if (drives->dpa_mode == false) {
	            HAL_IDE_WRITE_UINT16(device, IDE_REG_DATA, *p);
			}
			else {
				HAL_DPA_WRITE_UINT16(device, DPA_REG_DATA, *p);
			}
        }

	__wait_for_ready(device, drives);
    }
    return 0;
}

// Select a device 0/1 and either Primary/Secondary
int select_device(int device, sata_t *drives) {
	cyg_uint8 data;
	cyg_uint8 pri_sec; 

	if (drives->dpa_mode == false) {
		(device > 1) ? (pri_sec = 1) : (pri_sec = 0); // Primary or Secondary Controller
		__wait_for_ready(pri_sec, drives);
		HAL_IDE_WRITE_UINT8(pri_sec, IDE_REG_DEVICE, ((device % 2)<< 4));  // Device# of Controller
		CYGACC_CALL_IF_DELAY_US(100);
		HAL_IDE_READ_UINT8(pri_sec, IDE_REG_STATUS, data);
	}
	else {
		HAL_DPA_READ_UINT8(device, DPA_REG_STATUS, data);
		__wait_for_ready(device, drives);
		CYGACC_CALL_IF_DELAY_US(100);
		HAL_DPA_READ_UINT8(device, DPA_REG_STATUS, data);
	}
	if (data == 0x7f)  // No device attached
		return 1;
	else
		return 0;
}

void probe_sata (int slot, bool read_test, bool write_test) {
	cyg_pci_device dev_info;
	cyg_pci_device_id devid;
	cyg_uint16 cmd, devfn;
	cyg_uint8 led;
    cyg_uint32 buf[SECTOR/sizeof(cyg_uint32)];
	cyg_uint32 counter, status;
	int bus;
	sata_t drives;
	
	drives.found_at_least_one = false;
	drives.dpa_mode = false;

	bus = ((*(cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_PCIX_STAT)) >> 8) & 0xff;
	hal_pci_physical_memory_base = _PCI_MEM_BASE;
	hal_pci_physical_io_base     = _PCI_IO_BASE;
	cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
	cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

	if (bus == 0xff)
		bus = 0;
	if (!write_test) {
		diag_printf ("\n==========================================\n");
		diag_printf ("Probing Bus: %d, Device: %d\n", bus, slot);
	}

	devfn = (CYG_PCI_DEV_MAKE_DEVFN(slot, 0));
	devid = CYG_PCI_DEV_MAKE_ID(bus, devfn);
	cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND, &cmd);
	cmd &= ~(3);   // disable Mem and I/O space
	cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND, cmd);
	cyg_pci_get_device_info(devid, &dev_info);

	if (((dev_info.class_rev >> 16) & 0xff) == 0x1)  {// ATA Subclass
		diag_printf ("%s is in ATA Mode\n", (slot == __SATA1) ? "SATA-A" : "SATA-B");
		drives.dpa_mode = false;
	}
	else {
		diag_printf ("%s is in DPA Mode\n", (slot == __SATA1) ? "SATA-A" : "SATA-B");
		drives.dpa_mode = true;
//		print_info(&dev_info, slot);
//		diag_printf ("---> Must be in ATA Mode to run read/write diagnostic <---\n");
//		return;
	}

	/* Configure 31244 */
	cyg_pci_configure_device(&dev_info);
	if (drives.dpa_mode == false) {
		primary_cmd_base = ((dev_info.base_address[0] & (~(0x7))) | XS_PCI1_IO_BASE);     // Bar0 in ATA Mode
		secondary_cmd_base = ((dev_info.base_address[2] & ~(0x7)) | XS_PCI1_IO_BASE);   // Bar2 in ATA Mode
	}
	else
		primary_cmd_base = ((dev_info.base_address[0] & (~(0x7))) | XS_PCI1_MEM32_BASE);

//	for (counter = 0; counter < 6; counter++) 
//		diag_printf ("BAR[%d] is 0x%08x\n", counter, dev_info.base_address[counter]);
//	diag_printf ("Primary cmd: 0x%08x\n", primary_cmd_base);
//	diag_printf ("Secondary cmd: 0x%08x\n", secondary_cmd_base);

	/* Enable 31244 */
	cyg_pci_read_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, &cmd);
	cmd |= (CYG_PCI_CFG_COMMAND_IO | CYG_PCI_CFG_COMMAND_MEMORY);
	cyg_pci_write_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, cmd); 

	// Set LED config register to Reflect Ch0 and Ch1 activity
	cyg_pci_read_config_uint8(dev_info.devid, 0x9b, &led);
	led &= ~(0x10);  
	cyg_pci_write_config_uint8(dev_info.devid, 0x9b, led); 
	drives.dev_info = &dev_info;

	if (drives.dpa_mode == false) 
		reset_sata_ide(dev_info);  // Per C-spec update of 31244 - send reset to 31244s
	else
		reset_sata_dpa(dev_info);

	// Check if disk found, and if so print out some info from it
	for (counter = 0; counter < DEVICES_PER_31244; counter++) { // Check each device 0/1 on 31244 for disk
		status = select_device(counter, &drives);
		if (status) {
			if (!write_test)
				diag_printf ("\nNo Disk attached to Port: %d\n", counter);
			drives.port[counter] = false;
		}
		else {
			if (!write_test)
				diag_printf ("\nReading Disk for Port: %d\n", counter);
			if (sata_id(counter, (cyg_uint16 *)buf, &drives)) {
				if (!write_test)
					diag_printf ("Could Not Read Drive Data...\n");
				drives.port[counter] = false;
			}
			else {
				if (!write_test)
					print_sata_info((cyg_uint16*)buf);
				drives.port[counter] = true;
				drives.found_at_least_one = true;
				drives.device = counter;
			}
		}
	}
	if (write_test)
		write_test_drive(&drives, true);
	else if (read_test)
		write_test_drive(&drives, false);

	cyg_pcihw_read_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND, &cmd);
	cmd &= ~(3);   // disable Mem and I/O space
	cyg_pcihw_write_config_uint16(bus, devfn, CYG_PCI_CFG_COMMAND, cmd);

	//print_info(&dev_info, slot);
}

// Send a Hard and Soft reset to the 31244 controllers in IDE Mode
void reset_sata_ide(cyg_pci_device dev_info) {
	volatile cyg_uint8  *bar0 = (cyg_uint8*)(XS_PCI1_BASE_ADDR + (dev_info.base_address[0] & 0xFFFFFFF8));
	volatile cyg_uint8  *bar1 = (cyg_uint8*)(XS_PCI1_BASE_ADDR + (dev_info.base_address[1] & 0xFFFFFFF8));
	volatile cyg_uint8  *bar2 = (cyg_uint8*)(XS_PCI1_BASE_ADDR + (dev_info.base_address[2] & 0xFFFFFFF8));
	volatile cyg_uint8  *bar3 = (cyg_uint8*)(XS_PCI1_BASE_ADDR + (dev_info.base_address[3] & 0xFFFFFFF8));
	volatile cyg_uint32 *bar5 = (cyg_uint32*)(XS_PCI1_BASE_ADDR + (dev_info.base_address[5] & 0xFFFFFFF8));
	volatile cyg_uint8  *cmd  = bar0;
	volatile cyg_uint8  *cntl = bar1;
	cyg_uint32 count;


	// Reset all 4 ports of a 31244 device
	for (count = 0; count < 4; count++, cmd = bar2, cntl = bar3) {
		*(cmd + SUIDHR) = (0xa0 | ((count % 2) << 4));  // Device0, then Device 1 hard reset
		CYGACC_CALL_IF_DELAY_US(100);
		*(bar5 + (SUPDSSCR/4)) = 0x0;  	// Hard Reset
		*(bar5 + (SUPDSSCR/4)) = 0x1;
		__delay_bsy(cmd);
		*(bar5 + (SUPDSSCR/4)) = 0x0;
		__delay_bsy(cmd);
		CYGACC_CALL_IF_DELAY_US(10000);

		*(cntl + SUIDCR) = 0x4;		// Soft Reset for device
		CYGACC_CALL_IF_DELAY_US(1000);
		*(cntl + SUIDCR) = 0x0;		// Clear Reset
		CYGACC_CALL_IF_DELAY_US(1000);
		__delay_bsy(cmd);			// Wait for Reset to finish
	}
}

// Send a Hard and Soft reset to the 31244 controllers in DPA mode
void reset_sata_dpa(cyg_pci_device dev_info) {
	volatile cyg_uint8  *bar0 = (cyg_uint8*)(XS_PCI1_BASE_ADDR + (dev_info.base_address[0] & 0xFFFFFFF8));
	volatile cyg_uint8  *supdsscr  = (bar0 + 0x308);  // DPA SControl Register
	volatile cyg_uint8  *cmd   = (bar0 + 0x215);
	volatile cyg_uint8  *cntl  = (bar0 + 0x229);
	cyg_uint32 count;

	// Reset all 4 ports of a 31244 device
	for (count = 0; count < 4; count++, cmd +=0x200, supdsscr+=0x200, cntl+=0x200) {
		if (*supdsscr == 0x4) {  // No init done yet
			diag_printf ("...Port%d currently offline...reviving..\n", count);
			*supdsscr = 0x0;  	// Start Init sequence in DPA Mode
			CYGACC_CALL_IF_DELAY_US(1*1000*2000);  // Wait 2sec
		}
		else {  // Hard Reset this puppy
			*supdsscr = 0x0;
			CYGACC_CALL_IF_DELAY_US(10000);
			*supdsscr = 0x1;
			__delay_bsy(cmd);
			*(supdsscr) = 0x0;
			__delay_bsy(cmd);
			CYGACC_CALL_IF_DELAY_US(10000);
		}

		*cntl = 0x4;		// Soft Reset for device
		CYGACC_CALL_IF_DELAY_US(1000);
		*cntl = 0x0;		// Clear Reset
		CYGACC_CALL_IF_DELAY_US(1000);
		__delay_bsy(cmd);  	// Wait for Reset to finish
	}
}

// Wait for BSY to clear
void __delay_bsy(volatile cyg_uint8 *bar) {
	cyg_uint32 counter = 0;
	cyg_uint32 threshold = 0x100000;
	do {
		counter++;
		CYGACC_CALL_IF_DELAY_US(10000);
		if (counter > threshold)
			break;
	}while ((*(bar + 0x7)) & 0x80);
	if (counter > threshold) {
		diag_printf ("<delay_bsy> BUSY NEVER CLEARED!\n");
		diag_printf ("0x%02x\n", (*(bar + 0x7)));
	}
}

//Print ATA info retrieved from the IDENTIFY DEVICE Command
void
print_sata_info(cyg_uint16 *buf) {
	int i;
	cyg_uint8  data8u, data8l;
	cyg_uint16 data16;
	cyg_uint64 lba_sectors;
	cyg_uint64 total_gbytes;

	// Print out Serial Number 
	diag_printf ("Drive S/N: ");
	for (i = 10; i < 20; i++) {
		data16 = *(buf + i);
		data8u = ((data16 & 0xff00) >> 8);
		data8l = (data16 & 0xff);
		if (data8u != 0x20)
			diag_write_char(data8u);
		if (data8l != 0x20)
			diag_write_char(data8l);
		else
			break; // No more SN data
  	}

	// Print out F/W Rev
	diag_printf ("\nDrive F/W: ");
	for (i = 23; i < 27; i++) {
		data16 = *(buf + i);
		data8u = ((data16 & 0xff00) >> 8);
		data8l = (data16 & 0xff);
		if (data8u != 0x20)
			diag_write_char(data8u);
		if (data8l != 0x20)
			diag_write_char(data8l);
		else
			break; // No more data
  	}

	// Print out Model#
	diag_printf ("\nDrive Model#: ");
	for (i = 27; i < 47; i++) {
		data16 = *(buf + i);
		data8u = ((data16 & 0xff00) >> 8);
		data8l = (data16 & 0xff);
		if (data8u != 0x20)
			diag_write_char(data8u);
		if (data8l != 0x20)
			diag_write_char(data8l);
		else
			break; // No more data
  	}

	// Print Out Capacity of Device
	lba_sectors = *(buf + 100);
	lba_sectors |= (*(buf + 101) << 16);
	lba_sectors |= (*(buf + 102) << 32);
	lba_sectors |= (*(buf + 103) << 48);
	total_gbytes = ((lba_sectors*.512)/1000000);
	diag_printf ("\nTotal capacity: %u Gbytes\n", total_gbytes);
//	for (i = 0; i < 256; i++) {
//		diag_printf ("Word:%03d  Data:0x%04x\n", i, *buf++);
//	}
}

void write_test_drive (sata_t *drives, bool write_test) {
	int counter, selection, result;
	cyg_uint32 deviceid, counter2;
	bool done = false, fail;
	cyg_uint16 buf[SECTOR/sizeof(cyg_uint16)];
	cyg_uint16 rbuf[SECTOR/sizeof(cyg_uint16)];
	cyg_uint16 data_patterns[] = {(0xffff), (0x0000), (0xa5a5), (0x5a5a), (0xc3c3), (0x3c3c)};

	deviceid = drives->dev_info->devid;
	if (drives->found_at_least_one == false) {
		diag_printf ("No drives on Bus: %d, device: %d\n", ((deviceid >> 16) & 0xff) , ((deviceid>>11)&0xf));
	}
	else {
		while (!done) {
			diag_printf ("==========================================\n");
			diag_printf ("Bus: %d, Device %d\n", ((deviceid>>16) & 0xff), ((deviceid>>11) & 0xf));
			diag_printf ("   Select Port to %s Test       \n", ((write_test == true) ? "Write" : "Read"));
			if (write_test == true) {
				diag_printf ("WARNING - THIS DESTROYS DATA ON DISK\n");
				diag_printf ("   DO NOT USE ON ANY DATA YOU NEED\n");
			}
			diag_printf ("==========================================\n");
			for (counter = 0; counter < DEVICES_PER_31244; counter++) {
				diag_printf ("%d) Port%d %s\n", counter+1, counter, \
				(drives->port[counter] == false) ? "- No drive attached" : " ");
			}
			selection = decIn();
			if ((selection <= 0) || (selection > DEVICES_PER_31244))
				return;
			selection -= 1;
			if (drives->port[selection] == false) {
				diag_printf ("Sorry - no drive attached\n");
			}
			else {
				diag_printf ("Testing Port: %d with Device=%d\n", selection, drives->device);
				select_device(selection, drives);
				fail = false;
				for (counter2 = 0; counter2 < ((sizeof(data_patterns))/(sizeof(data_patterns[0]))); counter2++) {
					if (!write_test) {
						diag_printf ("Reading LBA: %d\n", counter2);
						read_sectors(selection, counter2, 1, (cyg_uint16*)rbuf, drives);
						print_data((cyg_uint32*)rbuf);
					}
					else {
						diag_printf ("Writing LBA: %d with pattern: 0x%02x\n", counter2, data_patterns[counter2]);
						memset((void*)buf, data_patterns[counter2], SECTOR_SIZE);
						write_sectors(selection, counter2, 1, (cyg_uint16*)buf, drives);
						flush_cache(selection, drives);
						read_sectors(selection, counter2, 1, (cyg_uint16*)rbuf, drives);
						result = memcmp((void*)buf, (void*)rbuf, SECTOR_SIZE);
						if (result != 0) {
							fail = true;
							find_fail(buf, rbuf);
							diag_printf ("*******************************\n");
							diag_printf ("   FAILED WRITE DRIVE TEST     \n");
							diag_printf ("   LBA: %d, Pattern: 0x%02x\n", counter2, data_patterns[counter2]);
							diag_printf ("*******************************\n");
							break;
						}
						// Set all LBA to Port# to verify correct mapping
						memset((void*)buf, selection, SECTOR_SIZE);
						write_sectors(selection, counter2, 1, (cyg_uint16*)buf, drives);
					}
				}
				if ((write_test) && (fail == false)) {
					diag_printf ("+++++++++++++++++++++++++++++++\n");
					diag_printf ("   PASSED WRITE DRIVE TEST     \n");
					diag_printf ("+++++++++++++++++++++++++++++++\n");

				}
			}
		}
	}
}

void find_fail(cyg_uint16 *buf, cyg_uint16 *rbuf) {
	int counter, fails =0;

	for (counter = 0; counter < SECTOR_SIZE/2; counter++) {
		if (*buf != *rbuf) {
			diag_printf ("Expected: 0x%04x, Found: 0x%04x\n", *buf, *rbuf);
			fails++;
		}
		if (fails > 10) {
			diag_printf ("Found more than 10 FAILS - bailing out\n");
			return;
		}
		*buf++;
		*rbuf++;
	}
}

void print_data(cyg_uint32* buf) {
	int counter;
	for (counter = 1; counter <= (SECTOR_SIZE/4); counter++) {
		diag_printf ("0x%04x: 0x%08x ", ((counter-1)*4), *buf);
		if ((!(counter % 4)) && (counter > 0))
			diag_printf ("\n");
	}
	diag_printf ("\n");
}

#endif

