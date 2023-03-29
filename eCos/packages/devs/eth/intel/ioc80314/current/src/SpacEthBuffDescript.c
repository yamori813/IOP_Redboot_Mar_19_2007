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
* FILENAME: 1.	SpacEthBuffDescript.c
*
* DESCRIPTION: Contains implementations of routines acting on TX/RX buffer
*            : descriptors.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added test routines. 
* NOTE: Clean-up(TX/RX "flush") routines should be implemented, in case 
*       of TX/RX stalling.
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

/* SPac GigE driver global object. */
extern Eth_t EthDriver; 

#ifdef ETH_REDBOOT 

  extern SpacQMngType buffMng; 
  extern SpacQMngType buffHdrMng; 

/* Display (print) frame flag. */
  extern int gigEDisplayFrm;
/* Action at RX time. */
  extern EthDemux_t demuxAction;  

/* Test status table. */
  extern EthTestDone_t ethTestStatus[];
/* Test 6_2 status table. */
  extern Eth6_2Port_t test6_2Status[ETH_NUM_PORTS];

/* local source. */  
  char sourceAddr[ETH_NUM_PORTS][ETH_MAC_ADDR_LEN]; 

#endif

#ifdef ETH_DIAG_EXTENDED
  extern UINT32 qInterruptsTable[ETH_NUM_PORTS][ETH_NUM_PORT_DIRECTIONS][ETH_NUM_Q_INTR];
#endif  //  ETH_DIAG_EXTENDED

/* SPac chip base address. */
extern  UINT32 SpacBaseAddr;
extern int net_debug;

/*=========================================================================
 *  Function    : EthSetupDescriptors
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : direction - input, TX/RX.
 *              : memSpaceId - input, PCI/SDRAM/SRAM. 
 * 
 *  Description : Sets up a ring descriptor scheme.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_MEM_ALLOC_ERR - Memory allocation error.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : The "putInIdx/takeOutIdx" points to the location in the 
 *              : descriptor ring where the next "PUT"/"GET" operation
 *              : shall take place at TX/RX time. It is maintained by
 *              : the  software.
 *              : This routine would be called before enabling a queue.
 =========================================================================*/
