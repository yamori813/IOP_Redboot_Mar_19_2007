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
 * FILENAME: hal_ioc80314_mpic.h
 *
 * ORIGINATOR: Dan Otell
 * COMPILER(S): GNUPro (gcc)
 * COMPILER VERSION: xscale-020523
 *
 *  $Revision: 1.7 $
 *      $Date: 2002/10/08 21:52:16 $
 *
 * DESCRIPTION:
 *     This file contains the register definitions for the Interrupt Controller
 *     Block (MPIC) used for StrongPac.  This includes definitions for register
 *     offsets, register bit fields, and a register map type.
 *
 *   History  :
 *   $Log: hal_ioc80314_mpic.h,v $
 *   Revision 1.7  2002/10/08 21:52:16  dotell
 *   Moved function prototypes to top of fl_*.c
 *
 *   Revision 1.6  2002/10/07 22:30:21  dotell
 *   Modified order of operations for register definitions
 *
 *   Revision 1.5  2002/10/07 22:17:49  dotell
 *   Add PRINTF defines
 *
 *   Revision 1.4  2002/10/01 18:41:57  dotell
 *   Incorporated Feedback from Review held on Sep. 27/2002
 *
 *   Revision 1.3  2002/09/26 23:28:19  dotell
 *   Addedd SUCCESS/FAILURE defintions
 *
 *   Revision 1.2  2002/09/24 19:04:34  dotell
 *   Added General definitions for VPR and DR registers
 *
 *   Revision 1.1  2002/09/19 21:55:09  dotell
 *   Initial revision
 *
 *
 *---------------------------------------------------------------------------*/
#ifndef _TS_MPIC_H_
#define _TS_MPIC_H_

#include "hal_ioc80314_types.h"


/* Local defines */
/* Debug console output defines. */
#define    MPIC_DEBUG

#ifdef     MPIC_DEBUG
  #define  MPIC_PRINTF(a)        flPrintf(a)
  #define  MPIC_PRINTF1(a,b)     flPrintf(a,b)
  #define  MPIC_PRINTF2(a,b,c)   flPrintf(a,b,c)
#else
  #define  MPIC_PRINTF(a)
  #define  MPIC_PRINTF1(a,b)
  #define  MPIC_PRINTF2(a,b,c)
#endif


/* Base Offset for start of SDRAM controller registers */
#define TS_MPIC_REG_BASE                     (0x00010000)

/* Used by MPIC functions to communicate success/failure  */
#define MPIC_FAILURE                         ((UINT32)0x0)
#define MPIC_SUCCESS                         ((UINT32)0x1)
#define MPIC_INV_PARAMETER                   (0xF0F0F0F0)

/** Calculates the register address using the base addr. and register offset.*/
#define TS_MPIC_REG_ADDR(offset)             (TS_MPIC_REG_BASE + offset)

/** Device-specific constants */
#define TS_MPIC_NUM_GLOBAL_TIMERS            (4)
#define TS_MPIC_NUM_IRQ_LINES                (4)
#define TS_MPIC_NUM_DOORBELLS                (4)
#define TS_MPIC_NUM_MAILBOX                  (4)
#define TS_MPIC_NUM_EXT_INTS                 (24)


/*****************************************************************************
 **                     MPIC Register Address offsets
 *****************************************************************************/


/* Feature Reporting Register                (FRR)                        */
#define TS_MPIC_FRR                          ((VUINT32*)0x000)

/* Global Configuration Register             (GCR)                        */
#define TS_MPIC_GCR                          ((VUINT32*)0x004)

/* MPIC Interrupt Configuration Register     (MICR)                       */
#define TS_MPIC_MICR                         ((VUINT32*)0x008)

/* MPIC Vendor Identification Register       (MVI)                        */
#define TS_MPIC_MVI                          ((VUINT32*)0x00C)

/* Spurious Vector Register                  (SVR)                        */
#define TS_MPIC_SVR                          ((VUINT32*)0x010)

/* Timer Frequency Reporting Register        (TFRR)                       */
#define TS_MPIC_TFRR                         ((VUINT32*)0x014)

/* Global Timer Current Count Register       (GTCCR)                      */
#define TS_MPIC_GTCCR(n)                     ((VUINT32*)(0x030 + (n * 0x10)))

/* Global Timer Base Count Register          (GTBCR)                      */
#define TS_MPIC_GTBCR(n)                     ((VUINT32*)(0x034 + (n * 0x10)))

