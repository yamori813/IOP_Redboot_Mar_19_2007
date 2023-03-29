/*=============================================================================
//
//      hal_iop34x.h
//
//      IOP34X I/O Coprocessor support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):      curt.e.bruns@intel.com
// Contributors:   drew.moseley@intel.com
// Date:           2004-12-10
// Purpose:
// Description:    IOP34X I/O Processor support.
// Usage:          #include <cyg/hal/hal_iop34x.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_IOP34X_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_IOP34X_H

#include <pkgconf/system.h>
#include <pkgconf/hal_arm_xscale_iop.h>
#include <cyg/hal/hal_xscale.h>
#include <cyg/hal/hal_xscale_iop.h>
#include <cyg/hal/hal_misc.h>

#define HAL_PCI_IGNORE_DEVICE(__bus,__dev,__fn)	cyg_hal_plf_ignore_devices(__bus,__dev,__fn)
#define CYGIMP_FLASH_ENABLE   get_fac_semaphores
#define CYGIMP_FLASH_DISABLE  release_fac_semaphores
// --------------------------------------------------------------------------
// IOP34X Processor Device ID
#define IOP34X_DEVICE_ID_A_0           0x69056810

// 
// IOP34X L2 Information
#define L2_CACHE_SIZE                  0x80000
#define MMU_Control_L2				   0x04000000

// --------------------------------------------------------------------------
// UART
#define IOP34X_UART0_ADDR       		0xffd82300
#define IOP34X_UART1_ADDR       		0xffd82340

#define UART0_RBR                       REG32(0,0xffd82300)
#define UART0_THR                       REG32(0,0xffd82300)
#define UART0_DLL                       REG32(0,0xffd82300)
#define UART0_IER                       REG32(0,0xffd82304)
#define UART0_DLH                       REG32(0,0xffd82304)
#define UART0_IIR                       REG32(0,0xffd82308)
#define UART0_FCR                       REG32(0,0xffd82308)
#define UART0_LCR                       REG32(0,0xffd8230c)
#define UART0_MCR                       REG32(0,0xffd82310)
#define UART0_LSR                       REG32(0,0xffd82314)
#define UART0_MSR                       REG32(0,0xffd82318)
#define UART0_SPR                       REG32(0,0xffd8231c)
#define UART0_FOR                       REG32(0,0xffd82324)
#define UART0_ABR                       REG32(0,0xffd82328)
#define UART0_ACR                       REG32(0,0xffd8232c)

#define UART1_RBR                       REG32(0,0xffd82340)
#define UART1_THR                       REG32(0,0xffd82340)
#define UART1_DLL                       REG32(0,0xffd82340)
#define UART1_IER                       REG32(0,0xffd82344)
#define UART1_DLH                       REG32(0,0xffd82344)
#define UART1_IIR                       REG32(0,0xffd82348)
#define UART1_FCR                       REG32(0,0xffd82348)
#define UART1_LCR                       REG32(0,0xffd8234c)
#define UART1_MCR                       REG32(0,0xffd82350)
#define UART1_LSR                       REG32(0,0xffd82354)
#define UART1_MSR                       REG32(0,0xffd82358)
#define UART1_SPR                       REG32(0,0xffd8235c)
#define UART1_FOR                       REG32(0,0xffd82364)
#define UART1_ABR                       REG32(0,0xffd82368)
#define UART1_ACR                       REG32(0,0xffd8236c)

// --------------------------------------------------------------------------
// Peripheral Bus Interface Unit
#define PBIU_PBCR                       REG32(0,0xffd81580)
#define PBIU_PBISR                      REG32(0,0xffd81584)
#define PBIU_PBAR0                      REG32(0,0xffd81588)
#define PBIU_PBLR0                      REG32(0,0xffd8158c)
#define PBIU_PBAR1                      REG32(0,0xffd81590)
#define PBIU_PBLR1                      REG32(0,0xffd81594)
#define PBIU_ESSTSR0					REG32(0,0xffd82188)

// PBCR Bit Definitions
#define PBCR_ENABLE                     0x1

// PBISR Bit Definitions
#define PBISR_OUT_OF_RANGE_ERROR        0x1

// PBAR Bit Definitions
#define PBAR_BUS_8                      (0 << 0)
#define PBAR_BUS_16                     (1 << 0)
#define PBAR_ADWAIT_4                   (0 << 2)
#define PBAR_ADWAIT_8                   (1 << 2)
#define PBAR_ADWAIT_12                  (2 << 2)
#define PBAR_ADWAIT_16                  (3 << 2)
#define PBAR_ADWAIT_20                  (7 << 2)
#define PBAR_RCWAIT_1                   (0 << 6)
#define PBAR_RCWAIT_4                   (1 << 6)
#define PBAR_RCWAIT_8                   (2 << 6)
#define PBAR_RCWAIT_12                  (3 << 6)
#define PBAR_RCWAIT_16                  (4 << 6)
#define PBAR_RCWAIT_20                  (7 << 6)
#define PBAR_DDWAIT_SYNC_ADWAIT         ((0 << 10) | (0 << 11))
#define PBAR_DDWAIT_4                   (2 << 9)
#define PBAR_DDWAIT_8                   (3 << 9)
#define PBAR_DDWAIT_12                  (4 << 9)
#define PBAR_DDWAIT_16                  (5 << 9)
#define PBAR_DDWAIT_20                  (7 << 9)
#define PBAR_BAR_MASK                   0xFFFFF000
#define PBAR_BUS_WIDTH_MASK             0x3


// PBLR Bit Definitions
#define PBLR_SZ_4K                      0xfffff000
#define PBLR_SZ_8K                      0xffffe000
#define PBLR_SZ_16K                     0xffffc000
#define PBLR_SZ_32K                     0xffff8000
#define PBLR_SZ_64K                     0xffff0000
#define PBLR_SZ_128K                    0xfffe0000
#define PBLR_SZ_256K                    0xfffc0000
#define PBLR_SZ_512K                    0xfff80000
#define PBLR_SZ_1M                      0xfff00000
#define PBLR_SZ_2M                      0xffe00000
#define PBLR_SZ_4M                      0xffc00000
#define PBLR_SZ_8M                      0xff800000
#define PBLR_SZ_16M                     0xff000000		
#define PBLR_SZ_32M                     0xfe000000
#define PBLR_SZ_DISABLE                 0x00000000

// PBDSCR Bit Definitions
#define PBDSCR_PULL_UP_DRVSTRN_MASK     0x0000003F
#define PBDSCR_PULL_DN_DRVSTRN_MASK     0x00000FC0
#define PBDSCR_PULL_UP_SLEW_MASK        0x0000F000
#define PBDSCR_PULL_DN_SLEW_MASK        0x000F0000
#define PBDSCR_PULL_UP_DRVSTRN(x)       (((x) << 0) & PBDSCR_PULL_DN_DRVSTRN_MASK)
#define PBDSCR_PULL_DN_DRVSTRN(x)       (((x) << 8) & PBDSCR_PULL_DN_DRVSTRN_MASK)

// ESSTSR0 bit definitions
#define MEM_FREQ_MASK                   (3<<4)
#define MEM_FREQ_400 					(3<<4)
#define MEM_FREQ_533					(2<<4)
#define DEV_SEL_MASK                    (7<<7)
#define CONTROLLER_ONLY					(1<<14)
#define INTERFACE_SEL_PCIX				(1<<15)

// --------------------------------------------------------------------------
// PCIe Extended Configuration Space
#define PCIE_ERRUNC_SEV_OFFSET          0x10c
#define PCIE_FLOW_CONTROL_ERR_SEVERITY  (1 << 13)

// --------------------------------------------------------------------------
// Address Translation Unit	- PCIe
// These registers can change their base address, depending on configuration
#define ATUE_BASE						 ((*PBIU_ESSTSR0 & CONTROLLER_ONLY) && \
										 (*PBIU_ESSTSR0 & INTERFACE_SEL_PCIX)) ? 0xffdc8000 : \
										 0xffdcd000
#define ATUE_ATUVID                      REG16(ATUE_BASE,0x0)
#define ATUE_ATUDID                      REG16(ATUE_BASE,0x2)
#define ATUE_ATUDID_ASSEMBLER            0xFFDC8002
#define ATUE_ATUCMD                      REG16(ATUE_BASE,0x4)
#define ATUE_ATUSR                       REG16(ATUE_BASE,0x6)
#define ATUE_ATURID                      REG8 (ATUE_BASE,0x8)
#define ATUE_ATUCCR                      REG8 (ATUE_BASE,0x9)
#define ATUE_ATUCLSR                     REG8 (ATUE_BASE,0xc)
#define ATUE_ATULT                       REG8 (ATUE_BASE,0xd)
#define ATUE_ATUHTR                      REG8 (ATUE_BASE,0xe)
#define ATUE_ATUBIST                     REG8 (ATUE_BASE,0xf)
#define ATUE_IABAR0                      REG32(ATUE_BASE,0x10)
#define ATUE_IAUBAR0                     REG32(ATUE_BASE,0x14)
#define ATUE_IABAR1                      REG32(ATUE_BASE,0x18)
#define ATUE_IAUBAR1                     REG32(ATUE_BASE,0x1c)
#define ATUE_IABAR2                      REG32(ATUE_BASE,0x20)
#define ATUE_IAUBAR2                     REG32(ATUE_BASE,0x24)
#define ATUE_ASVIR                       REG16(ATUE_BASE,0x2c)
#define ATUE_ASIR                        REG16(ATUE_BASE,0x2e)
#define ATUE_ERBAR                       REG32(ATUE_BASE,0x30)
#define ATUE_CAP_PTR                     REG8 (ATUE_BASE,0x34)
#define ATUE_ATUILR                      REG8 (ATUE_BASE,0x3c)
#define ATUE_ATUIPR                      REG8 (ATUE_BASE,0x3d)
#define ATUE_ATUMGNT                     REG8 (ATUE_BASE,0x3e)
#define ATUE_ATUMLAT                     REG8 (ATUE_BASE,0x3f)
#define ATUE_IALR0                       REG32(ATUE_BASE,0x40)
#define ATUE_IATVR0                      REG32(ATUE_BASE,0x44)
#define ATUE_IAUTVR0                     REG32(ATUE_BASE,0x48)
#define ATUE_IALR1                       REG32(ATUE_BASE,0x4C) 
#define ATUE_IATVR1                      REG32(ATUE_BASE,0x50)
#define ATUE_IAUTVR1                     REG32(ATUE_BASE,0x54)
#define ATUE_IALR2                       REG32(ATUE_BASE,0x58)
#define ATUE_IATVR2                      REG32(ATUE_BASE,0x5C)
#define ATUE_IAUTVR2                     REG32(ATUE_BASE,0x60)
#define ATUE_ERLR                        REG32(ATUE_BASE,0x64)
#define ATUE_ERTVR                       REG32(ATUE_BASE,0x68)
#define ATUE_ERUTVR                      REG32(ATUE_BASE,0x6C)
#define ATUE_ATUCR                       REG32(ATUE_BASE,0x70)
#define ATUE_PCSR                        REG32(ATUE_BASE,0x74)
#define ATUE_PCSR_ASSEMBLER              0xFFDC8074
#define ATUE_ATUISR                      REG32(ATUE_BASE,0x78)
#define ATUE_ATUIMR                      REG32(ATUE_BASE,0x7c)
#define ATUE_PEMCSR                      REG32(ATUE_BASE,0x80)
#define ATUE_PELCSR                      REG32(ATUE_BASE,0x84)
#define ATUE_VPD_CAPID                   REG8 (ATUE_BASE,0x90)
#define ATUE_VPD_NXTP                    REG8 (ATUE_BASE,0x91)
#define ATUE_VPD_AR                      REG16(ATUE_BASE,0x92)
#define ATUE_VPD_DR                      REG32(ATUE_BASE,0x94)
#define ATUE_PM_CAPID                    REG8 (ATUE_BASE,0x98)
#define ATUE_PM_NXTP                     REG8 (ATUE_BASE,0x99)
#define ATUE_PM_CAP                      REG16(ATUE_BASE,0x9A)
#define ATUE_PM_CSR                      REG16(ATUE_BASE,0x9C)
#define ATUE_MSI_CAPID                   REG8 (ATUE_BASE,0xA0)
#define ATUE_MSI_NXTP                    REG8 (ATUE_BASE,0xA1)
#define ATUE_MSI_MCR                     REG16(ATUE_BASE,0xA2)
#define ATUE_MSI_MAR                     REG32(ATUE_BASE,0xA4)
#define ATUE_MSI_MUAR                    REG32(ATUE_BASE,0xA8)
#define ATUE_MSI_MDR                     REG16(ATUE_BASE,0xAC)
#define ATUE_MSIX_CAPID                  REG8 (ATUE_BASE,0xB0)
#define ATUE_MSIX_NXTP                   REG8 (ATUE_BASE,0xB1)
#define ATUE_MSIX_MCR                    REG16(ATUE_BASE,0xB2)
#define ATUE_MSIX_TOR                    REG32(ATUE_BASE,0xB4)
#define ATUE_MSIX_PBAOR                  REG32(ATUE_BASE,0xB8)
#define ATUE_PCIE_CAPID                  REG8 (ATUE_BASE,0xD0)
#define ATUE_PCIE_NXTP                   REG8 (ATUE_BASE,0xD1)
#define ATUE_PCIE_CAP                    REG16(ATUE_BASE,0xD2)
#define ATUE_PCIE_DCAP                   REG32(ATUE_BASE,0xD4)
#define ATUE_PCIE_DCTL                   REG16(ATUE_BASE,0xD8)
#define ATUE_PCIE_DSTS                   REG16(ATUE_BASE,0xDA)
#define ATUE_PCIE_LCAP                   REG16(ATUE_BASE,0xDC)
#define ATUE_PCIE_LCTL                   REG16(ATUE_BASE,0xE0)
#define ATUE_PCIE_LSTS                   REG16(ATUE_BASE,0xE2)
#define ATUE_PCIE_SCAP                   REG32(ATUE_BASE,0xE4)
#define ATUE_PCIE_SCR                    REG16(ATUE_BASE,0xE8)
#define ATUE_PCIE_SSTS                   REG16(ATUE_BASE,0xEA)
#define ATUE_PCIE_RCR                    REG16(ATUE_BASE,0xEC)
#define ATUE_PCIE_RSR                    REG32(ATUE_BASE,0xF0)
#define ATUE_PCIE_ADVERR_CAPID           REG32(ATUE_BASE,0x100)
#define ATUE_PCIE_ERRUNC_STS             REG32(ATUE_BASE,0x104)
#define ATUE_PCIE_ERRUNC_MSK             REG32(ATUE_BASE,0x108)
#define ATUE_PCIE_ERRUNC_SEV             REG32(ATUE_BASE,0x10C)
#define ATUE_PCIE_ERRCOR_STS             REG32(ATUE_BASE,0x110)
#define ATUE_PCIE_ERRCOR_MSK             REG32(ATUE_BASE,0x114)
#define ATUE_PCIE_ADVERR_CTL             REG32(ATUE_BASE,0x118)
#define ATUE_PCIE_ADVERR_LOG0            REG32(ATUE_BASE,0x11C)
#define ATUE_PCIE_ADVERR_LOG1            REG32(ATUE_BASE,0x120)
#define ATUE_PCIE_ADVERR_LOG2            REG32(ATUE_BASE,0x124)
#define ATUE_PCIE_ADVERR_LOG3            REG32(ATUE_BASE,0x128)
#define ATUE_PCIE_RERR_ECR               REG32(ATUE_BASE,0x12C)
#define ATUE_PCIE_RERR_ESR               REG32(ATUE_BASE,0x130)
#define ATUE_PCIE_RERR_ID                REG32(ATUE_BASE,0x134)
#define ATUE_PCIE_EDBUG1                 REG32(ATUE_BASE,0x140)
#define ATUE_PCIE_EDBUG2                 REG32(ATUE_BASE,0x144)
#define ATUE_PCIE_EIBADDR                REG32(ATUE_BASE,0x148)
#define ATUE_PCIE_EIBDATA                REG32(ATUE_BASE,0x14C)
#define ATUE_DSN_CAP                     REG32(ATUE_BASE,0x1E0)
#define ATUE_DSN_LDW                     REG32(ATUE_BASE,0x1E4)
#define ATUE_DSN_UDW                     REG32(ATUE_BASE,0x1E8)
#define ATUE_PWRBGT_CAPID                REG32(ATUE_BASE,0x1F0)
#define ATUE_PWRBGT_DSEL                 REG32(ATUE_BASE,0x1F4)
#define ATUE_PWRBGT_DATA                 REG32(ATUE_BASE,0x1F8)
#define ATUE_PWRBGT_CAP                  REG32(ATUE_BASE,0x1FC)
#define ATUE_PWRBGT_INFO_0               REG32(ATUE_BASE,0x200)
#define ATUE_PWRBGT_INFO_1               REG32(ATUE_BASE,0x204)
#define ATUE_PWRBGT_INFO_2               REG32(ATUE_BASE,0x208)
#define ATUE_PWRBGT_INFO_3               REG32(ATUE_BASE,0x20C)
#define ATUE_PWRBGT_INFO_4               REG32(ATUE_BASE,0x210)
#define ATUE_PWRBGT_INFO_5               REG32(ATUE_BASE,0x214)
#define ATUE_PWRBGT_INFO_6               REG32(ATUE_BASE,0x218)
#define ATUE_PWRBGT_INFO_7               REG32(ATUE_BASE,0x21C)
#define ATUE_PWRBGT_INFO_8               REG32(ATUE_BASE,0x220)
#define ATUE_PWRBGT_INFO_9               REG32(ATUE_BASE,0x224)
#define ATUE_PWRBGT_INFO_10              REG32(ATUE_BASE,0x228)
#define ATUE_PWRBGT_INFO_11              REG32(ATUE_BASE,0x22C)
#define ATUE_PWRBGT_INFO_12              REG32(ATUE_BASE,0x230)
#define ATUE_PWRBGT_INFO_13              REG32(ATUE_BASE,0x234)
#define ATUE_PWRBGT_INFO_14              REG32(ATUE_BASE,0x238)
#define ATUE_PWRBGT_INFO_15              REG32(ATUE_BASE,0x23C)
#define ATUE_PWRBGT_INFO_16              REG32(ATUE_BASE,0x240)
#define ATUE_PWRBGT_INFO_17              REG32(ATUE_BASE,0x244)
#define ATUE_PWRBGT_INFO_18              REG32(ATUE_BASE,0x248)
#define ATUE_PWRBGT_INFO_19              REG32(ATUE_BASE,0x24C)
#define ATUE_PWRBGT_INFO_20              REG32(ATUE_BASE,0x250)
#define ATUE_PWRBGT_INFO_21              REG32(ATUE_BASE,0x254)
#define ATUE_PWRBGT_INFO_22              REG32(ATUE_BASE,0x258)
#define ATUE_PWRBGT_INFO_23              REG32(ATUE_BASE,0x25C)
#define ATUE_OIOBAR                      REG32(ATUE_BASE,0x300)
#define ATUE_OIOWTVR                     REG32(ATUE_BASE,0x304)
#define ATUE_OUMBAR0                     REG32(ATUE_BASE,0x308)
#define ATUE_OUMWTVR0                    REG32(ATUE_BASE,0x30C)
#define ATUE_OUMBAR1                     REG32(ATUE_BASE,0x310)
#define ATUE_OUMWTVR1                    REG32(ATUE_BASE,0x314)
#define ATUE_OUMBAR2                     REG32(ATUE_BASE,0x318)
#define ATUE_OUMWTVR2                    REG32(ATUE_BASE,0x31C)
#define ATUE_OUMBAR3                     REG32(ATUE_BASE,0x320)
#define ATUE_OUMWTVR3                    REG32(ATUE_BASE,0x324)
#define ATUE_OUDMABAR                    REG32(ATUE_BASE,0x328)
#define ATUE_OCCAR                       REG32(ATUE_BASE,0x32C)
#define ATUE_OCCDR                       REG32(ATUE_BASE,0x330)
#define ATUE_OCCFN                       REG32(ATUE_BASE,0x334)
#define ATUE_IVMHR0                      REG32(ATUE_BASE,0x340)
#define ATUE_IVMHR1                      REG32(ATUE_BASE,0x344)
#define ATUE_IVMHR2                      REG32(ATUE_BASE,0x348)
#define ATUE_IVMHR3                      REG32(ATUE_BASE,0x34C)
#define ATUE_IVMPR                       REG32(ATUE_BASE,0x350)
#define ATUE_OVMHR0                      REG32(ATUE_BASE,0x360)
#define ATUE_OVMHR1                      REG32(ATUE_BASE,0x364)
#define ATUE_OVMHR2                      REG32(ATUE_BASE,0x368)
#define ATUE_OVMHR3                      REG32(ATUE_BASE,0x36C)
#define ATUE_OVMPR                       REG32(ATUE_BASE,0x370)
#define ATUE_CSR                         REG32(ATUE_BASE,0x380)
#define ATUE_STS                         REG32(ATUE_BASE,0x384)
#define ATUE_MSK                         REG32(ATUE_BASE,0x388)
#define ATUE_LOG0                        REG32(ATUE_BASE,0x38C)
#define ATUE_LOG1                        REG32(ATUE_BASE,0x390)
#define ATUE_LOG2                        REG32(ATUE_BASE,0x394)
#define ATUE_LOG3                        REG32(ATUE_BASE,0x398)
#define ATUE_DLOG                        REG32(ATUE_BASE,0x39C)
#define DEVICE_ID_MASK                   0xFFF0
#define DEVICE_ID_81342                  0x3380
#define LINK_WIDTH_SHIFT                 4
#define LINK_WIDTH_MASK                  0xF
// Single Core
#define DEVICE_ID_3380                   0x3380
// Dual Core
#define DEVICE_ID_3382                   0x3382

// --------------------------------------------------------------------------
// Address Translation Unit	- PCI-X
// These registers can change their base address, depending on configuration
#define ATUX_BASE						((*PBIU_ESSTSR0 & CONTROLLER_ONLY) == 0) ? 0xffdcc000 : \
										 ((*PBIU_ESSTSR0 & INTERFACE_SEL_PCIX) == 0) ? 0xffdc8000 : \
										 0xffdcd000
#define ATUX_ATUVID                      REG16(ATUX_BASE,0x0)
#define ATUX_ATUDID                      REG16(ATUX_BASE,0x2)
#define ATUX_ATUCMD                      REG16(ATUX_BASE,0x4)
#define ATUX_ATUSR                       REG16(ATUX_BASE,0x6)
#define ATUX_ATURID                      REG8 (ATUX_BASE,0x8)
#define ATUX_ATUCCR_PROG_IF              REG8 (ATUX_BASE,0x9)
#define ATUX_ATUCCR_SUB_CLASS            REG8 (ATUX_BASE,0xA)
#define ATUX_ATUCCR_BASE_CLASS           REG8 (ATUX_BASE,0xB)
#define ATUX_ATUCLSR                     REG8 (ATUX_BASE,0xC)
#define ATUX_ATULT                       REG8 (ATUX_BASE,0xD)
#define ATUX_ATUHTR                      REG8 (ATUX_BASE,0xE)
#define ATUX_ATUBIST                     REG8 (ATUX_BASE,0xF)
#define ATUX_IABAR0                      REG32(ATUX_BASE,0x10)
#define ATUX_IAUBAR0                     REG32(ATUX_BASE,0x14)
#define ATUX_IABAR1                      REG32(ATUX_BASE,0x18)
#define ATUX_IAUBAR1                     REG32(ATUX_BASE,0x1c)
#define ATUX_IABAR2                      REG32(ATUX_BASE,0x20)
#define ATUX_IAUBAR2                     REG32(ATUX_BASE,0x24)
#define ATUX_ASVIR                       REG16(ATUX_BASE,0x2c)
#define ATUX_ASIR                        REG16(ATUX_BASE,0x2e)
#define ATUX_ERBAR                       REG32(ATUX_BASE,0x30)
#define ATUX_CAP_PTR                     REG8 (ATUX_BASE,0x34)
#define ATUX_ATUILR                      REG8 (ATUX_BASE,0x3c)
#define ATUX_ATUIPR                      REG8 (ATUX_BASE,0x3d)
#define ATUX_ATUMGNT                     REG8 (ATUX_BASE,0x3e)
#define ATUX_ATUMLAT                     REG8 (ATUX_BASE,0x3f)
#define ATUX_IALR0                       REG32(ATUX_BASE,0x40)
#define ATUX_IATVR0                      REG32(ATUX_BASE,0x44)
#define ATUX_IAUTVR0                     REG32(ATUX_BASE,0x48)
#define ATUX_IALR1                       REG32(ATUX_BASE,0x4C) 
#define ATUX_IATVR1                      REG32(ATUX_BASE,0x50)
#define ATUX_IAUTVR1                     REG32(ATUX_BASE,0x54)
#define ATUX_IALR2                       REG32(ATUX_BASE,0x58)
#define ATUX_IATVR2                      REG32(ATUX_BASE,0x5C)
#define ATUX_IAUTVR2                     REG32(ATUX_BASE,0x60)
#define ATUX_ERLR                        REG32(ATUX_BASE,0x64)
#define ATUX_ERTVR                       REG32(ATUX_BASE,0x68)
#define ATUX_ERUTVR                      REG32(ATUX_BASE,0x6C)
#define ATUX_ATUCR                       REG32(ATUX_BASE,0x70)
#define ATUX_PCSR                        REG32(ATUX_BASE,0x74)
#define ATUX_ATUISR                      REG32(ATUX_BASE,0x78)
#define ATUX_ATUIMR                      REG32(ATUX_BASE,0x7c)
#define ATUX_VPD_CAPID                   REG8 (ATUX_BASE,0x90)
#define ATUX_VPD_NXTP                    REG8 (ATUX_BASE,0x91)
#define ATUX_VPD_AR                      REG16(ATUX_BASE,0x92)
#define ATUX_VPD_DR                      REG32(ATUX_BASE,0x94)
#define ATUX_PM_CAPID                    REG8 (ATUX_BASE,0x98)
#define ATUX_PM_NXTP                     REG8 (ATUX_BASE,0x99)
#define ATUX_PM_CAP                      REG16(ATUX_BASE,0x9A)
#define ATUX_PM_CSR                      REG16(ATUX_BASE,0x9C)
#define ATUX_MSI_CAPID                   REG8 (ATUX_BASE,0xA0)
#define ATUX_MSI_NXTP                    REG8 (ATUX_BASE,0xA1)
#define ATUX_MSI_MCR                     REG16(ATUX_BASE,0xA2)
#define ATUX_MSI_MAR                     REG32(ATUX_BASE,0xA4)
#define ATUX_MSI_MUAR                    REG32(ATUX_BASE,0xA8)
#define ATUX_MSI_MDR                     REG16(ATUX_BASE,0xAC)
#define ATUX_MSIX_CAPID                  REG8 (ATUX_BASE,0xB0)
#define ATUX_MSIX_NXTP                   REG8 (ATUX_BASE,0xB1)
#define ATUX_MSIX_MCR                    REG16(ATUX_BASE,0xB2)
#define ATUX_MSIX_TOR                    REG32(ATUX_BASE,0xB4)
#define ATUX_MSIX_PBAOR                  REG32(ATUX_BASE,0xB8)
#define ATUX_PCIX_CAPID                  REG8 (ATUX_BASE,0xD0)
#define ATUX_PCIX_NXTP                   REG8 (ATUX_BASE,0xD1)
#define ATUX_PCIX_CMD                    REG16(ATUX_BASE,0xD2)
#define ATUX_PCIX_SR                     REG32(ATUX_BASE,0xD4)
#define ATUX_ECCSR                       REG32(ATUX_BASE,0xD8)
#define ATUX_ECCFAR                      REG32(ATUX_BASE,0xDC)
#define ATUX_ECCSAR                      REG32(ATUX_BASE,0xE0)
#define ATUX_ECCAR                       REG32(ATUX_BASE,0xE4)
#define ATUX_HS_CAPID                    REG8 (ATUX_BASE,0xE8)
#define ATUX_HS_NXTP                     REG8 (ATUX_BASE,0xE9)
#define ATUX_HS_CNTRL                    REG8 (ATUX_BASE,0xEA)
#define ATUX_IABAR3                      REG32(ATUX_BASE,0x200)
#define ATUX_IAUBAR3                     REG32(ATUX_BASE,0x204)
#define ATUX_IALR3                       REG32(ATUX_BASE,0x208)
#define ATUX_IATVR3                      REG32(ATUX_BASE,0x20C)
#define ATUX_IAUTVR3                     REG32(ATUX_BASE,0x210)
#define ATUX_OIOBAR                      REG32(ATUX_BASE,0x300)
#define ATUX_OIOWTVR                     REG32(ATUX_BASE,0x304)
#define ATUX_OUMBAR0                     REG32(ATUX_BASE,0x308)
#define ATUX_OUMWTVR0                    REG32(ATUX_BASE,0x30C)
#define ATUX_OUMBAR1                     REG32(ATUX_BASE,0x310)
#define ATUX_OUMWTVR1                    REG32(ATUX_BASE,0x314)
#define ATUX_OUMBAR2                     REG32(ATUX_BASE,0x318)
#define ATUX_OUMWTVR2                    REG32(ATUX_BASE,0x31C)
#define ATUX_OUMBAR3                     REG32(ATUX_BASE,0x320)
#define ATUX_OUMWTVR3                    REG32(ATUX_BASE,0x324)
#define ATUX_OUDMABAR                    REG32(ATUX_BASE,0x328)
#define ATUX_OUMSIBAR                    REG32(ATUX_BASE,0x32C)
#define ATUX_OCCAR                       REG32(ATUX_BASE,0x330)
#define ATUX_OCCDR                       REG32(ATUX_BASE,0x334)
#define ATUX_OCCFN                       REG32(ATUX_BASE,0x338)
#define ATUX_PIE_CSR                     REG32(ATUX_BASE,0x380)
#define ATUX_PIE_AR                      REG32(ATUX_BASE,0x384)
#define ATUX_PIE_UAR                     REG32(ATUX_BASE,0x388)
#define ATUX_PIE_CAR                     REG32(ATUX_BASE,0x38C)
#define ATUX_IACR                        REG16(ATUX_BASE,0x394)
#define ATUX_MTT                         REG8 (ATUX_BASE,0x398)
#define ATUX_PRCR                        REG32(ATUX_BASE,0x2100)
#define ATUX_PPODSMOVR                   REG32(ATUX_BASE,0x2104)
#define ATUX_PPDSMOVR33_15               REG32(ATUX_BASE,0x2108)
#define ATUX_PPDSMOVR_33                 REG32(ATUX_BASE,0x210C)
// End of PCI-X ATU definitions

// Key ATU Offset definitions
#define ATUVID_OFFSET                    0x0
#define ATUDID_OFFSET                    0x2
#define ATUCMD_OFFSET                    0x4
#define ATUSR_OFFSET                     0x6
#define ATURID_OFFSET                    0x8
#define ATUCCR_PROG_IF_OFFSET            0x9
#define ATUCCR_SUB_CLASS_OFFSET          0xA
#define ATUCCR_BASE_CLASS_OFFSET         0xB
#define ATUCLSR_OFFSET                   0xC
#define ATULT_OFFSET                     0xD
#define ATUHTR_OFFSET                    0xE
#define ATUBIST_OFFSET                   0xF
#define IABAR0_OFFSET                    0x10
#define IAUBAR0_OFFSET                   0x14
#define IABAR1_OFFSET                    0x18
#define IAUBAR1_OFFSET                   0x1c
#define IABAR2_OFFSET                    0x20
#define IAUBAR2_OFFSET                   0x24
#define ASVIR_OFFSET                     0x2c
#define ASIR_OFFSET                      0x2e
#define ERBAR_OFFSET                     0x30
#define CAP_PTR_OFFSET                   0x34
#define ATUILR_OFFSET                    0x3c
#define ATUIPR_OFFSET                    0x3d
#define ATUMGNT_OFFSET                   0x3e
#define ATUMLAT_OFFSET                   0x3f
#define IALR0_OFFSET                     0x40
#define IATVR0_OFFSET                    0x44
#define IAUTVR0_OFFSET                   0x48
#define IALR1_OFFSET                     0x4C 
#define IATVR1_OFFSET                    0x50
#define IAUTVR1_OFFSET                   0x54
#define IALR2_OFFSET                     0x58
#define IATVR2_OFFSET                    0x5C
#define IAUTVR2_OFFSET                   0x60
#define ERLR_OFFSET                      0x64
#define ERTVR_OFFSET                     0x68
#define ERUTVR_OFFSET                    0x6C
#define ATUCR_OFFSET                     0x70
#define PCSR_OFFSET                      0x74
#define ATUISR_OFFSET                    0x78
#define ATUIMR_OFFSET                    0x7c

// 128Meg window
#define ATU_OMWTV_MASK                  0xF8000000

// 64k window
#define ATU_OIOWTV_MASK                 0xFFFF0000

#define PCSR_PCIX_CAPABILITY_MASK       0x000F0000
#define PCSR_PCI_MODE                   0x000F0000
#define PCSR_PCIX_66_MODE               0x000E0000
#define PCSR_PCIX_100_MODE              0x000D0000
#define PCSR_PCIX_133_MODE              0x000C0000
#define PCSR_PCIX_266_66_MODE           0x00060000
#define PCSR_PCIX_266_100_MODE          0x00050000
#define PCSR_PCIX_266_133_MODE          0x00040000


#define PCSR_CFG_RETRY                  (1 << 2)
#define PCSR_ATUX_64_BIT_CAPABLE        (1 << 8)
#define PCSR_PMODE2			(1 << 9)
#define PCSR_ATUE_END_POINT_MODE        (1 << 13)
#define PCSR_ATUX_CR_BUS_RESET          (1 << 21)
#define PCSR_ATUX_CENTRAL_RESOURCE_MODE (1 << 25)
#define PCSR_ATUE_BUS_NUM_MASK			0xFF000000

#define ATUCR_OUTBOUND_ATU_ENABLE       0x2

#define ATUCMD_IO_SPACE_ENABLE          0x1
#define ATUCMD_MEM_SPACE_ENABLE         0x2
#define ATUCMD_BUS_MASTER_ENABLE        0x4

// ATU-E Device IDs (DI = Dual Interface) (PCI-X + PCIe)
#define ATUE_DID_DEVICE_IOP341_DI		0x33C0
#define AUTE_DID_DEVICE_IOP341          0x3310

// ATU-X Device IDs (DI = Dual Interface (PCI-X + PCIe)
#define ATUX_DID_DEVICE_IOP341_DI		0x33C8
#define ATUX_DID_DEVICE_IOP341			0x3318

// Revision IDs
#define IQ8134X_REV_A0                  0
#define IQ8134X_REV_A1                  1
#define IQ8134X_REV_B0                  4
#define IQ8134X_REV_C0                  8
#define IQ8134X_REV_C1                  9

#define ATU_PX_SR_BUS_NUM_MASK          0x0000FF00
#define ATU_PX_SR_BUS_NUM(p)            (((p) >> 8) & 0xff)
#define ATU_PX_SR_FCN_NUM_MASK          0x00000007

#define ATU_IALR_VAL(size)              ((0xffffffff - ((size) - 1)) &~ 0xFFF)
#define ATU_IALR_SIZE(lr)               (~(lr) + 1)


// IOP34x has Four 4-Gig Outbound windows starting at 0x1.0000.0000
// Therefore, only need 1 window to create our 128MB of outbound space for PCIX
// PCIe Outbound window will be 128M on top of the PCIx window
#define PCI_OUTBOUND_MEM_WINDOW       0x80000000
#define PCI_OUTBOUND_MEM_WINDOW_0     PCI_OUTBOUND_MEM_WINDOW
#define PCI_OUTBOUND_MEM_WINDOW_0_TOP 0x88000000
#define PCI_OUTBOUND_MEM_WINDOW_TOP   PCI_OUTBOUND_MEM_WINDOW_0_TOP
#define PCIE_OUTBOUND_MEM_WINDOW      PCI_OUTBOUND_MEM_WINDOW_0_TOP

#define PCI_OUTBOUND_IO_WINDOW        0x90000000
#define PCI_OUTBOUND_IO_WINDOW_TOP    0x90100000
#define PCIE_OUTBOUND_IO_WINDOW       PCI_OUTBOUND_IO_WINDOW_TOP
#define OUTBOUND_WINDOW_ENABLE        0x80000000


// --------------------------------------------------------------------------
// Application DMA Unit (ADMA) - 3 Channels
#define ACCR0                           REG32(0,0xffd80000)
#define ACSR0                           REG32(0,0xffd80004)
#define ADAR0                           REG32(0,0xffd80008)
#define IIPCR0                          REG32(0,0xffd80018)
#define ANDAR0                          REG32(0,0xffd80024)
#define ADCR0                           REG32(0,0xffd80028)
#define CARMDQ0                         REG32(0,0xffd8002C)
#define ABCR0                           REG32(0,0xffd80030)
#define DLADR0                          REG32(0,0xffd80034)
#define DUADR0                          REG32(0,0xffd80038)
#define SLAR0_0                         REG32(0,0xffd8003C)
#define SLAR0_1                         REG32(0,0xffd80044)
#define SLAR0_2                         REG32(0,0xffd8004C)
#define SLAR0_3                         REG32(0,0xffd80054)
#define SLAR0_4                         REG32(0,0xffd8005C)
#define SLAR0_5                         REG32(0,0xffd80064)
#define SLAR0_6                         REG32(0,0xffd8006C)
#define SLAR0_7                         REG32(0,0xffd80074)
#define SLAR0_8                         REG32(0,0xffd8007C)
#define SLAR0_9                         REG32(0,0xffd80084)
#define SLAR0_10                        REG32(0,0xffd8008C)
#define SLAR0_11                        REG32(0,0xffd80094)
#define SLAR0_12                        REG32(0,0xffd8009C)
#define SLAR0_13                        REG32(0,0xffd800A4)
#define SLAR0_14                        REG32(0,0xffd800AC)
#define SLAR0_15                        REG32(0,0xffd800B4)
#define SUAR0_0                         REG32(0,0xffd80040)
#define SUAR0_1                         REG32(0,0xffd80048)
#define SUAR0_2                         REG32(0,0xffd80050)
#define SUAR0_3                         REG32(0,0xffd80058)
#define SUAR0_4                         REG32(0,0xffd80060)
#define SUAR0_5                         REG32(0,0xffd80068)
#define SUAR0_6                         REG32(0,0xffd80070)
#define SUAR0_7                         REG32(0,0xffd80078)
#define SUAR0_8                         REG32(0,0xffd80080)
#define SUAR0_9                         REG32(0,0xffd80088)
#define SUAR0_10                        REG32(0,0xffd80090)
#define SUAR0_11                        REG32(0,0xffd80098)
#define SUAR0_12                        REG32(0,0xffd800A0)
#define SUAR0_13                        REG32(0,0xffd800A8)
#define SUAR0_14                        REG32(0,0xffd800B0)
#define SUAR0_15                        REG32(0,0xffd800B8)
#define TPDIK0                          REG32(0,0xffd801FC)

#define ACCR1                           REG32(0,0xffd80200)
#define ACSR1                           REG32(0,0xffd80204)
#define ADAR1                           REG32(0,0xffd80208)
#define IIPCR1                          REG32(0,0xffd80218)
#define ANDAR1                          REG32(0,0xffd80224)
#define ADCR1                           REG32(0,0xffd80228)
#define CARMDQ1                         REG32(0,0xffd8022C)
#define ABCR1                           REG32(0,0xffd80230)
#define DUADR1                          REG32(0,0xffd80238)
#define SLAR1_0                         REG32(0,0xffd80238)
#define SLAR1_1                         REG32(0,0xffd8023C)
#define SLAR1_2                         REG32(0,0xffd80244)
#define SLAR1_3                         REG32(0,0xffd80254)
#define SLAR1_4                         REG32(0,0xffd8025C)
#define SLAR1_5                         REG32(0,0xffd80264)
#define SLAR1_6                         REG32(0,0xffd8026C)
#define SLAR1_7                         REG32(0,0xffd80274)
#define SLAR1_8                         REG32(0,0xffd8027C)
#define SLAR1_9                         REG32(0,0xffd80284)
#define SLAR1_10                        REG32(0,0xffd8028C)
#define SLAR1_11                        REG32(0,0xffd80294)
#define SLAR1_12                        REG32(0,0xffd8029C)
#define SLAR1_13                        REG32(0,0xffd802A4)
#define SLAR1_14                        REG32(0,0xffd802AC)
#define SLAR1_15                        REG32(0,0xffd802B4)
#define SUAR1_0                         REG32(0,0xffd80240)
#define SUAR1_1                         REG32(0,0xffd80248)
#define SUAR1_2                         REG32(0,0xffd80250)
#define SUAR1_3                         REG32(0,0xffd80258)
#define SUAR1_4                         REG32(0,0xffd80260)
#define SUAR1_5                         REG32(0,0xffd80268)
#define SUAR1_6                         REG32(0,0xffd80270)
#define SUAR1_7                         REG32(0,0xffd80278)
#define SUAR1_8                         REG32(0,0xffd80280)
#define SUAR1_9                         REG32(0,0xffd80288)
#define SUAR1_10                        REG32(0,0xffd80290)
#define SUAR1_11                        REG32(0,0xffd80298)
#define SUAR1_12                        REG32(0,0xffd802A0)
#define SUAR1_13                        REG32(0,0xffd802A8)
#define SUAR1_14                        REG32(0,0xffd802B0)
#define SUAR1_15                        REG32(0,0xffd802B8)
#define TPDIK1                          REG32(0,0xffd803FC)

#define ACCR2                           REG32(0,0xffd80400)
#define ACSR2                           REG32(0,0xffd80404)
#define ADAR2                           REG32(0,0xffd80408)
#define IIPCR2                          REG32(0,0xffd80418)
#define ANDAR2                          REG32(0,0xffd80424)
#define ADCR2                           REG32(0,0xffd80428)
#define CARMDQ2                         REG32(0,0xffd8042C)
#define ABCR2                           REG32(0,0xffd80430)
#define DUADR2                          REG32(0,0xffd80438)
#define SLAR2_0                         REG32(0,0xffd8043C)
#define SLAR2_1                         REG32(0,0xffd80444)
#define SLAR2_2                         REG32(0,0xffd8044C)
#define SLAR2_3                         REG32(0,0xffd80454)
#define SLAR2_4                         REG32(0,0xffd8045C)
#define SLAR2_5                         REG32(0,0xffd80464)
#define SLAR2_6                         REG32(0,0xffd8046C)
#define SLAR2_7                         REG32(0,0xffd80474)
#define SLAR2_8                         REG32(0,0xffd8047C)
#define SLAR2_9                         REG32(0,0xffd80484)
#define SLAR2_10                        REG32(0,0xffd8048C)
#define SLAR2_11                        REG32(0,0xffd80494)
#define SLAR2_12                        REG32(0,0xffd8049C)
#define SLAR2_13                        REG32(0,0xffd804A4)
#define SLAR2_14                        REG32(0,0xffd804AC)
#define SLAR2_15                        REG32(0,0xffd804B4)
#define SUAR2_0                         REG32(0,0xffd80440)
#define SUAR2_1                         REG32(0,0xffd80448)
#define SUAR2_2                         REG32(0,0xffd80450)
#define SUAR2_3                         REG32(0,0xffd80458)
#define SUAR2_4                         REG32(0,0xffd80460)
#define SUAR2_5                         REG32(0,0xffd80468)
#define SUAR2_6                         REG32(0,0xffd80470)
#define SUAR2_7                         REG32(0,0xffd80478)
#define SUAR2_8                         REG32(0,0xffd80480)
#define SUAR2_9                         REG32(0,0xffd80488)
#define SUAR2_10                        REG32(0,0xffd80490)
#define SUAR2_11                        REG32(0,0xffd80498)
#define SUAR2_12                        REG32(0,0xffd804A0)
#define SUAR2_13                        REG32(0,0xffd804A8)
#define SUAR2_14                        REG32(0,0xffd804B0)
#define SUAR2_15                        REG32(0,0xffd804B8)
#define TPDIK2                          REG32(0,0xffd805FC)

#define ACR_ENABLE                      1
#define ACR_RESUME                      2
#define ACR_INTERFACE_PCIE				(1 << 29)
#define ASR_ACTIVE                      0x2000
#define DC_HOST_TO_LOCAL                (0 << 1)
#define DC_LOCAL_TO_HOST                (1 << 1)
#define DC_INTERNAL_TO_LOCAL            (2 << 1)
#define DC_LOCAL_TO_INTERNAL            (3 << 1)
#define DC_MEMORY_BLOCK_FILL            (1 << 8)
#define NUM_DMA_CHANNELS                2
#define ASR_ERROR_MASK                  0x23E
#define ACR_ENABLE						1
#define ADMA_SOURCES(x)					((x-1) << 3)

// --------------------------------------------------------------------------
// RSVD Unit
#define RSVD_BASE                       0xffdce000
#define RSVD_CMD						REG16(0, 0xffdce004)
#define RSVD_BAR0						REG32(0, 0xffdce010)

// --------------------------------------------------------------------------
// Memory Controller
#define MCU_SDIR                        REG32(0,0xffd81800)
#define MCU_SDCR0                       REG32(0,0xffd81804)
#define MCU_SDCR1                       REG32(0,0xffd81808)
#define MCU_SDBR                        REG32(0,0xffd8180c)
#define MCU_SDUBR                       REG32(0,0xffd81810)
#define MCU_SBSR                        REG32(0,0xffd81814)
#define MCU_S32SR                       REG32(0,0xffd81818)
#define MCU_DECCR                       REG32(0,0xffd8181c)
#define MCU_DELOG0                      REG32(0,0xffd81820)
#define MCU_DELOG1                      REG32(0,0xffd81824)
#define MCU_DEAR0                       REG32(0,0xffd81828)
#define MCU_DEAR1                       REG32(0,0xffd8182c)
#define MCU_DECAR0                      REG32(0,0xffd81830)
#define MCU_DECAR1                      REG32(0,0xffd81834)
#define MCU_DECUAR0                     REG32(0,0xffd81838)
#define MCU_DECUAR1                     REG32(0,0xffd8183c)
#define MCU_DECTST                      REG32(0,0xffd81840)
#define MCU_DPCSR                       REG32(0,0xffd81844)
#define MCU_DPAR                        REG32(0,0xffd81848)
#define MCU_DPUAR                       REG32(0,0xffd8184c)
#define MCU_DPCAR                       REG32(0,0xffd81850)
#define MCU_DPCUAR                      REG32(0,0xffd81858)
#define MCU_DMCISR                      REG32(0,0xffd81860)
#define MCU_DMACR                       REG32(0,0xffd81864)
#define MCU_DMPTCR                      REG32(0,0xffd81868)
#define MCU_DMPCR                       REG32(0,0xffd8186c)
#define MCU_RFR                         REG32(0,0xffd81870)
#define MCU_DSDPR0                      REG32(0,0xffd81874)
#define MCU_DSDPR1                      REG32(0,0xffd81878)
#define MCU_DSDPR2                      REG32(0,0xffd8187c)
#define MCU_DSDPR3                      REG32(0,0xffd81880)
#define MCU_DSDPR4                      REG32(0,0xffd81884)
#define MCU_DSDPR5                      REG32(0,0xffd81888)
#define MCU_DSDPR6                      REG32(0,0xffd8188c)
#define MCU_DSDPR7                      REG32(0,0xffd81890)
#define MCU_DSDPR8                      REG32(0,0xffd81894)
#define MCU_DSDPR9                      REG32(0,0xffd81898)
#define MCU_DSDPR10                     REG32(0,0xffd8189c)
#define MCU_DSDPR11                     REG32(0,0xffd818a0)
#define MCU_DSDPR12                     REG32(0,0xffd818a4)
#define MCU_DSDPR13                     REG32(0,0xffd818a8)
#define MCU_DSDPR14                     REG32(0,0xffd818ac)
#define MCU_DSDPR15                     REG32(0,0xffd818b0)
#define MCU_DRCR                        REG32(0,0xffd82000)
#define MCU_RPDSR                       REG32(0,0xffd82004)
#define MCU_DQPODSR                     REG32(0,0xffd82008)
#define MCU_DQPDSR                      REG32(0,0xffd8200C)
#define MCU_ADPDSR                      REG32(0,0xffd82010)
#define MCU_MPDSR                       REG32(0,0xffd82014)
#define MCU_CKEPDSR                     REG32(0,0xffd82018)
#define MCU_DLLR0                       REG32(0,0xffd8201c)
#define MCU_DLLR1                       REG32(0,0xffd82020)
#define MCU_DLLR2                       REG32(0,0xffd82024)
#define MCU_DLLR3                       REG32(0,0xffd82028)
#define MCU_DLLR4                       REG32(0,0xffd8202c)
#define MCU_DLLRCVER                    REG32(0,0xffd82030)
#define MCU_ECCR                        MCU_DECCR
#define MCU_MCISR                       MCU_DMCISR
#define MCU_ECTST                       MCU_DECTST
#define MCU_ELOG0                       MCU_DELOG0
#define MCU_ELOG1                       MCU_DELOG1 
#define MCU_ECAR0                       MCU_DECAR0 
#define MCU_ECAR1                       MCU_DECAR1

// Row and Column Sizes supported by IOP34X
#define SBR_ROW_ADDRESS_MIN             13
#define SBR_ROW_ADDRESS_MAX             14
#define SBR_COL_ADDRESS_MIN             10
#define SBR_COL_ADDRESS_MAX             10

// Banksize specific component of SBSR register
#define SBR_128MEG                      SZ_128M
#define SBR_256MEG                      SZ_256M
#define SBR_512MEG                      SZ_512M
#define SBR_1GIG                        SZ_1G
#define SBR_2GIG                        SZ_2G

// Number of Banks in SBSR
#define SBSR_ONE_BANK					0x4
#define SBSR_TWO_BANKS					0x0

// Banksize specific component of S32SR register
#define S32SR_MEG(n)                    ((n) << 20)

// ECCR Bit definitions
#define ECCR_SINGLE_BIT_REPORT_ENABLE   0x1
#define ECCR_MULTI_BIT_REPORT_ENABLE    0x2
#define ECCR_SINGLE_BIT_CORRECT_ENABLE  0x4
#define ECCR_ECC_ENABLE                 0x8

// Refresh rates
#define RFR_400_MHZ_7_8us               0x600
#define RFR_400_MHZ_15_6us              0xC00
#define RFR_533_MHZ_7_8us               0x820
#define RFR_533_MHZ_15_6us              0x1040

// SDCR Bit Definitions
#define SDCR_DIMM_TYPE_MASK             0x1
#define SDCR_DIMM_TYPE_UNBUFFERED       0x0
#define SDCR_DIMM_TYPE_REGISTERED       0x1
#define SDCR_BUS_WIDTH_MASK             0x2
#define SDCR_BUS_WIDTH_64_BITS          0x0
#define SDCR_BUS_WIDTH_32_BITS          0x2
#define SDCR_DDR_TYPE_MASK              0x4
#define SDCR_DDR_TYPE_DDR_II            0x0
#define SDCR_ODT_TERMINATION_MASK       0x30
#define SDCR_ODT_TERMINATION_DISABLED   0x00
#define SDCR_ODT_TERMINATION_75_OHM     0x10
#define SDCR_ODT_TERMINATION_150_OHM    0x20
#define SDCR_INTERNAL_BUS_MAP_MASK      0x40
#define SDCR_INTERNAL_BUS_MAP_SET       0x40
#define SDCR_NORMAL_FIFO                0x80
#define SDCR_tCAS_SHF                   8
#define SDCR_tCAS_MASK                  0x700
#define SDCR_tCAS_MCLK_2_5              0x100
#define SDCR_tCAS_MCLK_3                0x200
#define SDCR_tCAS_MCLK_4                0x300
#define SDCR_tCAS_MCLK_5                0x400

#define SDCR_tCAS_SHF					8
#define SDCR_tWDL_SHF                   12
#define SDCR_tEDP_SHF                   16
#define SDCR_tRCD_SHF                   20
#define SDCR_tRP_SHF                    24
#define SDCR_tRAS_SHF                   27
#define SDCR_tRMW_SHF					8
#define SDCR_tWDL_MAX                   0x3
#define SDCR_tCAS_MAX                   0x7
#define SDCR_tEDP_MAX                   0x7
#define tEDP_FROM_DESIGN_8_INCH_TRACE   0x4
#define tEDP_FROM_DESIGN_10_INCH_TRACE  0x5
#define SDCR_tRCD_MAX                   0x7
#define SDCR_tRP_MAX                    0x7
#define SDCR_tRAS_MAX                   0x1f
#define SDCR_tRMW_MAX					0xf
#define SDCR_tWDL_MCLK(m)               (((m) & SDCR_tWDL_MAX) << SDCR_tWDL_SHF)
#define SDCR_tEDP_MCLK(m)               (((m) & SDCR_tEDP_MAX) << SDCR_tEDP_SHF)
// Equation 4
#define SDCR_tRAS_MCLK(m)               (((m - 1) & SDCR_tRAS_MAX) << SDCR_tRAS_SHF)
// Equation 5
#define SDCR_tRP_MCLK(m)                (((m - 1) & SDCR_tRP_MAX) << SDCR_tRP_SHF)
// Equation 6 
#define SDCR_tRCD_MCLK(m)               (((m - 1) & SDCR_tRCD_MAX) << SDCR_tRCD_SHF)
// Equation 7
#define SDCR_WDL_MCLK(m)                (((m - 2) & SDCR_tWDL_MAX) << SDCR_tWDL_SHF)
// Equation 8 - CAS = tCAS - 1 
// i.e. (CAS of 10b is equal to 3 MCLKs and likewise a CAS of 11b is equal to 4 MCLKs)
#define SDCR_CAS_MCLK(m)                (((m - 1) & SDCR_tCAS_MAX) << SDCR_tCAS_SHF)
#define SDCR_tEDP_MCLK(m)               (((m) & SDCR_tEDP_MAX) << SDCR_tEDP_SHF)
#define SDCR_tRMW_MCLK(m)               (((m) & SDCR_tRMW_MAX) << SDCR_tRMW_SHF)
#define SDCR_tWDL_MASK                  SDCR_tWDL_MCLK(~0)
#define SDCR_tEDP_MASK                  SDCR_tEDP_MCLK(~0)
#define SDCR_tRCD_MASK                  SDCR_tRCD_MCLK(~0)
#define SDCR_tRP_MASK                   SDCR_tRP_MCLK(~0)
#define SDCR_tRAS_MASK                  SDCR_tRAS_MCLK(~0)

#define SDCR_tWTRD_SHF                  0
#define SDCR_tRC_SHF                    4
#define SDCR_tWR_SHF                    9
#define SDCR_tRFC_SHF                   12
#define SDCR_tRTW_SHF                   19
#define SDCR_tWTCMD_SHF                 23
#define SDCR_tRTCMD_SHF                 27
#define SDCR_tWTRD_MAX                  0xf
#define SDCR_tRC_MAX                    0x1f
#define SDCR_tWR_MAX                    0x7
#define SDCR_tRFC_MAX                   0x3f
#define SDCR_tRTW_MAX                   0xf
#define SDCR_tWTCMD_MAX                 0xf
#define SDCR_tRTCMD_MAX                 0xf
#define SDCR_tRTP_MAX                   0xf
#define SDCR_tWTRD_MCLK(m)              (((m) & SDCR_tWTRD_MAX) << SDCR_tWTRD_SHF)
// Equation 14
#define SDCR_tRC_MCLK(m)                (((m - 1) & SDCR_tRC_MAX) << SDCR_tRC_SHF)
#define SDCR_tWR_MCLK_0                 (0x0 << SDCR_tWR_SHF)
#define SDCR_tWR_MCLK_3                 (0x2 << SDCR_tWR_SHF)
// Equation 13
#define SDCR_tWR_MCLK(m)                (((m) & SDCR_tWR_MAX) << SDCR_tWR_SHF)
// Equation 12
#define SDCR_tRFC_MCLK(m)               (((m - 1) & SDCR_tRFC_MAX) << SDCR_tRFC_SHF)
#define SDCR_tRTW_MCLK(m)               (((m) & SDCR_tRTW_MAX) << SDCR_tRTW_SHF)
#define SDCR_tWTCMD_MCLK(m)             (((m) & SDCR_tWTCMD_MAX) << SDCR_tWTCMD_SHF)
// Equation 9
#define SDCR_tRTCMD_MCLK(m)             (((m) & SDCR_tRTCMD_MAX) << SDCR_tRTCMD_SHF)
#define SDCR_tWTRD_MASK                 SDCR_tWTRD_MCLK(~0)
#define SDCR_tRC_MASK                   SDCR_tRC_MCLK(~0)
#define SDCR_tWR_MASK                   (SDCR_tWR_MAX << SDCR_tWR_SHF)
#define SDCR_tRFC_MASK                  SDCR_tRFC_MCLK(~0)
#define SDCR_tRTW_MASK                  SDCR_tRTW_MCLK(~0)
#define SDCR_tWTCMD_MASK                SDCR_tWTCMD_MCLK(~0)
#define SDCR_tRTCMD_MASK                SDCR_tRTCMD_MCLK(~0)
#define SDCR_DQS_ENABLED                0x00000000
#define SDCR_DQS_DISABLED               0x80000000
#define SDCR_DQS_ENABLE_MASK            0x80000000

#define SDCR_tWR_JEDEC_NS               15

// SDRAM MODE COMMANDS
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS_IN_SDCR0     0x0
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS3_WR3			0x00029900
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS3_WR4			0x00039900
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS4_WR3			0x0002A100
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS4_WR4          0x0003A100
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS5_WR3			0x0002A900
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS5_WR4			0x0003A900
#define MRS_DLL_RESET     							0x8000

#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS3_WR3			0x00021900
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS3_WR4			0x00031900
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS4_WR3			0x00022100
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS4_WR4         0x00032100
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS5_WR3			0x00022900
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS5_WR4			0x00032900

#define SDIR_CMD_PRECHARGE_ALL                      0x00020020
#define SDIR_CMD_EMRS_DLL_ENABLED_NO_RTT			0x00800000
#define SDIR_CMD_EMRS_DLL_ENABLED_75_OHM_RTT		0x00800200
#define SDIR_CMD_EMRS_DLL_ENABLED_150_OHM_RTT		0x00802000
#define SDIR_CMD_NOP                                0x70
#define SDIR_CMD_AUTO_REFRESH                       0x00000010
#define SDIR_CMD_ROW_ACTIVATE						0x00020030

#define SDIR_CMD_EMRS2_PROGRAM_TO_0                 0x01000000
#define SDIR_CMD_EMRS3_PROGRAM_TO_0                 0x01800000

#define SDIR_CMD_EMRS_OCD_DEFAULT					0x0001C000

#define DMACR_RSV_MASK                   0xFFFFF000

#define DLLR4_B0_VALUE                   0x0D0F0F0F

// IACR Bit Definitions
#define DMACR_PRI_HIGH                   0
#define DMACR_PRI_MED                    1
#define DMACR_PRI_LOW                    2
#define DMACR_PRI_OFF                    3

// macros to set priority for various units
#define DMACR_NIB(x)                     ((x) << 0)
#define DMACR_SIB(x)                     ((x) << 2)
#define DMACR_ADMA0(x)                   ((x) << 4)
#define DMACR_ADMA1(x)                   ((x) << 6)
#define DMACR_ADMA2(x)                   ((x) << 8)
#define DMACR_MU(x)                      ((x) << 10)

// --------------------------------------------------------------------------
// I2C - Register definitions are in hal_xscale_iop.h   
// I2C_BASE0 is the presumed location of the SDRAM SPD Device.  
// However, on IQ8134x boards, the SPD is on the 3rd I2C Bus.
#define I2C_BASE0			0xffd82500   /* I2C Bus # 0 - Reserved for Transport Core */
#define I2C_BASE1			0xffd82520   /* I2C Bus # 1 */
#define I2C_BASE2           0xffd82540   /* I2C Bus # 2 */
#define I2C_BASE_SDRAM      I2C_BASE2    /* SDRAM SPD is on I2C Bus #2 */