SpacEthRtnCode_t EthSetupDescriptors(
  UINT32 portId, 
  UINT32 qId,
  EthDirFlow_t direction,
  UINT32 memSpaceId
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthDescrInfoPtr_t descrInfoPtr = NULL; 
  EthMemPoolPtr_t memPoolPtr = NULL;
  UINT32 ocnPortId = 0xFFFFFFFF;  
  EthMemDescrPtr_t memDescrPtr = NULL;  
  UINT32 descrMemSize = 0;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSetupDescriptors>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(qId >= ETH_NUM_TX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSetupDescriptors>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  descrInfoPtr = (direction == ETH_TX ? 
    &(EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo) : 
    &(EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo));
    
  memPoolPtr = (direction == ETH_TX ? 
    &(EthDriver.port[portId].mac.tx.txQ[qId].memPool) : 
    &(EthDriver.port[portId].mac.rx.rxQ[qId].memPool));
      
  if((descrInfoPtr->numDescr == 0) || 
     (descrInfoPtr->numDescr > ETH_NUM_DESCR_PER_Q)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSetupDescriptors>, \
             not supported number of descriptors = %d.", 
             descrInfoPtr->numDescr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  descrMemSize = descrInfoPtr->numDescr * ETH_DESCR_SIZE;
  
/* Use platform-specific means to get aligned memory. */
  if((memSpaceId == ETH_OCN_PCI1_ID) || (memSpaceId == ETH_OCN_PCI2_ID)){
    memDescrPtr = 
      (EthMemDescrPtr_t)((memSpaceId == ETH_OCN_PCI1_ID ? XS_PCI1_BASE_ADDR : XS_PCI2_BASE_ADDR));
    memPoolPtr->descrPoolPtr = 
      memDescrPtr->portDescr[portId].directDescrQ[direction].descrQ[qId].descrTable;
  }else if(memSpaceId == ETH_OCN_SDRAM_ID){
#ifdef HIGH_MEM_ALLOC
    memDescrPtr = (EthMemDescrPtr_t)SDRAM_HIGH_MEM;
    memPoolPtr->descrPoolPtr = 
      memDescrPtr->portDescr[portId].directDescrQ[direction].descrQ[qId].descrTable;
#else    
    memPoolPtr->descrPoolPtr = MEMALIGN(ETH_MEM_ALIGN, descrMemSize);
	if (net_debug == true) {
		db_printf ("Malloc'ing 0x%08x bytes for descrPoolPtr\n", descrMemSize);
		db_printf ("DescrPoolPtr returned: 0x%08x\n", memPoolPtr->descrPoolPtr);
	}
#endif    
  }else if(memSpaceId == ETH_OCN_XSCALE_ID){
    memDescrPtr = (EthMemDescrPtr_t)HAL_IOC80314_SRAM_BASE;
    memPoolPtr->descrPoolPtr = 
      memDescrPtr->portDescr[portId].directDescrQ[direction].descrQ[qId].descrTable;
  }else{
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSetupDescriptors>, \
             not supported memory space id = %d.", 
             memSpaceId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }
      
  if(memPoolPtr->descrPoolPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSetupDescriptors>, could not allocate descriptor \
        pool.");
    #endif // DEBUG_OVI
    return ETH_MEM_ALLOC_ERR;
  }else{
    memPoolPtr->putInIdx = memPoolPtr->takeOutIdx = 0;
  }
    
/* MUST clear the descriptor pool. */
  MEMSET((char *)memPoolPtr->descrPoolPtr,  0x00, descrMemSize); 

  {    
  int idx;      
/* Loop until the last descriptor/buffer pair, and set the descriptor 
   fields appropriately. */
    for(idx = 0; idx < descrInfoPtr->numDescr; idx++){
      rtnCode = sysVirtualToOcn(memPoolPtr->descrPoolPtr + ((idx +1) % descrInfoPtr->numDescr), 
        (unsigned int*)(&(memPoolPtr->descrPoolPtr[idx].nextDescrAddr1)), 
        (unsigned int*)(&(memPoolPtr->descrPoolPtr[idx].nextDescrAddr0)), &ocnPortId);
      if(rtnCode != 0){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthSetupDescriptors>, <sysVirtualToOcn> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }  
    }

    if(descrInfoPtr->descrTermMode == ETH_DESCR_TERM_LAST){
      memPoolPtr->descrPoolPtr[idx - 1].nextDescrAddr1 |= ETH_DESCR_LAST; 
    }else if(descrInfoPtr->descrTermMode == ETH_DESCR_TERM_NULL){
      memPoolPtr->descrPoolPtr[idx - 1].nextDescrAddr1 &= ~ETH_DESCR_ADDR_HI_MASK; 
      memPoolPtr->descrPoolPtr[idx - 1].nextDescrAddr0 = 0x00000000; 
    }
    
    EthSynchCache(memPoolPtr->descrPoolPtr, descrMemSize, ETH_TX);
    
  }
    
  memPoolPtr->freeEntries = descrInfoPtr->numDescr;
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthPutFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : framePtr - input, frame to transmit.
 *              : txFrmCfgInfoPtr - per-frame config info.
 * 
 *  Description : Puts a frame in a specified TX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_Q_FULL_ERR - Queue(TX ring) full condition.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : It is assumed that prior to enqueue a frame, a check for the
 *              : ring available space("freeEntries" value), has been done .
 =========================================================================*/
SpacEthRtnCode_t EthPutFrame(
  UINT32 portId, 
  UINT32 qId,
  EthSVFramePtr_t framePtr,
  EthTxFrmCfgInfoPtr_t txFrmCfgInfoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPutFrame>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(qId >= ETH_NUM_TX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPutFrame>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(txFrmCfgInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPutFrame>, \
             invalid TX config/status pointer = %p.", txFrmCfgInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(framePtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPutFrame>, \
             invalid frame pointer = %p.", framePtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(framePtr->buffPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPutFrame>, \
             invalid data pointer = %p.", framePtr->buffPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(framePtr->buffLen == 0){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPutFrame>, \
             invalid data length = %d.", framePtr->buffLen);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   

  
  { 
    EthTxFrmStatusCfgPtr_t txFrmCfgPtr = NULL;
    EthVlanBCntPtr_t txVlanBCntPtr = NULL;
    EthSVFramePtr_t walkingFrmPtr = framePtr;
    EthMemPoolPtr_t memPoolPtr = 
      &(EthDriver.port[portId].mac.tx.txQ[qId].memPool);
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
    UINT32 ocnPortId = 0xFFFFFFFF;      
    EthQPtrHi_t qPtrHi;
    *((UINT32 *)&qPtrHi) = 0x00000000;
    
    do{  
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx]), ETH_DESCR_SIZE, ETH_RX);      
/* Point to the descriptor's Config dword. */    
      txFrmCfgPtr = 
        (EthTxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].cfgStatus);        
/* If the "owner" bit of the pointed descriptor is own by DMA, or the 
   data buffer address is not NULL, the ring is full. It shouldn't happen if 
   checking for available space has ben done prior to call this routine. */
      if((txFrmCfgPtr->owner == ETH_DESCR_DMA) || 
         (memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0 != 0x00000000) || 
         ((memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000)){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthPutFrame>, TX queue full.");
        #endif // DEBUG_OVI
        return ETH_Q_FULL_ERR;    
      }else{
/* Cache synchronization. This may be done before reaching this point. */    
        EthSynchCache((void *)walkingFrmPtr->buffPtr, walkingFrmPtr->buffLen, ETH_TX); 
/* Virtual-to-OCN address translation. */    
        rtnCode = sysVirtualToOcn((void *)walkingFrmPtr->buffPtr,
          (unsigned int*)(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1)), 
          (unsigned int*)(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0)), &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthPutFrame>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
   
/* This should be atomic. */
        memPoolPtr->freeEntries--;
                
/* Frame configuration. */    
        txFrmCfgPtr->descrIntr = txFrmCfgInfoPtr->txFrmStatusCfg.descrIntr;
        txVlanBCntPtr = 
          (EthVlanBCntPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].vlanBCnt);
        txVlanBCntPtr->byteCount = walkingFrmPtr->buffLen;    
                 
        if(walkingFrmPtr == framePtr){ /* sof. */
          txFrmCfgPtr->padFrame = txFrmCfgInfoPtr->txFrmStatusCfg.padFrame;
          txFrmCfgPtr->hugeFrame = txFrmCfgInfoPtr->txFrmStatusCfg.hugeFrame;
/*#ifdef ETH_REDBOOT  - cebruns - should ETH_REDBOOT be def'd?
          if(doGigE7_6 == TRUE){
            txFrmCfgPtr->crc = ETH_BIT_SET;          
            txFrmCfgPtr->pfVlan = ETH_BIT_CLEARED;
          }else{
            txFrmCfgPtr->crc = txFrmCfgInfoPtr->txFrmStatusCfg.crc;          
            txFrmCfgPtr->pfVlan = txFrmCfgInfoPtr->txFrmStatusCfg.pfVlan;
          }
          if((doGigE6_6 == FALSE) || 
            (ethTestStatus[ETH_TEST_6_6].port[portId].direction[ETH_TX] != ETH_TEST_NOT_RUN)){
            txFrmCfgPtr->sof = ETH_BIT_SET;   
          }
#else
          txFrmCfgPtr->crc = txFrmCfgInfoPtr->txFrmStatusCfg.crc;          
          txFrmCfgPtr->pfVlan = txFrmCfgInfoPtr->txFrmStatusCfg.pfVlan;
          txFrmCfgPtr->sof = ETH_BIT_SET;   
#endif
*/
          if(txFrmCfgPtr->pfVlan){
            txVlanBCntPtr->vlanTag = txFrmCfgInfoPtr->vlanData;
          }   

/* Update head/tail pointers. */   
          if(memPoolPtr->qHead == NULL){ /* First "put". */
            memPoolPtr->qHead = framePtr;        
          }else{
            ((EthSVFramePtr_t)(memPoolPtr->qTail))->nextPtr = framePtr;        
          }
        }         
        if(walkingFrmPtr->nextPtr == NULL){ /* eof. */
/* Update tail pointer. */           
          memPoolPtr->qTail = walkingFrmPtr;
                    
        }

/* Update "walking" pointer. */      
        walkingFrmPtr = walkingFrmPtr->nextPtr;
/* This should be the last step for each descriptor. */      
        txFrmCfgPtr->owner = ETH_DESCR_DMA;       
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx]), ETH_DESCR_SIZE, ETH_TX);      
/* Update "PUT-IN" index. */
        memPoolPtr->putInIdx = (++memPoolPtr->putInIdx) % numDescr;
       }
    }while(walkingFrmPtr != NULL);
/* Re-validate the queue. */  
    qPtrHi.valid = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, 
      ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), 
      (UINT32 *)&qPtrHi, REG_OR);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthPutFrame>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_OR, 
          ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
  }  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthFreeTxFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3). 
 *              : memPoolPtr - input, used to retrieve the "TAKE-OUT" pointer.
 * 
 *  Description : Releases transmitted frames from a specified TX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_FREE_BUFF_ERR - Buffer release error.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This would be a good place to gather statistics about 
 *              : transmitted frames.
 =========================================================================*/
