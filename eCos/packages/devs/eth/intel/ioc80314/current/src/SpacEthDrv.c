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
* FILENAME: 1.	SpacEthDrv.c
*
* DESCRIPTION: Contains the GigE Driver object and associate routines..  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Sept 08,02  on     draft. 
*----------------------------------------------------------------------------*/
//#define DEBUG_OVI


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


#ifdef ETH_REDBOOT   
/* Action at RX time. */
  EthDemux_t demuxAction = ETH_DEMUX_LOOPBACK;
/* Display (print) frame flag. */
  int gigEDisplayFrm = ETH_DISPLAY;
  SpacQMngType buffMng; /* Bufer pool. */
  SpacQMngType buffHdrMng; /* Bufer header pool. */
/* Global flags. */

  
/* Global Gige init done flag. */
  extern int flGigeInitDone;
/* Test status table. */
  EthTestDone_t ethTestStatus[ETH_TEST_LAST];

/* local source. */  
  extern char sourceAddr[ETH_NUM_PORTS][ETH_MAC_ADDR_LEN]; 
    
#endif  

/* Display switch. Use this to toggle ON/OFF the loggings. */
int ethDisplay = ETH_DISPLAY;

/* SPac chip base address. */
UINT32 SpacBaseAddr = HAL_IOC80314_CSR_BASE;

/* SPac GigE driver global object. The C run-time system should zero-out it. 
   Please verify anyway!  */
Eth_t EthDriver; 

/* SPac GigE driver's global config info object. */
extern EthPortInfo_t EthDriverInfo[]; 


/*=========================================================================
 *  Function    : EthConfigPort
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : portInfoPtr - Configuration info pointer.
 * 
 *  Description : Configures the chip's specified GigE port.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_MII_IF_STATUS_ERR - MII interface status error.
 *              : ETH_TIMEOUT_ERR - Time-out error.
 *              : ETH_INVAL_CFG_ERR - Invalid configuration error.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthConfigPort(
  UINT32 portId, 
  EthPortInfoPtr_t portInfoPtr
){

//#define DEBUG
//#define DEBUG_OVI  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 resetVal = 0x00000000;
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

#ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, on port id = %d.\n", portId);
#endif // DEBUG_OVI
  
  if(portInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, invalid config info pointer = %p.", 
        portInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
/* Store a reference into the driver's structure. */
  EthDriver.portInfo[portId] = portInfoPtr;
  
