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
* FILENAME: 1.	SpacEthMii.h
*
* DESCRIPTION: Contains the code definitions for configuring/testing the 
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

#ifndef _SPAC_ETH_MII_H_
#define _SPAC_ETH_MII_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

#ifndef _SPAC_ETH_DEFS_H_
#include "SpacEthDefs.h"
#endif

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
);

/*=========================================================================
 *  Function    : GigEConfigLink
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : linkInfoPtr - Link configuration info pointer.
 * 
 *  Description : Configures the link partners through the MII/TBI register  
 *              : interfaces.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *              : ETH_TIMEOUT_ERR - Time-out error.
 *
 *  Note        : This routine is BCM5421-specific. It might need to be  
 *              : modified if using a different PHY device.
 *              : The current implementation assumes the AN is performed
 *              : by the PHY.
 =========================================================================*/
SpacEthRtnCode_t GigEConfigLink(
  UINT32 portId,
  EthLinkInfoPtr_t linkInfoPtr,
  EthPhySumStatusPtr_t phySumStatusPtr
);




#endif  /* _SPAC_ETH_MII_H_ */