#ifndef __ASSEMBLER__
extern void hal_platform_i2c_bus_reset_preamble(void);
#endif
#define PLATFORM_I2C_RESET_PREAMBLE hal_platform_i2c_bus_reset_preamble // Try without the pre-amble (could be screwing something up)



// --------------------------------------------------------------------------
// XSI Arbitration
#define ARB_IBACR                       REG32(0,0xffd81640)
#define ARB_SIBATCR						REG32(0,0xffd81644)
#define ARB_SIBDTCR						REG32(0,0xffd81648)
#define PMMRBAR							0xFFFFFFFFC  /* Need MMU setup to get to this 36-bit addressed location */
#define BWBAR							REG32(0,0xffd81780)
#define BWUBAR							REG32(0,0xffd81784)
#define BWLR							REG32(0,0xffd81788)
#define BECSR							REG32(0,0xffd8178c)
#define BERAR							REG32(0,0xffd81790)
#define BERUAR							REG32(0,0xffd81794)

#define IBACR_MASK						0x007F0003
#define BECSR_ERROR_FIELD               0x3

// IBACR Bit Definitions
#define IBACR_DISABLED					1
#define IBACR_ENABLED					0 

// macros to set arbitration for various units
#define IBACR_CORE0(x)			((x) << 0)
#define IBACR_CORE1(x)			((x) << 1)
#define IBACR_SMBUS(x)			((x) << 16)
#define IBACR_ATUE(x)			((x) << 17)
#define IBACR_ATUX(x)			((x) << 18)
#define IBACR_ADMA(x)			((x) << 19)
#define IBACR_TDMA(x)			((x) << 20)
#define IBACR_DDRMCU(x)			((x) << 21)
#define IBACR_TPMI(x)			((x) << 22)

