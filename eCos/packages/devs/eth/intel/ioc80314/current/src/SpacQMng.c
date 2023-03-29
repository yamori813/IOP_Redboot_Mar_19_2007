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
* FILENAME: 1.	SpacQMng.c
*
* DESCRIPTION: This file implements queue management routines.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     modified  SpacQMngCreate/SpacQMngDelete 
*                            function signatures.
*
* NOTE:
* It is advised to not attach to queues/managers, semaphores created
* on local stacks.
* If the system does not support synchronization primitives, the correspoinding
* routines are implemented as NOP. 
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

extern int net_debug;

/*
   The routines working on the queues are designed to present with three
   levels of protection against concurrent access.

   If one thread does only one type of access(put or take), and another
   does the complementary operation(take or put), no protection is needed.
   Similarly, if the queue is accessed only from the same thresd, no protection
   is needed(the FIFO natuture of the queue insures this).
   The SPAC_THREAD_SAFE or SPAC_ISR_SAFE flags should be carefuly used when
   need to access very often the queue(i.e. in a loop), as they increase system
   latency w.r.t. thread  or interrupt scheduling.

   SPAC_NOT_SAFE - no protection. May be used in the context of an ISR,
     but only on platforms not allowing inrerrupt preemption.
   SPAC_MTX_SAFE - mutual exclusion protection. This is the recommended mechanism
     to be used whenever not in the context of an ISR. However, involves more
     system overhead than SPAC_THREAD_SAFE or SPAC_ISR_SAFE flags.
   SPAC_THREAD_SAFE - thread preemption protection. Involves less system overhead
     than SPAC_MTX_SAFE flag.
   SPAC_ISR_SAFE - highest level of protection. Involves less system overhead
     than SPAC_THREAD_SAFE or SPAC_MTX_SAFE flags.

   The routines working on the queue manager are actually implemented
   as wrappers around the ones operating on queues. Therefore, they inherit
   the above-mentioned characteristics. A queue manager is merely a memory
   poll, managed through a FIFO.

  A user of this module might choose to use SpacQType queues for basic
  FIFO operations, or use SpacQMngType queue managers if need for custom
  memory management operations.

*/

/* Primitive way of allocating memory. Just to get the work done. */
static int prevAllocSize = 0;

/*******************************************************************************
* SpacQCreate
* DESCRIPTION:
* This is the routine that is to be called to initialize a queue.
*
* RETURNS:
*    ETH_OK - operation succeded.
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_MEM_ALLOC_ERR - memory allocation error.
*    ETH_SEMCREATE_ERR - semaphore creation error.
*    ETH_ALREADY_INIT_ERR - entity has already been initialized.
*
* NOTE:
* Call this routine before any queue operation.
*
******************************************************************************/
SpacEthRtnCode_t SpacQCreate(SpacQPtr_t qPtr, unsigned qSize, void * semPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qPtr == NULL || qSize < sizeof(void *) || (semPtr == NULL)){
    return ETH_PARAM_ERR;
  }

  if(qPtr->qInitFlag == TRUE){
    return ETH_ALREADY_INIT_ERR;
  }

  MEMSET((void *)qPtr, 0x00, sizeof(SpacQ_t));
  
  if((qPtr->qDataPtr = MEMALIGN(ETH_MEM_ALIGN, qSize * sizeof(void *))) == NULL){
    return ETH_MEM_ALLOC_ERR;
  }
  else {
	if (net_debug == true) {
		db_printf ("Malloc'ing 0x%08x bytes for qDataPtr\n", (qSize * sizeof (void*)));
		db_printf ("Pointer returned as: 0x%08x\n", qPtr->qDataPtr);
	}
  }

  MEMSET((void *)qPtr->qDataPtr, 0x00, qSize * sizeof(void *));

	qPtr->mtxKey = (int)semPtr;

  if((retCode = SpacSemCreate(SPAC_MTX_SEM, 1, &qPtr->mtxKey)) != ETH_OK){
    FREE_MEM(qPtr->qDataPtr, qSize * sizeof(void *));
    qPtr->qDataPtr = NULL;
    qPtr->mtxKey = 0;
    return ETH_SEMCREATE_ERR;
  }

  qPtr->qSize = qSize;
  qPtr->qInitFlag = TRUE;

  return retCode;

}

