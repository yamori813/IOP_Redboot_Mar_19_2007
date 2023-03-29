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
* FILENAME: 1.	SpacEthMii.c
*
* DESCRIPTION: Contains the code implementations for configuring/testing the 
*              SPac GigE MII interface.
*
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added test routines. 
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

/* Test status table. */
extern EthTestDone_t ethTestStatus[];
  
/* SPac GigE driver global object. */
extern Eth_t EthDriver; 

/* SPac chip base address. */
extern  UINT32 SpacBaseAddr;

extern int net_debug;
  

/*=========================================================================
 *  Function    : GigEDoLinkMngm
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : phyAddress - PHY address (0x1E for PE-TBI).
 *              : regAddress - Register index.
 *              : miiCycle - Cycle (scan/read/wite) type.
 *              : miiValPtr - Input/Output.
 * 
 *  Description : Runs a management (scan/read/write) cycle on the MII/TBI 
 *              : interface.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_TIMEOUT_ERR - Time-out error.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t GigEDoLinkMngm(
  UINT32 portId,
  UINT32 phyAddress,
  UINT32 regAddress,
  EthMacMiiCycle_t miiCycle,
  UINT32 * miiValPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthMacMiiCmd_t miiCmd; 
  EthMacMiiAddr_t miiAdr; 
  EthMacMiiIndic_t miiIndic;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEDoLinkMngm>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
/*  if((phyAddress < ETH_PHY0_ADDR) || 
     (phyAddress > ETH_TBI_PHY_ADDR)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEDoLinkMngm>, invalid PHY address = %d.", phyAddress);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
*/
  
  if((regAddress < ETH_PHY_REG_IDX_LOW) || 
     (regAddress > ETH_PHY_REG_IDX_HIGH)){
    #ifdef DEBUG_OVI
      db_printf ("\nIn <GigEDoLinkMngm>, invalid register address = %d.", regAddress);
	#endif
    return ETH_PARAM_ERR;
  } 
     
  if(miiValPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEDoLinkMngm>, invalid MII value pointer = %p.", 
        miiValPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }      
  
/* Begins the critical section. In a multitasking OS shal be protected by a per-port mutex. */  

/* Wait for a non-busy interface condition. */  
  do{
    rtnCode = EthRegAccess(portId, ETH_MAC_MII_INDIC_REG, 
      (UINT32 *)&miiIndic, REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_MAC_MII_INDIC_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
/*    
    counter++;
    if(counter > ETH_MII_MNGM_WAIT_COUNT){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEDoLinkMngm>, MII has timed out.");
      #endif // DEBUG_OVI
      return ETH_TIMEOUT_ERR;
    }
*/    
  }while(miiIndic.busy == ETH_BIT_SET);
/*
  counter = 0;
*/  
/* Start a cycle. */  
  *((UINT32 *)&miiCmd) = 0x00000000;
  *((UINT32 *)&miiAdr) = 0x00000000;
  miiAdr.phyAddress = phyAddress;    
  miiAdr.regAddress = regAddress;
  
  rtnCode = EthRegAccess(portId, ETH_MAC_MII_ADDR_REG, 
    (UINT32 *)&miiAdr, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_MII_ADDR_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
/* It seems that we have to clear the command register in between MII read operations. */    
  rtnCode = EthRegAccess(portId, ETH_MAC_MII_CMD_REG, 
    (UINT32 *)&miiCmd, REG_WRITE);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_WRITE, ETH_MAC_MII_CMD_REG, rtnCode);
    #endif // DEBUG_OVI
    return rtnCode;
  } 
  
  if(miiCycle == ETH_MAC_MII_WRITE){
    rtnCode = EthRegAccess(portId, ETH_MAC_MII_CTRL_REG, 
      miiValPtr, REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MAC_MII_CTRL_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
  }else{ 
    if(miiCycle == ETH_MAC_MII_SCAN){
      miiCmd.scanCycle = ETH_BIT_SET;
    }else{
      miiCmd.readCycle = ETH_BIT_SET;
    }
    rtnCode = EthRegAccess(portId, ETH_MAC_MII_CMD_REG, 
      (UINT32 *)&miiCmd, REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MAC_MII_CMD_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    } 
  
    if(miiCycle == ETH_MAC_MII_SCAN){
      SpacNanosleep(ETH_MII_SCAN_STATUS_WAIT);
    }else{
      do{
        rtnCode = EthRegAccess(portId, ETH_MAC_MII_INDIC_REG, 
          (UINT32 *)&miiIndic, REG_READ);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_READ, ETH_MAC_MII_INDIC_REG, rtnCode);
          #endif // DEBUG_OVI
          return rtnCode;
        }
/*        
        counter++;
        if(counter > ETH_MII_MNGM_WAIT_COUNT){
          #ifdef DEBUG_OVI
            db_printf("\nIn <GigEDoLinkMngm>, MII has timed out.");
          #endif // DEBUG_OVI
          return ETH_TIMEOUT_ERR;
        }
*/        
      }while(miiIndic.notValid == ETH_BIT_SET);
    }
    
    rtnCode = EthRegAccess(portId, ETH_MAC_MII_STATUS_REG, 
      miiValPtr, REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_MAC_MII_STATUS_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    
    if(miiCycle == ETH_MAC_MII_SCAN){
      *((UINT32 *)&miiCmd) = 0x00000000;
      
    /* We have to clear the command register after MII scan operations. */    
      rtnCode = EthRegAccess(portId, ETH_MAC_MII_CMD_REG, 
        (UINT32 *)&miiCmd, REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEDoLinkMngm>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_MAC_MII_CMD_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      } 
    }    
  }
  
