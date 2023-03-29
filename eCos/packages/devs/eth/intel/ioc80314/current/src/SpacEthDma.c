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
* FILENAME: 1.	SpacEthDma.c
*
* DESCRIPTION: Contains the code implementations for configuring/testing the 
*              SPac GigE DMA engine.
*
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added test routines. 
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

#endif

/* Global flags. */
volatile BOOL doGigE7_9 = FALSE;
volatile BOOL doGigE6_4and7_8 = FALSE;
volatile BOOL doGigE7_1 = FALSE;
/* Display (print) frame flag. */
extern int gigEDisplayFrm;
volatile BOOL doGigE7_2 = FALSE;
volatile BOOL doGigE7_4 = FALSE;
BOOL doGigE7_6 = FALSE;
BOOL doGigE7_10 = FALSE;

/* Test status table. */
  extern EthTestDone_t ethTestStatus[];

/*=========================================================================
 *  Function    : EthConfigTx
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : txInfoPtr - TX Configuration info pointer.
 * 
 *  Description : Configures the chip's DMA TX engine of a specified GigE 
 *              : port.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_MEM_ALLOC_ERR - Memory allocation error.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthConfigTx(
  UINT32 portId, 
  EthTxInfoPtr_t txInfoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(txInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, TX config info pointer = %p.", txInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_TX_CFG_REG, 
    (UINT32 *)&(txInfoPtr->txCfgInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_TX_THRESH_REG, 
    (UINT32 *)&(txInfoPtr->txThreshInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_THRESH_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_TX_PRI_THRESH_REG, 
    (UINT32 *)&(txInfoPtr->txPriThreshInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_PRI_THRESH_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_TX_Q_PRI_MAP_REG, 
    (UINT32 *)&(txInfoPtr->txPriMapInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_Q_PRI_MAP_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  rtnCode = EthRegAccess(portId, ETH_TX_PAUSE_CNT_REG, 
    (UINT32 *)&(txInfoPtr->txPauseCntInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_PAUSE_CNT_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  rtnCode = EthRegAccess(portId, ETH_TX_GVLAN_TAG_REG, 
    (UINT32 *)&(txInfoPtr->txGVlanInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_GVLAN_TAG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  {
    int idx;
    EthMemPoolPtr_t memPoolPtr = NULL; 
    UINT32 addrLo = 0x00000000; 
    UINT32 ocnPortId = 0xFFFFFFFF; 
    EthQPtrHi_t qPtrHi;
    *((UINT32 *)&qPtrHi) = 0x00000000;
    for(idx = 0; idx < ETH_NUM_TX_Q; idx++){
      if(txInfoPtr->txCtrlInfo.qEnables & (1 << idx)){
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_CFG_REG + (idx * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&(txInfoPtr->txQInfo[idx].txQCfgInfo), REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_CFG_REG + (idx * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
       
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_BUFF_CFG_REG + (idx * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&(txInfoPtr->txQInfo[idx].txQBuffCfgInfo), REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_BUFF_CFG_REG + (idx * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        
        rtnCode = EthSetupDescriptors(portId, idx, ETH_TX, ETH_MEM_ALLOC_ID);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigTx>, <EthSetupDescriptors> for queue %d \
              returns %d.", idx, rtnCode);
          #endif // DEBUG_OVI
/* Do not return. Try to setup other queues. 
          return rtnCode; */ 
        }else{
          memPoolPtr = &(EthDriver.port[portId].mac.tx.txQ[idx].memPool);
/* Virtual-to-OCN address translation. */    
          rtnCode = sysVirtualToOcn(memPoolPtr->descrPoolPtr, (UINT32 *)&qPtrHi, 
            &addrLo, &ocnPortId);
          if(rtnCode != 0){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigTx>, <sysVirtualToOcn> returns %d.", rtnCode);
            #endif // DEBUG_OVI
            return rtnCode;
          } 
/* Write LOW pointer registers before HIGH ones. */      
          rtnCode = EthRegAccess(portId, ETH_TX_Q0_PTR_LO_REG + (idx * ETH_TX_Q_REG_OFFSET), 
            &addrLo, REG_WRITE);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
                returns %d.", REG_WRITE, 
                ETH_TX_Q0_PTR_LO_REG + (idx * ETH_TX_Q_REG_OFFSET), rtnCode);
            #endif // DEBUG_OVI
            return rtnCode;
          } 
