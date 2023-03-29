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

/*-----------------------------------------------------------------------------
* FILENAME: 1.	SpacEthHardwareDefs.h
*
* DESCRIPTION: Contains the SPac GigE registers offsets, bitfield masks, 
*              predefined constants.
*
*              register offsets are calculated from the block's base address. 
*              Indirect register access requires specification of an index.
*
*              Bitfield masks are defined to match the descriptions in the specs. 
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added TX/RX mask definitions. 
*----------------------------------------------------------------------------*/

#ifndef _SPAC_ETH_HW_DEFS_H_
#define _SPAC_ETH_HW_DEFS_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

/* Begins user-configurable section. 
*************************************/

/* Discriminates which PHY device definitions are used used. */
/*  #define ETH_PHY_BCM5421  */
#define ETH_PHY_MARV88E

/* PHY addresses, as seen by the MII interface. */
#ifdef ETH_PHY_BCM5421
/* Start of BCM5421-specific code */ 

  #define ETH_PHY0_ADDR                     (0x10)
  #define ETH_PHY_ADDR_LOW                  (1)

/* End of BCM5421-specific code */ 

#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E
/* Start of MARVELL88E-specific code */ 
  #define ETH_PHY0_ADDR                     (0x00)
  #define ETH_PHY_ADDR_LOW                  (ETH_PHY0_ADDR)

/* End of MARVELL88E-specific code */ 
  
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
  
#endif 

#define ETH_PHY1_ADDR                     (0x01) 
#define ETH_PHY_ADDR_HIGH                 (ETH_TBI_PHY_ADDR)

#define ETH_PHY_REG_IDX_LOW               (0x00)
#define ETH_PHY_REG_IDX_HIGH              (0x1C)


/* Ends user-configurable section. 
*************************************/

/* helper macros */
#define REG32_PTR(a)                      ((VUINT32 *)(a))
#define REG32_VAL(a)                      ((UINT32)(a))

/* Xscale speed (in Hz). */
#define ETH_XSCALE_CLK                    (400000000)

/* Xscale cache line size. */
#define ETH_XSCALE_CACHE_LINE_LEN (32)  

/* Number of nanoseconds to wait before checking the MII interface status. */
#define ETH_MII_IF_STATUS_WAIT            (1000000)

/* Number of nanoseconds to wait before checking the MII status when doing 
   "scan" cycle. */
#define ETH_MII_SCAN_STATUS_WAIT          (1000000000)

/* Number of nanoseconds to wait before checking the link status. Refer to
   the IEEE specification for Auto-neg in clause 36 and clause 28. */
#define ETH_MII_LINK_WAIT                 (5000000000)

/* GigE low base address, as an offset from the chip base address. */
#define ETH_BASE_ADDR_LO                  (0x6000)

/* GigE high base address, as an offset from the chip base address. */
#define ETH_BASE_ADDR_HI                  (0x6fff)

#ifdef ETH_PHY_MARV88E

/* GPIO low base address, as an offset from the chip base address. */
  #define ETH_GPIO_BASE_ADDR_LO           (0x7500)

/* GPIO high base address, as an offset from the chip base address. */
  #define ETH_GPIO_BASE_ADDR_HI           (0x7Bff)

#endif

/* Descriptor HI address  mask. Only 12 bits are significant. */
#define ETH_DESCR_ADDR_HI_MASK            (0xfff) 

/* Descriptor size. */
#define ETH_DESCR_SIZE                     (sizeof(EthBuffDescr_t))

/* Number of GigE ports. */
#define ETH_NUM_PORTS                     (2)

/* Number of port directions. RX/TX. */
#define ETH_NUM_PORT_DIRECTIONS (2)

/* Port1 offset from Port0 base address. */
#define ETH_PORT_OFFSET                   (0x400)

/* Number of TX queues. */
#define ETH_NUM_TX_Q                      (4)

/* TX Queue (0, 1, 2, 3) Registers are offset by 0x10 from the precedent
 * queue register set. */
#define ETH_TX_Q_REG_OFFSET               (0x10)

/* RX Queue (0, 1, 2, 3) Registers are offset by 0x10 from the precedent
 * queue register set. */
