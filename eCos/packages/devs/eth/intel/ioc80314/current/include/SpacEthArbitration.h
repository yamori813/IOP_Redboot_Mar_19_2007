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
* FILENAME: 1.	SpacEthArbitration.h
*
* DESCRIPTION: Contains the code definitions for configuring/testing the 
*              arbitration mechanism of the GigE block.
*
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Sept 08,02  on     draft 
*----------------------------------------------------------------------------*/

#ifndef _SPAC_ETH_ARBITR_H_
#define _SPAC_ETH_ARBITR_H_

#ifndef __tuntypes
#include "tuntypes.h"
#endif

#ifndef _SPAC_ETH_DEFS_H_
#include "SpacEthDefs.h"
#endif

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
 *  Note        : Use of "PRINTF" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthChangePortPri(
  EthBitField_t port0Pri, 
  EthBitField_t port1Pri
);

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
 *  Note        : Use of "PRINTF" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthChangeDmaPri(
  UINT32 portId, 
  EthBitField_t dmaTxPri, 
  EthBitField_t dmaRxPri
);

/*=========================================================================
 *  Function    : EthDoGigE8_3and6_3
 *
 *  Arguments   : portId - port ID (0, 1)
 * 
 *  Description : Handler for the GigE8_3and6_3 test.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : The strategy used is to set threshold values close and small 
 *              : enough to guaranty an underrun condition.
 =========================================================================*/
SpacEthRtnCode_t EthDoGigE8_3and6_3(
  UINT32 portId
);

/*=========================================================================
 *  Function    : gige8_3and6_3 - Command Service Routine.
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3), this argument
 *                          may be ignored if the corresponding command does
 *                          not have any options.
 *                param_string - user specified test parameters, ignored.
 *
 *  Description : Executes the test command.
 * 
 *  Return      : always 0
 *
 ========================================================================*/
int gige8_3and6_3(int option, char* param_string);

/*=========================================================================
 *  Function    : GigE8_3and6_3
 *
 *  Arguments   : N/A.
 * 
 *  Description : Verifies the TX underrun condition.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address. 
 *
 *  Note        : Choosing all thresholds equal, should trigger an underrun.
 =========================================================================*/
SpacEthRtnCode_t GigE8_3and6_3(
  void 
);

/*=========================================================================
 *  Function    : EthDoGigE8_4and6_3
 *
 *  Arguments   : portId - port ID (0, 1)
 * 
 *  Description : Handler for the GigE8_4and6_3 test.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : The strategy used is to set threshold values small 
 *              : enough to guaranty an overrun condition.
 =========================================================================*/
SpacEthRtnCode_t EthDoGigE8_4and6_3(
  UINT32 portId
);

/*=========================================================================
 *  Function    : GigE8_4and6_3
 *
 *  Arguments   : N/A.
 * 
 *  Description : Verifies the RX overrun condition.
 * 
 *  Return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address. 
 *
 *  Note        : Choosing small thresholds, should trigger an overrun.
 =========================================================================*/
SpacEthRtnCode_t GigE8_4and6_3(
  void 
);

/*=========================================================================
 *  Function    : gige8_4and6_3 - Command Service Routine.
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3), this argument
 *                          may be ignored if the corresponding command does
 *                          not have any options.
 *                param_string - user specified test parameters, ignored.
 *
 *  Description : Executes the test command.
 * 
 *  Return      : always 0
 *
 ========================================================================*/
int gige8_4and6_3(int option, char* param_string);

/*=========================================================================
 *  Function    : GigE8_1
 *
 *  Arguments   : port0Pri - Port0 priority.
 *              : port1Pri - Port1 priority. 
 * 
 *  Description : Change GigE's ports priorities.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Start and stop data flow simultaneously. Read RX/TX counters
 *              : in order to determine the OCN port's time slice use.
 =========================================================================*/
SpacEthRtnCode_t GigE8_1(
  EthBitField_t port0Pri, 
  EthBitField_t port1Pri
);

/*=========================================================================
 *  Function    : gige8_1 - Command Service Routine.
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3). To be ignored.
 *                param_string - user specified test parameters.
 *
 *  Description : Executes the test command.
 * 
 *  Return      : always 0
 *
 ========================================================================*/
int gige8_1(int option, char* param_string);

/*=========================================================================
 *  Function    : GigE8_2
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
 *  Note        : Use of "PRINTF" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t GigE8_2(
  UINT32 portId, 
  EthBitField_t dmaTxPri, 
  EthBitField_t dmaRxPri
);

/*=========================================================================
 *  Function    : gige8_2 - Command Service Routine.
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3). To be ignored.
 *                param_string - user specified test parameters.
 *
 *  Description : Executes the test command.
 * 
 *  Return      : always 0
 *
 ========================================================================*/
int gige8_2(int option, char* param_string);



#endif  /* _SPAC_ETH_ARBITR_H_ */