/*******************************************************************************
* SpacQDelete
* DESCRIPTION:
* This is the routine that is to be called to delete a queue.
*
* RETURNS:
*    ETH_OK - operation succeded.
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_SEMDELETE_ERR - semaphore deletion error.
*    ETH_NOT_INIT_ERR - attempt to operate on a non initialized manager.
*
* NOTE:
* Call this routine to delete a queue. Free any dynamic allocated memory
* referenced in the queue, to prevent memory leaks.
*
******************************************************************************/
SpacEthRtnCode_t SpacQDelete(SpacQPtr_t qPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qPtr->qInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }

  FREE_MEM(qPtr->qDataPtr, qPtr->qSize * sizeof(void *));

  if((retCode = SpacSemDelete(&qPtr->mtxKey)) != ETH_OK){
    return ETH_SEMDELETE_ERR;
  }

  MEMSET((void *)qPtr, 0x00, sizeof(SpacQ_t));

  return retCode;

}

/*******************************************************************************
* SpacPutQ
* DESCRIPTION:
* This is the routine that is to be called to enqueue a cell.
*
* RETURNS:
*    ETH_OK    - operation succeded
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_NOT_INIT_ERR - resource (semaphore) not ready.
*    ETH_SYNC_OBJ_ERR - semaphore operation error.
*    ETH_Q_FULL_ERR - queue overflow condition.
*
* NOTES:
*
* Queue access is serialized.
*
******************************************************************************/
SpacEthRtnCode_t SpacPutQ(SpacQPtr_t qPtr, void * dataPtr, SpacQSafe_t qSafe)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qPtr == NULL || dataPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qPtr->qInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }

  if(qSafe == SPAC_NOT_SAFE){
    ;
  }else if(qSafe == SPAC_ISR_SAFE){
    qPtr->preemptKey = SpacInterruptDisable();
  }else if(qSafe == SPAC_THREAD_SAFE){
    qPtr->preemptKey = SpacSchedulerDisable();
  }else if(qSafe == SPAC_MTX_SAFE){
    if((retCode = SpacSemWait(&qPtr->mtxKey, SPAC_WAITFOREVER)) != ETH_OK){
      return ETH_SYNC_OBJ_ERR;
    }
  }else{
    return ETH_PARAM_ERR;
  }

  if((qPtr->qHead != qPtr->qTail) || (qPtr->qDataPtr[qPtr->qHead] == NULL)){
    qPtr->qDataPtr[qPtr->qHead++] = dataPtr;
    qPtr->qHead %= qPtr->qSize;
    qPtr->numEnqueued++;
  }
  else{
    retCode = ETH_Q_FULL_ERR;
  }

  if(qSafe == SPAC_NOT_SAFE){
    ;
  }else if(qSafe == SPAC_ISR_SAFE){
    SpacInterruptEnable(qPtr->preemptKey);
  }else if(qSafe == SPAC_THREAD_SAFE){
    SpacSchedulerEnable(qPtr->preemptKey);
  }else if(qSafe == SPAC_MTX_SAFE){
    if((retCode =SpacSemSignal(&qPtr->mtxKey)) != ETH_OK){
      return ETH_SYNC_OBJ_ERR;
    }
  }

  return retCode;

}

/*******************************************************************************
* SpacTakeQ
* DESCRIPTION:
* This is the routine that is to be called to dequeue a cell.
*
* RETURNS:
*    ETH_OK    - operation succeded
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_Q_EMPTY_ERR - queue empty condition.
*    ETH_NOT_INIT_ERR - queue not initialized.
*    ETH_SYNC_OBJ_ERR - semaphore operation error.
*
* HISTORY:
*    01.03.28 O.N.  creation
*
* NOTES:
*
* Queue access is serialized.
*
******************************************************************************/
SpacEthRtnCode_t SpacTakeQ(SpacQPtr_t qPtr, void ** dataPtr, SpacQSafe_t qSafe)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qPtr == NULL || dataPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qPtr->qInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }

  if(qSafe == SPAC_NOT_SAFE){
    ;
  }else if(qSafe == SPAC_ISR_SAFE){
    qPtr->preemptKey = SpacInterruptDisable();
  }else if(qSafe == SPAC_THREAD_SAFE){
    qPtr->preemptKey = SpacSchedulerDisable();
  }else if(qSafe == SPAC_MTX_SAFE){
    if((retCode = SpacSemWait(&qPtr->mtxKey, SPAC_WAITFOREVER)) != ETH_OK){
      return ETH_SYNC_OBJ_ERR;
    }
  }else{
    return ETH_PARAM_ERR;
  }

  if((qPtr->qHead != qPtr->qTail) || (qPtr->qDataPtr[qPtr->qTail] != NULL)){
    *dataPtr = qPtr->qDataPtr[qPtr->qTail];
    qPtr->qDataPtr[qPtr->qTail++] = NULL;
    qPtr->qTail %= qPtr->qSize ;
    qPtr->numEnqueued--;
  }
  else{
    retCode = ETH_Q_EMPTY_ERR;
  }

  if(qSafe == SPAC_NOT_SAFE){
    ;
  }else if(qSafe == SPAC_ISR_SAFE){
    SpacInterruptEnable(qPtr->preemptKey);
  }else if(qSafe == SPAC_THREAD_SAFE){
    SpacSchedulerEnable(qPtr->preemptKey);
  }else if(qSafe == SPAC_MTX_SAFE){
    if((retCode = SpacSemSignal(&qPtr->mtxKey)) != ETH_OK){
      return ETH_SYNC_OBJ_ERR;
    }
  }

  return retCode;

}