#define ETH_RX_Q_REG_OFFSET               (0x10)

/* Number of RX queues. */
#define ETH_NUM_RX_Q                      (4)

/* Number of RX 32-bit words in the unicast+multicast hash tables. */
#define ETH_NUM_RX_HASH_WORDS             (32)

/* Up to 32 frames can be oustanding in the TX FIFO. */
#define ETH_TX_NUM_FIFO_FRAMES            (32)

/* TBI PHY address, as seen by the MII interface. */
#define ETH_TBI_PHY_ADDR                  (0x1E)

/* Descriptor "last" bit field. */
#define ETH_DESCR_LAST                    (1 << 31)

/* Up to 32 interrupt sources can be signaled through the GigE 
   ETH_PORT_INT_STATUS_REG register. */
#define ETH_MAX_NUM_INTR_SOURCES          (32)

/* CLZ returns 32 if no bits are set in the source register. Refer to  
   ARM Architecture Reference Manual. */
#define ETH_ARM_CLZ_BIT_NOT_SET           (32)

/* RX/TX queue interrupt source bit limits. */
#define ETH_RX_Q_INTR_STATUS_LO           (16)
#define ETH_RX_Q_INTR_STATUS_HI           (19)
#define ETH_TX_Q_INTR_STATUS_LO           (0)
#define ETH_TX_Q_INTR_STATUS_HI           (3)

/* Byte width. */
#define ETH_BYTE_WIDTH                    (8)

/* OCN port ids. */
#define ETH_OCN_PCI1_ID                   (1)
#define ETH_OCN_PCI2_ID                   (2)
#define ETH_OCN_XSCALE_ID                 (3) /* SRAM id, also. */
#define ETH_OCN_SDRAM_ID                  (4)
#define ETH_OCN_GIGE_ID                   (6)

/* memory alignment. */
#define ETH_MEM_ALIGN (8)

/* MAC header length. */
#define ETH_MAC_HEADER_LEN (14)
/* MAC address length. */
#define ETH_MAC_ADDR_LEN (6)
/* type/length length. */
#define ETH_MAC_TYPE_LEN (2)
/* FCS address length. */
#define ETH_MAC_FCS_LEN (4)

/******************************************************************************
 * MAC Registers. 
 */
#define ETH_MAC_CFG1_REG                  (ETH_BASE_ADDR_LO + 0x000) 
#define ETH_MAC_CFG2_REG                  (ETH_BASE_ADDR_LO + 0x004) 
#define ETH_MAC_IPG_IFG_REG               (ETH_BASE_ADDR_LO + 0x008) 
#define ETH_MAC_HALF_DUPLEX_REG           (ETH_BASE_ADDR_LO + 0x00C) 
#define ETH_MAC_MAX_FRAME_REG             (ETH_BASE_ADDR_LO + 0x010) 
#define ETH_MAC_TEST_REG                  (ETH_BASE_ADDR_LO + 0x01C) 
#define ETH_MAC_MII_CFG_REG               (ETH_BASE_ADDR_LO + 0x020) 
#define ETH_MAC_MII_CMD_REG               (ETH_BASE_ADDR_LO + 0x024) 
#define ETH_MAC_MII_ADDR_REG              (ETH_BASE_ADDR_LO + 0x028) 
#define ETH_MAC_MII_CTRL_REG              (ETH_BASE_ADDR_LO + 0x02C) 
#define ETH_MAC_MII_STATUS_REG            (ETH_BASE_ADDR_LO + 0x030) 
#define ETH_MAC_MII_INDIC_REG             (ETH_BASE_ADDR_LO + 0x034) 
#define ETH_MAC_IF_CTRL_REG               (ETH_BASE_ADDR_LO + 0x038) 
#define ETH_MAC_IF_STATUS_REG             (ETH_BASE_ADDR_LO + 0x03C) 
#define ETH_MAC_ADDR_PART1_REG            (ETH_BASE_ADDR_LO + 0x040) 
#define ETH_MAC_ADDR_PART2_REG            (ETH_BASE_ADDR_LO + 0x044) 

/******************************************************************************
 * MSTAT Registers. 
 */
