/*****************************************************************************
 *
 * Tundra Semiconductor Corporation
 * 
 * Copyright 2002, 2003, Tundra Semiconductor Corporation
 * 
 * This software is the property of Tundra Semiconductor Corporation
 * (Tundra), which specifically grants the user the right to use this
 * software in connection with the Intel(r) 80314 Companion Chip, provided
 * this notice is not removed or altered.  All other rights are reserved
 * by Tundra.
 * 
 * TUNDRA MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD
 * TO THIS SOFTWARE.  IN NO EVENT SHALL AMD BE LIABLE FOR INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING FROM THE
 * FURNISHING, PERFORMANCE, OR USE OF THIS SOFTWARE.
 * 
 *****************************************************************************/

/*----------                           hal_ioc80314_pci.h                 ----------*/
/* Purpose:       This file provides defines for 80314 PCI ports             */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifndef IOC80314_PCI_H
#define IOC80314_PCI_H


//----------    PCI Registers
#define PCI_PCI_ID              0x000
#define PCI_PCI_CSR             0x004
#define PCI_PCI_CLASS           0x008
#define PCI_PCI_MISC0           0x00C
#define PCI_PCI_P2S_BAR0        0x010
#define PCI_PCI_P2S_BAR0_UPPER  0x014
#define PCI_PCI_BUSNUM          0x018
#define PCI_PCI_P2S_BAR2        0X018
#define PCI_PCI_MISC1           0x01C
#define PCI_PCI_P2S_BAR2_UPPER  0X01C
#define PCI_PCI_MIO_BL          0x020
#define PCI_PCI_P2S_BAR3        0X020
#define PCI_PCI_PFM_BL          0x024
#define PCI_PCI_P2S_BAR3_UPPER  0X024
#define PCI_PCI_PFM_B_UPPER     0x028
#define PCI_PCI_PFM_L_UPPER     0x02C
#define PCI_PCI_SID             0X02C
#define PCI_PCI_IO_UPPER        0x030
#define PCI_PCI_PE_EROM         0X030
#define PCI_PCI_CAP             0x034
#define PCI_PCI_PT_EROM         0x038
#define PCI_PCI_MISC2           0x03C
//----------    PCI Specific Registers
#define PCI_PCI_MISC_CSR        0x040
#define PCI_PCI_SERR_DIS        0x044
#define PCI_PCI_SERR_STAT       0x048
#define PCI_PCI_P2S_PAGE_SIZES  0x04C
#define PCI_PCI_VPD_CSR         0x050
#define PCI_PCI_PFM_FABRIC_BAR  0x064
#define PCI_PCI_PFM_FABRIC_B_UP 0x068
#define PCI_PCI_SLOT_ID         0x0C8
#define PCI_PCI_PCM             0x0CC
#define PCI_PCI_PMCS            0x0D0
#define PCI_PCI_HS_CSR          0x0D4
#define PCI_PCI_VPDC            0x0D8
#define PCI_PCI_VPDD            0x0DC
#define PCI_PCI_IC_MSIC         0x0E0
#define PCI_PCI_MSIA            0x0E4
#define PCI_PCI_MSIA_UPPER      0x0E8
#define PCI_PCI_MSID            0x0EC
#define PCI_PCI_PCIX_CAP        0x0F0
#define PCI_PCI_PCIX_STAT       0x0F4
#define PCI_PCI_PCIX_UP         0x0F8
#define PCI_PCI_PCIX_DOWN       0x0FC

//----------    PCI Capabilitiy List Registers
//----------    PCI Miscellaaneous Registers
#define PCI_ARB_CTRL            0x10C
#define PCI_CSR_SHADOW          0x110
#define PCI_MISC1_SHADOW        0x114
#define PCI_MISC2_SHADOW        0x118
#define PCI_SERR_STAT_SHADOW    0x11C
#define PCI_PCIX_CAP_SHADOW     0x120
#define PCI_PCIX_STAT_SHADOW    0x124
#define PCI_PCIX_OP_SHADOW      0x128
#define PCI_LAST_ADDR_UP_SHADOW 0x12C
#define PCI_LAST_ADDR_LO_SHADOW 0x130
#define PCI_LAST_ATTR_SHADOW    0x134
#define PCI_IRP_CFG_CTL         0x180
#define PCI_IRP_STAT            0x184
#define PCI_IRP_ENABLE          0x188
#define PCI_IRP_INTAD           0x18C
#define PCI_VID_OVERRIDE        0x19C
#define PCI_CLASS_OVERRIDE      0x1A0
#define PCI_SID_OVERRIDE        0x1A4
#define PCI_SLOT_ID_OVERRIDE    0x1A8