/* Bring DMA/FIFO out of reset. */ 
  rtnCode = EthRegAccess(portId, ETH_TX_CFG_REG, &resetVal, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_TX_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  rtnCode = EthRegAccess(portId, ETH_RX_CFG_REG, &resetVal, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_RX_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
/* Bring MAC out of reset. */ 
  rtnCode = EthRegAccess(portId, ETH_MAC_CFG1_REG, &resetVal, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_CFG1_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

/* DMA MAC configuration. */
  rtnCode = EthConfigMac(portId, &portInfoPtr->macInfo);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthConfigMac> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode; 
  } 

/* ETH_PORT_CTRL_REG.SPEED and BPT bits field must be set after auto-negotiation. */
  if(portInfoPtr->macInfo.cfg2Info.ifMode == ETH_MAC_IF_NIBLE){
    portInfoPtr->portCtrlInfo.speed = ETH_BIT_SET;
  }else if(portInfoPtr->macInfo.cfg2Info.ifMode == ETH_MAC_IF_BYTE){
    portInfoPtr->portCtrlInfo.speed = ETH_BIT_CLEARED;
  }
  portInfoPtr->portCtrlInfo.backPressureType = 
    ~portInfoPtr->macInfo.cfg2Info.fullDuplex;
  rtnCode = EthRegAccess(portId, ETH_PORT_CTRL_REG, 
    (UINT32 *)&(portInfoPtr->portCtrlInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_PORT_CTRL_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

/* DMA RX/TX configuration. */
  rtnCode = EthConfigRx(portId, &portInfoPtr->rxInfo);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthConfigRx> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  rtnCode = EthConfigTx(portId, &portInfoPtr->txInfo);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthConfigTx> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

/* Let the queues go! 
  {
  UINT32 qGoVal = 1 << 15;
  
    rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, &qGoVal, REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
    rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, &qGoVal, REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
  }
*/

/* Enable GigE interrupts. */
  rtnCode = EthRegAccess(portId, ETH_PORT_INT_MASK_REG, 
    (UINT32 *)&(portInfoPtr->portIntrMaskInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_PORT_INT_MASK_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  EthDriver.port[portId].portEnabled = TRUE;
  
#ifdef ETH_REDBOOT 


  
#endif    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthResetPort
 *
 *  Arguments   : portId - Port ID (0, 1)
 * 
 *  Description : Resets the chip's specified GigE port.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_TIMEOUT_ERR - Time-out error. 
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthResetPort(
  UINT32 portId
){

#define DEBUG
#define DEBUG_OVI
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 resetVal = 0x00000000;
  UINT32 counter;
  UINT32  phyAddress = 
    (portId == ETH_PORT0_ID ? ETH_PHY0_ADDR : ETH_PHY1_ADDR);  
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
    
/* Reset the STATS. */ 
  resetVal = ETH_PORT_CTRL_RESET_MASK;
  rtnCode = EthRegAccess(portId, ETH_PORT_CTRL_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_PORT_CTRL_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
/* Reset MAC0. */ 
  resetVal = ETH_MAC_RESET_MASK;
  rtnCode = EthRegAccess(ETH_PORT0_ID, ETH_MAC_CFG1_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_MAC_CFG1_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

/* Reset MAC1. */ 
  resetVal = ETH_MAC_RESET_MASK;
  rtnCode = EthRegAccess(ETH_PORT1_ID, ETH_MAC_CFG1_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_MAC_CFG1_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

/* Reset MII because we almost all cycles through MII on Port0 */ 
  resetVal = ETH_MAC_MII_RESET_MASK;
  rtnCode = EthRegAccess(ETH_PORT0_ID, ETH_MAC_MII_CFG_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_MAC_MII_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

	for (counter =0; counter < 0x5000; counter++) 
		;

/* Clear Reset */

/* Reset MAC0. */ 
  resetVal = 0;
  rtnCode = EthRegAccess(ETH_PORT0_ID, ETH_MAC_CFG1_REG, &resetVal, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_MAC_CFG1_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  resetVal = 0x17;
  rtnCode = EthRegAccess(ETH_PORT0_ID, ETH_MAC_MII_CFG_REG, &resetVal, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_MAC_MII_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

/* Reset DMA/FIFO. */   
  resetVal = ETH_RX_RESET_MASK;
  rtnCode = EthRegAccess(portId, ETH_RX_CFG_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_RX_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  resetVal = ETH_TX_RESET_MASK;
  rtnCode = EthRegAccess(portId, ETH_TX_CFG_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_TX_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
/* the first 15 PHY registers are standard. */
  resetVal = ETH_PHY_RESET_MASK;
  rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
    ETH_MAC_MII_WRITE, &resetVal);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <GigEDoLinkMngm> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }   
 
  resetVal = ETH_TBI_PHY_RESET_MASK;
  rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_CTRL1_REG_IDX, 
    ETH_MAC_MII_WRITE, &resetVal);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <GigEDoLinkMngm> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }   
  resetVal = ETH_TBI_RESET_MASK;
  rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_CTRL2_REG_IDX, 
    ETH_MAC_MII_WRITE, &resetVal);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <GigEDoLinkMngm> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  }   
  resetVal = ETH_MAC_MII_RESET_MASK;
  rtnCode = EthRegAccess(portId, ETH_MAC_MII_CFG_REG, &resetVal, REG_OR);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthResetPort>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_OR, ETH_MAC_MII_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  EthDriver.portInfo[portId] = NULL;
  EthDriver.port[portId].portEnabled = FALSE;
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthDrvInit
 *
 *  Arguments   : N/A.
 * 
 *  Description : Configures the chip's GigE block.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_MEM_ALLOC_ERR - Memory allocation error.
 * 
 *
 *  Note        : This is a good place to initialize data structure used 
 *              : by the driver.
 =========================================================================*/
SpacEthRtnCode_t EthDrvInit(
  void
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 buffSize = 
    EthDriverInfo[ETH_CFG_INFO_IDX].txInfo.txQInfo[0].descrInfo.buffSize;
  
  /* If Driver already init'd */
   if (EthDriver.drvEnabled == 1) 
   {
#ifdef DEBUG_OVI
    db_printf ("<EthDrvInit> already initialized driver - returning\n");
#endif
    EthDriver.drvEnabled++;
    return rtnCode;
   }
   else if (EthDriver.drvEnabled == 2) 
   {
    #ifdef DEBUG_OVI
      db_printf("<EthDrvInit>, can't init driver three times\n");
    #endif // DEBUG_OVI
   
    return ETH_PARAM_ERR;
   }

  if(buffSize > ETH_MAX_QMNG_CELL_SIZE){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDrvInit>, invalid configuration buffer size = %d.", buffSize);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  rtnCode = SpacQMngCreate(&buffMng, ETH_QMNG_SIZE, buffSize, 
    (void *)0xdeadbeef, ETH_MEM_ALLOC_ID);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDrvInit>, <SpacQMngCreate> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = SpacQMngCreate(&buffHdrMng, ETH_QMNG_SIZE, sizeof(EthSVFrame_t), 
    (void *)0xdeadbeef, ETH_OCN_SDRAM_ID);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDrvInit>, <SpacQMngCreate> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

    EthDriver.drvEnabled = 1;

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthDrvReset
 *
 *  Arguments   : N/A.
 * 
 *  Description : Resets the chip's GigE block.
 * 
 *  Return      : ETH_OK - Operation succeded.
 * 
 *
 *  Note        : This is a good place to clear data structure used 
 *              : by the driver.
 =========================================================================*/
SpacEthRtnCode_t EthDrvReset(
  void
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;    

  if (EthDriver.drvEnabled == 2) 
  {
    EthDriver.drvEnabled--;
    return rtnCode;
  }
  rtnCode = SpacQMngDelete(&buffMng, ETH_MEM_ALLOC_ID);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDrvReset>, <SpacQMngDelete> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = SpacQMngDelete(&buffHdrMng, ETH_OCN_SDRAM_ID);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthDrvReset>, <SpacQMngDelete> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  EthDriver.drvEnabled--;

  return rtnCode;
    
}