#define ETH_MSTAT_TX64_REG                (ETH_BASE_ADDR_LO + 0x050) 
#define ETH_MSTAT_TX127_REG               (ETH_BASE_ADDR_LO + 0x054) 
#define ETH_MSTAT_TX255_REG               (ETH_BASE_ADDR_LO + 0x058) 
#define ETH_MSTAT_TX511_REG               (ETH_BASE_ADDR_LO + 0x05C) 
#define ETH_MSTAT_TX1K_REG                (ETH_BASE_ADDR_LO + 0x060) 
#define ETH_MSTAT_TXMAX_REG               (ETH_BASE_ADDR_LO + 0x064) 
#define ETH_MSTAT_TXMGV_REG               (ETH_BASE_ADDR_LO + 0x068) 
#define ETH_MSTAT_RXBYTE_REG              (ETH_BASE_ADDR_LO + 0x06C) 
#define ETH_MSTAT_RXPKT_REG               (ETH_BASE_ADDR_LO + 0x070) 
#define ETH_MSTAT_RXFCS_REG               (ETH_BASE_ADDR_LO + 0x074) 
#define ETH_MSTAT_RXMCA_REG               (ETH_BASE_ADDR_LO + 0x078) 
#define ETH_MSTAT_RXBCA_REG               (ETH_BASE_ADDR_LO + 0x07C) 
#define ETH_MSTAT_RXCF_REG                (ETH_BASE_ADDR_LO + 0x080) 
#define ETH_MSTAT_RXPF_REG                (ETH_BASE_ADDR_LO + 0x084) 
#define ETH_MSTAT_RXUO_REG                (ETH_BASE_ADDR_LO + 0x088) 
#define ETH_MSTAT_RXALN_REG               (ETH_BASE_ADDR_LO + 0x08C) 
#define ETH_MSTAT_RXFLR_REG               (ETH_BASE_ADDR_LO + 0x090) 
#define ETH_MSTAT_RXCDE_REG               (ETH_BASE_ADDR_LO + 0x094) 
#define ETH_MSTAT_RXCSE_REG               (ETH_BASE_ADDR_LO + 0x098) 
#define ETH_MSTAT_RXUND_REG               (ETH_BASE_ADDR_LO + 0x09C) 
#define ETH_MSTAT_RXOVR_REG               (ETH_BASE_ADDR_LO + 0x0A0) 
#define ETH_MSTAT_RXFRG_REG               (ETH_BASE_ADDR_LO + 0x0A4) 
#define ETH_MSTAT_RXJBR_REG               (ETH_BASE_ADDR_LO + 0x0A8) 
#define ETH_MSTAT_RXDRP_REG               (ETH_BASE_ADDR_LO + 0x0AC) 
#define ETH_MSTAT_TXBYTE_REG              (ETH_BASE_ADDR_LO + 0x0B0) 
#define ETH_MSTAT_TXPKT_REG               (ETH_BASE_ADDR_LO + 0x0B4) 
#define ETH_MSTAT_TXMCA_REG               (ETH_BASE_ADDR_LO + 0x0B8) 
#define ETH_MSTAT_TXBCA_REG               (ETH_BASE_ADDR_LO + 0x0BC) 
#define ETH_MSTAT_TXPF_REG                (ETH_BASE_ADDR_LO + 0x0C0) 
#define ETH_MSTAT_TXDFR_REG               (ETH_BASE_ADDR_LO + 0x0C4) 
#define ETH_MSTAT_TXEDF_REG               (ETH_BASE_ADDR_LO + 0x0C8) 
#define ETH_MSTAT_TXSCL_REG               (ETH_BASE_ADDR_LO + 0x0CC) 
#define ETH_MSTAT_TXMCL_REG               (ETH_BASE_ADDR_LO + 0x0D0) 
#define ETH_MSTAT_TXLCL_REG               (ETH_BASE_ADDR_LO + 0x0D4) 
#define ETH_MSTAT_TXXCL_REG               (ETH_BASE_ADDR_LO + 0x0D8) 
#define ETH_MSTAT_TXNCL_REG               (ETH_BASE_ADDR_LO + 0x0DC) 
#define ETH_MSTAT_TXPFH_REG               (ETH_BASE_ADDR_LO + 0x0E0) 
#define ETH_MSTAT_TXDRP_REG               (ETH_BASE_ADDR_LO + 0x0E4) 
#define ETH_MSTAT_TXJBR_REG               (ETH_BASE_ADDR_LO + 0x0E8) 
#define ETH_MSTAT_TXFCS_REG               (ETH_BASE_ADDR_LO + 0x0EC) 
#define ETH_MSTAT_TXCF_REG                (ETH_BASE_ADDR_LO + 0x0F0) 
#define ETH_MSTAT_TXOVR_REG               (ETH_BASE_ADDR_LO + 0x0F4) 
#define ETH_MSTAT_TXUND_REG               (ETH_BASE_ADDR_LO + 0x0F8) 
#define ETH_MSTAT_TXFRG_REG               (ETH_BASE_ADDR_LO + 0x0FC) 
#define ETH_MSTAT_CAR1_REG                (ETH_BASE_ADDR_LO + 0x100) 
#define ETH_MSTAT_CAR2_REG                (ETH_BASE_ADDR_LO + 0x104) 
#define ETH_MSTAT_CARM1_REG               (ETH_BASE_ADDR_LO + 0x108) 
#define ETH_MSTAT_CARM2_REG               (ETH_BASE_ADDR_LO + 0x10C) 

