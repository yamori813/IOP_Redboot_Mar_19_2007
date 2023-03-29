//=============================================================================
//
//      dmatest.c
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
// Date:        2003-06-25
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

typedef struct _dma {
    struct _dma *next;
    cyg_uint32  *pci_addr;
    cyg_uint32  *pci_upper_addr;
    cyg_uint32  *local_addr;
    cyg_uint32  byte_count;
    cyg_uint32  control;
} dma_desc_t;

#define DMA_CCR0    REG32(0,0xffffe400)
#define DMA_CSR0    REG32(0,0xffffe404)
#define DMA_DAR0    REG32(0,0xffffe40C)
#define DMA_NDAR0   REG32(0,0xffffe410)
#define DMA_PADR0   REG32(0,0xffffe414)
#define DMA_PUADR0  REG32(0,0xffffe418)
#define DMA_LADR0   REG32(0,0xffffe41C)
#define DMA_BCR0    REG32(0,0xffffe420)
#define DMA_DCR0    REG32(0,0xffffe424)

#define DMA_CSR_ACTIVE (1 << 10)
#define DMA_CSR_EOT    (1 << 9)
#define DMA_CSR_EOC    (1 << 8)
#define DMA_CSR_INT_ABORT         (1 << 5)
#define DMA_CSR_PCI_MASTER_ABORT  (1 << 3)
#define DMA_CSR_PCI_TARGET_ABORT  (1 << 2)
#define DMA_CSR_TRANS_ERROR       (1 << 1)

#define DMA_CSR_ERRORS (DMA_CSR_INT_ABORT|DMA_CSR_PCI_MASTER_ABORT|\
                        DMA_CSR_PCI_TARGET_ABORT|DMA_CSR_TRANS_ERROR)

#define DMA_XFER_SIZE 128

static void
dump_xfer_buf(cyg_uint8 *p)
{
    int i;

    diag_printf("DMA target buffer is offset 0 to 0x%x\n", DMA_XFER_SIZE - 1);
    diag_printf("Byte data should be (offset & 0xff)\n");
    diag_printf("    -0x10: ");
    for (i = 0, p = p - 16; i < 16; i++)
	diag_printf("%02x ", *p++);
    
    for (i = 0; i < (DMA_XFER_SIZE+16); i++) {
	if ((i%16) == 0)
	    diag_printf("\n    0x%03x: ", i);
	diag_printf("%02x ", *p);
	p++;
    }
    diag_printf("\n\n");  
    
    diag_printf("CCR[%08x] BCR[%d] PADR[%08x] LADR[%08x]\n\n",
		*DMA_CCR0, *DMA_BCR0, *DMA_PADR0, *DMA_LADR0);
}

