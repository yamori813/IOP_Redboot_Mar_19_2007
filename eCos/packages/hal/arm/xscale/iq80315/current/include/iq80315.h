#ifndef CYGONCE_HAL_ARM_XSCALE_IQ80315_IQ80315_H
#define CYGONCE_HAL_ARM_XSCALE_IQ80315_IQ80315_H

/*=============================================================================
//
//      iq80315.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter, dkranak, cebruns
// Date:         2001-12-03
// Purpose:      Intel IQ80315 platform specific support routines
// Description:
// Usage:        #include <cyg/hal/iq80315.h>
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
//#include <cyg/hal/hal_ioc80314.h>  // IO Processor defines
//#include <pkgconf/hal_arm_xscale_iq80315.h>

// Defines for 31244/IDE CF Device tests
#define SECTOR    			512
#define SUPDSSCR  			8
#define SUISR     			7
#define SUIDHR    			6
#define SUICHR				5
#define SUICLR				4
#define SUISNR				3
#define SUIDCR 	  			2
#define SUISCR				2
#define DEVICES_PER_31244 	4
#define SECTOR_SIZE 512
#define IDENTIFY_DEVICE		0xEC
#define FLUSH_CACHE			0xE7
#define READ_SECTOR			0x20

//----   80314 Device Address Base Vaules
#define HAL_IQ80315_HLP_BASE       HAL_80314_HLP_BASE     +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_PCI1_BASE      HAL_80314_PCI1_BASE    +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_PCI2_BASE      HAL_80314_PCI2_BASE    +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_CIU_BASE       HAL_80314_CIU_BASE     +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_SDRAM_BASE     HAL_80314_SDRAM_BASE   +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_DMA_BASE       HAL_80314_DMA_BASE     +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_GIGE_BASE      HAL_80314_GIGE_BASE    +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_I2C_BASE       HAL_80314_I2C_BASE     +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_MPIC_BASE      HAL_80314_MPIC_BASE    +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_UART1_BASE     HAL_80314_UART1_BASE   +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_GPIO_BASE      HAL_80314_GPIO_BASE    +  HAL_IOC80314_CSR_BASE
#define HAL_IQ80315_UART2_BASE     HAL_80314_UART2_BASE   +  HAL_IOC80314_CSR_BASE

/* CIU Registers */
#define	CIU_ECC_STATUS			HAL_IQ80315_CIU_BASE + 0x3c
#define	CIU_CFG					HAL_IQ80315_CIU_BASE + 0x38
#define CIU_ECC_ERROR_AD		HAL_IQ80315_CIU_BASE + 0x40
#define CIU_ECC_ENABLE			(1<<1)
#define CIU_PARITY_ENABLE		(1<<2)
#define CIU_ECC_ERROR_MASK		(1<<1)


