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
* FILENAME: 1.	SpacEthDma.h
*
* DESCRIPTION: Contains the code definitions for configuring/testing the 
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

#ifndef _SPAC_ETH_DMA_H_
#define _SPAC_ETH_DMA_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

#ifndef _SPAC_ETH_DEFS_H_
#include "SpacEthDefs.h"
#endif

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
);

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
);




#endif  /* _SPAC_ETH_DMA_H_ */





