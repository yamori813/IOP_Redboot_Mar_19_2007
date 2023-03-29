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

/*----------------------------------------------------------------------------
 * FILENAME: hal_ioc80314_i2c.h
 *
 * ORIGINATOR: Dan Otell
 * COMPILER(S): GNUPro (gcc)
 * COMPILER VERSION: xscale-020523
 *
 *  $Revision: 1.2 $
 *      $Date: 2002/10/08 20:57:29 $
 *
 * DESCRIPTION:
 *     This file contains the register definitions for the I2C interface IP
 *     used for StrongPac.  This includes definitions for register
 *     offsets, register bit fields, and a register map type.
 *
 *   History  :
 *   $Log: hal_ioc80314_i2c.h,v $
 *   Revision 1.2  2002/10/08 20:57:29  dotell
 *   Removed device-specific constants
 *
 *   Revision 1.1  2002/10/08 20:56:20  dotell
 *   Initial revision
 *
 *
 *---------------------------------------------------------------------------*/
#ifndef _TS_I2C_H_
#define _TS_I2C_H_

#include "hal_ioc80314_types.h"


/* Local defines */
/* Debug console output defines. */
#define    I2C_DEBUG

#ifdef     I2C_DEBUG
  #define  I2C_PRINTF(a)        flPrintf(a)
  #define  I2C_PRINTF1(a,b)     flPrintf(a,b)
  #define  I2C_PRINTF2(a,b,c)   flPrintf(a,b,c)
#else
  #define  I2C_PRINTF(a)
  #define  I2C_PRINTF1(a,b)
  #define  I2C_PRINTF2(a,b,c)
#endif


/* Register set base address.                                               */
#define TS_I2C_REG_BASE                     (0x00000000)

/* Used by I2C functions to communicate success/failure                      */
#define TS_I2C_FAILURE                      ((UINT32)0x0)
#define TS_I2C_SUCCESS                      ((UINT32)0x1)
#define TS_I2C_INV_PARAMETER                (0xF0F0F0F0)

/** Calculates the register address using the base addr. and register offset.*/
#define TS_I2C_REG_ADDR(offset)             (TS_I2C_REG_BASE + offset)


/*****************************************************************************
 **                     I2C Register Address offsets
 *****************************************************************************/


/* I2C Control 1 Register                   (I2C_CNTRL1)                        */
#define TS_I2C_CNTRL1                       ((VUINT32*)0x000)

/* I2C Control 2 Register                   (I2C_CNTRL2)                        */
#define TS_I2C_CNTRL2                       ((VUINT32*)0x000)

/* I2C Read Data Register                   (I2C_RD_DATA)                        */
#define TS_I2C_RD_DATA                      ((VUINT32*)0x008)

/* I2C Transmit Data Register               (I2C_RD_DATA)                        */
#define TS_I2C_TX_DATA                      ((VUINT32*)0x00C)


/*
 * Define some register ranges
 */

/* I2C FIRST valid register offset */
#define TS_I2C_FIRST_REG_OFFSET             (TS_I2C_CNTRL1)

/* I2C LAST valid register offset */
#define TS_I2C_LAST_REG_OFFSET              (TS_I2C_TX_DATA)



/*****************************************************************************
 **                    Register Bit Field Enables & Masks
 *****************************************************************************/

/* I2C Control 1 Register                   (I2C_CNTRL1)                        */
#define TS_I2C_CNTRL1_I2CWRITE              (0x01000000)
#define TS_I2C_CNTRL1_BYTADDR               (0x00FF0000)
#define TS_I2C_CNTRL1_PAGE                  (0x00000700)
#define TS_I2C_CNTRL1_DEVCODE               (0x0000000F)

/* I2C Control 2 Register                   (I2C_CNTRL2)                        */
#define TS_I2C_CNTRL2_IIB                   (0x0F000000)
#define TS_I2C_CNTRL2_RD_STATUS             (0x00020000)
#define TS_I2C_CNTRL2_WR_STATUS             (0x00010000)
#define TS_I2C_CNTRL2_START                 (0x00000100)
#define TS_I2C_CNTRL2_LANE                  (0x0000000C)
#define TS_I2C_CNTRL2_SIZE                  (0x00000003)

/* I2C Read Data Register                   (I2C_RD_DATA)                        */

/* I2C Transmit Data Register               (I2C_RD_DATA)                        */

#  ifndef __ASSEMBLER__

/*****************************************************************************
 **               Tundra I2C Control Register Structure
 *****************************************************************************/

/* I2C Register Map Structure */
typedef struct {

 UINT32 i2c_cntrl1;
 UINT32 i2c_cntrl2;
 UINT32 i2c_rd_data;
 UINT32 i2c_wr_data;

} __i2c_reg_map;
#  endif // __ASSEMBLER__

#endif /* _TS_I2C_H_ */
