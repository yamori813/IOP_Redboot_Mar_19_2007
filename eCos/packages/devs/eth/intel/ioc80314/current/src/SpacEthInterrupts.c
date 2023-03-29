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
* FILENAME: 1.	SpacEthInterrupts.c
*
* DESCRIPTION: Contains implementations of routines servicing SPac's GigE 
*            : interrupts.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added test routines. 
* NOTE: 
*
*----------------------------------------------------------------------------*/


#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_intel_ioc80314.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
// So we can check the validity of the PCI window against the MLTs opinion,
// and thereby what the malloc heap consumes willy-nilly:
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif

#include <cyg/devs/eth/ioc80314_info.h>



/* This is used to map SDRAM eror interrupts, for debugging purposes.  */
// #define __GIGE_SDRAM_FIQ_DEBUG__

  /* Global flags. */
/*  extern volatile BOOL doGigE5_3;                              
  extern volatile BOOL doGigE5_4and6_1;
  extern volatile BOOL doGigE9_1;
  extern volatile BOOL doGigE7_9;
  volatile BOOL doGigE6_2and6_7 = FALSE;
  extern volatile BOOL doGigE8_3and6_3;
  extern volatile BOOL doGigE8_4and6_3;
  extern volatile BOOL doGigE6_4and7_8;
  volatile BOOL doGigE6_5 = FALSE;
  volatile BOOL doGigE6_6 = FALSE;
  volatile BOOL doGigE6_8 = FALSE;
  extern volatile BOOL doGigE7_2;
  extern volatile BOOL doGigE7_4;
  extern volatile BOOL doGigE8_1and8_2;
*/
    
  ISR_SERVICE gige_next_irq_service = 0;
  ISR_SERVICE gige_next_fiq_service = 0;
/* Dispatching table for external interrupts */ 
  SpacEthRtnCode_t (*fl_vector_table[MAX_USER_VECTORS])(void *); 
  
//  char gigeFiqStack[4 * 1024]__attribute__ ((section("STACK")))= {0};
  char gigeIrqStack[1024]__attribute__ ((section("STACK")))= {0};
  
/* SPac chip base address. */
  extern  UINT32 SpacBaseAddr;

/* SPac GigE driver global object. */
extern Eth_t EthDriver; 

/* CPU id. */
UINT32 GigeCpuId = 1;

/* IACK and EOI register. Discriminated at init time. */
static UINT32 GigeFiqIack = HAL_IOC80314_CSR_BASE + ETH_MPIC_IACK1_REG;
static UINT32 GigeFiqEoi = HAL_IOC80314_CSR_BASE + ETH_MPIC_EOI1_REG;
static UINT32 GigeIrqIack = HAL_IOC80314_CSR_BASE + ETH_MPIC_IACK3_REG;
static UINT32 GigeIrqEoi = HAL_IOC80314_CSR_BASE + ETH_MPIC_EOI3_REG;


/* The mapping is according to "(ETH_MAX_NUM_INTR_SOURCES - (tableIndex + 1))" 
   formula. for instance, at tableIndex = 1, EthIsrTable[1] contains the 
   OCN ERROR interrupt handler. */
typedef SpacEthRtnCode_t (* EthSourceIsrPtr_t)(UINT32 portId, UINT32 info);
const EthSourceIsrPtr_t EthIsrTable[ETH_MAX_NUM_INTR_SOURCES] = 
  {
/* interrupt status bit 31. */ NULL,
/* interrupt status bit 30. */ EthOcnErrIsr,
/* interrupt status bit 29. */ EthRxIdleIsr,
/* interrupt status bit 28. */ EthRxAbortIsr,
/* interrupt status bit 27. */ EthRxErrorIsr,
/* interrupt status bit 26. */ EthRxOverrunIsr,
/* interrupt status bit 25. */ EthRxThresholdIsr,
/* interrupt status bit 24. */ EthRxWaitIsr,
/* interrupt status bit 23. */ NULL,
/* interrupt status bit 22. */ NULL,
/* interrupt status bit 21. */ NULL,
/* interrupt status bit 20. */ NULL,
/* interrupt status bit 19. */ EthRxQueueIsr,
/* interrupt status bit 18. */ EthRxQueueIsr,
/* interrupt status bit 17. */ EthRxQueueIsr,
/* interrupt status bit 16. */ EthRxQueueIsr,
/* interrupt status bit 15. */ EthStatsCarryIsr,
/* interrupt status bit 14. */ NULL,
/* interrupt status bit 13. */ EthTxIdleIsr,
/* interrupt status bit 12. */ EthTxAbortIsr,
/* interrupt status bit 11. */ EthTxErrorIsr,
/* interrupt status bit 10. */ EthTxUnderrunIsr,
/* interrupt status bit 9. */  EthTxThresholdIsr,
/* interrupt status bit 8. */  EthTxWaitIsr,
/* interrupt status bit 7. */  NULL,
/* interrupt status bit 6. */  NULL,
/* interrupt status bit 5. */  NULL,
/* interrupt status bit 4. */  NULL,
/* interrupt status bit 3. */  EthTxQueueIsr,
/* interrupt status bit 2. */  EthTxQueueIsr,
/* interrupt status bit 1. */  EthTxQueueIsr,
/* interrupt status bit 0. */  EthTxQueueIsr,
  };
  
/* Test status table. */
  extern EthTestDone_t ethTestStatus[];
/* Test 6_2 status table. */
  Eth6_2Port_t test6_2Status[ETH_NUM_PORTS] = 
    {{{{{FALSE, FALSE, FALSE}}, {{FALSE, FALSE, FALSE}}}}, {{{{FALSE, FALSE, FALSE}}, {{FALSE, FALSE, FALSE}}}}};

#ifdef ETH_DIAG_EXTENDED
  UINT32 qInterruptsTable[ETH_NUM_PORTS][ETH_NUM_PORT_DIRECTIONS][ETH_NUM_Q_INTR] = 
    {{{{{0x0, 0x0, 0x0}}, {{0x0, 0x0, 0x0}}}}, {{{{0x0, 0x0, 0x0}}, {{0x0, 0x0, 0x0}}}}};
#endif  //  ETH_DIAG_EXTENDED

/*=========================================================================
 *  Function    : intMpicInit
 *
 *  Arguments   : N/A.
 * 
 *  Description : Initializes the MPIC block.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_INVAL_CFG_ERR - Invalid configuration error. 
 *
 *  Note        : for now GigE-oriented. 
 *           
 =========================================================================*/
