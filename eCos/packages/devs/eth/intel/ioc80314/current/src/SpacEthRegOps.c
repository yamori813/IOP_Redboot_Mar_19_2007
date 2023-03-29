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
* FILENAME: 1.	SpacEthRegOps.c
*
* DESCRIPTION: Contains the code implementations for configuring/testing the 
*              SPac GigE MAC block.
*
*              Indirect register access uses specific routines.
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added XOR operation. 
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


/* SPac chip base address. */
extern  UINT32 SpacBaseAddr;

/*=========================================================================
 *  Function    : EthRegAccess
 *
 *  Arguments   : portId - port ID (0, 1)
 *              : regName - register offset from the Port's base address.
 *              : regValue - value to be read/written from/to.
 *              : operation - operation to be completed.
 * 
 *  Description : Accesses the chip GigE block's registers.
 * 
 *  Return      : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthRegAccess(
  UINT32 portId, 
  UINT32 regName, 
  UINT32 * regValue,
  SpacEthRegOps_t operation
){

  
  SpacEthRtnCode_t rtnCode = ETH_OK;
    
  if(SpacBaseAddr == 0x00000000){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRegAccess>, invalid chip base address = %08X.", 
             SpacBaseAddr);
    #endif // DEBUG_OVI
    return ETH_BASE_ADDR_ERR;
  } 
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRegAccess>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((regName < ETH_BASE_ADDR_LO) || (regName > ETH_BASE_ADDR_HI)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRegAccess>, invalid register address = %08X.", 
             regName);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  {
    VUINT32 * regPtr = REG32_PTR(SpacBaseAddr + regName + (portId * ETH_PORT_OFFSET));

    if(operation == REG_WRITE){
      *regPtr = *regValue;
    }else if(operation == REG_READ){
      *regValue = *regPtr;
    }else if (operation == REG_AND){
      *regPtr &= *regValue;
    }else if (operation == REG_OR){
      *regPtr |= *regValue;
    }else if (operation == REG_XOR){
      *regPtr ^= *regValue;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRegAccess>, invalid operation id = %d.", 
               operation);
      #endif // DEBUG_OVI    
      return ETH_PARAM_ERR;
    }  
  }
  
  return rtnCode;
    
}
 
/*=========================================================================
 *  Function    : EthIndirectRegAccess
 *
 *  Arguments   : portId - port ID (0, 1)
 *              : addrRegName - register offset from the Port's base address.
 *              : dataRegName - register offset from the Port's base address.
 *              : regValue1 - value containing the index to be written. Input.
 *              : regValue2 - value to be read/written from/to. Input/output.
 *              : operation - operation to be completed.
 * 
 *  Description : Indirect accesses the chip GigE block's registers.
 * 
 *  Return      : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : See the Indirect Register Access mechanism description.
 *              : This routine is used to initiate the first access. 
 *              : Subsequent accesses use EthRegAccess routine.
 *              : Supported operations are REG_WRITE and REG_READ.
 =========================================================================*/
SpacEthRtnCode_t EthIndirectRegAccess(
  UINT32 portId, 
  UINT32 addrRegName, 
  UINT32 dataRegName, 
  UINT32 * regValue1,
  UINT32 * regValue2,
  SpacEthRegOps_t operation
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
    
  if(SpacBaseAddr == 0x00000000){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthIndirectRegAccess>, \
             invalid chip base address = %08X.", SpacBaseAddr);
    #endif // DEBUG_OVI
    return ETH_BASE_ADDR_ERR;
  } 
    
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthIndirectRegAccess>, invalid port id = %d.", 
             portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((addrRegName < ETH_BASE_ADDR_LO) || (addrRegName > ETH_BASE_ADDR_HI)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthIndirectRegAccess>, \
             invalid address register address = %08X.", addrRegName);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((dataRegName < ETH_BASE_ADDR_LO) || (dataRegName > ETH_BASE_ADDR_HI)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthIndirectRegAccess>, \
             invalid data register address = %08X.", dataRegName);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  {
    VUINT32 * addrRegPtr = REG32_PTR(SpacBaseAddr + addrRegName + 
      (portId * ETH_PORT_OFFSET));
    VUINT32 * dataRegPtr = REG32_PTR(SpacBaseAddr + dataRegName + 
      (portId * ETH_PORT_OFFSET));
      
    if(operation == REG_WRITE){
      *addrRegPtr = *regValue1;
      *dataRegPtr = *regValue2;    
    }else if(operation == REG_READ){
      *addrRegPtr = *regValue1;
      *regValue2 = *dataRegPtr;    
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthIndirectRegAccess>, invalid operation id = %d.", 
               operation);
      #endif // DEBUG_OVI    
      return ETH_PARAM_ERR;
    }  
  }
  
  return rtnCode;
    
}
 