/* Ends the critical section. */  
  
  return rtnCode;
    
}

#ifdef ETH_PHY_BCM5421  
/* Start of BCM5421-specific code */ 
/*=========================================================================
 *  Function    : GigEConfigLink
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : linkInfoPtr - Link configuration info pointer.
 *              : phySumStatusPtr - summary status pointer.
 * 
 *  Description : Configures the link partners through the MII/TBI register  
 *              : interfaces.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_TIMEOUT_ERR - Time-out error.
 *
 *  Note        : The current implementation assumes the AN is performed
 *              : by the PHY. The final configuration for both link partners
 *              : is controlled by the PHY's config info.
 =========================================================================*/
SpacEthRtnCode_t GigEConfigLink(
  UINT32 portId,
  EthLinkInfoPtr_t linkInfoPtr,
  EthPhySumStatusPtr_t phySumStatusPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;  
  unsigned int counter = 0;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEConfigLink>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(linkInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEConfigLink>, link info pointer = %p.", linkInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }      
  
  if(phySumStatusPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEConfigLink>, link summary status pointer = %p.", phySumStatusPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }      

/* Clear summary status.  */   
    phySumStatusPtr->linkStatus = ETH_PHY_UNRESOLVED;
    phySumStatusPtr->speedStatus = ETH_PHY_UNRESOLVED;
    phySumStatusPtr->duplexStatus = ETH_PHY_UNRESOLVED;
    phySumStatusPtr->anStatus = ETH_PHY_UNRESOLVED;  

  {
    EthPhyStatus_t phyStatus; 
    EthPhyCtrl_t phyCtrl;
    EthBcmExtCtrl_t bcmExtCtrlTemp;
    EthBcmSumStatus_t bcmSumStatus;
    UINT32 bcmSpareCtrl;
    UINT32 phyCtrlVal;
    UINT32  phyAddress = 
      (portId == ETH_PORT0_ID ? ETH_PHY0_ADDR : ETH_PHY1_ADDR);

/* Take the PHY out of reset. */  
    phyCtrlVal = ETH_PHY_RESET_MASK;     
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(phyCtrlVal));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEBcmConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
     
    phyCtrl.phyReset = ETH_BIT_SET;
    
/* Wait for the reset process to complete. about 2us. */             
    do{
/* Read ETH_PHY_CTRL_REG returned in ETH_MAC_MII_STATUS_REG register. */   
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
        ETH_MAC_MII_READ, (UINT32 *)&(phyCtrl));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEBcmConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
      counter++;if(counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR;
/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
    }while(phyCtrl.phyReset == ETH_BIT_SET);
	counter = 0;      
      
/* If phy addres is 0, take the phy out of "isolate" mode. */ 
    if(phyAddress == 0){
      phyCtrlVal = 0x00000000;
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
        ETH_MAC_MII_WRITE, &phyCtrlVal);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
    }
      
/* Get the defaults for ETH_PHY_EXT_CTRL_REG. */ 
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_BCM_EXT_CTRL_REG_IDX, 
      ETH_MAC_MII_READ, (UINT32 *)&bcmExtCtrlTemp);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    
    if(bcmExtCtrlTemp.ifMode != linkInfoPtr->bcmExtCtrlInfo.ifMode){
      bcmSpareCtrl = ETH_BCM_SPR_CTRL_REG_VAL;
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_BCM_SPR_CTRL_REG_IDX, 
        ETH_MAC_MII_WRITE, &bcmSpareCtrl);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }   
    }
    linkInfoPtr->tbiCtrl2Info.ifMode = ~linkInfoPtr->bcmExtCtrlInfo.ifMode;
    linkInfoPtr->tbiCtrl1Info.speed1 = linkInfoPtr->phyCtrlInfo.speed1;
    linkInfoPtr->tbiCtrl1Info.speed0 = linkInfoPtr->phyCtrlInfo.speed0;
    linkInfoPtr->tbiCtrl1Info.fullDuplex = linkInfoPtr->phyCtrlInfo.fullDuplex;