// --------------------------------------------------------------------------
// Timers

// TMR Bit Definitions
#define TMR_TERMINAL_COUNT              0x01
#define TMR_TIMER_ENABLE                0x02
#define TMR_AUTO_RELOAD_ENABLE          0x04
#define TMR_PRIVELEGED_WRITE_CONTROL    0x08
#define TMR_CLK_1                       0x00  // Internal Bus Clock
#define TMR_CLK_4                       0x10  // Internal Bus Clock/4
#define TMR_CLK_8                       0x20  // Internal Bus Clock/8
#define TMR_CLK_16                      0x30  // Internal Bus Clock/16

#ifndef __ASSEMBLER__
// For full read/write access, you have to use coprocessor insns.
#define TMR0_READ(val)     asm volatile ("mrc p6, 0, %0, c0, c9, 0" : "=r" (val))
#define _TMR0_WRITE(val)   asm volatile ("mcr p6, 0, %0, c0, c9, 0" : : "r" (val))
#define TMR1_READ(val)     asm volatile ("mrc p6, 0, %0, c1, c9, 0" : "=r" (val))
#define _TMR1_WRITE(val)   asm volatile ("mcr p6, 0, %0, c1, c9, 0" : : "r" (val))
#define TCR0_READ(val)     asm volatile ("mrc p6, 0, %0, c2, c9, 0" : "=r" (val))
#define _TCR0_WRITE(val)   asm volatile ("mcr p6, 0, %0, c2, c9, 0" : : "r" (val))
#define TCR1_READ(val)     asm volatile ("mrc p6, 0, %0, c3, c9, 0" : "=r" (val))
#define _TCR1_WRITE(val)   asm volatile ("mcr p6, 0, %0, c3, c9, 0" : : "r" (val))
#define TRR0_READ(val)     asm volatile ("mrc p6, 0, %0, c4, c9, 0" : "=r" (val))
#define _TRR0_WRITE(val)   asm volatile ("mcr p6, 0, %0, c4, c9, 0" : : "r" (val))
#define TRR1_READ(val)     asm volatile ("mrc p6, 0, %0, c5, c9, 0" : "=r" (val))
#define _TRR1_WRITE(val)   asm volatile ("mcr p6, 0, %0, c5, c9, 0" : : "r" (val))
#define TISR_READ(val)     asm volatile ("mrc p6, 0, %0, c6, c9, 0" : "=r" (val))
#define _TISR_WRITE(val)   asm volatile ("mcr p6, 0, %0, c6, c9, 0" : : "r" (val))
#define _WDTCR_READ(val)   asm volatile ("mrc p6, 0, %0, c7, c9, 0" : "=r" (val))
#define _WDTCR_WRITE(val)  asm volatile ("mcr p6, 0, %0, c7, c9, 0" : : "r" (val))
#define _WDTSR_READ(val)   asm volatile ("mrc p6, 0, %0, c8, c9, 0" : "=r" (val))
#define _WDTSR_WRITE(val)  asm volatile ("mcr p6, 0, %0, c8, c9, 0" : : "r" (val))
			     
