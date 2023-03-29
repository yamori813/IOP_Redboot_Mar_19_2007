//==========================================================================
//
//      iq31244_pci.c
//
//      HAL support code for IQ31244 PCI
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
// Date:         2002-01-04
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#ifdef CYGPKG_IO_PCI

cyg_uint32 hal_pci_alloc_base_memory;
cyg_uint32 hal_pci_alloc_base_io;
cyg_uint32 hal_pci_physical_memory_base;
cyg_uint32 hal_pci_physical_io_base;
cyg_uint32 hal_pci_inbound_window_base;
cyg_uint32 hal_pci_inbound_window_mask;

//
// Verde ATU Window Usage:
//   Inbound Window 0 - Access to Verde memory mapped registers.
//   Inbound Window 1 - Used to reserve space for outbound window.
//   Inbound Window 2 - Access to SDRAM
//   Inbound Window 3 - Not used.
//
//   Direct Outbound Window - Direct mapped.
//   Outbound Translate Window 0 - Access to Inbound Window 1 PCI space
//   Outbound Translate Window 1 - Unused.
//   Outbound IO Window - Unused.
//


void
cyg_hal_plf_pci_init(void)
{
    cyg_uint32 dram_limit = (0xFFFFFFFF - (hal_dram_size - 1)) & 0xFFFFFFC0;
    cyg_uint8  next_bus;

    // Enable NIC through GPIO pin. This may not have an effect depending
    // on switch settings.
    *GPIO_GPOE &= ~(1 << IQ31244_GBE_GPIO_PIN);
    *GPIO_GPOD |= (1 << IQ31244_GBE_GPIO_PIN);

    hal_pci_inbound_window_mask = ~dram_limit;

    // Force BAR0 to be non-prefetchable to allow proper MU usage
    *ATU_IABAR0 &= ~CYG_PRI_CFG_BAR_MEM_PREFETCH;

#ifdef CYG_HAL_STARTUP_ROM
    *ATU_IATVR2 = SDRAM_PHYS_BASE;

    *ATU_IALR2 = dram_limit;

    // 64-bit prefetchable
    *ATU_IABAR2 = SDRAM_PHYS_BASE | \
                  CYG_PRI_CFG_BAR_MEM_TYPE_64 | \
                  CYG_PRI_CFG_BAR_MEM_PREFETCH;
    *ATU_IAUBAR2 = 0;

    *ATU_IALR1 = (0xFFFFFFFF - (_PCI_MEM_LIMIT - _PCI_MEM_BASE)) & 0xFFFFFFC0;

    // Outbound window will be set based on the memory reserved
    // by inbound window 1
    *ATU_IABAR1 = _PCI_MEM_BASE | \
	          CYG_PRI_CFG_BAR_MEM_TYPE_64 | \
                  CYG_PRI_CFG_BAR_MEM_PREFETCH;
    *ATU_IAUBAR1 = 0;
#endif  // CYG_HAL_STARTUP_ROM

    // allow ATU to act as a bus master, respond to PCI memory accesses,
    // and assert S_SERR#
    *ATU_ATUCMD = (CYG_PCI_CFG_COMMAND_SERR   | \
		   CYG_PCI_CFG_COMMAND_PARITY | \
		   CYG_PCI_CFG_COMMAND_MASTER | \
		   CYG_PCI_CFG_COMMAND_MEMORY);

    hal_pci_alloc_base_memory = *ATU_IABAR1 & CYG_PRI_CFG_BAR_MEM_MASK;
    hal_pci_alloc_base_io = _PCI_IO_BASE;

    hal_pci_inbound_window_base = *ATU_IABAR2 & CYG_PRI_CFG_BAR_MEM_MASK;

    // set the outbound window PCI address
    *ATU_OMWTVR0 = hal_pci_alloc_base_memory;
    *ATU_OUMWTVR0 = 0;

    // outbound I/O window
    *ATU_OIOWTVR = hal_pci_alloc_base_io;

    hal_pci_physical_memory_base = _PCI_MEM_BASE - hal_pci_alloc_base_memory;
    hal_pci_physical_io_base     = _PCI_IO_BASE - hal_pci_alloc_base_io;

    // Adjust for Virt - Phys in CPU space
    hal_pci_physical_memory_base += 0x20000000;
    hal_pci_physical_io_base     += 0x20000000;

    cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
    cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

    // enable outbound ATU
    *ATU_ATUCR = 2 | (1 << 8);
    *ATU_APMCSR = 3;

    next_bus = 1;
    cyg_pci_configure_bus(0, &next_bus);
}

#endif // CYGPKG_IO_PCI