/* TBI Configuration. */  
    rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_CTRL2_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->tbiCtrl2Info));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_CTRL1_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->tbiCtrl1Info));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    
/* PHY Configuration. Always use Port0 MII Management register set. */  
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_1000BASE_T_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->phy1000BaseTInfo));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_BCM_EXT_CTRL_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->bcmExtCtrlInfo));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    if(linkInfoPtr->phyCtrlInfo.anEnable == ETH_BIT_SET){
      rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_AN_ADV_REG_IDX, 
        ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->tbiAnAdvInfo));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_AN_ADV_REG_IDX, 
        ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->phyAnAdvInfo));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }      
      linkInfoPtr->phyCtrlInfo.resetAn = ETH_BIT_SET;
    }
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->phyCtrlInfo));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
        
/* Wait for the link to be established.    
    SpacNanosleep(ETH_MII_LINK_WAIT);
*/ 

    do{
/* Read ETH_PHY_STATUS_REG returned in ETH_MAC_MII_STATUS_REG register. */   
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_STATUS_REG_IDX, 
        ETH_MAC_MII_READ, (UINT32 *)&phyStatus);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
      counter++;if (counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR;
/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
    }while(
      ((linkInfoPtr->phyCtrlInfo.anEnable == ETH_BIT_SET) && 
       ((phyStatus.anComplete != ETH_BIT_SET) || (phyStatus.linkStatus != ETH_BIT_SET))
      )
      || 
      ((linkInfoPtr->phyCtrlInfo.anEnable != ETH_BIT_SET) && 
       (phyStatus.linkStatus != ETH_BIT_SET)
      )
    );
    counter = 0;
    do{
/* Read ETH_PHY_SUM_STATUS_REG returned in ETH_MAC_MII_STATUS_REG register. */   
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_BCM_SUM_STATUS_REG_IDX, 
        ETH_MAC_MII_READ, (UINT32 *)(&bcmSumStatus));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
     counter++;if (counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR;      
/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
    }while(
      ((linkInfoPtr->phyCtrlInfo.anEnable == ETH_BIT_SET) && 
       ((bcmSumStatus.anComplete != ETH_BIT_SET) || (bcmSumStatus.linkStatus != ETH_BIT_SET))
      )
      || 
      ((linkInfoPtr->phyCtrlInfo.anEnable != ETH_BIT_SET) && 
       (bcmSumStatus.linkStatus != ETH_BIT_SET)
      )
    );
	counter = 0;
    