SpacEthRtnCode_t EthFreeTxFrame(
  UINT32 portId, 
  UINT32 qId,  
  EthMemPoolPtr_t memPoolPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFreeTxFrame>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(qId >= ETH_NUM_TX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFreeTxFrame>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(memPoolPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFreeTxFrame>, port id %d, queue id %d, \
             invalid memory pool pointer = %p.", portId, qId, memPoolPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(memPoolPtr->qHead == NULL){ /* Ring empty. */
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFreeTxFrame>, port id %d, queue id %d, TX queue empty.", portId, qId);
    #endif // DEBUG_OVI
    /*  return ETH_PARAM_ERR; */ return ETH_OK;
  } 
  
  {
/* Point to the descriptor's Config/Status dword. */    
    EthTxFrmStatusCfgPtr_t txFrmStatusCfgPtr = 
      (EthTxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].cfgStatus);
    EthSVFramePtr_t walkingFrmPtr = (EthSVFramePtr_t)memPoolPtr->qHead;
    EthSVFramePtr_t qHeadFrmPtr = NULL;
    EthSVFramePtr_t prevFrmCellPtr = NULL; /* Previous cell frame pointer. */
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
    
/* Cache synchronization. */    
    EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);          
/* If the "owner" bit of the pointed descriptor is not own by DMA and the 
   data buffer address is not NULL, the ring has entries to be freed. */
    while((txFrmStatusCfgPtr->owner != ETH_DESCR_DMA) && 
       ((memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr0 != 0x00000000) || 
       ((memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000))){        
/* Update statistics, if EOF. */  
      if(txFrmStatusCfgPtr->eof == ETH_BIT_SET){
        EthGetTxFrameStatus(txFrmStatusCfgPtr);
      } 
/* Clear descriptor configuration/status. */      
      *((UINT32 *)txFrmStatusCfgPtr) = 0x00000000;       
/* Clear VLAN/Byte count configuration. */      
      memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].vlanBCnt = 0x00000000;
/* This should be the last clean-up step for each descriptor. */      
      memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr0 = 0x00000000;   
      memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr1 &= ~ETH_DESCR_ADDR_HI_MASK;   
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx]), ETH_DESCR_SIZE, ETH_TX);           
/* This should be atomic. */
      memPoolPtr->freeEntries++;
            
/* Update "TAKE-OUT" index. */
      memPoolPtr->takeOutIdx = (++memPoolPtr->takeOutIdx) % numDescr;
/* Point to the next descriptor's Config dword. */    
      txFrmStatusCfgPtr = 
        (EthTxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].cfgStatus); 
      prevFrmCellPtr = walkingFrmPtr;
      walkingFrmPtr = walkingFrmPtr->nextPtr;    
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);           
    } 
     
    if(prevFrmCellPtr != NULL){  
      qHeadFrmPtr = (EthSVFramePtr_t)(memPoolPtr->qHead);   
/* Update head/tail pointers. */         
      memPoolPtr->qHead = walkingFrmPtr;
      if(memPoolPtr->qHead == NULL){
        memPoolPtr->qTail = NULL;
      }
      prevFrmCellPtr->nextPtr = NULL;
/* Release frame. */     
      rtnCode = EthReleaseFrm(qHeadFrmPtr);
      if(rtnCode != ETH_OK){ /* shouldn't happen. */
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthFreeTxFrame>,  port id %d, queue id %d, \
            <EthReleaseFrm> returns %d.", portId, qId, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }  
      
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthFreeTxFrame>, port id %d, queue id %d, frame transmitted.", 
          portId, qId);
      #endif // DEBUG_OVI
      
    }
  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthFeedRxQ
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : framePtrPtr - input, linked-list of buffers to receive into. 
 *              : descrIntr - input, allows per-descriptor interrupts.
 * 
 *  Description : Feeds buffers to a specified RX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_Q_FULL_ERR - Queue(RX ring) full condition. 
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This routine would would be called immediately after  
 *              : EthSetupDescriptors on a RX queue, and before enabling
 *              : the queue.
 *              : It is assumed that prior to feed a frame, a check for the
 *              : ring available space("freeEntries" value), has been done .
 =========================================================================*/
SpacEthRtnCode_t EthFeedRxQ(
  UINT32 portId, 
  UINT32 qId,
  EthSVFramePtr_t * framePtrPtr,  
  BOOL descrIntr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthMemPoolPtr_t memPoolPtr = NULL;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFeedRxQ>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFeedRxQ>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  memPoolPtr = &(EthDriver.port[portId].mac.rx.rxQ[qId].memPool);
  
  if(memPoolPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFeedRxQ>, port id %d, queue id %d, invalid memory pool pointer = %p.", 
        portId, qId, memPoolPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
    
  if(framePtrPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFeedRxQ>, port id %d, queue id %d, invalid frame pointer = %p.", 
        portId, qId, framePtrPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if((*framePtrPtr)->buffPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFeedRxQ>, port id %d, queue id %d, invalid data pointer = %p.", 
        portId, qId, (*framePtrPtr)->buffPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((*framePtrPtr)->buffLen == 0){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthFeedRxQ>, port id %d, queue id %d, invalid data length = %d.", 
        portId, qId, (*framePtrPtr)->buffLen);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   
  
  {
    EthRxFrmStatusCfgPtr_t rxFrmCfgPtr = NULL;
    EthSVFramePtr_t walkingFrmPtr = (*framePtrPtr);  
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;
    UINT32 ocnPortId = 0xFFFFFFFF;            
    
    do{     
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx]), ETH_DESCR_SIZE, ETH_RX);            
/* Point to the descriptor's Config dword. */    
      rxFrmCfgPtr = 
        (EthRxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].cfgStatus);  
/* If the "owner" bit of the pointed descriptor is own by DMA, or the 
   data buffer address is not NULL, the ring is full. It shouldn't happen if 
   checking for available space has ben done prior to call this routine. */
      if((rxFrmCfgPtr->owner == ETH_DESCR_DMA) || 
         (memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0 != 0x00000000) ||
         ((memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000)){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthFeedRxQ>, port id %d, queue id %d, RX queue full.", portId, qId);
        #endif // DEBUG_OVI
        *framePtrPtr = walkingFrmPtr;
        return ETH_Q_FULL_ERR;    
      }else{
/* Virtual-to-OCN address translation. */    
        rtnCode = sysVirtualToOcn((void *)(walkingFrmPtr->buffPtr), 
          (unsigned int*)(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1)), 
          (unsigned int*)(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0)), &ocnPortId);
        if(rtnCode != 0){
          memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0 = 0x00000000;   
          memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1 &= ~ETH_DESCR_ADDR_HI_MASK;            
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthFeedRxQ>, port id %d, queue id %d, <sysVirtualToOcn> returns %d.", 
            portId, qId, rtnCode);
          #endif // DEBUG_OVI
          *framePtrPtr = walkingFrmPtr;
          return rtnCode;
        } 
/* Clear VLAN/Byte count configuration. Needed in the WAIT ISR. */      
        memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].vlanBCnt = 0x00000000;