SpacEthRtnCode_t intMpicInit(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 cpsr;
  
  GigeCpuId = 
    (*(REG32_PTR(SpacBaseAddr + ETH_CIU_CFG_REG)) & ETH_CIU_PID_MASK) >> ETH_BYTE_WIDTH;
  
  if((GigeCpuId != ETH_CPU0_ID) && (GigeCpuId != ETH_CPU1_ID)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <intMpicInit>, invalid CPU id = %d.", GigeCpuId);
    #endif // DEBUG_OVI
    return ETH_INVAL_CFG_ERR;
  } 
  
/* Make sure that interrupts are disabled */
  cpsr = _Read_CPSR;
  _Write_CPSR(cpsr | _IRQ_DISABLE | _FIQ_DISABLE);

  rtnCode = gigeHandlerAttachVector(0, ETH_MPIC_GIGE_VECTOR, EthIsr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <intMpicInit>, <gigeHandlerAttachVector> returns %d.", rtnCode);
    #endif // DEBUG_OVI
  } 
  
  gigeHandlerAttachIRQ(SpacIrqIsr);
  
#ifdef __GIGE_SDRAM_FIQ_DEBUG__     
  gigeHandlerAttachFIQ(SpacFiqIsr);
#endif

    /* Make sure that interrupt vectors will be updated correctly */

    // Force new interrupt handler settings out of the Data Cache
    // and Write Buffer.
    _Clean_D_Cache_Line(IRQVECTOR/*FIQVECTOR*/);
    _CPWAIT;
    _Invalidate_D_Cache_Line(IRQVECTOR/*FIQVECTOR*/);
    _CPWAIT;
    _Drain_Write_Buffer;
    _CPWAIT;
    // Invalidate Instruction Cache and BTB
    _Invalidate_I_BTB;
    _CPWAIT;

/* Set IRQ to be enabled later */  
  cpsr &= ~_IRQ_DISABLE;     
  
#ifdef __GIGE_SDRAM_FIQ_DEBUG__     
/* Set FIQ to be enabled later */  
  cpsr &= ~_FIQ_DISABLE;     
#endif    

/* Enable IRQs. */  
  _Write_INT_Control(ETH_XSCALE_INTCTL_MASK);
  
#ifdef __GIGE_SDRAM_FIQ_DEBUG__        
/* Enable FIQs and IRQs. */  
//  _Write_INT_Control(3);  
/* Enable FIQs. */  
  _Write_INT_Control(1);  
#endif    
  
  if(GigeCpuId == ETH_CPU0_ID){
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_CSR2_REG)) = (1 << 1);  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_TASK2_REG)) = ETH_MPIC_TASK_MASK;  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IDR16_REG)) = ETH_MPIC_IDR_OUT3_MASK;
    GigeIrqIack = SpacBaseAddr + ETH_MPIC_IACK2_REG;
    GigeIrqEoi = SpacBaseAddr + ETH_MPIC_EOI2_REG;
    
#ifdef __GIGE_SDRAM_FIQ_DEBUG__              
    
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_CSR0_REG)) = (1 << 1);  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_TASK0_REG)) = ETH_MPIC_TASK_MASK;  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IDR16_REG)) = ETH_MPIC_IDR_OUT1_MASK;
//    *(REG32_PTR(SpacBaseAddr + (ETH_MPIC_BASE_ADDR_LO + 0x19C))) = ETH_MPIC_IDR_OUT1_MASK;        
    GigeFiqIack = SpacBaseAddr + ETH_MPIC_IACK0_REG;
    GigeFiqEoi = SpacBaseAddr + ETH_MPIC_EOI0_REG;
    
#endif    

  }else{
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_CSR3_REG)) = (1 << 1);  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_TASK3_REG)) = ETH_MPIC_TASK_MASK;  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IDR16_REG)) = ETH_MPIC_IDR_OUT4_MASK;
    GigeIrqIack = SpacBaseAddr + ETH_MPIC_IACK3_REG;
    GigeIrqEoi = SpacBaseAddr + ETH_MPIC_EOI3_REG;
    
#ifdef __GIGE_SDRAM_FIQ_DEBUG__              
    
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_CSR1_REG)) = (1 << 1);  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_TASK1_REG)) = ETH_MPIC_TASK_MASK;  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IDR16_REG)) = ETH_MPIC_IDR_OUT2_MASK;
//    *(REG32_PTR(SpacBaseAddr + (ETH_MPIC_BASE_ADDR_LO + 0x19C))) = ETH_MPIC_IDR_OUT2_MASK;       
    GigeFiqIack = SpacBaseAddr + ETH_MPIC_IACK1_REG;
    GigeFiqEoi = SpacBaseAddr + ETH_MPIC_EOI1_REG;

#endif     
    
  }
  
  *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IVPR16_REG)) = ETH_MPIC_IVPR_PRIO_MASK | 
    ETH_MPIC_IVPR_S_MASK | ETH_MPIC_IVPR_P_MASK | ETH_MPIC_GIGE_VECTOR;  
   
#ifdef __GIGE_SDRAM_FIQ_DEBUG__          
//  *(REG32_PTR(SpacBaseAddr + (ETH_MPIC_BASE_ADDR_LO + 0x198))) = ETH_MPIC_IVPR_PRIO_MASK | 
//    ETH_MPIC_IVPR_S_MASK | ETH_MPIC_IVPR_P_MASK | 0x13;  
//    
    
#endif     

/* Enable interrupts */
  _Write_CPSR(cpsr);
    
  #ifdef DEBUG_OVI
    db_printf("\nIn <intMpicInit>, GIGE ISRs routed to CPU%d.", GigeCpuId);
  #endif // DEBUG_OVI
    
  return rtnCode;
  
}
  
/*=========================================================================
 *  Function    : intMpicReset
 *
 *  Arguments   : N/A.
 * 
 *  Description : Resets the MPIC block.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : for now GigE-oriented. 
 *           
 =========================================================================*/
SpacEthRtnCode_t intMpicReset(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if (EthDriver.intEnabled == 2) 
  {
    EthDriver.intEnabled--;
    return rtnCode;
  }
//  gigeHandlerDetachIRQ();

/* Need to detach interrupts here - FIXME cebruns
    cyg_drv_interrupt_detach(p_ioc80314->interrupt_handle);
*/   

  
#ifdef __GIGE_SDRAM_FIQ_DEBUG__     
  gigeHandlerDetachFIQ();
#endif
  
/*  *(REG32_PTR(SpacBaseAddr + ETH_MPIC_GCR_REG)) = TS_MPIC_GCR_R; */
  EthDriver.intEnabled--; 
  return rtnCode;
  
}
  
/*=========================================================================
 *  Function    : SpacIrqIsr
 *
 *  Arguments   : N/A.
 * 
 *  Description : SPac's ISR.
 * 
 *  return      : N/A.
 *
 *  Note        : for now servicing only IRQ GigE interrupts. They are expected
 *              : at output 1 only.
 *              : See note in SpacEthInterrupts.h file about the usage of the
 *              : IRQ_OVI_EPILOG macro.
 *              
 =========================================================================*/
