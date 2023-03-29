//==========================================================================
//
//      iop34x_pci.c
//
//      HAL support code for IOP34X PCI
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    curt.e.bruns@intel.com
// Contributors: adam.j.brooks@intel.com
//               drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <pkgconf/io_pci.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_iop34x.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#include <redboot.h>

#ifdef CYGPKG_IO_PCI


#ifdef CYGPKG_IO_PCI_DEBUG
#define pcidebug_printf diag_printf
#else
#define pcidebug_printf(...)
#endif

#ifdef VERBOSE_DEBUG
#define verbose_pcidebug_printf diag_printf
#else
#define verbose_pcidebug_printf(...)
#endif

cyg_uint64 hal_pci_alloc_base_memory;
cyg_uint32 hal_pci_alloc_base_io;
cyg_uint64 hal_pci_physical_memory_base;
cyg_uint32 hal_pci_physical_io_base;
cyg_uint64 hal_pci_inbound_window_base;
cyg_uint64 hal_pci_inbound_window_mask;

cyg_uint64 hal_pcie_alloc_base_memory;
cyg_uint32 hal_pcie_alloc_base_io;
cyg_uint64 hal_pcie_physical_memory_base;
cyg_uint64 hal_pcix_physical_memory_base;
cyg_uint32 hal_pcie_physical_io_base;
cyg_uint64 hal_pcie_inbound_window_base;
cyg_uint64 hal_pcie_inbound_window_mask;

cyg_uint32 pcix_bus_num;
cyg_uint32 pcie_bus_num;

// IOP34X ATU-X Window Usage:
//   Inbound Window 0 - Access to IOP34X memory mapped registers.
//   Inbound Window 1 - Access to SDRAM
//   Inbound Window 2 - Not used.
//   Inbound Window 3 - Not used.
//
//   Outbound Translate Window 0 - Translates to PCI-X Mem xactions from IB 0x1.0000.0000
//   Outbound Translate Window 1 - Unused.
//   Outbound Translate Window 2 - Unused.
//   Outbound Translate Window 3 - Unused.
//
//   Outbound IO Window	- Tranlates to PCI-X IO xactions from IB 0x9000.0000
//
#define PCI_PARAM_ERROR(m, n) {HEX_DISPLAY_QUICK(m, n); while (1);}

extern int hal_pci_cfg_flags;
static cyg_uint8 configure_atue(cyg_uint32 localbus);
static cyg_uint8 configure_atux(cyg_uint32 localbus);
static cyg_bool requesting_pcie_bus ( cyg_uint32 bus );
static void modify_omw_pt(cyg_uint64 hal_pci_physical_memory_base, cyg_uint32 outbound_mem_window);
inline cyg_uint8 iop34x_pcie_bus_number(void);
inline cyg_uint8 iop34x_pcix_bus_number(void);
static void set_pci_phys_base_ram(void);

// Use "naked" attribute to suppress C prologue/epilogue
// This is a data abort handler which simply returns. Data aborts
// occur during configuration cycles if no device is present.
static void __attribute__ ((naked)) __pci_abort_handler(void)
{
	asm ( "subs	pc, lr, #4\n" );
}

static cyg_uint32 orig_abort_vec;


// Check if a device needs to be ignored.  If we're an endpoint on the bus that is
// being checked, we will ignore the device.
// If we're RC on the PCIe bus, we check our Express bus number and only scan 
// for device #0.  PCIe spec states that a PCIe device will respond to every device#
// during a config read command.  That will mess up RedBoot's scan routine, so we ignore
// any device > 0.
bool cyg_hal_plf_ignore_devices(int bus, int device, int function)
{
	cyg_bool pcie = requesting_pcie_bus(bus);
	cyg_uint8 localbus_pcie;
	
	// Check that we are the master on the ATU that we are trying to do 
	// do config cycles out of.  
	// If we're an endpoint, we should not access any device
	if (pcie)
	{
		localbus_pcie = iop34x_pcie_bus_number();
		if (!(hal_pci_cfg_flags & IOP34X_PCI_MASTER_ATUE))
		{
			verbose_pcidebug_printf ("PCIe Endpoint - IGNORING ALL DEVICES\n");
			verbose_pcidebug_printf ("PCIe Ignoring: Bus: %d, Device: %d, Function: %d\n", bus, device, function);
			return true;
		}
		// Check if we're scanning the local pcie bus, if so, we only want to scan device0
		else if ((device > 0) && (localbus_pcie == bus))
		{
			verbose_pcidebug_printf ("PCIe Ignoring: Bus: %d, Device: %d, Function: %d\n", bus, device, function); 
			return true;
		}
	}
	else
	{
		if (!(hal_pci_cfg_flags & IOP34X_PCI_MASTER_ATUX))
 			return true;
		else
		{
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
			char prodcode = *IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK;
			//diag_printf("b=%x,d=%x=",bus, device);
			if(prodcode==IQ8134X_PROD_CODE_IQ8134xMC_MODE2)
			{
				if((bus==iop34x_pcix_bus_number()) && (device>0))
				{
					//diag_printf("T\n");
					return true;
				}
				else
				{
					//diag_printf("F\n");
					return false;
				}
			}
			else
#endif
			{
				//diag_printf("=NOT MODE2\n");
				return false;
			}
		}
	}
	return false;
}


config_read(int dev, int offset)
{
	int config_data;
	*ATUX_OCCAR = ((1 << (dev + 16)) | (offset));
	config_data = *ATUX_OCCAR;
	config_data = *ATUX_OCCDR;
	return config_data;
}


