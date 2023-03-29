//==========================================================================
//
//      devs/eth/arm/mainstone/include/devs_eth_arm_mainstone.inl
//
//      Mainstone ethernet I/O definitions.
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
// Author(s):   msalter
// Contributors:jskov
// Date:        2002-08-01
// Purpose:     Mainstone ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/mainstone.h>

#ifdef CYGPKG_DEVS_ETH_ARM_MAINSTONE_ETH0

#define LAN91CXX_IS_LAN91C111 1
#define LAN91CXX_32BIT_RX 1

static lan91cxx_priv_data lan91cxx_eth0_priv_data = { 
    base : (unsigned short*) (MAINSTONE_ETHERNET_BASE | 0x300),
    interrupt: CYGNUM_HAL_INTERRUPT_ETHERNET,
#ifdef CYGSEM_DEVS_ETH_ARM_MAINSTONE_ETH0_STATIC_ESA
    enaddr : CYGDAT_DEVS_ETH_ARM_MAINSTONE_ETH0_ESA,
#endif
    config_enaddr : NULL              // rely on the hardwired address for now
};

ETH_DRV_SC(smsc_lan91cxx_sc,
           &lan91cxx_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_ARM_MAINSTONE_ETH0_NAME,
           lan91cxx_start,
           lan91cxx_stop,
           lan91cxx_control,
           lan91cxx_can_send,
           lan91cxx_send,
           lan91cxx_recv,
           lan91cxx_deliver,     // "pseudoDSR" called from fast net thread
           lan91cxx_poll,        // poll function, encapsulates ISR and DSR
           lan91cxx_int_vector);

NETDEVTAB_ENTRY(smsc_netdev, 
                "smsc_" CYGDAT_DEVS_ETH_ARM_MAINSTONE_ETH0_NAME,
                smsc_lan91cxx_init, 
                &smsc_lan91cxx_sc);


// Due to the hw implementation on Mainstone, we can't write to odd halfword
// registers using 16-bit accesses. So we override the default macro:
#define SMSC_WRITE_UINT16(a,n)                                  \
    CYG_MACRO_START                                             \
    if (((CYG_WORD32)(a)) & 2) {                                \
        volatile CYG_WORD32 *__p;                               \
        __p = (volatile CYG_WORD32 *)(((CYG_WORD32)(a)) & ~3);  \
        *__p = (*__p & 0xFFFF) | ((n) << 16);                   \
    } else                                                      \
        (*((volatile CYG_WORD16 *)(a)) = (n));                  \
    CYG_MACRO_END

#define SMSC_READ_UINT16(a,n)                                   \
    CYG_MACRO_START                                             \
    CYG_WORD32 __val;                                           \
    __val = *(volatile CYG_WORD32 *)(((CYG_WORD32)(a)) & ~3);   \
    if (((CYG_WORD32)(a)) & 2)                                  \
        (n) = (__val >> 16);                                    \
    else                                                        \
        (n) = (__val & 0xffff);                                 \
    CYG_MACRO_END

// ------------------------------------------------------------------------
#define SMSC_PLATFORM_DEFINED_GET_REG 1
static __inline__ unsigned short
get_reg(struct eth_drv_sc *sc, int regno)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    unsigned short val;
    
    HAL_WRITE_UINT32(cpd->base+(LAN91CXX_BS-1), (regno >> 3) << 16);
    SMSC_READ_UINT16(cpd->base+(regno&0x7), val);

#if DEBUG & 2
    diag_printf("read reg %d val 0x%04x\n", regno, val);
#endif
    return val;
}

// ------------------------------------------------------------------------
#define SMSC_PLATFORM_DEFINED_PUT_REG 1
static __inline__ void
put_reg(struct eth_drv_sc *sc, int regno, unsigned short val)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
        
    HAL_WRITE_UINT32(cpd->base+(LAN91CXX_BS-1), (regno >> 3) << 16);

    if (regno == LAN91CXX_PNR) {
        HAL_WRITE_UINT32(cpd->base+(regno&0x6), val << 16);
    } else {
        SMSC_WRITE_UINT16(cpd->base+(regno&0x7), val);
    }

#if DEBUG & 2
    diag_printf("write reg %d val 0x%04x\n", regno, val);
#endif
}

// ------------------------------------------------------------------------
// Assumes bank2 has been selected
#define SMSC_PLATFORM_DEFINED_PUT_DATA 1
static __inline__ void
put_data(struct eth_drv_sc *sc, unsigned short val)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
        
    HAL_WRITE_UINT16(cpd->base+((LAN91CXX_DATA_HIGH & 0x7) << cpd->addrsh), val);

#if DEBUG & 2
    diag_printf("write data 0x%04x\n", val);
#endif
}



#endif // CYGPKG_DEVS_ETH_ARM_MAINSTONE_ETH0

// EOF devs_eth_arm_mainstone.inl
