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

#ifndef _TS_DMA_H
#define _TS_DMA_H

/*----------------------------------------------------------------------------
 * FILENAME: hal_ioc80314_dma.h
 *
 * ORIGINATOR: Alex Bounine
 * COMPILER(S): GNUPro (gcc)
 * COMPILER VERSION: xscale-020523
 *
 *  $Revision: $
 *      $Date: $
 *
 * DESCRIPTION: This file contains the register definitions for the DMA/XOR
 *              functional block.
 *
 *   History  :
 *   $Log: hal_ioc80314_dma.h,v $
 *---------------------------------------------------------------------------*/


#define DMA_REG_BASE        (XS_CSR_BASE_ADDR + 0x5000)

#define XOR_MAX_BLOCK_NUM 15


/*----------------------------------------------------------------------------
 * DMA/XOR CHANNEL REGISTER OFFSETS
 *--------------------------------------------------------------------------*/

/* Channel X Source Address (Most Significant Bits) */
#define CH_SRC_ADDR_M       (0x000)
/* Channel X Source Address (Least Significant Bits) */
#define CH_SRC_ADDR_L       (0x004)
/* Channel X Destination Address (Most Significant Bits) */
#define CH_DST_ADDR_M       (0x008)
/* Channel X Destination Address (Least Significant Bits) */
#define CH_DST_ADDR_L       (0x00C)
/* Channel X Transfer Control Register 1 */
#define CH_TCR1             (0x010)
/* Channel X Transfer Control Register 2 */
#define CH_TCR2             (0x014)
/* Channel X Next Descriptor Address (Most Significant Bits) */
#define CH_ND_ADDR_M        (0x018)
/* Channel X Next Descriptor Address (Least Significant Bits) */
#define CH_ND_ADDR_L        (0x01C)
/* Channel X Next Descriptor Transfer Control */
#define CH_ND_TCR           (0x020)
/* Channel X General Control and Status Register */
#define CH_GCSR             (0x024)
/* Channel X CRC Source Address (Most Significant Bits) */
#define CH_CRC_ADDR_M       (0x028)
/* Channel X CRC Source Address (Least Significant Bits) */
#define CH_CRC_ADDR_L       (0x02C)
/* Channel X CRC Value */
#define CH_CRC              (0x030)
/* Channel X Current Descriptor Address (Most Significant Bits) */
#define CH_CD_ADDR_M        (0x034)
/* Channel X Current Descriptor Address (Least Significant Bits) */
#define CH_CD_ADDR_L        (0x038)
/* Channel X Current Descriptor Transfer Control */
#define CH_CD_TCR           (0x03C)

/*
 * Source Address Pairs (Most and Least Significant) for blocks 1 - 15
 */
#define CH_SRC1_ADDR_M      (0x040)
#define CH_SRC1_ADDR_L      (0x044)
#define CH_SRC2_ADDR_M      (0x048)
#define CH_SRC2_ADDR_L      (0x04C)
#define CH_SRC3_ADDR_M      (0x050)
#define CH_SRC3_ADDR_L      (0x054)
#define CH_SRC4_ADDR_M      (0x058)
#define CH_SRC4_ADDR_L      (0x05C)
#define CH_SRC5_ADDR_M      (0x060)
#define CH_SRC5_ADDR_L      (0x064)
#define CH_SRC6_ADDR_M      (0x068)
#define CH_SRC6_ADDR_L      (0x06C)
#define CH_SRC7_ADDR_M      (0x070)
#define CH_SRC7_ADDR_L      (0x074)
#define CH_SRC8_ADDR_M      (0x078)
#define CH_SRC8_ADDR_L      (0x07C)
#define CH_SRC9_ADDR_M      (0x080)
#define CH_SRC9_ADDR_L      (0x084)
#define CH_SRC10_ADDR_M     (0x088)
#define CH_SRC10_ADDR_L     (0x08C)
#define CH_SRC11_ADDR_M     (0x090)
#define CH_SRC11_ADDR_L     (0x094)
#define CH_SRC12_ADDR_M     (0x098)
#define CH_SRC12_ADDR_L     (0x09C)
#define CH_SRC13_ADDR_M     (0x0A0)
#define CH_SRC13_ADDR_L     (0x0A4)
#define CH_SRC14_ADDR_M     (0x0A8)
#define CH_SRC14_ADDR_L     (0x0AC)
#define CH_SRC15_ADDR_M     (0x0B0)
#define CH_SRC15_ADDR_L     (0x0B4)


