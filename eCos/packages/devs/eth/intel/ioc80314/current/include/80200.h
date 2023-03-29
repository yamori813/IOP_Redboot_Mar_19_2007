#ifndef _80200_H

#define _80200_H



/* Filename: 80200.h
 *
 * Copyright:    (C) 2003-2004 Intel Corporation.
 *
 * This software has been developed by Intel Corporation.
 * Intel specifically disclaims all warranties, express or
 * implied, and all liability, including consequential and
 * other indirect damages, for the use of this program, including
 * liability for infringement of any proprietary rights,
 * and including the warranties of merchantability and fitness
 * for a particular purpose. Intel does not assume any
 * responsibility for and errors which may appear in this program
 * not any responsibility to update it
 */

 

/*
 *  Author:
 *   Brett Gaines
 *
 *  Revision History:
 *   08/23/00 Initial release
 *   03/02/01 Added Remaining CP definitions
 *   04/11/01 Added read masks for read-unpredictable data,
 *            added DCSR and _GET_ macros.
 *   09/27/02 (avb) Added _BCUCTL_ bit definitions.
 */


#if defined(__cplusplus)

#define REGISTER

#else

#define REGISTER register

#endif



/***************************************************************
 * Coprocessor 15 
 **************************************************************/

#define _Read_Cache_Type \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c0, c0, 1" : "=r" (_val_)); \
   _val_; \
})



#define _Read_ARM_Control \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c1, c0, 0" : "=r" (_val_)); \
   _val_ & 0x00003FFF; \
})



#define _Write_ARM_Control(VAL) \
asm("mcr\tp15, 0, %0, c1, c0, 0" : : "r" (VAL) : "memory")



#define _VECTOR_RELOC (1 << 13)

#define _I_CACHE_ENABLE (1 << 12)

#define _BTB_ENABLE (1 << 11)

#define _ROM_PROTECT (1 << 9)

#define _SYS_PROTECT (1 << 8)

#define _BIG_ENDIAN (1 << 7)

#define _D_CACHE_ENABLE (1 << 2)

#define _ALIGN_FAULT (1 << 1)

#define _MMU_ENABLE (1 << 0)



#define _Read_AUX_Control \
({ REGISTER unsigned _val_; \
   asm("mrc\tp15, 0, %0, c1, c0, 1" : "=r" (_val_)); \
   _val_ & 0x00000033; \
})



#define _Write_AUX_Control(VAL) \
asm("mcr\tp15, 0, %0, c1, c0, 1" : : "r" (VAL))



#define _Read_Translation_Table_Base \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c2, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_Translation_Table_Base(TTB) \
asm("mcr\tp15, 0, %0, c2, c0, 0" : : "r" (TTB))



#define _Read_Domain_Access_Control \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c3, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_Domain_Access_Control(VAL) \
asm("mcr\tp15, 0, %0, c3, c0, 0" : : "r" (VAL))


/*
#define _Read_FSR \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c5, c0, 0" : "=r" (_val_)); \=
   _val_ & 0x000007FF; \
})
*/

#define _Read_FSR \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c5, c0, 0" : "=r" (_val_)); \
   _val_; \
})


#define _Write_FSR(VAL) \
asm("mcr\tp15, 0, %0, c5, c0, 0" : : "r" (VAL))



#define _FSR_STATUS(VAL) ((VAL) & 15)

#define _GET_FSR_STATUS(VAL) ((VAL) & 15)

#define _FSR_DOMAIN(VAL) ((VAL) << 4 & 0xF0)

#define _GET_FSR_DOMAIN(VAL) ((VAL) >> 4 & 15)

#define _FSR_DEBUG_EVENT (1 << 9)

#define _FSR_X_BIT (1 << 10)



#define _Read_FAR \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c6, c0, 0" : "=r" (_val_)); \
   _val_; \
})



/* Cache functions */

#define _Invalidate_I_D_BTB \
asm("mcr\tp15, 0, r0, c7, c7, 0" : : : "memory")



#define _Invalidate_I_BTB \
asm("mcr\tp15, 0, r0, c7, c5, 0")



#define _Invalidate_I_Cache_Line(MVA) \
asm("mcr\tp15, 0, %0, c7, c5, 1" : : "r" (MVA))



#define _Invalidate_D_Cache \
asm("mcr\tp15, 0, r0, c7, c6, 0" : : : "memory")