// PCI Initialization Function for IOP34x.  This will configure the PCI-X
// and PCIe busses.  If we're the Master on those busses, we configure them,
// else we copy the inbound memory BAR information from the host-connected 
// bus to the the other bus. 
void
cyg_hal_plf_pci_init(void)
{
	cyg_uint8 localbus, next_bus;
	cyg_uint64 dram_limit;
	cyg_uint64 alloc_base = CYGPKG_IO_PCI_MEMORY_ALLOCATION_BASE;
	cyg_uint32 pcsr, fcn_num;
	cyg_uint16 device_id;
	cyg_uint8 rev_id;
	cyg_uint32 config_data,devices,loads,low_cap,dev,status,pcix,pointer,old_pointer,delay_time,max_pci_devs;
	char prodcode;

		
	if(atue_safe())
	{
		device_id = *ATUE_ATUDID;
		rev_id = *ATUE_ATURID;
	}
	else
	{
		device_id = *ATUX_ATUDID;
		rev_id = *ATUX_ATURID;
	}
	
	HAL_PCI_ALLOC_BASE_MEMORY = alloc_base;

	pcidebug_printf ("\n\n\n   <CYG_HAL_PLF_PCI_INIT>\n");
	pcidebug_printf (" with MemAllocBase: 0x%08x\n", alloc_base);

	if(atue_safe())
		pcidebug_printf("Redboot will be initializing the ATUe\n");
	else
		pcidebug_printf("Redboot will NOT be initializing the ATUe\n");
	
	if(atux_safe())
		pcidebug_printf("Redboot will be initializing the ATU-X\n");
	else
		pcidebug_printf("Redboot will NOT be initializing the ATU-X\n");
	

	// RAM Version has BARs already setup, so we need to figure out where the outbound window
	// offset is so the GigE device driver can access its BARs.  Need to check the outbound
	// memory window PT entries to find the lower 32-bits.
	#if defined(CYG_HAL_STARTUP_RAM)
		set_pci_phys_base_ram();
	#endif

	
	if(atux_safe())
	{
		// Enable Outbound ATU bits in control register
		*ATUX_ATUCR |= ATUCR_OUTBOUND_ATU_ENABLE;

		// Configure Outbound IO window to be 1:1 with RedBoot PT entries.
		// OIOBAR represents [35:16] of internal bus address to compare against, so we RSH by 4 to
		// claim against the standard IOP 0x9000.0000 address.
		*ATUX_OIOBAR = (PCI_OUTBOUND_IO_WINDOW >> 4);
		
		// If atux is CR, take bus out of reset, allow ATU-X to act as a bus master,
		// respond to PCI memory accesses, and assert S_SERR#
		if (*ATUX_PCSR & PCSR_ATUX_CENTRAL_RESOURCE_MODE)
		{
			*ATUX_ATUCMD = (CYG_PCI_CFG_COMMAND_SERR   | \
			                CYG_PCI_CFG_COMMAND_PARITY | \
			                CYG_PCI_CFG_COMMAND_MASTER | \
			                CYG_PCI_CFG_COMMAND_MEMORY);
			
			// Take PCI-X bus out of reset
			hal_pci_cfg_flags |= IOP34X_PCI_MASTER_ATUX;
			
			pcidebug_printf ("PBIU_ESSTSR0: 0x%08x\n", *PBIU_ESSTSR0);
			pcidebug_printf ("ATUX is CR.  CMD: 0x%08x, PCSR: 0x%08x, CR: 0x%08x, Flags: 0x%04x\n", \
					*ATUX_ATUCMD, *ATUX_PCSR, *ATUX_ATUCR, hal_pci_cfg_flags);
//			pcidebug_printf ("ATUE PCSR: 0x%08x\n", *ATUE_PCSR);

#if defined (CYG_HAL_STARTUP_ROM) || defined (CYG_HAL_STARTUP_ROMRAM)
			config_data = *ATUX_PCSR;
			config_data &= ~(PCSR_PCIX_CAPABILITY_MASK);
			config_data |= (PCSR_PCI_MODE);
			*ATUX_PCSR = config_data;
			
			pcidebug_printf("New PCSR = 0x%08x\n",config_data);
			
			// Delay until the init pattern is on the bus for at least 10clks
			hal_delay_us(10);
			
			// Deassert Reset on the PCI-X bus
			config_data &= ~(PCSR_ATUX_CR_BUS_RESET);
			*ATUX_PCSR = config_data;
			
			// Wait for Reset to be deasserted on PCI-X bus
			while (*ATUX_PCSR & PCSR_ATUX_CR_BUS_RESET);
	
			// Wait a few sec after releasing reset  (Trhfa = 2^25 clocks, which is ~1.01sec @ 33MHz)
			pcidebug_printf("Waiting 2^25 clocks are releasing reset\n");
			hal_delay_us(1010000);

			pcidebug_printf("Starting PCIXCAP psudeo-scan\n");
			devices = 0;
			low_cap = 5;
			loads = 0;

#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
			prodcode = *IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK;
			if(prodcode==IQ8134X_PROD_CODE_IQ8134xMC_MODE2)
				max_pci_devs = 1;
			else
#endif
				max_pci_devs = 32;

			// Put the dummy Data Abort handler in place
			orig_abort_vec = ((volatile cyg_uint32 *)0x20)[4];
			((volatile unsigned *)0x20)[4] = (unsigned)__pci_abort_handler;
			
			for(dev = 0; dev < max_pci_devs; dev++)
			{
				// Read the device/vendor ID to see if a device is present
				config_data = config_read(dev,0);
				hal_delay_us(1000); // NEED the proper delay structure here
				status = *ATUX_ATUISR;
				if ((status & 0x1F) == 0)
				{
					devices++;
					pcidebug_printf("dev=%d:devid/venid=%x,",dev,config_data);
					
					// Check to see if the device has extended capabilities.  If it does, it may be PCI-X,
					// If not, then it is a PCI device
					config_data = config_read(dev,4);
					pcidebug_printf("stat=%x,",config_data);
					if(config_data & (1<<(4+16)))
					{
						// The capabilities bit is set, we will traverse the capabilities linked list looking for
						// the PCI-X capability structure(ID=0x7)
						pcidebug_printf("Cap bit set\n");
						config_data = config_read(dev,0x34);
						pcix = false;
						pointer = config_data & 0xff;
						old_pointer = 0;
						while(pointer && (pointer != old_pointer))
						{
							pcidebug_printf("  Cap struct at=%x,",pointer);
							config_data = config_read(dev,pointer);
							old_pointer = pointer;
							pointer = (config_data & 0xff00) >> 8;
							pcidebug_printf("1st word=%08x,ID=%x,new pointer=%x\n", config_data,(config_data & 0xFF),pointer);
							if((config_data & 0xff) == 7)
							{
								// We found the PCI-X capability structure.  Check to see if we are MODE 1 or MODE 2
								// Sunrise Lake PCI-X devices do not implement the PCI-X 266 or PCI-X 533 capbility bits
								// Therefore we look at the ECC capability field to infer if the device is MODE 1 or MODE2
								pcix = true;
								if(config_data & (3 << (16+12)))
								{
									if (low_cap > 5)
										low_cap = 5;
									pcidebug_printf("    PCI-X 266 MODE2\n");
								}
								else
								{
									config_data = config_read(dev,old_pointer+4);
									pcidebug_printf("    PCIXSR(0x%08x)=%08x\n",(old_pointer+4),config_data);
									if(config_data & (1<<17))
									{
										pcidebug_printf("    PCI-X 133\n");
										if(low_cap > 4)
											low_cap = 4;
									}
									else
									{
										pcidebug_printf("    PCI-X 66\n");
										if(low_cap > 2)
											low_cap = 2;
									}
								}
							}
						}
						if(pcix == false)
							low_cap = 1;
					}
					else
					{
						pcidebug_printf("Cap bit NOT set, must be PCI device\n");
						low_cap = 1;
					}
				}
				else
				{
					*ATUX_ATUISR &= 0x1F;
					status = *ATUX_ATUISR;
					verbose_pcidebug_printf("dev %d:Not Present\n",dev);
				}
			}
			((volatile unsigned *)0x20)[4] = orig_abort_vec;


			// The speed of the bus is also determined by the number of devices on the bus.
			// Drop the low_cap variable if there are two many loads for the given mode
			// The NUM_SLOTS define is used to specify loads that the chip can not scan for.
			// We assume that a slot is 2 loads.
			loads = devices + ((CYGNUM_HAL_NUM_SLOTS)*2);
			pcidebug_printf("Found %d device(s), %d load(s), and the lowest capability is %d\n",devices,loads,low_cap);
			if((devices > 1) && (low_cap >= 5))
				low_cap = 4;

			if((!(*ATUX_PCSR & (PCSR_PMODE2))) && (low_cap >=5))
				low_cap = 4;
			
			if((loads > 3) && (low_cap >= 4))
				low_cap = 3;
			
			if((loads > 6) && (low_cap >= 3))
				low_cap = 2;
 
			config_data = *ATUX_PCSR;
			
			pcidebug_printf("PCSR before bus back in reset = 0x%08x\n",config_data);
			config_data |= (PCSR_ATUX_CR_BUS_RESET); 
			*ATUX_PCSR = config_data;
			config_data &= ~(PCSR_PCIX_CAPABILITY_MASK);
			// Translate the mode into the PCSR format. Also set the delay that is required after releasign reset.
			switch(low_cap)
			{
				case 5:
					delay_time = 510000;
					config_data |= (PCSR_PCIX_266_133_MODE);
					break;
				case 4:
					delay_time = 510000;
					config_data |= (PCSR_PCIX_133_MODE);
					break;
				case 3:
					delay_time = 680000;
					config_data |= (PCSR_PCIX_100_MODE);
					break;
				case 2:
					delay_time = 1010000;
					config_data |= (PCSR_PCIX_66_MODE);
					break;
				case 1:
				default:
					delay_time = 1010000;
					config_data |= (PCSR_PCI_MODE);
					break;
			}
			
			// Write to the PCSR to update the init pattern, then hold the bus in reset for an additional 1ms
			// to meet the PCI min reset time
			
			
			*ATUX_PCSR = config_data;

			hal_delay_us(1000);

			*ATUX_PCSR &= ~(PCSR_ATUX_CR_BUS_RESET);
			
			while (*ATUX_PCSR & PCSR_ATUX_CR_BUS_RESET);
			pcidebug_printf("Waiting 2^25 clocks after releasing reset\n");
			// Wait for the minimum time allowed based on the bus speed.
			hal_delay_us(delay_time);
#endif
		}
		else // We're an endpoint - clear Master Flag for ATU-X
			hal_pci_cfg_flags &= ~IOP34X_PCI_MASTER_ATUX;
	}
	else 
	{	
		// Set the flag for bus master
		// This is done because later on Redboot determines which end is master
		// by checking what bus isn't master
		hal_pci_cfg_flags |= IOP34X_PCI_MASTER_ATUX;
	}
		
	if(atue_safe())
	{
		// Enable Outbound ATU bits in control register
		*ATUE_ATUCR |= ATUCR_OUTBOUND_ATU_ENABLE;

		// Set OIOBAR to the Outbound Window setting (default: 0x9010.0000).
		// Also set the Function number to either 0 or 5 based on what the ATU-X is set to.
		// (i.e. If ATU-X is function 5, then ATUe needs to be function 0 and vice-versa).
		*ATUE_OIOBAR = (PCIE_OUTBOUND_IO_WINDOW >> 4);
		
		
		// Check if ATUe is RC and if so, enable the ATUCMD register
		// allow ATU-E to act as a bus master, respond to PCI memory accesses,
		// and assert S_SERR#
		if (!(*ATUE_PCSR & PCSR_ATUE_END_POINT_MODE))
		{
			hal_pci_cfg_flags |= IOP34X_PCI_MASTER_ATUE;
			*ATUE_ATUCMD = (CYG_PCI_CFG_COMMAND_SERR   | \
			                CYG_PCI_CFG_COMMAND_PARITY | \
			                CYG_PCI_CFG_COMMAND_MASTER | \
			                CYG_PCI_CFG_COMMAND_MEMORY);
			fcn_num = 0; // Set provional fcn_num incase we are SI
		}
		else
		{
			// We're a PCIe endpoint - clear the Master Flag
			hal_pci_cfg_flags &= ~IOP34X_PCI_MASTER_ATUE;
			
			fcn_num = 5; // Set provional fcn_num incase we are SI
		}
		
		
		if(atux_safe())
		{
			if ((*ATUX_PCIX_SR & ATU_PX_SR_FCN_NUM_MASK) == 5)
				fcn_num = 0;
			else
				fcn_num = 5;
		}
			
			
		*ATUE_OIOBAR  |= fcn_num;
		*ATUE_OCCFN   |= fcn_num;
		*ATUE_OUMBAR0 |= (fcn_num << 28);
		*ATUE_OUMBAR1 |= (fcn_num << 28);
		*ATUE_OUMBAR2 |= (fcn_num << 28);
		*ATUE_OUMBAR3 |= (fcn_num << 28);
	}
	else
	{
		// Set the flag for bus master
		// This is done because later on Redboot determines which end is master
		// by checking what bus isn't master
		hal_pci_cfg_flags |= IOP34X_PCI_MASTER_ATUE;
	}



    // Setup up limit of sharing SDRAM over PCI.
    dram_limit = (~(cyg_uint64)0 - (hal_dram_pci_size - 1)) &~ 0x3f;
    hal_pci_inbound_window_mask = ~dram_limit;

#if defined (CYG_HAL_STARTUP_ROM) || defined (CYG_HAL_STARTUP_ROMRAM)
	// If we're an endpoint on the bus the host is responsible for configuring our BARs.  
	// If we're the Master on either bus we will have to scan the bus and give out
	// Memory and IO space to the devices on those busses.  
	// First - if we're NOT the master on PCI-X, then we're a PCI-X endpoint and we
	// will be RC on the PCIe so we can go configure devices on our PCIe leg.
	if ((hal_pci_cfg_flags & IOP34X_PCI_MASTER_ATUX) == 0)
	{ 
		localbus = iop34x_pcix_bus_number();
		pcidebug_printf ("Config Flags: 0x%x\n", hal_pci_cfg_flags);
		pcidebug_printf ("PCI-X Endpoint - ");
		
		if(atue_safe())
		{
			pcidebug_printf("configuring ATUe w/ Bus#: %d\n", (localbus+1));
			configure_atue(localbus+1);
		}
		else
			pcidebug_printf("Single Interface or Redboot does not own ATUe, NOT configuring ATUe\n");
	}
			   
	// Else check if we're an endpoint on the PCIe leg
	else if ((hal_pci_cfg_flags & IOP34X_PCI_MASTER_ATUE) == 0)
	{
		localbus = iop34x_pcie_bus_number();
		pcidebug_printf ("Config Flags: 0x%x\n", hal_pci_cfg_flags);
		pcidebug_printf ("ATU-e bus is: %d\n", localbus);
		
		if(atux_safe())
		{
			pcidebug_printf("configuring ATU-X w/ Bus#: 0\n");
			configure_atux(0);
		}
		else
			pcidebug_printf("Single Interface or Redboot does not own ATU-X, NOT configuring ATU-X\n");
	}
	// Else we're the Master on both PCI-X and PCIe legs.  We will initialize the
	// PCI-X bus first and then the PCIe bus.
	else
	{
		pcidebug_printf ("Master on both busses!\n");
		pcidebug_printf ("Config Flags: 0x%x\n", hal_pci_cfg_flags);
	
		if(atux_safe())
		{
			// Clear the Bus# field in ATU-X PCSR
			*ATUX_PCIX_SR &= ~(ATU_PX_SR_BUS_NUM_MASK);
		}
		
		if(atue_safe())
		{
			// Clear Bus# field in ATUe PCSR
			*ATUE_PCSR &= ~(0xFF << 24);
			// Before we let the HAL configure the PCI-X leg, we need to set the PCIe bus
			// number to some arbitrarily high value so that it doesn't intefere with the PCI-X
			// numbering and the error checking because the two bus numbers can't be equal if both
			// legs are masters.
			*ATUE_PCSR |= ((CYG_PCI_MAX_BUS/2) << 24);
			
		}

		if(atux_safe())
		{
			// Configure ATU-X
			next_bus = configure_atux(0);
			pcidebug_printf ("Back from configure atux.  Localbus: %d, NextBus: %d\n", localbus, next_bus);
		}
		else
			next_bus = 0;
		
		if(atue_safe())
		{
			// Copy ATU-X to ATUe and set bus number to highest ATU-X bus number + 1
			configure_atue(next_bus);
		}
	}
#endif // STARTUP_ROM

	if(atux_safe())
	{
		// Setup the Virt-to-bus offset value to
		// whichever BAR is redirected to SDRAM
			// This value should be the same regardless if our i/f is PCI-X or PCIe
#if defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR0_FUNCTION_RAM)
		hal_pci_inbound_window_base = (((cyg_uint64)*ATUX_IAUBAR0) << 32) | (*ATUX_IABAR0 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR1_FUNCTION_RAM)
		hal_pci_inbound_window_base = (((cyg_uint64)*ATUX_IAUBAR1) << 32) | (*ATUX_IABAR1 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM)
		hal_pci_inbound_window_base = (((cyg_uint64)*ATUX_IAUBAR2) << 32) | (*ATUX_IABAR2 & CYG_PRI_CFG_BAR_MEM_MASK);
#elif defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR3_FUNCTION_RAM)
		hal_pci_inbound_window_base = (((cyg_uint64)*ATUX_IAUBAR3) << 32) | (*ATUX_IABAR3 & CYG_PRI_CFG_BAR_MEM_MASK);
#else
		#error NO BAR IS CONFIGURED TO SHARE RAM
#endif
		
		// enable outbound ATU on ATUX Leg
		*ATUX_ATUCR  |= ATUCR_OUTBOUND_ATU_ENABLE;
	}
	
	if(atue_safe())
	{
		// enable outbound ATU on ATUe Leg
	    *ATUE_ATUCR  |= ATUCR_OUTBOUND_ATU_ENABLE;
	}

}



