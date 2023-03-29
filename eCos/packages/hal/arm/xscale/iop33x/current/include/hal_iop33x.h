/*=============================================================================
//
//      hal_iop33x.h
//
//      IOP331 and IOP332 I/O Coprocessor support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005 Red Hat, Inc.
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
// Author(s):      drew.moseley@intel.com
// Contributors:
// Date:           2003-07-10
// Purpose:
// Description:    IOP331 and IOP332 I/O Processor support.
// Usage:          #include <cyg/hal/hal_iop33x.h>
//
// Copyright:    (C) 2003-2005 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_IOP33X_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_IOP33X_H

#include <pkgconf/system.h>
#include <pkgconf/hal_arm_xscale_iop.h>
#include <cyg/hal/hal_xscale.h>
#include <cyg/hal/hal_xscale_iop.h>
#include <cyg/hal/hal_misc.h>

#define HAL_PCI_IGNORE_DEVICE(__bus,__dev,__fn)	cyg_hal_plf_ignore_devices(__bus,__dev,__fn)

// --------------------------------------------------------------------------
// IOP331 and IOP332 Processor Device ID's
#define IOP331_DEVICE_ID_A_1           0x69054090
#define IOP331_DEVICE_ID_B_0           0x69054094
#define IOP331_DEVICE_ID_C_0           0x69054096
#define IOP331_DEVICE_ID_C_1           0x69054097
#define IOP331_DEVICE_ID_D_0           0x6905409A

#define IOP332_DEVICE_ID_A_1            0x69054010
#define IOP332_DEVICE_ID_A_2            0x69054010
#define IOP332_DEVICE_ID_B_0            0x69054014
#define IOP332_DEVICE_ID_C_0            0x69054016
#define IOP332_DEVICE_ID_C_1            0x69054017

// --------------------------------------------------------------------------
// UART
#define IOP33X_UART0_ADDR       0xFFFFF700
#define IOP33X_UART1_ADDR       0xFFFFF740

#define UART0_RBR                       REG32(0,0xfffff700)
#define UART0_THR                       REG32(0,0xfffff700)
#define UART0_DLL                       REG32(0,0xfffff700)
#define UART0_IER                       REG32(0,0xfffff704)
#define UART0_DLH                       REG32(0,0xfffff704)
#define UART0_IIR                       REG32(0,0xfffff708)
#define UART0_FCR                       REG32(0,0xfffff708)
#define UART0_LCR                       REG32(0,0xfffff70c)
#define UART0_MCR                       REG32(0,0xfffff710)
#define UART0_LSR                       REG32(0,0xfffff714)
#define UART0_MSR                       REG32(0,0xfffff718)
#define UART0_SPR                       REG32(0,0xfffff71C)
#define UART0_FOR                       REG32(0,0xfffff724)
#define UART0_ABR                       REG32(0,0xfffff728)
#define UART0_ACR                       REG32(0,0xfffff72C)

#define UART1_RBR                       REG32(0,0xfffff740)
#define UART1_THR                       REG32(0,0xfffff740)
#define UART1_DLL                       REG32(0,0xfffff740)
#define UART1_IER                       REG32(0,0xfffff744)
#define UART1_DLH                       REG32(0,0xfffff744)
#define UART1_IIR                       REG32(0,0xfffff748)
#define UART1_FCR                       REG32(0,0xfffff748)
#define UART1_LCR                       REG32(0,0xfffff74c)
#define UART1_MCR                       REG32(0,0xfffff750)
#define UART1_LSR                       REG32(0,0xfffff754)
#define UART1_MSR                       REG32(0,0xfffff758)
#define UART1_SPR                       REG32(0,0xfffff75C)
#define UART1_FOR                       REG32(0,0xfffff764)
#define UART1_ABR                       REG32(0,0xfffff768)
#define UART1_ACR                       REG32(0,0xfffff76C)

// --------------------------------------------------------------------------
// PCIE/PCIX/PCI Bridge
#define PCIE_BINIT_OFFSET                0xFC
#define PCIE_BINIT_PCI_PRIVMEM_ENABLE    0x10
#define PCIE_BINIT_CFG_RETRY             0x08
#define PCIE_BINIT_DEV_HIDING_ENABLE     0x04
#define PCIE_BINIT_INBOUND_CONFIG_ENABLE 0x02
#define PCIE_EXPRESS_DEVICE_CONTROL      0x4C
#define PCIE_DEVICE_HIDE_LOCKOUT         0x40
#define BRIDGE_CONFIGURE_RETRY_ENABLE    (1 << 15)

#define PCI_SISR_OFFSET                 0x5C

#define PCIE_BRIDGE_A_DEV_NUM           0
#define PCIE_BRIDGE_A_FN_NUM            0
#define PCIE_BRIDGE_B_DEV_NUM           0
#define PCIE_BRIDGE_B_FN_NUM            2
#define PCI_BRIDGE_DEV_NUM              0
#define PCI_BRIDGE_FN_NUM               0
#define PCI_BRIDGE_HIDDEN_DEVNUM_MIN    0
#define PCI_BRIDGE_HIDDEN_DEVNUM_MAX    9
#define PCI_BRIDGE_PUBLIC_DEVNUM_MIN    10
#define PCI_BRIDGE_PUBLIC_DEVNUM_MAX    31

// PCIE Extended Configuration Space
#define PCIE_ERRUNC_SEV_OFFSET          0x10c
#define PCIE_FLOW_CONTROL_ERR_SEVERITY  (1 << 13)

// PCIE ESD Enhancement
#define PCIE_ESD_ENHANCE_SET_OFFSET     0x260
#define PCIE_ESD_ENHANCE_SET            (1 << 15)
#define PCIE_ESD_ENHANCE_CLEAR_OFFSET   0x270
#define PCIE_ESD_ENHANCE_CLEAR          (1 << 31)

// --------------------------------------------------------------------------
// Address Translation Unit
#define ATU_ATUVID                      REG16(0,0xffffe100)
#define ATU_ATUDID                      REG16(0,0xffffe102)
#define ATU_ATUCMD                      REG16(0,0xffffe104)
#define ATU_ATUSR                       REG16(0,0xffffe106)
#define ATU_ATURID                      REG8(0,0xffffe108)
#define ATU_ATUCCR                      REG8(0,0xffffe109)
#define ATU_ATUCLSR                     REG8(0,0xffffe10c)
#define ATU_ATULT                       REG8(0,0xffffe10d)
#define ATU_ATUHTR                      REG8(0,0xffffe10e)
#define ATU_ATUBIST                     REG8(0,0xffffe10f)
#define ATU_IABAR0                      REG32(0,0xffffe110)
#define ATU_IAUBAR0                     REG32(0,0xffffe114)
#define ATU_IABAR1                      REG32(0,0xffffe118)
#define ATU_IAUBAR1                     REG32(0,0xffffe11c)
#define ATU_IABAR2                      REG32(0,0xffffe120)
#define ATU_IAUBAR2                     REG32(0,0xffffe124)
#define ATU_ASVIR                       REG16(0,0xffffe12c)
#define ATU_ASIR                        REG16(0,0xffffe12e)
#define ATU_ERBAR                       REG32(0,0xffffe130)
#define ATU_Cap_Ptr                     REG8(0,0xffffe134)
#define ATU_ATUILR                      REG8(0,0xffffe13c)
#define ATU_ATUIPR                      REG8(0,0xffffe13d)
#define ATU_ATUMGNT                     REG8(0,0xffffe13e)
#define ATU_ATUMLAT                     REG8(0,0xffffe13f)
#define ATU_IALR0                       REG32(0,0xffffe140)
#define ATU_IATVR0                      REG32(0,0xffffe144)
#define ATU_ERLR                        REG32(0,0xffffe148)
#define ATU_ERTVR                       REG32(0,0xffffe14c)
#define ATU_IALR1                       REG32(0,0xffffe150)
#define ATU_IALR2                       REG32(0,0xffffe154)
#define ATU_IATVR2                      REG32(0,0xffffe158)
#define ATU_OIOWTVR                     REG32(0,0xffffe15c)
#define ATU_OMWTVR0                     REG32(0,0xffffe160)
#define ATU_OUMWTVR0                    REG32(0,0xffffe164)
#define ATU_OMWTVR1                     REG32(0,0xffffe168)
#define ATU_OUMWTVR1                    REG32(0,0xffffe16c)
#define ATU_OUDWTVR                     REG32(0,0xffffe178)
#define ATU_PEBSABNR                    REG8(0,0xffffe17c)
#define ATU_PEBSBBNR                    REG8(0,0xffffe17d)
#define ATU_PEBPBNR                     REG8(0,0xffffe17e)
#define ATU_PEBDNUM                     REG8(0,0xffffe17f)
#define ATU_ATUCR                       REG32(0,0xffffe180)
#define ATU_PCSR                        REG32(0,0xffffe184)
#define ATU_ATUISR                      REG32(0,0xffffe188)
#define ATU_ATUIMR                      REG32(0,0xffffe18c)
#define ATU_IABAR3                      REG32(0,0xffffe190)
#define ATU_IAUBAR3                     REG32(0,0xffffe194)
#define ATU_IALR3                       REG32(0,0xffffe198)
#define ATU_IATVR3                      REG32(0,0xffffe19c)
#define ATU_OCCAR                       REG32(0,0xffffe1a4)
#define ATU_OCCDR                       REG32(0,0xffffe1ac)
#define ATU_VPD_CAPID                   REG8(0,0xffffe1b8)
#define ATU_VPD_NXTP                    REG8(0,0xffffe1b9)
#define ATU_VPD_AR                      REG16(0,0xffffe1ba)
#define ATU_VPD_DR                      REG16(0,0xffffe1bc)
#define ATU_PM_CAPID                    REG8(0,0xffffe1c0)
#define ATU_PM_NXTP                     REG8(0,0xffffe1c1)
#define ATU_PM_CAP                      REG16(0,0xffffe1c2)
#define ATU_PM_CSR                      REG16(0,0xffffe1c4)
#define ATU_MSI_CAPID                   REG8(0,0xffffe1d0)
#define ATU_MSI_NXTP                    REG8(0,0xffffe1d1)
#define ATU_MSI_MCR                     REG16(0,0xffffe1d2)
#define ATU_MSI_MAR                     REG32(0,0xffffe1d4)
#define ATU_MSI_MUAR                    REG32(0,0xffffe1d8)
#define ATU_MSI_MDR                     REG16(0,0xffffe1dc)
#define ATU_PX_CAPID                    REG8(0,0xffffe1e0)
#define ATU_PX_NXTP                     REG8(0,0xffffe1e1)
#define ATU_PX_CMD                      REG16(0,0xffffe1e2)
#define ATU_PX_SR                       REG32(0,0xffffe1e4)
#define ATU_PIRSR                       REG32(0,0xffffe1ec)
#define ATU_SPDSCR                      REG32(0,0xfffff5c0)
#define ATU_PPDSCR                      REG32(0,0xfffff5c8)

#define ATU_OMWTV_MASK                  0xFC000000
#define ATU_OIOWTV_MASK                 0xFFFF0000

#define PCSR_PCIX_CAPABILITY_MASK       0x00030000
#define PCSR_PCI_MODE                   0x00000000
#define PCSR_PCIX_66_MODE               0x00010000
#define PCSR_PCIX_100_MODE              0x00020000
#define PCSR_PCIX_133_MODE              0x00030000

#define PCSR_RESET_I_BUS                0x20
#define PCSR_PRIVMEM_ENABLE             0x01
#define PCSR_CFG_RETRY                  0x04
#define PCSR_PRIVDEV_ENABLE             0x08

#define ATUCR_OUTBOUND_ATU_ENABLE       0x2

#define ATUCMD_IO_SPACE_ENABLE          0x1
#define ATUCMD_MEM_SPACE_ENABLE         0x2
#define ATUCMD_BUS_MASTER_ENABLE        0x4

#define ATUDID_DEVICE_IOP333            0x0374
#define ATUDID_DEVICE_IOP332            0x0334
#define ATUDID_DEVICE_IOP331            0x0336

#define ATU_PX_SR_BUS_NUM_MASK          0x0000FF00
#define ATU_PX_SR_BUS_NUM(p)            (((p) >> 8) & 0xff)

#define ATU_IALR_VAL(size)              ((0xffffffff - ((size) - 1)) &~ 0xFFF)
#define ATU_IALR_SIZE(lr)               (~(lr) + 1)

#define PCI_OUTBOUND_MEM_WINDOW       0x80000000
#define PCI_OUTBOUND_MEM_WINDOW_0     PCI_OUTBOUND_MEM_WINDOW
#define PCI_OUTBOUND_MEM_WINDOW_0_TOP 0x84000000
#define PCI_OUTBOUND_MEM_WINDOW_1     0x84000000
#define PCI_OUTBOUND_MEM_WINDOW_1_TOP 0x88000000
#define PCI_OUTBOUND_MEM_WINDOW_TOP   PCI_OUTBOUND_MEM_WINDOW_1_TOP
#define PCI_OUTBOUND_IO_WINDOW        0x90000000
#define PCI_OUTBOUND_IO_WINDOW_TOP    0x90010000

#ifndef __ASSEMBLER__
static inline cyg_uint8 iop33x_localbus_number(void)
{
    cyg_uint8 localbus;

    if ((*ATU_ATUDID == ATUDID_DEVICE_IOP332)||(*ATU_ATUDID == ATUDID_DEVICE_IOP333)) {
        // For IOP332 we have a local mirror of the Bridge Bus Number registers
        localbus = *ATU_PEBSABNR;
    } else {
        if ((*ATU_PCSR & PCSR_PCIX_CAPABILITY_MASK) == PCSR_PCI_MODE)
        {
            // In PCI mode on IOP331 we have no reliable way of
            // determining our Bus Number.
            // Always assume localbus == 0.
            // This has the implication that software running on the XScale
            // has a different view of the PCI topology than the rest of the world
            // but that is probably OK.
            localbus = 0;
        } else {
            localbus = ATU_PX_SR_BUS_NUM(*ATU_PX_SR);
            if (localbus == 0xff)
                localbus = 0;
        }
    }

    return localbus;
}
#endif

// --------------------------------------------------------------------------
// Application Accelerator Unit
#define AAU_ACR                         REG32(0,0xffffe800)
#define AAU_ASR                         REG32(0,0xffffe804)
#define AAU_ADAR                        REG32(0,0xffffe808)
#define AAU_ANDAR                       REG32(0,0xffffe80c)
#define AAU_SAR1                        REG32(0,0xffffe810)
#define AAU_SAR2                        REG32(0,0xffffe814)
#define AAU_SAR3                        REG32(0,0xffffe818)
#define AAU_SAR4                        REG32(0,0xffffe81c)
#define AAU_DAR                         REG32(0,0xffffe820)
#define AAU_ABCR                        REG32(0,0xffffe824)
#define AAU_ADCR                        REG32(0,0xffffe828)
#define AAU_SAR5                        REG32(0,0xffffe82c)
#define AAU_SAR6                        REG32(0,0xffffe830)
#define AAU_SAR7                        REG32(0,0xffffe834)
#define AAU_SAR8                        REG32(0,0xffffe838)
#define AAU_EDCR0                       REG32(0,0xffffe83c)
#define AAU_SAR9                        REG32(0,0xffffe840)
#define AAU_SAR10                       REG32(0,0xffffe844)
#define AAU_SAR11                       REG32(0,0xffffe848)
#define AAU_SAR12                       REG32(0,0xffffe84c)
#define AAU_SAR13                       REG32(0,0xffffe850)
#define AAU_SAR14                       REG32(0,0xffffe854)
#define AAU_SAR15                       REG32(0,0xffffe858)
#define AAU_SAR16                       REG32(0,0xffffe85c)
#define AAU_EDCR1                       REG32(0,0xffffe860)
#define AAU_SAR17                       REG32(0,0xffffe864)
#define AAU_SAR18                       REG32(0,0xffffe868)
#define AAU_SAR19                       REG32(0,0xffffe86c)
#define AAU_SAR20                       REG32(0,0xffffe870)
#define AAU_SAR21                       REG32(0,0xffffe874)
#define AAU_SAR22                       REG32(0,0xffffe878)
#define AAU_SAR23                       REG32(0,0xffffe87c)
#define AAU_SAR24                       REG32(0,0xffffe880)
#define AAU_EDCR2                       REG32(0,0xffffe884)
#define AAU_SAR25                       REG32(0,0xffffe888)
#define AAU_SAR26                       REG32(0,0xffffe88c)
#define AAU_SAR27                       REG32(0,0xffffe890)
#define AAU_SAR28                       REG32(0,0xffffe894)
#define AAU_SAR29                       REG32(0,0xffffe898)
#define AAU_SAR30                       REG32(0,0xffffe89c)
#define AAU_SAR31                       REG32(0,0xffffe8a0)
#define AAU_SAR32                       REG32(0,0xffffe8a4)

#define ACR_ENABLE                      1
#define ACR_RESUME                      2

#define ASR_ACTIVE                      0x400
#define ASR_ERROR_MASK					0x20

#define ADCR_DWE                        0x80000000
#define ADCR_SUPPLEMENTAL_BLOCK_MASK    0x06000000
#define ADCR_EXTENDED_8                 0x02000000
#define ADCR_EXTENDED_16                0x04000000
#define ADCR_EXTENDED_32                0x06000000
#define ADCR_XOR_COMMAND				0x1
#define ADCR_MEMFILL					0x2
#define ADCR_DIRECTFILL					0x7
#define ADCR_BLOCK1_COMMAND(x)          (x << 1)
#define ADCR_BLOCK2_COMMAND(x)          (x << 4)
#define ADCR_BLOCK3_COMMAND(x)          (x << 7)
#define ADCR_BLOCK4_COMMAND(x)          (x << 10)
#define ADCR_BLOCK5_COMMAND(x)          (x << 13)
#define ADCR_BLOCK6_COMMAND(x)          (x << 16)
#define ADCR_BLOCK7_COMMAND(x)          (x << 19)
#define ADCR_BLOCK8_COMMAND(x)          (x << 22)

#define EDCR0_BLOCK9_COMMAND(x)          (x << 1)
#define EDCR0_BLOCK10_COMMAND(x)          (x << 4)
#define EDCR0_BLOCK11_COMMAND(x)          (x << 7)
#define EDCR0_BLOCK12_COMMAND(x)          (x << 10)
#define EDCR0_BLOCK13_COMMAND(x)          (x << 13)
#define EDCR0_BLOCK14_COMMAND(x)          (x << 16)
#define EDCR0_BLOCK15_COMMAND(x)          (x << 19)
#define EDCR0_BLOCK16_COMMAND(x)          (x << 22)

#define EDCR1_BLOCK17_COMMAND(x)          (x << 1)
#define EDCR1_BLOCK18_COMMAND(x)          (x << 4)
#define EDCR1_BLOCK19_COMMAND(x)          (x << 7)
#define EDCR1_BLOCK20_COMMAND(x)          (x << 10)
#define EDCR1_BLOCK21_COMMAND(x)          (x << 13)
#define EDCR1_BLOCK22_COMMAND(x)          (x << 16)
#define EDCR1_BLOCK23_COMMAND(x)          (x << 19)
#define EDCR1_BLOCK24_COMMAND(x)          (x << 22)

#define EDCR2_BLOCK25_COMMAND(x)          (x << 1)
#define EDCR2_BLOCK26_COMMAND(x)          (x << 4)
#define EDCR2_BLOCK27_COMMAND(x)          (x << 7)
#define EDCR2_BLOCK28_COMMAND(x)          (x << 10)
#define EDCR2_BLOCK29_COMMAND(x)          (x << 13)
#define EDCR2_BLOCK30_COMMAND(x)          (x << 16)
#define EDCR2_BLOCK31_COMMAND(x)          (x << 19)
#define EDCR2_BLOCK32_COMMAND(x)          (x << 22)

// --------------------------------------------------------------------------
// DMA Unit
// Channel 0
#define DMA_CCR0                        REG32(0,0xffffe400)
#define DMA_CSR0                        REG32(0,0xffffe404)
#define DMA_DAR0                        REG32(0,0xffffe40c)
#define DMA_NDAR0                       REG32(0,0xffffe410)
#define DMA_PADR0                       REG32(0,0xffffe414)
#define DMA_PUADR0                      REG32(0,0xffffe418)
#define DMA_LADR0                       REG32(0,0xffffe41c)
#define DMA_BCR0                        REG32(0,0xffffe420)
#define DMA_DCR0                        REG32(0,0xffffe424)

// Channel 1
#define DMA_CCR1                        REG32(0,0xffffe440)
#define DMA_CSR1                        REG32(0,0xffffe444)
#define DMA_DAR1                        REG32(0,0xffffe44c)
#define DMA_NDAR1                       REG32(0,0xffffe450)
#define DMA_PADR1                       REG32(0,0xffffe454)
#define DMA_PUADR1                      REG32(0,0xffffe458)
#define DMA_LADR1                       REG32(0,0xffffe45c)
#define DMA_BCR1                        REG32(0,0xffffe460)
#define DMA_DCR1                        REG32(0,0xffffe464)

#define NUM_DMA_CHANNELS  				2

#define CCR_ENABLE						0x1

#define CSR_ACTIVE						0x400
#define CSR_ERROR_MASK                  0x2E

#define DCR_MEM_TO_MEM					0x40
#define DCR_MRB_COMMAND                 0xE

// --------------------------------------------------------------------------
// DMA Unit
// Channel 0
#define DMA_CCR0                        REG32(0,0xffffe400)
#define DMA_CSR0                        REG32(0,0xffffe404)
#define DMA_DAR0                        REG32(0,0xffffe40c)
#define DMA_NDAR0                       REG32(0,0xffffe410)
#define DMA_PADR0                       REG32(0,0xffffe414)
#define DMA_PUADR0                      REG32(0,0xffffe418)
#define DMA_LADR0                       REG32(0,0xffffe41c)
#define DMA_BCR0                        REG32(0,0xffffe420)
#define DMA_DCR0                        REG32(0,0xffffe424)

// Channel 1
#define DMA_CCR1                        REG32(0,0xffffe440)
#define DMA_CSR1                        REG32(0,0xffffe444)
#define DMA_DAR1                        REG32(0,0xffffe44c)
#define DMA_NDAR1                       REG32(0,0xffffe450)
#define DMA_PADR1                       REG32(0,0xffffe454)
#define DMA_PUADR1                      REG32(0,0xffffe458)
#define DMA_LADR1                       REG32(0,0xffffe45c)
#define DMA_BCR1                        REG32(0,0xffffe460)
#define DMA_DCR1                        REG32(0,0xffffe464)

#define NUM_DMA_CHANNELS  				2
#define CCR_ENABLE						0x1
#define CSR_ACTIVE						0x400
#define DCR_MEM_TO_MEM					0x400

// --------------------------------------------------------------------------
// Memory Controller
#define MCU_SDIR                        REG32(0,0xffffe500)
#define MCU_SDCR0                       REG32(0,0xffffe504)
#define MCU_SDCR1                       REG32(0,0xffffe508)
#define MCU_SDBR                        REG32(0,0xffffe50c)
#define MCU_SBR0                        REG32(0,0xffffe510)
#define MCU_SBR1                        REG32(0,0xffffe514)
#define MCU_S32SR                       REG32(0,0xffffe518)
#define MCU_ECCR                        REG32(0,0xffffe51c)
#define MCU_ELOG0                       REG32(0,0xffffe520)
#define MCU_ELOG1                       REG32(0,0xffffe524)
#define MCU_ECAR0                       REG32(0,0xffffe528)
#define MCU_ECAR1                       REG32(0,0xffffe52c)
#define MCU_ECTST                       REG32(0,0xffffe530)
#define MCU_MCISR                       REG32(0,0xffffe534)
#define MCU_MPTCR                       REG32(0,0xffffe53c)
#define MCU_MPCR                        REG32(0,0xffffe540)
#define MCU_RFR                         REG32(0,0xffffe548)
#define MCU_DCALCSR                     REG32(0,0xfffff500)
#define MCU_DCALADDR                    REG32(0,0xfffff504)
#define MCU_DCALDATA0                   REG32(0,0xfffff508)
#define MCU_DCALDATA1                   REG32(0,0xfffff50C)
#define MCU_DCALDATA2                   REG32(0,0xfffff510)
#define MCU_DCALDATA3                   REG32(0,0xfffff514)
#define MCU_DCALDATA4                   REG32(0,0xfffff518)
#define MCU_DCALDATA5                   REG32(0,0xfffff51c)
#define MCU_DCALDATA6                   REG32(0,0xfffff520)
#define MCU_DCALDATA7                   REG32(0,0xfffff524)
#define MCU_DCALDATA8                   REG32(0,0xfffff528)
#define MCU_DCALDATA9                   REG32(0,0xfffff52c)
#define MCU_DCALDATA10                  REG32(0,0xfffff530)
#define MCU_DCALDATA11                  REG32(0,0xfffff534)
#define MCU_DCALDATA12                  REG32(0,0xfffff538)
#define MCU_DCALDATA13                  REG32(0,0xfffff53c)
#define MCU_DCALDATA14                  REG32(0,0xfffff540)
#define MCU_DCALDATA15                  REG32(0,0xfffff544)
#define MCU_DCALDATA16                  REG32(0,0xfffff548)
#define MCU_DCALDATA17                  REG32(0,0xfffff54c)
#define MCU_RCVDLY                      REG32(0,0xfffff550)
#define MCU_SLVLMIX0                    REG32(0,0xfffff554)
#define MCU_SLVLMIX1                    REG32(0,0xfffff558)
#define MCU_SLVHMIX0                    REG32(0,0xfffff55c)
#define MCU_SLVHMIX1                    REG32(0,0xfffff560)
#define MCU_SLVLEN                      REG32(0,0xfffff564)
#define MCU_MASTMIX                     REG32(0,0xfffff568)
#define MCU_MASTLEN                     REG32(0,0xfffff56c)
#define MCU_DDRDSSR                     REG32(0,0xfffff570)
#define MCU_DDRDSCR                     REG32(0,0xfffff574)
#define MCU_DDRMPCR                     REG32(0,0xfffff578)

#define MCU_SLVLMIX0_DEFAULT_VALUE_DDRII         0x33333333
#define MCU_SLVLMIX1_DEFAULT_VALUE_DDRII         0x00000003
#define MCU_SLVHMIX0_DEFAULT_VALUE_DDRII         0x33333333
#define MCU_SLVHMIX1_DEFAULT_VALUE_DDRII         0x00000003
#define MCU_SLVLEN_DEFAULT_VALUE_DDRII           0x00000003
#define MCU_MASTMIX_DEFAULT_VALUE_DDRII          0x0000000A
#define MCU_MASTLEN_DEFAULT_VALUE_DDRII          0x00000002

#define MCU_SLVLMIX0_DEFAULT_VALUE_DDR           0x66666666
#define MCU_SLVLMIX1_DEFAULT_VALUE_DDR           0x00000006
#define MCU_SLVHMIX0_DEFAULT_VALUE_DDR           0x66666666
#define MCU_SLVHMIX1_DEFAULT_VALUE_DDR           0x00000006
#define MCU_SLVLEN_DEFAULT_VALUE_DDR             0x00000003
#define MCU_MASTMIX_DEFAULT_VALUE_DDR            0x00000000
#define MCU_MASTLEN_DEFAULT_VALUE_DDR            0x00000002

// SDRAM Address Translation component of SBRx register
#define SBR_AddrTX_1                    (0x0 << 30)
#define SBR_AddrTX_2                    (0x2 << 30)
#define SBR_AddrTX_3                    (0x3 << 30)

// Row and Column Sizes supported by IOP331/IOP332
#define SBR_ROW_ADDRESS_MIN             12
#define SBR_ROW_ADDRESS_MAX             14
#define SBR_COL_ADDRESS_MIN             9
#define SBR_COL_ADDRESS_MAX             11

// Banksize specific component of SBRx register
#define SBR_32MEG                       (SZ_32M >> 25)
#define SBR_64MEG                       (SZ_64M >> 25)
#define SBR_128MEG                      (SZ_128M >> 25)
#define SBR_256MEG                      (SZ_256M >> 25)
#define SBR_512MEG                      (SZ_512M >> 25)
#define SBR_1GIG                        (SZ_1G >> 25)
#define SBR_2GIG                        (SZ_2G >> 25)

// Banksize specific component of S32SR register
#define S32SR_MEG(n)                    ((n) << 20)

// ECCR Bit definitions
#define ECCR_SINGLE_BIT_REPORT_ENABLE   0x1
#define ECCR_MULTI_BIT_REPORT_ENABLE    0x2
#define ECCR_SINGLE_BIT_CORRECT_ENABLE  0x4
#define ECCR_ECC_ENABLE                 0x8

// Refresh rates
#define RFR_333_MHZ_7_8us               0x500
#define RFR_333_MHZ_15_6us              0xA00
#define RFR_400_MHZ_7_8us               0x600
#define RFR_400_MHZ_15_6us              0xC00

// SDCR Bit Definitions
#define SDCR_DIMM_TYPE_MASK             0x1
#define SDCR_DIMM_TYPE_UNBUFFERED       0x0
#define SDCR_DIMM_TYPE_REGISTERED       0x1
#define SDCR_BUS_WIDTH_MASK             0x2
#define SDCR_BUS_WIDTH_64_BITS          0x0
#define SDCR_BUS_WIDTH_32_BITS          0x2
#define SDCR_DDR_TYPE_MASK              0x4
#define SDCR_DDR_TYPE_DDR_II            0x0
#define SDCR_DDR_TYPE_DDR               0x4
#define SDCR_ODT_TERMINATION_MASK       0x30
#define SDCR_ODT_TERMINATION_DISABLED   0x00
#define SDCR_ODT_TERMINATION_75_OHM     0x10
#define SDCR_ODT_TERMINATION_150_OHM    0x20
#define SDCR_tCAS_SHF                   8
#define SDCR_tCAS_MASK                  0x300
#define SDCR_tCAS_MCLK_2_5              0x100
#define SDCR_tCAS_MCLK_4                0x300

#define SDCR_tWDL_SHF                   12
#define SDCR_tEDP_SHF                   16
#define SDCR_tRCD_SHF                   20
#define SDCR_tRP_SHF                    24
#define SDCR_tRAS_SHF                   28
#define SDCR_tWDL_MAX                   0x3
#define SDCR_tEDP_MAX                   0x3
#define SDCR_tRCD_MAX                   0x7
#define SDCR_tRP_MAX                    0x7
#define SDCR_tRAS_MAX                   0xf
#define SDCR_tWDL_MCLK(m)               (((m) & SDCR_tWDL_MAX) << SDCR_tWDL_SHF)
#define SDCR_tEDP_MCLK(m)               (((m) & SDCR_tEDP_MAX) << SDCR_tEDP_SHF)
#define SDCR_tRCD_MCLK(m)               (((m) & SDCR_tRCD_MAX) << SDCR_tRCD_SHF)
#define SDCR_tRP_MCLK(m)                (((m) & SDCR_tRP_MAX) << SDCR_tRP_SHF)
#define SDCR_tRAS_MCLK(m)               (((m) & SDCR_tRAS_MAX) << SDCR_tRAS_SHF)
#define SDCR_tWDL_MASK                  SDCR_tWDL_MCLK(~0)
#define SDCR_tEDP_MASK                  SDCR_tEDP_MCLK(~0)
#define SDCR_tRCD_MASK                  SDCR_tRCD_MCLK(~0)
#define SDCR_tRP_MASK                   SDCR_tRP_MCLK(~0)
#define SDCR_tRAS_MASK                  SDCR_tRAS_MCLK(~0)

#define SDCR_tWTRD_SHF                  0
#define SDCR_tRC_SHF                    4
#define SDCR_tWR_SHF                    9
#define SDCR_tRFC_SHF                   12
#define SDCR_tRTW_SHF                   20
#define SDCR_tWTCMD_SHF                 24
#define SDCR_tRTCMD_SHF                 28
#define SDCR_tWTRD_MAX                  0xf
#define SDCR_tRC_MAX                    0x1f
#define SDCR_tWR_MAX                    0x7
#define SDCR_tRFC_MAX                   0x1f
#define SDCR_tRTW_MAX                   0x7
#define SDCR_tWTCMD_MAX                 0xf
#define SDCR_tRTCMD_MAX                 0x7
#define SDCR_tWTRD_MCLK(m)              (((m) & SDCR_tWTRD_MAX) << SDCR_tWTRD_SHF)
#define SDCR_tRC_MCLK(m)                (((m) & SDCR_tRC_MAX) << SDCR_tRC_SHF)
#define SDCR_tWR_MCLK_0                 (0x0 << SDCR_tWR_SHF)
#define SDCR_tWR_MCLK_3                 (0x2 << SDCR_tWR_SHF)
#define SDCR_tRFC_MCLK(m)               (((m) & SDCR_tRFC_MAX) << SDCR_tRFC_SHF)
#define SDCR_tRTW_MCLK(m)               (((m) & SDCR_tRTW_MAX) << SDCR_tRTW_SHF)
#define SDCR_tWTCMD_MCLK(m)             (((m) & SDCR_tWTCMD_MAX) << SDCR_tWTCMD_SHF)
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
#define SDIR_CMD_MRS_DLL_NOT_RESET_CAS_IN_SDCR0      0x0
#define SDIR_CMD_MRS_DLL_IS_RESET_CAS_4_BURST_4      0x1
#define SDIR_CMD_PRECHARGE_ALL                       0x2
#define SDIR_CMD_NOP                                 0x3
#define SDIR_CMD_EMRS_DLL_ENABLED                    0x4
#define SDIR_CMD_EMRS_DLL_DISABLED                   0x5
#define SDIR_CMD_AUTO_REFRESH                        0x6
#define SDIR_CMD_NORMAL_OPERATION                    0xf

// DCALCSR Commands
#define DCALCSR_AUTO_REFRESH_CS_0       0x80000001
#define DCALCSR_AUTO_REFRESH_CS_1       0x80100001
#define DCALCSR_EMRS_CS_0     			0x81000003
#define DCALCSR_EMRS_CS_1     			0x81100003
#define DCALCSR_OPERATION_IN_PROGRESS   0x80000000
#define DCALCSR_PASS_FAIL_MASK          0x70000000

// DCALADDR Addresses
#define DCALADDR_EMRS1      			0x00000001
#define DCALADDR_EMRS2      			0x00000002
#define DCALADDR_EMRS3			        0x00000003
#define DCALADDR_OCD_CAL_DEFAULT		(0x380 << 16)
#define DCALADDR_OCD_CAL_EXIT			0
#define DCALADDR_RTT_75_OHM				(0x4 << 16)
#define DCALADDR_RTT_150_OHM            (0x40 << 16)
#define DCALADDR_RTT_50_OHM             (0x44 << 16)


// --------------------------------------------------------------------------
// Peripheral Bus Interface Unit
#define PBIU_PBCR                       REG32(0,0xffffe680)
#define PBIU_PBSR                       REG32(0,0xffffe684)
#define PBIU_PBAR0                      REG32(0,0xffffe688)
#define PBIU_PBLR0                      REG32(0,0xffffe68c)
#define PBIU_PBAR1                      REG32(0,0xffffe690)
#define PBIU_PBLR1                      REG32(0,0xffffe694)
#define PBIU_PMBR0                      REG32(0,0xffffe6c0)
#define PBIU_PMBR1                      REG32(0,0xffffe6e0)
#define PBIU_PMBR2                      REG32(0,0xffffe6e4)
#define PBIU_PBDSCR                     REG32(0,0xfffff580)

// PBCR Bit Definitions
#define PBCR_ENABLE                     0x1
#define PBCR_PCI_BOOT_ENABLE            0x8

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
#define PBAR_READONLY                   (1 << 9)
#define PBAR_BAR_MASK                   0xFFFFF000

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
#define PBLR_SZ_DISABLE                 0x00000000

// PBDSCR Bit Definitions
#define PBDSCR_PULL_UP_DRVSTRN_MASK     0x3F
#define PBDSCR_PULL_DN_DRVSTRN_MASK     0x3F00
#define PBDSCR_PULL_UP_DRVSTRN(x)       (((x) << 0) & PBDSCR_PULL_DN_DRVSTRN_MASK)
#define PBDSCR_PULL_DN_DRVSTRN(x)       (((x) << 8) & PBDSCR_PULL_DN_DRVSTRN_MASK)

// --------------------------------------------------------------------------
// I2C
// The bulk of the definitions are provided by the IOP_COMMON package
#define I2C_BASE0			0xfffff680
#define I2C_BASE1			0xfffff6A0

#ifndef __ASSEMBLER__
extern void hal_platform_i2c_bus_reset_preamble(void);
#endif
#define PLATFORM_I2C_RESET_PREAMBLE     hal_platform_i2c_bus_reset_preamble

// --------------------------------------------------------------------------
// Arbitration
#define ARB_IACR                        REG32(0,0xffffe7f0)
#define ARB_MTTR1                       REG32(0,0xffffe7f4)
#define ARB_MTTR2                       REG32(0,0xffffe7f8)

#define IACR_RSV_MASK                   0xFFFC030C

// IACR Bit Definitions
#define IACR_PRI_HIGH                   0
#define IACR_PRI_MED                    1
#define IACR_PRI_LOW                    2
#define IACR_PRI_OFF                    3

// macros to set priority for various units
#define IACR_ATU(x)                     ((x) << 0)
#define IACR_DMA0(x)                    ((x) << 4)
#define IACR_DMA1(x)                    ((x) << 6)
#define IACR_CORE(x)                    ((x) << 10)
#define IACR_AAU(x)                     ((x) << 12)
#define IACR_PBI(x)                     ((x) << 14)
#define IACR_MCU(x)                     ((x) << 16)


// --------------------------------------------------------------------------
// Timers
#define TU_TMR0                         REG32(0,0xffffe7d0)
#define TU_TMR1                         REG32(0,0xffffe7d4)
#define TU_TCR0                         REG32(0,0xffffe7d8)
#define TU_TCR1                         REG32(0,0xffffe7dc)
#define TU_TRR0                         REG32(0,0xffffe7e0)
#define TU_TRR1                         REG32(0,0xffffe7e4)
#define TU_TISR                         REG32(0,0xffffe7e8)
#define TU_WDTCR                        REG32(0,0xffffe7ec)

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
#define TMR0_READ(val)    asm volatile ("mrc p6, 0, %0, c0, c1, 0" : "=r" (val))
#define _TMR0_WRITE(val)  asm volatile ("mcr p6, 0, %0, c0, c1, 0" : : "r" (val))
#define TMR1_READ(val)    asm volatile ("mrc p6, 0, %0, c1, c1, 0" : "=r" (val))
#define _TMR1_WRITE(val)  asm volatile ("mcr p6, 0, %0, c1, c1, 0" : : "r" (val))
#define TCR0_READ(val)    asm volatile ("mrc p6, 0, %0, c2, c1, 0" : "=r" (val))
#define _TCR0_WRITE(val)  asm volatile ("mcr p6, 0, %0, c2, c1, 0" : : "r" (val))
#define TCR1_READ(val)    asm volatile ("mrc p6, 0, %0, c3, c1, 0" : "=r" (val))
#define _TCR1_WRITE(val)  asm volatile ("mcr p6, 0, %0, c3, c1, 0" : : "r" (val))
#define TRR0_READ(val)    asm volatile ("mrc p6, 0, %0, c4, c1, 0" : "=r" (val))
#define _TRR0_WRITE(val)  asm volatile ("mcr p6, 0, %0, c4, c1, 0" : : "r" (val))
#define TRR1_READ(val)    asm volatile ("mrc p6, 0, %0, c5, c1, 0" : "=r" (val))
#define _TRR1_WRITE(val)  asm volatile ("mcr p6, 0, %0, c5, c1, 0" : : "r" (val))
#define TISR_READ(val)    asm volatile ("mrc p6, 0, %0, c6, c1, 0" : "=r" (val))
#define _TISR_WRITE(val)  asm volatile ("mcr p6, 0, %0, c6, c1, 0" : : "r" (val))
#define _WDTCR_READ(val)  asm volatile ("mrc p6, 0, %0, c7, c1, 0" : "=r" (val))
#define _WDTCR_WRITE(val) asm volatile ("mcr p6, 0, %0, c7, c1, 0" : : "r" (val))

static inline void TMR0_WRITE(cyg_uint32 val) { _TMR0_WRITE(val); }
static inline void TMR1_WRITE(cyg_uint32 val) { _TMR1_WRITE(val); }
static inline void TCR0_WRITE(cyg_uint32 val) { _TCR0_WRITE(val); }
static inline void TCR1_WRITE(cyg_uint32 val) { _TCR1_WRITE(val); }
static inline void TRR0_WRITE(cyg_uint32 val) { _TRR0_WRITE(val); }
static inline void TRR1_WRITE(cyg_uint32 val) { _TRR1_WRITE(val); }
static inline void TISR_WRITE(cyg_uint32 val) { _TISR_WRITE(val); }
#endif

// --------------------------------------------------------------------------
// Interrupts
#define INTCTL0                         REG32(0,0xffffe790)
#define INTCTL1                         REG32(0,0xffffe794)
#define INTSTR0                         REG32(0,0xffffe798)
#define INTSTR1                         REG32(0,0xffffe79c)
#define IINTSRC0                        REG32(0,0xffffe7a0)
#define IINTSRC1                        REG32(0,0xffffe7a4)
#define FINTSRC0                        REG32(0,0xffffe7a8)
#define FINTSRC1                        REG32(0,0xffffe7ac)
#define IPR0                            REG32(0,0xffffe7b0)
#define IPR1                            REG32(0,0xffffe7b4)
#define IPR2                            REG32(0,0xffffe7b8)
#define IPR3                            REG32(0,0xffffe7bc)
#define INTBASE                         REG32(0,0xffffe7c0)
#define INTSIZE                         REG32(0,0xffffe7c4)
#define IINTVEC                         REG32(0,0xffffe7c8)
#define FINTVEC                         REG32(0,0xffffe7cc)
#define PIRSR                           REG32(0,0xffffe1ec)

#ifndef __ASSEMBLER__
#define INTCTL0_READ(val)   asm volatile ("mrc p6, 0, %0, c0,  c0, 0" : "=r" (val))
#define _INTCTL0_WRITE(val) asm volatile ("mcr p6, 0, %0, c0,  c0, 0" : : "r" (val))
#define INTCTL1_READ(val)   asm volatile ("mrc p6, 0, %0, c1,  c0, 0" : "=r" (val))
#define _INTCTL1_WRITE(val) asm volatile ("mcr p6, 0, %0, c1,  c0, 0" : : "r" (val))
#define INTSTR0_READ(val)   asm volatile ("mrc p6, 0, %0, c2,  c0, 0" : "=r" (val))
#define _INTSTR0_WRITE(val) asm volatile ("mcr p6, 0, %0, c2,  c0, 0" : : "r" (val))
#define INTSTR1_READ(val)   asm volatile ("mrc p6, 0, %0, c3,  c0, 0" : "=r" (val))
#define _INTSTR1_WRITE(val) asm volatile ("mcr p6, 0, %0, c3,  c0, 0" : : "r" (val))
#define IINTSRC0_READ(val)  asm volatile ("mrc p6, 0, %0, c4,  c0, 0" : "=r" (val))
#define IINTSRC1_READ(val)  asm volatile ("mrc p6, 0, %0, c5,  c0, 0" : "=r" (val))
#define FINTSRC0_READ(val)  asm volatile ("mrc p6, 0, %0, c6,  c0, 0" : "=r" (val))
#define FINTSRC1_READ(val)  asm volatile ("mrc p6, 0, %0, c7,  c0, 0" : "=r" (val))
#define IPR0_READ(val)      asm volatile ("mrc p6, 0, %0, c8,  c0, 0" : "=r" (val))
#define _IPR0_WRITE(val)    asm volatile ("mrc p6, 0, %0, c8,  c0, 0" : : "r" (val))
#define IPR1_READ(val)      asm volatile ("mrc p6, 0, %0, c9,  c0, 0" : "=r" (val))
#define _IPR1_WRITE(val)    asm volatile ("mrc p6, 0, %0, c9,  c0, 0" : : "r" (val))
#define IPR2_READ(val)      asm volatile ("mrc p6, 0, %0, c10, c0, 0" : "=r" (val))
#define _IPR2_WRITE(val)    asm volatile ("mrc p6, 0, %0, c10, c0, 0" : : "r" (val))
#define IPR3_READ(val)      asm volatile ("mrc p6, 0, %0, c11, c0, 0" : "=r" (val))
#define _IPR3_WRITE(val)    asm volatile ("mrc p6, 0, %0, c11, c0, 0" : : "r" (val))
#define INTBASE_READ(val)   asm volatile ("mrc p6, 0, %0, c12, c0, 0" : "=r" (val))
#define _INTBASE_WRITE(val) asm volatile ("mrc p6, 0, %0, c12, c0, 0" : : "r" (val))
#define INTSIZE_READ(val)   asm volatile ("mrc p6, 0, %0, c13, c0, 0" : "=r" (val))
#define _INTSIZE_WRITE(val) asm volatile ("mrc p6, 0, %0, c13, c0, 0" : : "r" (val))
#define IINTVEC_READ(val)   asm volatile ("mrc p6, 0, %0, c14, c0, 0" : "=r" (val))
#define _IINTVEC_WRITE(val) asm volatile ("mrc p6, 0, %0, c14, c0, 0" : : "r" (val))
#define FINTVEC_READ(val)   asm volatile ("mrc p6, 0, %0, c15, c0, 0" : "=r" (val))
#define _FINTVEC_WRITE(val) asm volatile ("mrc p6, 0, %0, c15, c0, 0" : : "r" (val))

static inline void INTCTL0_WRITE(cyg_uint32 val) { _INTCTL0_WRITE(val); }
static inline void INTCTL1_WRITE(cyg_uint32 val) { _INTCTL1_WRITE(val); }
static inline void INTSTR0_WRITE(cyg_uint32 val) { _INTSTR0_WRITE(val); }
static inline void INTSTR1_WRITE(cyg_uint32 val) { _INTSTR1_WRITE(val); }
static inline void IPR0_WRITE   (cyg_uint32 val) { _IPR0_WRITE   (val); }
static inline void IPR1_WRITE   (cyg_uint32 val) { _IPR1_WRITE   (val); }
static inline void IPR2_WRITE   (cyg_uint32 val) { _IPR2_WRITE   (val); }
static inline void IPR3_WRITE   (cyg_uint32 val) { _IPR3_WRITE   (val); }
static inline void INTBASE_WRITE(cyg_uint32 val) { _INTBASE_WRITE(val); }
static inline void INTSIZE_WRITE(cyg_uint32 val) { _INTSIZE_WRITE(val); }
static inline void IINTVEC_WRITE(cyg_uint32 val) { _IINTVEC_WRITE(val); }
static inline void FINTVEC_WRITE(cyg_uint32 val) { _FINTVEC_WRITE(val); }
#endif

// --------------------------------------------------------------------------
// GPIO
#define GPIO_GPOE                       REG32(0,0xfffff780)
#define GPIO_GPID                       REG32(0,0xfffff784)
#define GPIO_GPOD                       REG32(0,0xfffff788)

// --------------------------------------------------------------------------
// Peripheral Bus Interface Unit
#define BIU_BIUCR                       REG32(0,0xffffe608)
#define BIU_BIUSR                       REG32(0,0xffffe600)
#define BIU_BEAR                        REG32(0,0xffffe604)

#define BIUSR_RC_ERROR_BITS             0x000080011   

// BIUCR Bit Definitions
#define BIUCR_MCU_PORT_ENABLE           0x1

// --------------------------------------------------------------------------
// Software Flags
#define IOP33X_CONFIG_RETRY     0x1
#define IOP33X_DEV_HIDING       0x2
#define IOP33X_PCI_MASTER       0x4
#define IOP33X_PCI_PRIVMEM      0x8

#endif // CYGONCE_HAL_ARM_XSCALE_HAL_IOP33X_H

// EOF hal_iop33x.h
