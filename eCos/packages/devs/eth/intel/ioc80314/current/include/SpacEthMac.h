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
* FILENAME: 1.	SpacEthMac.h
*
* DESCRIPTION: Contains the code definitions for configuring/testing the 
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

#ifndef _SPAC_ETH_MAC_H_
#define _SPAC_ETH_MAC_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

#ifndef _SPAC_ETH_DEFS_H_
#include "SpacEthDefs.h"
#endif

/* Interface(to which MAC is connected) mode type. */
typedef enum {
  ETH_MAC_IF_SPEED_10 = 1, /* 10Mb/s. */     
  ETH_MAC_IF_SPEED_100 = 2, /* 100Mb/s. */    
  ETH_MAC_IF_SPEED_1000 = 3, /* 1000Mb/s. */     
} EthMacIfSpeed_t;

/* Duplex mode type. */
typedef enum {
  ETH_MAC_DPLX_HALF = 1, /* Half-duplex. */     
  ETH_MAC_DPLX_FULL = 2, /* Full-duplex. */    
} EthMacDuplex_t;

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
);



#endif  /* _SPAC_ETH_MAC_H_ */