void SpacIrqIsr(void)
{
    
  SpacEthRtnCode_t rtnCode = ETH_OK; 
  UINT32 irqVal = 0x00000000;
  UINT32 iAckVal = 0x00000000;  
  
/* Identify the source of interrupt. */
  irqVal = _Read_INT_Source;
  if(irqVal & _INTSRC_IRQ_ACTIVE) /* Check IRQ# pin. */
  {
/* IACK cycle. */
    iAckVal = *(REG32_PTR(GigeIrqIack));  
    irqVal = iAckVal & ETH_MPIC_IACK_MASK;  
    if((irqVal < MAX_USER_VECTORS) && (NULL != fl_vector_table[irqVal]))
    {
/* Call a service function. */
      rtnCode = fl_vector_table[irqVal]((void *)&EthDriver);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <SpacIrqIsr>, ISR handler at vector %d returns %d.", 
            irqVal, rtnCode);
        #endif // DEBUG_OVI
      } 
    }else{        
      #ifdef DEBUG_OVI
        db_printf("\nIn <SpacIrqIsr>, not handled interrupt, IACK is %08X.", 
          iAckVal);      
      #endif // DEBUG_OVI
    }
/* EOI cycle. */
    *(REG32_PTR(GigeIrqEoi)) = ETH_MPIC_EOI_MASK;  
    _WaitForNoIRQ();          
  }
  else
  {
    #ifdef DEBUG_OVI
      db_printf("\nIn <SpacIrqIsr>, not reported interrupt for vector %d.", 
        irqVal);
    #endif // DEBUG_OVI
/* Call the redboot interupt handler */
/* GoTo "OldVectorValue" stored in intHandlerAttach() */
    EXCEPTION_CHAIN(gige_next_irq_service);
  }

  IRQ_OVI_EPILOG;
  
}

/*=========================================================================
 *  Function    : SpacFiqIsr
 *
 *  Arguments   : N/A.
 * 
 *  Description : SPac's ISR.
 * 
 *  return      : N/A.
 *
 *  Note        : for now servicing only FIQ GigE interrupts. They are expected
 *              : at output 0 only.
 *              
 =========================================================================*/
void SpacFiqIsr(void)
{
  
// __OVI__   just to have a breakpoint placeholder.     
  asm volatile("mov r0,r0");
   
//  static SpacEthRtnCode_t rtnCode = ETH_OK;  
//  static UINT32 fiqVal = 0x00000000;
//  static UINT32 iAckVal = 0x00000000; 
//   
///* Identify the source of interrupt. */
//  fiqVal = _Read_INT_Source;
//  if(fiqVal & _INTSRC_FIQ_ACTIVE) /* Check FIQ# pin. */
//  {
///* IACK cycle. */
//    iAckVal = *(REG32_PTR(GigeFiqIack));
//    fiqVal = iAckVal & ETH_MPIC_IACK_MASK;  
//    if((fiqVal < MAX_USER_VECTORS) && (NULL != fl_vector_table[fiqVal]))
//    {
///* Call a service function. */
////      *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IVPR16_REG)) |= ETH_MPIC_IVPR_M_MASK; 
//      rtnCode = fl_vector_table[fiqVal]((void *)&EthDriver);
////      *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IVPR16_REG)) &= ~ETH_MPIC_IVPR_M_MASK; 
//      if(rtnCode != ETH_OK){
//        #ifdef DEBUG_OVI
//          db_printf("\nIn <SpacFiqIsr>, ISR handler at vector %d returns %d.", 
//            fiqVal, rtnCode);
//        #endif // DEBUG_OVI
//      } 
//    }else{
//      #ifdef DEBUG_OVI
//        db_printf("\nIn <SpacFiqIsr>, not handled interrupt, IACK is %08X.", 
//          iAckVal);
//      #endif // DEBUG_OVI
//    }
///* EOI cycle. */
//    *(REG32_PTR(GigeFiqEoi)) = ETH_MPIC_EOI_MASK;  
//    _WaitForNoFIQ();      
//  }
//  else
//  {
//    #ifdef DEBUG_OVI
//      db_printf("\nIn <SpacFiqIsr>, not not reported interrupt for vector %d.", 
//        fiqVal);
//    #endif // DEBUG_OVI    
///* Call the redboot interupt handler */
///* GoTo "OldVectorValue" stored in intHandlerAttach() */
//    EXCEPTION_CHAIN(gige_next_fiq_service);
//  }
//

}

/*=========================================================================
 *  Function    : gigeHandlerAttachIRQ
 *
 *  Arguments   : irq_service - ISR.
 * 
 *  Description : SPac's ISR attach routine.
 * 
 *  return      : N/A.
 *
 =========================================================================*/
void gigeHandlerAttachIRQ(ISR_SERVICE irq_service)
{
  
  static unsigned int cpsr;
  
/* Setup stack for the IRQ mode. */
  cpsr = _Read_CPSR;
  _Write_MODE(0xd2);  /* Set IRQ Mode (I and F disabled) */
  INIT_STACK_PTR(gigeIrqStack + sizeof(gigeIrqStack)); // Set SP_irq
/* Restore CPSR */
  _Write_CPSR(cpsr);
  
  gige_next_irq_service=*(ISR_SERVICE*)IRQVECTOR;  /* Save current IRQ vector */
  *(ISR_SERVICE*)IRQVECTOR = irq_service;     /* Set new vector */
    
}

/*=========================================================================
 *  Function    : gigeHandlerAttachFIQ
 *
 *  Arguments   : irq_service - ISR.
 * 
 *  Description : SPac's ISR attach routine.
 * 
 *  return      : N/A.
 *
 =========================================================================*/
void gigeHandlerAttachFIQ(ISR_SERVICE fiq_service)
{
  
///* Setup stack for the FIQ mode. */
//  cpsr = _Read_CPSR;
//  _Write_MODE(0xd1);  /* Set FIQ Mode (I and F disabled) */
//  INIT_STACK_PTR(gigeFiqStack + sizeof(gigeFiqStack)); // Set SP_fiq
///* Restore CPSR */
//  _Write_CPSR(cpsr);
//  

/* Set new vector for FIQ interrupts */
  gige_next_fiq_service = *(ISR_SERVICE*)FIQVECTOR;  /* Save current FIQ vector */
  *(ISR_SERVICE*)FIQVECTOR = fiq_service;       /* Place new vector value */
}

/*=========================================================================
 *  Function    : gigeHandlerDetachIRQ
 *
 *  Arguments   : N/A.
 * 
 *  Description : SPac's ISR dettach routine.
 * 
 *  return      : N/A.
 *
 =========================================================================*/
void gigeHandlerDetachIRQ(void)
{
  *IRQVECTOR = (unsigned int)gige_next_irq_service;
}