/*----------------------------------------------------------------------------
 * BIT-FIELDS DEFINITIONS FOR DMA/XOR REGISTERS
 *--------------------------------------------------------------------------*/
#define MAX_DMA_SIZE        (16 * 0x100000)  // 16 Meg

/* Bit-fields definitions for the
 * DMA Channel Transfer Control Register 1
 */

#define TCR1_XOR_WR_EN      0x80000000
#define TCR1_CRC_EN         0x40000000
#define TCR1_CRC_PORT       0x3F000000
#define TCR1_CRC_WR_EN      0x00800000
#define TCR1_READ_CRC_SEED  0x00400000
#define TCR1_SRC_PORT       0x003F0000
#define TCR1_DST_PORT       0x00003F00
#define TCR1_DIRECT_FILL    0x00000020
#define TCR1_DATA_BLOCKS    0x0000000F
#define TCR1_RESERVED       0x0000C0D0

/* Bit-fields definitions for the
 * DMA Channel Transfer Control Register 2
 */

#define TCR2_CRC_ORIENT     0x80000000
#define TCR2_CRC_SWAP       0x30000000
#define TCR2_BURST_SIZE     0x0C000000
#define TCR2_SWAP           0x03000000
#define TCR2_BC             0x00FFFFFF
#define TCR2_RESERVED       0x40000000

/* Bit-fields definitions for the
 * DMA Channel Next Descriptor Address (Least Significant)
 */

#define ND_ADDR_LB          0xFFFFFFC0
#define ND_ADDR_LAST        0x00000001
#define ND_ADDR_RESERVED    0x0000003E

/* Bit-fields definitions for the
 * DMA Channel Next Descriptor Control Register
 */

#define ND_TCR_ND_SWAP      0xC0000000
#define ND_TCR_ND_PORT      0x3F000000
#define ND_TCR_RESERVED     0x00FFFFFF

/* Bit-fields definitions for the
 * DMA Channel General Control and Status Register
 */

#define GCSR_GO             0x80000000
#define GCSR_CHAIN          0x40000000
#define GCSR_OP_CMD         0x30000000
#define GCSR_STOP_REQ       0x04000000
#define GCSR_HALT_REQ       0x02000000
#define GCSR_SOFT_RST       0x01000000
#define GCSR_DACT           0x00800000
#define GCSR_ERR_CODE       0x0000F000
#define GCSR_RESUME         0x00000800
#define GCSR_STOP           0x00000400
#define GCSR_HALT           0x00000200
#define GCSR_DONE           0x00000100
#define GCSR_EX_PKT_ERR     0x00000080
#define GCSR_PERR           0x00000040
#define GCSR_PAR_EN         0x00000020
#define GCSR_ERR_EN         0x00000010
#define GCSR_EX_PKT_EN      0x00000008
#define GCSR_STOP_EN        0x00000004
#define GCSR_HALT_EN        0x00000002
#define GCSR_DONE_EN        0x00000001
#define GCSR_RESERVED       0x087F0000
#define GCSR_ALL_INT_EN     (GCSR_PAR_EN | GCSR_ERR_EN | GCSR_EX_PKT_EN | \
                                GCSR_STOP_EN | GCSR_HALT_EN | GCSR_DONE_EN)
#  ifndef __ASSEMBLER__

