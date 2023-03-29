//==========================================================================
//
//      ioc80314_pci.c
//
//      HAL support code for Verde PCI
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
// Contributors: msalter, , dkranak, cebruns
// Date:         2002-01-30
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
#include <cyg/hal/hal_ioc80314_pci.h>

#ifdef CYGPKG_IO_PCI


#define ORIGINALMAP

 /*----------             Map table for CIU SF BAR2 to PCI1          ----------*/
const bar_look_up_entry_t xs_pci_bar2[CIU_LUT_ENTRIES] = {
     // LUT for the PCI1 (32 entries)
     // 512MB (16 entries) mapped into the PCI1 MEM32 Space (including PCI Config and I/O)
{(PCI1_MEM32_OFFSET + 0x00000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x02000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x04000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x06000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},

{(PCI1_MEM32_OFFSET + 0x08000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x0A000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x0C000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x0E000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},

{(PCI1_MEM32_OFFSET + 0x10000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x12000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x14000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x16000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},

{(PCI1_MEM32_OFFSET + 0x18000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x1A000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x1C000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},
{(PCI1_MEM32_OFFSET + 0x1E000000) | PORT_PCI1,  PCI1_MEM32_OFFSET_UPPER},

// 256MB (8 entries) mapped into the PCI1 PFM1 Space - will equate to 0x4000.0000+
{(PCI1_PFM1_OFFSET + 0x20000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},
{(PCI1_PFM1_OFFSET + 0x22000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},
{(PCI1_PFM1_OFFSET + 0x24000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},
{(PCI1_PFM1_OFFSET + 0x26000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},

{(PCI1_PFM1_OFFSET + 0x28000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},
{(PCI1_PFM1_OFFSET + 0x2A000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},
{(PCI1_PFM1_OFFSET + 0x2C000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},
{(PCI1_PFM1_OFFSET + 0x2E000000) | PORT_PCI1,  PCI1_PFM1_OFFSET_UPPER},

// 256MB (8 entries) mapped into the PCI1 PFM2 Space - will equate to 0x8000.0000+
{(PCI1_PFM2_OFFSET + 0x50000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},
{(PCI1_PFM2_OFFSET + 0x52000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},
{(PCI1_PFM2_OFFSET + 0x54000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},
{(PCI1_PFM2_OFFSET + 0x56000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},

{(PCI1_PFM2_OFFSET + 0x58000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},
{(PCI1_PFM2_OFFSET + 0x5A000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},
{(PCI1_PFM2_OFFSET + 0x5C000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER},
{(PCI1_PFM2_OFFSET + 0x5E000000) | PORT_PCI1,  PCI1_PFM2_OFFSET_UPPER}
};

 /*----------             Map table for CIU SF BAR3 to PCI2          ----------*/
const bar_look_up_entry_t xs_pci_bar3[CIU_LUT_ENTRIES] = {
     // LUT for the PCI2 (32 entries)
     // 512MB (16 entries) mapped into the PCI2 MEM32 Space (including PCI Config and I/O)
{(PCI2_MEM32_OFFSET + 0x00000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x02000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x04000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x06000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},

{(PCI2_MEM32_OFFSET + 0x08000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x0A000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x0C000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x0E000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},

{(PCI2_MEM32_OFFSET + 0x10000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x12000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x14000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x16000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},

{(PCI2_MEM32_OFFSET + 0x18000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x1A000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x1C000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},
{(PCI2_MEM32_OFFSET + 0x1E000000) | PORT_PCI2,  PCI2_MEM32_OFFSET_UPPER},

// 256MB (8 entries) mapped into the PCI2 PFM1 Space
{(PCI2_PFM1_OFFSET + 0x20000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},
{(PCI2_PFM1_OFFSET + 0x22000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},
{(PCI2_PFM1_OFFSET + 0x24000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},
{(PCI2_PFM1_OFFSET + 0x26000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},

{(PCI2_PFM1_OFFSET + 0x28000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},
{(PCI2_PFM1_OFFSET + 0x2A000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},
{(PCI2_PFM1_OFFSET + 0x2C000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},
{(PCI2_PFM1_OFFSET + 0x2E000000) | PORT_PCI2,  PCI2_PFM1_OFFSET_UPPER},

// 256MB (8 entries) mapped into the PCI2 PFM2 Space
{(PCI2_PFM2_OFFSET + 0x50000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},
{(PCI2_PFM2_OFFSET + 0x52000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},
{(PCI2_PFM2_OFFSET + 0x54000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},
{(PCI2_PFM2_OFFSET + 0x56000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},

{(PCI2_PFM2_OFFSET + 0x58000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},
{(PCI2_PFM2_OFFSET + 0x5A000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},
{(PCI2_PFM2_OFFSET + 0x5C000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER},
{(PCI2_PFM2_OFFSET + 0x5E000000) | PORT_PCI2,  PCI2_PFM2_OFFSET_UPPER}
};

#ifdef ORIGINALMAP   /* This is for original testing script verification */
 /*----------             Map table for PCI BAR2 to SFN          ----------*/
const bar_look_up_entry_t pci1_sf_barx[CIU_LUT_ENTRIES] = {
     // LUT for the PCI2 (32 entries)
     //  PCI1 256Meg (4 entries) mapped into the PBI  Space
{(SF_PCI1_PBI_BASE   + 0x00000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x04000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x08000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x0c000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the SRAM Space
{(SF_PCI1_SRAM_BASE  + 0x00000000) | PORT_SRAM,   SF_PCI1_SRAM_BASE_UPPER},
{(SF_PCI1_SRAM_BASE  + 0x04000000) | PORT_SRAM,   SF_PCI1_SRAM_BASE_UPPER},
{(SF_PCI1_SRAM_BASE  + 0x08000000) | PORT_SRAM,   SF_PCI1_SRAM_BASE_UPPER},
{(SF_PCI1_SRAM_BASE  + 0x0c000000) | PORT_SRAM,   SF_PCI1_SRAM_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the SDRAM Space
{(SF_PCI1_SDRAM_BASE + 0x00000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x04000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x08000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x0c000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
     //   (1 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_CONFIG_BASE + 0x0c000000) | PORT_PCI2,   SF_PCI1_PCI2_CONFIG_BASE_UPPER},
     //  256Meg PCI1 (4 entries) mapped into the PCI2 pfm1 Space
{(SF_PCI2_PFM1_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
{(SF_PCI2_PFM1_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
{(SF_PCI2_PFM1_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
{(SF_PCI2_PFM1_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
     //  256Meg PCI (4 entries) mapped into the PCI2  PFM2 Space
{(SF_PCI2_PFM2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER},
{(SF_PCI2_PFM2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER},
{(SF_PCI2_PFM2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER},
{(SF_PCI2_PFM2_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER}
};

 /*----------             Map table for PCI BAR3 to SFN          ----------*/
const bar_look_up_entry_t pci2_sf_barx[CIU_LUT_ENTRIES] = {
     // LUT for the PCI2 (32 entries)
     //  PCI1 256Meg (4 entries) mapped into the PBI  Space
{(SF_PCI2_PBI_BASE   + 0x00000000) | PORT_PBI,    SF_PCI2_PBI_BASE_UPPER},
{(SF_PCI2_PBI_BASE   + 0x04000000) | PORT_PBI,    SF_PCI2_PBI_BASE_UPPER},
{(SF_PCI2_PBI_BASE   + 0x08000000) | PORT_PBI,    SF_PCI2_PBI_BASE_UPPER},
{(SF_PCI2_PBI_BASE   + 0x0c000000) | PORT_PBI,    SF_PCI2_PBI_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the SRAM Space
{(SF_PCI2_SRAM_BASE  + 0x00000000) | PORT_SRAM,   SF_PCI2_SRAM_BASE_UPPER},
{(SF_PCI2_SRAM_BASE  + 0x04000000) | PORT_SRAM,   SF_PCI2_SRAM_BASE_UPPER},
{(SF_PCI2_SRAM_BASE  + 0x08000000) | PORT_SRAM,   SF_PCI2_SRAM_BASE_UPPER},
{(SF_PCI2_SRAM_BASE  + 0x0c000000) | PORT_SRAM,   SF_PCI2_SRAM_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the SDRAM Space
{(SF_PCI2_SDRAM_BASE + 0x00000000) | PORT_SDRAM,  SF_PCI2_SDRAM_BASE_UPPER},
{(SF_PCI2_SDRAM_BASE + 0x04000000) | PORT_SDRAM,  SF_PCI2_SDRAM_BASE_UPPER},
{(SF_PCI2_SDRAM_BASE + 0x08000000) | PORT_SDRAM,  SF_PCI2_SDRAM_BASE_UPPER},
{(SF_PCI2_SDRAM_BASE + 0x0c000000) | PORT_SDRAM,  SF_PCI2_SDRAM_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
     //  PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
     //  (1 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_CONFIG_BASE + 0x0c000000) | PORT_PCI1,   SF_PCI2_PCI1_CONFIG_BASE_UPPER},
     //  256Meg PCI1 (4 entries) mapped into the PCI2 pfm1 Space
{(SF_PCI1_PFM1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
{(SF_PCI1_PFM1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
{(SF_PCI1_PFM1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
{(SF_PCI1_PFM1_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
     //  256Meg PCI (4 entries) mapped into the PCI2  PFM2 Space
{(SF_PCI1_PFM2_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER},
{(SF_PCI1_PFM2_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER},
{(SF_PCI1_PFM2_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER},
{(SF_PCI1_PFM2_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER}
};
#else   /* This is the more modern - "Map SDRAM at 0x0-type" mapping */
 /*----------             Map table for PCI BAR2 to SFN          ----------*/
const bar_look_up_entry_t pci1_sf_barx[CIU_LUT_ENTRIES] = {
     // LUT for the PCI2 (32 entries)
     //  PCI1 512Meg (8 entries) mapped into the SDRAM Space
{(SF_PCI1_SDRAM_BASE + 0x00000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x04000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x08000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x0c000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x10000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x14000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x18000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x1c000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
     //  PCI1 128Meg (4 entries) mapped into the PBI  Space
{(SF_PCI1_PBI_BASE   + 0x00000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x04000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x08000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x0c000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI1_PCI2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI1_PCI2_BASE_UPPER},
     //   (1 entries) mapped into the PCI2 Space
{(SF_PCI1_PCI2_CONFIG_BASE + 0x0c000000) | PORT_PCI2,   SF_PCI1_PCI2_CONFIG_BASE_UPPER},
     //  256Meg PCI1 (4 entries) mapped into the PCI2 pfm1 Space
{(SF_PCI2_PFM1_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
{(SF_PCI2_PFM1_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
{(SF_PCI2_PFM1_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
{(SF_PCI2_PFM1_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI2_PFM1_BASE_UPPER},
     //  256Meg PCI (4 entries) mapped into the PCI2  PFM2 Space
{(SF_PCI2_PFM2_BASE  + 0x00000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER},
{(SF_PCI2_PFM2_BASE  + 0x04000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER},
{(SF_PCI2_PFM2_BASE  + 0x08000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER},
{(SF_PCI2_PFM2_BASE  + 0x0C000000) | PORT_PCI2,   SF_PCI2_PFM2_BASE_UPPER}
};

 /*----------             Map table for PCI BAR3 to SFN          ----------*/
const bar_look_up_entry_t pci2_sf_barx[CIU_LUT_ENTRIES] = {
     // LUT for the PCI2 (32 entries)
     //  PCI1 512Meg (8 entries) mapped into the SDRAM Space
{(SF_PCI1_SDRAM_BASE + 0x00000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x04000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x08000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x0c000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x10000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x14000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x18000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
{(SF_PCI1_SDRAM_BASE + 0x1c000000) | PORT_SDRAM,  SF_PCI1_SDRAM_BASE_UPPER},
     //  PCI1 128Meg (4 entries) mapped into the PBI  Space
{(SF_PCI1_PBI_BASE   + 0x00000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x04000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x08000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
{(SF_PCI1_PBI_BASE   + 0x0c000000) | PORT_PBI,    SF_PCI1_PBI_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
     //   PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI1_PCI2_BASE_UPPER},
     //  PCI1 256Meg (4 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
{(SF_PCI2_PCI1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI2_PCI1_BASE_UPPER},
     //  (1 entries) mapped into the PCI2 Space
{(SF_PCI2_PCI1_CONFIG_BASE + 0x0c000000) | PORT_PCI1,   SF_PCI2_PCI1_CONFIG_BASE_UPPER},
     //  256Meg PCI1 (4 entries) mapped into the PCI2 pfm1 Space
{(SF_PCI1_PFM1_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
{(SF_PCI1_PFM1_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
{(SF_PCI1_PFM1_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
{(SF_PCI1_PFM1_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI1_PFM1_BASE_UPPER},
     //  256Meg PCI (4 entries) mapped into the PCI2  PFM2 Space
{(SF_PCI1_PFM2_BASE  + 0x00000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER},
{(SF_PCI1_PFM2_BASE  + 0x04000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER},
{(SF_PCI1_PFM2_BASE  + 0x08000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER},
{(SF_PCI1_PFM2_BASE  + 0x0C000000) | PORT_PCI1,   SF_PCI1_PFM2_BASE_UPPER}
};
#endif


void cyg_hal_ioc_pci_init(void)
{
    long                 indx;
             cyg_uint32*  pciu;
             cyg_uint32*  pbar2lut;
             cyg_uint32*  pbar3lut;
    volatile cyg_uint32   ciu_sizes;

    /* Init CIU BAR2 and BAR3 Look Up Tables to map to PCI1 and PCI2*/
    pbar2lut = (cyg_uint32*)(HAL_IOC80314_CIU_BASE + TS_CIU_SF_BAR2_LUT0); // Pointer to BAR 2 Look up tabel cntl
    pbar3lut = (cyg_uint32*)(HAL_IOC80314_CIU_BASE + TS_CIU_SF_BAR3_LUT0); // Pointer to BAR 3 Look up tabel cntl
    for (indx = 0; indx < CIU_LUT_ENTRIES; indx++)
    {
        *pbar2lut++ = xs_pci_bar2[indx].lut;
        *pbar2lut++ = xs_pci_bar2[indx].lut_upper;
        *pbar3lut++ = xs_pci_bar3[indx].lut;
        *pbar3lut++ = xs_pci_bar3[indx].lut_upper;
    }
    //---------- Set up CIU Cntl Registers
    pciu                 = (cyg_uint32*)(HAL_IOC80314_CIU_BASE);
    pciu[TS_CIU_SF_BAR2/4] =  HAL_IOC80314_CIU_PCI1_BASE;
    pciu[TS_CIU_SF_BAR3/4] =  HAL_IOC80314_CIU_PCI2_BASE;
    ciu_sizes            = pciu[TS_CIU_SF_SIZES/4];
    ciu_sizes           &= (TS_CIU_SF_SIZES_SF_BAR1_SIZE + TS_CIU_SF_SIZES_PASS1 + TS_CIU_SF_SIZES_EN1) +  // preserve BAR1 & BAR4
                           (TS_CIU_SF_SIZES_SF_BAR4_SIZE + TS_CIU_SF_SIZES_PASS4 + TS_CIU_SF_SIZES_EN4);
    ciu_sizes           |= ( (0x0f<<(3+16)) | TS_CIU_SF_SIZES_EN2)  +    // Window size - 1G, pass = LUT
                           ( (0x0f<<(3+ 8)) | TS_CIU_SF_SIZES_EN3)   ;
    pciu[TS_CIU_SF_SIZES/4]= ciu_sizes;

    //----------  Set up PCI1 BAR2 and BAR3 Look Up Tables
    pbar2lut = (cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_P2S_BAR2_LUT); // Pointer to BAR 2 Look up tabel cntl
    pbar3lut = (cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_P2S_BAR3_LUT); // Pointer to BAR 3 Look up tabel cntl
    for (indx = 0; indx < CIU_LUT_ENTRIES; indx++)
    {
        *pbar2lut++ = pci1_sf_barx[indx].lut;
        *pbar2lut++ = pci1_sf_barx[indx].lut_upper;
        *pbar3lut++ = pci1_sf_barx[indx].lut;
        *pbar3lut++ = pci1_sf_barx[indx].lut_upper;
    }
    //........... Set up PCI1 PFAB registers
    pbar2lut = (cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PFAB_BAR0); // Pointer to BAR 2 Look up tabel cntl
    *pbar2lut++ = PCI1_PFAB_BAR0;
    *pbar2lut++ = PCI1_PFAB_BAR0_UPPER;
    *pbar2lut++ = PCI1_PFAB_IO;
    *pbar2lut++ = PCI1_PFAB_IO_UPPER;
    *pbar2lut++ = PCI1_PFAB_MEM32;
    *pbar2lut++ = PCI1_PFAB_MEM32_REMAP;
    *pbar2lut++ = PCI1_PFAB_MEM32_MASK;
    *pbar2lut++ = PCI1_PFAB_PFM3;
    *pbar2lut++ = PCI1_PFAB_PFM3_REMAP_LOWER;
    *pbar2lut++ = PCI1_PFAB_PFM3_REMAP_UPPER;
    *pbar2lut++ = PCI1_PFAB_PFM3_MASK;
    *pbar2lut++ = PCI1_PFAB_PFM4;
    *pbar2lut++ = PCI1_PFAB_PFM4_REMAP_LOWER;
    *pbar2lut++ = PCI1_PFAB_PFM4_REMAP_UPPER;
    *pbar2lut   = PCI1_PFAB_PFM4_MASK;
    pbar2lut    = (cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_P2S_PAGE_SIZES); // Pointer to BAR2 BAR3 Page Sizes
    *pbar2lut   = SF_PCI1_P2S_PAGE_SIZES;

    //----------  Set up PCI2 BAR2 and BAR3 Look Up Tables
    pbar2lut = (cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_P2S_BAR2_LUT); // Pointer to BAR 2 Look up tabel cntl
    pbar3lut = (cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_P2S_BAR3_LUT); // Pointer to BAR 3 Look up tabel cntl
    for (indx = 0; indx < CIU_LUT_ENTRIES; indx++)
    {
        *pbar2lut++ = pci2_sf_barx[indx].lut;
        *pbar2lut++ = pci2_sf_barx[indx].lut_upper;
        *pbar3lut++ = pci2_sf_barx[indx].lut;
        *pbar3lut++ = pci2_sf_barx[indx].lut_upper;
    }
    //........... Set up PCI2 PFAB registers
    pbar2lut = (cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PFAB_BAR0); // Pointer to BAR 2 Look up tabel cntl
    *pbar2lut++ = PCI2_PFAB_BAR0;
    *pbar2lut++ = PCI2_PFAB_BAR0_UPPER;
    *pbar2lut++ = PCI2_PFAB_IO;
    *pbar2lut++ = PCI2_PFAB_IO_UPPER;
    *pbar2lut++ = PCI2_PFAB_MEM32;
    *pbar2lut++ = PCI2_PFAB_MEM32_REMAP;
    *pbar2lut++ = PCI2_PFAB_MEM32_MASK;
    *pbar2lut++ = PCI2_PFAB_PFM3;
    *pbar2lut++ = PCI2_PFAB_PFM3_REMAP_LOWER;
    *pbar2lut++ = PCI2_PFAB_PFM3_REMAP_UPPER;
    *pbar2lut++ = PCI2_PFAB_PFM3_MASK;
    *pbar2lut++ = PCI2_PFAB_PFM4;
    *pbar2lut++ = PCI2_PFAB_PFM4_REMAP_LOWER;
    *pbar2lut++ = PCI2_PFAB_PFM4_REMAP_UPPER;
    *pbar2lut   = PCI2_PFAB_PFM4_MASK;
    pbar2lut    = (cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PCI_P2S_PAGE_SIZES); // Pointer to BAR2 BAR3 Page Sizes
    *pbar2lut   = SF_PCI2_P2S_PAGE_SIZES;
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
static cyg_uint32 pci_port_ctrl_base_addr;

#define DEBUG_CONFIG_VERBOSE 0

static inline cyg_uint32*
pci_config_setup(cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint8    pci_1_localbus;            // PCI1 PCI-X status reg bus value
    cyg_uint8    pci_2_localbus;            // PCI2 PCI-X status reg bus value

    cyg_uint32   pci_port;                  // defines the base translation memory address for PCI1 or PCI2
    cyg_uint32*  pci_address;               // is the composit address used to excite the config cycle

    pci_1_localbus = (*((cyg_uint32*)(HAL_IOC80314_PCI1_BASE + PCI_PCI_PCIX_STAT)) >> 8) & 0xff;
    pci_2_localbus = (*((cyg_uint32*)(HAL_IOC80314_PCI2_BASE + PCI_PCI_PCIX_STAT)) >> 8) & 0xff;
    /*  The following code attempts to analize whether the configeration cycle is intended for PCI1
     * or PCI2 based on the vaule of the bus number and whether the port is in PCI or PCI-X mode.
     * In PCI mode, this hardware does intrinsially indicate what BUS nmubers are to be used and
     * what range of bus values are used for PCI2.  In PCI-X mode, the PCI-X status register defines
     * the bus number being used for the local bus, but the hardware does not define a secoandary bus
     * range or PCI2.  This code uses compile time constants to resolve the undefinded bus numbers.
     * A system which can dynamially assign vaules to these vaules will need modify this approach to
     * accomidate dynmically assigning values for compairison in these functions.
     *
     */
    if (pci_1_localbus == 0xff) // PCI1 is in PCI mode
    {
      if (( bus <=  PCI1_BUS) && ( bus <  PCI2_BUS_MAX))
      {  // then bus is on PCI1
          pci_port_ctrl_base_addr = (cyg_uint32)HAL_IOC80314_PCI1_BASE;
          pci_port = (cyg_uint32)(XS_PCI1_BASE_ADDR + PCI1_CFG_OFFSET);
      }
      else
      {   // else bus is on PCI2
          pci_port_ctrl_base_addr = (cyg_uint32)HAL_IOC80314_PCI2_BASE;
          pci_port  = (cyg_uint32)(XS_PCI2_BASE_ADDR + PCI2_CFG_OFFSET);
      }
    }
    else                         //PCI1 is in PCI-X mode
    {
      if (( bus <=  pci_1_localbus) && ( bus <  PCI2_BUS_MAX))
      {   // then bus is on PCI1 in PCI-X mode
          pci_port_ctrl_base_addr = (cyg_uint32)HAL_IOC80314_PCI1_BASE;
          pci_port = (cyg_uint32)(XS_PCI1_BASE_ADDR + PCI1_CFG_OFFSET);
      }
      else
      {   // else bus is on PCI2
          pci_port_ctrl_base_addr = (cyg_uint32)HAL_IOC80314_PCI2_BASE;
          pci_port  = (cyg_uint32)(XS_PCI2_BASE_ADDR + PCI2_CFG_OFFSET);
      }
    }
    /* Offsets must be dword-aligned */
    offset &= ~3;

    /* Immediate bus use type 0 config, all others use type 1 config */
#if DEBUG_CONFIG_VERBOSE
    diag_printf("config: pci1 localbus[%d] bus[%d] dev[%d] fn[%d] offset[0x%x]\n",
        pci_1_localbus, bus, CYG_PCI_DEV_GET_DEV(devfn), CYG_PCI_DEV_GET_FN(devfn), offset);
#endif

    orig_abort_vec = ((volatile cyg_uint32 *)0x20)[4];
    ((volatile unsigned *)0x20)[4] = (unsigned)__pci_abort_handler;

    pci_address = (cyg_uint32 *)(pci_port | (bus << 16) |(devfn << 8) | offset );
#if DEBUG_CONFIG_VERBOSE
	diag_printf ("PCI_Address = 0x%08x\n", pci_address);
#endif
    return pci_address;
}

static inline int
pci_config_cleanup(cyg_uint32 bus)
{
    cyg_uint32 status = 0, err = 0;

    status = *((volatile cyg_uint32*)(pci_port_ctrl_base_addr + PCI_PCI_CSR));
    if ((status & 0xF9000000) != 0) {
	err = 1;
    *((volatile cyg_uint32*)(pci_port_ctrl_base_addr + PCI_PCI_CSR)) = (status & ~(0x0));   //Clear err status bits
    }

    ((volatile unsigned *)0x20)[4] = orig_abort_vec;

    return err;
}


cyg_uint32
cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint32  config_data;
    cyg_uint32 *config_addr;
    int err;

    config_addr = pci_config_setup(bus, devfn, offset);

    config_data = *config_addr;

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
    cyg_uint32 *config_addr;
    int err;

    config_addr = pci_config_setup(bus, devfn, offset);

    *config_addr = data;

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
    cyg_uint32 *config_addr;
    cyg_uint16 config_data;
    int err;

    config_addr = pci_config_setup(bus, devfn, offset & ~3);

    config_data = (cyg_uint16)(((*config_addr) >> ((offset % 0x4) * 8)) & 0xffff);

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
    cyg_uint32 *config_addr;
    int err;
    cyg_uint32 mask, temp;

    config_addr = pci_config_setup(bus, devfn, offset & ~3);

    mask = ~(0x0000ffff << ((offset % 0x4) * 8));

    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *config_addr = (*config_addr & mask) | temp;

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
    cyg_uint32 *config_addr;
    int err;
    cyg_uint8 config_data;

    config_addr = pci_config_setup(bus, devfn, offset & ~3);

    config_data = (cyg_uint8)(((*config_addr) >> ((offset % 0x4) * 8)) & 0xff);

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
    cyg_uint32 *config_addr;
    int err;
    cyg_uint32 mask, temp;

    config_addr = pci_config_setup(bus, devfn, offset & ~3);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *config_addr = (*config_addr & mask) | temp;

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write byte: data[0x%x] err[%d]\n",
		data, err);
#endif
}

#endif // CYGPKG_IO_PCI


