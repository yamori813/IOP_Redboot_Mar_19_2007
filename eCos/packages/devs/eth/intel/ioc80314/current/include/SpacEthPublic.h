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
* FILENAME: 1.	SpacEthPublic.h
*
* DESCRIPTION: Contains public defines for SPac's GigE block.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Sept 08,02  on     draft 
*----------------------------------------------------------------------------*/

#ifndef _SPAC_ETH_PUBLIC_H_
#define _SPAC_ETH_PUBLIC_H_
    
/* Display switch. Edit this to toggle ON/OFF the loggings. */
extern int ethDisplay;
    
/* Base offset for return codes. Edit this to avoid collisions with the OS 
 *  return codes. 
 */
#define ETH_BASE_RTN_CODE (0)

/* Return codes. */
typedef enum {
  ETH_OK = 0, /* Operation succeded. */   
  ETH_ERR = -1, /* Operation failed. */   
  ETH_PARAM_ERR = ETH_BASE_RTN_CODE -2, /* Invalid parameter. */    
  ETH_BASE_ADDR_ERR = ETH_BASE_RTN_CODE -3, /* Invalid chip base address. */    
  ETH_MEM_ALLOC_ERR = ETH_BASE_RTN_CODE -4, /* Memory allocation error. */ 
  ETH_MEM_ALIGN_ERR = ETH_BASE_RTN_CODE -5, /* Memory alignment error. */ 
  ETH_GET_BUFF_ERR = ETH_BASE_RTN_CODE -6, /* Out of buffers condition. */ 
  ETH_Q_FULL_ERR = ETH_BASE_RTN_CODE -7, /* Queue full condition. */ 
  ETH_Q_EMPTY_ERR = ETH_BASE_RTN_CODE -8, /* Queue empty condition. */ 
  ETH_SEMCREATE_ERR = ETH_BASE_RTN_CODE -9, /* Semaphore creation error. */
  ETH_SEMDELETE_ERR = ETH_BASE_RTN_CODE -10, /* Semaphore deletion error. */
  ETH_NOT_INIT_ERR = ETH_BASE_RTN_CODE -11, /* Attempt to operate on a non 
                                          initialized object. */
  ETH_ALREADY_INIT_ERR = ETH_BASE_RTN_CODE -12, /* Attempt to operate on an 
                                          already object. */
  ETH_SYNC_OBJ_ERR = ETH_BASE_RTN_CODE -13, /* Semaphore operation error. */
  ETH_Q_INTEGRITY_ERR = ETH_BASE_RTN_CODE -14, /* Queue integrity error. */
  ETH_FREE_BUFF_ERR = ETH_BASE_RTN_CODE -15, /* Buffer release error. */ 
  ETH_RX_DEMUX_ERR = ETH_BASE_RTN_CODE -16, /* RX demux error. */ 
  ETH_MII_IF_STATUS_ERR = ETH_BASE_RTN_CODE -17, /* MII interface status 
                                                    error. */ 
  ETH_TIMEOUT_ERR = ETH_BASE_RTN_CODE -18, /* Time-out error. */ 
  ETH_INVAL_CFG_ERR = ETH_BASE_RTN_CODE -19, /* Invalid configuration error. */ 


} SpacEthRtnCode_t;





#endif  /* _SPAC_ETH_PUBLIC_H_ */