#define _Invalidate_D_Cache_Line(MVA) \
asm("mcr\tp15, 0, %0, c7, c6, 1" : : "r" (MVA) : "memory")



#define _Clean_D_Cache_Line(MVA) \
asm("mcr\tp15, 0, %0, c7, c10, 1" : : "r" (MVA))



#define _Drain_Write_Buffer \
asm("mcr\tp15, 0, r0, c7, c10, 4")



#define _Invalidate_BTB \
asm("mcr\tp15, 0, r0, c7, c5, 6")



#define _Allocate_D_Cache_Line(MVA) \
asm("mcr\tp15, 0, %0, c7, c2, 5" : : "r" (MVA) : "memory")



/* TLB functions */

#define _Invalidate_I_D_TLB \
asm("mcr\tp15, 0, r0, c8, c7, 0" : : : "memory")



#define _Invalidate_I_TLB \
asm("mcr\tp15, 0, r0, c8, c5, 0")



#define _Invalidate_I_TLB_Entry(MVA) \
asm("mcr\tp15, 0, %0, c8, c5, 1" : : "r" (MVA))



#define _Invalidate_D_TLB \
asm("mcr\tp15, 0, r0, c8, c6, 0" : : : "memory")



#define _Invalidate_D_TLB_Entry(MVA) \
asm("mcr\tp15, 0, %0, c8, c6, 1" : : "r" (MVA) : "memory")



/* Cache Lock functions */



#define _Lock_I_Cache_Line(MVA) \
asm("mcr\tp15, 0, %0, c9, c1, 0" : : "r" (MVA))



#define _Unlock_I_Cache \
asm("mcr\tp15, 0, r0, c9, c1, 1")



#define _Read_D_Cache_Lock \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c9, c2, 0" : "=r" (_val_)); \
   _val_ & 0x1; \
})



#define _Write_D_Cache_Lock(LOCK) \
asm("mcr\tp15, 0, %0, c9, c2, 0" : : "r" (LOCK))



#define _Unlock_D_Cache \
asm("mcr\tp15, 0, r0, c9, c2, 1")


/* TLB Lock functions */



#define _Lock_I_TLB_Entry(MVA) \
asm("mcr\tp15, 0, %0, c10, c4, 0" : : "r" (MVA))



#define _Lock_D_TLB_Entry(MVA) \
asm("mcr\tp15, 0, %0, c10, c8, 0" : : "r" (MVA))



#define _Unlock_I_TLB \
asm("mcr\tp15, 0, r0, c10, c4, 1")



#define _Unlock_D_TLB \
asm("mcr\tp15, 0, r0, c10, c8, 1")



#define _Read_PID \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c13, c0, 0" : "=r" (_val_)); \
   _val_ & 0xFE000000; \
})



#define _Write_PID(PID) \
asm("mcr\tp15, 0, %0, c13, c0, 0" : : "r" (PID))



#define _Read_IBCR0 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c14, c8, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_IBCR0(MVA) \
asm("mcr\tp15, 0, %0, c14, c8, 0" : : "r" (MVA))


#define _Read_IBCR1 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c14, c9, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_IBCR1(MVA) \
asm("mcr\tp15, 0, %0, c14, c9, 0" : : "r" (MVA))


#define _Read_DBR0 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c14, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_DBR0(MVA) \
asm("mcr\tp15, 0, %0, c14, c0, 0" : : "r" (MVA))



#define _Read_DBR1 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c14, c3, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_DBR1(MVA) \
asm("mcr\tp15, 0, %0, c14, c3, 0" : : "r" (MVA))



#define _Read_DBCON \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c14, c4, 0" : "=r" (_val_)); \
   _val_ & ; \
})



#define _Write_DBCON(VAL) \
asm("mcr\tp15, 0, %0, c14, c4, 0" : : "r" (VAL))



#define _DBCON_DBR0_STORE (1 << 0)

#define _DBCON_DBR0_ANY (2 << 0)

#define _DBCON_DBR0_LOAD (3 << 0)

#define _DBCON_DBR1_STORE (1 << 2)

#define _DBCON_DBR1_ANY (2 << 2)

#define _DBCON_DBR1_LOAD (3 << 2)

#define _DBCON_DBR1_MASK (1 << 8)



#define _Read_CAP \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp15, 0, %0, c15, c1, 0" : "=r" (_val_)); \
   _val_ & 0x0000FFFF; \
})



