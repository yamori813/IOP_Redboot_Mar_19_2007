//==========================================================================
//
//      devs/eth/arm/iq80315/include/devs_eth_arm_iq80315.inl
//
//      IQ80315 ethernet I/O definitions.
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
// Author(s):   cebruns
// Contributors:msalter
// Date:        2003-05-15
// Purpose:     IQ80315 ethernet defintions
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHERNET
#include <cyg/hal/hal_cache.h>          // HAL_DCACHE_LINE_SIZE
#include <cyg/hal/plf_io.h>             // CYGARC_UNCACHED_ADDRESS




#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0

// Use auto speed detection
#define CYGHWR_DEVS_ETH_INTEL_IOC80314_USE_ASD

static IOC80314 ioc80314_eth0_priv_data = {
#ifdef CYGSEM_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0_ESA,
#else
    hardwired_esa: 0,
#endif
	port_id: 0,
	head: NULL,
	tail: NULL

};

ETH_DRV_SC(ioc80314_sc0,
           &ioc80314_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0_NAME, // Name for device
           ioc80314_start,
           ioc80314_stop,
           ioc80314_ioctl,
           ioc80314_can_send,
           ioc80314_send,
           ioc80314_recv,
           ioc80314_deliver,
           ioc80314_poll,
           ioc80314_int_vector
    );

NETDEVTAB_ENTRY(ioc80314_netdev0,
                "ioc80314_" CYGDAT_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0_NAME,
                ioc80314_init,
                &ioc80314_sc0);

#endif // CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0


#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1

// Use auto speed detection
#define CYGHWR_DEVS_ETH_INTEL_IOC80314_USE_ASD

static IOC80314 ioc80314_eth1_priv_data = {
#ifdef CYGSEM_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1_ESA,
#else
    hardwired_esa: 0,
#endif
	port_id: 1,
	head: NULL,
	tail: NULL

};

ETH_DRV_SC(ioc80314_sc1,
           &ioc80314_eth1_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1_NAME, // Name for device
           ioc80314_start,
           ioc80314_stop,
           ioc80314_ioctl,
           ioc80314_can_send,
           ioc80314_send,
           ioc80314_recv,
           ioc80314_deliver,
           ioc80314_poll,
           ioc80314_int_vector
    );

NETDEVTAB_ENTRY(ioc80314_netdev1,
                "ioc80314_" CYGDAT_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1_NAME,
                ioc80314_init,
                &ioc80314_sc1);

#endif // CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1


// These arrays are used for sanity checking of pointers
IOC80314 *
ioc80314_priv_array[CYGNUM_DEVS_ETH_INTEL_IOC80314_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0
    &ioc80314_eth0_priv_data,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1
    &ioc80314_eth1_priv_data,
#endif

};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
ioc80314_netdev_array[CYGNUM_DEVS_ETH_INTEL_IOC80314_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0
    &ioc80314_netdev0,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1
    &ioc80314_netdev1,
#endif
};

struct eth_drv_sc *
ioc80314_sc_array[CYGNUM_DEVS_ETH_INTEL_IOC80314_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH0
    &ioc80314_sc0,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80315_IOC80314_ETH1
    &ioc80314_sc1,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// EOF devs_eth_arm_iq80315.inl
