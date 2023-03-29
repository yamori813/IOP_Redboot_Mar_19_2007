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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-07-10
// Purpose:      Intel IQ80331/IQ80332 CRB PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/iq8033x.h>
#include <cyg/hal/hal_iop33x.h>
#include <cyg/hal/hal_xscale_iop.h>
#include CYGBLD_HAL_PLF_INTS_H

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

//-----------------------------------------------------------------------------
// Resources

#define __IQ80332_PCIX_SLOT_DEVNUM        4
#define __IQ80332_PCIX_B_SLOT_DEVNUM      1
#define __IQ80332_GBE_DEVNUM              6
#define __IQ80332_GBE_DEVNUM_ALPHA_BOARD  5

#define __IQ80331_PCIX_SLOT_DEVNUM        1
#define __IQ80331_GBE_DEVNUM              2

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)           \
    CYG_MACRO_START                                                            \
    cyg_uint32 __dev = CYG_PCI_DEV_GET_DEV(__devfn);                           \
    cyg_uint32 __fn  = CYG_PCI_DEV_GET_FN(__devfn);                            \
    cyg_uint32 __xbus = iop33x_localbus_number();                              \
    cyg_uint32 __onboard_gbe_devnum1, __onboard_gbe_devnum2;                   \
    if ((*ATU_ATUDID == ATUDID_DEVICE_IOP332) ||                               \
        (*ATU_ATUDID == ATUDID_DEVICE_IOP333)) {                               \
        __onboard_gbe_devnum1 = __IQ80332_GBE_DEVNUM;                          \
        __onboard_gbe_devnum2 = __IQ80332_GBE_DEVNUM_ALPHA_BOARD;              \
    } else {                                                                   \
        __onboard_gbe_devnum1 = __IQ80331_GBE_DEVNUM;                          \
        __onboard_gbe_devnum2 = __IQ80331_GBE_DEVNUM;                          \
    }                                                                          \
    __valid = false;                                                           \
    if ((__fn==0) &&                                                           \
        ((__dev==__onboard_gbe_devnum1) || (__dev==__onboard_gbe_devnum2)) &&  \
        (__bus==__xbus)) {                                                     \
        __vec = CYGNUM_HAL_INTERRUPT_ETHERNET;                                 \
        __valid = true;                                                        \
    } else {                                                                   \
        cyg_uint8 __req;                                                       \
        HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);    \
        switch(__req) {                                                        \
          case 1: /* INTA */                                                   \
            __vec=CYGNUM_HAL_INTERRUPT_XINT0; __valid=true; break;             \
          case 2: /* INTB */                                                   \
            __vec=CYGNUM_HAL_INTERRUPT_XINT1; __valid=true; break;             \
          case 3: /* INTC */                                                   \
            __vec=CYGNUM_HAL_INTERRUPT_XINT2; __valid=true; break;             \
          case 4: /* INTD */                                                   \
            __vec=CYGNUM_HAL_INTERRUPT_XINT3; __valid=true; break;             \
        }                                                                      \
    }                                                                          \
    CYG_MACRO_END

static inline CYG_ADDRWORD cygarc_cached_address(CYG_ADDRWORD ua)
{
  if ((ua >= SDRAM_UNCACHED_BASE) && (ua < (SDRAM_UNCACHED_BASE + SDRAM_MAX_UNCACHED)))
    return ua - SDRAM_UNCACHED_BASE + SDRAM_BASE;
  else
    return ua;
}

#define CYGARC_CACHED_ADDRESS(_x_) cygarc_cached_address(_x_)

static inline CYG_ADDRWORD cygarc_uncached_address(CYG_ADDRWORD ca)
{
  if ((ca >= SDRAM_BASE) && (ca < (SDRAM_BASE + SDRAM_MAX_UNCACHED)))
    return ca - SDRAM_BASE + SDRAM_UNCACHED_BASE;
  else
    return ca;
}

#define CYGARC_UNCACHED_ADDRESS(_x_) cygarc_uncached_address(_x_)

static inline CYG_ADDRWORD cygarc_physical_address(CYG_ADDRWORD va)
{
    cyg_uint32 *ram_mmutab = (cyg_uint32 *)(SDRAM_BASE | 0x4000);
    cyg_uint32 pte;

    pte = ram_mmutab[va >> 20];

    return (pte & 0xfff00000) | (va & 0xfffff);
}

#define CYGARC_PHYSICAL_ADDRESS(_x_) cygarc_physical_address(_x_)

static inline CYG_ADDRWORD cygarc_virtual_address(CYG_ADDRWORD pa)
{
#if defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE) && !defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE_0)
#define ECC32_HOLE_SIZE  (CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE<<20)
#define ECC32_BOTTOM     (SDRAM_PHYS_BASE)
#define ECC32_TOP        (ECC32_HOLE_SIZE + SDRAM_PHYS_BASE)
#define ECC32_HOLE_TOP   (ECC32_HOLE_SIZE*2 + SDRAM_PHYS_BASE)
    if ((ECC32_BOTTOM <= pa) && (pa < ECC32_TOP)) {
        // In the 32-bit ECC Region
        return (pa - SDRAM_PHYS_BASE + SDRAM_32BIT_ECC_BASE);
    } else if ((ECC32_TOP <= pa) && (pa < ECC32_HOLE_TOP)) {
        // In the 32-bit ECC Hole.  Invalid Address.
        return ~0;
    } else if ((ECC32_HOLE_TOP <= pa) && (pa < (unsigned long)SDRAM_MAX)) {
        // In the 64-bit ECC Region
        return pa - ECC32_HOLE_TOP + SDRAM_BASE;
    } else {
#endif
        // All other regions map 1:1
        return pa;
#if defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE) && !defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE_0)
    }
#endif
}

#define CYGARC_VIRTUAL_ADDRESS(_x_) cygarc_virtual_address(_x_)

static inline cyg_uint64 cygarc_virt_to_bus(CYG_ADDRWORD va)
{
    cyg_uint32 offset = CYGARC_UNCACHED_ADDRESS(va) - SDRAM_UNCACHED_BASE;
    cyg_uint64 ba = (cyg_uint64)offset + hal_pci_inbound_window_base;
    return (ba);
}

#define CYGARC_VIRT_TO_BUS(_x_) cygarc_virt_to_bus(_x_)

static inline CYG_ADDRWORD cygarc_bus_to_virt(cyg_uint64 ba)
{
    cyg_uint32 offset;
    cyg_uint32 va;

    offset = (cyg_uint32)(ba & hal_pci_inbound_window_mask & 0xFFFFFFFF);
    va = offset + SDRAM_UNCACHED_BASE;

    return (va);
}

#define CYGARC_BUS_TO_VIRT(_x_) cygarc_bus_to_virt(_x_)


//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