// We are the master on the ATU-X leg, so configure our base registers.
cyg_uint8 configure_atux(cyg_uint32 localbus)
{
	cyg_uint64 pcix_base = HAL_PCI_ALLOC_BASE_MEMORY;
	cyg_uint8 next_bus;
	cyg_uint32 pxsr;
	cyg_uint32 fcn_num;

	// We better be the master on the PCI-X bus

	if ((hal_pci_cfg_flags & IOP34X_PCI_MASTER_ATUX) == 0)
		PCI_PARAM_ERROR(4,3);

	pcidebug_printf ("Setting ATU-X bus number to : %d\n", localbus);

	// Clear Bus Number field    
	HAL_READ_UINT32(ATUX_PCIX_SR,  pxsr);
	pxsr &= ~(ATU_PX_SR_BUS_NUM_MASK);
	pxsr |= (localbus << 8);
	HAL_WRITE_UINT32(ATUX_PCIX_SR, pxsr);

	// Determine ATU-X function number and copy it to all registers that need to know
	fcn_num = (*ATUX_PCIX_SR & ATU_PX_SR_FCN_NUM_MASK);
	*ATUX_OIOBAR  |= fcn_num;
	*ATUX_OCCFN   |= fcn_num;
	*ATUX_OUMBAR0 |= (fcn_num << 28);
	*ATUX_OUMBAR1 |= (fcn_num << 28);
	*ATUX_OUMBAR2 |= (fcn_num << 28);
	*ATUX_OUMBAR3 |= (fcn_num << 28);

	// Assign BAR0
	if (ATU_IALR_SIZE(*ATUX_IALR0))
	{ 
		pcix_base = cyg_pci_boundary_align(pcix_base, ATU_IALR_SIZE(*ATUX_IALR0));
		*ATUX_IABAR0 = (cyg_uint32)(pcix_base & 0xFFFFFFFF) | (*ATUX_IABAR0 &~ CYG_PRI_CFG_BAR_MEM_MASK);
		*ATUX_IAUBAR0 = (cyg_uint32)(pcix_base >> 32);
		pcix_base += ATU_IALR_SIZE(*ATUX_IALR0);
		pcidebug_printf ("\n\nATUX_BAR0 base: 0x%08x\n", *ATUX_IABAR0);
	}

	// Assign BAR1
	if (ATU_IALR_SIZE(*ATUX_IALR1))
	{ 
		pcix_base = cyg_pci_boundary_align(pcix_base, ATU_IALR_SIZE(*ATUX_IALR1));
		*ATUX_IABAR1 = (cyg_uint32)(pcix_base & 0xFFFFFFFF) | (*ATUX_IABAR1 &~ CYG_PRI_CFG_BAR_MEM_MASK);
		*ATUX_IAUBAR1 = (cyg_uint32)(pcix_base >> 32);
		pcix_base += ATU_IALR_SIZE(*ATUX_IALR1);
		pcidebug_printf ("ATUX_BAR1 base: 0x%08x\n", *ATUX_IABAR1);
	}

	// Assign BAR2
	if (ATU_IALR_SIZE(*ATUX_IALR2))
	{ 
		pcix_base = cyg_pci_boundary_align(pcix_base, ATU_IALR_SIZE(*ATUX_IALR2));
		*ATUX_IABAR2 = (cyg_uint32)(pcix_base & 0xFFFFFFFF) | (*ATUX_IABAR2 &~ CYG_PRI_CFG_BAR_MEM_MASK);
		*ATUX_IAUBAR2 = (cyg_uint32)(pcix_base >> 32);
		pcix_base += ATU_IALR_SIZE(*ATUX_IALR2);
		pcidebug_printf ("ATUX_BAR2 base: 0x%08x\n", *ATUX_IABAR2);
	}

    // Assign BAR3
	if (ATU_IALR_SIZE(*ATUX_IALR3))
	{ 
		pcix_base = cyg_pci_boundary_align(pcix_base, ATU_IALR_SIZE(*ATUX_IALR3));
		*ATUX_IABAR3 = (cyg_uint32)(pcix_base & 0xFFFFFFFF) | (*ATUX_IABAR3 &~ CYG_PRI_CFG_BAR_MEM_MASK);
		*ATUX_IAUBAR3 = (cyg_uint32)(pcix_base >> 32);
		pcix_base += ATU_IALR_SIZE(*ATUX_IALR3);
		pcidebug_printf ("ATUX_BAR3 base: 0x%08x\n\n\n", *ATUX_IABAR3);
	}

    // allow ATU-X to act as a bus master, respond to PCI memory accesses,
    // and assert S_SERR#
    *ATUX_ATUCMD = (CYG_PCI_CFG_COMMAND_SERR   | \
		   CYG_PCI_CFG_COMMAND_PARITY | \
		   CYG_PCI_CFG_COMMAND_MASTER | \
		   CYG_PCI_CFG_COMMAND_MEMORY);

	// Setup the PCI subsystem memory allocation pointer to point to
	// next available PCI address space.
	// SuperSections must be aligned on 16M boundary
   	pcix_base = cyg_pci_boundary_align(pcix_base, SZ_16M);
	hal_pci_alloc_base_memory = pcix_base;
	pcidebug_printf ("Setting hal_pci_alloc_base_memory to: 0x%08x\n", (int)pcix_base);
	
	modify_omw_pt(hal_pci_alloc_base_memory, PCI_OUTBOUND_MEM_WINDOW);
	
	// Setup the PCI subsystem IO allocation pointer to the CDL configured value
	hal_pci_alloc_base_io = CYGPKG_IO_PCI_IO_ALLOCATION_BASE;

	// set the outbound window PCI addresses
	*ATUX_OUMWTVR0 = 0;
	// outbound I/O window - cdl option default is 0x9000.0000
	*ATUX_OIOWTVR = hal_pci_alloc_base_io;

	hal_pci_physical_memory_base = hal_pcix_physical_memory_base = ((cyg_uint64)*ATUX_OUMWTVR0 << 32) | 
		(cyg_uint64)(PCI_OUTBOUND_MEM_WINDOW_0 - pcix_base);
	
	hal_pci_physical_io_base = PCI_OUTBOUND_IO_WINDOW - *ATUX_OIOWTVR;

	pcidebug_printf ("<SETUP_ATUX>Aligned pcix_base to: 0x%08x\n", (int)pcix_base);
	pcidebug_printf ("<SETUP_ATUX>ATUX_OIOWTVR: 0x%08x\n", *ATUX_OIOWTVR);	
	pcidebug_printf ("<SETUP_ATUX>PCI_OUT_IO  : 0x%08x\n", PCI_OUTBOUND_IO_WINDOW);	
	pcidebug_printf ("<SETUP_ATUX>Setting hal_pci_alloc_base_memory to: 0x%08x\n", (int)pcix_base);
	pcidebug_printf ("<SETUP_ATUX>Setting hal_pci_physical_memory   to: 0x%08x\n", hal_pci_physical_memory_base);
	pcidebug_printf ("<SETUP_ATUX>Setting hal_pci_alloc_io  to: 0x%08x\n", HAL_PCI_ALLOC_BASE_IO);
	pcidebug_printf ("<SETUP_ATUX>Setting hal_pci_physical_io to: 0x%08x\n", hal_pci_physical_io_base);

	cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY);
	cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

	// Scan and allocate memory/io space to any device on our PCI-X bus
	// Next bus is updated in case any bridges are encountered
	pcidebug_printf ("In Function: %s\n", __FUNCTION__);
	pcidebug_printf ("Configuring ATUX Bus.  Num: %d\n", localbus);

	next_bus = localbus + 1;
	cyg_pci_configure_bus(localbus, &next_bus);
	return(next_bus);
}