//----------    PCI SFN Fabric Registers
#define PCI_PFAB_CSR            0x200
#define PCI_PFAB_BAR0           0x204
#define PCI_PFAB_BAR0_UPPER     0x208
#define PCI_PFAB_IO             0x20C
#define PCI_PFAB_IO_UPPER       0x210
#define PCI_PFAB_MEM32          0x214
#define PCI_PFAB_MEM32_REMAP    0x218
#define PCI_PFAB_MEM32_MASK     0x21C

#define PCI_PFAB_PFM3           0x220
#define PCI_PFAB_PFM3_REMAP_UP  0x224
#define PCI_PFAB_PFM3_REMAP_L0  0x228
#define PCI_PFAB_PFM3_MASK      0x22C

#define PCI_PFAB_PFM4           0x230
#define PCI_PFAB_PFM4_REMAP_UP  0x234
#define PCI_PFAB_PFM4_REMAP_LO  0x238
#define PCI_PFAB_PFM4_MASK      0x23C

#define PCI_PFAB_SYNC_BAR       0x240
#define PCI_PFAB_SYNC_BAR_UPPER 0x244

#define PCI_EROM_MAP            0X2FC
#define PCI_P2S_FAB_LUT         0X300
#define PCI_P2S_BAR2_LUT        0X500
#define PCI_P2S_BAR3_LUT        0X600





//..........  PCI SFN Fabric Registers Bit field definitions
#define PCI_PFAB_BAR0_EN        0x00000001
#define PCI_PFAB_IO_EN          0x00000001

#define PCI_PFAB_MEM32_SIZE     0x00020000
#define PCI_PFAB_MEM32_EN       0x00010000

#define PCI_PFAB_PFM3_SIZE      0x00020000
#define PCI_PFAB_PFM3_EN        0x00010000
#define PCI_PFAB_PFM3_REMAP_U   0xFFFFF000
#define PCI_PFAB_PFM3_REMAP_LU  0x00000FFF
#define PCI_PFAB_PFM3_REMAP_L   0xFFFFF000

#define PCI_PFAB_PFM4_SIZE      0x00020000
#define PCI_PFAB_PFM4_EN        0x00010000
#define PCI_PFAB_PFM4_REMAP_U   0xFFFFF000
#define PCI_PFAB_PFM4_REMAP_LU  0x00000FFF
#define PCI_PFAB_PFM4_REMAP_L   0xFFFFF000

//----------    PCI SFN P2S Lookup Table Registers
//----------    PCI SFN P2S Prefetch Memory BAR2 Lookup Table Registers
//----------    PCI SFN P2S Prefetch Memory BAR3 Lookup Table Registers



/********************************************************/

#ifndef __ASSEMBLER__

typedef struct _EMB_PCI_DECODE
{
    UINT32 config_bar_upper;
    UINT32 config_bar;

    UINT32 io_bar_upper;;
    UINT32 io_bar;

    UINT32 mem32_bar;
    UINT32 mem32_remap;
    UINT32 mem32_mask;

    UINT32 pfm3_bar;
    UINT32 pfm3_remap_u;
    UINT32 pfm3_remap_l;
    UINT32 pfm3_mask;

    UINT32 pfm4_bar;
    UINT32 pfm4_remap_u;
    UINT32 pfm4_remap_l;
    UINT32 pfm4_mask;
} EMB_PCI_DECODE;

#endif  // __ASSEMBLER__

#endif  // 80314_PCI_H

/*----------          EOF for          hal_ioc80314_pci.h                 ----------*/
