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
* FILENAME: 1.	SpacEthArbitration.c
*
* DESCRIPTION: Contains the code implementations for configuring/testing the 
*              arbitration mechanism of the GigE block.
*
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Sept 08,02  on     draft 
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
UINT32 txThreshBackUp[ETH_NUM_PORTS]; 
UINT32 txPriBackUp[ETH_NUM_PORTS]; 
UINT32 rxThreshBackUp[ETH_NUM_PORTS]; 
/* Test status table. */
  extern EthTestDone_t ethTestStatus[];


/*=========================================================================
 *  Function    : EthChangePortPri
 *
 *  Arguments   : port0Pri - Port0 priority.
 *              : port1Pri - Port0 priority. 
 * 
 *  Description : Change GigE's ports priorities.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthChangePortPri(
  EthBitField_t port0Pri, 
  EthBitField_t port1Pri
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
    
/* Change port priorities. */    
  {
    EthPortCtrl_t portCtrl;
    int portIdx;    
/* Change port priorities. */    
    for(portIdx = ETH_PORT0_ID; portIdx < ETH_NUM_PORTS; portIdx++){
      rtnCode = EthRegAccess(portIdx, ETH_PORT_CTRL_REG, (UINT32 *)&portCtrl, 
        REG_READ);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthChangePortPri>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_READ, ETH_PORT_CTRL_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
      portCtrl.portPriority = (portIdx == ETH_PORT0_ID ? port0Pri : port1Pri);
      rtnCode = EthRegAccess(portIdx, ETH_PORT_CTRL_REG, (UINT32 *)&portCtrl, 
        REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthChangePortPri>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_PORT_CTRL_REG, rtnCode);
        #endif // DEBUG_OVI
        return rtnCode;
      }
    }  
  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthChangeDmaPri
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : dmaTxPri - DMA TX priority.
 *              : dmaRxPri - DMA RX priority. 
 * 
 *  Description : Change port's TX/RX priorities.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthChangeDmaPri(
  UINT32 portId, 
  EthBitField_t dmaTxPri, 
  EthBitField_t dmaRxPri
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthChangeDmaPri>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   
    
/* Change DMA TX/RX priorities. */    
  {
    EthRxCfg_t rxCfg;
    EthTxCfg_t txCfg;
    
    rtnCode = EthRegAccess(portId, ETH_TX_CFG_REG, (UINT32 *)&txCfg, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthChangeDmaPri>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_CFG_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    txCfg.changePriority = dmaTxPri;
    rtnCode = EthRegAccess(portId, ETH_TX_CFG_REG, (UINT32 *)&txCfg, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthChangeDmaPri>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_CFG_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    rtnCode = EthRegAccess(portId, ETH_RX_CFG_REG, (UINT32 *)&rxCfg, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthChangeDmaPri>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_CFG_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
    rxCfg.changePriority = dmaRxPri;
    rtnCode = EthRegAccess(portId, ETH_RX_CFG_REG, (UINT32 *)&rxCfg, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthChangeDmaPri>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_CFG_REG, rtnCode);
      #endif // DEBUG_OVI
      return rtnCode;
    }
  }
  
  return rtnCode;
    
}