// We are the Master/Root Complex on the ATUe leg, so configure our 
// base registers.
static cyg_uint8 configure_atue(cyg_uint32 localbus)
{
	cyg_uint64 pcie_base = CYGPKG_IO_PCI_MEMORY_ALLOCATION_BASE;
	cyg_uint32 pcie_io_base =  CYGPKG_IO_PCI_IO_ALLOCATION_BASE;
	cyg_uint8 next_bus = localbus + 1;
	
	pcidebug_printf ("Setting ATUe bus number to : %d\n", localbus);

	// Set up Bus# field
	*ATUE_PCSR &= ~(0xFF << 24);
	*ATUE_PCSR |=  (localbus << 24);

     // Assign BAR0
	if (ATU_IALR_SIZE(*ATUE_IALR0))
	{ 
    		pcie_base = cyg_pci_boundary_align(pcie_base, ATU_IALR_SIZE(*ATUE_IALR0));
		*ATUE_IABAR0 = (cyg_uint32)(pcie_base & 0xFFFFFFFF) | (*ATUE_IABAR0 &~ CYG_PRI_CFG_BAR_MEM_MASK);
		*ATUE_IAUBAR0 = (cyg_uint32)(pcie_base >> 32);
		pcie_base += ATU_IALR_SIZE(*ATUE_IALR0);
		pcidebug_printf ("\n\nATUE_BAR0 base: 0x%08x\n", *ATUE_IABAR0);
	}

	// Assign BAR1
	if (ATU_IALR_SIZE(*ATUE_IALR1))
	{ 
		pcie_base = cyg_pci_boundary_align(pcie_base, ATU_IALR_SIZE(*ATUE_IALR1));
		*ATUE_IABAR1 = (cyg_uint32)(pcie_base & 0xFFFFFFFF) | (*ATUE_IABAR1 &~ CYG_PRI_CFG_BAR_MEM_MASK);
		*ATUE_IAUBAR1 = (cyg_uint32)(pcie_base >> 32);
		pcie_base += ATU_IALR_SIZE(*ATUE_IALR1);
		pcidebug_printf ("ATUE_BAR1 base: 0x%08x\n", *ATUE_IABAR1);
	}

	// Assign BAR2
	if (ATU_IALR_SIZE(*ATUE_IALR2))
	{ 
		// Check if BAR2 is using IO Space
		if (*ATUE_IABAR2 & CYG_PCI_CFG_BAR_SPACE_IO)
		{ 
			pcie_io_base = cyg_pci_boundary_align(pcie_io_base, ATU_IALR_SIZE(*ATUE_IALR2));
			*ATUE_IABAR2 = (cyg_uint32)(pcie_io_base & 0xFFFFFFFF) | (*ATUE_IABAR2 &~ CYG_PRI_CFG_BAR_MEM_MASK);
			*ATUE_IAUBAR2 = 0x0;
			pcie_io_base += ATU_IALR_SIZE(*ATUE_IALR2);
		}
		else
		{
			pcie_base = cyg_pci_boundary_align(pcie_base, ATU_IALR_SIZE(*ATUE_IALR2));
			*ATUE_IABAR2 = (cyg_uint32)(pcie_base & 0xFFFFFFFF) | (*ATUE_IABAR2 &~ CYG_PRI_CFG_BAR_MEM_MASK);
			*ATUE_IAUBAR2 = (cyg_uint32)(pcie_base >> 32);
			pcie_base += ATU_IALR_SIZE(*ATUE_IALR2);
		}
		pcidebug_printf ("ATUE_BAR2 base: 0x%08x\n", *ATUE_IABAR2);
	}

    // allow ATU-E to act as a bus master, respond to PCI memory and IO accesses,
    // and assert S_SERR#
    *ATUE_ATUCMD = (CYG_PCI_CFG_COMMAND_SERR   | \
		   CYG_PCI_CFG_COMMAND_PARITY | \
		   CYG_PCI_CFG_COMMAND_MASTER | \
		   CYG_PCI_CFG_COMMAND_IO | \
		   CYG_PCI_CFG_COMMAND_MEMORY);

	// Setup the PCI subsystem memory allocation pointer to point to
	// next available PCI address space.
	// SuperSections must be aligned on 16M boundary
	pcie_base = cyg_pci_boundary_align(pcie_base, SZ_16M);
    	hal_pcie_alloc_base_memory = pcie_base;

	pcidebug_printf ("Setting hal_pci_alloc_base_memory in configure_atue to: 0x%08x\n", (int)pcie_base);

	modify_omw_pt(hal_pcie_alloc_base_memory, PCIE_OUTBOUND_MEM_WINDOW);
	
	// Setup the PCIe subsystem IO allocation pointer
	hal_pcie_alloc_base_io = pcie_io_base;
	
	// set the outbound window PCI addresses
	*ATUE_OUMWTVR0 = 0;
	// outbound I/O window
	*ATUE_OIOWTVR = hal_pcie_alloc_base_io;
	
	hal_pci_physical_memory_base = hal_pcie_physical_memory_base = ((cyg_uint64)*ATUE_OUMWTVR0 << 32) | 
		(cyg_uint64)(PCIE_OUTBOUND_MEM_WINDOW - pcie_base);
	
	hal_pci_physical_io_base = hal_pcie_physical_io_base = PCIE_OUTBOUND_IO_WINDOW - *ATUE_OIOWTVR;
				    
	pcidebug_printf ("<SETUP_ATUE>Aligned pcie_base to: 0x%08x\n", (int)pcie_base);
	pcidebug_printf ("<SETUP_ATUE>ATUE_OIOWTVR: 0x%08x\n", *ATUE_OIOWTVR);	
	pcidebug_printf ("<SETUP_ATUE>PCI_OUT_IO  : 0x%08x\n", PCIE_OUTBOUND_IO_WINDOW);	
	pcidebug_printf ("<SETUP_ATUE>Setting hal_pci_alloc_base_memory to: 0x%08x\n", (int)pcie_base);
	pcidebug_printf ("<SETUP_ATUE>Setting hal_pcie_physical_memory   to: 0x%08x\n", hal_pcie_physical_memory_base);
	pcidebug_printf ("<SETUP_ATUE>Setting hal_pci_alloc_io  to: 0x%08x\n", hal_pcie_alloc_base_io);
	pcidebug_printf ("<SETUP_ATUE>Setting hal_pci_physical_io to: 0x%08x\n", hal_pcie_physical_io_base);

	cyg_pci_set_memory_base(hal_pcie_alloc_base_memory);
	cyg_pci_set_io_base(hal_pcie_alloc_base_io);

	// Scan and allocate memory/io space to any device on our PCI-X bus
	// Next bus is updated in case any bridges are encountered

	pcidebug_printf ("In Function: %s\n", __FUNCTION__);
	pcidebug_printf ("Configuring ATUE Bus.  Num: %d\n", localbus);

	cyg_pci_configure_bus(localbus, &next_bus);

	return(next_bus);
}