/* Defines to go into iq80315.h */
#define SDRAM_SPD_MODULE_ROWS	5
#define SDRAM_SPD_ROW_DENSITY	31
#define	SDRAM_I2C_CNTRL1	(HAL_IQ80315_SDRAM_BASE 	+ 0x400)
#define	SDRAM_I2C_CNTRL2	(HAL_IQ80315_SDRAM_BASE 	+ 0x404)
#define SDRAM_I2C_RD_DATA	(HAL_IQ80315_SDRAM_BASE 	+ 0x408)
#define SDRAM_I2C_WR_DATA 	(HAL_IQ80315_SDRAM_BASE 	+ 0x40C)
#define I2C_CNTRL1			(HAL_IQ80315_I2C_BASE 	+ 0x0)
#define	I2C_CNTRL2			(HAL_IQ80315_I2C_BASE	+ 0x4)
#define	I2C_RD_DATA			(HAL_IQ80315_I2C_BASE	+ 0x8)
#define I2C_WR_DATA			(HAL_IQ80315_I2C_BASE	+ 0xC)
#define	I2C_READ			0x00000000
#define	I2C_WRITE			0x01000000
#define	I2C_DDR0_SPD		0x00000000
#define	I2C_DDR1_SPD		0x00000100
#define	I2C_DDR2_SPD		0x00000200
#define	I2C_DDR_SPD_DEVICE	0x0000000A
#define I2C_START			0x00000100
#define I2C_BYTE0			(0  << 16)
#define I2C_BYTE4			(4  << 16)
#define I2C_BYTE5			(5  << 16)
#define I2C_BYTE11			(11 << 16)
#define I2C_BYTE12          (12 << 16)
#define	I2C_BYTE13			(13 << 16)
#define I2C_BYTE14			(14 << 16)
#define I2C_BYTE16			(16 << 16)
#define I2C_BYTE17			(17 << 16)
#define	I2C_BYTE18			(18 << 16)
#define I2C_BYTE21			(21 << 16)
#define I2C_BYTE27			(27 << 16)
#define I2C_BYTE29			(29 << 16)
#define I2C_BYTE30			(30 << 16)
#define I2C_BYTE31			(31 << 16)
#define I2C_BYTE42			(42 << 16)
/* SDRAM Register Defines for SPD Search */
#define SD_REFRESH 				HAL_IQ80315_SDRAM_BASE + 0x0
#define	SD_CNTRL				HAL_IQ80315_SDRAM_BASE + 0x4
#define SD_BANK_CNTRL			HAL_IQ80315_SDRAM_BASE + 0x8
#define	SD_BANK0_ADDR			HAL_IQ80315_SDRAM_BASE + 0x10
#define	SD_BANK1_ADDR			HAL_IQ80315_SDRAM_BASE + 0x20
#define SD_BANK2_ADDR			HAL_IQ80315_SDRAM_BASE + 0x30
#define	SD_BANK0_MASK			HAL_IQ80315_SDRAM_BASE + 0x14
#define	SD_BANK1_MASK			HAL_IQ80315_SDRAM_BASE + 0x24
#define SD_BANK2_MASK			HAL_IQ80315_SDRAM_BASE + 0x34
#define	SD_ECC_ADDR1			HAL_IQ80315_SDRAM_BASE + 0x50
#define	SD_ECC_STATUS			HAL_IQ80315_SDRAM_BASE + 0x58
#define SD_ECC_ERROR_MASK		((0xFF << 16) | (1<<27))
#define SD_ECC_UERR				0x1000000
#define SD_ANY_ECC_ERROR		0x9ff0000
#define	SD_ECC_ENABLE			(1<<26)
#define	REFRESH_FOR_15_62us		0x0000061a
#define	REFRESH_FOR_3_9us		0x00000186
#define REFRESH_FOR_7_8us		0x0000030c
#define REFRESH_FOR_31_3us		0x00000C3a
#define	SDRAM_CAS_25			0x30000000
#define	SDRAM_CAS_15			0x28000000
#define	DDR_EN					0x04000000
#define SPD_RFR_15_62us			0x00000080
#define	SPD_RFR_3_9us			0x00000081
#define	SPD_RFR_7_8us			0x00000082
#define	SPD_RFR_31_3us			0x00000083
#define	SPD_CAS_25				0x00000004
#define SPD_CAS_15				0x00000002
#define SPD_DENSITY_1G			(1 << 0)
#define SPD_DENSITY_2G			(1 << 1)
#define SPD_DENSITY_16M			(1 << 2)
#define SPD_DENSITY_32M			(1 << 3)
#define SPD_DENSITY_64M			(1 << 4)
#define SPD_DENSITY_128M		(1 << 5)
#define SPD_DENSITY_256M		(1 << 6)
#define SPD_DENSITY_512M		(1 << 7)
#define	SPD_BURST_LENGTH1		0x00000001
#define	SPD_BURST_LENGTH2		0x00000002
#define	SPD_BURST_LENGTH4		0x00000004
#define	SPD_BURST_LENGTH8		0x00000008
#define SPD_X4					0x4
#define	SPD_X8					0x8
#define	SPD_X16					0x10
#define SPD_ECC					0x2
#define SPD_BUFFERED			0x1
#define SPD_REGISTERED			0x2
#define SPD_PHY_BANKS1			0x1
#define SPD_PHY_BANKS2			0x2
#define	SPD_LOG_BANKS2			0x2
#define	SPD_LOG_BANKS4			0x4
#define SPD_A_MODE0				0x8
#define SPD_A_MODE1				0x9
#define SPD_A_MODE2				0xa
#define SPD_A_MODE3				0xb
#define SPD_A_MODE4				0xc
#define SPD_A_MODE5				0xd
#define SPD_A_MODE6				0xe
#define SDRAM_CNTRL_ENABLE		(1 << 31)
#define SDRAM_CNTRL_WAKE 		(1 << 30)
#define SDRAM_DIMM_ENABLE		(1 << 20)
#define SDRAM_DENSITY_2G		0x80000000
#define	SDRAM_MASK_2G			0x000F8000
#define SDRAM_DENSITY_1G		0x40000000
#define	SDRAM_MASK_1G			0x000FC000
#define SDRAM_DENSITY_512M		0x20000000
#define	SDRAM_MASK_512M			0x000FE000
#define SDRAM_DENSITY_256M		0x10000000
#define SDRAM_MASK_256M			0x000FF000
#define SDRAM_DENSITY_128M		0x08000000
#define SDRAM_MASK_128M			0x000FF800
#define SDRAM_DENSITY_64M		0x04000000
#define SDRAM_MASK_64M			0x000FFC00
#define SDRAM_BANKS_OPEN		0x0
#define SDRAM_TWR_CLOCKS_2		(2 << 5)
#define SDRAM_A_MODE0			(0 << 24)
#define SDRAM_A_MODE1			(1 << 24)
#define SDRAM_A_MODE2			(2 << 24)
#define SDRAM_A_MODE3			(3 << 24)
#define SDRAM_A_MODE4			(4 << 24)
#define SDRAM_A_MODE5			(5 << 24)
#define SDRAM_A_MODE6			(6 << 24)
#define	SDRAM_BUFFERED			(1 << 29)
#define SDRAM_UNBUFFERED		(0 << 29)
#define	SDRAM_PHY_BANKS1		(0 << 28)
#define SDRAM_PHY_BANKS2		(1 << 28)
#define	SDRAM_LOG_BANKS2		(0 << 27)
#define SDRAM_LOG_BANKS4		(1 << 27)
#define	SDRAM_BURST_LENGTH2		0x00040000
#define	SDRAM_BURST_LENGTH4		0x00080000
#define	SDRAM_X4				0x00000000
#define	SDRAM_X8				0x00408000
#define SDRAM_X8_DQM_EN			0x00408000
#define	SDRAM_X16				0x00408000
#define SDRAM_X16_DQM_EN		0x00408000
#define	ECC_X4					0x00000000
#define	ECC_X8					0x00004000
#define	ECC_X16					0x00004000
#define SDRAM_PORT1_PRIORITY	(0 << 8)
#define	SDRAM_PORT2_PRIORITY	(1 << 8)
#define	SDRAM_PORT1_80pct		(2 << 8)
#define	SDRAM_BURST_EXCHANGE	(3 << 8)
#define	SDRAM_APB_09			0x00000000
#define	SDRAM_APB_10			0x00100000
#define	SDRAM_APB_11			0x00200000
#define SDRAM_APB_12			0x00300000
#define	SDRAM_DQM_EN			0x00400000
/* END SDRAM Register Defines */
/* End of defines to go into iq80315.h */


