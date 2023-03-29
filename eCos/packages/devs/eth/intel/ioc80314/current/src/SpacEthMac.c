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
* FILENAME: 1.	SpacEthMac.c
*
* DESCRIPTION: Contains the code implementations for configuring/testing the 
*              SPac GigE MAC block.
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


/* Global flags. */

/* Display (print) frame flag. */
  extern int gigEDisplayFrm;
/* Test status table. */
  extern EthTestDone_t ethTestStatus[];
  

/*=========================================================================
 *  Function    : EthConfigMac
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : macInfoPtr - Configuration info pointer.
 * 
 *  Description : Configures the chip's MAC block of a specified GigE port.
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
SpacEthRtnCode_t EthConfigMac(
  UINT32 portId, 
  EthMacInfoPtr_t macInfoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  BOOL serialMii = FALSE;
  EthPhySumStatus_t phySumStatus;
  UINT32  phyAddress = 
    (portId == ETH_PORT0_ID ? ETH_PHY0_ADDR : ETH_PHY1_ADDR);
 

#ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, on port id = %d.\n", portId);
#endif // DEBUG_OVI
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(macInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, config info pointer = %p.", macInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_MAC_CFG1_REG, 
    (UINT32 *)&(macInfoPtr->cfg1Info), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_CFG1_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
    
  rtnCode = EthRegAccess(portId, ETH_MAC_MII_CFG_REG, 
    (UINT32 *)&(macInfoPtr->miiCfgInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_MII_CFG_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  rtnCode = EthRegAccess(portId, ETH_MAC_MAX_FRAME_REG, 
    (UINT32 *)&(macInfoPtr->maxFrmInfo), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_MAX_FRAME_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  rtnCode = EthRegAccess(portId, ETH_MAC_ADDR_PART1_REG, 
    (UINT32 *)&(macInfoPtr->addrPart1Info), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_ADDR_PART1_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  rtnCode = EthRegAccess(portId, ETH_MAC_ADDR_PART2_REG, 
    (UINT32 *)&(macInfoPtr->addrPart2Info), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_ADDR_PART2_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
    
/* FHY Configurtion. */  
  rtnCode = GigEConfigLink(portId, &(macInfoPtr->linkInfo), &phySumStatus);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <GigEConfigLink> returns %d.", rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 

  if(phySumStatus.speedStatus == ETH_PHY_SPEED_10){
    macInfoPtr->ifCtrlInfo.phyMode = ETH_BIT_SET;
    serialMii = TRUE;
  }else if(phySumStatus.speedStatus == ETH_PHY_SPEED_100){
    macInfoPtr->ifCtrlInfo.phyMode = ETH_BIT_CLEARED;
    serialMii = TRUE;    
  }
    
  if(serialMii == TRUE){
    rtnCode = EthRegAccess(portId, ETH_MAC_IF_CTRL_REG, 
      (UINT32 *)&(macInfoPtr->ifCtrlInfo), REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MAC_IF_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
/* Sleep. 
    SpacNanosleep(ETH_MII_IF_STATUS_WAIT); 
*/
    {
      EthMacIfStatus_t ifStatusInfo;
      int counter = 0;
      do{
/* Read ETH_MAC_IF_STATUS_REG register. */   
/* Check the interface status. */
        rtnCode = EthRegAccess(portId, ETH_MAC_IF_STATUS_REG, 
          (UINT32 *)&ifStatusInfo, REG_READ);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_READ, ETH_MAC_IF_STATUS_REG, rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        }
/*        else{ 
          if(ifStatusInfo.linkFail == ETH_BIT_SET){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
                detects an invalid link condition.", REG_READ, ETH_MAC_IF_STATUS_REG);
            #endif // DEBUG_OVI
            return ETH_MII_IF_STATUS_ERR;
          }
        }
*/        
       counter++;if(counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR;
/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
      }while(ifStatusInfo.linkFail == ETH_BIT_SET);
    }
  }  
  
  if(phySumStatus.speedStatus == ETH_PHY_SPEED_1000){
    macInfoPtr->cfg2Info.ifMode = ETH_MAC_IF_BYTE;
  }else if(serialMii == TRUE){
    macInfoPtr->cfg2Info.ifMode = ETH_MAC_IF_NIBLE;
  }else{
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, invalid link speed configuration detected.");
    #endif // DEBUG_OVI
    return ETH_INVAL_CFG_ERR;
  }        
    
  if(phySumStatus.duplexStatus == ETH_PHY_DUPLEX_HALF){
    macInfoPtr->cfg2Info.fullDuplex = ETH_BIT_CLEARED;
  }else if(phySumStatus.duplexStatus == ETH_PHY_DUPLEX_FULL){
    macInfoPtr->cfg2Info.fullDuplex = ETH_BIT_SET;
  }else{
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, invalid link duplex mode detected.");
    #endif // DEBUG_OVI
    return ETH_INVAL_CFG_ERR;
  }
               
  rtnCode = EthRegAccess(portId, ETH_MAC_CFG2_REG, 
    (UINT32 *)&(macInfoPtr->cfg2Info), REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthConfigMac>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_CFG2_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  return rtnCode;
    
}