static void
pci_config_setup(cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset, cyg_bool pcie)
{
	cyg_uint32 dev = CYG_PCI_DEV_GET_DEV(devfn);
	cyg_uint32 fn  = CYG_PCI_DEV_GET_FN(devfn);
	cyg_uint8 localbus_pcie;
	cyg_uint8 localbus_pcix;
	cyg_uint8 pci_offset;
	volatile cyg_uint32 temp_value;

	if(atue_safe())
		localbus_pcie = iop34x_pcie_bus_number();
	
	if(atux_safe())
		localbus_pcix = iop34x_pcix_bus_number();
	
	/*
	*	Standard PCI Configuration Space Offset
	*	must be dword-aligned 8 bits only
	*/
    pci_offset = (cyg_uint8)(offset & 0xFC);

	if (pcie == true) {
		// Toggle Between PCIe and PCI-X memory windows based on which 
		// bus is being scanned
	    hal_pci_physical_memory_base = hal_pcie_physical_memory_base;
	    //cyg_pci_set_io_base(hal_pcie_alloc_base_io);

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
	    if (bus == localbus_pcie)
			*ATUE_OCCAR = ((bus << 24) | (dev << 19) | (fn << 16) | (offset) | 0);
    	else  {
			verbose_pcidebug_printf ("Creating Type1 on PCIe bus[%d] for bus[%d]\n", localbus_pcie, bus);
			*ATUE_OCCAR = ((bus << 24) | (dev << 19) | (fn << 16) | (offset) | 1);
    	}
	    HAL_READ_UINT32(ATUE_OCCAR, temp_value);
	}
	else  {
		// Toggle Between PCIe and PCI-X memory windows based on which 
		// bus is being scanned
	    hal_pci_physical_memory_base = hal_pcix_physical_memory_base;
	    //cyg_pci_set_io_base(hal_pci_alloc_base_io);
		/* PCI(-X) Type0 Config Cycle */
		/* [31:11] : Reserved
		   [10:08] : Function #
		   [07:02] : DW Number
		   [01:00] : 0
		*/
		if (bus == localbus_pcix) { 
			verbose_pcidebug_printf ("Creating Type0 on PCI-X bus[%d] for bus[%d]\n", localbus_pcix, bus);
	    	    *ATUX_OCCAR = ((1 << (dev + 16)) | (dev << 11) | (fn << 8) | pci_offset | 0 );
		}
    	else  {
	        *ATUX_OCCAR = ((bus << 16) | (dev << 11) | (fn << 8) | pci_offset | 1 );
			verbose_pcidebug_printf ("Creating Type1 on PCI-X bus[%d] for bus[%d]\n", localbus_pcix, bus);
    	}
		HAL_READ_UINT32(ATUX_OCCAR, temp_value);
	}

    /* Immediate I'llI'llbus use type 0 config, all others use type 1 config */
#ifdef VERBOSE_DEBUG
	if (pcie == true) 
	    diag_printf("PCIe config: localbus[%d] bus[%d] dev[%d] fn[%d] offset[0x%x], pcie[%d]\n",
                	localbus_pcie, bus, dev, fn, offset, pcie);
    else
    	diag_printf("PCI-X config: localbus[%d] bus[%d] dev[%d] fn[%d] offset[0x%x], pcie[%d]\n",
                	localbus_pcix, bus, dev, fn, offset, pcie);
#endif
    orig_abort_vec = ((volatile cyg_uint32 *)0x20)[4];
    ((volatile unsigned *)0x20)[4] = (unsigned)__pci_abort_handler;

}