#define _Write_CAP(MASK) \
asm("mcr\tp15, 0, %0, c15, c1, 0" : : "r" (MASK))


/***************************************************************
 * Coprocessor 14 
 **************************************************************/

#define _Read_PMNC \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c0, c0, 0" : "=r" (_val_)); \
   _val_ & 0x0FFFF779; \
})

#define _Write_PMNC(VAL) \
 asm("mcr\tp14, 0, %0, c0, c0, 0" : : "r" (VAL))


#define _PMNC_ENABLE    (1 << 0)

#define _PMNC_EC_RESET (1 << 1)

#define _PMNC_CLK_RESET (1 << 2)

#define _PMNC_DIV_64 (1 << 3)

#define _PMNC_CLK_INT_ENABLE (1 << 6)

#define _PMNC_EC1_INT_ENABLE (1 << 5)

#define _PMNC_EC0_INT_ENABLE (1 << 4)

#define _PMNC_CLK_OVERFLOW (1 << 10)

#define _PMNC_EC1_OVERFLOW (1 << 9)

#define _PMNC_EC0_OVERFLOW (1 << 8)

#define _PMNC_EVENT1(VAL) ((VAL) << 20)

#define _GET_PMNC_EVENT1(VAL) ((VAL) >> 20 & 0x1F)

#define _PMNC_EVENT0(VAL) ((VAL) << 12)

#define _GET_PMNC_EVENT0(VAL) ((VAL) >> 12 & 0x1F)



#define _EVENT_I_CACHE_MISS 0

#define _EVENT_I_CACHE_STALL_CYCLES 1

#define _EVENT_DATA_DEPEND_STALL 2

#define _EVENT_I_TLB_MISSES 3

#define _EVENT_D_TLB_MISSES 4

#define _EVENT_BRANCHES 5

#define _EVENT_BRANCH_MISSPRED 6

#define _EVENT_INSTRUCTIONS 7

#define _EVENT_D_CACHE_STALL_CYCLES 8

#define _EVENT_D_CACHE_STALLS 9

#define _EVENT_D_CACHE_REQ 10

#define _EVENT_D_CACHE_MISS 11

#define _EVENT_D_CACHE_WRITE_BACK 12

#define _EVENT_PC_MODIFIED 13

#define _EVENT_MEMORY_ACCESS 16

#define _EVENT_BCU_QUEUE_FULL 17

#define _EVENT_BCU_DRAINS 18

#define _EVENT_ECC_MISSED 20

#define _EVENT_ECC_SINGLE_BIT 21

#define _EVENT_ECC_RMW 22


#define _Read_CCNT \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c1, c0, 0" : "=r" (_val_)); \
   _val_; \
})

#define _Write_CCNT(VAL) \
 asm("mcr\tp14, 0, %0, c1, c0, 0" : : "r" (VAL))

#define _Read_PMN0 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c2, c0, 0" : "=r" (_val_)); \
   _val_; \
})

#define _Write_PMN0(VAL) \
 asm("mcr\tp14, 0, %0, c2, c0, 0" : : "r" (VAL))

#define _Read_PMN1 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c3, c0, 0" : "=r" (_val_)); \
   _val_; \
})

#define _Write_PMN1(VAL) \
 asm("mcr\tp14, 0, %0, c3, c0, 0" : : "r" (VAL))

#define _Write_CCLKCFG(VAL) \
asm("mcr\tp14, 0, %0, c6, c0, 0" : : "r" (VAL))

#define _GET_CLK_MULT(VAL) (VAL) ((VAL)+2)

#define _SET_CLK_MULT(VAL) (VAL) ((VAL)<3?1:(VAL)>9?7:(VAL)-2)



#define _Read_PWRMODE \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c7, c0, 0" : "=r" (_val_)); \
   _val_ & 0x3; \
})



#define _Write_PWRMODE(VAL) \
asm("mcr\tp14, 0, %0, c7, c0, 0" : : "r" (VAL))



#define _ACTIVE_PWRMODE 0

#define _IDLE_PWRMODE 1

#define _DROWSY_PWRMODE 2

#define _SLEEP_PWRMODE 3



#define _Read_TX_Debug \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c8, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_TX_Debug(VAL) \
asm("mcr\tp14, 0, %0, c8, c0, 0" : : "r" (VAL))



#define _Read_RX_Debug \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c9, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_RX_Debug(VAL) \
asm("mcr\tp14, 0, %0, c9, c0, 0" : : "r" (VAL))