#define  CSR_ADDR(reg_comp,reg_name)    (HAL_IQ80315_CSR + reg_comp##BASE   + red_comp##reg_name)
#define  DMA_REG_ADDR(chanel, reg)      (HAL_IQ80315_DMA_BASE + (0x100 * chanel) + CH_##reg)
#define  DMA_CHN_ADDR(chanel)           (HAL_IQ80315_DMA_BASE + (0x100 * chanel))

//----   80200 Address  Map for IQ80315 board
#define SDRAM_PHYS_BASE     0x00000000
#define SDRAM_BASE          0x00000000
#define SDRAM_WINDOW_BASE   (SDRAM_BASE + SDRAM_SIZE) // locate window at 512M
#define SDRAM_BASE_UnCUnB   0x60000000  // Changed from 0x20000000 for memory windowing
#define SRAM_BASE           0x50000000
#define SRAM_BASE__UnCUnB   0x50200000
//  SRAM which has 1Meg size

#define SDRAM_SIZE          0x20000000  // 512MB
#define SDRAM_MAX           0x40000000  // 1GigB
#define IQ80315_SRAM_SIZE   0x00100000  // 1Meg     for Standard parts
#define SRAM_MAX            0x00100000  // 1MB

#define OCN_PORT_SDRAM		0x4
#define IQ80315_BAR1_SIZE   ((0x68 + TS_CIU_SF_PASSx_LUT_INDEX + TS_CIU_CFG_BAR_EN)  << 24)
//#define IQ80315_BAR1_SIZE   ((0x68 + TS_CIU_SF_PASSx_PORT_DEST + TS_CIU_CFG_BAR_EN)  << 24)
#define IQ80315_BAR2_SIZE   ((0x78 + TS_CIU_SF_PASSx_LUT_INDEX)<< 16)
#define IQ80315_BAR3_SIZE   ((0x78 + TS_CIU_SF_PASSx_LUT_INDEX)<<  8)
#define IQ80315_BAR4_SIZE   (0x70 + TS_CIU_CFG_BAR_EN)  // 512M Window

#define IQ80315_PCI1_ADDR  HAL_IOC80314_CIU_PCI1_BASE
#define IQ80315_PCI2_ADDR  HAL_IOC80314_CIU_PCI2_BASE

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_UNCACHED_BASE    0x60000000   // Changed from 0x2000.000 to support memory windowing
#define DCACHE_FLUSH_AREA      0x51000000  // Needed in hal_cache.h for sync flash
#define DCACHE_FLUSH_BASE      0x51000000  // Same as  DCACHE_FLUSH_AREA
#define SDRAM_MINI_DCACHE_BASE 0x52000000
#define MINI_DCACHE_FLUSH_BASE 0x53000000
#define DCACHE_SIZE           (32 * 1024)

// ------------------------------------------------------------------------
// SDRAM configuration

#define IQ80315_FLASH_BASE          (0)           //
#define IQ80315_FLASH_MASK          0xFFE00000                             //32Meg reserved for FLASH
#define IQ80315_FLASH_CTRL0         0x56FC44C0                             //Write Disabled
#define IQ80315_FLASH_CTRL1         0xFC002000                             //
#define IQ80315_FPGA_BASE           (0x0E600000>>4)                        //FPGA memory base address
#define IQ80315_FPGA_MASK           0xFFFF0000                             //1Meg reserved for FPGA Memory
#define IQ80315_FPGA_CTRL0          0x56FC44C0                             //Write enabled
#define IQ80315_FPGA_CTRL1          0xFC002000                             //
#define IQ80315_RESERVED_BASE       (0x0E700000>>4)                        //RESERVED memory base address
#define IQ80315_RESERVED_MASK       0xFFFF0000                             //1Meg reserved for RESERVED space
#define IQ80315_RESERVED_CTRL0      0x56FC44C0                             //Write enabled
#define IQ80315_RESERVED_CTRL1      0xFC002000                             //
#define IQ80315_IOREG_BASE          (0x0E800000>>4)                        //FPGA memory base address
#define IQ80315_CF_BASE				0x0E600000							   //Compact Flash Base
#define IQ80315_IOREG_MASK          0xFFFF0000                             //1Meg reserved for FPGA Memory
#define IQ80315_IOREG_CTRL0         0x56FC44C0                             //Write enabled
#define IQ80315_IOREG_CTRL1         0xFC002000                             //
#define IQ80315_PRODUCT_CODE_ADDR   (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x00000000)      //
#define IQ80315_BOARD_REVISION_ADDR (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x00010000)      //
#define IQ80315_FPGA_REVISION_ADDR  (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x00020000)      //
#define IQ80315_DISPLAY_RIGHT_ADDR  (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x00050000)      //
#define IQ80315_DISPLAY_LEFT_ADDR   (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x00040000)      //
#define IQ80315_ROTARY_SWITCH_ADDR  (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x000D0000)      //
#define IQ80315_ESERIAL_NUMBER_ADDR (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x000E0000)      //
#define IQ80315_BATTERY_STATUS_ADDR (HAL_IOC80314_FLASH_BASE + (IQ80315_IOREG_BASE<<4) + 0x000F0000)      // Does not really exist.
#define IQ80315_COMPACT_FLASH_ADDR  (HAL_IOC80314_FLASH_BASE + (IQ80315_CF_BASE))
// SDRAM configuration
// ------------------------------------------------------------------------