/* This should be atomic. */
        memPoolPtr->freeEntries--;
                
        if(walkingFrmPtr == (*framePtrPtr)){ /* first buffer. */
/* Update head/tail pointers. */   
          if(memPoolPtr->qHead == NULL){ /* First "put". */
            memPoolPtr->qHead = (*framePtrPtr);        
          }else{
            ((EthSVFramePtr_t)(memPoolPtr->qTail))->nextPtr = (*framePtrPtr);        
          }
        }
/* Frame configuration. */    
        rxFrmCfgPtr->byteCount = (*framePtrPtr)->buffLen;
        rxFrmCfgPtr->descrIntr = descrIntr;      
        if(walkingFrmPtr->nextPtr == NULL){ /* last buffer. */
/* Update tail pointer. */           
          memPoolPtr->qTail = walkingFrmPtr;
        }      
/* Update "walking" pointer. */      
        walkingFrmPtr = walkingFrmPtr->nextPtr;      
/* This should be the last step for each descriptor. */      
        rxFrmCfgPtr->owner = ETH_DESCR_DMA; 
/* Cache synchronization. */    
        EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx]), ETH_DESCR_SIZE, ETH_TX);            
/* Update "PUT-IN" index. */
        memPoolPtr->putInIdx = (++memPoolPtr->putInIdx) % numDescr;
      }   
    }while(walkingFrmPtr != NULL);
  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthGetFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 * 
 *  Description : Gets received frames from a specified RX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_INTEGRITY_ERR - Queue integrity error. 
 *
 *  Note        : This would be a good place to gather statistics about 
 *              : received frames. 
 *              : We chose to allow frame processing despite an error at
 *              : release invalid frame and demux time.
 =========================================================================*/
SpacEthRtnCode_t EthGetFrame(
  UINT32 portId, 
  UINT32 qId
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthRxQPtr_t rxQPtr = NULL;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetFrame>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetFrame>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  rxQPtr = &(EthDriver.port[portId].mac.rx.rxQ[qId]);
  
  if(rxQPtr->memPool.qHead == NULL){ /* Ring empty or header list insane. */
        
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetFrame>, port id %d, queue id %d, list integrity error.", portId, qId);
    #endif // DEBUG_OVI
    
    return ETH_Q_INTEGRITY_ERR;
  } 
  
  {      
/* Point to the descriptor's Config dword. */    
    EthRxFrmStatusCfgPtr_t rxFrmStatusCfgPtr = 
      (EthRxFrmStatusCfgPtr_t)&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].cfgStatus);
    EthSVFramePtr_t walkingFrmPtr = NULL;
    EthSVFramePtr_t qHeadFrmPtr = NULL;
    EthVlanBCntPtr_t vlanBCntPtr = NULL;
    BOOL foundEof = FALSE;
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;
          
    if(rxQPtr->prevFrmCellPtr != NULL){  
      walkingFrmPtr = rxQPtr->prevFrmCellPtr->nextPtr;   
    }else{
      walkingFrmPtr = (EthSVFramePtr_t)(rxQPtr->memPool.qHead);
    }  
      
/* Cache synchronization. */    
    EthSynchCache(&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);                  
/* If the "owner" bit of the pointed descriptor is not own by DMA and the 
   data buffer address is not NULL, the ring has entries to be reassembled. */
    while((rxFrmStatusCfgPtr->owner != ETH_DESCR_DMA) && 
      ((rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr0 != 0x00000000) ||
      ((rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000))){

/* Point to the descriptor's VlanBcount dword. */    
      vlanBCntPtr = 
        (EthVlanBCntPtr_t)&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].vlanBCnt);       
         
/* SOF or SOFEOF */      
      if(rxFrmStatusCfgPtr->sof == ETH_BIT_SET){ 
        if((rxQPtr->invalidFrm == TRUE) ||
           (rxQPtr->prevFrmCell == ETH_FRM_CELL) || 
           (rxQPtr->prevFrmCell == ETH_FRM_SOF)){ /* Invalid cell sequence. */
          if(rxQPtr->prevFrmCell != ETH_FRM_EOF){
            EthIncrRxErr();
          }
          if(rxQPtr->prevFrmCellPtr != NULL){  
            
            qHeadFrmPtr = (EthSVFramePtr_t)(rxQPtr->memPool.qHead);   
/* Update head/tail pointers. */         
            rxQPtr->memPool.qHead = walkingFrmPtr;
            if(rxQPtr->memPool.qHead == NULL){              
              rxQPtr->memPool.qTail = NULL;
            }
            rxQPtr->prevFrmCellPtr->nextPtr = NULL;
/* Release frame. */     
            rtnCode = EthReleaseFrm(qHeadFrmPtr);
/* Clear the per-frame cached info. */                         
            rxQPtr->prevFrmCellPtr = NULL;
            rxQPtr->prevFrmCell = ETH_FRM_UNKNOWN;  
            rxQPtr->invalidFrm = FALSE;
            rxQPtr->rxSofFrmStatus = 0x00000000;
            rxQPtr->vLanTag = 0x0000;
            
            if(rtnCode != ETH_OK){ /* shouldn't happen. */
              #ifdef DEBUG_OVI
                db_printf("\nIn <EthGetFrame>, port id %d, queue id %d, <EthReleaseFrm> returns %d.", 
                  portId, qId, rtnCode);
              #endif // DEBUG_OVI
/* Do not return. Allow proceding with the next frame.      
              return rtnCode; */
            }    
          }else{ /* shouldn't happen. */
          
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthGetFrame>, port id %d, queue id %d, RX queue integrity error.", 
              portId, qId);
            #endif // DEBUG_OVI
            return ETH_Q_INTEGRITY_ERR;
          }
        } 
/* Save data in case this frame may be completed in a next call. */      
        rxQPtr->rxSofFrmStatus = *((UINT32 *)rxFrmStatusCfgPtr);
        if(rxFrmStatusCfgPtr->vtf == ETH_BIT_SET){
          rxQPtr->vLanTag = vlanBCntPtr->vlanTag;
        }
        rxQPtr->prevFrmCell = ETH_FRM_SOF;  
        if(rxFrmStatusCfgPtr->eof == ETH_BIT_SET){/* SOFEOF. */ 
          foundEof = TRUE;
          rxQPtr->prevFrmCell = ETH_FRM_SOFEOF;  
        }   
        rxQPtr->invalidFrm = FALSE;
/* This could potentially invalidate the frame. */      
        EthGetRxFrameStatus(rxQPtr);
      }
/* CELL or EOF */     
      else if(rxFrmStatusCfgPtr->sof == ETH_BIT_CLEARED){ 
        if((rxQPtr->prevFrmCell != ETH_FRM_SOF) &&
           ((rxQPtr->prevFrmCell != ETH_FRM_CELL) || 
            (rxQPtr->invalidFrm == TRUE))){ /* Invalid cell sequence. */
          rxQPtr->invalidFrm = TRUE;
        }
        if(rxFrmStatusCfgPtr->eof == ETH_BIT_CLEARED){ /* CELL */
          rxQPtr->prevFrmCell = ETH_FRM_CELL;
        }else{ /* EOF */
          if(rxQPtr->invalidFrm == TRUE){
            EthIncrRxErr();
          }
          foundEof = TRUE;
          rxQPtr->prevFrmCell = ETH_FRM_EOF;  
        }    
      }      
      walkingFrmPtr->buffLen = vlanBCntPtr->byteCount;  
