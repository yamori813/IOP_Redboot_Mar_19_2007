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
* FILENAME: 1.	SpacSynch.h
*
* DESCRIPTION: This file contains the IPP synchronization routines. They are
*            : implemented as wrappers around the primitives specific to  
*            : a particular RTOS.
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     modified synchronization function signatures.
*----------------------------------------------------------------------------*/

#ifndef _SPAC_SYNC_H_
#define _SPAC_SYNC_H_

/* semaphore creation types */
typedef enum {
  SPAC_MTX_SEM,    /* mutex */
  SPAC_CNT_SEM,    /* counting */
} SpacSemType;

/* semaphore timeout types */
typedef enum {
  SPAC_WAITFOREVER = -1,
  SPAC_NOWAIT = 0,
} SpacWaitType;

/*******************************************************************************
* SpacInterruptDisable
* DESCRIPTION:
* Call this routine in order to disable interrupts.
*
*
* RETURNS:
*    int - the interrupt key.
*
******************************************************************************/
int SpacInterruptDisable(void);

/*******************************************************************************
* SpacInterruptEnable
* DESCRIPTION:
* Call this routine in order to enable interrupts.
*
*
* RETURNS:
*    N/A
*
******************************************************************************/
void SpacInterruptEnable(int interruptKey);

/*******************************************************************************
* SpacSchedulerDisable
* DESCRIPTION:
* Call this routine in order to disable task/thread preemption.
*
*
* RETURNS:
*    int - the preemption key.
*
******************************************************************************/
int SpacSchedulerDisable(void);

/*******************************************************************************
* SpacSchedulerEnable
* DESCRIPTION:
* Call this routine in order to enable task/thread preemption.
*
*
* RETURNS:
*    N/A
*
******************************************************************************/
void SpacSchedulerEnable(int preemptionKey);

/*******************************************************************************
* SpacSemCreate
* DESCRIPTION:
* Call this routine in order to create semaphores.
*
*
* RETURNS:
*    ETH_OK - operation suceded..
*
******************************************************************************/
int SpacSemCreate(SpacSemType semType, int count, int * semIdPtr);

/*******************************************************************************
* SpacSemDelete
* DESCRIPTION:
* Call this routine in order to delete previously created semaphores.
*
*
* RETURNS:
*    ETH_OK - operation suceded.
*
******************************************************************************/
int SpacSemDelete(int * semIdPtr);

/*******************************************************************************
* SpacSemWait
* DESCRIPTION:
* Call this routine in order to take a semaphore.
*
*
* RETURNS:
*    ETH_OK - operation suceded..
*
*******************************************************************************/
int SpacSemWait(int * semIdPtr,int timeout);

/*******************************************************************************
* SpacSemTryWait
* DESCRIPTION:
* Call this routine in order to take a semaphore, if available.
*
*
* RETURNS:
*    ETH_OK - operation suceded..
*
******************************************************************************/
int SpacSemTryWait(int * semIdPtr);

/*******************************************************************************
* SpacSemSignal
* DESCRIPTION:
* Call this routine in order to signal a semaphore.
*
*
* RETURNS:
*    ETH_OK - operation suceded..
*
******************************************************************************/
int SpacSemSignal(int * semIdPtr);

/*******************************************************************************
* SpacNanosleep
* DESCRIPTION:
* Call this routine in order to relinquish the CPU for the parameter-specified
* number of nanoseconds.
*
*
* RETURNS:
*    N/A
*
******************************************************************************/
void SpacNanosleep(unsigned nanoseconds);

/*=========================================================================
 *  Function    : EthSynchCache
 *
 *  Arguments   : address - input, address of DMA buffer.
 *              : direction - input, TX/RX direction flag.
 *              : len - input, length of DMA buffer. 
 * 
 *  Description : Synchronize precessor's cache with the DMA buffers.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : Before reading from DMA memory we call this routine with 
 *              : direction = ETH_RX. After writing to DMA memory we call 
 *              : this routine with direction = ETH_TX.
 =========================================================================*/
int EthSynchCache(void * address, unsigned len, 
  unsigned direction);
  
/*=========================================================================
 *  Function    : EthVirt2Bus
 *
 *  Arguments   : virtAddr - input, virtual address.
 *              : busAddr - output, buss address.
 * 
 *  Description : Translate a virtual address into a buss address.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : 
 =========================================================================*/

int EthVirt2Bus(unsigned virtAddr, unsigned * busAddrPtr);
  
/*=========================================================================
 *  Function    : EthHaltSystem
 *  Arguments   : N/A.
 * 
 *  Description : Disables RX. Halts the DMA test processing.  
 *              : Disables xScale interrupts.
 * 
 *  return      : N/A. 
 *
 *  Note        : This is intended for debugging purposes.
 =========================================================================*/
void EthHaltSystem(void);
  
/*=========================================================================
 *  Function    : EthDbgSystem
 *  Arguments   : message - message to be printed.
 *              : tagNum - tag number.
 *              : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : freeEntries - free entries in queue.
 *              : tagNum - location tag.
 *
 *  Description : Prints debug information.  
 * 
 *  return      : N/A. 
 *
 *  Note        : This is intended for debugging purposes.
 =========================================================================*/

void EthDbgSystem(const char * message, unsigned portId, unsigned qId, 
  unsigned freeEntries, unsigned tagNum);


#endif /* _SPAC_SYNC_H_ */