// Which SFN speed and other options based on whether board has been reworked for Clock Sync and the flavor of board
#if defined(CYGPKG_HAL_ARM_XSCALE_IQ80315_BOARD_FLAVOR_IQ80315)
	#define BAUD_RATE_TABELE_INDEX ((*((cyg_uint8*)IQ80315_BOARD_REVISION_ADDR) & 0x0F) == CLOCK_SYNC_CHECK)
	#define _TICKS_INDEX           ((*((cyg_uint8*)IQ80315_BOARD_REVISION_ADDR) & 0x0F) == CLOCK_SYNC_CHECK) 
	// IQ80315 Setting: 0x01930800, IQ80315/HBA: 0x01930810 - Set bus=8, device=1, funcion=0
	#define PCI_PCI1_PCIX_STAT   ((((*(cyg_uint8*)IQ80315_PRODUCT_CODE_ADDR) & 0xF0) == 0x10) ? 0x01930800 : 0x01930810)
	// IQ80315 Setting:0x01930900
	#define PCI_PCI2_PCIX_STAT           0x01930900          // Set bus=9, device=1, funcion=0
	#define HAL_PLATFORM_BOARD  "IQ80315"
	#define CLOCK_SYNC_CHECK    0x01
	#define HAL_PLATFORM_STEPPING strcat(__get_stepping_info(), HAL_PLATFORM_SPEED)
	#define HAL_PLATFORM_SPEED  ((_TICKS_INDEX == 0x1) ? " 100MHz SFN" : " 133MHz SFN")
	#define HAL_PLATFORM_EXTRA HAL_PLATFORM_STEPPING
	#define SF_PCI2_P2S_PAGE_SIZES       0x00008181          // PCII BAR2 BAR3 page size and enable  size=2GigB
	#define SF_PCI1_P2S_PAGE_SIZES       0x00008181          // PCII BAR2 BAR3 page size and enable  size=2GigB

#elif defined (CYGPKG_HAL_ARM_XSCALE_IQ80315_BOARD_FLAVOR_XCARD)
	#define BAUD_RATE_TABELE_INDEX 0x0	// 0 = 133MHz, 1=100MHz SFN Frequency  (no way to verify on XCard)
	#define _TICKS_INDEX           0x0  // 0 = 133MHz, 1=100MHz SFN Frequency  (no way to verify on XCard)
	#define PCI_PCI1_PCIX_STAT     0x01930800
	#define PCI_PCI2_PCIX_STAT     0x01930900
	#define HAL_PLATFORM_BOARD     "XCARD"
	#define HAL_PLATFORM_STEPPING  __get_stepping_info()
	#define HAL_PLATFORM_EXTRA     HAL_PLATFORM_STEPPING
	#define SF_PCI2_P2S_PAGE_SIZES 0x00000101  // Sizes for XCard set to 64k
	#define SF_PCI1_P2S_PAGE_SIZES 0x00000101  // Sizes for XCard set to 64k
#else
	error (YOU NEED TO DEFINE THE BOARD_FLAVOR in the cdl file)
#endif

/*----------   XScale PCI1 initiator (outbound) window to PCI1 bus   ----------*/
#define IOC80314_IS_HOST             true
#define XS_PCI1_BASE_ADDR            IQ80315_PCI1_ADDR   /* XScale Base Address for the PCI1 window */
#define XS_PCI1_SIZE                 0x40000000         /* XScale size of window allocated for the PCI1 bus*/
#define PCI1_CFG_OFFSET              0x1F000000         /* PCI Config space offset within the PCI1 window */
#define PCI1_IO_OFFSET               0x1EFF0000         /* PCI I/O space offset within the PCI1 wondow */
#define PCI1_MEM32_OFFSET            0x00000000          /* PCI MEM32 space offset within the PCI1 window */
#define PCI1_MEM32_OFFSET_UPPER      0x00000000         /* PCI MEM32 space offset within the PCI1 window */
#define PCI1_PFM1_OFFSET             0x20000000         /* PCI PFM1 space offset within the PCI1 window */
#define PCI1_PFM1_OFFSET_UPPER       0x00000000         /* PCI PFM1 space offset within the PCI1 window */
#define PCI1_PFM2_OFFSET             0x30000000         /* PCI PFM2 space offset within the PCI1 window */
#define PCI1_PFM2_OFFSET_UPPER       0x00000000         /* PCI PFM2 space offset within the PCI1 window */
#define PCIX_STATUS_OFFSET			 0x000000F4