/*=========================================================================
 *  Function    : gigeHandlerDetachFIQ
 *
 *  Arguments   : N/A.
 * 
 *  Description : SPac's ISR dettach routine.
 * 
 *  return      : N/A.
 *
 =========================================================================*/
void gigeHandlerDetachFIQ(void)
{
  *FIQVECTOR = (unsigned int)gige_next_fiq_service;
}

/*=========================================================================
 *  Function    : gigeHandlerAttachVector
 *
 *  Arguments   : req_type - IRQ/FIQ (not used for now).
 *              : vector - index into the vector table.
 *              : int_routine - vector handler to attach. 
 * 
 *  Description : Attach service routine for interrupt vector.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *              
 =========================================================================*/
SpacEthRtnCode_t gigeHandlerAttachVector(
  int req_type,  
  unsigned vector, 
  SpacEthRtnCode_t (*int_routine)(void *))
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(vector >= MAX_USER_VECTORS)
  {
    #ifdef DEBUG_OVI
      db_printf("\nIn <gigeHandlerAttachVector>, invalid vector parameter = %d.", 
        vector);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

  *(fl_vector_table + vector) = int_routine;
  
  return rtnCode;
  
}

/*=========================================================================
 *  Function    : EthIsr
 *
 *  Arguments   : infoPtr - input, user info passed in.
 * 
 *  Description : GigE's ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : Port0 will get first chance to service its interrupts.
 *              
 =========================================================================*/
