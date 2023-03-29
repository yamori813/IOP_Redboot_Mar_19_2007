//==========================================================================
//
//      mainstone_misc.c
//
//      HAL misc board support code for Intel XScale mainstone
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

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
#include <cyg/hal/hal_bulverde.h>       // Hardware definitions
#include <cyg/hal/mainstone.h>          // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#ifdef CYGPKG_IO_PCI
cyg_uint32 cyg_pci_window_real_base = 0;
#endif

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{
    *GPIO_GPSR0 = (1 << 12); // Set FLASH nWP (enable for writing)
    *GPIO_GPSR0 = (1 << 22); // Set FLASH VPEN (enable for writing)

    // Initialize platform interrupts
    *MAINSTONE_INTMSKENA = 0;
    *MAINSTONE_INTSETCLR = 0;
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_PLATFORM,1,0);
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_PLATFORM);
}

int
hal_plf_IRQ_handler(void)
{
    cyg_uint16 sources;
    int i;

    // Read sources and mask off reserved bits 8 and 12.
    sources = *MAINSTONE_INTSETCLR & 0xeeff;

    for (i = 0; i < 16; i++)
	if (sources & (1 << i))
	    return CYGNUM_HAL_INTERRUPT_MMC_CARD + i;

    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

void
hal_plf_interrupt_mask(int vector)
{
    cyg_uint16 val = *MAINSTONE_INTMSKENA & 0xeeff;
    cyg_uint16 mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_MMC_CARD);

    *MAINSTONE_INTMSKENA = val & ~mask;
}

void
hal_plf_interrupt_unmask(int vector)
{
    cyg_uint16 val = *MAINSTONE_INTMSKENA & 0xeeff;
    cyg_uint16 mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_MMC_CARD);

    *MAINSTONE_INTMSKENA = val | mask;
}

void
hal_plf_interrupt_acknowledge(int vector)
{
    cyg_uint16 val = *MAINSTONE_INTSETCLR & 0xeeff;
    cyg_uint16 mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_MMC_CARD);

    // First acknowledge the GPIO int
    HAL_INTERRUPT_ACKNOWLEDGE(CYGNUM_HAL_INTERRUPT_PLATFORM);

    // Now clear the platform interrupt
    *MAINSTONE_INTSETCLR = val & ~mask;
}

// ------------------------------------------------------------------------
// EOF mainstone_misc.c