/* Return summary status.  */   

    if(bcmSumStatus.linkStatus == ETH_BIT_SET){
      phySumStatusPtr->linkStatus = ETH_PHY_LINK_UP;
    }else{
      phySumStatusPtr->linkStatus = ETH_PHY_LINK_DOWN;
    }
    
    if((bcmSumStatus.reportedStatus == ETH_BCM_10_HALF) ||
     (bcmSumStatus.reportedStatus == ETH_BCM_10_FULL)){
      phySumStatusPtr->speedStatus = ETH_PHY_SPEED_10;
    }else if((bcmSumStatus.reportedStatus == ETH_BCM_100TX_HALF) ||
     (bcmSumStatus.reportedStatus == ETH_BCM_100TX_FULL)){
      phySumStatusPtr->speedStatus = ETH_PHY_SPEED_100;
    }else if((bcmSumStatus.reportedStatus == ETH_BCM_1000T_HALF) ||
     (bcmSumStatus.reportedStatus == ETH_BCM_1000T_FULL)){
      phySumStatusPtr->speedStatus = ETH_PHY_SPEED_1000;
    }else{
      phySumStatusPtr->speedStatus = ETH_PHY_UNRESOLVED;
    }
     
    if((bcmSumStatus.reportedStatus == ETH_BCM_10_HALF) ||
       (bcmSumStatus.reportedStatus == ETH_BCM_100TX_HALF) ||
       (bcmSumStatus.reportedStatus == ETH_BCM_1000T_HALF)){
      phySumStatusPtr->duplexStatus = ETH_PHY_DUPLEX_HALF;
    }else if((bcmSumStatus.reportedStatus == ETH_BCM_10_FULL) ||
       (bcmSumStatus.reportedStatus == ETH_BCM_100TX_FULL) ||
       (bcmSumStatus.reportedStatus == ETH_BCM_1000T_FULL)){
      phySumStatusPtr->duplexStatus = ETH_PHY_DUPLEX_FULL;
    }else{
      phySumStatusPtr->duplexStatus = ETH_PHY_UNRESOLVED;
    }
     
    if(bcmSumStatus.anComplete == ETH_BIT_SET){
      phySumStatusPtr->anStatus = ETH_PHY_AN_COMPLETE;
    }else{
      phySumStatusPtr->anStatus = ETH_PHY_AN_NOT_COMPLETE;
    }
         
  }    
    
  return rtnCode;
    
}

/* End of BCM5421-specific code */ 
#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E  
  /* Start of MARVELL88E-specific code */ 

/*=========================================================================
 *  Function    : GigEConfigLink
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : linkInfoPtr - Link configuration info pointer.
 *              : phySumStatusPtr - summary status pointer.
 * 
 *  Description : Configures the link partners through the MII/TBI register  
 *              : interfaces.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_TIMEOUT_ERR - Time-out error.
 *              : ETH_INVAL_CFG_ERR - Invalid configuration error.  
 *
 *  Note        : This routine is MARVELL88E-specific. 
 *              : The current implementation assumes the AN is performed
 *              : by the PHY. The final configuration for both link partners
 *              : is controlled by the PHY's config info.
 =========================================================================*/