static int
pci_config_cleanup(cyg_uint32 bus, cyg_bool pcie)
{
    cyg_uint32 status = 0, err = 0;
	cyg_uint32 atu_reg;

	if (pcie == true)
		atu_reg = (cyg_uint32)ATUE_ATUISR;
	else
		atu_reg = (cyg_uint32)ATUX_ATUISR;		

    HAL_READ_UINT32(atu_reg, status);

    if ((status & 0x1F) != 0) {
		err = 1;
		HAL_WRITE_UINT32(atu_reg, status & 0x1F);
    }

    ((volatile unsigned *)0x20)[4] = orig_abort_vec;

    return err;
}


cyg_uint32
cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint32 config_data;
    int err;
	cyg_bool  pcie = false;

	pcie = requesting_pcie_bus(bus);

    pci_config_setup(bus, devfn, offset, pcie);
	
	if (pcie == true)
	    config_data = *ATUE_OCCDR;
	else
		config_data = *ATUX_OCCDR;

    err = pci_config_cleanup(bus, pcie);

    verbose_pcidebug_printf("config read dword: data[0x%x] err[%d]\n",
		config_data, err);
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
    int err;
	cyg_bool pcie;

	pcie = requesting_pcie_bus(bus);

    pci_config_setup(bus, devfn, offset, pcie);

	if (pcie == true) 
    	*ATUE_OCCDR = data;
	else
		*ATUX_OCCDR = data;

    err = pci_config_cleanup(bus, pcie);


    verbose_pcidebug_printf("config write dword: data[0x%x] err[%d]\n",
		data, err);
}