/******************************************************************************
 * PORT Registers. 
 */
#define ETH_PORT_CTRL_REG                 (ETH_BASE_ADDR_LO + 0x200) 
#define ETH_PORT_INT_STATUS_REG           (ETH_BASE_ADDR_LO + 0x204) 
#define ETH_PORT_INT_MASK_REG             (ETH_BASE_ADDR_LO + 0x208) 
#define ETH_PORT_OCN_STATUS_REG           (ETH_BASE_ADDR_LO + 0x20C)   
  
/******************************************************************************
 * TX Registers. 
 */  
#define ETH_TX_CFG_REG                    (ETH_BASE_ADDR_LO + 0x220) 
#define ETH_TX_CTRL_REG                   (ETH_BASE_ADDR_LO + 0x224) 
#define ETH_TX_STATUS_REG                 (ETH_BASE_ADDR_LO + 0x228) 
#define ETH_TX_XSTATUS_REG                (ETH_BASE_ADDR_LO + 0x22C) 
#define ETH_TX_THRESH_REG                 (ETH_BASE_ADDR_LO + 0x230) 
#define ETH_TX_PRI_THRESH_REG             (ETH_BASE_ADDR_LO + 0x234) 
#define ETH_TX_Q_PRI_MAP_REG              (ETH_BASE_ADDR_LO + 0x240) 
#define ETH_TX_PEND_COMP_REG              (ETH_BASE_ADDR_LO + 0x248)
#define ETH_TX_PAUSE_CNT_REG              (ETH_BASE_ADDR_LO + 0x250)
#define ETH_TX_GVLAN_TAG_REG              (ETH_BASE_ADDR_LO + 0x254)
#define ETH_TX_DIAG_ADDR_REG              (ETH_BASE_ADDR_LO + 0x270)
#define ETH_TX_DIAG_DATA_REG              (ETH_BASE_ADDR_LO + 0x274)
#define ETH_TX_ERR_STATUS_REG             (ETH_BASE_ADDR_LO + 0x278)
/******************************************************************************
 * TX Queue (0, 1, 2, 3) Registers are offset by 0x10 from the precedent
 * queue register set.
 */
#define ETH_TX_Q0_CFG_REG                 (ETH_BASE_ADDR_LO + 0x280)
#define ETH_TX_Q0_BUFF_CFG_REG            (ETH_BASE_ADDR_LO + 0x284)
#define ETH_TX_Q0_PTR_LO_REG              (ETH_BASE_ADDR_LO + 0x288)
#define ETH_TX_Q0_PTR_HI_REG              (ETH_BASE_ADDR_LO + 0x28C)

/******************************************************************************
 * RX Registers. 
 */