//
// DMA Test
//
// This test assumes that an IQ80321 eval board is installed in the
// secondary PCI slot.
//
//
void
dma_test (MENU_ARG arg)
{
    cyg_pci_device dev_info;
    cyg_pci_device_id devid;
    cyg_uint32 *pci_mem, *pci_bus_mem;
    cyg_uint32 *local_mem, *local_phys;
    cyg_uint32 dma_desc_phys;
    cyg_uint8 *p;
    int bus, i, failed;
    dma_desc_t  *dma_desc;

    // Look for iq80321 in slot

    // iop821 is on other side of IBM bridge
    devid = CYG_PCI_DEV_MAKE_ID(0, CYG_PCI_DEV_MAKE_DEVFN(__SLOT, 0));
    cyg_pci_get_device_info(devid, &dev_info);

    // IBM bridge?
    if (dev_info.vendor != 0x1014 || dev_info.device != 0x01A7) {
	diag_printf("No iq80321 in PCI slot. (IBM Bridge)\n");
	return;
    }

    bus = dev_info.header.bridge.sec_bus;

    devid = CYG_PCI_DEV_MAKE_ID(bus, CYG_PCI_DEV_MAKE_DEVFN(12, 0));
    cyg_pci_get_device_info(devid, &dev_info);

    if (dev_info.vendor != 0x8086) {
	diag_printf("No iq80321 in PCI slot.\n");
	return;
    }

    cyg_pci_configure_device(&dev_info);

    diag_printf ("iq80321 DRAM starts at PCI address %p, CPU address %p\n",
		 dev_info.base_address[4] & CYG_PRI_CFG_BAR_MEM_MASK,
		 dev_info.base_map[4]);

    pci_mem     = (cyg_uint32 *)dev_info.base_map[4];
    pci_bus_mem = (cyg_uint32 *)(dev_info.base_address[4] & CYG_PRI_CFG_BAR_MEM_MASK);

    // skip over 1st Mbyte of target DRAM
    pci_mem += 0x100000/sizeof(cyg_uint32);
    pci_bus_mem += 0x100000/sizeof(cyg_uint32);

    local_mem = (cyg_uint32 *)0xC0100000;  // uncached
    local_phys = (cyg_uint32 *)0xA0100000;

    dma_desc = (dma_desc_t *)(local_mem + 0x100000);  // uncached
    dma_desc_phys = (cyg_uint32)(local_phys + 0x100000);

    // Local to PCI transfers.
    diag_printf("Testing local to PCI transfers...");
    failed = 0;
    
    for (i = 0, p = (cyg_uint8 *)local_mem; i < DMA_XFER_SIZE; i++)
	*p++ = i & 0xff;

    for (i = 0, p = (cyg_uint8 *)pci_mem - 16; i < (DMA_XFER_SIZE + 32); i++)
	*p++ = 0;

    dma_desc->next = NULL;
    dma_desc->pci_addr = pci_bus_mem;
    dma_desc->pci_upper_addr = NULL;
    dma_desc->local_addr = local_phys;
    dma_desc->byte_count = DMA_XFER_SIZE;
    dma_desc->control = 0x7;   // PCI mem write

    *DMA_CCR0 = 0;
    if (*DMA_CSR0 & DMA_CSR_ACTIVE) {
	diag_printf("failed!\nChannel already active.\n");
	failed = 1;
    } else {
	*DMA_NDAR0 = dma_desc_phys;
	*DMA_CCR0 = 1;

	for (i = 0; i < 10000; i++);

	while (*DMA_CSR0 & DMA_CSR_ACTIVE)
	    ;

	if (*DMA_CSR0 & DMA_CSR_ERRORS) {
	    diag_printf("failed!\nTransfer errors. CSR[0x%x].\n", *DMA_CSR0);
	    failed = 1;
	} else {
    
	    for (i = 0, p = (cyg_uint8 *)pci_mem; i < DMA_XFER_SIZE; i++)
		if (*p++ != (i & 0xff)) {
		    diag_printf("failed!\nData errors. offset[%d]\n", i);
		    dump_xfer_buf((cyg_uint8 *)pci_mem);
		    failed = 1;
		    break;
		}
	}
    }

    if (!failed)
	diag_printf("passed.\n");
    else
	diag_wait();

    // PCI to local transfers.
    diag_printf("Testing PCI to local transfers...");
    failed = 0;
    
    for (i = 0, p = (cyg_uint8 *)pci_mem; i < DMA_XFER_SIZE; i++)
	*p++ = i & 0xff;

    for (i = 0, p = (cyg_uint8 *)local_mem - 16; i < (DMA_XFER_SIZE + 32); i++)
	*p++ = 0;

    dma_desc->next = NULL;
    dma_desc->pci_addr = pci_bus_mem;
    dma_desc->pci_upper_addr = NULL;
    dma_desc->local_addr = local_phys;
    dma_desc->byte_count = DMA_XFER_SIZE;
    dma_desc->control = 0xe;   // PCI mem read

    *DMA_CCR0 = 0;
    if (*DMA_CSR0 & DMA_CSR_ACTIVE) {
	diag_printf("failed\nChannel already active.\n");
	failed = 1;
    } else {
	*DMA_NDAR0 = dma_desc_phys;
	*DMA_CCR0 = 1;

	for (i = 0; i < 10000; i++);

	while (*DMA_CSR0 & DMA_CSR_ACTIVE)
	    ;

	if (*DMA_CSR0 & DMA_CSR_ERRORS) {
	    diag_printf("failed\nTransfer errors. CSR[0x%x].\n", *DMA_CSR0);
	    failed = 1;
	} else {
    
	    for (i = 0, p = (cyg_uint8 *)local_mem; i < DMA_XFER_SIZE; i++)
		if (*p++ != (i & 0xff)) {
		    diag_printf("failed\nData errors. i[%d]\n", i);
		    dump_xfer_buf((cyg_uint8 *)local_mem);
		    failed = 1;
		    break;
		}
	}
    }

    if (!failed)
	diag_printf("passed.\n");
    else
	diag_wait();


    // PCI to local transfers.
    diag_printf("Testing local to local transfers...");
    failed = 0;
    
    for (i = 0, p = (cyg_uint8 *)local_mem; i < DMA_XFER_SIZE; i++)
	*p++ = i & 0xff;

    for (i = 0, p = (cyg_uint8 *)(local_mem + 0x1000) - 16; i < (DMA_XFER_SIZE + 32); i++)
	*p++ = 0;

    dma_desc->next = NULL;
    dma_desc->pci_addr = local_phys;
    dma_desc->pci_upper_addr = NULL;
    dma_desc->local_addr = local_phys + 0x1000;
    dma_desc->byte_count = DMA_XFER_SIZE;
    dma_desc->control = 0x4e;

    *DMA_CCR0 = 0;
    if (*DMA_CSR0 & DMA_CSR_ACTIVE) {
	diag_printf("failed\nChannel already active.\n");
	failed = 1;
    } else {
	*DMA_NDAR0 = dma_desc_phys;
	*DMA_CCR0 = 1;

	for (i = 0; i < 10000; i++);

	while (*DMA_CSR0 & DMA_CSR_ACTIVE)
	    ;

	if (*DMA_CSR0 & DMA_CSR_ERRORS) {
	    diag_printf("failed\nTransfer errors. CSR[0x%x].\n", *DMA_CSR0);
	    failed = 1;
	} else {
    
	    for (i = 0, p = (cyg_uint8 *)(local_mem + 0x1000); i < DMA_XFER_SIZE; i++)
		if (*p++ != (i & 0xff)) {
		    diag_printf("failed\nData errors. i[%d]\n", i);
		    dump_xfer_buf((cyg_uint8 *)(local_mem + 0x1000));
		    failed = 1;
		    break;
		}
	}
    }

    if (!failed)
	diag_printf("passed.\n");

    diag_printf("\n");
    diag_wait();
}