SpacEthRtnCode_t GigEConfigLink(
  UINT32 portId,
  EthLinkInfoPtr_t linkInfoPtr,
  EthPhySumStatusPtr_t phySumStatusPtr
){
  SpacEthRtnCode_t rtnCode = ETH_OK;    
  unsigned int counter = 0;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEMarvConfigLink>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(linkInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEMarvConfigLink>, link info pointer = %p.", linkInfoPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }      
  
  if(phySumStatusPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <GigEMarvConfigLink>, link summary status pointer = %p.", phySumStatusPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }        
  
/* Clear summary status.  */   
    phySumStatusPtr->linkStatus = ETH_PHY_UNRESOLVED;
    phySumStatusPtr->speedStatus = ETH_PHY_UNRESOLVED;
    phySumStatusPtr->duplexStatus = ETH_PHY_UNRESOLVED;
    phySumStatusPtr->anStatus = ETH_PHY_UNRESOLVED;
  
  {
    EthPhyStatus_t phyStatus; 
    EthPhyCtrl_t phyCtrl;
    EthMarvSumStatus_t marvSumStatus;
    UINT32 phyCtrlVal;
    UINT32  phyAddress = 
      (portId == ETH_PORT0_ID ? ETH_PHY0_ADDR : ETH_PHY1_ADDR);
      
    if(linkInfoPtr->marvIfMode == ETH_MARV_SERDES2COOPER){
      phyCtrlVal = ETH_MARV_SERDES; /* __OVI__ make it port specific!!! */
      linkInfoPtr->tbiCtrl2Info.ifMode = ETH_BIT_CLEARED;            
    }else if(linkInfoPtr->marvIfMode == ETH_MARV_TBI2COOPER){
      phyCtrlVal = ETH_MARV_TBI; /* __OVI__ make it port specific!!! 
      linkInfoPtr->tbiCtrl2Info.ifMode = ETH_BIT_CLEARED;  */          
    }else if(linkInfoPtr->marvIfMode == ETH_MARV_GMII2COOPER){
      phyCtrlVal = ETH_MARV_GMII; /* __OVI__ make it port specific!!! */
      linkInfoPtr->tbiCtrl2Info.ifMode = ETH_BIT_SET;      
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, portId %d, invalid interface configuration mode = %d.", 
          portId, linkInfoPtr->marvIfMode);
      #endif // DEBUG_OVI
      return ETH_INVAL_CFG_ERR;
    }      
/* Enable GPIO port, output mode. */ 
    *(REG32_PTR(SpacBaseAddr + ETH_GPIO_CTRL_REG)) = ETH_GPIO_CTRL_REG_VAL;
/* Set the interface mode via the GPIO port.    
   Toggle the MODE[1:0] pins to chose the interface mode (TBI, G/MII, SERDES). */     
    *(REG32_PTR(SpacBaseAddr + ETH_GPIO_DATA_REG)) = phyCtrlVal;
   
/* Take the PHY out of reset. */  
    phyCtrlVal = ETH_PHY_RESET_MASK;     
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(phyCtrlVal));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
     
    phyCtrl.phyReset = ETH_BIT_SET;
    
/* Wait for the reset process to complete. */             
    do{
/* Read ETH_PHY_CTRL_REG returned in ETH_MAC_MII_STATUS_REG register. */   
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
        ETH_MAC_MII_READ, (UINT32 *)&(phyCtrl));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
       counter++;if(counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR;
/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
    }while(phyCtrl.phyReset == ETH_BIT_SET);
	counter = 0;      
/* __OVI__ we don't know so far how to toggle between G/MII and TBI!!! */ 
/*    linkInfoPtr->tbiCtrl2Info.ifMode = ~linkInfoPtr->marvExtCtrlInfo.ifMode; */

    linkInfoPtr->tbiCtrl1Info.speed1 = linkInfoPtr->phyCtrlInfo.speed1;
    linkInfoPtr->tbiCtrl1Info.speed0 = linkInfoPtr->phyCtrlInfo.speed0;
    linkInfoPtr->tbiCtrl1Info.fullDuplex = linkInfoPtr->phyCtrlInfo.fullDuplex;


/* TBI Configuration. */  
    rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_CTRL2_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->tbiCtrl2Info));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_CTRL1_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->tbiCtrl1Info));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    
/* PHY Configuration. Always use Port0 MII Management register set. */  
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_1000BASE_T_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->phy1000BaseTInfo));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
/* Marvell PHY specific. Must set bit 3. */  
    linkInfoPtr->marvExtCtrl1Info.reserved1 = ETH_BIT_SET;
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_MARV_EXT_CTRL1_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->marvExtCtrl1Info));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_MARV_EXT_CTRL2_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->marvExtCtrl2Info));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }   
    if(linkInfoPtr->phyCtrlInfo.anEnable == ETH_BIT_SET){
      rtnCode = GigEDoLinkMngm(portId, ETH_TBI_PHY_ADDR, ETH_TBI_AN_ADV_REG_IDX, 
        ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->tbiAnAdvInfo));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_AN_ADV_REG_IDX, 
        ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->phyAnAdvInfo));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }      
      linkInfoPtr->phyCtrlInfo.resetAn = ETH_BIT_SET;
    }
    rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_CTRL_REG_IDX, 
      ETH_MAC_MII_WRITE, (UINT32 *)&(linkInfoPtr->phyCtrlInfo));
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }

	if (net_debug == TRUE)
		db_printf ("Starting Auto-Negotiate...\n"); 

