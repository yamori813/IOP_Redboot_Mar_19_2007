//==========================================================================
//
//      iop33x_pci.c
//
//      HAL support code for IOP331 and IOP332 PCI
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-07-10
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <pkgconf/io_pci.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_iop33x.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#ifdef CYGPKG_IO_PCI

cyg_uint64 hal_pci_alloc_base_memory;
cyg_uint32 hal_pci_alloc_base_io;
cyg_uint64 hal_pci_physical_memory_base;
cyg_uint32 hal_pci_physical_io_base;
cyg_uint64 hal_pci_inbound_window_base;
cyg_uint64 hal_pci_inbound_window_mask;

//
// IOP331/IOP332 ATU Window Usage:
//   Inbound Window 0 - Access to IOP331/IOP332 memory mapped registers.
//   Inbound Window 1 - Used to reserve space for outbound window.
//   Inbound Window 2 - Access to SDRAM
//   Inbound Window 3 - Not used.
//
//   Direct Outbound Window - Disabled
//
//   Outbound Translate Window 0 - Access to Inbound Window 1 PCI space
//   Outbound Translate Window 1 - Unused.
//   Outbound IO Window - Unused.
//

extern int hal_pci_cfg_flags;
bool cyg_hal_plf_ignore_devices(int bus, int device, int function)
{
    unsigned localbus = iop33x_localbus_number();
    if (localbus != bus)
        return false;
    else if ((device < 9) && ((hal_pci_cfg_flags & IOP33X_PCI_MASTER) == 0) && (hal_pci_cfg_flags & IOP33X_DEV_HIDING))
        return false; 
    else if ((device < 16) && (hal_pci_cfg_flags & IOP33X_PCI_MASTER))
        return false;
    else									
        return true;
}

#ifdef CYG_HAL_STARTUP_ROM
// Wait for BIOS to configure IOP331/IOP332 PCI.
// Returns true if BIOS done, false if timeout
bool
cyg_hal_plf_wait_for_bios(void)
{
    int delay = 400;  // 40 seconds, tops
    while (delay-- > 0) {
	if (*ATU_ATUCMD & CYG_PCI_CFG_COMMAND_MEMORY)
	    return true;
	hal_delay_us(100000);
    }
    return false;
}
#endif // CYG_HAL_STARTUP_ROM

void
cyg_hal_plf_pci_init(void)
{
    cyg_uint8 localbus, next_bus;
    cyg_uint64 pci_base = CYGPKG_IO_PCI_MEMORY_ALLOCATION_BASE;
    cyg_uint64 dram_limit;

#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_PRIVMEM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
    if ((hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) == 0)
        // We are not using opaque memory;  therefore we are sharing only a
        // limited amount of SDRAM over PCI.
        dram_limit = (~(cyg_uint64)0 - (hal_dram_pci_size - 1)) &~ 0x3f;
    else
        // We are using opaque memory;  therefore we are sharing the entire range
        // of SDRAM over PCI.
        dram_limit = (~(cyg_uint64)0 - (hal_dram_size - 1)) &~ 0x3F;
#else
    // We are not using opaque memory;  therefore we are sharing only a
    // limited amount of SDRAM over PCI.
    dram_limit = (~(cyg_uint64)0 - (hal_dram_pci_size - 1)) &~ 0x3f;
#endif
    hal_pci_inbound_window_mask = ~dram_limit;

#ifdef CYG_HAL_STARTUP_ROM
    // Wait for host BIOS configuration
    if (((hal_pci_cfg_flags & IOP33X_PCI_MASTER) == 0) && (cyg_hal_plf_wait_for_bios() == false))
    {
        // We thought we were a PCI slave. 
        // However there was no configuration by a Host BIOS.
        // Take over Master role
        hal_pci_cfg_flags |= IOP33X_PCI_MASTER;

        // If hiding is disabled we will not have assigned a limit
        // register for redirecting to downstream devices. We need to
        // do that here since we are now the PCI host and will be
        // ignoring device hiding
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_REDIRECT)
        HAL_WRITE_UINT32(ATU_IALR0, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_REDIRECT_PCI_SIZE));
#endif
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_REDIRECT)
        HAL_WRITE_UINT32(ATU_IALR1, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_REDIRECT_PCI_SIZE));
#endif
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_REDIRECT)
        HAL_WRITE_UINT32(ATU_IALR2, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_REDIRECT_PCI_SIZE));
#endif
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_REDIRECT)
        HAL_WRITE_UINT32(ATU_IALR3, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_REDIRECT_PCI_SIZE));
