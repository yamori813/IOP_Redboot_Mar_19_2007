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

/*****************************************************************************
 *****************************************************************************
 **
 **   Revision : $Revision: 1.2 $
 **   Author   : $Author: bwood $
 **   Date     : $Date: 2002/09/12 18:51:45 $
 **   Purpose  : This file contains the register definitions for the
 **              Core Interface Unit (CIU) used for StrongPac.
 **              This includes definitions for register offsets,
 **              register bit fields, and a register map type.
 **              It includes XScale, SRAM, and interfaces to OCN.
 **   History  :
 **   $Log: hal_ioc80314_ciu.h,v $
 **
 **
 **
 *****************************************************************************
 *****************************************************************************/
#include "hal_ioc80314_basic_types.h"

#ifndef _TS_CIU_H_
#define _TS_CIU_H_

/*******************************************************
 *******       Start of Tundra CIU           ***********
 *******************************************************/

/* Base Offset for start of SDRAM controller registers */
#define TS_CIU_REG_BASE (0x00010000)


/** This macro calculates the register address using the register offset*/
#define  TS_CIU_REG_ADDR(offset)        \
               (TS_CIU_REG_BASE + offset)

/******************************************************************************
 **                     CIU  Register Address offsets
 **/

/* CIU Timer 0 Register                               */
#define TS_CIU_TIMER0                (0x00000000)
/* CIU Timer 0 Register Shadow                        */
#define TS_CIU_TIMER0_SHADOW         (0x00000004)
/* CIU Timer 1 Register                               */
#define TS_CIU_TIMER1                (0x00000008)
/* CIU Timer 1 Register Shadow                        */
#define TS_CIU_TIMER1_SHADOW         (0x0000000C)
/* CIU SRAM Base Address Register                     */
#define TS_CIU_SRAM_BAR              (0x00000010)
/* CIU Internal Configuration Base Address Register   */
#define TS_CIU_CFG_BAR               (0x00000014)
/* CIU Switch Fabric Base Address Register 1          */
#define TS_CIU_SF_BAR1               (0x00000018)
/* CIU Switch Fabric Base Address Register 2          */
#define TS_CIU_SF_BAR2               (0x0000001C)
/* CIU Switch Fabric Base Address Register 3          */
#define TS_CIU_SF_BAR3               (0x00000020)
/* CIU Switch Fabric Base Address Register 4          */
#define TS_CIU_SF_BAR4               (0x00000024)
/* CIU Switch Fabric Sizes Register                   */
#define TS_CIU_SF_SIZES              (0x00000028)
/* CIU OCN Error Status Register                      */
#define TS_CIU_OCN_ERR_STATUS        (0x0000002C)
/* CIU Sram Status Register                           */
#define TS_CIU_SRAM_STATUS           (0x00000030)
/* CIU Request Queue Status Register                  */
#define TS_CIU_RQ_STATUS             (0x00000034)
/* CIU Configuration Register                         */
#define TS_CIU_CFG                   (0x00000038)
/* CIU ECC Status Register                            */
#define TS_CIU_ECC_STATUS            (0x0000003C)
/* CIU ECC Error Address                              */
#define TS_CIU_ECC_ERROR_AD          (0x00000040)
/* CIU XScale Arbiter Error Register                  */
#define TS_CIU_XARB_ERROR            (0x00000044)
/* CIU Error Register                                 */
#define TS_CIU_ERROR                 (0x00000048)
/* CIU Watch Dog Timer Register                       */
#define TS_CIU_WDTIMER               (0x0000004C)
/* CIU Watch Dog Timer Shadow0 Register               */
#define TS_CIU_WDTIMER_SHADOW0       (0x00000050)
/* CIU Watch Dog Timer Shadow1 Register               */
#define TS_CIU_WDTIMER_SHADOW1       (0x00000054)
/* CIU Watch Dog Timer Address Register               */
#define TS_CIU_WD_ADDRESS            (0x00000058)
/* CIU Length Error Address 0 Register                */
#define TS_CIU_LEN_ERROR_AD0         (0x0000005C)
/* CIU Length Error Address 1 Register                */
#define TS_CIU_LEN_ERROR_AD1         (0x00000060)
/* CIU Abort Address Register                         */
#define TS_CIU_ABORT_AD              (0x00000064)
/* CIU SRAM Byte Register                             */
#define TS_CIU_SRAM_BYTE             (0x00000068)

