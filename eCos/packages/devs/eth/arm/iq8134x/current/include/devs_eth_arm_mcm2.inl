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
// Author(s):   Adam Brooks 
// Contributors: msalter
// Date:        2006-11-06
// Purpose:    Maple Creek Mode 2 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>

#ifdef CYGPKG_DEVS_ETH_ARM_MCM2_ETH0

#define LAN91CXX_IS_LAN91C111 1
#define CYGNUM_DEVS_ETH_SMSC_LAN91CXX_SHIFT_ADDR 1


static lan91cxx_priv_data lan91cxx_eth0_priv_data = { 
    base : (unsigned short*) (MCM2_ETHERNET_BASE | 0x600),
    interrupt: MCM2_ETHERNET_IRQ,
    config_enaddr: NULL
//  enaddr : {0x00,0x0e,0x0c,0xb9,0xa9,0x9a},   // rely on the hardwired address for now
//  hardwired_esa : true,
};

ETH_DRV_SC(smsc_lan91cxx_sc,
           &lan91cxx_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_ARM_MCM2_ETH0_NAME,
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
                "smsc_" CYGDAT_DEVS_ETH_ARM_MCM2_ETH0_NAME,
                smsc_lan91cxx_init, 
                &smsc_lan91cxx_sc);

#endif
