#ifndef CYGONCE_HAL_ARM_IOP34X_VAR_IO_H
#define CYGONCE_HAL_ARM_IOP34X_VAR_IO_H

/*=============================================================================
//
//      var_io.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):      curt.e.bruns@intel.com
// Original data:  jskov
// Contributors:   drew.moseley@intel.com
// Date:           2004-12-10
// Purpose:        Platform specific support routines
// Description: 
// Usage:          #include <cyg/hal/hal_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

//-----------------------------------------------------------------------------

extern cyg_uint64 hal_pci_alloc_base_memory;
extern cyg_uint32 hal_pci_alloc_base_io;
extern cyg_uint64 hal_pci_physical_memory_base;
extern cyg_uint32 hal_pci_physical_io_base;
extern cyg_uint64 hal_pci_inbound_window_base;
extern cyg_uint64 hal_pci_inbound_window_mask;

#define HAL_PCI_PHYSICAL_MEMORY_BASE hal_pci_physical_memory_base
#define HAL_PCI_PHYSICAL_IO_BASE     hal_pci_physical_io_base

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY hal_pci_alloc_base_memory
#define HAL_PCI_ALLOC_BASE_IO     hal_pci_alloc_base_io

#include <cyg/hal/plf_io.h>

//-----------------------------------------------------------------------------

extern cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern cyg_uint16 cyg_hal_plf_pci_cfg_read_word  (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern cyg_uint8 cyg_hal_plf_pci_cfg_read_byte   (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
					     cyg_uint32 devfn,
					     cyg_uint32 offset,
					     cyg_uint32 val);
extern void cyg_hal_plf_pci_cfg_write_word  (cyg_uint32 bus,
					     cyg_uint32 devfn,
					     cyg_uint32 offset,
					     cyg_uint16 val);
extern void cyg_hal_plf_pci_cfg_write_byte   (cyg_uint32 bus,
					      cyg_uint32 devfn,
					      cyg_uint32 offset,
					      cyg_uint8 val);

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
    __val = cyg_hal_plf_pci_cfg_read_byte((__bus),  (__devfn), (__offset))
    
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_word((__bus),  (__devfn), (__offset))

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_dword((__bus),  (__devfn), (__offset))

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )  \
    cyg_hal_plf_pci_cfg_write_byte((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_word((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_dword((__bus),  (__devfn), (__offset), (__val))


// Memory mapping details
#ifndef CYGARC_PHYSICAL_ADDRESS
# define CYGARC_PHYSICAL_ADDRESS(x) (((unsigned long)x & 0x1FFFFFFF) + SDRAM_PHYS_BASE)
#endif
  
#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
extern void cyg_hal_plf_pci_minimal_initialization(void);
#define HAL_PCI_MINIMAL_INIT() cyg_hal_plf_pci_minimal_initialization()
#endif // defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)

#endif // CYGONCE_HAL_ARM_IOP34X_VAR_IO_H
// EOF var_io.h
