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
 **   ORIGINATOR: Barry Wood
 **   COMPILER(S): GNUPro (gcc)
 **   COMPILER VERSION: xscale-020523
 **
 **   Revision : $Revision: 1.8 $
 **   Author   : $Author: bwood $
 **   Date     : $Date: 2002/10/09 16:45:40 $
 **   Purpose  : This file contains the register definitions for the SDRAM
 **              controller IP used for StrongPac.
 **   History  :
 **   $Log: hal_ioc80314_dram_ctlr.h,v $
 **   Revision 1.8  2002/10/09 16:45:40  bwood
 **   Change code to compile under GNU and debug
 **
 **   Revision 1.7  2002/10/09 04:23:58  bwood
 **   Change code to compile under GNU and debug
 **
 **   Revision 1.6  2002/10/07 14:59:45  bwood
 **   Update SDRAM tests
 **
 **   Revision 1.5  2002/10/04 11:03:16  bwood
 **   Move bit walk tests to mem_utils.c, add command interpreter
 **
 **   Revision 1.4  2002/09/18 20:15:53  bwood
 **   Changes to add test cases and make march tests common
 **
 **   Revision 1.3  2002/09/16 14:29:17  bwood
 **   Make changes to make code run in Redboot
 **
 **   Revision 1.2  2002/09/12 18:51:45  bwood
 **   SDRAM Tests - ECC closure, Page mode closure, FS 1.0
 **
 **   Revision 1.1  2002/09/06 18:08:52  bwood
 **   Initial revision
 **
 **
 **
 *****************************************************************************
 *****************************************************************************/
#include "hal_ioc80314_types.h"

#ifndef _TS_DRAM_CTLR_H_
#define _TS_DRAM_CTLR_H_

/*****************************************************************************
 *******       Start of Tundra SDRAM Controller Register Map       ***********
 *****************************************************************************/

/* Base Offset for start of SDRAM controller registers */
#define TS_SDRAM_CONTROLLER_BASE                   (0x00010000)


/** This macro calculates the register address using the register offset*/
#define  TS_SDRAM_REG_ADDR(offset)        \
               (TS_SDRAM_CONTROLLER_BASE + offset)

/******************************************************************************
 **                     SDRAM Controller Register Address offsets
 **/

/* SDRAM Refresh Interval Register               */
#define TS_SDRAM_REFRESH                (0x00000000)
/* SDRAM Control Register                        */
#define TS_SDRAM_CNTRL                  (0x00000004)
/* SDRAM Memory Bank Control and Status Register */
#define TS_SDRAM_BANK_CTRL              (0x00000008)
/* SDRAM Interrupt Status Register               */
#define TS_SDRAM_INT_STATUS             (0x0000000C)  /* ADD TO REGISTER TABLE */
/* SDRAM Memory Bank 0 Address                   */
#define TS_SDRAM_B0_ADDR                (0x00000010)
/* SDRAM Memory Bank 0 Address Mask              */
#define TS_SDRAM_B0_MASK                (0x00000014)
/* SDRAM Memory Bank 1 Address                   */
#define TS_SDRAM_B1_ADDR                (0x00000020)
/* SDRAM Memory Bank 1 Address Mask              */
#define TS_SDRAM_B1_MASK                (0x00000024)
/* SDRAM Memory Bank 2 Address                   */
#define TS_SDRAM_B2_ADDR                (0x00000030)
/* SDRAM Memory Bank 2 Address Mask              */
#define TS_SDRAM_B2_MASK                (0x00000034)
/* SDRAM Memory Bank 3 Address                   */
#define TS_SDRAM_B3_ADDR                (0x00000040)
/* SDRAM Memory Bank 3 Address Mask              */
#define TS_SDRAM_B3_MASK                (0x00000044)
/* SDRAM ECC Address of Error                    */
#define TS_SDRAM_ECC_ADDR1              (0x00000050)
/* SDRAM ECC Address of Error                    */
#define TS_SDRAM_ECC_ADDR2              (0x00000054)
/* SDRAM ECC Status                              */
#define TS_SDRAM_ECC_STATUS             (0x00000058)
/* SDRAM Gasket Status                           */
#define TS_SDRAM_GASKET_STATUS          (0x00000060)
/* SDRAM Shadow ECC Address of Error             */
#define TS_SDRAM_SECC_ADDR1             (0x00000150)
/* SDRAM Shadow ECC address of Error             */
#define TS_SDRAM_SECC_ADDR2             (0x00000154)
/* SDRAM Shadow ECC Status                       */
#define TS_SDRAM_SECC_STATUS            (0x00000158)
/* SDRAM DLL Adjust                              */
#define TS_SDRAM_DLL_ADJ                (0x00000160)
/* SDRAM I2C CNTRL1                              */
#define TS_SDRAM_I2C_CNTRL1             (0x00000400)
/* SDRAM I2C CNTRL2                              */
#define TS_SDRAM_I2C_CNTRL2             (0x00000404)
/* SDRAM I2C RD DATA                             */
#define TS_SDRAM_I2C_RD_DATA            (0x00000408)
/* SDRAM I2C WRT DATA                             */
#define TS_SDRAM_I2C_WRT_DATA           (0x0000040C)