/* CIU Fabric Control and Status Register             */
#define TS_CIU_FAB_CSR               (0x00000200)
/* CIU Fabric Sync Request Base Address Register      */
#define TS_CIU_FAB_SYNC_BAR          (0x00000204)
/* CIU Switch Fabric BAR1 Look Up Table Registers      */
#define TS_CIU_SF_BAR1_LUT0          (0x00000300)
/* CIU Switch Fabric BAR1 Look Up Table Upper Register */
#define TS_CIU_SF_BAR1_LUT_UPPER0    (0x00000304)
/* CIU Switch Fabric BAR2 Look Up Table Registers      */
#define TS_CIU_SF_BAR2_LUT0          (0x00000400)
/* CIU Switch Fabric BAR2 Look Up Table Upper Register */
#define TS_CIU_SF_BAR2_LUT_UPPER0    (0x00000404)
/* CIU Switch Fabric BAR3 Look Up Table Registers      */
#define TS_CIU_SF_BAR3_LUT0          (0x00000500)
/* CIU Switch Fabric BAR3 Look Up Table Upper Register */
#define TS_CIU_SF_BAR3_LUT_UPPER0    (0x00000504)
/* CIU Switch Fabric BAR4 Look Up Table Registers      */
#define TS_CIU_SF_BAR4_LUT0          (0x00000600)
/* CIU Switch Fabric BAR4 Look Up Table Upper Register */
#define TS_CIU_SF_BAR4_LUT_UPPER0    (0x00000604)

/*
 * Define some register ranges
 */
#define TS_CIU_FIRST_REG_OFFSET     (TS_CIU_TIMER0)
#define TS_CIU_START_UNDEF_RANGE1   (TS_CIU_SRAM_BYTE+4)
#define TS_CIU_END_UNDEF_RANGE1     (TS_CIU_FAB_CSR-1)
#define TS_CIU_START_UNDEF_RANGE2   (TS_CIU_FAB_SYNC_BAR+4)
#define TS_CIU_END_UNDEF_RANGE2     (TS_CIU_SF_BAR1_LUT0-1)

/* CIU LAST valid register offset */
#define TS_CIU_LAST_REG_OFFSET       (0x000006FC)

/* Macros to compute the address of Look Up Table Registers */

#define BARx_LUTy_ADDR( a, b )   ((UINT32)(TS_CIU_SF_BAR1_LUT0) + \
                                  (UINT32)(0x100 * a) + (UINT32)(8 * b))
#define BARx_LUTy_U_ADDR( a, b ) ((UINT32)(TS_CIU_SF_BAR1_LUT_UPPER0) + \
		                          (UINT32)(0x100 * a) + (UINT32)(8 * b))

/*******************************************************
 *******        End of Tundra CIU            ***********
 *******************************************************/


/*****************************************************************************
 **                    Register Bit Field Enables & Masks                   **
 *****************************************************************************/


/******************************************************************************
 **               Tundra CIU Control Register Bit Fields
 **/

/* CIU Timer 0 Register                               */
#define TS_CIU_TIMER0_TIME_SLICE            (0xFFFFFFFF)
#define TS_CIU_TIMER0_UNDEF                 (0x00000000)

/* CIU Timer 0 Register Shadow                        */
#define TS_CIU_TIMER0_SHADOW_TIME_SLICE_CNT (0xFFFFFFFF)
#define TS_CIU_TIMER0_SHADOW_UNDEF          (0x00000000)

/* CIU Timer 1 Register                               */
#define TS_CIU_TIMER1_TIME_SLICE            (0xFFFFFFFF)
#define TS_CIU_TIMER1_UNDEF                 (0x00000000)

/* CIU Timer 1 Register Shadow                        */
#define TS_CIU_TIMER1_SHADOW_TIME_SLICE_CNT (0xFFFFFFFF)
#define TS_CIU_TIMER1_SHADOW_UNDEF          (0x00000000)

/* CIU SRAM Base Address Register                     */
#define TS_CIU_SRAM_BAR_BASE_ADDRESS        (0xFFF00000)
#define TS_CIU_SRAM_BAR_EN                  (0x00000001)
#define TS_CIU_SRAM_BAR_UNDEF               (0x00000000)

/* CIU Internal Configuration Base Address Register   */
#define TS_CIU_CFG_BAR_BASE_ADDRESS         (0xFFFF0000)
#define TS_CIU_CFG_BAR_EN                   (0x00000001)
#define TS_CIU_CFG_BAR_UNDEF                (0x0000FFFE)

/* CIU Switch Fabric Base Address Register 1          */
/* CIU Switch Fabric Base Address Register 2          */
/* CIU Switch Fabric Base Address Register 3          */
/* CIU Switch Fabric Base Address Register 4          */
#define TS_CIU_SF_BARx_BASE_ADDRESS         (0xFFFF0000)
#define TS_CIU_SF_BARx_UNDEF                (0x0000FFFF)