//----------       XScale Base Addresses for PCI address spaces on the PCI1 bus
#define XS_PCI1_MEM32_BASE  (XS_PCI1_BASE_ADDR + PCI1_MEM32_OFFSET)        /* PCI1 MEM32 Space Base Address */
#define XS_PCI1_MEM32_SIZE  0x1eff0000								 /* 495 Meg MEM32 Space */
#define XS_PCI1_CFG_BASE    (XS_PCI1_BASE_ADDR + PCI1_CFG_OFFSET)          /* PCI1 Configuration Space Base Address */
#define XS_PCI1_IO_BASE     (XS_PCI1_BASE_ADDR + PCI1_IO_OFFSET)           /* PCI1 I/O Space Base Address */
#define XS_PCI1_IO_SIZE		0x00100000								 /* 1 Meg I/O Space */
#define XS_PCI1_PFM1_BASE   (XS_PCI1_BASE_ADDR + PCI1_PFM1_OFFSET)         /* PCI1 PFM1 Space Base Address */
#define XS_PCI1_PFM1_SIZE	0x10000000								 /* 256 Meg PFM1 Space */
#define XS_PCI1_PFM2_BASE   (XS_PCI1_BASE_ADDR + PCI1_PFM2_OFFSET)         /* PCI1 PFM2 Space Base Address */
#define XS_PCI1_PFM2_SIZE	0x10000000								 /* 256 Meg PFM2 Space */

//----------      SFN Base Addresses for PCI1 block.
#define SF_PCI1_PBI_BASE_UPPER       0x00000000          /* PCI PBI Space window for the PCI2 functional block */
#define SF_PCI1_PBI_BASE             0x00000000
#define SF_PCI1_SRAM_BASE_UPPER      0x00000000          /* PCI SRAM Space window for the PCI2 functional block */
#define SF_PCI1_SRAM_BASE            0x00000000
#define SF_PCI1_SDRAM_BASE_UPPER     0x00000000          /* PCI SRAM Space window for the PCI2 functional block */
#define SF_PCI1_SDRAM_BASE           0x00000000
#define SF_PCI1_CONFIG_BASE_UPPER    0x00000000          /* PCI Configuration Space window for the PCI1 functional block */
#define SF_PCI1_CONFIG_BASE          0x1F000000
#define SF_PCI1_IO_BASE_UPPER        0x00000000          /* PCI I/O Space window for the PCI1 functional block */
#define SF_PCI1_IO_BASE              0x1EFF0000
#define SF_PCI1_PCI2_BASE_UPPER      0x00000000     /* PCI to PCI Space window for the PCI1 functional block */
#define SF_PCI1_PCI2_BASE            0x00000000
#define SF_PCI1_PCI2_CONFIG_BASE_UPPER 0x00000000     /* PCI1 to PCI2 config Space window */
#define SF_PCI1_PCI2_CONFIG_BASE       0x10000000
#define SF_PCI1_PFM1_BASE_UPPER      0x00000000          /* PCI PFM1 Space window for the PCI1 functional block */
#define SF_PCI1_PFM1_BASE            0x40000000
#define SF_PCI1_PFM2_BASE_UPPER      0x00000000          /* PCI PFM2 Space window for the PCI1 functional block */
#define SF_PCI1_PFM2_BASE            0x80000000

#define PCI_PCI1_CSR                 0xFBB80147          // See PCI1_MISC_CSR_INIT  for boot value
#define PCI_PCI1_MISC_CSR            0x00010327          // See PCI_PCI1_MISC_CSR_INIT for boot time value - was 0x10323
                                                         // Changed to 10327 for B0 (1k buffers)
#define PCI_PCI1_LTIMER				 (0x80 << 8)                // Changed to 0x80 (was defaulting to 0x40) for B0

#define PCI_PCI1_P2S_BAR0            0x20000004
#define PCI_PCI1_P2S_BAR0_UPPER      0x00000000
#define PCI_PCI1_P2S_BAR2            0x80000000
#define PCI_PCI1_P2S_BAR2_UPPER      0x00000000
#define PCI_PCI1_P2S_BAR3            0x80000000
#define PCI_PCI1_P2S_BAR3_UPPER      0x00000003

//----------  Remapped Adresses on the PCI1 bus.  This is how transactions issued by the CPU to the XS_PCI1_XXX_BASE appear on the PCI1 bus
#define BUS_PCI1_MEM32_BASE          0x00000000
#define BUS_PCI1_PFM1_BASE_U         0x00000001  /* 64-bit address upper part */
#define BUS_PCI1_PFM1_BASE_L         0x00000000  /* 64-bit address lower part */
#define BUS_PCI1_PFM2_BASE_U         0x00000002  /* 64-bit address upper part */
#define BUS_PCI1_PFM2_BASE_L         0x00000000  /* 64-bit address lower part */

//----------  PCI1 setup values used during boot is system is not a HOST
#define PCI1_CSR_INIT                0xFBB80157   // Clear errors enable writes
#define PCI1_MISC_CSR_INIT           0x00008002   // Enable VPD & BAR0

