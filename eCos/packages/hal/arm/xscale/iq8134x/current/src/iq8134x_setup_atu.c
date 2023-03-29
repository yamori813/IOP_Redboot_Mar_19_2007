/*=============================================================================
//
//      iq8134x_setup_atu.c
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
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-04
// Purpose:      Intel XScale IQ8134X CRB platform specific ATU initialization
// Description: 
//     This file contains low-level initialization code called by hal_platform_setup.h
//     Since we may be using "alternative" memory (ie different stack and heap than
//     we were compiled for), we cannot use any global data or C Library calls.
// Usage:
//     Only called by "hal_platform_setup.c"
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/iq8134x.h>             // Platform specific hardware definitions
#include <cyg/hal/hal_iop34x.h> // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/hal_spd.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_cfg.h>



// + Not used during initialization at this point, but may be
//   needed in the future so it's been modified for 
//   IOP34x
#if 0 
//
// PCI Configuration access setup
static inline void
setup_pci_config_setup(cyg_uint32 localbus, 
                       cyg_uint32 bus,
                       cyg_uint32 dev,
                       cyg_uint32 fn,
                       cyg_uint32 offset, 
                       cyg_bool   pcie)
{
    cyg_uint32 occar_val;
	cyg_uint32 temp_val;
    cyg_uint8  pci_offset;

	if (pcie == true) {
		/* IOP34X PCIe OCCAR Definitions:
			[31:24] : Bus #
			[23:19]	: Device #
			[18:16] : Function #
			[15:12] : RSVD (0000b)
			[11:08] : Extended Register #
			[07:02] : Register #
			   [01] : RSVD (0b)
			   [00] : 0 for Type0, 1 for Type1
		*/
	    /* Immediate bus use type 0 config, all others use type 1 config */
	    if (bus == localbus)
			occar_val = ((bus << 24) | (dev << 19) | (fn << 16) | (offset << 2) | 0);
    	else
			occar_val = ((bus << 24) | (dev << 19) | (fn << 16) | (offset << 2) | 1);

		// IOP34x A0 requirement - write to OCCAR and read back to make sure the write completed	
	    HAL_WRITE_UINT32(ATUE_OCCAR, occar_val);
	    HAL_READ_UINT32(ATUE_OCCAR, temp_val);
	}
	else  {
	   	/*
		* Standard PCI Configuration Space Offset
		*     must be dword-aligned
		*     8 bits only
		*/
	   	pci_offset = (cyg_uint8)(offset & 0xFC);

    	/* Immediate bus use type 0 config, all others use type 1 config */
	    if (bus == localbus)
	        occar_val = ((1 << (dev + 16)) | (dev << 11) | (fn << 8) | pci_offset | 0);
	    else
        	occar_val = ((bus << 16) | (dev << 11) | (fn << 8) | pci_offset | 1);
		// IOP34x A0 requirement - write to OCCAR and read back to make sure the write completed	
	    HAL_WRITE_UINT32(ATUX_OCCAR, occar_val);
	    HAL_READ_UINT32(ATUX_OCCAR, temp_val);
	}
}