SpacEthRtnCode_t EthIsr(
  void * infoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  int idx;
  EthPortIntrStatus_t gigeIntrStatus[ETH_NUM_PORTS];
  static int portIdx = ETH_PORT0_ID;
  
  
  if(infoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthIsr>, invalid info pointer = %p.", infoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }
  
  for(idx = 0; idx < ETH_NUM_PORTS; idx++){  
  /* Read both ports' interrupt status register, to avoid spurious interrupts. */      
    rtnCode = EthRegAccess(idx, ETH_PORT_INT_STATUS_REG, 
      (UINT32 *)&(gigeIntrStatus[idx]), REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthIsr>, <EthRegAccess> access %d to %08X \
          for port %d returns %d.", REG_READ, ETH_PORT_INT_STATUS_REG, 
          idx, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    if(gigeIntrStatus[idx].intrStatus == ETH_BIT_SET){
      rtnCode = EthRegAccess(idx, ETH_PORT_INT_STATUS_REG, 
        (UINT32 *)&(gigeIntrStatus[idx]), REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthIsr>, <EthRegAccess> access %d to %08X, \
            port %d, returns %d.", REG_WRITE, ETH_PORT_INT_STATUS_REG, 
            idx, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;   
      } 
    }    
  } 
      
  for(idx = 0; idx < ETH_NUM_PORTS; idx++){
    if(gigeIntrStatus[portIdx].intrStatus == ETH_BIT_SET){
      rtnCode = EthPortIsr(portIdx, 
        *((UINT32 *)&(gigeIntrStatus[portIdx])) & ~ETH_PORT_INT_STATUS_MASK, 
        infoPtr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthIsr>, <EthPortIsr> port %d returns %d.", 
            portIdx, rtnCode);
        #endif // DEBUG_OVI
/* Give a chance to service next port.       
        return rtnCode; */  
      }   
    } 
    portIdx = (portIdx == ETH_PORT0_ID ? ETH_PORT1_ID : ETH_PORT0_ID);
  }

  portIdx = (portIdx == ETH_PORT0_ID ? ETH_PORT1_ID : ETH_PORT0_ID);

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthPortIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : intrStatus - Port interrupt status.
 *              : infoPtr - input, user info passed in. 
 * 
 *  Description : GigE's ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : Prior to returning, this routine, attempts to clear the 
 *              : interrupt sources.
 =========================================================================*/
SpacEthRtnCode_t EthPortIsr(
  UINT32 portId, 
  UINT32 intrStatus,   
  void * infoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 intrMask;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPortIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(infoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPortIsr>, invalid info pointer = %p.", infoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
    
  rtnCode = EthRegAccess(portId, ETH_PORT_INT_MASK_REG, &intrMask, REG_READ);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPortIsr>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_READ, ETH_PORT_INT_MASK_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  {
    register unsigned idx, statusBit, isrParam, loopCounter = 0;
/* If there are any interrupts, dispatch them. */      
    do{
/* Get the index into the EthIsrTable. */ 
      __asm volatile ("CLZ %0, %1": "=r" (idx) : "r" (intrStatus));
  		
      if(idx == ETH_ARM_CLZ_BIT_NOT_SET){
        break;
      }
      statusBit = ETH_MAX_NUM_INTR_SOURCES - (idx + 1);
/* If the interrupt is enabled, call its handler. */      
      if((intrMask & (1 << statusBit)) == 
        ETH_BIT_CLEARED){
        if(EthIsrTable[idx] != NULL){
          if((intrStatus & ETH_TX_Q_WAIT_INTR_MASK) ||
            (intrStatus & ETH_RX_Q_WAIT_INTR_MASK)){
            isrParam = (UINT32)(&intrStatus);
          }else{
            isrParam = statusBit;
          }
          rtnCode = EthIsrTable[idx](portId, isrParam);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthPortIsr>, <EthIsrTable[%d]> returns %d.", 
                idx, rtnCode);
            #endif // DEBUG_OVI
/* Give a chance to service next interrupt source.       
            return rtnCode; */  
          }
        }else{
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthPortIsr>, could not found handler for GigE \
              interrupt source %d.", statusBit);
          #endif // DEBUG_OVI
/* Give a chance to service next interrupt source.       
          return rtnCode; */  
        }
      }
/* Clear the serviced interrupt source bit. */      
      intrStatus &= ~(1 << statusBit);
/* Paranoid redundancy mechanism, in case something is VERY wrong with the 
   CLZ instruction. */      
      loopCounter++;
    }while(loopCounter < ETH_MAX_NUM_INTR_SOURCES);
  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthOcnErrIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's OCN ERROR ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthOcnErrIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthOcnErrIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

/* The interrupt for Port1 should be masked, to reduce system overhead. */  
  if(portId == ETH_PORT1_ID){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthOcnErrIsr>, warning, OCN ERROR interrupt for port \
        id = %d is not masked.", portId);
    #endif // DEBUG_OVI
//    return ETH_PARAM_ERR;
  } 
  
  {
    EthPortOcnStatus_t portOcnStatus;
    int idx;
    for(idx = 0; idx < ETH_NUM_PORTS; idx++){
      rtnCode = EthRegAccess(idx, ETH_PORT_OCN_STATUS_REG, 
        (UINT32 *)&portOcnStatus, REG_READ);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthOcnErrIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_READ, ETH_PORT_OCN_STATUS_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
      
      #ifdef DEBUG_OVI
      db_printf("\nIn <EthOcnErrIsr>, OCN error count = %d, %s OCN response \
        header is %08X.", portOcnStatus.ocnErrCount,
        (idx == ETH_PORT0_ID ? "first" : "last"), 
        portOcnStatus.ocnResponseHdr);
      #endif // DEBUG_OVI
            
  /* Clear error count field(s). */  
      portOcnStatus.ocnErrCount = 0x00;
      rtnCode = EthRegAccess(idx, ETH_PORT_OCN_STATUS_REG, 
        (UINT32 *)&portOcnStatus, REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthOcnErrIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_PORT_OCN_STATUS_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
    }
  }
  
/* T.B.D what else to do here. */  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxIdleIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX IDLE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxIdleIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxIdleIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

  {
    EthRxStatus_t rxStatus;
/* Get the idle queue(s). */      
    rtnCode = EthRegAccess(portId, ETH_RX_STATUS_REG, (UINT32 *)&rxStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxIdleIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxIdleIsr>, RX queue idle field = %02X.", 
          rxStatus.qIdle);
      #endif // DEBUG_OVI
    }
  }
        
/* Signal a semaphore, if needed. */  

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxAbortIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX ABORT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxAbortIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxAbortIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  #ifdef DEBUG_OVI
    db_printf("\nIn <EthRxAbortIsr>, port id %d, RX abort completed.", portId);
  #endif // DEBUG_OVI
  
#ifdef ETH_REDBOOT  
    
  ethTestStatus[ETH_TEST_6_4and7_8].port[portId].direction[ETH_RX] = ETH_TEST_PASSED;
  

#endif
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxErrorIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX ERROR ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxErrorIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxErrorIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  {
  UINT32 xStatus, errStatus; 
  int idx; 
/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &xStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Get the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_ERR_STATUS_REG, &errStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_ERR_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    
    for(idx = (ETH_NUM_RX_Q - 1); idx >= 0; idx--){
      if(xStatus & (ETH_RX_Q0_ERR_MASK << idx)){
/* Shouldn't happen, for now. */        
        if(errStatus & (ETH_RX_Q0_DESC_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, descriptor error in queue %d.", idx);
          #endif // DEBUG_OVI
        }
        if(errStatus & (ETH_RX_Q0_TIMEOUT_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, timeout error in queue %d.", idx);
          #endif // DEBUG_OVI
        }
        if(errStatus & (ETH_RX_Q0_READ_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, read error in queue %d, target \
              response is %02X", idx, 
              (errStatus & (ETH_RX_Q0_TRESP_ERR_MASK << (idx * ETH_BYTE_WIDTH))));
          #endif // DEBUG_OVI
        }
        if(errStatus & (ETH_RX_Q0_TEA_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, TEA error in queue %d.", idx);
          #endif // DEBUG_OVI
        }
      }
    }
    
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &xStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Clear the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_ERR_STATUS_REG, &errStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_ERR_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
  }
    
/* According to the specs, chip reset may be needed. TBD. 
  EthReset(); 
*/    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxOverrunIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX OVERRUN ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxOverrunIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxOverrunIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  #ifdef DEBUG_OVI
    db_printf("\nIn <EthRxOverrunIsr>, port id %d, overrun condition.", portId);
  #endif // DEBUG_OVI

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);  
  
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxOverrunIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }

/* Increase DMA RX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_CLEARED, ETH_BIT_CLEARED);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxOverrunIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxThresholdIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX THRESHOLD ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxThresholdIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxThresholdIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);  
  
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxThresholdIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }

/* Increase DMA RX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_CLEARED, ETH_BIT_CLEARED);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxThresholdIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxWaitIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX WAIT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : A WAIT condition is expected to happen if a frame was 
 *              : bigger than the space provided in the queue. 
 *              : if there are no more space to receive frames, abort the  
 *              : frame, clean-up and restart the queue. else, advance the 
 *              : DMA pointer and restart the queue.
 *              : Also, this handler clears the EOQ interrupt associated
 *              : with.
 =========================================================================*/
SpacEthRtnCode_t EthRxWaitIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxWaitIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   
    
  {
    EthRxStatus_t rxStatus;
    EthRxCtrl_t rxCtrl;
    EthQPtrHi_t qPtrHi;
    UINT32 qId;
    EthBuffDescrPtr_t descrPoolPtr = NULL;
    UINT16 numDescr = 0xFFFF;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0;
    INT32 sofBuffDescrIdx = 0;
    INT32 walkingBuffDescrIdx = 0;
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;
    SpacEthRegOps_t regOp = REG_OR;
    *((UINT32 *)&qPtrHi) = 0x00000000;  
    UINT32 loopExitCounter = 0;
    BOOL sysTermination = FALSE;
    UINT32 extIntrStatus;

/* Get the current queue. */      
    rtnCode = EthRegAccess(portId, ETH_RX_STATUS_REG, (UINT32 *)&rxStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 

    qId = rxStatus.currentQ;
    
/* Get the control info. */          
    rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&rxCtrl, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
        
/* Because a WAIT is also a EOQ, clear the queue and EOQ interrupt bits. */      
    *((UINT32 *)info) &= ~(ETH_RX_Q0_INTR_MASK << qId);
    extIntrStatus = (ETH_RX_EOQ_Q_X_INTR_MASK << qId);
/* Clear the EOQ extended status bit. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &extIntrStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 

/* only service enabled queueus. */
  if((rxCtrl.qEnables & (1 << qId)) != 0){
    descrPoolPtr = EthDriver.port[portId].mac.rx.rxQ[qId].memPool.descrPoolPtr;
    numDescr = EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;
        
/*  First, try to make some space, to reduce the chance of getting another WAIT 
    immediatelly. Signal the queue's counting semaphore, if using a RTOS. Call the 
    queue RX handler if running RedBoot. */ 
//    SpacSemSignal(&rxQCountSem[qId]);
        
/* Find the SOF and last descriptor (processed by DMA). */  
    lastBuffDescrIdxIncr = sofBuffDescrIdx = 
      EthDriver.port[portId].mac.rx.rxQ[qId].memPool.takeOutIdx;  
    lastBuffDescrIdx = sofBuffDescrIdx - 1;
    if(lastBuffDescrIdx < 0){
      lastBuffDescrIdx = numDescr - 1;
    }          
/* Cache synchronization. */    
    EthSynchCache(&(descrPoolPtr[sofBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);   
    EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);   
                
    if(EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.descrTermMode == 
      ETH_DESCR_TERM_OWN){    
      if(((*((EthRxFrmStatusCfgPtr_t)&(descrPoolPtr[sofBuffDescrIdx].
        cfgStatus))).owner == ETH_DESCR_SYS)
/*
 || 
        (descrPoolPtr[sofBuffDescrIdx].vlanBCnt == 0x00000000)){
*/
		) {
        sysTermination = TRUE;
      }else /* if(descrPoolPtr[sofBuffDescrIdx].vlanBCnt != 0x00000000) */{
        lastBuffDescrIdx = sofBuffDescrIdx;
        lastBuffDescrIdxIncr = (lastBuffDescrIdx + 1) % numDescr;
  /* Cache synchronization. */    
        EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX); 
        loopExitCounter = 1;
        while((descrPoolPtr[lastBuffDescrIdxIncr].vlanBCnt != 0x00000000) && 
          (loopExitCounter < numDescr)){
          lastBuffDescrIdx = lastBuffDescrIdxIncr;
          lastBuffDescrIdxIncr = (lastBuffDescrIdxIncr + 1) % numDescr;
  /* Cache synchronization. */    
          EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX); 
          loopExitCounter++;                         
        }
        if(loopExitCounter == numDescr){
          lastBuffDescrIdx = sofBuffDescrIdx - 1;
          if(lastBuffDescrIdx < 0){
            lastBuffDescrIdx = numDescr - 1;
          }
          lastBuffDescrIdxIncr = sofBuffDescrIdx;
        }
      }
    }
      
/* If the queue was SYSTEM terminated or to small, for now, abort. Else, there may 
   be a chance that more available RX descriptors are chained. */ 
    if((sysTermination == TRUE) || (loopExitCounter != 0)){          
//      if(loopExitCounter1 < numDescr){
      if(loopExitCounter != 0){
/* Return the aborted buffers to DMA. */ 
        walkingBuffDescrIdx = sofBuffDescrIdx;
        do{
          (*((EthRxFrmStatusCfgPtr_t)&(descrPoolPtr[walkingBuffDescrIdx].
            cfgStatus))).owner = ETH_DESCR_DMA;
/* Clear VLAN/Byte count configuration. Needed in the WAIT ISR. */                
          descrPoolPtr[walkingBuffDescrIdx].vlanBCnt = 0x00000000;  
/* Cache synchronization. */    
          EthSynchCache(&(descrPoolPtr[walkingBuffDescrIdx]), ETH_DESCR_SIZE, ETH_TX);                                             
          walkingBuffDescrIdx = (walkingBuffDescrIdx + 1) % numDescr;
        }while(walkingBuffDescrIdx != lastBuffDescrIdxIncr);          
        lastBuffDescrIdx = sofBuffDescrIdx - 1;
        if(lastBuffDescrIdx < 0){
          lastBuffDescrIdx = numDescr - 1;
        }
        lastBuffDescrIdxIncr = sofBuffDescrIdx;        
      } 
            
      rxCtrl.enableAbortIntr = ETH_BIT_SET;   
      rxCtrl.abort = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&rxCtrl, 
        REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_RX_CTRL_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
    }
   
/* Cache synchronization. */    
   EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);                          
   
/* Manually update the descriptor pointer, if NULL terminated, or aborted buffers
   were returned to DMA. */   
    if(((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
     ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)) ||
//     ((loopExitCounter1 < numDescr)))
     ((loopExitCounter != 0))){

      rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
        (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
      if(rtnCode != 0){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxWaitIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
      rtnCode = EthRegAccess(portId, 
        ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), 
        (UINT32 *)&descrAddrLo, REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, 
            ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
      regOp = REG_WRITE;
    }

    /* Re-validate the queue. If NULL terminated, qPtrHi contains the HI bits. */    
    qPtrHi.valid = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), (UINT32 *)&qPtrHi, regOp);

    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", regOp, 
          ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
          
  }else{
    rxCtrl.enableAbortIntr = ETH_BIT_SET;   
    rxCtrl.abort = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&rxCtrl, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
  }

  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxQueueIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, used to retrieve the qeue id. 
 * 
 *  Description : GigE's RX QUEUE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : An EOQ condition is expected to happen if a frame was 
 *              : received, and DMA has encountered a LAST/NULL EOQ 
 *              : condition on the last processed descriptor. Update the 
 *              : queue pointer, clean-up and restart the queue. 
 =========================================================================*/
SpacEthRtnCode_t EthRxQueueIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  UINT32 extIntrStatus, rxQCfg, qId;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxQueueIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((info > ETH_RX_Q_INTR_STATUS_HI) || (info < ETH_RX_Q_INTR_STATUS_LO)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxQueueIsr>, invalid interrupt status bit = %d.", info);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }else{
    qId = info - ETH_RX_Q_INTR_STATUS_LO;
  } 
  
  {
    EthQPtrHi_t qPtrHi;
    EthBuffDescrPtr_t descrPoolPtr = 
      EthDriver.port[portId].mac.rx.rxQ[qId].memPool.descrPoolPtr;
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0;
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;    
    SpacEthRegOps_t regOp = REG_OR;    
    *((UINT32 *)&qPtrHi) = 0x00000000;
    UINT32 loopExitCounter = 0;      

/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &extIntrStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Keep only the extended status bits specific to a particular queue. */  
    extIntrStatus &= (ETH_RX_Q_X_INTR_MASK << qId);
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &extIntrStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
/* Get the config enable bits. */      
    rtnCode = EthRegAccess(portId, 
      ETH_RX_Q0_CFG_REG + (qId * ETH_RX_Q_REG_OFFSET), &rxQCfg, REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, 
          ETH_RX_Q0_CFG_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
      
/* Descriptor interrupt. */  
    if((extIntrStatus & (ETH_RX_DESCR_Q_X_INTR_MASK << qId)) && 
      (rxQCfg & ETH_RX_DESCR_Q_INTR_EN_MASK)){

#ifdef ETH_DIAG_EXTENDED
  ++(qInterruptsTable[portId][ETH_RX][ETH_Q_DESCR_INTR]);
#endif  //  ETH_DIAG_EXTENDED

    }
/* EOF interrupt. */  
    if((extIntrStatus & (ETH_RX_EOF_Q_X_INTR_MASK << qId)) &&
      (rxQCfg & ETH_RX_EOF_Q_INTR_EN_MASK)){                
        
/* Signal the queue's counting semaphore, if using a RTOS. Call the RX handler 
   if running RedBoot. */ 
//      SpacSemSignal(&rxQCountSem[qId]);

#ifdef ETH_DIAG_EXTENDED
  ++(qInterruptsTable[portId][ETH_RX][ETH_Q_EOF_INTR]);
#endif  //  ETH_DIAG_EXTENDED

    }
/* EOQ interrupt. */  
    if((extIntrStatus & (ETH_RX_EOQ_Q_X_INTR_MASK << qId)) &&
      (rxQCfg & ETH_RX_EOQ_Q_INTR_EN_MASK)){
        
                

#ifdef ETH_DIAG_EXTENDED
  ++(qInterruptsTable[portId][ETH_RX][ETH_Q_EOQ_INTR]);
#endif  //  ETH_DIAG_EXTENDED
  
    }
            
    if((extIntrStatus & (ETH_RX_EOQ_Q_X_INTR_MASK << qId)) &&
      (rxQCfg & ETH_RX_EOQ_Q_INTR_EN_MASK)){    
        
  /* Find the last descriptor (processed by DMA). */  
      lastBuffDescrIdx = 
        EthDriver.port[portId].mac.rx.rxQ[qId].memPool.takeOutIdx;
  /* Cache synchronization. */    
      EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);            
        
      if(descrPoolPtr[lastBuffDescrIdx].vlanBCnt == 0x00000000){
        lastBuffDescrIdxIncr = lastBuffDescrIdx;
        lastBuffDescrIdx--;
        if(lastBuffDescrIdx < 0){
          lastBuffDescrIdx = numDescr - 1;
        }
      }else{
        loopExitCounter = 1;            
        lastBuffDescrIdxIncr = (lastBuffDescrIdx + 1) % numDescr;
  /* Cache synchronization. */    
        EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX);            
        while((descrPoolPtr[lastBuffDescrIdxIncr].vlanBCnt != 0x00000000) && 
         (loopExitCounter < numDescr)){
          lastBuffDescrIdx = lastBuffDescrIdxIncr;
          lastBuffDescrIdxIncr = (lastBuffDescrIdxIncr + 1) % numDescr;
  /* Cache synchronization. */    
          EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX);         
          loopExitCounter++;
         }
         if(loopExitCounter == numDescr){
           lastBuffDescrIdxIncr = EthDriver.port[portId].mac.rx.rxQ[qId].memPool.takeOutIdx;        
           lastBuffDescrIdx--;
           if(lastBuffDescrIdx < 0){
             lastBuffDescrIdx = numDescr - 1;
          }
        }
      }        
        
/* Manually update the descriptor pointer, if NULL terminated. */         
      if((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
       ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)){
        rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
          (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxQueueIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), 
          (UINT32 *)&descrAddrLo, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        regOp = REG_WRITE;
      }
      /* Re-validate the queue. If NULL terminated, qPtrHi contains the HI bits. */    
      qPtrHi.valid = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, 
        ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), 
        (UINT32 *)&qPtrHi, regOp);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", regOp, 
            ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
    }
  }
      
  return rtnCode;   
    
}

