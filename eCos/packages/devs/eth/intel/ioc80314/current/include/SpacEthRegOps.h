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
* FILENAME: 1.	SpacRegisterOps.h
*
* DESCRIPTION: Contains the declarations for the SPac register operations
*              (READ, WRITE, READ-MODIFY-WRITE).
*
*              Indirect register access uses specific routines.
**
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added XOR operation. 
*----------------------------------------------------------------------------*/

#ifndef _SPAC_REG_OPS_H_
#define _SPAC_REG_OPS_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

#ifndef _SPAC_ETH_DEFS_H_
#include "SpacEthDefs.h"
#endif

/* Operation codes. */
typedef enum {
  REG_WRITE = 0,   
  REG_READ = 1,   
  REG_AND = 2,   
  REG_OR = 3,   
  REG_XOR = 4,   
} SpacEthRegOps_t;


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
);
 
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
);
 





#endif  /* _SPAC_REG_OPS_H_ */