/******************************************************************************
 **            Macro's for SDRAM Bank Address & Mask Registers offsets
 **/

/** TS_SDRAM_BANK_REG_ADDR calculates the register address using the bank
 ** number and register offset as parameters.  The offset parameter must
 ** be one of TS_SDRAM_BANK_ADDR or TS_SDRAM_BANK_MASK.  The bank parameter
 ** must be in the range of TS_SDRAM_FIRST_BANK and TS_SDRAM_LAST_BANK.
 **/

#define  TS_SDRAM_BANK_ADDR  0x00000000
#define  TS_SDRAM_BANK_MASK  0x00000004

#define  TS_SDRAM_FIRST_BANK 0x00
#define  TS_SDRAM_LAST_BANK  0x03

#define  TS_SDRAM_BANK_REG_ADDR(bank, offset)        \
               (TS_SDRAM_CONTROLLER_BASE + TS_SDRAM_B0_ADDR + (0x10*bank) + offset)


/*****************************************************************************
 *********       End of Tundra SDRAM Controller Register Map       ***********
 *****************************************************************************/


/*****************************************************************************
 **                    Register Bit Field Enables & Masks                   **
 *****************************************************************************/


/******************************************************************************
 **               Tundra SDRAM Control Registers
 **/

/* SDRAM Refresh Interval Register               */

#define TS_REFRESH_T                  (0x0000FFFF)

#define TS_REFRESH_UNDEF              (0xFFFF0000)

/******************************************************************************
 ** Macro to compute setting for TS_SDRAM_REFRESH register, given
 ** SDRAM refresh interval, multiplied by 10, in microseconds and the
 ** clock speed of the SDRAM bus in megahertz.
 **/

#define TS_SDRAM_COMP_REFRESH (refresh_usec, clock_speed_mhz) \
         (long) ((refresh_usec * clock_speed_mhz) / 10)

/* SDRAM Control Register                        */

#define TS_CNTRL_ENABLE         (0x80000000)
#define TS_CNTRL_WAKE           (0x40000000)
#define TS_CNTRL_CL             (0x38000000)
#define TS_CNTRL_DD_EN          (0x04000000)
#define TS_CNTRL_DQM_EN         (0x00400000)
#define TS_CNTRL_APB            (0x00300000)
#define TS_CNTRL_BL             (0x000C0000)
#define TS_CNTRL_DQS_SZ         (0x00008000)
#define TS_CNTRL_ECC_SZ         (0x00004000)
#define TS_CNTRL_PORT_ARB       (0x00000300)

#define TS_CNTRL_UNDEF          (0x03833CFF)

/* SDRAM Memory Bank Control and Status Register */

#define TS_BANK_CTRL_BUF        (0x20000000)
#define TS_BANK_CTRL_NBANK      (0x18000000)
#define TS_BANK_CTRL_AMODE      (0x07000000)
#define TS_BANK_CTRL_BMGT       (0x00F00000)
#define TS_BANK_CTRL_T_RAS      (0x000F0000)
#define TS_BANK_CTRL_T_RCD      (0x00007000)
#define TS_BANK_CTRL_T_RP       (0x00000700)
#define TS_BANK_CTRL_T_WR       (0x000000E0)
#define TS_BANK_CTRL_T_RFC      (0x0000001F)