#define _Read_Debug_DCSR \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c10, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_Debug_DCSR(VAL) \
asm("mcr\tp14, 0, %0, c10, c0, 0" : : "r" (VAL))



#define _DCSR_GE (1 << 31)

#define _DCSR_H  (1 << 30)

#define _DCSR_TF (1 << 23)

#define _DCSR_TI (1 << 22)

#define _DCSR_TD (1 << 20)

#define _DCSR_TA (1 << 19)

#define _DCSR_TS (1 << 18)

#define _DCSR_TU (1 << 17)

#define _DCSR_TR (1 << 16)

#define _DCSR_SA (1 << 5)

#define _DCSR_MOE(VAL) ((VAL) << 2 & 0x1C)

#define _GET_DCSR_MOE(VAL) ((VAL) >> 2 & 7)

#define _DCSR_M (1 << 1)

#define _DCSR_E (1 << 0)



#define _MOE_RESET 0

#define _MOE_INS_BKPT 1

#define _MOE_DATA_BKPT 2

#define _MOE_BKPT 3

#define _MOE_DEBUG 4

#define _MOE_TRAP 5

#define _MOE_TRACE_FULL 6



#define _Read_Trace_Buffer \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c11, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_Trace_Buffer(VAL) \
asm("mcr\tp14, 0, %0, c11, c0, 0" : : "r" (VAL))



#define _Read_Checkpoint_0 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c12, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_Checkpoint_0(VAL) \
asm("mcr\tp14, 0, %0, c12, c0, 0" : : "r" (VAL))



#define _Read_Checkpoint_1 \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c13, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_Checkpoint_1(VAL) \
asm("mcr\tp14, 0, %0, c13, c0, 0" : : "r" (VAL))



#define _Read_TX_RX_Control \
({ REGISTER unsigned _val_; \
   asm volatile("mrc\tp14, 0, %0, c14, c0, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_TX_RX_Control(VAL) \
asm("mcr\tp14, 0, %0, c14, c0, 0" : : "r" (VAL))



/***************************************************************
 * Coprocessor 13 
 **************************************************************/

#define _Read_INT_Control \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c0, c0, 0" : "=r" (_val_)); \
   _val_ & 0xF; \
})

#define _Write_INT_Control(VAL) \
 asm("mcr\tp13, 0, %0, c0, c0, 0" : : "r" (VAL))

#define _INTCTL_BCU_ENABLE (1 << 3)
#define _INTCTL_PMU_ENABLE (1 << 2)
#define _INTCTL_IRQ_ENABLE (1 << 1)
#define _INTCTL_FIQ_ENABLE (1 << 0)

#define _Read_INT_Source \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c4, c0, 0" : "=r" (_val_)); \
   _val_; \
})

#define _INTSRC_FIQ_ACTIVE (1 << 31)
#define _INTSRC_IRQ_ACTIVE (1 << 30)
#define _INTSRC_BCU_ACTIVE (1 << 29)
#define _INTSRC_PMU_ACTIVE (1 << 28)

#define _Read_INT_Steering \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c8, c0, 0" : "=r" (_val_)); \
   _val_ & 0x3; \
})

#define _Write_INT_Steering(VAL) \
 asm("mcr\tp13, 0, %0, c8, c0, 0" : : "r" (VAL))

#define _INTSTR_BCU_FIQ (1 << 1)
#define _INTSTR_BCU_IRQ (0)
#define _INTSTR_PMU_FIQ (1 << 0)
#define _INTSTR_PMU_IRQ (0)


/* D. Kranak addition (inserted by avb) */
/* Get high bits into flags */
#define _WaitForNoIRQ() \
 asm("1:\tmrc\tp13, 0, r15, c4, c0, 0\n\tnop\n\tbeq\t1b")

#define _WaitForNoFIQ() \
 asm("1:\tmrc\tp13, 0, r15, c4, c0, 0\n\tnop\n\tbmi\t1b")

#define _WaitForNoBCU() \
 asm("1:\tmrc\tp13, 0, r15, c4, c0, 0\n\tnop\n\tbcs\t1b")

#define _WaitForNoPMU() \
 asm("1:\tmrc\tp13, 0, r15, c4, c0, 0\n\tnop\n\tbvs\t1b")


#define _Read_BCU_Control \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c0, c1, 0" : "=r" (_val_)); \
   (_val_ & 0xF000000F) | 0x2; \
})