static inline void TMR0_WRITE(cyg_uint32 val) { _TMR0_WRITE(val); }
static inline void TMR1_WRITE(cyg_uint32 val) { _TMR1_WRITE(val); }
static inline void TCR0_WRITE(cyg_uint32 val) { _TCR0_WRITE(val); }
static inline void TCR1_WRITE(cyg_uint32 val) { _TCR1_WRITE(val); }
static inline void TRR0_WRITE(cyg_uint32 val) { _TRR0_WRITE(val); }
static inline void TRR1_WRITE(cyg_uint32 val) { _TRR1_WRITE(val); }
static inline void TISR_WRITE(cyg_uint32 val) { _TISR_WRITE(val); }
static inline void WDTCR_WRITE(cyg_uint32 val) {_WDTCR_WRITE(val); }
static inline void WDTSR_WRITE(cyg_uint32 val) {_WDTSR_WRITE(val); }
#endif


// --------------------------------------------------------------------------
// Interrupts

#ifndef __ASSEMBLER__
#define INTBASE_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c2, 0" : "=r" (val))
#define _INTBASE_WRITE(val) asm volatile ("mcr p6, 0, %0, c0, c2, 0" : : "r" (val))
#define INTSIZE_READ(val)   asm volatile ("mrc p6, 0, %0, c2, c2, 0" : "=r" (val))
#define _INTSIZE_WRITE(val) asm volatile ("mcr p6, 0, %0, c2, c2, 0" : : "r" (val))
#define IINTVEC_READ(val)   asm volatile ("mrc p6, 0, %0, c3, c2, 0" : "=r" (val))
#define _IINTVEC_WRITE(val) asm volatile ("mcr p6, 0, %0, c3, c2, 0" : : "r" (val))
#define FINTVEC_READ(val)   asm volatile ("mrc p6, 0, %0, c4, c2, 0" : "=r" (val))
#define _FINTVEC_WRITE(val) asm volatile ("mcr p6, 0, %0, c4, c2, 0" : : "r" (val))
#define IPIPNDR_READ(val)   asm volatile ("mrc p6, 0, %0, c8, c2, 0" : "=r" (val))
#define _IPIPNDR_WRITE(val) asm volatile ("mcr p6, 0, %0, c8, c2, 0" : : "r" (val))