/*=========================================================================
 *  Function    : EthStatsCarryIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's STATS CARRY ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthStatsCarryIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthStatsCarryIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  {
    UINT32 mstatCar1;
    UINT32 mstatCar2;
/* Get the Carry1 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR1_REG, &mstatCar1, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_MSTAT_CAR1_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          reads %08X.", REG_READ, ETH_MSTAT_CAR1_REG, *((UINT32 *)&mstatCar1));
      #endif // DEBUG_OVI
    }
/* Clear the Carry1 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR1_REG, &mstatCar1, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MSTAT_CAR1_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Get the Carry2 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR2_REG, &mstatCar2, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_MSTAT_CAR2_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          reads %08X.", REG_READ, ETH_MSTAT_CAR2_REG, *((UINT32 *)&mstatCar2));
      #endif // DEBUG_OVI
    }
/* Clear the Carry2 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR2_REG, &mstatCar2, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MSTAT_CAR2_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    
    
  }
  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxIdleIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX IDLE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxIdleIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxIdleIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
 
  {
    EthTxStatus_t txStatus;
/* Get the idle queue(s). */      
    rtnCode = EthRegAccess(portId, ETH_TX_STATUS_REG, (UINT32 *)&txStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxIdleIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxIdleIsr>, TX queue idle field = %02X.", 
          txStatus.qIdle);
      #endif // DEBUG_OVI
    }
  }