//
// PCI Configuration access cleanup
//
static inline int
setup_pci_config_cleanup(cyg_bool pcie)
{
    cyg_uint32 status = 0, err = 0;

	if (pcie == true) 
    	HAL_READ_UINT32(ATUE_ATUSR, status);
	else
		HAL_READ_UINT32(ATUX_ATUSR, status);

    if ((status & 0xF900) != 0) {
		err = 1;
		if (pcie == true) 
        	HAL_WRITE_UINT32(ATUE_ATUSR, status & 0xF900);
		else
			HAL_WRITE_UINT32(ATUX_ATUSR, status & 0xF900);
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
                                          cyg_uint32 offset,
                                          cyg_bool   pcie)
{
    int err;
    cyg_uint8 config_data;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset, pcie);

	if (pcie == true)
    	HAL_READ_UINT32(ATUE_OCCDR, occdr_val);
	else
		HAL_READ_UINT32(ATUX_OCCDR, occdr_val);

    config_data = (occdr_val >> ((offset % 0x4) * 8)) & 0xff;

    err = setup_pci_config_cleanup(pcie);

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
                                      cyg_uint8 val,
                                      cyg_bool  pcie)
{
    int err;
    cyg_uint32 mask, temp;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset, pcie);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (((cyg_uint32)val) << ((offset % 0x4) * 8));

	if (pcie == true) { 
    	HAL_READ_UINT32(ATUE_OCCDR, occdr_val);
    	HAL_WRITE_UINT32(ATUE_OCCDR, (occdr_val & mask) | temp);
	}
	else  {
    	HAL_READ_UINT32(ATUX_OCCDR, occdr_val);
    	HAL_WRITE_UINT32(ATUX_OCCDR, (occdr_val & mask) | temp);
	}

    err = setup_pci_config_cleanup(pcie);
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
                                            cyg_uint32 offset,
                                            cyg_bool   pcie)
{
    int err;
    cyg_uint16 config_data;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset, pcie);

	if (pcie == true)
    	HAL_READ_UINT32(ATUE_OCCDR, occdr_val);
	else 
		HAL_READ_UINT32(ATUX_OCCDR, occdr_val);

    config_data = (occdr_val >> ((offset % 0x4) * 8)) & 0xffff;

    err = setup_pci_config_cleanup(pcie);

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
                                       cyg_uint16 val,
                                       cyg_bool   pcie)
{
    int err;
    cyg_uint32 mask, temp;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset, pcie);

    mask = ~(0x0000ffff << ((offset % 0x4) * 8));
    temp = (((cyg_uint32)val) << ((offset % 0x4) * 8));

	if (pcie == true) { 
    	HAL_READ_UINT32(ATUE_OCCDR, occdr_val);
    	HAL_WRITE_UINT32(ATUE_OCCDR, (occdr_val & mask) | temp);
	}
	else  {
    	HAL_READ_UINT32(ATUX_OCCDR, occdr_val);
    	HAL_WRITE_UINT32(ATUX_OCCDR, (occdr_val & mask) | temp);
	}

    err = setup_pci_config_cleanup(pcie);
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
                                            cyg_uint32 offset,
                                            cyg_bool   pcie)
{
    int err;
    cyg_uint32 config_data;
    cyg_uint32 occdr_val;

    setup_pci_config_setup(localbus, bus, dev, fn, offset, pcie);

	if (pcie == true) 
    	HAL_READ_UINT32(ATUE_OCCDR, occdr_val);
	else
		HAL_READ_UINT32(ATUX_OCCDR, occdr_val);

    config_data = occdr_val;

    err = setup_pci_config_cleanup(pcie);

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
                                       cyg_uint32 val,
                                       cyg_bool   pcie)
{
    int err;

    setup_pci_config_setup(localbus, bus, dev, fn, offset, pcie);
    
	if (pcie == true)
    	HAL_WRITE_UINT32(ATUE_OCCDR, val);
	else
		HAL_WRITE_UINT32(ATUX_OCCDR, val);
    
    err = setup_pci_config_cleanup(pcie);
}
#endif /* if 0 */