cyg_uint16
cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
	cyg_uint16 config_data;
	int err;
	cyg_bool pcie;
	
	pcie = requesting_pcie_bus(bus);
	
	pci_config_setup(bus, devfn, offset &~ 3, pcie);
	
	
	if (pcie == true)
	{
	   	config_data = (((*ATUE_OCCDR) >> ((offset % 0x4) * 8)) & 0xffff);
	}
	else
	{
    		config_data = (((*ATUX_OCCDR) >> ((offset % 0x4) * 8)) & 0xffff);
	}

	err = pci_config_cleanup(bus, pcie);

	// ATUe Type1 Config cycle bug that needs more investigation - cebruns
	if ((config_data == 0x8330) || (config_data == 0xffdc))
		err  = 1;

	verbose_pcidebug_printf("config read word: data[0x%x] err[%d]\n",config_data, err);
	
	if (err)
		return 0xffff;
	else
	{
		verbose_pcidebug_printf ("Found 0x%02x at: bus[%d] devfn[%d] offset[%d]\n", config_data, bus, devfn, offset);
		return config_data;
	}
}

void
cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint16 data)
{
	int err;
	cyg_uint32 mask, temp;
	cyg_bool pcie;
	
	pcie = requesting_pcie_bus(bus);
	
	pci_config_setup(bus, devfn, offset &~ 3, pcie);
	
	mask = ~(0x0000ffff << ((offset % 0x4) * 8));
	
	temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
	
	if (pcie == true)  
		*ATUE_OCCDR = (*ATUE_OCCDR & mask) | temp; 
	else
		*ATUX_OCCDR = (*ATUX_OCCDR & mask) | temp;
	
	err = pci_config_cleanup(bus, pcie);
	
	verbose_pcidebug_printf("config write word: data[0x%x] err[%d]\n", data, err);
}

cyg_uint8
cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
	int err;
	cyg_uint8 config_data;
	cyg_bool pcie;
	
	pcie = requesting_pcie_bus(bus);
	
	pci_config_setup(bus, devfn, offset &~ 3, pcie);
	
	if (pcie == true)  
		config_data = (cyg_uint8)(((*ATUE_OCCDR) >> ((offset % 0x4) * 8)) & 0xff);
	else
		config_data = (cyg_uint8)(((*ATUX_OCCDR) >> ((offset % 0x4) * 8)) & 0xff);
	
	err = pci_config_cleanup(bus, pcie);

	verbose_pcidebug_printf("config read byte: data[0x%x] err[%d]\n",config_data, err);
	
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
    int err;
    cyg_uint32 mask, temp;
	cyg_bool pcie;

	pcie = requesting_pcie_bus(bus);

    pci_config_setup(bus, devfn, offset &~ 3, pcie);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));

	if (pcie == true) 
    	*ATUE_OCCDR = (*ATUE_OCCDR & mask) | temp; 
	else
    	*ATUX_OCCDR = (*ATUX_OCCDR & mask) | temp; 

    err = pci_config_cleanup(bus, pcie);

    verbose_pcidebug_printf("config write byte: data[0x%x] err[%d]\n",data, err);
}

// Function which checks which bus# we are looking for
// and determines if that is on the PCIe leg or PCI-X
// leg.  The return is TRUE if the bus is on the PCIe
// leg and FALSE if the bus is on the PCI-X leg.
cyg_bool requesting_pcie_bus ( cyg_uint32 bus ) {
	cyg_uint8 localbus_pcix;
	cyg_uint8 localbus_pcie;
	cyg_bool pcie = false;

	if(atux_safe())
		localbus_pcix = iop34x_pcix_bus_number();
	else
		return true;
	
	if(atue_safe())
		localbus_pcie = iop34x_pcie_bus_number();
	else
		return false;
	


	// If we are an endpoint on the PCIe leg, we cannot request
	// config cycles out, so we will return false.
	if (*ATUE_PCSR & PCSR_ATUE_END_POINT_MODE) {
		return false;
	}
	// else if we're a PCI-X endpoint, we can't do config out, so 
	// we must be trying to configure PCIe leg - return true
	else if ((*ATUX_PCSR & PCSR_ATUX_CENTRAL_RESOURCE_MODE) == 0) { /*((localbus_pcix == bus) && \ */
		return true;
	}
/*	else  {
		diag_printf ("Err: Cannot be endpoint on BOTH PCIe and PCI-X Leg!\n");
		PCI_PARAM_ERROR(4, 7);
	}
*/

	// At this point, we must be a master on both busses.		
	// If requested bus is equal to one of our bus numbers, we
	// know which ATU to use for the outbound config cycle.
	// But, if the bus number is not equal then we need to figure
	// out which ATU the config cycle is trying to do the Type1
	// config out of.
	if (localbus_pcix > localbus_pcie) {
		if (bus >= localbus_pcix)
			pcie = false;
		else
			pcie = true; 
	}
	else if (localbus_pcie > localbus_pcix) {
		if (bus >= localbus_pcie)
			pcie = true;
		else
			pcie = false;
	}
	else   {/* Busses are equal - should not happen */
		diag_printf ("ERR: localbus_pcie == localbus_pcix\n");
		diag_printf ("Local_PCIe: %d, Local_PCIx: %d\n", localbus_pcie, localbus_pcix);
		PCI_PARAM_ERROR(4,6);
	}
	return (pcie);
}


// Modify_omw_pt : Modifies the Outbound Memory Page tables so the
//                 lower 32-bits of the outbound address line up with
//                 where we are allocating PCI memory addresses.