/*******************************************************************************
* SpacQMngCreate
* DESCRIPTION:
* This is the routine that is to be called to initialize a queue manager.
*
* RETURNS:
*    ETH_OK - operation succeded.
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_MEM_ALLOC_ERR - memory allocation error.
*    ETH_SEMCREATE_ERR - semaphore creation error.
*    ETH_ALREADY_INIT_ERR - entity has already been initialized.
*    ETH_Q_FULL_ERR - queue overflow condition.
*
* HISTORY:
*    01.03.28 O.N.  creation
*
* NOTE:
* Call this routine before any queue manager operation. This implementation 
* allows only for a one-time call to allocate memory elsewhere than SDRAM.
*
******************************************************************************/
SpacEthRtnCode_t SpacQMngCreate(SpacQMngPtrType qMngPtr, unsigned qMngQSize,
  unsigned qMngCellSize, void * mtxPtr, UINT32 memSpaceId)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if((qMngPtr == NULL) || (qMngQSize == 0) || (qMngCellSize == 0) ||
    (mtxPtr == NULL)){
    return ETH_PARAM_ERR;
  }

  if(qMngPtr->qMngInitFlag == TRUE){
    return ETH_ALREADY_INIT_ERR;
  }

/* pointer to memory poll. */
	qMngPtr->qMngPollPtr = NULL;
/* initialization flag. */
	qMngPtr->qMngInitFlag = FALSE;
/* queue manager memory poll cell size. */
	qMngPtr->qMngCellSize = 0;

  if((retCode = SpacQCreate(&qMngPtr->memQ, qMngQSize, mtxPtr)) != ETH_OK){
    return retCode;
  }

  if((memSpaceId == ETH_OCN_PCI1_ID) || (memSpaceId == ETH_OCN_PCI2_ID)){
    qMngPtr->qMngPollPtr = (void *)(ETH_DESCR_MEM_SIZE + prevAllocSize + 
      (memSpaceId == ETH_OCN_PCI1_ID ? XS_PCI1_BASE_ADDR : XS_PCI2_BASE_ADDR));
  }else if(memSpaceId == ETH_OCN_SDRAM_ID){
#ifdef HIGH_MEM_ALLOC
    qMngPtr->qMngPollPtr = (void *)(ETH_DESCR_MEM_SIZE + prevAllocSize + SDRAM_HIGH_MEM);
#else  
    if((qMngPtr->qMngPollPtr = MEMALIGN(ETH_MEM_ALIGN, qMngQSize * qMngCellSize)) == NULL){
      SpacQDelete(&qMngPtr->memQ);
      return ETH_MEM_ALLOC_ERR;
    }
	else	  {
		if (net_debug == true) {
			db_printf ("Malloc'ing 0x%08x bytes for qMngPollPtr\n", (qMngQSize * qMngCellSize));
			db_printf ("Pointer returned as: 0x%08x\n", qMngPtr->qMngPollPtr);
		}
  }
#endif    
  }else if(memSpaceId == ETH_OCN_XSCALE_ID){
    qMngPtr->qMngPollPtr = (void *)(ETH_DESCR_MEM_SIZE + prevAllocSize + HAL_IOC80314_SRAM_BASE);
  }

  {
    char * cellPtr = qMngPtr->qMngPollPtr;
    unsigned int idx;
    for(idx = 0; idx < qMngQSize; idx++){
      if((retCode = SpacPutQ(&qMngPtr->memQ,
          (void *)(cellPtr + idx * qMngCellSize), SPAC_NOT_SAFE)) != ETH_OK){
        SpacQDelete(&qMngPtr->memQ);
#ifndef HIGH_MEM_ALLOC
        if(memSpaceId == ETH_OCN_SDRAM_ID){
          FREE_MEM(qMngPtr->qMngPollPtr, qMngQSize * qMngCellSize);
        }
#endif        
        qMngPtr->qMngPollPtr = NULL;
        return retCode;
      }
    }
  }
  
  prevAllocSize += qMngQSize * qMngCellSize;
  
  qMngPtr->qMngCellSize = qMngCellSize;
  qMngPtr->qMngInitFlag = TRUE;

  return retCode;

}

