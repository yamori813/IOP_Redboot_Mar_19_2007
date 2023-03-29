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
* FILENAME: 1.	SpacEthBuffDescript.h
*
* DESCRIPTION: Contains declarations of routines acting on TX/RX buffer
*            : descriptors.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added test routines. 
*----------------------------------------------------------------------------*/

#ifndef _SPAC_ETH_BUFF_DESC_H_
#define _SPAC_ETH_BUFF_DESC_H_

#ifndef _SPAC_ETH_DEFS_H_
#include "SpacEthDefs.h"
#endif

/*=========================================================================
 *  Function    : EthSetupDescriptors
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : direction - input, TX/RX.
 *              : memSpaceId - input, PCI/SDRAM/SRAM. 
 * 
 *  Description : Sets up a ring descriptor scheme.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_MEM_ALLOC_ERR - Memory allocation error.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : The "putInIdx/takeOutIdx" points to the location in the 
 *              : descriptor ring where the next "PUT"/"GET" operation
 *              : shall take place at TX/RX time. It is maintained by
 *              : the  software.
 *              : This routine would be called before enabling a queue.
 =========================================================================*/
SpacEthRtnCode_t EthSetupDescriptors(
  UINT32 portId, 
  UINT32 qId,
  EthDirFlow_t direction,
  UINT32 memSpaceId
);

/*=========================================================================
 *  Function    : EthPutFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : framePtr - input, frame to transmit.
 *              : txFrmCfgInfoPtr - per-frame config info.
 * 
 *  Description : Puts a frame in a specified TX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_Q_FULL_ERR - Queue(TX ring) full condition.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : It is assumed that prior to enqueue a frame, a check for the
 *              : ring available space("freeEntries" value), has been done .
 =========================================================================*/
SpacEthRtnCode_t EthPutFrame(
  UINT32 portId, 
  UINT32 qId,
  EthSVFramePtr_t framePtr,
  EthTxFrmCfgInfoPtr_t txFrmCfgInfoPtr
);

/*=========================================================================
 *  Function    : EthFreeTxFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3). 
 *              : memPoolPtr - input, used to retrieve the "TAKE-OUT" pointer.
 * 
 *  Description : Releases transmitted frames from a specified TX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_FREE_BUFF_ERR - Buffer release error.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This would be a good place to gather statistics about 
 *              : transmitted frames.
 =========================================================================*/
SpacEthRtnCode_t EthFreeTxFrame(
  UINT32 portId, 
  UINT32 qId,  
  EthMemPoolPtr_t memPoolPtr
);

/*=========================================================================
 *  Function    : EthFeedRxQ
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : framePtrPtr - input, linked-list of buffers to receive into. 
 *              : descrIntr - input, allows per-descriptor interrupts.
 * 
 *  Description : Feeds buffers to a specified RX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_Q_FULL_ERR - Queue(RX ring) full condition. 
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This routine would would be called immediately after  
 *              : EthSetupDescriptors on a RX queue, and before enabling
 *              : the queue.
 *              : It is assumed that prior to feed a frame, a check for the
 *              : ring available space("freeEntries" value), has been done .
 =========================================================================*/
SpacEthRtnCode_t EthFeedRxQ(
  UINT32 portId, 
  UINT32 qId,
  EthSVFramePtr_t * framePtrPtr,  
  BOOL descrIntr
);
/*=========================================================================
 *  Function    : EthGetFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 * 
 *  Description : Gets received frames from a specified RX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_INTEGRITY_ERR - Queue integrity error. 
 *
 *  Note        : This would be a good place to gather statistics about 
 *              : received frames. 
 *              : We chose to allow frame processing despite an error at
 *              : release invalid frame and demux time.
 =========================================================================*/
SpacEthRtnCode_t EthGetFrame(
  UINT32 portId, 
  UINT32 qId
);