void modify_omw_pt(cyg_uint64 hal_pci_alloc_base_memory, cyg_uint32 outbound_mem_window) {
	cyg_uint32 ttb_base_va, ttb_base_pa, omw_offset, counter, count_max, new_entry;
	cyg_uint32 *omw_pt_entry;
	ttb_base_pa = (_Read_Translation_Table_Base & 0xFFFFC000);
	ttb_base_va = CYGARC_VIRTUAL_ADDRESS (ttb_base_pa);
	omw_offset = ((outbound_mem_window >> 20)*4);   // Outbound mem base and 4bytes per entry
	omw_pt_entry = (cyg_uint32*)(ttb_base_va + omw_offset);  // Pointer to the PT entries	

#ifdef DEBUG_ATU
	diag_printf ("OMW is: 0x%08x \n", outbound_mem_window);
	diag_printf ("TTB_Base_PA is: 0x%08x\n", ttb_base_pa);
	diag_printf ("TTB_Base_VA is: 0x%08x\n", ttb_base_va);
	diag_printf ("omw_offset is: 0x%08x and 1st omw_pt_entry is: 0x%08x\n", \
		omw_offset, *omw_pt_entry);
#endif

	// Calculate # of PT entries to modify
	count_max = (PCI_OUTBOUND_MEM_WINDOW_0_TOP - PCI_OUTBOUND_MEM_WINDOW_0) >> 20;
	new_entry = (hal_pci_alloc_base_memory & SUPERSECTION_BASE_ADDRESS);
	for (counter = 1; counter <= count_max; counter++ ) {
		*omw_pt_entry &= ~(SUPERSECTION_BASE_ADDRESS);
		*omw_pt_entry |= new_entry;
#ifdef DEBUG_ATU
		diag_printf ("Modifed PT entry being written: 0x%08x\n", *omw_pt_entry);	
#endif
		omw_pt_entry++;
		if ((counter % 16) == 0) {  // Every Super Entry takes 16 descriptors
			new_entry += 0x1000000;
		}  
	} 
	HAL_DCACHE_SYNC();			  // Sync cache w/ Memory
	HAL_DCACHE_INVALIDATE_ALL();  // Also clears TLBs
	HAL_L2_CACHE_SYNC();
}
#endif // CYGPKG_IO_PCI


#ifdef CYGSEM_HAL_BUILD_MINIMAL_REDBOOT
#include <redboot.h>
// If we are not building a full RedBoot setup a function
// that will do a minimal PCI initialization configuration here.
void cyg_hal_plf_pci_minimal_initialization(void)
{
    static int pci_minimally_initialized = 0;

    if (pci_minimally_initialized == 0)
    {
        pci_minimally_initialized = 1;

#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using previous IOP ATU setup
        // Clear any outstanding ATU Interrupts
        *ATU_ATUSR = 0xFFFF;

        // enable outbound ATU
        *ATU_ATUCR |= ATUCR_OUTBOUND_ATU_ENABLE;

        // Enable the ATU as a bus master and memory space
        *ATU_ATUCMD |= (ATUCMD_BUS_MASTER_ENABLE | ATUCMD_MEM_SPACE_ENABLE);

#else  // Using IOP gen 4 (dual ATU setup - determine which one)

		// ATU-X Initialization
		// Clear any PCI-X Errors
		*ATUX_ATUSR = 0xFFFF;	
        // enable outbound ATU-X
		*ATUX_ATUCR |= ATUCR_OUTBOUND_ATU_ENABLE;
        // Enable the ATU-X as a bus master and enable memory space
		*ATUX_ATUCMD |= (ATUCMD_BUS_MASTER_ENABLE | ATUCMD_MEM_SPACE_ENABLE);


		// ATU-E Initialization
		// Clear any PCIe Errors
		*ATUE_ATUSR = 0xFFFF;	
        // enable outbound ATUe
		*ATUE_ATUCR |= ATUCR_OUTBOUND_ATU_ENABLE;
        // Enable the ATUe as a bus master and enable memory space
		*ATUE_ATUCMD |= (ATUCMD_BUS_MASTER_ENABLE | ATUCMD_MEM_SPACE_ENABLE);
#endif
    }
}



#endif

inline cyg_uint8 iop34x_pcie_bus_number(void) {
    cyg_uint8 localbus;
    
    // Localbus for PCIe can be assigned by IOP or by host.  Check
	// the bus# field of PCSR
	localbus = ((*ATUE_PCSR & PCSR_ATUE_BUS_NUM_MASK) >> 24);

    return localbus;
}

inline cyg_uint8 iop34x_pcix_bus_number(void)
{
    cyg_uint8 localbus;
	// If we're Central Resource, we will start bus numbering at 0
	// on PCI(-X) leg
	if ((*ATUX_PCSR & PCSR_ATUX_CENTRAL_RESOURCE_MODE) == 0)
		localbus = 0;
	else if ((*ATUX_PCSR & PCSR_PCIX_CAPABILITY_MASK) == PCSR_PCI_MODE) {
    	// In PCI mode on IOP34X we have no reliable way of
        // determining our Bus Number.
        localbus = 0;
    } 
    else {
	    localbus = ATU_PX_SR_BUS_NUM(*ATUX_PCIX_SR);
        if (localbus == 0xff)
            localbus = 0;
    }
    return localbus;
}

// Set_pci_phys_base_ram : Required for RedBoot RAM builds to inform the GigE
// device driver where to access his PCI Memory Mapped MMRs in the outbound
// window.  
static void set_pci_phys_base_ram(void) {
	cyg_uint32 ttb_base_pa, ttb_base_va, omw_offset, counter, count_max, new_entry;
	cyg_uint32 *omw_pt_entry_p, pt_entry, count;

	ttb_base_pa = (_Read_Translation_Table_Base & 0xFFFFC000);
	ttb_base_va = CYGARC_VIRTUAL_ADDRESS (ttb_base_pa);

	// First determine the PCIx Physical Memory Base
	for (count = 0; count < 2; count++) { 
		// Outbound mem base and 4bytes per entry
		omw_offset = (count) ? ((PCIE_OUTBOUND_MEM_WINDOW >> 20)*4) : \
		                       ((PCI_OUTBOUND_MEM_WINDOW >> 20)*4);   
		omw_pt_entry_p = (cyg_uint32*)(ttb_base_va + omw_offset);  // Pointer to the PT entries	
		// Pull first entry for outbound window
		pt_entry = *omw_pt_entry_p;
#ifdef DEBUG_ATU
		diag_printf ("First outbound PT_Entry is: 0x%08x\n", pt_entry);
#endif
		// Clear off all but base address
		pt_entry &= SUPERSECTION_BASE_ADDRESS;
#ifdef DEBUG_ATU
		diag_printf ("First outbound PT_Entry is cleared to : 0x%08x\n", pt_entry);
#endif
		// This is our base address that should be used to calculate where the device driver can
		// access its BARs
		if (count == 0) { 
			hal_pci_physical_memory_base = ((cyg_uint64)*ATUX_OUMWTVR0 << 32) | (cyg_uint64)(PCI_OUTBOUND_MEM_WINDOW_0 - pt_entry);
			hal_pcix_physical_memory_base = hal_pci_physical_memory_base ;
		}
		else  {
			hal_pci_physical_memory_base = ((cyg_uint64)*ATUE_OUMWTVR0 << 32) | (cyg_uint64)(PCIE_OUTBOUND_MEM_WINDOW - pt_entry);
			hal_pcie_physical_memory_base = hal_pci_physical_memory_base ;
		}
			
#ifdef DEBUG_ATU
		diag_printf ("Setting pci_phys_memory_base to: 0x%08x\n", hal_pci_physical_memory_base);
#endif
	    hal_pci_physical_io_base     = PCI_OUTBOUND_IO_WINDOW - *ATUX_OIOWTVR;
	}
}