/*******************************************************************************
* SpacQMngDelete
* DESCRIPTION:
* This is the routine that is to be called to delete a queue manager.
*
* RETURNS:
*    ETH_OK - operation succeded.
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_SEMDELETE_ERR - semaphore deletion error.
*    ETH_NOT_INIT_ERR - attempt to operate on a non initialized manager.
*
* HISTORY:
*    01.03.28 O.N.  creation
*
* NOTE:
* Call this routine to delete a queue manager. Free any dynamic allocated memory
* referenced in the queue, to prevent memory leaks.
*
******************************************************************************/
SpacEthRtnCode_t SpacQMngDelete(SpacQMngPtrType qMngPtr, UINT32 memSpaceId)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qMngPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qMngPtr->qMngInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }
#ifndef HIGH_MEM_ALLOC
  if(memSpaceId == ETH_OCN_SDRAM_ID){
    FREE_MEM(qMngPtr->qMngPollPtr, qMngQSize * qMngCellSize);
  }
#endif        
  if((retCode = SpacQDelete(&qMngPtr->memQ)) != ETH_OK){
    return retCode;
  }

	qMngPtr->qMngPollPtr = NULL;
	qMngPtr->qMngCellSize = 0;
	qMngPtr->qMngInitFlag = FALSE;

  return retCode;

}

/*******************************************************************************
* SpacQMngReturn
* DESCRIPTION:
* This is the routine that is to be called to return a cell to the queue
* manager's memory poll.
*
* RETURNS:
*    ETH_OK    - operation succeded
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_Q_FULL_ERR - queue overflow condition.
*    ETH_NOT_INIT_ERR - resource (semaphore) not ready.
*    ETH_SYNC_OBJ_ERR - semaphore operation error.
*
* HISTORY:
*    01.03.28 O.N.  creation
*
******************************************************************************/
SpacEthRtnCode_t SpacQMngReturn(SpacQMngPtrType qMngPtr, void * dataPtr,
  SpacQSafe_t qSafe)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qMngPtr == NULL || dataPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qMngPtr->qMngInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }

  MEMSET(dataPtr, 0x00, sizeof(qMngPtr->qMngCellSize));

  retCode =SpacPutQ(&qMngPtr->memQ, dataPtr, qSafe);

  return retCode;
  
}

/*******************************************************************************
* SpacQMngGet
* DESCRIPTION:
* This is the routine that is to be called to get a new cell from the queue
* manager's memory poll.
*
* RETURNS:
*    ETH_OK    - operation succeded
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_Q_EMPTY_ERR - queue empty condition.
*    ETH_NOT_INIT_ERR - queue not initialized.
*    ETH_SYNC_OBJ_ERR - semaphore operation error.
*
* HISTORY:
*    01.03.28 O.N.  creation
*
******************************************************************************/
SpacEthRtnCode_t SpacQMngGet(SpacQMngPtrType qMngPtr, void ** dataPtr,
  SpacQSafe_t qSafe)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qMngPtr == NULL || dataPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qMngPtr->qMngInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }

  retCode = SpacTakeQ(&qMngPtr->memQ, dataPtr, qSafe);
  
  return retCode;

}

/*******************************************************************************
* SpacQMngTry
* DESCRIPTION:
* This is the routine that is to be called to get the number of the
* queue manager's memory queue available cells.
*
* RETURNS:
*    ETH_OK    - operation succeded
*    ETH_PARAM_ERR - invalid parameter.
*    ETH_NOT_INIT_ERR - queue not initialized.
*
* HISTORY:
*    01.03.28 O.N.  creation
*
******************************************************************************/
SpacEthRtnCode_t SpacQMngTry(SpacQMngPtrType qMngPtr, int * numCellsPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  if(qMngPtr == NULL || numCellsPtr == NULL){
    return ETH_PARAM_ERR;
  }

  if(qMngPtr->qMngInitFlag == FALSE){
    return ETH_NOT_INIT_ERR;
  }

  *numCellsPtr = qMngPtr->memQ.numEnqueued;

  return retCode;

}

















