/* Signal a semaphore, if needed.  */  


  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxAbortIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX ABORT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxAbortIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxAbortIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  #ifdef DEBUG_OVI
    db_printf("\nIn <EthTxAbortIsr>, port id %d, TX abort completed.", portId);
  #endif // DEBUG_OVI
    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxErrorIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX ERROR ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxErrorIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxErrorIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
    
  {
  UINT32 xStatus, errStatus; 
  int idx; 
/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &xStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Get the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_ERR_STATUS_REG, &errStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_ERR_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
        
    for(idx = (ETH_NUM_TX_Q - 1); idx >= 0; idx--){
      if(xStatus & (ETH_TX_Q0_ERR_MASK << idx)){
/* Missing/misplaced SOF. */        
        if(errStatus & (ETH_TX_Q0_DESC_ERR_MASK << (idx * ETH_BYTE_WIDTH))){          
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, descriptor error in queue %d.", idx);
          #endif // DEBUG_OVI
          

        }
        if(errStatus & (ETH_TX_Q0_TIMEOUT_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
//          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, timeout error in queue %d.", idx);
//          #endif // DEBUG_OVI
        }
        if(errStatus & (ETH_TX_Q0_READ_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
//          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, read error in queue %d, target \
              response is %02X", idx, 
              (errStatus & (ETH_TX_Q0_TRESP_ERR_MASK << (idx * ETH_BYTE_WIDTH))));
//          #endif // DEBUG_OVI
        }
        if(errStatus & (ETH_TX_Q0_TEA_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
//          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, TEA error in queue %d.", idx);
//          #endif // DEBUG_OVI
        }
      }
    }
    
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &xStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Clear the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_ERR_STATUS_REG, &errStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_ERR_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
  }
    
/* According to the specs, chip reset may be needed. TBD.  
  EthReset(); 
*/    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxUnderrunIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX UNDERRUN ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxUnderrunIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxUnderrunIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  #ifdef DEBUG_OVI
    db_printf("\nIn <EthTxUnderrunIsr>, port id %d, underrun condition.", portId);
  #endif // DEBUG_OVI
    

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED); 
   
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxUnderrunIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
  }

/* Increase DMA TX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_SET, ETH_BIT_SET);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxUnderrunIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxThresholdIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX THRESHOLD ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxThresholdIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxThresholdIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);  
  
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxThresholdIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
  }

/* Increase DMA TX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_SET, ETH_BIT_SET);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxThresholdIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif // DEBUG_OVI
  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxWaitIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX WAIT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : As the driver checks for available space in the TX queue,
 *              : a WAIT condition is expected to happen if a frame was 
 *              : interrupted, and DMA has encountered an EOQ condition.
 *              : if the EOF was forgotten, abort the frame, clean-up
 *              : and restart the queue. if the frame continues, advance the 
 *              : DMA pointer and restart the queue.
 =========================================================================*/