/*=========================================================================
 *  Function    : EthIncrRx
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of RX valid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrRx(void);
 
/*=========================================================================
 *  Function    : EthIncrRxErr
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of RX invalid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrRxErr(void);
 
/*=========================================================================
 *  Function    : EthReleaseFrm
 *
 *  Arguments   : frmPtr - pointer to frame to be released.
 * 
 *  Description : Releases frames back to the memory pool.
 * 
 *  return      : ETH_OK - Operation succeded.
 *              : ETH_PARAM_ERR - Invalid parameter. 
 *
 *  Note        : N/A. 
 =========================================================================*/
SpacEthRtnCode_t EthReleaseFrm(
EthSVFramePtr_t frmPtr
);
 
/*=========================================================================
 *  Function    : EthGetRxFrameStatus
 *
 *  Arguments   : rxQPtr - input/output. Specifies a particular RX queue.
 * 
 *  Description : Gets status info for a received frame.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This routine only updates the invalid frame flag, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthGetRxFrameStatus(EthRxQPtr_t rxQPtr);
 
/*=========================================================================
 *  Function    : EthGetTxFrameStatus
 *
 *  Arguments   : txFrmStatusCfgPtr - TX status info.
 * 
 *  Description : Gets status info for a transmitted frame.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This routine only updates the invalid frame flag, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthGetTxFrameStatus(EthTxFrmStatusCfgPtr_t txFrmStatusCfgPtr);

/*=========================================================================
 *  Function    : EthIncrTx
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of TX valid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrTx(void);

/*=========================================================================
 *  Function    : EthIncrTxErr
 *
 *  Arguments   : N/A
 * 
 *  Description : Keeps track of TX invalid frame count.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine is implemented as a stub, for now. 
 =========================================================================*/
SpacEthRtnCode_t EthIncrTxErr(void);

/*=========================================================================
 *  Function    : EthRxDemux
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 3).
 *              : frmPtr - pointer to frame to be passed out.
 * 
 *  Description : Dispatches a RX frame to the upper layer.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_FREE_BUFF_ERR - Buffer release error.
 *              : ETH_Q_FULL_ERR - Queue(TX ring) full condition.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthRxDemux(
  UINT32 portId, 
  UINT32 qId,
  EthSVFramePtr_t frmPtr
);


/*=========================================================================
 *  Function    : EthRedBootFreeTx
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : 
 * 
 *  Description : Releases the transmitted frames from the  TX queues.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This implementation uses a simplified priority scheme.
 *              : It assumes the following priority Q3 > Q2 > Q1 > Q0. 
 *              : This is intended to be used on a RedBoot platform, in an 
 *              : interrupt context.
 =========================================================================*/
SpacEthRtnCode_t EthRedBootFreeTx(
  UINT32 portId 
);

/*=========================================================================
 *  Function    : EthGetNewBuffers
 *
 *  Arguments   : memPoolPtr - input, used to retrieve handles to the memory 
 *              :              managers.
 *              : framePtr - output, linked-list of buffers to receive into. 
 * 
 *  Description : Gets new buffers to feed a specified queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_EMPTY_ERR - queue empty condition.
 *              : ETH_NOT_INIT_ERR - queue not initialized.
 *              : ETH_SYNC_OBJ_ERR - semaphore operation error. 
 *
 *  Note        : This routine would would be called immediately after  
 *              : EthSetupDescriptors on a RX queue, and before enabling
 *              : the queue.
 *              : Prior to feed a frame, a check for the available
 *              : queue space("freeEntries" value), shall be done .
 =========================================================================*/
SpacEthRtnCode_t EthGetNewBuffers(
  EthMemPoolPtr_t memPoolPtr,
  EthSVFramePtr_t * framePtrPtr 
);