//
// Set up the ATU settings needed to allow host BIOS to configure us properly.
// Once that is done we release the retry bits
// This is done as early as possible to minimize racing w/ the host BIOS
// The bool option configuing_atue is used to determine if we're initializing
// the ATUe interface or the ATU-X so that the register offsets can be calculated
// properly.
void minimal_ATU_setup(cyg_bool sdram_installed,
                       cyg_uint32 sdram_size,
                       cyg_uint32 *sdram_pci_size,
                       cyg_uint32 *config_flags, 
                       cyg_bool   configuring_atue )
{
    cyg_uint32 privmem_size;
    cyg_uint32 pcsr;
    cyg_uint32 localbus = 0;
    cyg_uint32 atucr;
    cyg_uint16 did;
	cyg_uint32 atu_base;
	cyg_uint32 atu_reg;

    hal_platform_lowlevel_serial_debug_puts("In minimal_ATU_setup()\r\n");
	if (configuring_atue == true) {
		atu_base = ATUE_BASE;
		localbus = iop34x_pcie_bus_number();
	}
	else  {
		atu_base = ATUX_BASE;
		localbus = iop34x_pcix_bus_number();
	}
    HEX_DISPLAY_QUICK(2, 4);

	atu_reg = atu_base + ATUDID_OFFSET;
    // Read the Device ID
    HAL_READ_UINT16(atu_reg, did);

	atu_reg = atu_base + PCSR_OFFSET;
    // Read the PCSR
    HAL_READ_UINT32(atu_reg, pcsr);

    // Determine config-retry and PCI Master/Slave if possible.
	// It is possible that the other core or a FW timeout has cleared
	// retry already.   So this is not a guarantee.
    if ((pcsr & PCSR_CFG_RETRY) != 0)   {
		if (configuring_atue == true) { 
	        *config_flags |= IOP34X_CONFIG_RETRY_ATUE;
	        *config_flags &=~ IOP34X_PCI_MASTER_ATUE;
		}
		else  {
	        *config_flags |= IOP34X_CONFIG_RETRY_ATUX;
	        *config_flags &=~ IOP34X_PCI_MASTER_ATUX;
		}
			
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
    if ((sdram_installed) && (*config_flags & IOP34X_PCI_PRIVMEM))
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
    if (*config_flags & (IOP34X_DEV_HIDING|IOP34X_PCI_MASTER)) {
        downstream_hidden_size = CYGNUM_HAL_ARM_XSCALE_IOP_REDIRECT_PCI_SIZE;
    }
#endif

    // Setup ATU Inbound Window 0 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
	atu_reg = atu_base + IALR0_OFFSET;
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_RAM)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_SIZE));
#endif

	atu_reg = atu_base + IATVR0_OFFSET;
	HAL_WRITE_UINT32(atu_reg, CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_TVR);

    // BAR0 is 64-bit prefetchable memory space for the MU, if enabled
	atu_reg = atu_base + IABAR0_OFFSET;
	/* If BAR0 disabled, then clear the 64-bit and prefetchable default bits */
#ifdef CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_NOTHING
    HAL_WRITE_UINT32(atu_reg, 0);
#else
    HAL_WRITE_UINT32(atu_reg, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#endif
	atu_reg = atu_base + IAUBAR0_OFFSET;
    HAL_WRITE_UINT32(atu_reg, 0);

    // Setup ATU Inbound Window 1 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
	atu_reg = atu_base + IALR1_OFFSET;
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_SIZE));
#endif

	atu_reg = atu_base + IATVR1_OFFSET;
    HAL_WRITE_UINT32(atu_reg, CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_TVR);

	atu_reg = atu_base + IABAR1_OFFSET;
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_PRIVMEM) || defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM) || defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_FLASH) \
     ||  defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM)
    // BAR1 is 64-bit prefetchable
    HAL_WRITE_UINT32(atu_reg, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#else
    // BAR1 is 32-bit non-prefetchable
    HAL_WRITE_UINT32(atu_reg, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
#endif
	atu_reg = atu_base + IAUBAR1_OFFSET;
    HAL_WRITE_UINT32(atu_reg, 0);

    // Setup ATU Inbound Window 2 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
	atu_reg = atu_base + IALR2_OFFSET;
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_REDIRECT)
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(downstream_hidden_size));
#else
    HAL_WRITE_UINT32(atu_reg, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_SIZE));
