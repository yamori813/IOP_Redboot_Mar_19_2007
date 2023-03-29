#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-01-10
// Purpose:      Intel Mainstone IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/mainstone.h>
#include CYGBLD_HAL_PLF_INTS_H

// SDRAM is aliased as uncached memory for drivers.
#define CYGARC_UNCACHED_ADDRESS(_x_) \
  (((((unsigned long)(_x_)) >> 27)==0x0) ? (((unsigned long)(_x_))|0xA0000000) : (_x_))

#if 0
#define CYGARC_VIRT_TO_BUS(_x_) \
  (((unsigned long)(_x_) & 0x1fffffff) | hal_pci_inbound_window_base)
#define CYGARC_BUS_TO_VIRT(_x_) \
  (((unsigned long)(_x_) & hal_pci_inbound_window_mask) | 0xC0000000)
#endif

static inline unsigned cygarc_physical_address(unsigned va)
{
    unsigned *ram_mmutab = (unsigned *)(SDRAM_BASE | 0x4000);
    unsigned pte;

    pte = ram_mmutab[va >> 20];

    return (pte & 0xfff00000) | (va & 0xfffff);
}

#define CYGARC_PHYSICAL_ADDRESS(_x_) cygarc_physical_address(_x_)

static inline unsigned cygarc_virtual_address(unsigned pa)
{
    if (0xa0000000 <= pa && pa < 0xa8000000)
	return pa - 0xa0000000;
    return pa;
}

#define CYGARC_VIRTUAL_ADDRESS(_x_) cygarc_virtual_address(_x_)



//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