#define ETH_RX_CFG_REG                    (ETH_BASE_ADDR_LO + 0x320) 
#define ETH_RX_CTRL_REG                   (ETH_BASE_ADDR_LO + 0x324) 
#define ETH_RX_STATUS_REG                 (ETH_BASE_ADDR_LO + 0x328) 
#define ETH_RX_XSTATUS_REG                (ETH_BASE_ADDR_LO + 0x32C) 
#define ETH_RX_PAUSE_THRESH_REG           (ETH_BASE_ADDR_LO + 0x330) 
#define ETH_RX_THRESH_REG                 (ETH_BASE_ADDR_LO + 0x334) 
#define ETH_RX_VLAN_TAG_MAP_REG           (ETH_BASE_ADDR_LO + 0x340)
#define ETH_RX_HASH_ADDR_REG              (ETH_BASE_ADDR_LO + 0x360)
#define ETH_RX_HASH_DATA_REG              (ETH_BASE_ADDR_LO + 0x364)
#define ETH_RX_DIAG_ADDR_REG              (ETH_BASE_ADDR_LO + 0x370)
#define ETH_RX_DIAG_DATA_REG              (ETH_BASE_ADDR_LO + 0x374)
#define ETH_RX_ERR_STATUS_REG             (ETH_BASE_ADDR_LO + 0x378)

/******************************************************************************
 * RX Queue (0, 1, 2, 3) Registers are offset by 0x10 from the precedent
 * queue register set.
 */
#define ETH_RX_Q0_CFG_REG                 (ETH_BASE_ADDR_LO + 0x380)
#define ETH_RX_Q0_BUFF_CFG_REG            (ETH_BASE_ADDR_LO + 0x384)
#define ETH_RX_Q0_PTR_LO_REG              (ETH_BASE_ADDR_LO + 0x388)
#define ETH_RX_Q0_PTR_HI_REG              (ETH_BASE_ADDR_LO + 0x38C)

/******************************************************************************
 * TX/RX Diagnostic (Indirect Access) register Indexes.
 */
#define ETH_BUFF_ADDR0_REG_IDX            (0x0)
#define ETH_BUFF_ADDR1_REG_IDX            (0x1)
#define ETH_NEXT_ADDR0_REG_IDX            (0x2)
#define ETH_NEXT_ADDR1_REG_IDX            (0x3)
#define ETH_VLAN_BCOUNT_REG_IDX           (0x4)
#define ETH_STATUS_CFG_REG_IDX            (0x5)

/******************************************************************************
 * TX Cache Word0 and Word1 (Indirect Access) register Indexes. 
 * Even indexes (0x40-0x7E) are for Word0. Odd indexes (0x41-0x7F)
 * are for Word1.
 */
#define ETH_TX_CACHE1_WORD0_REG_IDX       (0x40)
#define ETH_TX_CACHE32_WORD0_REG_IDX      (0x7E)
#define ETH_TX_CACHE1_WORD1_REG_IDX       (0x41)
#define ETH_TX_CACHE32_WORD1_REG_IDX      (0x7F)

/******************************************************************************
 * RX Start Frame (0, 1, 2) (Indirect Access) register Indexes. 
 * These registers hold information about starting descriptor from the current
 * frame being received.
 */
#define ETH_RX_START_FRM0_REG_IDX         (0x8)
#define ETH_RX_START_FRM1_REG_IDX         (0x9)
#define ETH_RX_START_FRM2_REG_IDX         (0xA)

/******************************************************************************
 * TBI (Indirect Access) register Indexes.
 */
#define ETH_TBI_CTRL1_REG_IDX             (0x00)
#define ETH_TBI_STATUS_REG_IDX            (0x01)
#define ETH_TBI_AN_ADV_REG_IDX            (0x04)
#define ETH_TBI_AN_LP_BP_REG_IDX          (0x05)
#define ETH_TBI_AN_EXP_REG_IDX            (0x06)
#define ETH_TBI_AN_NP_TX_REG_IDX          (0x07)
#define ETH_TBI_AN_LP_ANP_REG_IDX         (0x08)
#define ETH_TBI_EXT_STATUS_REG_IDX        (0x0F)
#define ETH_TBI_JITT_DIAG_REG_IDX         (0x10)
#define ETH_TBI_CTRL2_REG_IDX             (0x11)

/* the first 15 PHY registers are standard. */