/* Cache synchronization. */    
      EthSynchCache((void *)walkingFrmPtr->buffPtr, walkingFrmPtr->buffLen, ETH_RX);
/* Clear frame configuration. */      
      *((UINT32 *)rxFrmStatusCfgPtr) = 0x00000000;      
/* Clear VLAN/Byte count configuration. */      
      rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].vlanBCnt = 0x00000000;
/* This should be the last clean-up step for each descriptor. */      
      rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr0 = 0x00000000;   
      rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr1 &= ~ETH_DESCR_ADDR_HI_MASK;  
/* Cache synchronization. */    
      EthSynchCache(&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx]), ETH_DESCR_SIZE, ETH_TX);                  
/* This should be atomic. */
      rxQPtr->memPool.freeEntries++;
            
/* Update "TAKE-OUT" index. */
      rxQPtr->memPool.takeOutIdx = (++(rxQPtr->memPool.takeOutIdx)) % numDescr;
/* Cache synchronization. */    
      EthSynchCache(&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);                        
/* Point to the descriptor's Config dword. */    
      rxFrmStatusCfgPtr = 
        (EthRxFrmStatusCfgPtr_t)&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].cfgStatus);                 
/* Update "previous" and "current" pointers. */      
      rxQPtr->prevFrmCellPtr = walkingFrmPtr;
      walkingFrmPtr = walkingFrmPtr->nextPtr;
/* Call a "demux" routine to present the extracted frame to a registered 
   client. */    
      if((foundEof == TRUE) && (rxQPtr->invalidFrm == FALSE)){    
        EthIncrRx();
        qHeadFrmPtr = (EthSVFramePtr_t)rxQPtr->memPool.qHead;   
    /* Update head/tail pointers. */         
        rxQPtr->memPool.qHead = walkingFrmPtr;
        if(rxQPtr->memPool.qHead == NULL){
          
          rxQPtr->memPool.qTail = NULL;
        }
        rxQPtr->prevFrmCellPtr->nextPtr = NULL;
        
    /* Demux frame. */     
        rtnCode = EthRxDemux(portId, qId, qHeadFrmPtr);
    /* Clear the per-frame cached info. */             
        rxQPtr->prevFrmCellPtr = NULL;
        rxQPtr->prevFrmCell = ETH_FRM_UNKNOWN;          
        rxQPtr->rxSofFrmStatus = 0x00000000;
        rxQPtr->vLanTag = 0x0000;
        if(rtnCode != ETH_OK){ /* shouldn't happen. */
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthGetFrame>, port id %d, queue id %d, <EthRxDemux> returns %d.", 
            portId, qId, rtnCode);
          #endif // DEBUG_OVI
/* Do not return. Allow proceding with the next frame.              
          return rtnCode; */
        } 
      }
    }
  }   

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthIncrRx
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of RX valid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrRx(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  return rtnCode;
    
}
 
/*=========================================================================
 *  Function    : EthIncrRxErr
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of RX invalid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrRxErr(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  return rtnCode;
    
}
 
/*=========================================================================
 *  Function    : EthReleaseFrm
 *
 *  Arguments   : frmPtr - pointer to frame to be released.
 * 
 *  Description : Releases frames back to the memory pool.
 * 
 *  return      : ETH_OK - Operation succeded.
 *              : ETH_PARAM_ERR - Invalid parameter. 
 *
 *  Note        : N/A 
 =========================================================================*/
SpacEthRtnCode_t EthReleaseFrm(
EthSVFramePtr_t frmPtr
)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthSVFramePtr_t walkingFrmPtr = NULL;
#ifdef ETH_REDBOOT   
  extern SpacQMngType buffMng; /* Bufer pool. */
  extern SpacQMngType buffHdrMng; /* Bufer header pool. */
#endif  

  while(frmPtr != NULL){
    if(frmPtr->buffPtr != NULL){
      rtnCode = SpacQMngReturn(&buffMng, (void *)frmPtr->buffPtr, SPAC_NOT_SAFE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthReleaseFrm> frmPtr->buffPtr if statement, <SpacQMngReturn> returns %d.", 
            rtnCode);
        #endif // DEBUG_OVI
/* Give a chance to release other buffers. */        
      }
    }
    walkingFrmPtr = frmPtr->nextPtr;
    frmPtr->nextPtr = NULL;
    frmPtr->buffPtr = NULL;
    frmPtr->buffLen = 0;
    rtnCode = SpacQMngReturn(&buffHdrMng, frmPtr, SPAC_NOT_SAFE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthReleaseFrm> frmPtr->nextPtr if statement, <SpacQMngReturn> returns %d.", 
          rtnCode);
      #endif // DEBUG_OVI
/* Give a chance to release other buffers. */        
    }
    frmPtr = walkingFrmPtr;
  }

  return rtnCode;
    
}
 