/* GLobal Timer Vector/Prioirty Register     (GTVPR)                      */
#define TS_MPIC_GTVPR(n)                     ((VUINT32*)(0x038 + (n * 0x10)))

/* Global Timer Desitnation Register         (GTDR)                       */
#define TS_MPIC_GTDR(n)                      ((VUINT32*)(0x03C + (n * 0x10)))

/* Interrupt Software Override Register      (INT_SOFT_SET)               */
#define TS_MPIC_INT_SOFT_SET                 ((VUINT32*)0x020)

/* Interrupt SW Override Enable Register     (INT_SOFT_ENABLE)            */
#define TS_MPIC_INT_SOFT_ENABLE              ((VUINT32*)0x024)

/* IRQ Vector/Priority Register              (IVPR)                       */
#define TS_MPIC_IVPR(n)                      ((VUINT32*)(0x100 + (n * 0x8)))

/* IRQ Destination Register                  (IDR)                        */
#define TS_MPIC_IDR(n)                       ((VUINT32*)(0x104 + (n * 0x8)))

/* Doorbell Activation Register              (DAR)                        */
#define TS_MPIC_DAR                          ((VUINT32*)0x200)

/* Doorbell Vector/Priority Register         (DVPR)                       */
#define TS_MPIC_DVPR(n)                      ((VUINT32*)(0x204 + (n * 0x10)))

/* Doorbell Destination Register             (DDR)                        */
#define TS_MPIC_DDR(n)                       ((VUINT32*)(0x208 + (n * 0x10)))

/* Doorbell Messaging Register               (DMR)                        */
#define TS_MPIC_DMR(n)                       ((VUINT32*)(0x20C + (n * 0x10)))

/* Mailbox Register                          (MBR)                        */
#define TS_MPIC_MBR(n)                       ((VUINT32*)(0x280 + (n * 0x10)))

/* Mailbox Vector/Priority Register          (MBVPR)                      */
#define TS_MPIC_MBVPR(n)                     ((VUINT32*)(0x284 + (n * 0x10)))

/* Mailbox Destination Register              (MBDR)                       */
#define TS_MPIC_MBDR(n)                      ((VUINT32*)(0x288 + (n * 0x10)))

/* Processor Current Task Priority Register  (TASK)                       */
#define TS_MPIC_TASK(n)                      ((VUINT32*)(0x300 + (n * 0x40)))

/* CPU Interrupt Acknowledge Register        (VECTOR)                     */
#define TS_MPIC_VECTOR(n)                    ((VUINT32*)(0x304 + (n * 0x40)))

/* CPU End Of Interrupt Register             (EOI)                        */
#define TS_MPIC_EOI(n)                       ((VUINT32*)(0x308 + (n * 0x40)))

/* CPU Sensitivity Register                  (CSR)                        */
#define TS_MPIC_CSR(n)                       ((VUINT32*)(0x30C + (n * 0x40)))


/*
 * Define some register ranges
 */

/* MPIC FIRST valid register offset */
#define TS_MPIC_FIRST_REG_OFFSET             (TS_MPIC_FRR)

/* MPIC LAST valid register offset */
#define TS_MPIC_LAST_REG_OFFSET              (TS_MPIC_CSR(3))



/*****************************************************************************
 **                    Register Bit Field Enables & Masks
 *****************************************************************************/

/* General VPR and DR defines                                                */
#define TS_MPIC_VPR_M                        (0x80000000)
#define TS_MPIC_VPR_A                        (0x40000000)
#define TS_MPIC_VPR_S                        (0x02000000)
#define TS_MPIC_VPR_P                        (0x01000000)
#define TS_MPIC_VPR_PRESCALE                 (0x00F00000)
#define TS_MPIC_VPR_PRIORITY                 (0x000F0000)
#define TS_MPIC_VPR_VECTOR                   (0x800000FF)
#define TS_MPIC_DR_SEL_OUT_MASK              (0x0000000F)
#define TS_MPIC_DR_SEL_OUT(n)                ((UINT32)0x1 << n)

/* Feature Reporting Register                (FRR)                        */
#define TS_MPIC_FRR_NDOOR                    (0xE0000000)
#define TS_MPIC_FRR_NIRQ                     (0x07FF0000)
#define TS_MPIC_FRR_NITM                     (0x0000E000)
#define TS_MPIC_FRR_NCPU                     (0x00001F00)
#define TS_MPIC_FRR_VID                      (0x000000FF)

/* Global Configuration Register             (GCR)                        */
#define TS_MPIC_GCR_R                        (0x80000000)