#define _Write_BCU_Control(VAL) \
asm("mcr\tp13, 0, %0, c0, c1, 0" : : "r" (VAL))



#define _Wait_While_BCU_Pending \
asm("1:\tmrc\tp13, 0, r15, c0, c1, 0\n\tbmi\t1b")



#define _BCUCTL_PENDING (1 << 31)

#define _BCUCTL_OVERFLOW (1 << 30)

#define _BCUCTL_ELOG1 (1 << 29)

#define _BCUCTL_ELOG0 (1 << 28)

#define _BCUCTL_ECC (1 << 3)

#define _BCUCTL_CORRECT (1 << 2)

#define _BCUCTL_REPORT (1 << 0)


#define _Write_BCUMOD(VAL) \
asm("mcr\tp13, 0, %0, c1, c1, 0" : : "r" (VAL))



#define _BCUMOD_ALIGNED_FETCH (1 << 0)



#define _Read_ELOG0 \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c4, c1, 0" : "=r" (_val_)); \
   _val_ & 0xE00000FF; \
})



#define _Write_ELOG0(VAL) \
asm("mcr\tp13, 0, %0, c4, c1, 0" : : "r" (VAL))



#define _Read_ELOG1 \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c5, c1, 0" : "=r" (_val_)); \
   _val_ & 0xE00000FF; \
})



#define _Write_ELOG1(VAL) \
asm("mcr\tp13, 0, %0, c5, c1, 0" : : "r" (VAL))



#define _ELOG_WRITE (1 << 31)

#define _ELOG_READ  (0)

#define _ELOG_SINGLE_BIT (0 << 29)

#define _ELOG_MULTI_BIT  (1 << 29)

#define _ELOG_BUS_ABORT  (2 << 29)

#define _ELOG_ERROR(VAL) ((VAL) << 29 & (3 << 29))

#define _GET_ELOG_ERROR(VAL) ((VAL) >> 29 & 3)

#define _ELOG_SYNDROME(VAL) ((VAL)&0xFF)



#define _Read_ECAR0 \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c6, c1, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_ECAR0(VAL) \
asm("mcr\tp13, 0, %0, c6, c1, 0" : : "r" (VAL))



#define _Read_ECAR1 \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c7, c1, 0" : "=r" (_val_)); \
   _val_; \
})



#define _Write_ECAR1(VAL) \
asm("mcr\tp13, 0, %0, c7, c1, 0" : : "r" (VAL))



#define _Read_ECTST \
({ REGISTER unsigned _val_; \
   asm("mrc\tp13, 0, %0, c8, c1, 0" : "=r" (_val_)); \
   _val_ & 0xF; \
})



#define _Write_ECTST(VAL) \
asm("mcr\tp13, 0, %0, c8, c1, 0" : : "r" (VAL))



/***************************************************************
 * Misc
 **************************************************************/

#define _CPWAIT \
asm("mrc\tp15, 0, r0, c2, c0, 0\n\t" \
	"mov\tr0, r0\n\t" \
	"sub\tpc, pc, #4" : : : "r0", "memory")

#define _Data_Prefetch(ADR) \
asm("pld\t%0" : : "m" (ADR))

#define _Read_CPSR \
({ REGISTER unsigned _val_; \
   asm volatile("mrs\t%0, cpsr" : "=r" (_val_)); \
   _val_; \
})

#define _Write_CPSR(VAL) \
asm("msr\tcpsr, %0" : : "rI" (VAL) : "cc", "memory")

#define _Write_MODE(VAL) \
asm("msr\tcpsr_c, %0" : : "rI" (VAL) : "memory")

#define _Write_FLAGS(VAL) \
asm("msr\tcpsr_f, %0" : : "rI" (VAL) : "cc")


#define _IRQ_MODE	0x12

#define _FIQ_MODE	0x11

#define _ABORT_MODE	0x17

#define _UNDEF_MODE	0x1B

#define _SYSTEM_MODE	0x1F

#define _SVC_MODE	0x13

#define _USER_MODE	0x10



#define _CPSR_MODE(VAL) ((VAL) & 0x1F)

#define _GET_CPSR_MODE(VAL) ((VAL) & 0x1F)



#define _THUMB_FLAG (1 << 5)

#define _FIQ_DISABLE (1 << 6)

#define _IRQ_DISABLE (1 << 7)



#endif