#define INTPND0_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c3, 0" : "=r" (val))
#define _INTPND0_WRITE(val) asm volatile ("mcr p6, 0, %0, c0, c3, 0" : : "r" (val))
#define INTPND1_READ(val)   asm volatile ("mrc p6, 0, %0, c1, c3, 0" : "=r" (val))
#define _INTPND1_WRITE(val) asm volatile ("mcr p6, 0, %0, c1, c3, 0" : : "r" (val))
#define INTPND2_READ(val)   asm volatile ("mrc p6, 0, %0, c2, c3, 0" : "=r" (val))
#define _INTPND2_WRITE(val) asm volatile ("mcr p6, 0, %0, c2, c3, 0" : : "r" (val))
#define INTPND3_READ(val)   asm volatile ("mrc p6, 0, %0, c3, c3, 0" : "=r" (val))
#define _INTPND3_WRITE(val) asm volatile ("mcr p6, 0, %0, c3, c3, 0" : : "r" (val))

#define INTCTL0_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c4, 0" : "=r" (val))
#define _INTCTL0_WRITE(val) asm volatile ("mcr p6, 0, %0, c0, c4, 0" : : "r" (val))
#define INTCTL1_READ(val)   asm volatile ("mrc p6, 0, %0, c1, c4, 0" : "=r" (val))
#define _INTCTL1_WRITE(val) asm volatile ("mcr p6, 0, %0, c1, c4, 0" : : "r" (val))
#define INTCTL2_READ(val)   asm volatile ("mrc p6, 0, %0, c2, c4, 0" : "=r" (val))
#define _INTCTL2_WRITE(val) asm volatile ("mcr p6, 0, %0, c2, c4, 0" : : "r" (val))
#define INTCTL3_READ(val)   asm volatile ("mrc p6, 0, %0, c3, c4, 0" : "=r" (val))
#define _INTCTL3_WRITE(val) asm volatile ("mcr p6, 0, %0, c3, c4, 0" : : "r" (val))