/*=========================================================================
 *  Function    : EthQRx
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : qId - Queue ID (0, 4).
 * 
 *  Description : Processes a RX queue.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_FULL_ERR - Queue(RX ring) full condition.  
 *              : ETH_Q_EMPTY_ERR - queue empty condition.
 *              : ETH_NOT_INIT_ERR - queue not initialized.
 *              : ETH_SYNC_OBJ_ERR - semaphore operation error. 
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthQRx(
  UINT32 portId,
  UINT32 qId
);

/*=========================================================================
 *  Function    : EthGetNumFrmBuffers
 *
 *  Arguments   : frmPtr - pointer to frame to be passed out.
 *              : frmCountPtr - pointer to counter value to be returned.
 *              : frmLenPtr - pointer to length value to be returned. 
 * 
 *  Description : Counts the number of comprising frame bufffers.
 * 
 *  return      : ETH_OK - Operation succeded.
 *              : ETH_PARAM_ERR - Invalid parameter.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthGetNumFrmBuffers(
  EthSVFramePtr_t frmPtr,
  UINT32 * frmCountPtr,
  UINT32 * frmLenPtr
);

/*=========================================================================
 *  Function    : EthRedBootTx
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : frmPtr - Pointer to frame to be passed out. 
 *              : txFrmCfgInfoPtr - Pointer to frame config info.
 * 
 *  Description : Puts frames into the TX queues.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : This implementation uses a simplified priority scheme.
 *              : It assumes the following priority Q3 > Q2 > Q1 > Q0. 
 *              : This is intended to be used on a RedBoot platform, in an 
 *              : interrupt context.
 =========================================================================*/
SpacEthRtnCode_t EthRedBootTx(
  UINT32 portId,
  EthSVFramePtr_t frmPtr,
  EthTxFrmCfgInfoPtr_t txFrmCfgInfoPtr
);

/*=========================================================================
 *  Function    : EthDumpFrame
 *
 *  Arguments   : frmPtr - buffer pointer.
 *              : len - number of bytes to be dumped.
 * 
 *  Description : Dumps frame bufffers.
 * 
 *  return      : ETH_OK - Operation succeded.
 *              : ETH_PARAM_ERR - Invalid parameter.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthDumpFrame(
  UINT8 * buffPtr, 
  UINT16 len
);

/*=========================================================================
 *  Function    : EthProcessRxFrame
 *
 *  Arguments   : portId - Port ID (0, 1).
 *              : frmPtr - Pointer to frame to be processed. 
 * 
 *  Description : Removes the CRC and swaps the MAC addresses of a RX frame.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : N/A.
 =========================================================================*/
SpacEthRtnCode_t EthProcessRxFrame(
  UINT32 portId,
  EthSVFramePtr_t frmPtr
);


/*=========================================================================
 *  Function    : gigeDbg
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3). To be ignored.
 *                param_string - user specified test parameters.
 * 
 *  Description : Controls the verbosity of the gige driver.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : N/A.
 =========================================================================*/
int gigeDbg(int option, char* param_string);

/*=========================================================================
 *  Function    : gigeDataFlow
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3). To be ignored.
 *                param_string - user specified test parameters.
 * 
 *  Description : Controls the frame routing of the gige driver.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : N/A.
 =========================================================================*/
int gigeDataFlow(int option, char* param_string);

/*=========================================================================
 *  Function    : gigeDataFlow
 *
 *  Arguments   : testStatus - test status information.
 * 
 *  Description : Return an explanatory string message.
 * 
 *  return      : character string.
 *
 *  Note        : N/A.
 =========================================================================*/
char * getTestStatus(EthTestStatus_t testStatus);

/*=========================================================================
 *  Function    : gigeTestStatus
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3). To be ignored.
 *                param_string - user specified test parameters.
 * 
 *  Description : Displays test status info of the gige driver.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : N/A.
 =========================================================================*/
int gigeTestStatus(int option, char* param_string);

/*=========================================================================
 *  Function    : gigeGo
 *
 *  Arguments   : option - command option (/h, /1, /2 and /3). To be ignored.
 *                param_string -  To be ignored.
 * 
 *  Description : Allows for gige interrupts to be received by the xScale 
 *              : processor.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : This routine must be called before interrupt-dependent 
 *              : processing could be done by the gige driver. It bypasses 
 *              : interrupt blocking by the RedBoot implementation of the 
 *              : user input routines.
 *
 =========================================================================*/
int gigeGo(int option, char* param_string);



#endif  /* _SPAC_ETH_BUFF_DESC_H_ */