#endif
    }

    localbus = iop33x_localbus_number();	
    next_bus = CYGNUM_HAL_ARM_XSCALE_IOP_HIDDEN_BUS_NUMBER;	                

    // Setup our BARS first if we are the PCI Master
    if (hal_pci_cfg_flags & IOP33X_PCI_MASTER)
    {
        cyg_uint32 pxsr;

        // Set the localbus to 0 when we are the master
        HAL_READ_UINT32(ATU_PX_SR, pxsr);
        HAL_WRITE_UINT32(ATU_PX_SR, pxsr &~ ATU_PX_SR_BUS_NUM_MASK);

        // Use localbus+1 as the next bus when we are the master
        localbus = iop33x_localbus_number();	
        next_bus = localbus + 1;

        // Assign BAR0
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_PRIVMEM)
        if ((hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) == 0) {
            *ATU_IABAR0 = 0;
            *ATU_IAUBAR0 = 0;
        } else {
            *ATU_IABAR0 = 0 | (*ATU_IABAR0 &~ CYG_PRI_CFG_BAR_MEM_MASK);
            *ATU_IAUBAR0 = 0x80000000;
        }
#else
        pci_base = cyg_pci_boundary_align(pci_base, ATU_IALR_SIZE(*ATU_IALR0));
	*ATU_IABAR0 = (cyg_uint32)(pci_base & 0xFFFFFFFF) | (*ATU_IABAR0 &~ CYG_PRI_CFG_BAR_MEM_MASK);
	*ATU_IAUBAR0 = (cyg_uint32)(pci_base >> 32);
        pci_base += ATU_IALR_SIZE(*ATU_IALR0);
#endif

        // Assign BAR1
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM)
        if ((hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) == 0) {
            *ATU_IABAR1 = 0;
            *ATU_IAUBAR1 = 0;
        } else {
            *ATU_IABAR1 = 0 | (*ATU_IABAR1 &~ CYG_PRI_CFG_BAR_MEM_MASK);
            *ATU_IAUBAR1 = 0x80000000;
        }
#else
        pci_base = cyg_pci_boundary_align(pci_base, ATU_IALR_SIZE(*ATU_IALR1));
	*ATU_IABAR1 = (cyg_uint32)(pci_base & 0xFFFFFFFF) | (*ATU_IABAR1 &~ CYG_PRI_CFG_BAR_MEM_MASK);
	*ATU_IAUBAR1 = (cyg_uint32)(pci_base >> 32);
        pci_base += ATU_IALR_SIZE(*ATU_IALR1);
#endif

        // Assign BAR2
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM)
        if ((hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) == 0) {
            *ATU_IABAR2 = 0;
            *ATU_IAUBAR2 = 0;
        } else {
            *ATU_IABAR2 = 0 | (*ATU_IABAR2 &~ CYG_PRI_CFG_BAR_MEM_MASK);
            *ATU_IAUBAR2 = 0x80000000;
        }
#else
        pci_base = cyg_pci_boundary_align(pci_base, ATU_IALR_SIZE(*ATU_IALR2));
	*ATU_IABAR2 = (cyg_uint32)(pci_base & 0xFFFFFFFF) | (*ATU_IABAR2 &~ CYG_PRI_CFG_BAR_MEM_MASK);
	*ATU_IAUBAR2 = (cyg_uint32)(pci_base >> 32);
        pci_base += ATU_IALR_SIZE(*ATU_IALR2);
#endif

        // Assign BAR3
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
        if ((hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) == 0) {
            *ATU_IABAR3 = 0;
            *ATU_IAUBAR3 = 0;
        } else {
            *ATU_IABAR3 = 0 | (*ATU_IABAR3 &~ CYG_PRI_CFG_BAR_MEM_MASK);
            *ATU_IAUBAR3 = 0x80000000;
        }
#else
        pci_base = cyg_pci_boundary_align(pci_base, ATU_IALR_SIZE(*ATU_IALR3));
	*ATU_IABAR3 = (cyg_uint32)(pci_base & 0xFFFFFFFF) | (*ATU_IABAR3 &~ CYG_PRI_CFG_BAR_MEM_MASK);
	*ATU_IAUBAR3 = (cyg_uint32)(pci_base >> 32);
        pci_base += ATU_IALR_SIZE(*ATU_IALR3);
#endif
    }
    else
    {
        // Assign BAR3
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
        if ((hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) == 0) {
            *ATU_IABAR3 = 0;
            *ATU_IAUBAR3 = 0;
        } else {
            *ATU_IABAR3 = 0 | (*ATU_IABAR3 &~ CYG_PRI_CFG_BAR_MEM_MASK);
            *ATU_IAUBAR3 = 0x80000000;
        }
#else
        pci_base = cyg_pci_boundary_align(pci_base, ATU_IALR_SIZE(*ATU_IALR3));
	*ATU_IABAR3 = (cyg_uint32)(pci_base & 0xFFFFFFFF) | (*ATU_IABAR3 &~ CYG_PRI_CFG_BAR_MEM_MASK);
	*ATU_IAUBAR3 = (cyg_uint32)(pci_base >> 32);
        pci_base += ATU_IALR_SIZE(*ATU_IALR3);
#endif
    }