/*=========================================================================
 *  Function    : EthGetRxFrameStatus
 *
 *  Arguments   : rxQPtr - input/output. Specifies a particular RX queue.
 * 
 *  Description : Gets status info for a received frame.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This routine only updates the invalid frame flag, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthGetRxFrameStatus(EthRxQPtr_t rxQPtr)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(rxQPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetRxFrameStatus>, \
             invalid RX queue pointer = %p.", rxQPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  {
    EthRxFrmStatusCfgPtr_t rxFrmStatusCfgPtr = 
      (EthRxFrmStatusCfgPtr_t)&(rxQPtr->rxSofFrmStatus);
  
    if(rxFrmStatusCfgPtr->badFrame == ETH_BIT_SET){
      rxQPtr->invalidFrm = TRUE;    
    #ifdef DEBUG_OVI
      db_printf("<EthGetRxFrameStatus> rxFrmStatusCfgPtr->badFrame set\n");
    #endif // DEBUG_OVI
      
      return 1;
    }
  }
  
  return rtnCode;
    
}
 
/*=========================================================================
 *  Function    : EthGetTxFrameStatus
 *
 *  Arguments   : txFrmStatusCfgPtr - TX status info.
 * 
 *  Description : Gets status info for a transmitted frame.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This routine only updates the invalid frame flag, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthGetTxFrameStatus(EthTxFrmStatusCfgPtr_t txFrmStatusCfgPtr)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(txFrmStatusCfgPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetTxFrameStatus>, \
             invalid TX status info pointer = %p.", txFrmStatusCfgPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(txFrmStatusCfgPtr->frmTxOk == ETH_BIT_CLEARED){
    EthIncrTx();    
  }else{
  /* Gather statistics, etc... */
    EthIncrTxErr();
  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthIncrTx
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of TX valid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrTx(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthIncrTxErr
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of TX invalid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrTxErr(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxDemux
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : frmPtr - pointer to frame to be passed out.
 * 
 *  Description : Dispatches a RX frame to the upper layer.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_FREE_BUFF_ERR - Buffer release error.
 *              : ETH_Q_FULL_ERR - Queue(TX ring) full condition.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthRxDemux(
  UINT32 portId, 
  UINT32 qId,
  EthSVFramePtr_t frmPtr
)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
#ifdef ETH_REDBOOT   
  extern EthDemux_t demuxAction;
  EthTxFrmCfgInfo_t txFrmCfgInfo;
  UINT32 frmCount = 0, frmLen = 0;
  EthMemPoolPtr_t memPoolPtr = NULL;
  EthRxQPtr_t rxQPtr = NULL; 
  EthMacMaxFrm_t macMaxFrmInfo; 
    
#endif  

  if(frmPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxDemux>, invalid frame pointer = %p.", frmPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxDemux>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    goto discard;
  } 

  if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxDemux>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    goto discard;
  } 

  if((EthDriver.portInfo[portId]->txInfo.txCtrlInfo.qEnables & (1 << qId)) == 
    FALSE){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxDemux>, disabled queue id = %d.", qId);
    #endif // DEBUG_OVI
    goto discard;
  }
  
#ifdef ETH_REDBOOT   

  #ifdef DEBUG_OVI
    db_printf("In <EthRxDemux>, port id %d, queue id %d, frame received\n", portId, qId);
  #endif // DEBUG_OVI
  
  if(demuxAction == ETH_DEMUX_LOOPBACK){
    rxQPtr = &(EthDriver.port[portId].mac.rx.rxQ[qId]);
    
    
    if(gigEDisplayFrm){
      rtnCode = EthDumpFrame((unsigned char*)frmPtr->buffPtr, frmPtr->buffLen);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxDemux>, <EthDumpFrame> returns %d.", rtnCode);
        #endif // DEBUG_OVI
      }
    }
    
    txFrmCfgInfo.vlanData = 0x0000;
    *((UINT32 *)&(txFrmCfgInfo.txFrmStatusCfg)) = 0x00000000;
    
    txFrmCfgInfo.txFrmStatusCfg.crc = ETH_BIT_SET;
    txFrmCfgInfo.txFrmStatusCfg.descrIntr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.descrIntr;
    memPoolPtr = &(EthDriver.port[portId].mac.tx.txQ[qId].memPool);  
    
    if(((EthRxFrmStatusCfgPtr_t)&(rxQPtr->rxSofFrmStatus))->vtf == ETH_BIT_SET){
      txFrmCfgInfo.vlanData = rxQPtr->vLanTag;
      txFrmCfgInfo.txFrmStatusCfg.pfVlan = ETH_BIT_SET;
    }
    
    rtnCode = EthRegAccess(portId, ETH_MAC_MAX_FRAME_REG, 
      (UINT32 *)&(macMaxFrmInfo), REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxDemux>, <EthRegAccess> access %d to %08X \
          for port %d returns %d.", REG_READ, ETH_MAC_MAX_FRAME_REG, 
          portId, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    
/* Attempt to send the fresh-received frame. */  
    rtnCode = EthGetNumFrmBuffers(frmPtr, &frmCount, &frmLen); 
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxDemux>, <EthGetNumFrmBuffers> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      goto discard; 
    }
    
    if(frmLen > macMaxFrmInfo.maxFrmLen){
      txFrmCfgInfo.txFrmStatusCfg.hugeFrame = ETH_BIT_SET;
    }
    
    if(memPoolPtr->freeEntries >= frmCount){
      rtnCode = EthFreeTxFrame(portId, qId, memPoolPtr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxDemux>, <EthFreeTxFrame> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        goto discard;         
      }
      
/* The received frame would need to have the last 4 bytes (CRC data) stripped-off, 
   and the MAC addresses swapped, before sending the frame out. */  
      rtnCode = EthProcessRxFrame(portId, frmPtr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxDemux>, <EthProcessRxFrame> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        goto discard; 
      }

      rtnCode = EthPutFrame(portId, qId, frmPtr, &txFrmCfgInfo);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxDemux>, <EthPutFrame> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        goto discard; 
      }else{
        return rtnCode;
      }
    }else{
      goto discard;
    }  
  }else if(demuxAction == ETH_DEMUX_DISCARD){
    if(gigEDisplayFrm){
      rtnCode = EthDumpFrame((unsigned char*)frmPtr->buffPtr, frmPtr->buffLen);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxDemux>, <EthDumpFrame> returns %d.", rtnCode);
        #endif // DEBUG_OVI
      }
    }
    goto discard;
  }
#endif  
  
  discard:
    
    rtnCode = EthReleaseFrm(frmPtr);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxDemux>, <EthReleaseFrm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
  
  return rtnCode;
    
}


/*=========================================================================
 *  Function    : EthRedBootFreeTx
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : 
 * 
 *  Description : Releases the transmitted frames from the  TX queues.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This implementation uses a simplified priority scheme.
 *              : It assumes the following priority Q3 > Q2 > Q1 > Q0. 
 *              : This is intended to be used on a RedBoot platform, in an 
 *              : interrupt context.
 =========================================================================*/