#endif

	atu_reg = atu_base + IATVR2_OFFSET;
    HAL_WRITE_UINT32(atu_reg, CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_TVR);

	atu_reg = atu_base + IABAR2_OFFSET;
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_PRIVMEM) || defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM) || defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_FLASH)
    // BAR2 is 64-bit prefetchable
    HAL_WRITE_UINT32(atu_reg, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#elif defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_MMRS)
	// Setup BAR2 as 32-bit IO Space when sharing MMRs
    HAL_WRITE_UINT32(atu_reg, CYG_PCI_CFG_BAR_SPACE_IO);
#else
    // BAR2 is 32-bit non-prefetchable
    HAL_WRITE_UINT32(atu_reg, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
#endif
	atu_reg = atu_base + IAUBAR2_OFFSET;
    HAL_WRITE_UINT32(atu_reg, 0);

    // Setup ATU Inbound Window 3 base-configuration.
    // We want to setup enough here so that host BIOS
    // can properly configure us.
	// Only ATU-X interface has BAR3 available, so skip if we're doing ATUe
	if (configuring_atue == false) { 
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM)
	    HAL_WRITE_UINT32(ATUX_IALR3, ATU_IALR_VAL(privmem_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM)
	    HAL_WRITE_UINT32(ATUX_IALR3, ATU_IALR_VAL(*sdram_pci_size));
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_REDIRECT)
	    HAL_WRITE_UINT32(ATUX_IALR3, ATU_IALR_VAL(downstream_hidden_size));
#else
	    HAL_WRITE_UINT32(ATUX_IALR3, ATU_IALR_VAL(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_SIZE));
#endif
#ifdef ATU_IATVR3
	    HAL_WRITE_UINT32(ATUX_IATVR3, CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_TVR);
#endif

#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_PRIVMEM) || defined (CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM) || defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_FLASH)
	    // BAR3 is 64-bit prefetchable
	    HAL_WRITE_UINT32(ATUX_IABAR3, CYG_PRI_CFG_BAR_MEM_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_64);
#else
    	// BAR3 is 32-bit non-prefetchable
	    HAL_WRITE_UINT32(ATUX_IABAR3, CYG_PRI_CFG_BAR_MEM_NON_PREFETCH | CYG_PRI_CFG_BAR_MEM_TYPE_32);
#endif
	    HAL_WRITE_UINT32(ATUX_IAUBAR3, 0);
	}

	atu_reg = atu_base + ATUCR_OFFSET;
    // Enable outbound ATU
    HAL_READ_UINT32(atu_reg, atucr);
    HAL_WRITE_UINT32(atu_reg, atucr | ATUCR_OUTBOUND_ATU_ENABLE);
    HEX_DISPLAY_QUICK(2, 5);

	if (!configuring_atue) { 
		// Enable Outbound Window0 for ATU-X and disable all other windows (only using 128M outbound space)
		*ATUX_OUMBAR0 |= OUTBOUND_WINDOW_ENABLE;
		*ATUX_OUMBAR1 &= ~(OUTBOUND_WINDOW_ENABLE);
		*ATUX_OUMBAR2 &= ~(OUTBOUND_WINDOW_ENABLE);
		*ATUX_OUMBAR3 &= ~(OUTBOUND_WINDOW_ENABLE);
		*ATUX_HS_NXTP = 0x0;
	}
	else  {
		*ATUE_OUMBAR0 &= ~(OUTBOUND_WINDOW_ENABLE);
		*ATUE_OUMBAR1 |= OUTBOUND_WINDOW_ENABLE;
		*ATUE_OUMBAR2 &= ~(OUTBOUND_WINDOW_ENABLE);
		*ATUE_OUMBAR3 &= ~(OUTBOUND_WINDOW_ENABLE);
	}

	// Enable Outbound Window1 for ATUe and disable all other windows (only using 128M outbound space)
	atu_reg = atu_base + PCSR_OFFSET;
    // See if we need to release the Retry bit in current ATU
	if ((configuring_atue == true) && (*config_flags & IOP34X_CONFIG_RETRY_ATUE))  {
        // Release the retry bit in the ATU
        HAL_WRITE_UINT32(atu_reg, pcsr &~ PCSR_CFG_RETRY);
        HEX_DISPLAY_QUICK(2, 6);
	}
	else if ((configuring_atue == false) && (*config_flags & IOP34X_CONFIG_RETRY_ATUX))  {
        // Release the retry bit in the ATU
        HAL_WRITE_UINT32(atu_reg, pcsr &~ PCSR_CFG_RETRY);
        HEX_DISPLAY_QUICK(2, 7);
	}

    return;
}