/* CIU Switch Fabric Sizes Register                   */
#define TS_CIU_SF_SIZES_SF_BAR1_SIZE        (0xF8000000)
#define TS_CIU_SF_SIZES_PASS1               (0x02000000)
#define TS_CIU_SF_SIZES_EN1                 (0x01000000)
#define TS_CIU_SF_SIZES_SF_BAR2_SIZE        (0x00F80000)
#define TS_CIU_SF_SIZES_PASS2               (0x00020000)
#define TS_CIU_SF_SIZES_EN2                 (0x00010000)
#define TS_CIU_SF_SIZES_SF_BAR3_SIZE        (0x0000F800)
#define TS_CIU_SF_SIZES_PASS3               (0x00000200)
#define TS_CIU_SF_SIZES_EN3                 (0x00000100)
#define TS_CIU_SF_SIZES_SF_BAR4_SIZE        (0x000000F8)
#define TS_CIU_SF_SIZES_PASS4               (0x00000002)
#define TS_CIU_SF_SIZES_EN4                 (0x00000001)
#define TS_CIU_SF_SIZES_UNDEF               (0x04040404)

#define TS_CIU_SF_PASSx_LUT_INDEX 0x0
#define TS_CIU_SF_PASSx_PORT_DEST 0x1

/* CIU OCN Error Status Register                      */
#define TS_CIU_OCN_ERR_STATUS_DEF           (0xFFFFFFFF)
#define TS_CIU_OCN_ERR_STATUS_UNDEF         (0x00000000)

/* CIU Sram Status Register                           */
#define TS_CIU_SRAM_STATUS_PARITY_BYTE      (0xFF000000)
#define TS_CIU_SRAM_STATUS_ADDRESS          (0x00FFFFF0)
#define TS_CIU_SRAM_STATUS_CMD              (0x0000000C)
#define TS_CIU_SRAM_STATUS_DCNT             (0x00000003)
#define TS_CIU_SRAM_STATUS_UNDEF            (0x00000000)

#define TS_CIU_SRAM_CMD_NO_REQ  0x0
#define TS_CIU_SRAM_CMD_WR_REQ  0x1
#define TS_CIU_SRAM_CMD_RD_REQ  0x2
#define TS_CIU_SRAM_CMD_RSVD    0x3

/* CIU Request Queue Status Register                  */
#define TS_CIU_RQ_STATUS_TS1                (0x01000000)
#define TS_CIU_RQ_STATUS_Q1_W3              (0x00800000)
#define TS_CIU_RQ_STATUS_Q1_W2              (0x00400000)
#define TS_CIU_RQ_STATUS_Q1_W1              (0x00200000)
#define TS_CIU_RQ_STATUS_Q1_W0              (0x00100000)
#define TS_CIU_RQ_STATUS_REQ1_CURR_STATE    (0x000F0000)
#define TS_CIU_RQ_STATUS_TS0                (0x01000000)
#define TS_CIU_RQ_STATUS_Q0_W3              (0x00800000)
#define TS_CIU_RQ_STATUS_Q0_W2              (0x00400000)
#define TS_CIU_RQ_STATUS_Q0_W1              (0x00200000)
#define TS_CIU_RQ_STATUS_Q0_W0              (0x00100000)
#define TS_CIU_RQ_STATUS_REQ0_CURR_STATE    (0x000F0000)
#define TS_CIU_RQ_STATUS_UNDEF              (0xFE00FE00)

/* Constant definitions for REQx_CURR_STATE meanings */
#define REQ_STATE_EMPTY  0x0
#define REQ_STATE_HALF_1 0x1
#define REQ_STATE_FULL_1 0x2
#define REQ_STATE_HALF_2 0x3
#define REQ_STATE_FULL_2 0x4
#define REQ_STATE_HALF_3 0x5
#define REQ_STATE_FULL_3 0x6
#define REQ_STATE_HALF_4 0x7
#define REQ_STATE_FULL   0x8
#define REQ_STATE_BAD_9  0x9
#define REQ_STATE_BAD_A  0xA
#define REQ_STATE_BAD_B  0xB
#define REQ_STATE_BAD_C  0xC
#define REQ_STATE_BAD_D  0xD
#define REQ_STATE_BAD_E  0xE
#define REQ_STATE_BAD_F  0xF

