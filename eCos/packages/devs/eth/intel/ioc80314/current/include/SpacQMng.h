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
* FILENAME: 1.	SpacQMng.h
*
* DESCRIPTION: This file defines queue management routines.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     modified  SpacQMngCreate/SpacQMngDelete 
*                            function signatures.
*----------------------------------------------------------------------------*/

#ifndef _SPAC_QMNG_H_
#define _SPAC_QMNG_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

#ifndef _SPAC_ETH_PUBLIC_H_
#include "SpacEthPublic.h"
#endif

/* default queue size. */
#define SPAC_Q_SIZE (1024)

/* queue access safety level. */
typedef enum {
 SPAC_NOT_SAFE           = 0,
 SPAC_MTX_SAFE           = 1,
 SPAC_THREAD_SAFE        = 2,
 SPAC_ISR_SAFE           = 3,
} SpacQSafe_t; 

/* as much as posible, keep data members declared as integers, to insure atomic
   read/write operations. */
typedef struct {
/* pointer to data to enqueue. */
	void ** qDataPtr;
/* initialization flag. */
	int qInitFlag;
/* queue size. */
	int qSize ;
/* number of enqueued cells. */
	int numEnqueued;
/* head pointer. */
	int qHead ;
/* tail pointer. */
	int qTail ;
/* preempt key. */
	int preemptKey ;
/* mutex key. */
	int mtxKey;

} SpacQ_t , * SpacQPtr_t;

/* as much as posible, keep data members declared as integers, to insure atomic
   read/write operations. */
typedef struct {
/* cell pointer queue. */
	SpacQ_t memQ;
/* pointer to memory poll. */
	void * qMngPollPtr;
/* initialization flag. */
	int qMngInitFlag;
/* queue manager memory poll cell size. */
	int qMngCellSize ;
} SpacQMngType , * SpacQMngPtrType;

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
SpacEthRtnCode_t SpacQCreate(SpacQPtr_t qPtr, unsigned qSize, void * semPtr);

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
SpacEthRtnCode_t SpacQDelete(SpacQPtr_t qPtr);

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
SpacEthRtnCode_t SpacPutQ(SpacQPtr_t qPtr, void * dataPtr, SpacQSafe_t qSafe);

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
SpacEthRtnCode_t SpacTakeQ(SpacQPtr_t qPtr, void ** dataPtr, SpacQSafe_t qSafe);

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
  unsigned qMngCellSize, void * mtxPtr, UINT32 memSpaceId);

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
SpacEthRtnCode_t SpacQMngDelete(SpacQMngPtrType qMngPtr, UINT32 memSpaceId);

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
  SpacQSafe_t qSafe);

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
  SpacQSafe_t qSafe);

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
SpacEthRtnCode_t SpacQMngTry(SpacQMngPtrType qMngPtr, int * numCellsPtr);


#endif /* _SPAC_QMNG_H_ */