/*----------   XScale PCI2 initiator (outbound) window to PCI2 bus   ----------*/
#define XS_PCI2_BASE_ADDR            IQ80315_PCI2_ADDR   /* XScale Base Address for the PCI2 window */
#define XS_PCI2_SIZE                 0x40000000          /* Size of window allocated for the PCI2 bus*/
#define PCI2_CFG_OFFSET              0x1F000000          /* PCI Config space offset within the PCI2 window */
#define PCI2_IO_OFFSET               0x1EFF0000          /* PCI I/O space offset within the PCI2 wondow */
#define PCI2_MEM32_OFFSET            0x00000000          /* PCI MEM32 space offset within the PCI2 window */
#define PCI2_MEM32_OFFSET_UPPER      0x00000000          /* PCI MEM32 space offset within the PCI2 window */
#define PCI2_PFM1_OFFSET             0x20000000          /* PCI PFM1 space offset within the PCI2 window */
#define PCI2_PFM1_OFFSET_UPPER       0x00000000          /* PCI PFM1 space offset within the PCI2 window */
#define PCI2_PFM2_OFFSET             0x30000000          /* PCI PFM2 space offset within the PCI2 window */
#define PCI2_PFM2_OFFSET_UPPER       0x00000000          /* PCI PFM2 space offset within the PCI2 window */

//----------       XScale Base Addresses for PCI address spaces on the PCI2 bus
#define XS_PCI2_CFG_BASE    (XS_PCI2_BASE_ADDR + PCI2_CFG_OFFSET)    /* PCI2 Configuration Space Base Address */
#define XS_PCI2_IO_BASE     (XS_PCI2_BASE_ADDR + PCI2_IO_OFFSET)     /* PCI2 I/O Space Base Address */
#define XS_PCI2_IO_SIZE		0x00100000								 /* 1 Meg I/O Space */
#define XS_PCI2_MEM32_BASE  (XS_PCI2_BASE_ADDR + PCI2_MEM32_OFFSET)  /* PCI2 MEM32 Space Base Address */
#define XS_PCI2_MEM32_SIZE  0x1eff0000								 /* 495 Meg MEM32 Space */
#define XS_PCI2_PFM1_BASE   (XS_PCI2_BASE_ADDR + PCI2_PFM1_OFFSET)   /* PCI2 PFM1 Space Base Address */
#define XS_PCI2_PFM1_SIZE	0x10000000								 /* 256 Meg PFM1 Space */
#define XS_PCI2_PFM2_BASE   (XS_PCI2_BASE_ADDR + PCI2_PFM2_OFFSET)   /* PCI2 PFM2 Space Base Address */
#define XS_PCI2_PFM2_SIZE	0x10000000								 /* 256 Meg PFM2 Space */

//----------      SFN Base Addresses for PCI2 block.
#define SF_PCI2_MEM32_BASE_UPPER     0x00000000     /* PCI MEM32 Space window for the PCI2 functional block */
#define SF_PCI2_MEM32_BASE           0x00000000

#define SF_PCI2_PBI_BASE_UPPER       0x00000000     /* PCI PBI Space window for the PCI2 functional block */
#define SF_PCI2_PBI_BASE             0x00000000
#define SF_PCI2_SRAM_BASE_UPPER      0x00000000     /* PCI SRAM Space window for the PCI2 functional block */
#define SF_PCI2_SRAM_BASE            0x00000000
#define SF_PCI2_SDRAM_BASE_UPPER     0x00000000     /* PCI SRAM Space window for the PCI2 functional block */
#define SF_PCI2_SDRAM_BASE           0x10000000
#define SF_PCI2_CONFIG_BASE_UPPER    0x00000000     /* PCI Configuration Space window for the PCI2 functional block */
#define SF_PCI2_CONFIG_BASE          0x1F000000
#define SF_PCI2_IO_BASE_UPPER        0x00000000     /* PCI I/O Space window for the PCI2 functional block */
#define SF_PCI2_IO_BASE              0x1EFF0000
#define SF_PCI2_PCI1_BASE_UPPER      0x00000000     /* PCI to PCI Space window for the PCI2 functional block */
#define SF_PCI2_PCI1_BASE            0x00000000
#define SF_PCI2_PCI1_CONFIG_BASE_UPPER 0x00000000     /* PCI2 Config Space window for the PCI1 functional block */
#define SF_PCI2_PCI1_CONFIG_BASE       0x10000000
#define SF_PCI2_PFM1_BASE_UPPER      0x00000000     /* PCI PFM1 Space window for the PCI2 functional block */
#define SF_PCI2_PFM1_BASE            0x40000000
#define SF_PCI2_PFM2_BASE_UPPER      0x00000000     /* PCI PFM2 Space window for the PCI2 functional block */
#define SF_PCI2_PFM2_BASE            0x80000000

#define PCI_PCI2_CSR                 0xFBB80147
#define PCI_PCI2_MISC_CSR            0x00010327     // Changed to 10327 for B0 (1k buffers) - was 0x10323
#define PCI_PCI2_LTIMER				 (0x80 << 8)                // Changed to 0x80 (was defaulting to 0x40) for B0

#define PCI_PCI2_P2S_BAR0            0x20000004          // Access to control registers.
#define PCI_PCI2_P2S_BAR0_UPPER      0x00000000
#define PCI_PCI2_P2S_BAR2            0x80000000
#define PCI_PCI2_P2S_BAR2_UPPER      0x00000000
#define PCI_PCI2_P2S_BAR3            0x80000000
#define PCI_PCI2_P2S_BAR3_UPPER      0x00000003

