/*=============================================================================
//
//      iq8033x_setup_atu.c
//
//      Platform specific ATU Initialization for HAL (C code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// Date:         2003-08-04
// Purpose:      Intel XScale iq80331/iq80332 CRB platform specific ATU initialization
// Description: 
//     This file contains low-level initialization code called by hal_platform_setup.h
//     Since we may be using "alternative" memory (ie different stack and heap than
//     we were compiled for), we cannot use any global data or C Library calls.
// Usage:
//     Only called by "hal_platform_setup.c"
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/iq8033x.h>             // Platform specific hardware definitions
#include <cyg/hal/hal_iop33x.h> // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/hal_spd.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_cfg.h>

//
// PCI Configuration access setup
//
static inline void
setup_pci_config_setup(cyg_uint32 localbus, 
                       cyg_uint32 bus,
                       cyg_uint32 dev,
                       cyg_uint32 fn,
                       cyg_uint32 offset)
{
    cyg_uint32 occar_val;
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
    if (bus == localbus)
        occar_val = (pcie_offset | (1 << (dev + 16)) | (dev << 11) | (fn << 8) | pci_offset | 0);
    else
        occar_val = (pcie_offset | (bus << 16) | (dev << 11) | (fn << 8) | pci_offset | 1);

    HAL_WRITE_UINT32(ATU_OCCAR, occar_val);
}

//
// PCI Configuration access cleanup
//
static inline int
setup_pci_config_cleanup(void)
{
    cyg_uint32 status = 0, err = 0;

    HAL_READ_UINT32(ATU_ATUSR, status);
    if ((status & 0xF900) != 0) {
	err = 1;
        HAL_WRITE_UINT32(ATU_ATUSR, status & 0xF900);
    }

    return err;
}

//
// Issue an 8-bit PCI configuration Read
// We are not using the HAL_PCI_CFG_READ_UINT* macros
// because they rely on static data which is not available
// at this time.
//
static cyg_uint8 setup_pci_cfg_read_uint8(cyg_uint32 localbus, 
                                          cyg_uint32 bus,
                                          cyg_uint32 dev,
                                          cyg_uint32 fn,
                                          cyg_uint32 offset)
{
    int err;
    cyg_uint8 config_data;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset);

    HAL_READ_UINT32(ATU_OCCDR, occdr_val);
    config_data = (occdr_val >> ((offset % 0x4) * 8)) & 0xff;

    err = setup_pci_config_cleanup();

    if (err)
	return 0xff;
    else
	return config_data;
}

//
// Issue an 8-bit PCI configuration Write
// We are not using the HAL_PCI_CFG_READ_UINT* macros
// because they rely on static data which is not available
// at this time.
//
static void setup_pci_cfg_write_uint8(cyg_uint32 localbus,
                                      cyg_uint32 bus,
                                      cyg_uint32 dev,
                                      cyg_uint32 fn,
                                      cyg_uint32 offset,
                                      cyg_uint8 val)
{
    int err;
    cyg_uint32 mask, temp;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (((cyg_uint32)val) << ((offset % 0x4) * 8));

    HAL_READ_UINT32(ATU_OCCDR, occdr_val);
    HAL_WRITE_UINT32(ATU_OCCDR, (occdr_val & mask) | temp);

    err = setup_pci_config_cleanup();
}

//
// Issue a 16-bit PCI configuration Read
// We are not using the HAL_PCI_CFG_READ_UINT* macros
// because they rely on static data which is not available
// at this time.
//
static cyg_uint16 setup_pci_cfg_read_uint16(cyg_uint32 localbus,
                                            cyg_uint32 bus,
                                            cyg_uint32 dev,
                                            cyg_uint32 fn,
                                            cyg_uint32 offset)
{
    int err;
    cyg_uint16 config_data;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset);

    HAL_READ_UINT32(ATU_OCCDR, occdr_val);
    config_data = (occdr_val >> ((offset % 0x4) * 8)) & 0xffff;

    err = setup_pci_config_cleanup();

    if (err)
	return 0xffff;
    else
	return config_data;
}

//
// Issue a 16-bit PCI configuration Write
// We are not using the HAL_PCI_CFG_READ_UINT* macros
// because they rely on static data which is not available
// at this time.
//
static void setup_pci_cfg_write_uint16(cyg_uint32 localbus,
                                       cyg_uint32 bus,
                                       cyg_uint32 dev,
                                       cyg_uint32 fn,
                                       cyg_uint32 offset,
                                       cyg_uint16 val)
{
    int err;
    cyg_uint32 mask, temp;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset);

    mask = ~(0x0000ffff << ((offset % 0x4) * 8));
    temp = (((cyg_uint32)val) << ((offset % 0x4) * 8));

    HAL_READ_UINT32(ATU_OCCDR, occdr_val);
    HAL_WRITE_UINT32(ATU_OCCDR, (occdr_val & mask) | temp);

    err = setup_pci_config_cleanup();
}

//
// Issue a 32-bit PCI configuration Read
// We are not using the HAL_PCI_CFG_READ_UINT* macros
// because they rely on static data which is not available
// at this time.
//
static cyg_uint32 setup_pci_cfg_read_uint32(cyg_uint32 localbus,
                                            cyg_uint32 bus,
                                            cyg_uint32 dev,
                                            cyg_uint32 fn,
                                            cyg_uint32 offset)
{
    int err;
    cyg_uint32 config_data;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset);

    HAL_READ_UINT32(ATU_OCCDR, occdr_val);
    config_data = occdr_val;

    err = setup_pci_config_cleanup();

    if (err)
	return 0xffffffff;
    else
	return config_data;
}

//
// Issue a 32-bit PCI configuration Write
// We are not using the HAL_PCI_CFG_READ_UINT* macros
// because they rely on static data which is not available
// at this time.
//
static void setup_pci_cfg_write_uint32(cyg_uint32 localbus,
                                       cyg_uint32 bus,
                                       cyg_uint32 dev,
                                       cyg_uint32 fn,
                                       cyg_uint32 offset,
                                       cyg_uint32 val)
{
    int err;

    setup_pci_config_setup(localbus, bus, dev, fn, offset);
    HAL_WRITE_UINT32(ATU_OCCDR, val);
    err = setup_pci_config_cleanup();
}

//
// Set up the ATU settings needed to allow host BIOS to configure us properly.
// Once that is done we release the retry bits
// This is done as early as possible to minimize racing w/ the host BIOS
//
void minimal_ATU_setup(cyg_bool sdram_installed,
                       cyg_uint32 sdram_size,
                       cyg_uint32 *sdram_pci_size,
                       cyg_uint32 *config_flags)
{
    cyg_uint32 privmem_size;
    cyg_uint32 pcsr;
    cyg_uint16 atucmd;
    cyg_uint32 localbus = iop33x_localbus_number();
    cyg_uint32 atucr;
    cyg_uint16 did;
    cyg_uint8 rot_sw;
    cyg_uint32 bridge_A_binit = 0;
    cyg_uint32 downstream_hidden_size = 0;

    hal_platform_lowlevel_serial_debug_puts("In minimal_ATU_setup()\r\n");
    HEX_DISPLAY_QUICK(2, 4);

    // Initialize Flags
    *config_flags = 0;

    // Read the Device ID
    HAL_READ_UINT16(ATU_ATUDID, did);

    // Read the PCSR
    HAL_READ_UINT32(ATU_PCSR, pcsr);

    HAL_READ_UINT8(IQ8033X_ROTARY_SWITCH_ADDR, rot_sw);

    // Determine Hiding
    if (((did == ATUDID_DEVICE_IOP332) && ((rot_sw & IQ8033X_ROTARY_SWITCH_DEV_HIDING) != 0)) ||
        ((did == ATUDID_DEVICE_IOP333) && ((rot_sw & IQ8033X_ROTARY_SWITCH_DEV_HIDING) != 0)) ||
        ((did == ATUDID_DEVICE_IOP331) && ((pcsr & PCSR_PRIVDEV_ENABLE) != 0)))
        *config_flags |= IOP33X_DEV_HIDING;

    // Determine Private Memory Status
    if (((did == ATUDID_DEVICE_IOP332) && ((rot_sw & IQ8033X_ROTARY_SWITCH_PCI_PRIVMEM) != 0)) ||
        ((did == ATUDID_DEVICE_IOP333) && ((rot_sw & IQ8033X_ROTARY_SWITCH_PCI_PRIVMEM) != 0)) ||
        ((did == ATUDID_DEVICE_IOP331) && ((pcsr & PCSR_PRIVMEM_ENABLE) != 0)))
        *config_flags |= IOP33X_PCI_PRIVMEM;

    // Determine config-retry and PCI Master/Slave
    // We are going to cheat a little;  there is no perfect
    // indication of whether we are the PCI host or not.
    // If config_retry is enabled; assume we are a target
    if ((pcsr & PCSR_CFG_RETRY) != 0)
    {
        *config_flags |= IOP33X_CONFIG_RETRY;
        *config_flags &=~ IOP33X_PCI_MASTER;
    } else {
        *config_flags |= IOP33X_PCI_MASTER;
    }

    //
    // Determine desired amount of SDRAM to share over public PCI BAR
    //
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_RAM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM)
    // One of our BARS is sharing SDRAM.
    if (sdram_installed)
        *sdram_pci_size = (CYGNUM_HAL_ARM_XSCALE_IOP_RAM_PCI_SIZE == 0) ? sdram_size : CYGNUM_HAL_ARM_XSCALE_IOP_RAM_PCI_SIZE;
    else
        *sdram_pci_size = 0;
#else
    // No SDRAM being shared
    *sdram_pci_size = 0;
#endif

    //
    // Determine desired amount of SDRAM to share over private PCI BAR
    //
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_PRIVMEM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
    // One of our BARS is sharing SDRAM via PRIVMEM.
    if ((sdram_installed) && (*config_flags & IOP33X_PCI_PRIVMEM))
        privmem_size = sdram_size;
    else
        privmem_size = 0;
#else
    // No SDRAM being shared via PRIVMEM.
    privmem_size = 0;
#endif

    // Determine amount of PCI memory space needed for downstream hidden devices
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_REDIRECT) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_REDIRECT) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_REDIRECT) || \
    defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_REDIRECT)
    if (*config_flags & (IOP33X_DEV_HIDING|IOP33X_PCI_MASTER)) {
        downstream_hidden_size = CYGNUM_HAL_ARM_XSCALE_IOP_REDIRECT_PCI_SIZE;
    }
#endif

    // Setup ATU Inbound Window 0 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(ATU_IALR0, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_RAM)
    HAL_WRITE_UINT32(ATU_IALR0, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(ATU_IALR0, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(ATU_IALR0, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_SIZE));
#endif
#ifdef ATU_IATVR0
    HAL_WRITE_UINT32(ATU_IATVR0, CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_TVR);
#endif
    // BAR0 is 32-bit prefetchable memory space for the MU
    HAL_WRITE_UINT32(ATU_IABAR0, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
    HAL_WRITE_UINT32(ATU_IAUBAR0, 0);

    // Setup ATU Inbound Window 1 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(ATU_IALR1, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM)
    HAL_WRITE_UINT32(ATU_IALR1, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(ATU_IALR1, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(ATU_IALR1, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_SIZE));
#endif
#ifdef ATU_IATVR1
    HAL_WRITE_UINT32(ATU_IATVR1, CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_TVR);
#endif

#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM) || defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM) || defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_FLASH)
    // BAR1 is 64-bit prefetchable
    HAL_WRITE_UINT32(ATU_IABAR1, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#else
    // BAR1 is 32-bit non-prefetchable
    HAL_WRITE_UINT32(ATU_IABAR1, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
#endif
    HAL_WRITE_UINT32(ATU_IAUBAR1, 0);

    // Setup ATU Inbound Window 2 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(ATU_IALR2, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM)
    HAL_WRITE_UINT32(ATU_IALR2, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(ATU_IALR2, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(ATU_IALR2, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_SIZE));
#endif
#ifdef ATU_IATVR2
    HAL_WRITE_UINT32(ATU_IATVR2, CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_TVR);
#endif

#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM) || defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM) || defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_FLASH)
    // BAR2 is 64-bit prefetchable
    HAL_WRITE_UINT32(ATU_IABAR2, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#else
    // BAR2 is 32-bit non-prefetchable
    HAL_WRITE_UINT32(ATU_IABAR2, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
#endif
    HAL_WRITE_UINT32(ATU_IAUBAR2, 0);

    // Setup ATU Inbound Window 3 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(ATU_IALR3, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM)
    HAL_WRITE_UINT32(ATU_IALR3, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(ATU_IALR3, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(ATU_IALR3, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_SIZE));
#endif
#ifdef ATU_IATVR3
    HAL_WRITE_UINT32(ATU_IATVR3, CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_TVR);
#endif

#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM) || defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM) || defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_FLASH)
    // BAR3 is 64-bit prefetchable
    HAL_WRITE_UINT32(ATU_IABAR3, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#else
    // BAR3 is 32-bit non-prefetchable
    HAL_WRITE_UINT32(ATU_IABAR3, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
#endif
    HAL_WRITE_UINT32(ATU_IAUBAR3, 0);

    // Enable outbound ATU
    HAL_READ_UINT32(ATU_ATUCR, atucr);
    HAL_WRITE_UINT32(ATU_ATUCR, atucr | ATUCR_OUTBOUND_ATU_ENABLE);

    // Set Bus Master Enable in ATU
    HAL_READ_UINT16(ATU_ATUCMD, atucmd);
    HAL_WRITE_UINT16(ATU_ATUCMD, atucmd | ATUCMD_BUS_MASTER_ENABLE);

    HEX_DISPLAY_QUICK(2, 5);

    // Read binit
    if ((did == ATUDID_DEVICE_IOP332) || (did == ATUDID_DEVICE_IOP333))
    {
        bridge_A_binit = setup_pci_cfg_read_uint32(localbus,
                                                   localbus,
                                                   PCIE_BRIDGE_A_DEV_NUM,
                                                   PCIE_BRIDGE_A_FN_NUM,
                                                   PCIE_BINIT_OFFSET);
    }
	
    HEX_DISPLAY_QUICK(2, 6);

	
    // See if we need to enable device hiding
    if (((did == ATUDID_DEVICE_IOP332) && (*config_flags & IOP33X_DEV_HIDING)) ||
        ((did == ATUDID_DEVICE_IOP333) && (*config_flags & IOP33X_DEV_HIDING)))
        bridge_A_binit |= (PCIE_BINIT_DEV_HIDING_ENABLE | PCIE_DEVICE_HIDE_LOCKOUT);
				    
    // See if we need to enable private memory space
    if (((did == ATUDID_DEVICE_IOP332) && (*config_flags & IOP33X_PCI_PRIVMEM)) ||
        ((did == ATUDID_DEVICE_IOP333) && (*config_flags & IOP33X_PCI_PRIVMEM)))
        bridge_A_binit |= PCIE_BINIT_PCI_PRIVMEM_ENABLE;

    // See if we need to release the Retry bits
    if (*config_flags & IOP33X_CONFIG_RETRY)
    {
        cyg_uint16 bridge_A_cmd, bridge_A_exp_dctl;
        cyg_uint32 bridge_B_binit, bridge_reg, pci_offset, pcie_offset;
        cyg_uint32 bridge_A_errunc_sev, bridge_B_errunc_sev, 
	    bridge_A_esd_enhance_set, bridge_A_esd_enhance_clear,
	    bridge_B_esd_enhance_set, bridge_B_esd_enhance_clear, 
	    occar_val;
        cyg_uint8 bridge_A_priNr;

        HEX_DISPLAY_QUICK(2, 7);

        //
        // Additional steps needed for IQ80332 and IQ80333 when the Bridge is 
		// retrying (i.e. PCIe is reset as well).  If the Bridge is not retrying,
		// then the PCI-X side of the bridge has been reset without the Express side
		// being reset.
        //
        if (((did == ATUDID_DEVICE_IOP332) || (did == ATUDID_DEVICE_IOP333)) && \
			(bridge_A_binit & PCIE_BINIT_CFG_RETRY))

        {
            // Set Bus Master Enable on Bridge A
            bridge_A_cmd = setup_pci_cfg_read_uint16(localbus,
                                                     localbus,
                                                     PCIE_BRIDGE_A_DEV_NUM,
                                                     PCIE_BRIDGE_A_FN_NUM,
                                                     CYG_PCI_CFG_COMMAND);

            setup_pci_cfg_write_uint16(localbus,
                                       localbus,
                                       PCIE_BRIDGE_A_DEV_NUM,
                                       PCIE_BRIDGE_A_FN_NUM,
                                       CYG_PCI_CFG_COMMAND,
                                       bridge_A_cmd | ATUCMD_BUS_MASTER_ENABLE);

			// Read the primary bus number from the A-segment for use in creating Type1
			// transactions to the B-side
			bridge_A_priNr = *ATU_PEBPBNR;

            // Set the DCTL register in the A-side to pass Retry cycles from the ATU
            // upstream.
            bridge_A_exp_dctl = setup_pci_cfg_read_uint16(localbus,
													 localbus,
													 PCIE_BRIDGE_A_DEV_NUM,
													 PCIE_BRIDGE_A_FN_NUM,
													 PCIE_EXPRESS_DEVICE_CONTROL);

            setup_pci_cfg_write_uint16(localbus,
                                       localbus,
                                       PCIE_BRIDGE_A_DEV_NUM,
                                       PCIE_BRIDGE_A_FN_NUM,
                                       PCIE_EXPRESS_DEVICE_CONTROL,
                                       bridge_A_exp_dctl | BRIDGE_CONFIGURE_RETRY_ENABLE);

			// Implement Bridge Errata on the B-segment
			// Create Type1 transactions to B-segment using the OCCAR/OCCDR directly
            // Implement Errata workaround for:
            // Incorrect default value for PCI Express Flow Control Protocol Error Severity
			bridge_reg = PCIE_ERRUNC_SEV_OFFSET;
		    pci_offset = (cyg_uint8)(bridge_reg & 0xFC);
		    pcie_offset = (bridge_reg & 0xF00) << 16;
	        occar_val = (pcie_offset | (bridge_A_priNr << 16) | (PCIE_BRIDGE_B_DEV_NUM << 11)|\
	        			(PCIE_BRIDGE_B_FN_NUM << 8) | pci_offset | 1);
		    HAL_WRITE_UINT32(ATU_OCCAR, occar_val);
            HAL_READ_UINT32(ATU_OCCDR, bridge_B_errunc_sev);
			HAL_WRITE_UINT32(ATU_OCCDR, bridge_B_errunc_sev | PCIE_FLOW_CONTROL_ERR_SEVERITY);

		    // Implement Errata workaround for:
		    // PCI Express ESD Enhancement
			bridge_reg = PCIE_ESD_ENHANCE_SET_OFFSET;
		    pci_offset = (cyg_uint8)(bridge_reg & 0xFC);
		    pcie_offset = (bridge_reg & 0xF00) << 16;
	        occar_val = (pcie_offset | (bridge_A_priNr << 16) | (PCIE_BRIDGE_B_DEV_NUM << 11)|\
	        			(PCIE_BRIDGE_B_FN_NUM << 8) | pci_offset | 1);
		    HAL_WRITE_UINT32(ATU_OCCAR, occar_val);
            HAL_READ_UINT32(ATU_OCCDR, bridge_B_esd_enhance_set);
			HAL_WRITE_UINT32(ATU_OCCDR, bridge_B_esd_enhance_set | PCIE_ESD_ENHANCE_SET);

			bridge_reg = PCIE_ESD_ENHANCE_CLEAR_OFFSET;
		    pci_offset = (cyg_uint8)(bridge_reg & 0xFC);
		    pcie_offset = (bridge_reg & 0xF00) << 16;
	        occar_val = (pcie_offset | (bridge_A_priNr << 16) | (PCIE_BRIDGE_B_DEV_NUM << 11)|\
	        			(PCIE_BRIDGE_B_FN_NUM << 8) | pci_offset | 1);
		    HAL_WRITE_UINT32(ATU_OCCAR, occar_val);
            HAL_READ_UINT32(ATU_OCCDR, bridge_B_esd_enhance_clear);
			HAL_WRITE_UINT32(ATU_OCCDR, bridge_B_esd_enhance_clear &~ PCIE_ESD_ENHANCE_CLEAR);

	    // Fix errata 83:
	    // PCI Express ESD Enhancement
	    bridge_B_esd_enhance_set = setup_pci_cfg_read_uint32(localbus,
						  bridge_A_priNr,
						  PCIE_BRIDGE_B_DEV_NUM,
						  PCIE_BRIDGE_B_FN_NUM,
						  PCIE_ESD_ENHANCE_SET_OFFSET);
	    setup_pci_cfg_write_uint32(localbus,
				       bridge_A_priNr,
				       PCIE_BRIDGE_B_DEV_NUM,
				       PCIE_BRIDGE_B_FN_NUM,
				       PCIE_ESD_ENHANCE_SET_OFFSET,
				       bridge_B_esd_enhance_set | PCIE_ESD_ENHANCE_SET);
	    bridge_B_esd_enhance_clear = setup_pci_cfg_read_uint32(localbus,
						bridge_A_priNr,
						PCIE_BRIDGE_B_DEV_NUM,
						PCIE_BRIDGE_B_FN_NUM,
						PCIE_ESD_ENHANCE_CLEAR_OFFSET);
	    setup_pci_cfg_write_uint32(localbus,
				       bridge_A_priNr,
				       PCIE_BRIDGE_B_DEV_NUM,
				       PCIE_BRIDGE_B_FN_NUM,
				       PCIE_ESD_ENHANCE_CLEAR_OFFSET,
				       bridge_B_esd_enhance_clear | PCIE_ESD_ENHANCE_CLEAR);

            HEX_DISPLAY_QUICK(2, 8);

            // Release the retry bit on Bridge B
			bridge_reg = PCIE_BINIT_OFFSET;
		    pci_offset = (cyg_uint8)(bridge_reg & 0xFC);
		    pcie_offset = (bridge_reg & 0xF00) << 16;
	        occar_val = (pcie_offset | (bridge_A_priNr << 16) | (PCIE_BRIDGE_B_DEV_NUM << 11)|\
	        			(PCIE_BRIDGE_B_FN_NUM << 8) | pci_offset | 1);
		    HAL_WRITE_UINT32(ATU_OCCAR, occar_val);
            HAL_READ_UINT32(ATU_OCCDR, bridge_B_binit);
			HAL_WRITE_UINT32(ATU_OCCDR, bridge_B_binit &~ PCIE_BINIT_CFG_RETRY);


            // Restore Command Register on Bridge A
            setup_pci_cfg_write_uint16(localbus,
                                       localbus,
                                       PCIE_BRIDGE_A_DEV_NUM,
                                       PCIE_BRIDGE_A_FN_NUM,
                                       CYG_PCI_CFG_COMMAND,
                                       bridge_A_cmd);

			// Implement Bridge Errata on the A-segment
            // Implement Errata workaround for:
            // Incorrect default value for PCI Express Flow Control Protocol Error Severity
            bridge_A_errunc_sev = setup_pci_cfg_read_uint32(localbus,
                                                            localbus,
                                                            PCIE_BRIDGE_A_DEV_NUM,
                                                            PCIE_BRIDGE_A_FN_NUM,
                                                            PCIE_ERRUNC_SEV_OFFSET);
            setup_pci_cfg_write_uint32(localbus,
                                       localbus,
                                       PCIE_BRIDGE_A_DEV_NUM,
                                       PCIE_BRIDGE_A_FN_NUM,
                                       PCIE_ERRUNC_SEV_OFFSET,
                                       bridge_A_errunc_sev | PCIE_FLOW_CONTROL_ERR_SEVERITY);

		    // Implement Errata workaround for:
		    // PCI Express ESD Enhancement
		    bridge_A_esd_enhance_set = setup_pci_cfg_read_uint32(localbus,
							 localbus,
							 PCIE_BRIDGE_A_DEV_NUM,
							 PCIE_BRIDGE_A_FN_NUM,
							 PCIE_ESD_ENHANCE_SET_OFFSET);
		    setup_pci_cfg_write_uint32(localbus,
					       localbus,
					       PCIE_BRIDGE_A_DEV_NUM,
					       PCIE_BRIDGE_A_FN_NUM,
					       PCIE_ESD_ENHANCE_SET_OFFSET,
					       bridge_A_esd_enhance_set | PCIE_ESD_ENHANCE_SET);
		    bridge_A_esd_enhance_clear = setup_pci_cfg_read_uint32(localbus,
						       localbus,
						       PCIE_BRIDGE_A_DEV_NUM,
						       PCIE_BRIDGE_A_FN_NUM,
						       PCIE_ESD_ENHANCE_CLEAR_OFFSET);
		    setup_pci_cfg_write_uint32(localbus,
					       localbus,
					       PCIE_BRIDGE_A_DEV_NUM,
					       PCIE_BRIDGE_A_FN_NUM,
					       PCIE_ESD_ENHANCE_CLEAR_OFFSET,
					       bridge_A_esd_enhance_clear &~ PCIE_ESD_ENHANCE_CLEAR);

            // Release the retry bit on Bridge A
            // Also, clear the "Inbound Configuration Enable" bit
            bridge_A_binit &=~ PCIE_BINIT_CFG_RETRY;
			/* WARNING: The following command prevents the ATU from perfoming any 
			   subsequent type 0 config cycles to the back side of the A segment 
			   (until after a hard reset).,  Therefore if your application requires 
			   the ability for the code to tolerate an isolated secondary bus reset, 
			   you MUST comment out the next line of code.  HBA usage models will 
			   typically require this and should comment out this next line.  */
            bridge_A_binit &=~ PCIE_BINIT_INBOUND_CONFIG_ENABLE;
            setup_pci_cfg_write_uint32(localbus,
                                       localbus,
                                       PCIE_BRIDGE_A_DEV_NUM,
                                       PCIE_BRIDGE_A_FN_NUM,
                                       PCIE_BINIT_OFFSET,
                                       bridge_A_binit);
            HEX_DISPLAY_QUICK(2, 9);
        }
    }

    // Restore ATUCMD
    HAL_WRITE_UINT16(ATU_ATUCMD, atucmd);

	// Release Retry last 
    if (*config_flags & IOP33X_CONFIG_RETRY) { 
	    // Release the retry bit in the ATU
    	HAL_WRITE_UINT32(ATU_PCSR, pcsr &~ PCSR_CFG_RETRY);
	}

    return;
}