#define INTSTR0_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c5, 0" : "=r" (val))
#define _INTSTR0_WRITE(val) asm volatile ("mcr p6, 0, %0, c0, c5, 0" : : "r" (val))
#define INTSTR1_READ(val)   asm volatile ("mrc p6, 0, %0, c1, c5, 0" : "=r" (val))
#define _INTSTR1_WRITE(val) asm volatile ("mcr p6, 0, %0, c1, c5, 0" : : "r" (val))
#define INTSTR2_READ(val)   asm volatile ("mrc p6, 0, %0, c2, c5, 0" : "=r" (val))
#define _INTSTR2_WRITE(val) asm volatile ("mcr p6, 0, %0, c2, c5, 0" : : "r" (val))
#define INTSTR3_READ(val)   asm volatile ("mrc p6, 0, %0, c3, c5, 0" : "=r" (val))
#define _INTSTR3_WRITE(val) asm volatile ("mcr p6, 0, %0, c3, c5, 0" : : "r" (val))

#define IINTSRC0_READ(val)  asm volatile ("mrc p6, 0, %0, c0, c6, 0" : "=r" (val))
#define IINTSRC1_READ(val)  asm volatile ("mrc p6, 0, %0, c1, c6, 0" : "=r" (val))
#define IINTSRC2_READ(val)  asm volatile ("mrc p6, 0, %0, c2, c6, 0" : "=r" (val))
#define IINTSRC3_READ(val)  asm volatile ("mrc p6, 0, %0, c3, c6, 0" : "=r" (val))