/* MPIC Interrupt Configuration Register     (MICR)                       */
#define TS_MPIC_MICR_R                       (0x0000000F)

/* MPIC Vendor Identification Register       (MVI)                        */
#define TS_MPIC_MVI_STEP                     (0x00FF0000)
#define TS_MPIC_MVI_DID                      (0x0000FF00)
#define TS_MPIC_MVI_VID                      (0x000000FF)

/* Spurious Vector Register                  (SVR)                        */
#define TS_MPIC_SVR_VECTOR                   (0x000000FF)

/* Timer Frequency Reporting Register        (TFRR)                       */
#define TS_MPIC_TFRR_TIME_FREQ               (0xFFFFFFFF)

/* Global Timer Current Count Register       (GTCCR)                      */
#define TS_MPIC_GTCCR_T                      (0x80000000)
#define TS_MPIC_GTCCR_COUNT                  (0x7FFFFFFF)

/* Global Timer Base Count Register          (GTBCR)                      */
#define TS_MPIC_GTBCR_CI                     (0x80000000)
#define TS_MPIC_GTBCR_B_COUNT                (0x7FFFFFFF)

/* GLobal Timer Vector/Prioirty Register     (GTVPR)                      */
#define TS_MPIC_GTVPR_M                      (0x80000000)
#define TS_MPIC_GTVPR_A                      (0x40000000)
#define TS_MPIC_GTVPR_PRESCALE               (0x00F00000)
#define TS_MPIC_GTVPR_PRIORITY               (0x000F0000)
#define TS_MPIC_GTVPR_VECTOR                 (0x000000FF)

/* Global Timer Desitnation Register         (GTDR)                       */
#define TS_MPIC_GTDR_SEL_OUT_MASK            (0x0000000F)
#define TS_MPIC_GTDR_SEL_OUT(n)              ((UINT32)0x1 << n)

/* Interrupt Software Override Register      (INT_SOFT_SET)               */
#define TS_MPIC_INT_SOFT_SET_S_MASK          (0x00FFFFFF)
#define TS_MPIC_INT_SOFT_SET_S(n)            ((UINT32)0x1 << n)

/* Interrupt SW Override Enable Register     (INT_SOFT_ENABLE)            */
#define TS_MPIC_INT_SOFT_ENABLE_En_MASK      (0x00FFFFFF)
#define TS_MPIC_INT_SOFT_ENABLE_En(n)        ((UINT32)0x1 << n)

/* IRQ Vector/Priority Register              (IVPR)                       */
#define TS_MPIC_IVPR_M                       (0x80000000)
#define TS_MPIC_IVPR_A                       (0x40000000)
#define TS_MPIC_IVPR_DIRECTED                (0x00000000)
#define TS_MPIC_IVPR_DISTRIBUTED             (0x20000000)
#define TS_MPIC_IVPR_P                       (0x01000000)
#define TS_MPIC_IVPR_P_ACTIVE_LOW            (0x00000000)
#define TS_MPIC_IVPR_P_ACTIVE_HIGH           (0x00000001)
#define TS_MPIC_IVPR_S                       (0x02000000)
#define TS_MPIC_IVPR_S_EDGE_SENSITIVE        (0x00000000)
#define TS_MPIC_IVPR_S_LEVEL_SENSITIVE       (0x00000002)
#define TS_MPIC_IVPR_PRIORITY                (0x000F0000)
#define TS_MPIC_IVPR_VECTOR                  (0x800000FF)

/* IRQ Destination Register Register         (IDR)                        */
#define TS_MPIC_IDR_SEL_OUT_MASK             (0x0000000F)
#define TS_MPIC_IDR_SEL_OUT(n)               ((UINT32)0x1 << n)

/* Doorbell Activation Register              (DAR)                        */
#define TS_MPIC_DAR_A_MASK                   (0x0000000F)
#define TS_MPIC_DAR_A(n)                     ((UINT32)0x1 << n)

/* Doorbell Vector/Priority Register         (DVPR)                       */
#define TS_MPIC_DVPR_M                       (0x80000000)
#define TS_MPIC_DVPR_A                       (0x40000000)
#define TS_MPIC_DVPR_PRIORITY                (0x000F0000)
#define TS_MPIC_DVPR_VECTOR                  (0x800000FF)

/* Doorbell Destination Register             (DDR)                        */
#define TS_MPIC_DDR_SEL_OUT_MASK             (0x0000000F)
#define TS_MPIC_DDR_SEL_OUT(n)               ((UINT32)0x1 << n)