#endif  // CYG_HAL_STARTUP_ROM

    // allow ATU to act as a bus master, respond to PCI memory accesses,
    // and assert S_SERR#
    *ATU_ATUCMD = (CYG_PCI_CFG_COMMAND_SERR   | \
		   CYG_PCI_CFG_COMMAND_PARITY | \
		   CYG_PCI_CFG_COMMAND_MASTER | \
		   CYG_PCI_CFG_COMMAND_MEMORY);

    // Setup the PCI subsystem memory allocation pointer to point to
    // whichever BAR is redirected to hidden devices.
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_REDIRECT)
    hal_pci_alloc_base_memory = (((cyg_uint64)*ATU_IAUBAR0) << 32) | (*ATU_IABAR0 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_REDIRECT)
    hal_pci_alloc_base_memory = (((cyg_uint64)*ATU_IAUBAR1) << 32) | (*ATU_IABAR1 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_REDIRECT)
    hal_pci_alloc_base_memory = (((cyg_uint64)*ATU_IAUBAR2) << 32) | (*ATU_IABAR2 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_REDIRECT)
    hal_pci_alloc_base_memory = (((cyg_uint64)*ATU_IAUBAR3) << 32) | (*ATU_IABAR3 & CYG_PRI_CFG_BAR_MEM_MASK);
#else
    // Default is to assign new space.
    hal_pci_alloc_base_memory = pci_base;
#endif

    // Setup the PCI subsystem IO allocation pointer to the CDL configured value
    hal_pci_alloc_base_io = CYGPKG_IO_PCI_IO_ALLOCATION_BASE;

    // Setup the Virt-to-bus offset value to
    // whichever BAR is redirected to SDRAM
    if (hal_pci_cfg_flags & IOP33X_PCI_PRIVMEM) {
        // PRIVMEM is enabled; give preference to PRIVMEM bars
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_PRIVMEM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR0) << 32) | (*ATU_IABAR0 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR1) << 32) | (*ATU_IABAR1 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR2) << 32) | (*ATU_IABAR2 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR3) << 32) | (*ATU_IABAR3 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR0) << 32) | (*ATU_IABAR0 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR1) << 32) | (*ATU_IABAR1 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR2) << 32) | (*ATU_IABAR2 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR3) << 32) | (*ATU_IABAR3 & CYG_PRI_CFG_BAR_MEM_MASK);
#endif
    } else {
        // PRIVMEM is disabled; ignore PRIVMEM bars
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR0) << 32) | (*ATU_IABAR0 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR1) << 32) | (*ATU_IABAR1 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR2) << 32) | (*ATU_IABAR2 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM)
        hal_pci_inbound_window_base = (((cyg_uint64)*ATU_IAUBAR3) << 32) | (*ATU_IABAR3 & CYG_PRI_CFG_BAR_MEM_MASK);
#endif
    }

    // set the outbound window PCI addresses
    *ATU_OMWTVR0 = (cyg_uint32)(hal_pci_alloc_base_memory & 0xFFFFFFFF);
    *ATU_OUMWTVR0 = (cyg_uint32)(hal_pci_alloc_base_memory >> 32);
    *ATU_OMWTVR1 = (*ATU_OMWTVR0 + (PCI_OUTBOUND_MEM_WINDOW_1 - PCI_OUTBOUND_MEM_WINDOW_0));
    *ATU_OUMWTVR1 = *ATU_OUMWTVR0;

    // outbound I/O window
    *ATU_OIOWTVR = hal_pci_alloc_base_io;

    hal_pci_physical_memory_base = ((cyg_uint64)*ATU_OUMWTVR0 << 32) | 
        (cyg_uint64)(PCI_OUTBOUND_MEM_WINDOW_0 - *ATU_OMWTVR0);
    hal_pci_physical_io_base     = PCI_OUTBOUND_IO_WINDOW - *ATU_OIOWTVR;

    cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
    cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

    // enable outbound ATU
    *ATU_ATUCR = 2;

    *ATU_PM_CSR = 3;

    cyg_pci_configure_bus(localbus, &next_bus);

}

// Use "naked" attribute to suppress C prologue/epilogue
// This is a data abort handler which simply returns. Data aborts
// occur during configuration cycles if no device is present.
static void __attribute__ ((naked))
__pci_abort_handler(void) 
{
    asm ( "subs	pc, lr, #4\n" );
}

static cyg_uint32 orig_abort_vec;

#define DEBUG_CONFIG_VERBOSE 0