#define FINTSRC0_READ(val)  asm volatile ("mrc p6, 0, %0, c0, c7, 0" : "=r" (val))
#define FINTSRC1_READ(val)  asm volatile ("mrc p6, 0, %0, c1, c7, 0" : "=r" (val))
#define FINTSRC2_READ(val)  asm volatile ("mrc p6, 0, %0, c2, c7, 0" : "=r" (val))
#define FINTSRC3_READ(val)  asm volatile ("mrc p6, 0, %0, c3, c7, 0" : "=r" (val))

#define IPR0_READ(val)      asm volatile ("mrc p6, 0, %0, c0, c8, 0" : "=r" (val))
#define _IPR0_WRITE(val)    asm volatile ("mcr p6, 0, %0, c0, c8, 0" : : "r" (val))
#define IPR1_READ(val)      asm volatile ("mrc p6, 0, %0, c1, c8, 0" : "=r" (val))
#define _IPR1_WRITE(val)    asm volatile ("mcr p6, 0, %0, c1, c8, 0" : : "r" (val))
#define IPR2_READ(val)      asm volatile ("mrc p6, 0, %0, c2, c8, 0" : "=r" (val))
#define _IPR2_WRITE(val)    asm volatile ("mcr p6, 0, %0, c2, c8, 0" : : "r" (val))
#define IPR3_READ(val)      asm volatile ("mrc p6, 0, %0, c3, c8, 0" : "=r" (val))
#define _IPR3_WRITE(val)    asm volatile ("mcr p6, 0, %0, c3, c8, 0" : : "r" (val))
#define IPR4_READ(val)      asm volatile ("mrc p6, 0, %0, c4, c8, 0" : "=r" (val))
#define _IPR4_WRITE(val)    asm volatile ("mcr p6, 0, %0, c4, c8, 0" : : "r" (val))
#define IPR5_READ(val)      asm volatile ("mrc p6, 0, %0, c5, c8, 0" : "=r" (val))
#define _IPR5_WRITE(val)    asm volatile ("mcr p6, 0, %0, c5, c8, 0" : : "r" (val))
#define IPR6_READ(val)      asm volatile ("mrc p6, 0, %0, c6, c8, 0" : "=r" (val))
#define _IPR6_WRITE(val)    asm volatile ("mcr p6, 0, %0, c6, c8, 0" : : "r" (val))
#define IPR7_READ(val)      asm volatile ("mrc p6, 0, %0, c7, c8, 0" : "=r" (val))
#define _IPR7_WRITE(val)    asm volatile ("mcr p6, 0, %0, c7, c8, 0" : : "r" (val))