#define ETH_PHY_CTRL_REG_IDX              (0x00)
#define ETH_PHY_STATUS_REG_IDX            (0x01)
#define ETH_PHY_AN_ADV_REG_IDX            (0x04)
#define ETH_PHY_1000BASE_T_REG_IDX        (0x09)
#define ETH_PHY_EXT_STATUS_REG_IDX        (0x0F)


#ifdef ETH_PHY_BCM5421

/* Start of BCM5421-specific code */ 
/******************************************************************************
 * BCM 5421 PHY register addresses.
 */
  #define ETH_BCM_EXT_CTRL_REG_IDX        (0x10)
  #define ETH_BCM_SUM_STATUS_REG_IDX      (0x19)
  #define ETH_BCM_SPR_CTRL_REG_IDX        (0x1C)

/* End of BCM5421-specific code */ 

#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E
/* Start of MARVELL88E-specific code */ 

/******************************************************************************
 * MARVELL 88E1020(S) PHY register addresses.
 */
    #define ETH_MARV_EXT_CTRL1_REG_IDX    (0x10)
    #define ETH_MARV_SUM_STATUS_REG_IDX   (0x11)
    #define ETH_MARV_EXT_CTRL2_REG_IDX    (0x14)

/******************************************************************************
 * GPIO register addresses.
 */
    #define ETH_GPIO_DATA_REG             (ETH_GPIO_BASE_ADDR_LO + 0x5A0)
    #define ETH_GPIO_CTRL_REG             (ETH_GPIO_BASE_ADDR_LO + 0x5A4)
    
/* End of MARVELL88E-specific code */ 
  
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
  
#endif 

/******************************************************************************
 * MAC Register bit masks. 
 */
#define ETH_MAC_IF_CTRL_PHY_MASK          (1 << 24) 
#define ETH_MAC_RESET_MASK                (1 << 31)
#define ETH_MAC_MII_RESET_MASK            (1 << 31)                              

/******************************************************************************
 * MSTAT Register bit masks. 
 */
#define ETH_MSTAT_CAR1_RPKT_MASK          (1 << 15) 
#define ETH_MSTAT_CAR2_TPKT_MASK          (1 << 12)  

/******************************************************************************
 * PORT Register bit masks. 
 */
#define ETH_PORT_INT_STATUS_MASK          (1 << 31)
#define ETH_PORT_CTRL_RESET_MASK          (1 << 31)
 
/******************************************************************************
 * TX Register bit masks. 
 */
#define ETH_TX_Q_X_INTR_MASK              (0x00010101)
#define ETH_TX_DESCR_Q_X_INTR_MASK        (1 << 16)
#define ETH_TX_EOF_Q_X_INTR_MASK          (1 << 8)
#define ETH_TX_EOQ_Q_X_INTR_MASK          (1)
#define ETH_TX_Q0_INTR_MASK               (1)
#define ETH_TX_Q_WAIT_INTR_MASK           (1 << 8)
#define ETH_TX_Q0_ERR_MASK                (1 << 24)
#define ETH_TX_Q_ERR_MASK                 (0xF << 24)
#define ETH_TX_Q0_DESC_ERR_MASK           (1 << 7)
#define ETH_TX_Q0_TIMEOUT_ERR_MASK        (1 << 6)
#define ETH_TX_Q0_READ_ERR_MASK           (1 << 5)
#define ETH_TX_Q0_TEA_ERR_MASK            (1 << 4)
#define ETH_TX_Q0_TRESP_ERR_MASK          (0xF)
#define ETH_TX_ABORT_INTR_MASK            (1 << 30)
#define ETH_TX_ABORT_EN_INTR_MASK         (1 << 29)
#define ETH_TX_IDLE_EN_INTR_MASK          (1 << 31)
#define ETH_TX_BUFF_W_SWAP_MASK           (1 << 11)
#define ETH_TX_BUFF_B_SWAP_MASK           (1 << 10)
#define ETH_TX_Q_ENABLE_ALL_MASK          (0xF)
#define ETH_TX_Q_CFG_GVI_MASK             (1 << 15)
#define ETH_TX_RESET_MASK                 (1 << 31)
#define ETH_TX_CTRL_GO_MASK               (1 << 15)
#define ETH_TX_STATUS_ACTIVE_MASK         (1 << 15)
#define ETH_TX_DESCR_Q_INTR_EN_MASK       (1 << 20)
#define ETH_TX_EOF_Q_INTR_EN_MASK         (1 << 16)
#define ETH_TX_EOQ_Q_INTR_EN_MASK         (7 << 17)