/* CIU Configuration Register                         */
#define TS_CIU_CFG_PID                      (0x00000100)
#define TS_CIU_CFG_SW_RST                   (0x00000020)
#define TS_CIU_CFG_PTST_EN                  (0x00000010)
#define TS_CIU_CFG_WD_EN                    (0x00000008)
#define TS_CIU_CFG_PAR_EN                   (0x00000004)
#define TS_CIU_CFG_ECC_EN                   (0x00000002)
#define TS_CIU_CFG_ARB_EN                   (0x00000001)
#define TS_CIU_CFG_UNDEF                    (0xFFFFFEC0)

/* CIU ECC Status Register                            */
#define TS_CIU_ECC_STATUS_CORRECTED_DCB     (0xFF000000)
#define TS_CIU_ECC_STATUS_ORIGINAL_DCB      (0x00FF0000)
#define TS_CIU_ECC_STATUS_ECC_SYNDROME      (0x0000FF00)
#define TS_CIU_ECC_STATUS_CERR              (0x00000002)
#define TS_CIU_ECC_STATUS_UERR              (0x00000001)
#define TS_CIU_ECC_STATUS_UNDEF             (0x000000FC)

/* CIU ECC Error Address                              */
#define TS_CIU_ECC_ERROR_AD_DEF             (0xFFFFFFFF)
#define TS_CIU_ECC_ERROR_AD_UNDEF           (0x00000000)

/* CIU XScale Arbiter Error Register                  */
#define TS_CIU_XARB_ERROR_TS1               (0x00000020)
#define TS_CIU_XARB_ERROR_TS0               (0x00000010)
#define TS_CIU_XARB_ERROR_P1AH2L            (0x00000008)
#define TS_CIU_XARB_ERROR_P1AL2H            (0x00000004)
#define TS_CIU_XARB_ERROR_P0AH2L            (0x00000002)
#define TS_CIU_XARB_ERROR_P0AL2H            (0x00000001)
#define TS_CIU_XARB_ERROR_UNDEF             (0xFFFFFFC0)

/* CIU Error Register                                 */
#define TS_CIU_ERROR_MECC                   (0x00000400)
#define TS_CIU_ERROR_OCN_INT                (0x00000200)
#define TS_CIU_ERROR_CECC                   (0x00000100)
#define TS_CIU_ERROR_UECC                   (0x00000080)
#define TS_CIU_ERROR_XARB                   (0x00000040)
#define TS_CIU_ERROR_PARITY                 (0x00000020)
#define TS_CIU_ERROR_RQ1                    (0x00000010)
#define TS_CIU_ERROR_RQ0                    (0x00000008)
#define TS_CIU_ERROR_RSP                    (0x00000004)
#define TS_CIU_ERROR_WD                     (0x00000002)
#define TS_CIU_ERROR_ABORT                  (0x00000001)
#define TS_CIU_ERROR_UNDEF                  (0xFFFFF800)

/* CIU Watch Dog Timer  Register               */
#define TS_CIU_WDTIMER_WD_TIME              (0xFFFFFFFF)
#define TS_CIU_WDTIMER_UNDEF                (0x00000000)


/* CIU Watch Dog Timer Shadow0 Register               */
#define TS_CIU_WDTIMER_SHADOW0_WD_COUNT     (0xFFFFFFFF)
#define TS_CIU_WDTIMER_SHADOW0_UNDEF        (0x00000000)

/* CIU Watch Dog Timer Shadow1 Register               */
#define TS_CIU_WDTIMER_SHADOW1_WD_COUNT     (0xFFFFFFFF)
#define TS_CIU_WDTIMER_SHADOW1_UNDEF        (0x00000000)

/* CIU Watch Dog Timer Address Register               */
#define TS_CIU_WD_ADDRESS_ADDRESS           (0xFFFFFFFF)
#define TS_CIU_WD_ADDRESS_UNDEF             (0x00000000)

/* CIU Length Error Address 0 Register                */
#define TS_CIU_LEN_ERROR_AD0_ADDRESS        (0xFFFFFFFF)
#define TS_CIU_LEN_ERROR_AD0_UNDEF          (0x00000000)

/* CIU Length Error Address 1 Register                */
#define TS_CIU_LEN_ERROR_AD1_ADDRESS        (0xFFFFFFFF)
#define TS_CIU_LEN_ERROR_AD1_UNDEF          (0x00000000)

/* CIU Abort Address Register                         */
#define TS_CIU_ABORT_AD_ADDRESS             (0xFFFFFFFF)
#define TS_CIU_ABORT_AD_UNDEF               (0x00000000)

/* CIU SRAM Byte Register                             */
#define TS_CIU_SRAM_BYTE_PARITY_BYTE        (0x000000FF)
#define TS_CIU_SRAM_BYTE_UNDEF              (0xFFFFFF00)