/* Wait for the link to be established. __OVI__ real time issue with the MII ot MARVELL PHY? */  
    SpacNanosleep((unsigned int)ETH_MII_LINK_WAIT);

    do{
/* Read ETH_PHY_STATUS_REG returned in ETH_MAC_MII_STATUS_REG register. */   
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_PHY_STATUS_REG_IDX, 
        ETH_MAC_MII_READ, (UINT32 *)&phyStatus);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
       counter++;if(counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR;
/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
    }while(
      ((linkInfoPtr->phyCtrlInfo.anEnable == ETH_BIT_SET) && 
       ((phyStatus.anComplete != ETH_BIT_SET) || (phyStatus.linkStatus != ETH_BIT_SET))
      )
      || 
      ((linkInfoPtr->phyCtrlInfo.anEnable != ETH_BIT_SET) && 
       (phyStatus.linkStatus != ETH_BIT_SET)
      )
    );

	CYGACC_CALL_IF_DELAY_US(2*1000*1000);   // Added Sep5th (Happy Birthday, G) - to get around GigE switch issue - cebruns

	counter = 0;
    do{
/* Read ETH_PHY_SUM_STATUS_REG returned in ETH_MAC_MII_STATUS_REG register. */   
      rtnCode = GigEDoLinkMngm(ETH_PORT0_ID, phyAddress, ETH_MARV_SUM_STATUS_REG_IDX, 
        ETH_MAC_MII_READ, (UINT32 *)(&marvSumStatus));
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <GigEMarvConfigLink>, <GigEDoLinkMngm> returns %d.", rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
       counter++;if(counter == MAX_THRESHOLD_COUNT) return ETH_TIMEOUT_ERR; 

/* Here we can insert a counter threshold conditional, to avoid a system lock. */      
    }while(
      ((linkInfoPtr->phyCtrlInfo.anEnable == ETH_BIT_SET) && 
       ((marvSumStatus.resolvedStatus != ETH_BIT_SET) || (marvSumStatus.linkStatus != ETH_BIT_SET) || (marvSumStatus.speedStatus == 0x3))
      ) 
      || 
      ((linkInfoPtr->phyCtrlInfo.anEnable != ETH_BIT_SET) && 
       (marvSumStatus.linkStatus != ETH_BIT_SET)
      )
    );
	counter = 0;

/* Return summary status.  */   

    if(marvSumStatus.linkStatus == ETH_BIT_SET){
      phySumStatusPtr->linkStatus = ETH_PHY_LINK_UP;
    }else{
      phySumStatusPtr->linkStatus = ETH_PHY_LINK_DOWN;
    }
    
    if(marvSumStatus.speedStatus == ETH_MARV_10){
      phySumStatusPtr->speedStatus = ETH_PHY_SPEED_10;
    }else if(marvSumStatus.speedStatus == ETH_MARV_100){
      phySumStatusPtr->speedStatus = ETH_PHY_SPEED_100;
    }else if(marvSumStatus.speedStatus == ETH_MARV_1000){
      phySumStatusPtr->speedStatus = ETH_PHY_SPEED_1000;
    }else{
      phySumStatusPtr->speedStatus = ETH_PHY_UNRESOLVED;
    }
     
    if(marvSumStatus.duplexStatus != ETH_BIT_SET){
      phySumStatusPtr->duplexStatus = ETH_PHY_DUPLEX_HALF;
    }else{
      phySumStatusPtr->duplexStatus = ETH_PHY_DUPLEX_FULL;
    }
     
    if(phyStatus.anComplete == ETH_BIT_SET){
      phySumStatusPtr->anStatus = ETH_PHY_AN_COMPLETE;
    }else{
      phySumStatusPtr->anStatus = ETH_PHY_AN_NOT_COMPLETE;
    }
  if (net_debug == true) {
  db_printf ("=============<GigEConfigLink> Findings============\n");
  db_printf ("LinkStatus:  0x%04x\n", marvSumStatus.linkStatus);
  db_printf ("Crossover :  0x%04x\n", marvSumStatus.mdiCrossoverStatus);
  db_printf ("Resolved  :  0x%04x\n", marvSumStatus.resolvedStatus);
  db_printf ("SpeedStatus: 0x%04x\n", marvSumStatus.speedStatus);
  db_printf ("DuplexStatus:0x%04x\n", marvSumStatus.duplexStatus);
  db_printf ("ANComplete  :0x%04x\n", phyStatus.anComplete);
  db_printf ("==================================================\n");
  }
  }    
  
  return rtnCode;
    
}

  /* End of MARVELL88E-specific code */ 
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
#endif