/******************************************************************************
 * TX Queue (0, 1, 2, 3) Register bit masks.
 */

/******************************************************************************
 * RX Register bit masks. 
 */
#define ETH_RX_Q_X_INTR_MASK              (0x00010101)
#define ETH_RX_DESCR_Q_X_INTR_MASK        (1 << 16)
#define ETH_RX_EOF_Q_X_INTR_MASK          (1 << 8)
#define ETH_RX_EOQ_Q_X_INTR_MASK          (1)
#define ETH_RX_Q0_INTR_MASK               (1 << 16)
#define ETH_RX_Q_WAIT_INTR_MASK           (1 << 24)
#define ETH_RX_Q0_ERR_MASK                (1 << 24)
#define ETH_RX_Q_ERR_MASK                 (0xF << 24)
#define ETH_RX_Q0_DESC_ERR_MASK           (1 << 7)
#define ETH_RX_Q0_TIMEOUT_ERR_MASK        (1 << 6)
#define ETH_RX_Q0_READ_ERR_MASK           (1 << 5)
#define ETH_RX_Q0_TEA_ERR_MASK            (1 << 4)
#define ETH_RX_Q0_TRESP_ERR_MASK          (0xF)
#define ETH_RX_ABORT_INTR_MASK            (1 << 30)
#define ETH_RX_ABORT_EN_INTR_MASK         (1 << 29)
#define ETH_RX_IDLE_EN_INTR_MASK          (1 << 31)
#define ETH_RX_BUFF_W_SWAP_MASK           (1 << 11)
#define ETH_RX_BUFF_B_SWAP_MASK           (1 << 10)
#define ETH_RX_Q_ENABLE_ALL_MASK          (0xF)
#define ETH_RX_RESET_MASK                 (1 << 31)
#define ETH_RX_STATUS_ACTIVE_MASK         (1 << 15)
#define ETH_RX_DESCR_Q_INTR_EN_MASK       (1 << 20)
#define ETH_RX_EOF_Q_INTR_EN_MASK         (1 << 16)
#define ETH_RX_EOQ_Q_INTR_EN_MASK         (7 << 17)


/******************************************************************************
 * RX Queue (0, 1, 2, 3) Register bit masks.
 */


/******************************************************************************
 * TX descriptor bit masks. 
 */
 
/* Last descriptor. */
#define ETH_TX_DESCR_LAST                 (1 << 31)


/******************************************************************************
 * RX descriptor bit masks. 
 */
 
/* Last descriptor. */
#define ETH_RX_DESCR_LAST                 (1 << 31)


/******************************************************************************
 * TBI Register bit masks. 
 */
 
#define ETH_TBI_PHY_RESET_MASK            (1 << 15)
#define ETH_TBI_RESET_MASK                (1 << 15)


/******************************************************************************
 * PHY Register bit masks. 
 */

#define ETH_PHY_RESET_MASK                (1 << 15)

#ifdef ETH_PHY_BCM5421

/* Start of BCM5421-specific code */ 
 
  #define ETH_BCM_SPR_CTRL_REG_VAL        (0x9040)

/* End of BCM5421-specific code */ 

#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E
  /* Start of MARVELL88E-specific code */ 

/******************************************************************************
 * GPIO Register bit masks. 
 */
    #define ETH_GPIO_CTRL_REG_VAL        (0x01000001) /* Enable GPIO port, output mode. */
    #define ETH_MARV_SERDES              (0x0000ff00) /* SERDES. */
    #define ETH_MARV_TBI                 (0x0000ff00) /* TBI. */
    #define ETH_MARV_GMII                (0x0000ff00) /* G/MII. */

  /* End of MARVELL88E-specific code */ 
  
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
  
#endif 


#endif  /* _SPAC_ETH_HW_DEFS_H_ */

