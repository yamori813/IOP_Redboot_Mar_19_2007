/************************************************************************;
;*                                                                      
;*      Filename: sata_lib.h  								 
;*                                                                      
;*      Intel(r) Restricted Use                                      
;*                                                                      
;*      Intel Sample Serial ATA Code                          
;*                                                                      
;*      SCD Advanced Engineering                               
;*                                                                      
;*	Copyright:    (C) 2003-2004 Intel Corporation.          
;*                                                                      
;*      Version 1.00                                                    
;*                                                                      
;*      This program has been developed by Intel Corporation.           
;*      Licensee has Intel's permission to incorporate this source code 
;*      into their product, royalty free.  This source code may NOT be  
;*      redistributed to anyone without Intel's written permission.     
;*                                                                      
;*      Intel specifically disclaims all warranties, express or         
;*      implied, and all liability, including consequential and other   
;*      indirect damages, for the use of this code, including liability 
;*      for infringement of any proprietary rights, and including the   
;*      warranties of merchantability and fitness for a particular      
;*      purpose.  Intel does not assume any responsibility for any      
;*      errors which may appear in this code nor any responsibility to  
;*      update it.                                                      
;************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*   DESCRIPTION                                                              */
/*                                                                            */
/*       This file contains all the definations and prototypes common to the  */
/*       SATA library. All the functions and global variables listed here     */
/*       are for internal use of the library and should not be used by the    */
/*       driver modules (OSDMs). The shared API exposed to the OSDMs is       */
/*       defined in other header files.                                       */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_LIB_HEADER
#define SATA_LIB_HEADER

/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver */
#include <artisea.h>

#ifdef WINNT
#define INLINE
#else
#define INLINE inline
#endif

/******************************************************************************/
/*                                                                            */
/*                C O N S T A N T   D E F I N I T I O N S                     */
/*                                                                            */
/******************************************************************************/

/* The Reference driver will support only 1 HBA with a single Artisea  */
/* controller on it.  It will NOT support RSM or Fan-out.              */
#define MAX_ADAPTERS                     (4)	// we treat each Artesia on the 701 as its own adapter
#define MAX_CONTROLLERS_PER_ADAPTER      (1)
#define MAX_DEVICES_PER_PORT             (1)
#define MAX_PORTS_PER_CONTROLLER (ARTISEA_PORT_COUNT)

#define MAX_CONTROLLERS  (MAX_ADAPTERS * MAX_CONTROLLERS_PER_ADAPTER)
#define MAX_PORTS        (MAX_CONTROLLERS * MAX_PORTS_PER_CONTROLLER)
#define MAX_DEVICES      (MAX_PORTS * MAX_DEVICES_PER_PORT)

#define MAX_DEVICES_PER_ADAPTER (MAX_DEVICES / MAX_ADAPTERS)

/* The maximum depth of the Tagged Command Queue ( 1 / 32 )*/
#define MAX_TCQ_DEPTH      1

/* Replace these for a different controller */
#define COMMON_PORT_REG_SIZE (ARTISEA_COMMON_PORT_REG_SIZE)
#define SATA_PORT_REG_SIZE   (ARTISEA_SATA_PORT_REG_SIZE)

extern const INT INVALID_TAG; /* invalid {adapter,controller,port,device}Tag    */

/******************************************************************************/
/*                                                                            */
/*                   M A C R O   D E F I N I T I O N S                        */
/*                                                                            */
/******************************************************************************/

/* Returns True if val is between min, max ( min <= val < max ) */
#define WITHIN_RANGE(val, min, max) \
 ( ((val) >= min) && ((val) < (max)) )

/* Returns True if Tag is in allowed range, False if invalid tag */
#define TAG_WITHIN_RANGE(tag, max) \
 ( ((tag) != INVALID_TAG) && WITHIN_RANGE(tag, 0, max ) )

/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*    V A R I A B L E   A N D   S T R U C T U R E   D E F I N I T I O N S     */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                 F U N C T I O N    P R O T O T Y P E S                     */
/*                                                                            */
/******************************************************************************/

 /* Return index of next null ptr in array */
INT getNextSlot(void ** pointerArray_i, INT max_i);

#endif /* SATA_LIB_HEADER */
