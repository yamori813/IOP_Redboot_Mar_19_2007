//==========================================================================
//
//      devs/eth/arm/iq31244/include/devs_eth_arm_iq31244.inl
//
//      IQ31244 ethernet I/O definitions.
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
// Contributors:msalter, dmoseley
// Date:        2002-01-10
// Purpose:     IQ31244 ethernet defintions
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHERNET
#include <cyg/hal/hal_cache.h>          // HAL_DCACHE_LINE_SIZE
#include <cyg/hal/plf_io.h>             // CYGARC_UNCACHED_ADDRESS

#if defined(CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0) || defined(CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1)

#if defined(CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0) && defined(CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1)
#define MAX_PORTS 2
#else
#define MAX_PORTS 1
#endif

#define CYGHWR_INTEL_I82544_PCI_VIRT_TO_BUS( _x_ ) CYGARC_VIRT_TO_BUS(_x_)
#define CYGHWR_INTEL_I82544_PCI_BUS_TO_VIRT( _x_ ) CYGARC_BUS_TO_VIRT(_x_)

#define MAX_PACKET_SIZE   1536
#define SIZEOF_DESCRIPTOR 16

#ifdef CYGPKG_REDBOOT
#define ACTIVE_PORTS 1
#else
#define ACTIVE_PORTS MAX_PORTS
#endif

#define CYGHWR_INTEL_I82544_PCI_MEM_MAP_SIZE \
  (((MAX_PACKET_SIZE + SIZEOF_DESCRIPTOR) * \
     ((MAX_TX_DESCRIPTORS + MAX_RX_DESCRIPTORS) * ACTIVE_PORTS)) + (64 * ACTIVE_PORTS))

static char pci_mem_buffer[CYGHWR_INTEL_I82544_PCI_MEM_MAP_SIZE + HAL_DCACHE_LINE_SIZE];

#define CYGHWR_INTEL_I82544_PCI_MEM_MAP_BASE \
  (CYGARC_UNCACHED_ADDRESS(((unsigned)pci_mem_buffer + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1)))

#endif

#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0

static I82544 i82544_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_ARM_IQ31244_I82544_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_IQ31244_I82544_ETH0_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82544_sc0,
           &i82544_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_IQ31244_I82544_ETH0_NAME, // Name for device
           i82544_start,
           i82544_stop,
           i82544_ioctl,
           i82544_can_send,
           i82544_send,
           i82544_recv,
           i82544_deliver,
           i82544_poll,
           i82544_int_vector
    );

NETDEVTAB_ENTRY(i82544_netdev0, 
                "i82546_" CYGDAT_DEVS_ETH_ARM_IQ31244_I82544_ETH0_NAME,
                i82544_init, 
                &i82544_sc0);

#endif // CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0


#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1

static I82544 i82544_eth1_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_ARM_IQ31244_I82544_ETH1_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_IQ31244_I82544_ETH1_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82544_sc1,
           &i82544_eth1_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_IQ31244_I82544_ETH1_NAME, // Name for device
           i82544_start,
           i82544_stop,
           i82544_ioctl,
           i82544_can_send,
           i82544_send,
           i82544_recv,
           i82544_deliver,
           i82544_poll,
           i82544_int_vector
    );

NETDEVTAB_ENTRY(i82544_netdev1, 
                "i82546_" CYGDAT_DEVS_ETH_ARM_IQ31244_I82544_ETH1_NAME,
                i82544_init, 
                &i82544_sc1);

#endif // CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1



// These arrays are used for sanity checking of pointers
I82544 *
i82544_priv_array[MAX_PORTS] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0
    &i82544_eth0_priv_data,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1
    &i82544_eth1_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i82544_netdev_array[MAX_PORTS] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0
    &i82544_netdev0,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1
    &i82544_netdev1,
#endif
};

struct eth_drv_sc *
i82544_sc_array[MAX_PORTS] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH0
    &i82544_sc0,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_IQ31244_I82544_ETH1
    &i82544_sc1,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// EOF devs_eth_arm_iq31244.inl