#define TS_BANK_CTRL_BMGT_B0    (0x00100000)
#define TS_BANK_CTRL_BMGT_B1    (0x00200000)
#define TS_BANK_CTRL_BMGT_B2    (0x00400000)
#define TS_BANK_CTRL_BMGT_B3    (0x00800000)

#define TS_BANK_CTRL_UNDEF      (0xC0008800)

/******************************************************************************
 ** Macro to compute setting for TS_BANK_CTRL T RAS, RP,  WR, RFC register, given
 ** the SDRAM parameter in nanoseconds multiplied by 10, and the system clock interval
 ** (1 / clock speed in megahertz) in nanoseconds, multiplied by 10.  Both parameters
 ** must be entered as integers.
 **
 ** This computation only handles one decimal place of accuracy for the input parameters.
 ** To get more accuracy, multiply both input parameters by the same, larger value.
 **/

#define TS_SDRAM_COMP_BANK_CTRL (sdram_timing_parm_nsec, clock_interval_nsec) \
         (long) ((sdram_timing_parm_nsec / clock_interval_nsec) + 1 )

/* SDRAM Interrupt Status                        */

#define TS_INT_STATUS_ECC_INT   (0x00000004)
#define TS_INT_STATUS_I2C_INT   (0x00000002)
#define TS_INT_STATUS_GSK_INT   (0x00000001)

#define TS_INT_STATUS_UNDEF     (0xFFFFFFF8)

/* SDRAM Memory Bank x Address                   */

#define TS_RAM_Bx_ADDR_ENABLE        (0x00100000)
#define TS_RAM_Bx_ADDR_A             (0x000FFFFF)

#define TS_RAM_Bx_ADDR_UNDEF         (0xFFE00000)

/******************************************************************************
 ** Macro to compute address value for Bank x address given a 32 bit address.
 **/

#define TS_RAM_Bx_ADDR_ENABLE_COMP   (addr) ((addr >> 16 ) & TS_RAM_Bx_ADDR_A)

/* SDRAM Memory Bank x Address Mask              */

#define TS_RAM_Bx_MASK_M             (0x000FFFFF)
#define TS_RAM_Bx_MASK_UNDEF         (0xFFF00000)

/* SDRAM DLL Adjust                              */
#define TS_DLL_ADJ_AD_SB         (0x00000080)
#define TS_DLL_ADJ_DLL_OFFSET    (0x0000001F)

#define TS_DLL_ADJ_UNDEF         (0xFFFFFF60)

/* SDRAM Gasket Status                           */
#define TS_GASKET_STATUS_GSKT2_MSG (0x00FF0000)
#define TS_GASKET_STATUS_GSKT1_MSG (0x0000FF00)
#define TS_GASKET_STATUS_GSKT2     (0x00000002)
#define TS_GASKET_STATUS_GSKT1     (0x00000001)

#define TS_GASKET_STATUS_UNDEF     (0xFF0000FC)

/******************************************************************************
 **               Tundra SDRAM ECC Registers
 **/

/* SDRAM ECC Address of Error                    */
#define TS_ECC_ADDR1_EA           (0xFFFFFFF8)
#define TS_ECC_ADDR1_CLEAR        (0x00000001)

#define TS_ECC_ADDR1_UNDEF        (0x00000006)

/* SDRAM ECC Address of Error                    */
#define TS_ECC_ADDR2_EA           (0x0000000F)

#define TS_ECC_ADDR2_UNDEF        (0xFFFFFFF0)

/* SDRAM ECC Status                              */
#define TS_ECC_STATUS_ECC_EE      (0x08000000)
#define TS_ECC_STATUS_ECC_EN      (0x04000000)
#define TS_ECC_STATUS_EG_EN       (0x02000000)
#define TS_ECC_STATUS_ECC_UC      (0x01000000)
#define TS_ECC_STATUS_ECC_CO      (0x04000000)
#define TS_ECC_STATUS_ECC_SYND    (0x04000000)
#define TS_ECC_STATUS_ECC_GEN     (0x04000000)