SpacEthRtnCode_t EthRedBootFreeTx(
  UINT32 portId 
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRedBootFreeTx>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  {
    int qIdx;
    EthTxCtrlPtr_t txCtrlInfoPtr = &(EthDriver.portInfo[portId]->txInfo.txCtrlInfo);
    
    for(qIdx = (ETH_NUM_TX_Q - 1); qIdx >= 0; qIdx--){
      if(txCtrlInfoPtr->qEnables & (1 << qIdx)){            
/* Free transmitted frames. */      
        rtnCode = EthFreeTxFrame(portId, qIdx, &(EthDriver.port[portId].mac.tx.txQ[qIdx].memPool));
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRedBootFreeTx>, <EthFreeTxFrame> for queue %d \
              returns %d.", qIdx, rtnCode);
          #endif // DEBUG_OVI
/* Give a chance to service next queue.       
        return rtnCode; */
        }
      }
    }
  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthGetNewBuffers
 *
 *  Arguments   : memPoolPtr - input, used to retrieve handles to the memory 
 *              :              managers.
 *              : framePtr - output, linked-list of buffers to receive into. 
 * 
 *  Description : Gets new buffers to feed a specified queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_EMPTY_ERR - queue empty condition.
 *              : ETH_NOT_INIT_ERR - queue not initialized.
 *              : ETH_SYNC_OBJ_ERR - semaphore operation error. 
 *
 *  Note        : This routine would would be called immediately after  
 *              : EthSetupDescriptors on a RX queue, and before enabling
 *              : the queue.
 *              : Prior to feed a frame, a check for the available
 *              : queue space("freeEntries" value), shall be done .
 =========================================================================*/
SpacEthRtnCode_t EthGetNewBuffers(
  EthMemPoolPtr_t memPoolPtr,
  EthSVFramePtr_t * framePtrPtr 
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthSVFramePtr_t prevFramePtr = NULL;  
  int idx;
  void * workingPtr = NULL; 
  *framePtrPtr =  NULL;
  
  if(memPoolPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetNewBuffers>, invalid memory pool pointer = %p.", memPoolPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
        
  for(idx = 0; idx < memPoolPtr->freeEntries; idx++){    
/* Get a header. */      
    rtnCode = SpacQMngGet(&buffHdrMng, &workingPtr, SPAC_NOT_SAFE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthGetNewBuffers>, <SpacQMngGet> returns %d.", rtnCode);
      #endif // DEBUG_OVI
/* Return what you have gotten so far. */      
      return rtnCode;
    }else{
      ((EthSVFramePtr_t)workingPtr)->nextPtr = NULL;    
      if(idx == 0){
        *framePtrPtr = prevFramePtr = (EthSVFramePtr_t)workingPtr;
      }else{
        prevFramePtr->nextPtr = workingPtr;
      }
    } 
/* Get a data buffer. */          
    rtnCode = SpacQMngGet(&buffMng, 
      (void **)&(((EthSVFramePtr_t)workingPtr)->buffPtr), SPAC_NOT_SAFE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthGetNewBuffers>, <SpacQMngGet> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      if(idx == 0){
        *framePtrPtr =  NULL;
      }else{
        prevFramePtr->nextPtr = NULL; 
      }     
      rtnCode = EthReleaseFrm(workingPtr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthGetNewBuffers>, <EthReleaseFrm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
      return rtnCode;
    }else{
      ((EthSVFramePtr_t)workingPtr)->buffLen = buffMng.qMngCellSize;
    }  
    if(idx != 0){
      prevFramePtr = workingPtr;
    }
  }
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthQRx
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 4).
 * 
 *  Description : Processes a RX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_FULL_ERR - Queue(RX ring) full condition.  
 *              : ETH_Q_EMPTY_ERR - queue empty condition.
 *              : ETH_NOT_INIT_ERR - queue not initialized.
 *              : ETH_SYNC_OBJ_ERR - semaphore operation error. 
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthQRx(
  UINT32 portId,
  UINT32 qId
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthQRx>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthQRx>, invalid queue id = %d.", qId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  {
    EthSVFramePtr_t framePtr = NULL;   
    BOOL descrIntr = EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.descrIntr;
 
/* Get new received frames. */      
    rtnCode = EthGetFrame(portId, qId);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthQRx>, port id %d, queue id %d, <EthGetFrame> returns %d.", 
          portId, qId, rtnCode);
      #endif // DEBUG_OVI
//      return rtnCode; 
    }
   
/* Feed with new frame buffers. */      
    rtnCode = EthGetNewBuffers(&(EthDriver.port[portId].mac.rx.rxQ[qId].memPool), 
      &framePtr);
    if((rtnCode == ETH_OK) && (framePtr != NULL)){
      rtnCode = EthFeedRxQ(portId, qId, &framePtr, descrIntr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthQRx>, port id %d, queue id %d, <EthFeedRxQ> returns %d.", 
            portId, qId, rtnCode);
        #endif // DEBUG_OVI
        if(framePtr != NULL){
          rtnCode = EthReleaseFrm(framePtr);
          if(rtnCode != ETH_OK){ /* shouldn't happen. */
            #ifdef DEBUG_OVI
            db_printf("\nIn <EthQRx>, port id %d, queue id %d, <EthReleaseFrm> returns %d.", 
              portId, qId, rtnCode);
            #endif // DEBUG_OVI
            return rtnCode; 
          }    
        }
// do not return here. give EthGetFrame a chance.        
        return rtnCode;  
      }
    }else if(framePtr != NULL){
      rtnCode = EthReleaseFrm(framePtr);
      if(rtnCode != ETH_OK){ /* shouldn't happen. */
        #ifdef DEBUG_OVI
        db_printf("\nIn <EthQRx>, port id %d, queue id %d, <EthReleaseFrm> returns %d.", 
          portId, qId, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode; 
      }    
    }
  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthGetNumFrmBuffers
 *
 *  Arguments   : frmPtr - pointer to frame to be passed out.
 *              : frmCountPtr - pointer to counter value to be returned.
 *              : frmLenPtr - pointer to length value to be returned.  
 * 
 *  Description : Counts the number of comprising frame bufffers.
 * 
 *  return      : ETH_OK - Operation succeded.
 *              : ETH_PARAM_ERR - Invalid parameter.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthGetNumFrmBuffers(
  EthSVFramePtr_t frmPtr,
  UINT32 * frmCountPtr,
  UINT32 * frmLenPtr
)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  *frmLenPtr = 0;
  
  if(frmPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetNumFrmBuffers>, invalid frame pointer = %p.", 
        frmPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(frmCountPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthGetNumFrmBuffers>, invalid frame count pointer = %p.", 
        frmCountPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  *frmCountPtr = 0;
  
  do{
    ++(*frmCountPtr);
    *frmLenPtr += frmPtr->buffLen;
  }while((frmPtr = frmPtr->nextPtr) != NULL);


  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRedBootTx
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : frmPtr - Pointer to frame to be passed out. 
 *              : txFrmCfgInfoPtr - Pointer to frame config info.
 * 
 *  Description : Puts frames into the TX queues.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This implementation uses a simplified priority scheme.
 *              : It assumes the following priority Q3 > Q2 > Q1 > Q0. 
 *              : This is intended to be used on a RedBoot platform, in an 
 *              : interrupt context.
 =========================================================================*/
SpacEthRtnCode_t EthRedBootTx(
  UINT32 portId,
  EthSVFramePtr_t frmPtr,
  EthTxFrmCfgInfoPtr_t txFrmCfgInfoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(frmPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRedBootTx>, invalid frame pointer = %p.", 
        frmPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(txFrmCfgInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRedBootTx>, invalid frame config info pointer = %p.", 
        txFrmCfgInfoPtr);
    #endif // DEBUG_OVI
    goto discard;
  } 

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRedBootTx>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    goto discard;
  } 

  {
    int qIdx;
    UINT32 frmCount = 0, frmLen = 0;
    EthMemPoolPtr_t memPoolPtr = NULL;
    EthTxCtrlPtr_t txCtrlInfoPtr = &(EthDriver.portInfo[portId]->txInfo.txCtrlInfo);
    
    rtnCode = EthGetNumFrmBuffers(frmPtr, &frmCount, &frmLen); 
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRedBootTx>, <EthGetNumFrmBuffers> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      goto discard; 
    }
    
    for(qIdx = (ETH_NUM_TX_Q - 1); qIdx >= 0; qIdx--){
      if(txCtrlInfoPtr->qEnables & (1 << qIdx)){      
        memPoolPtr = &(EthDriver.port[portId].mac.tx.txQ[qIdx].memPool);    
  /* Attempt to send the fresh-received frame. */  
        if(memPoolPtr->freeEntries >= frmCount){
          rtnCode = EthFreeTxFrame(portId, qIdx, memPoolPtr);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthRedBootTx>, <EthFreeTxFrame> returns %d.", rtnCode);
            #endif // DEBUG_OVI
          }else{
            rtnCode = EthPutFrame(portId, qIdx, frmPtr, txFrmCfgInfoPtr);
            if(rtnCode != ETH_OK){
              #ifdef DEBUG_OVI
                db_printf("\nIn <EthRedBootTx>, <EthPutFrame> returns %d.", rtnCode);
              #endif // DEBUG_OVI
            }else{
              return rtnCode;
            }
          }         
        }
      }
    }
  }
  
  discard:
    rtnCode = EthReleaseFrm(frmPtr);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxDemux>, <EthReleaseFrm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthDumpFrame
 *
 *  Arguments   : frmPtr - buffer pointer.
 *              : len - number of bytes to be dumped.
 * 
 *  Description : Dumps frame bufffers.
 * 
 *  return      : ETH_OK - Operation succeded.
 *              : ETH_PARAM_ERR - Invalid parameter.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthDumpFrame(
  UINT8 * buffPtr, 
  UINT16 len
)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(buffPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDumpFrame>, invalid buffer pointer = %p.", 
        buffPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(len > ETH_MAX_BUFF_SIZE){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDumpFrame>, invalid length parameter = %d. \
        Trimmed down to %d.", len, ETH_MAX_BUFF_SIZE);
    #endif // DEBUG_OVI
    len =  ETH_MAX_BUFF_SIZE;
  } 
  
  #ifdef DEBUG_OVI
  {
    int idx;
    
    db_printf("\nIn <EthDumpFrame>, start frame dump :\n");
    for(idx = 0; idx < len; idx++){
      db_printf("%02X", buffPtr[idx]);
    }
    db_printf("\nIn <EthDumpFrame>, end frame dump.\n");
  }
  #endif // DEBUG_OVI

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthProcessRxFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : frmPtr - Pointer to frame to be processed. 
 * 
 *  Description : Removes the CRC and swaps the MAC addresses of a RX frame.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthProcessRxFrame(
  UINT32 portId,
  EthSVFramePtr_t frmPtr
)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


    #ifdef DEBUG_OVI
      db_printf("In <EthProcessRxFrame> on port id = %d\n", portId);
    #endif // DEBUG_OVI

  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthProcessRxFrame>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(frmPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthProcessRxFrame>, invalid frame pointer = %p.", frmPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }
  
  if(frmPtr->buffPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthProcessRxFrame>, invalid frame data pointer = %p.", frmPtr->buffPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }
  
  if(frmPtr->buffLen <= ETH_MAC_HEADER_LEN){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthProcessRxFrame>, invalid frame data length = %p.", frmPtr->buffLen);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }
  
/* Copy source into destination field, and local MAC address into source field. */
  MEMCPY((void *)frmPtr->buffPtr, (void *)(frmPtr->buffPtr + ETH_MAC_ADDR_LEN), ETH_MAC_ADDR_LEN);
  MEMCPY((void *)(frmPtr->buffPtr + ETH_MAC_ADDR_LEN), &(sourceAddr[portId][0]), ETH_MAC_ADDR_LEN);

/* Strip-off the CRC. */
  for( ; frmPtr->nextPtr != NULL; frmPtr = frmPtr->nextPtr ){
    ;
  }
/* This limits frame processing to frames with CRC data spawned in full in the last data bufer. */
  if(frmPtr->buffLen >= ETH_MAC_FCS_LEN){
    frmPtr->buffLen -= ETH_MAC_FCS_LEN;
  }else{
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthProcessRxFrame>, not supported frame data length = %d.", 
        frmPtr->buffLen);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

    #ifdef DEBUG_OVI
      db_printf("Finished <EthProcessRxFrame> on port id = %d\n", portId);
    #endif // DEBUG_OVI
  
  return rtnCode;
    
}


/*=========================================================================
 *  Function    : ioc80314GetNewBuffers
 *
 *  Arguments   : buffNum - input, the number of requested buffers.
 *              : framePtr - output, linked-list of buffers to receive into. 
 * 
 *  Description : Gets new buffers to feed a specified queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_EMPTY_ERR - queue empty condition.
 *              : ETH_NOT_INIT_ERR - queue not initialized.
 *              : ETH_SYNC_OBJ_ERR - semaphore operation error. 
 *
 *  Note        : This routine would would be called immediately after  
 *              : EthSetupDescriptors on a RX queue, and before enabling
 *              : the queue.
 *              : Prior to feed a frame, a check for the available
 *              : queue space("freeEntries" value), shall be done .
 =========================================================================*/
SpacEthRtnCode_t ioc80314GetNewBuffers(
  UINT32 buffNum,
  EthSVFramePtr_t * framePtrPtr 
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthSVFramePtr_t prevFramePtr = NULL;  
  int idx;
  void * workingPtr = NULL; 
  *framePtrPtr =  NULL;
  
  if(buffNum == 0){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314GetNewBuffers>, number of buffers = %d.", buffNum);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
        
  for(idx = 0; idx < buffNum; idx++){    
/* Get a header. */      
    rtnCode = SpacQMngGet(&buffHdrMng, &workingPtr, SPAC_NOT_SAFE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <ioc80314GetNewBuffers>, <SpacQMngGet> returns %d.", rtnCode);
      #endif // DEBUG_OVI
/* Return what you have gotten so far. */      
      return rtnCode;
    }else{
      ((EthSVFramePtr_t)workingPtr)->nextPtr = NULL;    
      if(idx == 0){
        *framePtrPtr = prevFramePtr = (EthSVFramePtr_t)workingPtr;
      }else{
        prevFramePtr->nextPtr = workingPtr;
      }
    } 
/* Get a data buffer. */          
    rtnCode = SpacQMngGet(&buffMng, 
      (void **)&(((EthSVFramePtr_t)workingPtr)->buffPtr), SPAC_NOT_SAFE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <ioc80314GetNewBuffers>, <SpacQMngGet> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      if(idx == 0){
        *framePtrPtr =  NULL;
      }else{
        prevFramePtr->nextPtr = NULL; 
      }     
      rtnCode = EthReleaseFrm(workingPtr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <ioc80314GetNewBuffers>, <EthReleaseFrm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
      return rtnCode;
    }else{
      ((EthSVFramePtr_t)workingPtr)->buffLen = buffMng.qMngCellSize;
    }  
    if(idx != 0){
      prevFramePtr = workingPtr;
    }
  }
    
  return rtnCode;
    
}