/* Set the valid bit only when you put frames in the TX queues.       
          qPtrHi.valid = ETH_BIT_SET;
*/
          rtnCode = EthRegAccess(portId, ETH_TX_Q0_PTR_HI_REG + (idx * ETH_TX_Q_REG_OFFSET), 
            (UINT32 *)&qPtrHi, REG_WRITE);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
                returns %d.", REG_WRITE, 
                ETH_TX_Q0_PTR_HI_REG + (idx * ETH_TX_Q_REG_OFFSET), rtnCode);
            #endif // DEBUG_OVI
            return rtnCode;
          }
          *((UINT32 *)&qPtrHi) = 0x00000000;
        }       
      }
    }
  }
    
  rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&(txInfoPtr->txCtrlInfo), 
    REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigTx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_CTRL_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthConfigRx
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : rxInfoPtr - RX Configuration info pointer.
 * 
 *  Description : Configures the chip's DMA RX engine of a specified GigE 
 *              : port.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_MEM_ALLOC_ERR - Memory allocation error.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthConfigRx(
  UINT32 portId, 
  EthRxInfoPtr_t rxInfoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(rxInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, RX config info pointer = %p.", rxInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_RX_CFG_REG, 
    (UINT32 *)&(rxInfoPtr->rxCfgInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_RX_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_RX_PAUSE_THRESH_REG, 
    (UINT32 *)&(rxInfoPtr->rxPauseThreshInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_RX_PAUSE_THRESH_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  rtnCode = EthRegAccess(portId, ETH_RX_THRESH_REG, 
    (UINT32 *)&(rxInfoPtr->rxThreshInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_RX_THRESH_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_RX_VLAN_TAG_MAP_REG, 
    (UINT32 *)&(rxInfoPtr->rxVlanMapInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_RX_VLAN_TAG_MAP_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  {
    int idx;
    EthSVFramePtr_t framePtr = NULL;     
    EthMemPoolPtr_t memPoolPtr = NULL; 
    UINT32 addrLo = 0x00000000; 
    UINT32 ocnPortId = 0xFFFFFFFF; 
    BOOL descrIntr = FALSE;    
    EthQPtrHi_t qPtrHi;
    *((UINT32 *)&qPtrHi) = 0x00000000;
    for(idx = 0; idx < ETH_NUM_RX_Q; idx++){
      if(rxInfoPtr->rxCtrlInfo.qEnables & (1 << idx)){
        rtnCode = EthRegAccess(portId, 
          ETH_RX_Q0_CFG_REG + (idx * ETH_RX_Q_REG_OFFSET), 
          (UINT32 *)&(rxInfoPtr->rxQInfo[idx].rxQCfgInfo), REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_RX_Q0_CFG_REG + (idx * ETH_RX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_RX_Q0_BUFF_CFG_REG + (idx * ETH_RX_Q_REG_OFFSET), 
          (UINT32 *)&(rxInfoPtr->rxQInfo[idx].rxQBuffCfgInfo), REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_RX_Q0_BUFF_CFG_REG + (idx * ETH_RX_Q_REG_OFFSET), rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthSetupDescriptors(portId, idx, ETH_RX, ETH_MEM_ALLOC_ID);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigRx>, <EthSetupDescriptors> for queue %d \
              returns %d.", idx, rtnCode);
          #endif // DEBUG_OVI
/* Do not return. Try to setup other queues. 
          return rtnCode; */ 
        }else{
/* Feed with new frame buffers. */      
          rtnCode = EthGetNewBuffers(&(EthDriver.port[portId].mac.rx.rxQ[idx].memPool), 
            &framePtr);
          if(framePtr != NULL){
            descrIntr = EthDriver.portInfo[portId]->rxInfo.rxQInfo[idx].descrInfo.descrIntr;            
            rtnCode = EthFeedRxQ(portId, idx, &framePtr, descrIntr);
            if(rtnCode != ETH_OK){
              #ifdef DEBUG_OVI
                db_printf("\nIn <EthConfigRx>, <EthFeedRxQ> for queue %d \
                  returns %d.", idx, rtnCode);
              #endif // DEBUG_OVI
/* Give a chance to service next queue.       
              return rtnCode; */
            }
          }else{
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigRx>, <EthGetNewBuffers> for queue %d \
                returns %d.", idx, rtnCode);
            #endif // DEBUG_OVI
/* Give a chance to service next queue.       
            return rtnCode; */        
          }
          memPoolPtr = &(EthDriver.port[portId].mac.rx.rxQ[idx].memPool);
/* Virtual-to-OCN address translation. */    
          rtnCode = sysVirtualToOcn(memPoolPtr->descrPoolPtr, (UINT32 *)&qPtrHi, 
            &addrLo, &ocnPortId);
          if(rtnCode != 0){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigRx>, <sysVirtualToOcn> returns %d.", rtnCode);
            #endif // DEBUG_OVI
            return rtnCode;
          }           
/* Write LOW pointer registers before HIGH ones. */      
          rtnCode = EthRegAccess(portId, ETH_RX_Q0_PTR_LO_REG + (idx * ETH_RX_Q_REG_OFFSET), 
            &addrLo, REG_WRITE);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
                returns %d.", REG_WRITE, 
                ETH_RX_Q0_PTR_LO_REG + (idx * ETH_RX_Q_REG_OFFSET), rtnCode);
            #endif // DEBUG_OVI
            return rtnCode;
          } 
/* Set the valid bit.  */     
          qPtrHi.valid = ETH_BIT_SET;
          rtnCode = EthRegAccess(portId, ETH_RX_Q0_PTR_HI_REG + (idx * ETH_RX_Q_REG_OFFSET), 
            (UINT32 *)&qPtrHi, REG_WRITE);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
                returns %d.", REG_WRITE, 
                ETH_RX_Q0_PTR_HI_REG + (idx * ETH_RX_Q_REG_OFFSET), rtnCode);
            #endif // DEBUG_OVI
            return rtnCode;
          }
          *((UINT32 *)&qPtrHi) = 0x00000000;
        } 
      }
    }
  }
  
  rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&(rxInfoPtr->rxCtrlInfo),
    REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigRx>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_RX_CTRL_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  return rtnCode;
    
}

