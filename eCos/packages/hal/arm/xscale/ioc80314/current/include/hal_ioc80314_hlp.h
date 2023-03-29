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
 **   Date     : $Date: 2002/10/09 18:14:37 $
 **   Purpose  : This file contains the register definitions for the
 **              Host Local Port (HLP) used for StrongPac.
 **              This includes definitions for register offsets,
 **              register bit fields, and a register map type.
 **   History  :
 **   $Log: hal_ioc80314_hlp.h,v $
 **   Revision 1.2  2002/10/09 18:14:37  bwood
 **   Changes to allow HLP tests to run under GNU compiler
 **
 **   Revision 1.1  2002/10/03 12:11:23  bwood
 **   Initial revision
 **
 **
 *****************************************************************************
 *****************************************************************************/
#include "hal_ioc80314_types.h"

#ifndef _TS_HLP_H_
#define _TS_HLP_H_

/*******************************************************
 *******       Start of Tundra HLP           ***********
 *******************************************************/

/* Base Offset for start of HLP controller registers */
#define TS_HLP_REG_BASE (0x00010000)


/** This macro calculates the register address using the register offset*/
#define  TS_HLP_REG_ADDR(offset)        \
               (TS_HLP_REG_BASE + offset)

/******************************************************************************
 **                     HLP  Register Address offsets
 **/

/*    HLP ROM Bank 0 Base Address                     */
#define TS_HLP_B0_ADDR              (0x00000000)
/*    HLP ROM Bank 0 Base Address Compare Mask        */
#define TS_HLP_B0_MASK              (0x00000004)
/*    HLP ROM Bank 0 Base Address Control 0           */
#define TS_HLP_B0_CTRL0             (0x00000008)
/*    HLP ROM Bank 0 Base Address Control 1           */
#define TS_HLP_B0_CTRL1             (0x0000000C)

/*    HLP ROM Bank 1 Base Address                     */
#define TS_HLP_B1_ADDR              (0x00000010)
/*    HLP ROM Bank 1 Base Address Compare Mask        */
#define TS_HLP_B1_MASK              (0x00000014)
/*    HLP ROM Bank 1 Base Address Control 0           */
#define TS_HLP_B1_CTRL0             (0x00000018)
/*    HLP ROM Bank 1 Base Address Control 1           */
#define TS_HLP_B1_CTRL1             (0x0000001C)

/*    HLP ROM Bank 2 Base Address                     */
#define TS_HLP_B2_ADDR              (0x00000020)
/*    HLP ROM Bank 2 Base Address Compare Mask        */
#define TS_HLP_B2_MASK              (0x00000024)
/*    HLP ROM Bank 2 Base Address Control 0           */
#define TS_HLP_B2_CTRL0             (0x00000028)
/*    HLP ROM Bank 2 Base Address Control 1           */
#define TS_HLP_B2_CTRL1             (0x0000002C)

/*    HLP ROM Bank 3 Base Address                     */
#define TS_HLP_B3_ADDR              (0x00000030)
/*    HLP ROM Bank 3 Base Address Compare Mask        */
#define TS_HLP_B3_MASK              (0x00000034)
/*    HLP ROM Bank 3 Base Address Control 0           */
#define TS_HLP_B3_CTRL0             (0x00000038)
/*    HLP ROM Bank 0 Base Address Control 1           */
#define TS_HLP_B3_CTRL1             (0x0000003C)



/*    HLP ROM Bank 3 Base Address Compare Mask        */
#define TS_HLP_B3_CTRL1             (0x0000003C)

/*
 * Define some Register offset computations
 */

#define TS_HLP_Bx_ADDR( x )  (TS_HLP_B0_ADDR  + (x*0x10))
#define TS_HLP_Bx_MASK( x )  (TS_HLP_B0_MASK  + (x*0x10))
#define TS_HLP_Bx_CTRL0( x ) (TS_HLP_B0_CTRL0 + (x*0x10))
#define TS_HLP_Bx_CTRL1( x ) (TS_HLP_B0_CTRL1 + (x*0x10))

/*******************************************************
 *******        End of Tundra HLP            ***********
 *******************************************************/


/*****************************************************************************
 **                    Register Bit Field Enables & Masks                   **
 *****************************************************************************/


/******************************************************************************
 **               Tundra CIU Control Register Bit Fields
 **/

/*    HLP ROM Bank X Base Address                     */
#define TS_HLP_Bx_ADDR_A                     (0xFFFFFF00)
#define TS_HLP_Bx_ADDR_UNDEF                 (0x000000FF)

/*    HLP ROM Bank X Base Address Mask                */
#define TS_HLP_Bx_MASK_A                     (0xFFFFFF00)
#define TS_HLP_Bx_MASK_UNDEF                 (0x000000FF)

/*    HLP ROM Bank X Control 0                        */
#define TS_HLP_Bx_CTRL0_BM                   (0x80000000)
#define TS_HLP_Bx_CTRL0_FWE                  (0x40000000)
#define TS_HLP_Bx_CTRL0_WAIT                 (0x3F000000)
#define TS_HLP_Bx_CTRL0_CSON                 (0x00C00000)
#define TS_HLP_Bx_CTRL0_OEON                 (0x00300000)
#define TS_HLP_Bx_CTRL0_WEON                 (0x000C0000)
#define TS_HLP_Bx_CTRL0_THRD                 (0x0000F000)
#define TS_HLP_Bx_CTRL0_THWR                 (0x00000F00)
#define TS_HLP_Bx_CTRL0_FWT                  (0x000000FC)
#define TS_HLP_Bx_CTRL0_WIDTH                (0x00000003)

#define TS_HLP_Bx_CTRL0_UNDEF                (0x00030000)

#define TS_HLP_8BIT_BANK   0
#define TS_HLP_16BIT_BANK  1
#define TS_HLP_32BIT_BANK  2

/*    HLP ROM Bank X Control 1                        */
#define TS_HLP_Bx_CTRL1_MODE                 (0x80000000)
#define TS_HLP_Bx_CTRL1_ENABLE               (0x40000000)
#define TS_HLP_Bx_CTRL1_LE                   (0x30000000)
#define TS_HLP_Bx_CTRL1_LEHD                 (0x0C000000)
#define TS_HLP_Bx_CTRL1_RE                   (0x02000000)
#define TS_HLP_Bx_CTRL1_ARE                  (0x01000000)
#define TS_HLP_Bx_CTRL1_WEOFF                (0x000F0000)
#define TS_HLP_Bx_CTRL1_PAGE                 (0x0000E000)

#define TS_HLP_Bx_CTRL1_UNDEF                (0x00F01FFF)

#define TS_HLP_4_WORD_BURST    1
#define TS_HLP_8_WORD_BURST    2
#define TS_HLP_16_WORD_BURST   3
#define TS_HLP_32_WORD_BURST   4
#define TS_HLP_64_WORD_BURST   5
#define TS_HLP_128_WORD_BURST  6
#define TS_HLP_256_WORD_BURST  7

/******************************************************************************
 **               Tundra HLP Control Register Structure
 **/

#  ifndef __ASSEMBLER__

typedef struct {
 UINT32 addr;
 UINT32 addr_mask;
 UINT32 control0;
 UINT32 control1;
 } hlp_control_regs_t;

typedef hlp_control_regs_t hlp_reg_map_t[4];
#  endif // __ASSEMBLER__
#endif /* _TS_HLP_H_ */