static inline void INTCTL0_WRITE(cyg_uint32 val) { _INTCTL0_WRITE(val); }
static inline void INTCTL1_WRITE(cyg_uint32 val) { _INTCTL1_WRITE(val); }
static inline void INTCTL2_WRITE(cyg_uint32 val) { _INTCTL2_WRITE(val); }
static inline void INTCTL3_WRITE(cyg_uint32 val) { _INTCTL3_WRITE(val); }
static inline void INTSTR0_WRITE(cyg_uint32 val) { _INTSTR0_WRITE(val); }
static inline void INTSTR1_WRITE(cyg_uint32 val) { _INTSTR1_WRITE(val); }
static inline void INTSTR2_WRITE(cyg_uint32 val) { _INTSTR2_WRITE(val); }
static inline void INTSTR3_WRITE(cyg_uint32 val) { _INTSTR3_WRITE(val); }
static inline void IPR0_WRITE   (cyg_uint32 val) { _IPR0_WRITE   (val); }
static inline void IPR1_WRITE   (cyg_uint32 val) { _IPR1_WRITE   (val); }
static inline void IPR2_WRITE   (cyg_uint32 val) { _IPR2_WRITE   (val); }
static inline void IPR3_WRITE   (cyg_uint32 val) { _IPR3_WRITE   (val); }
static inline void IPR4_WRITE   (cyg_uint32 val) { _IPR4_WRITE   (val); }
static inline void IPR5_WRITE   (cyg_uint32 val) { _IPR5_WRITE   (val); }
static inline void IPR6_WRITE   (cyg_uint32 val) { _IPR6_WRITE   (val); }
static inline void IPR7_WRITE   (cyg_uint32 val) { _IPR7_WRITE   (val); }
static inline void INTBASE_WRITE(cyg_uint32 val) { _INTBASE_WRITE(val); }
static inline void INTSIZE_WRITE(cyg_uint32 val) { _INTSIZE_WRITE(val); }
static inline void IINTVEC_WRITE(cyg_uint32 val) { _IINTVEC_WRITE(val); }
static inline void FINTVEC_WRITE(cyg_uint32 val) { _FINTVEC_WRITE(val); }
#endif

// --------------------------------------------------------------------------
// IMU

#define IMU_DBCR                        REG32(0,0xffd80a00)
#define IMU_DEBER                       REG32(0,0xffd80a04)
#define IMU_DBAR                        REG32(0,0xffd80a10)
#define IMU_DEBEOR                      REG32(0,0xffd80a14)
// FAC0 and FAC1 are using TSR0 and TSR1 respectively 
#define IMU_TSR_FAC0			REG8(0,0xffd80b00)
#define IMU_TSR_FAC1                    REG8(0,0xffd80b01)
#define IMU_TSR_HALT                    REG8(0,0xffd80b02)
#define IMU_TSR_MASTER                  REG8(0,0xffd80b03)
#define PROC_ID_MASK					0x1
#define IMU_TSR_BASE					0xffd80b00

#define BOOT_STARTED_DOORBELL           0x1
#define BOOT_COMPLETE_DOORBELL          0x2

// --------------------------------------------------------------------------
// Exception Initiator
#ifndef __ASSEMBLER__
#define CIDR_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c0, 0" : "=r" (val))
#define _RUNNING_ON_CORE1 \
({  unsigned int _val_; \
   asm("mrc\tp6, 0, %0, c0, c0, 0" : "=r" (_val_)); \
   _val_ & 0x00000001; \
})

#define _TARRSTR_WRITE(val)  asm volatile ("mcr p6, 0, %0, c2, c1, 0" : : "r" (val))
static inline void TARRSTR_WRITE(cyg_uint32 val) { _TARRSTR_WRITE(val); }

#endif

// --------------------------------------------------------------------------
// GPIO
#define GPIO_GPOE                       REG32(0,0xffd82480)
#define GPIO_GPID                       REG32(0,0xffd82484)
#define GPIO_GPOD                       REG32(0,0xffd82488)

// --------------------------------------------------------------------------
// L2 and SuperSection Definitions
#define SUPERSECTION_BASE_ADDRESS 0xFF000000
#define SUPERSECTION_BASE_ADDRESS_OFFSET 24

#define OUTER_CACHEABLE_TABLE_WALKS 0x18

#define CB_OFFSET                   2
				   
#define TEX_OFFSET_EXTENDED_SMALL   6
#define TEX_OFFSET_TINY             6
#define TEX_OFFSET_SECTION          12
#define TEX_OFFSET_LARGE            12

#define TEX_OUTER_CACHEABLE         0x5
#define X_BIT_SECTION				(1 << 12)
#define T_BIT_SECTION				(1 << 14)

#define X_BIT_LARGE                 (1 << 12)
#define T_BIT_LARGE                 (1 << 14)

#define X_BIT_TINY					(1 << 6)
#define T_BIT_TINY					(1 << 8)

// XScale Core Instructions
#define _Read_Translation_Table_Base \
({ unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c2, c0, 0" : "=r" (_val_)); \
   _val_; \
})

#define _Write_Translation_Table_Base(TTB) \
({ asm volatile ("mcr\tp15, 0, %0, c2, c0, 0" : : "r" (TTB));  })

#define _Write_FSR(VAL) \
asm("mcr\tp15, 0, %0, c5, c0, 0" : : "r" (VAL))

#define _Write_FAR(VAL) \
asm("mcr\tp15, 0, %0, c6, c0, 0" : : "r" (VAL))

#define _Read_CCLKCFG()                                       \
({ unsigned _val_;                                            \
   asm volatile("mrc\tp14, 0, %0, c6, c0, 0" : "=r" (_val_)); \
   _val_ & 0xF;                                               \
})

#define IB_RATIO_MASK   (1 << 3)
#define IB_RATIO_2_TO_1 0
#define IB_RATIO_3_TO_1 1
#define CORE_SPEED_MASK 7

#define _Read_FAR \
({ unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c6, c0, 0" : "=r" (_val_)); \
   _val_; \
})
 
#define _Read_FSR \
({ unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c5, c0, 0" : "=r" (_val_)); \
   _val_; \
})

#define _DISABLE_MMU \
({ unsigned _val_; \
    asm volatile ("mrc\tp15, 0, %0, c1, c0, 0" : "=r" (_val_)); \
	asm volatile ("mcr\tp15, 0, %0, c1, c0, 0" : : "r" (_val_ &~ 1)); \
})

#ifdef __ASSEMBLER__
	.macro PREFETCH_FLUSH reg0
	mov	\reg0, #0
	mcr p15, 0, \reg0, c7, c5, 4
	.endm

	.macro DATA_WRITE_BARRIER reg0
	mov \reg0, #0
	mcr p15, 0, \reg0, c7, c10, 4
	.endm			  

	.macro DATA_MEMORY_BARRIER reg0
	mov \reg0, #0
	mcr p15, 0, \reg0, c7, c10, 5
	.endm

#else // __ASSEMBLER__
static inline void PREFETCH_FLUSH(void) {
	cyg_uint32 temp = 0;
	asm volatile ("mcr\tp15, 0, %0, c7, c5, 4" : : "r" (temp));
}

static inline void DATA_WRITE_BARRIER(void) {
	cyg_uint32 temp = 0;
	asm volatile ("mcr\tp15, 0, %0, c7, c10, 4" : : "r" (temp));
}

static inline void DATA_MEMORY_BARRIER(void) {
	cyg_uint32 temp = 0;
	asm volatile ("mcr\tp15, 0, %0, c7, c10, 5" : : "r" (temp));
}

// Read the CCLKFG register to determine the Core
// Speed.  This register is not available on A-step parts.
static inline int GET_CORE_SPEED_MHZ(void) {
	cyg_uint32 value;

    	value = (_Read_CCLKCFG() & CORE_SPEED_MASK);
		switch (value) {
			case 0:
				return 600;
				break;
			case 1:
				return 667;
				break;
			case 2:
				return 800;
				break;
			case 3: 
				return 933;
				break;
			case 4:
				return 1000;
				break;
			case 5:
				return 1200;
				break;
			// Should never reach here
			default: 
				return -1;
		}
}

// Read the CCLKFG register to determine the Internal Bus
// Speed.  This register is not available on A-step parts.
static inline int GET_IB_SPEED_MHZ(void) {
	cyg_uint32 value;
    	value = _Read_CCLKCFG();
		// If IB_RATIO_SET, we're 3:1
	    if (value & IB_RATIO_MASK)
			return (GET_CORE_SPEED_MHZ() / 3);
		else 
			return (GET_CORE_SPEED_MHZ() / 2);
}


#endif // __ASSEMBLER__

// --------------------------------------------------------------------------
// Software Flags used by hal_pci_cfg_flags
#define IOP34X_CONFIG_RETRY_ATUE 0x1
#define IOP34X_PCI_MASTER_ATUE   0x2
#define IOP34X_CONFIG_RETRY_ATUX 0x4
#define IOP34X_PCI_MASTER_ATUX   0x8
#define IOP34X_PCI_PRIVMEM       0x10
#define REMAP_DOWNSTREAM_ATU     0x20

// Software Flags used by hal_dual_boot_flags
#define CORE0_IS_MASTER          0x1
#define CORE1_IS_MASTER          0x2
#define CORE0_IS_RESET           0x4
#define CORE1_IS_RESET           0x8
#define MEMORY_INSTALLED_128     0x10
#define MEMORY_INSTALLED_256     0x20
#define MEMORY_INSTALLED_512     0x40
#define MEMORY_INSTALLED_1024    0x80
#define MEMORY_INSTALLED_2048    0x100
#endif // CYGONCE_HAL_ARM_XSCALE_HAL_IOP34X_H

// EOF hal_iop34x.h