SpacEthRtnCode_t EthTxWaitIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxWaitIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
    
  {
    EthRxStatus_t txStatus;
    EthRxCtrl_t txCtrl;
    EthQPtrHi_t qPtrHi;
    UINT32 qId;
    EthBuffDescrPtr_t descrPoolPtr = NULL;
    UINT16 numDescr = 0xFFFF;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0;
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;    
    SpacEthRegOps_t regOp = REG_OR;        
    *((UINT32 *)&txCtrl) = 0x00000000;    
    *((UINT32 *)&qPtrHi) = 0x00000000;  
//    UINT16 loopExitCounter;  

/* Get the current queue. */      
    rtnCode = EthRegAccess(portId, ETH_TX_STATUS_REG, (UINT32 *)&txStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
    qId = txStatus.currentQ;
    
/* Get the control info. */              
    rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&txCtrl, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    
  if((txCtrl.qEnables & (1 << qId)) != 0){
    descrPoolPtr = EthDriver.port[portId].mac.tx.txQ[qId].memPool.descrPoolPtr;
    numDescr = EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
              
/* Signal the queue's counting semaphore, if using a RTOS. Call the queue 
   RX handler if running RedBoot. */ 
//    SpacSemSignal(&txQCountSem[qId]);
              
/* Find the last descriptor. */  
// EthFreeTxFrame should give the lastBuffDescrIdx + 1.
    lastBuffDescrIdxIncr = 
      EthDriver.port[portId].mac.tx.txQ[qId].memPool.takeOutIdx;
    lastBuffDescrIdx = lastBuffDescrIdxIncr - 1;
    if(lastBuffDescrIdx < 0){
      lastBuffDescrIdx = numDescr - 1;
    }
      
/* Cache synchronization. */    
    EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX);      

/* If the queue was not NULL or LAST terminated, for now, abort. Else, there
   may be a chance that more available TX descriptors are chained. */ 
    if(
      ((*((EthTxFrmStatusCfgPtr_t)&(descrPoolPtr[lastBuffDescrIdxIncr].cfgStatus))).owner == 
        ETH_DESCR_SYS)){
      txCtrl.enableAbortIntr = ETH_BIT_SET;
      txCtrl.abort = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&txCtrl, 
        REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_TX_CTRL_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
    }else{
/* Cache synchronization. */    
      EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);      
/* Manually update the descriptor pointer, if NULL terminated. */   
      if((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
       ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)){
        rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
          (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxWaitIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&descrAddrLo, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        regOp = REG_WRITE;
      }
/* Re-validate the queue. If NULL terminated, qPtrHi contains the HI bits. */    
      qPtrHi.valid = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, 
        ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), 
        (UINT32 *)&qPtrHi, regOp);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", regOp, 
            ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
    }
               
  
  }else{
    txCtrl.enableAbortIntr = ETH_BIT_SET;   
    txCtrl.abort = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&txCtrl, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
  }

  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxQueueIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, used to retrieve the qeue id. 
 * 
 *  Description : GigE's TX QUEUE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : As the driver checks for available space in the TX queue,
 *              : a EOQ condition is expected to happen if  DMA has encountered
 *              : an ownership EOQ condition, after an EOF. Restart the queue.
 *              : Update the queue pointer if NULL termination.
 =========================================================================*/
SpacEthRtnCode_t EthTxQueueIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  UINT32 extIntrStatus, txQCfg, qId;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxQueueIsr>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((info > ETH_TX_Q_INTR_STATUS_HI) || (info < ETH_TX_Q_INTR_STATUS_LO)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxQueueIsr>, invalid interrupt status bit = %d.", info);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }else{
    qId = info - ETH_TX_Q_INTR_STATUS_LO;
  }  
  
  {
    EthQPtrHi_t qPtrHi;
    EthBuffDescrPtr_t descrPoolPtr = 
      EthDriver.port[portId].mac.tx.txQ[qId].memPool.descrPoolPtr;
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0; 
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;    
    *((UINT32 *)&qPtrHi) = 0x00000000;  

/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &extIntrStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/* Keep only the extended status bits specific to a particular queue. */  
    extIntrStatus &= (ETH_TX_Q_X_INTR_MASK << qId);
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &extIntrStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_XSTATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
    
/* Get the config enable bits. */      
    rtnCode = EthRegAccess(portId, 
      ETH_TX_Q0_CFG_REG + (qId * ETH_TX_Q_REG_OFFSET), &txQCfg, REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, 
          ETH_TX_Q0_CFG_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
    
/* This implementations uses only EOF/EOQ interrupts. */  
    if(((extIntrStatus & (ETH_TX_EOF_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOF_Q_INTR_EN_MASK)) || 
      ((extIntrStatus & (ETH_TX_EOQ_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOQ_Q_INTR_EN_MASK))){
/* Signal the queue's counting semaphore, if using a RTOS. Call the TX 
   free frames handler if running RedBoot. */ 
//      SpacSemSignal(&txQCountSem[qId]);

/* Find the last descriptor. */  
// EthFreeTxFrame should give the lastBuffDescrIdx + 1.
      lastBuffDescrIdxIncr = 
        EthDriver.port[portId].mac.tx.txQ[qId].memPool.takeOutIdx;
      lastBuffDescrIdx = lastBuffDescrIdxIncr - 1;
      if(lastBuffDescrIdx < 0){
        lastBuffDescrIdx = numDescr - 1;
      }
    } 

/* Descriptor interrupt. */  
    if((extIntrStatus & (ETH_TX_DESCR_Q_X_INTR_MASK << qId)) && 
      (txQCfg & ETH_TX_DESCR_Q_INTR_EN_MASK)){

#ifdef ETH_DIAG_EXTENDED
  ++(qInterruptsTable[portId][ETH_TX][ETH_Q_DESCR_INTR]);
#endif  //  ETH_DIAG_EXTENDED

    }
/* EOF interrupt. */  
    if((extIntrStatus & (ETH_TX_EOF_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOF_Q_INTR_EN_MASK)){

    }
/* EOQ interrupt. */  
    if((extIntrStatus & (ETH_TX_EOQ_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOQ_Q_INTR_EN_MASK)){
#ifdef ETH_DIAG_EXTENDED
  ++(qInterruptsTable[portId][ETH_TX][ETH_Q_EOQ_INTR]);
#endif  //  ETH_DIAG_EXTENDED

    }
    
    if((extIntrStatus & (ETH_TX_EOQ_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOQ_Q_INTR_EN_MASK)){   
/* Cache synchronization. */    
      EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);      

/* Manually update the descriptor pointer, if NULL terminated. */   
      if((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
       ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)){
        rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
          (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxQueueIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&descrAddrLo, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&qPtrHi, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        }
      }
    }
  }


  return rtnCode;
    
}