static inline void
pci_config_setup(cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint32 dev = CYG_PCI_DEV_GET_DEV(devfn);
    cyg_uint32 fn  = CYG_PCI_DEV_GET_FN(devfn);
    cyg_uint8 localbus = iop33x_localbus_number();
    cyg_uint8 pci_offset;
    cyg_uint32 pcie_offset;

    /*
     * Standard PCI Configuration Space Offset
     *     must be dword-aligned
     *     8 bits only
     */
    pci_offset = (cyg_uint8)(offset & 0xFC);

    /*
     * PCIE Extended Configuration Space Offset
     *     Bits 24-27 of the OCCAR specify the upper 4 bits of the 
     *     PCI extended configuration space
     */
    pcie_offset = (offset & 0xF00) << 16;
	
    /* Immediate bus use type 0 config, all others use type 1 config */
#if DEBUG_CONFIG_VERBOSE
    diag_printf("config: localbus[%d] bus[%d] dev[%d] fn[%d] offset[0x%x]\n",
                localbus, bus, dev, fn, offset);
#endif

    if (bus == localbus)
        *ATU_OCCAR = ( pcie_offset | (1 << (dev + 16)) | (dev << 11) | (fn << 8) | pci_offset | 0 );
    else
        *ATU_OCCAR = ( pcie_offset | (bus << 16) | (dev << 11) | (fn << 8) | pci_offset | 1 );

    orig_abort_vec = ((volatile cyg_uint32 *)0x20)[4];
    ((volatile unsigned *)0x20)[4] = (unsigned)__pci_abort_handler;
}

static inline int
pci_config_cleanup(cyg_uint32 bus)
{
    cyg_uint32 status = 0, err = 0;

    status = *ATU_ATUSR;
    if ((status & 0xF900) != 0) {
	err = 1;
	*ATU_ATUSR = status & 0xF900;
    }

    ((volatile unsigned *)0x20)[4] = orig_abort_vec;

    return err;
}


cyg_uint32
cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint32 config_data;
    int err;

    pci_config_setup(bus, devfn, offset);

    config_data = *ATU_OCCDR;

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config read dword: data[0x%x] err[%d]\n",
		config_data, err);
#endif
    if (err)
      return 0xffffffff;
    else
      return config_data;
}


void
cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
				 cyg_uint32 devfn,
				 cyg_uint32 offset,
				 cyg_uint32 data)
{
    int err;

    pci_config_setup(bus, devfn, offset);

    *ATU_OCCDR = data;

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write dword: data[0x%x] err[%d]\n",
		data, err);
#endif
}


cyg_uint16
cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
    cyg_uint16 config_data;
    int err;

    pci_config_setup(bus, devfn, offset &~ 3);

    config_data = (cyg_uint16)(((*ATU_OCCDR) >> ((offset % 0x4) * 8)) & 0xffff);

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config read word: data[0x%x] err[%d]\n",
		config_data, err);
#endif
    if (err)
      return 0xffff;
    else
      return config_data;
}

void
cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint16 data)
{
    int err;
    cyg_uint32 mask, temp;

    pci_config_setup(bus, devfn, offset &~ 3);

    mask = ~(0x0000ffff << ((offset % 0x4) * 8));

    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *ATU_OCCDR = (*ATU_OCCDR & mask) | temp; 

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write word: data[0x%x] err[%d]\n",
		data, err);
#endif
}

cyg_uint8
cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
    int err;
    cyg_uint8 config_data;

    pci_config_setup(bus, devfn, offset &~ 3);

    config_data = (cyg_uint8)(((*ATU_OCCDR) >> ((offset % 0x4) * 8)) & 0xff);

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config read byte: data[0x%x] err[%d]\n",
		config_data, err);
#endif
    if (err)
	return 0xff;
    else
	return config_data;
}


void
cyg_hal_plf_pci_cfg_write_byte (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint8 data)
{
    int err;
    cyg_uint32 mask, temp;

    pci_config_setup(bus, devfn, offset &~ 3);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *ATU_OCCDR = (*ATU_OCCDR & mask) | temp; 

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write byte: data[0x%x] err[%d]\n",
		data, err);
#endif
}

#endif // CYGPKG_IO_PCI

#include <redboot.h>
#ifdef CYGSEM_HAL_BUILD_MINIMAL_REDBOOT
// If we are not building a full RedBoot setup a function
// that will do a minimal PCI initialization configuration here.
void cyg_hal_plf_pci_minimal_initialization(void)
{
    static int pci_minimally_initialized = 0;

    if (pci_minimally_initialized == 0)
    {
        pci_minimally_initialized = 1;

        // Clear any outstanding ATU Interrupts
        *ATU_ATUSR = 0xFFFF;

        // enable outbound ATU
        *ATU_ATUCR |= ATUCR_OUTBOUND_ATU_ENABLE;

        // Enable the ATU as a bus master and memory space
        *ATU_ATUCMD |= (ATUCMD_BUS_MASTER_ENABLE | ATUCMD_MEM_SPACE_ENABLE);
    }
}
#endif