/* Doorbell Messaging Register               (DMR)                        */
#define TS_MPIC_DMR_M                        (0xFFFFFFFF)

/* Mailbox Register                          (MBR)                        */
#define TS_MPIC_MBR_M                        (0xFFFFFFFF)

/* Mailbox Vector/Priority Register          (MBVPR)                      */
#define TS_MPIC_MBVPR_M                      (0x80000000)
#define TS_MPIC_MBVPR_A                      (0x40000000)
#define TS_MPIC_MBVPR_PRIORITY               (0x000F0000)
#define TS_MPIC_MBVPR_VECTOR                 (0x800000FF)

/* Mailbox Destination Register              (MBDR)                       */
#define TS_MPIC_MBDR_SEL_OUT_MASK            (0x0000000F)
#define TS_MPIC_MBDR_SEL_OUT(n)              ((UINT32)0x1 << n)

/* Processor Current Task Priority Register  (TASK)                       */
#define TS_MPIC_TASK_TASK_MASK               (0x0000000F)
#define TS_MPIC_TASK_TASK(n)                 ((UINT32)0x1 << n)

/* CPU Interrupt Acknowledge Register        (VECTOR)                     */
#define TS_MPIC_VECTOR_LS_VECTOR             (0xFF000000)
#define TS_MPIC_VECTOR_VECTOR                (0x000000FF)

/* CPU End Of Interrupt Register             (EOI)                        */
#define TS_MPIC_EOI_EOI                      (0x000000FF)

/* CPU Sensitivity Register                  (CSR)                        */
#define TS_MPIC_CSR_P                        (0x00000001)
#define TS_MPIC_CSR_P_ACTIVE_LOW             (0x00000000)
#define TS_MPIC_CSR_P_ACTIVE_HIGH            (0x00000001)
#define TS_MPIC_CSR_S                        (0x00000002)
#define TS_MPIC_CSR_S_EDGE_SENSITIVE         (0x00000000)
#define TS_MPIC_CSR_S_LEVEL_SENSITIVE        (0x00000002)


#  ifndef __ASSEMBLER__

/*****************************************************************************
 **               Tundra MPIC Control Register Structures
 *****************************************************************************/

/* Global Timer Structure */
typedef struct {
 UINT32 gtccr;
 UINT32 gtbcr;
 UINT32 gtvpr;
 UINT32 gtbdr;
 } __global_timer;


/* IRQ Structure */
typedef struct {
 UINT32 ivpr;
 UINT32 idr;
  } __irq;


/* Doorbell Structure */
typedef struct {
 UINT32 dar;
 UINT32 dvpr;
 UINT32 ddr;
 UINT32 dmr;
  } __doorbell;


/* Mailbox Structure */
typedef struct {
 UINT32 mbr;
 UINT32 mbvpr;
 UINT32 mbdr;
   } __mailbox;


/* Interrupt Servicing Structure */
typedef struct {
 UINT32 task;
 UINT32 vector;
 UINT32 eoi;
 UINT32 csr;
   } __interrupt_service;


/* MPIC Register Map Structure */
typedef struct {
 UINT32 mpic_frr;
 UINT32 mpic_gcr;
 UINT32 mpic_micr;
 UINT32 mpic_mvi;
 UINT32 mpic_svr;
 UINT32 mpic_tfrr;
 char reserved0[TS_MPIC_INT_SOFT_SET - TS_MPIC_TFRR];

 UINT32 mpic_int_soft_set;
 UINT32 mpic_int_soft_enable;

 char reserved1[TS_MPIC_GTCCR(0) - TS_MPIC_INT_SOFT_ENABLE];

  __global_timer global_timer[TS_MPIC_NUM_GLOBAL_TIMERS];

 char reserved2[TS_MPIC_IVPR(0) - TS_MPIC_GTDR(3)];

  __irq irq[TS_MPIC_NUM_EXT_INTS];

 char reserved3[TS_MPIC_DAR - TS_MPIC_IDR(3)];

 __doorbell doorbell[TS_MPIC_NUM_DOORBELLS];

 char reserved4[TS_MPIC_MBR(3) - TS_MPIC_DMR(3)];

 __mailbox mailbox[TS_MPIC_NUM_MAILBOX];

 char reserved5[TS_MPIC_TASK(3) - TS_MPIC_MBDR(3)];

 __interrupt_service interrupt_service[TS_MPIC_NUM_IRQ_LINES];

} __mpic_reg_map;

#  endif  //  __ASSEMBLER__

#endif /* _TS_MPIC_H_ */