//----------  Remapped Adresses on the PCI2 bus.  This is how transactions issued by the CPU to the XsCALE PCI2_XXX_BASE appear on the PCI2 bus
#define BUS_PCI2_MEM32_BASE          0x00000000
#define BUS_PCI2_PFM1_BASE_U         0x00000001  /* 64-bit address upper part */
#define BUS_PCI2_PFM1_BASE_L         0x00000000  /* 64-bit address lower part */
#define BUS_PCI2_PFM2_BASE_U         0x00000002  /* 64-bit address upper part */
#define BUS_PCI2_PFM2_BASE_L         0x00000000  /* 64-bit address lower part */

//#define PCI1_P2S_BAR2                0X80000000
//#define PCI1_P2S_BAR2_UPPER          0X00000000
//#define PCI1_P2S_BAR3                0X80000000
//#define PCI1_P2S_BAR3_UPPER          0X00000003
//----------- PCI PFAB vaules
#define PCI1_PFAB_BAR0               0X1F000001  // Configuration addresses
#define PCI1_PFAB_BAR0_UPPER         0X00000000
#define PCI1_PFAB_IO                 0X1EFF0001  // IO addresses
#define PCI1_PFAB_IO_UPPER           0X00000000
#define PCI1_PFAB_MEM32              0X00010000  // 512MegB memrory address
#define PCI1_PFAB_MEM32_REMAP        0X00000000
#define PCI1_PFAB_MEM32_MASK         0XE0000000
#define PCI1_PFAB_PFM3               0X40010000  // Prefetch 1 Memory 1GigB
#define PCI1_PFAB_PFM3_REMAP_LOWER   0X00000000
#define PCI1_PFAB_PFM3_REMAP_UPPER   0X00000001
#define PCI1_PFAB_PFM3_MASK          0XF0000FFF
#define PCI1_PFAB_PFM4               0X80010000  // Prefetch 1 Memory 1GigB
#define PCI1_PFAB_PFM4_REMAP_LOWER   0X00000000
#define PCI1_PFAB_PFM4_REMAP_UPPER   0X00000002
#define PCI1_PFAB_PFM4_MASK          0XF0000FFF

//---------- PCI BAR vaules when master
#define PCI2_P2S_BAR2                0X80000000
#define PCI2_P2S_BAR2_UPPER          0X00000000
#define PCI2_P2S_BAR3                0X80000000
#define PCI2_P2S_BAR3_UPPER          0X00000003
//----------- PCI PFAB vaules
#define PCI2_PFAB_BAR0               0X1F000001  // Configuration addresses
#define PCI2_PFAB_BAR0_UPPER         0X00000000
#define PCI2_PFAB_IO                 0X1EFF0001  // IO addresses
#define PCI2_PFAB_IO_UPPER           0X00000000
#define PCI2_PFAB_MEM32              0X00010000  // 512MegB memrory address
#define PCI2_PFAB_MEM32_REMAP        0X00000000
#define PCI2_PFAB_MEM32_MASK         0XE0000000
#define PCI2_PFAB_PFM3               0X40010000  // Prefetch 1 Memory 1GigB
#define PCI2_PFAB_PFM3_REMAP_LOWER   0X00000000
#define PCI2_PFAB_PFM3_REMAP_UPPER   0X00000001
#define PCI2_PFAB_PFM3_MASK          0XF0000FFF
#define PCI2_PFAB_PFM4               0X80010000  // Prefetch 1 Memory 1GigB
#define PCI2_PFAB_PFM4_REMAP_LOWER   0X00000000
#define PCI2_PFAB_PFM4_REMAP_UPPER   0X00000002
#define PCI2_PFAB_PFM4_MASK          0XF0000FFF

// Vaules in PCI config read/write funtions to determin which port, PCI1 or PCI2 to perform operation on.
#define PCI1_BUS        8
#define PCI2_BUS        9
#define PCI2_BUS_MAX   10

#define PCI_PCI_CLASS           0x008


/*----------         End of PCI memory mapping                       ----------*/

//   -------- Debug flags
//#define DEBUG_SERIAL_GETC               // save a copy of first and last rs323 error & data
//#define HAVE_LED_DISPLAY                // record serial errors on led displays


// I2C slave address to which the unit responds when in slave-receive mode
#define I2C_DEVID                    0x02
#define SDRAM_DEVID                  0xAE

// Timeout limit for SDRAM EEPROM to respond
#define I2C_TIMOUT                   0x1000000

// ------------------------------------------------------------------------
// Battery Status Register
//
#define IQ80315_BATTERY_PRESENT      0x01
#define IQ80315_BATTERY_CHARGED      0x02
#define IQ80315_BATTERY_ENABLED      0x04
#define IQ80315_BATTERY_DISCHARGE    0x08

// Address used for battery backup test
#define SDRAM_BATTERY_TEST_ADDR     (SDRAM_UNCACHED_BASE + 0x1000000)

// ------------------------------------------------------------------------
// 7 Segment Display
#define DISPLAY_LEFT                IQ80315_DISPLAY_LEFT_ADDR
#define DISPLAY_RIGHT               IQ80315_DISPLAY_RIGHT_ADDR