#define TS_ECC_STATUS_UNDEF       (0xF0000000)


/* SDRAM Shadow ECC Address of Error             */
#define TS_SECC_ADDR1_SEA         (0xFFFFFFF8)

#define TS_SECC_ADDR1_UNDEF       (0x00000007)

/* SDRAM Shadow ECC address of Error             */
#define TS_SECC_ADDR2_SEA         (0x0000000F)

#define TS_SECC_ADDR2_UNDEF       (0xFFFFFFF0)

/* SDRAM Shadow ECC Status                       */
#define TS_SECC_STATUS_SECC_EE      (0x08000000)
#define TS_SECC_STATUS_SECC_EN      (0x04000000)
#define TS_SECC_STATUS_SEG_EN       (0x02000000)
#define TS_SECC_STATUS_SECC_UC      (0x01000000)
#define TS_SECC_STATUS_SECC_CO      (0x00FF0000)
#define TS_SECC_STATUS_SECC_SYND    (0x0000FF00)
#define TS_SECC_STATUS_SECC_GEN     (0x000000FF)

#define TS_SECC_STATUS_UNDEF        (0xF0000000)



/******************************************************************************
 **               Tundra SDRAM I2C Registers
 **/

/* SDRAM I2C CNTRL1                              */
#define TS_I2C_CNTRL1_WR_EN         (0x01000000)
#define TS_I2C_CNTRL1_I2C_ADDR      (0x00FF0000)
#define TS_I2C_CNTRL1_PAGE_SELECT   (0x00000700)
#define TS_I2C_CNTRL1_DEF_CODE      (0x0000000F)

#define TS_I2C_CNTRL1_UNDEF         (0xFE00F8F0)

/* SDRAM I2C CNTRL2                              */
#define TS_I2C_CNTRL2_I2C_CMP       (0x10000000)
#define TS_I2C_CNTRL2_I2C_CFG_ERR   (0x08000000)
#define TS_I2C_CNTRL2_I2C_TO        (0x04000000)
#define TS_I2C_CNTRL2_I2C_WR_PROT   (0x02000000)
#define TS_I2C_CNTRL2_I2C_ABH_ERR   (0x01000000)
#define TS_I2C_CNTRL2_RD_STAT       (0x00020000)
#define TS_I2C_CNTRL2_WR_STAT       (0x00010000)
#define TS_I2C_CNTRL2_START         (0x00000100)
#define TS_I2C_CNTRL2_LANE          (0x0000000C)
#define TS_I2C_CNTRL2_SIZE          (0x00000003)

#define TS_I2C_CNTRL2_UNDEF         (0xE0FCFEF0)

/* SDRAM I2C RD DATA                             */
#define TS_I2C_RD_DATA_I2C_RD_BYTE3 (0xFF000000)
#define TS_I2C_RD_DATA_I2C_RD_BYTE2 (0x00FF0000)
#define TS_I2C_RD_DATA_I2C_RD_BYTE1 (0x0000FF00)
#define TS_I2C_RD_DATA_I2C_RD_BYTE0 (0x000000FF)

/* SDRAM I2C WRT DATA                             */
#define TS_I2C_RD_DATA_I2C_WRT_BYTE3 (0xFF000000)
#define TS_I2C_RD_DATA_I2C_WRT_BYTE2 (0x00FF0000)
#define TS_I2C_RD_DATA_I2C_WRT_BYTE1 (0x0000FF00)
#define TS_I2C_RD_DATA_I2C_WRT_BYTE0 (0x000000FF)



#  ifndef __ASSEMBLER__

/******************************************************************************
 **               Tundra SDRAM I2C Registers Information Structure
 **/

typedef struct {
    int     reg_offset;
    int     reset_value;
    int     read_write_mask;
    char    test_for_read;
    char    test_for_write; } ts_sdram_regs_info_t;

extern ts_sdram_regs_info_t ts_sdram_list_of_regs[24];

#define max_ts_sdram_list_of_regs_index 23
#  endif  // __ASSEMBLER__

#endif /* _TS_DRAM_CTLR_H_ */