/* Operation command codes for the DMA/XOR engine */
typedef enum {CMD_DMA, CMD_XOR, CMD_MEM_FILL, CMD_PARITY_CHECK} DMA_OP_CMD;
#  endif // __ASSEMBLER__


/* Bit-fields definitions for the
 * DMA Channel Current Descriptor Address Register (Least Significant)
 */

#define CD_ADDR_LB          0xFFFFFFC0

/* Bit-fields definitions for the
 * DMA Channel Current Descriptor Control Register
 */

#define CD_TCR_CD_SWAP      0xC0000000
#define CD_TCR_CD_PORT      0x3F000000
#define CD_TCR_RESERVED     0x00FFFFFF


/*----------------------------------------------------------------------------
 * STRUCTURES and TYPE DEFINITIONS
 *--------------------------------------------------------------------------*/

/* Type definition of a DMA descriptor used for Linked List DMA operations.
 * It has to be 64-bytes aligned.
 */
#  ifndef __ASSEMBLER__

typedef struct _DMA_DESCRIPTOR
{
    UINT32 src_addr_l;
    UINT32 src_addr_m;
    UINT32 dst_addr_l;
    UINT32 dst_addr_m;
    UINT32 tcr1;
    UINT32 tcr2;
    UINT32 nd_addr_l;
    UINT32 nd_addr_m;
    UINT32 nd_tcr;
} DMA_DESCRIPTOR __attribute__ ((aligned(64)));

typedef DMA_DESCRIPTOR *PDMA_DESCRIPTOR; // pointer to the DMA descriptor


/* Type definition of a XOR descriptor used for Linked List XOR operations.
 * It has to be 256-bytes aligned.
 */

typedef struct _XOR_DESCRIPTOR
{
    UINT32 src_addr_l;
    UINT32 src_addr_m;
    UINT32 dst_addr_l;
    UINT32 dst_addr_m;
    UINT32 tcr1;
    UINT32 tcr2;
    UINT32 nd_addr_l;
    UINT32 nd_addr_m;
    UINT32 nd_tcr;
    UINT32 src1_addr_l;
    UINT32 src1_addr_m;
    UINT32 src2_addr_l;
    UINT32 src2_addr_m;
    UINT32 src3_addr_l;
    UINT32 src3_addr_m;
    UINT32 src4_addr_l;
    UINT32 src4_addr_m;
    UINT32 src5_addr_l;
    UINT32 src5_addr_m;
    UINT32 src6_addr_l;
    UINT32 src6_addr_m;
    UINT32 src7_addr_l;
    UINT32 src7_addr_m;
    UINT32 src8_addr_l;
    UINT32 src8_addr_m;
    UINT32 src9_addr_l;
    UINT32 src9_addr_m;
    UINT32 src10_addr_l;
    UINT32 src10_addr_m;
    UINT32 src11_addr_l;
    UINT32 src11_addr_m;
    UINT32 src12_addr_l;
    UINT32 src12_addr_m;
    UINT32 src13_addr_l;
    UINT32 src13_addr_m;
    UINT32 src14_addr_l;
    UINT32 src14_addr_m;
    UINT32 src15_addr_l;
    UINT32 src15_addr_m;

} XOR_DESCRIPTOR __attribute__ ((aligned(256)));

typedef XOR_DESCRIPTOR *PXOR_DESCRIPTOR; // pointer to the XOR descriptor


/*----------------------------------------------------------------------------
 * MACROS
 *--------------------------------------------------------------------------*/

#define GET_DMA_STATUS(dma_ch_num) \
            (*(UINT32*)(DMA_REG_BASE + (0x100 * dma_ch_num) + CH_GCSR))

#define SET_DMA_GCSR(dma_ch_num, value) \
            (*(UINT32*)(DMA_REG_BASE + (0x100 * dma_ch_num) + CH_GCSR) = value)

#  endif // __ASSEMBLER__


#endif  // _TS_DMA_H