#define DISPLAY_0  0x7e            /* 0111111-0 */
#define DISPLAY_1  0x0c            /* 0000110-0 */
#define DISPLAY_2  0xb6            /* 1012011-0 */
#define DISPLAY_3  0x9e            /* 1001111-0 */
#define DISPLAY_4  0xcc            /* 1100110-0 */
#define DISPLAY_5  0xda            /* 1101101-0 */
#define DISPLAY_6  0xfa            /* 1111101-0 */
#define DISPLAY_7  0x0e            /* 0000111-0 */
#define DISPLAY_8  0xfe            /* 1111111-0 */
#define DISPLAY_9  0xce            /* 1100111-0 */
#define DISPLAY_A  0xee            /* 1110111-0 */
#define DISPLAY_B  0xf8            /* 1111100-0 */
#define DISPLAY_C  0x72            /* 0111001-0 */
#define DISPLAY_D  0xbc            /* 1011110-0 */
#define DISPLAY_E  0xf2            /* 1111001-0 */
#define DISPLAY_F  0xe2            /* 1110001-0 */

#define DISPLAY_G  0xFA            /* 1111101-0 */
#define DISPLAY_H  0x6C            /* 0110110-0 */
#define DISPLAY_I  0x60            /* 0110000-0 */
#define DISPLAY_J  0x1C            /* 0001110-0 */
#define DISPLAY_K  0x6C            /* 0110110-1 *//* cannot do a K, H with a decimal point */
#define DISPLAY_L  0x70            /* 0111000-0 */
#define DISPLAY_M  0x7e            /* 0111111-1 *//* Cannot do an M, overscore with the decimal point */
#define DISPLAY_N  0x03            /* 0000001-1 */
#define DISPLAY_O  0x7E            /* 0111111-0 */
#define DISPLAY_P  0xe6            /* 1110011-0 */
#define DISPLAY_Q  0x7F            /* 0111111-1 *//* same as an "0" with a decimal point           */
#define DISPLAY_R  0xEF            /* 1110111-1 *//* same as an "A", except with the decimal point */
#define DISPLAY_S  0xDB            /* 1101101-1 *//* same as a "5", except with the decimal point  */
#define DISPLAY_T  0x0F            /* 0000111-1 *//* same as a "7", except with the decimal point  */
#define DISPLAY_U  0x38            /* 0011100-0 */
#define DISPLAY_V  0x39            /* 0011100-1 *//* same as a "U", except with the decimal point */
#define DISPLAY_W  0x11            /* 0001000-1 *//* Cannot do an W, underscore with the decimal point */
#define DISPLAY_X  0xE7            /* 1110011-1 *//* cannot do an X, upside-down h, with decimal point */
#define DISPLAY_Y  0xCC            /* 1100110-0 */
#define DISPLAY_Z  0x93            /* 1001001-1 *//* cannot do a Z, dash/dash/dash with decimal point */

#define DISPLAY_OFF 0x00            /* 0000000-1 */
#define DISPLAY_ON  0xff            /* 1111111-1 */

#define DISPLAY_SPACE       0xff    /* 0000000-0 */
#define DISPLAY_ERROR       0xF2    /* 1111001-0 */
#define DISPLAY_UNDERSCORE  0x10    /* 0001000-0 */
#define DISPLAY_DASH        0x80    /* 1000000-0 */
#define DISPLAY_PERIOD      0x01    /* 0000000-1 */
#define DISPLAY_EXCLAMATION 0x61    /* 0110000-1 */

#ifdef __ASSEMBLER__
        // Display 'lvalue:rvalue' on the hex display
        // lvalue and rvalue must be of the form 'DISPLAY_x'
        // where 'x' is a hex digit from 0-F.
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue

	ldr		\reg0, =DISPLAY_LEFT		// display left digit
	ldr		\reg1, =\lvalue
	strb	\reg1, [\reg0]
	ldr		\reg0, =DISPLAY_RIGHT
	ldr		\reg1, =\rvalue			// display right digit
	strb	\reg1, [\reg0]

#if 0
	// delay
        ldr     \reg0, =0x1800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
#endif

	.endm

	.macro HEX_DISPLAY2 reg0, reg1, lvalue, rvalue

	ldr		\reg0, =DISPLAY_LEFT		// display left digit
	ldr		\reg1, =\lvalue
	strb	\reg1, [\reg0]
	ldr		\reg0, =DISPLAY_RIGHT
	ldr		\reg1, =\rvalue			// display right digit
	strb	\reg1, [\reg0]

#if 1
	// delay
        ldr     \reg0, =0x5800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
#endif

	.endm

	.macro REG_DISPLAY reg0, reg1, reg2
/*
	b	667f
   666:
	.byte	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3
	.byte	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7
	.byte	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B
	.byte	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
   667:
	ldr	\reg0, =666b
	add	\reg0, \reg0, \reg2, lsr #4
	ldrb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_LEFT
	str	\reg1, [\reg0]
	ldr	\reg0, =666b
	and     \reg2, \reg2, #0xf
	add	\reg0, \reg0, \reg2
	ldrb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	str	\reg1, [\reg0]

	// delay
        ldr     \reg0, =0x7800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
 */
	.endm
#else
static inline void HEX_DISPLAY(int lval, int rval)
{
  /*  int i; */
    static unsigned char hchars[] = {
	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3,
	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B,
	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
    };
    volatile unsigned int *ldisp = (volatile unsigned int *)DISPLAY_LEFT;
    volatile unsigned int *rdisp = (volatile unsigned int *)DISPLAY_RIGHT;

    *ldisp = hchars[lval & 0xf];
    *rdisp = hchars[rval & 0xf];

    //for (i = 0; i < 0x10000000; i++);

}

#endif // __ASSEMBLER__





#endif // CYGONCE_HAL_ARM_XSCALE_IQ80315_IQ80315_H









// EOF iq80315.h