/* CIU Fabric Control and Status Register              */
#define TS_CIU_FAB_CSR_TEA                  (0x08000000)
#define TS_CIU_FAB_CSR_QUE_OVR              (0x04000000)
#define TS_CIU_FAB_CSR_IVLD_CMD             (0x02000000)
#define TS_CIU_FAB_CSR_SIZEMISMATCH         (0x01000000)
#define TS_CIU_FAB_CSR_INT_ENABLE_MASK      (0x00FF0000)
#define TS_CIU_FAB_CSR_SW_RST               (0x00000100)
#define TS_CIU_FAB_CSR_RGSWAP               (0x00000004)
#define TS_CIU_FAB_CSR_WSWAP                (0x00000002)
#define TS_CIU_FAB_CSR_BSWAP                (0x00000001)
#define TS_CIU_FAB_CSR_UNDEF                (0xF000FEF8)

/* CIU Fabric Sync Request Base Address Register       */
#define TS_CIU_FAB_SYNC_BAR_SYNC_BAR        (0xFFFFFF00)
#define TS_CIU_FAB_SYNC_BAR_EN              (0x00000001)
#define TS_CIU_FAB_SYNC_BAR_UNDEF           (0x000000FE)

/* CIU Switch Fabric BARx Look Up Table Registers      */
#define TS_CIU_SF_BARx_PAGE_ADDR            (0xFFFFFFE0)
#define TS_CIU_SF_BARx_DESTID               (0x0000000F)
#define TS_CIU_SF_BARx_LUTy_UNDEF           (0x00000010)

/* CIU Switch Fabric BARx Look Up Table Upper Register */
#define TS_CIU_SF_BARx_PAGE_ADDR_U          (0xFFFFFFFF)
#define TS_CIU_SF_BARx_LUTy_UPPER_UNDEF     (0x00000000)

/* SDRAM Control Register                           */

#define TS_SD_CNTRL_ENABLE         (0x80000000)
#define TS_SD_CNTRL_WAKE           (0x40000000)
#define TS_SD_CNTRL_CL             (0x38000000)
#define TS_SD_CNTRL_DD_EN          (0x04000000)
#define TS_SD_CNTRL_DQM_EN         (0x00400000)
#define TS_SD_CNTRL_APB            (0x00300000)
#define TS_SD_CNTRL_BL             (0x000C0000)
#define TS_SD_CNTRL_DQS_SZ         (0x00008000)
#define TS_SD_CNTRL_ECC_SZ         (0x00004000)
#define TS_SD_CNTRL_PORT_ARB       (0x00000300)

#define TS_SD_CNTRL_UNDEF          (0x03833CFF)



#  ifndef __ASSEMBLER__

/******************************************************************************
 **               CIU Control Register Structures
 **/

typedef struct {
 UINT32 lut;
 UINT32 lut_upper;
 } bar_look_up_entry_t;

typedef struct {
 UINT32 ciu_timer0;
 UINT32 ciu_timer0_shadow;
 UINT32 ciu_timer1;
 UINT32 ciu_timer1_shadow;
 UINT32 sram_bar;
 UINT32 ciu_cfg_bar;
 UINT32 ciu_sf_bar1;
 UINT32 ciu_sf_bar2;
 UINT32 ciu_sf_bar3;
 UINT32 ciu_sf_bar4;
 UINT32 ciu_sf_sizes;
 UINT32 ciu_ocn_err_status;
 UINT32 ciu_sram_status;
 UINT32 ciu_rq_status;
 UINT32 ciu_cfg;
 UINT32 ciu_ecc_status;
 UINT32 ciu_ecc_error_ad;
 UINT32 ciu_xarb_error;
 UINT32 ciu_error;
 UINT32 ciu_wdtimer;
 UINT32 ciu_wdtimer_shadow0;
 UINT32 ciu_wdtimer_shadow1;
 UINT32 ciu_wd_address;
 UINT32 ciu_len_error_ad0;
 UINT32 ciu_len_error_ad1;
 UINT32 ciu_abort_ad;
 UINT32 ciu_sram_byte;
 char   filler1[TS_CIU_FAB_CSR-TS_CIU_SRAM_BYTE+4];
 UINT32 ciu_fab_csr;
 UINT32 ciu_fab_sync_bar;
 char   filler2[TS_CIU_SF_BAR1_LUT0-TS_CIU_FAB_SYNC_BAR+4];
 bar_look_up_entry_t bar_lut[4][32];
 } ciu_reg_map_t;
#  endif  // __ASSEMLBER__

#endif /* _TS_CIU_H_ */
