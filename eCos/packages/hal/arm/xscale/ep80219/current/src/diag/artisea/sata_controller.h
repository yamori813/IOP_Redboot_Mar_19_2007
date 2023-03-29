/************************************************************************;
;*                                                                      
;*      Filename: sata_controller.h  								 
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
/*         This file includes definations related to the SATA controller.     */
/*         Many of the definations might be controller specific.              */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_CONTROLLER_HEADER
#define SATA_CONTROLLER_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver          */
#include <sata_lib.h>   /* Definations of structures internal to SATA Lib */
#include <artisea.h>
#include <sata.h>


/******************************************************************************/
/*                                                                            */
/*                C O N S T A N T   D E F I N I T I O N S                     */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                   M A C R O   D E F I N I T I O N S                        */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/

typedef struct CONTROLLER_INFO_S
{
    boolean  present;    /* controller present and struct data valid ?        */
    INT     port[MAX_PORTS_PER_CONTROLLER]; /* Index #s into ports_gp         */
    INT     adapter;     /* Index number of the parent adapter                */
    INT     controllerNumber; /* 0 - n controller on the adapter              */
    SATA_CONTROLLER_MODE_T mode; /* Mode of operation                         */
    SATA_REG_ADDR_T commonRegBase; /* Base address for common SATA registers  */
} CONTROLLER_INFO_T;

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
/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function allocates the private memory for the controller.*/
/*              it allocates a CONTROLLER_INFO_T structure and records the    */
/*              pointer in the next available slot in the controllers_gp array*/
/*              This function also initializes the allocated memory structure */
/*              and sets the adapter member to the passed adapterTag.         */
/*                                                                            */
/* PRECONDITIONS    controllers_gp array has been allocated.                  */
/*                                                                            */
/* POST-CONDITIONS  The next NULL pointer in controllers_gp is set.           */
/*                  *controllerTag_op is set to it's index number/INVALID_TAG */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  The Tag of the parent adapter.                            */
/*              controllerTag_op                                              */
/*                  Address of the INT variable to hold the controllerTag.    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in controllers_gp array    */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/

STATUS_CODE_T ControllerAllocatePrivateMemory (
    INT adapterTag_i,
    INT *controllerTag_op
    );

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function initializes the controller.                     */
/*                                                                            */
/* PRECONDITIONS    The private memory for the controller has been allocated. */
/*                                                                            */
/* POST-CONDITIONS  The controller and attachec ports are initialized and     */
/*                  ready for I/O                                             */
/*                                                                            */
/* PARAMETERS   controllerTag_i                                               */
/*                  Tag for the controller to initialize.                     */
/*              controllerNumber_i                                            */
/*                  The controller number on the adapter represented by this  */
/*                  controller. Currently we expect only one controller per   */
/*                  adapter.                                                  */
/*              baseAddr_i                                                    */
/*                  The base address to the registers on the controller.      */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in controllers_gp array    */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T ControllerInit (
    INT controllerTag_i,       /* Tag for the controller to initialize.*/
    INT controllerNumber_i,    /* The controller number on the adapter */
    SATA_REG_ADDR_T baseAddr_i,/* base addr to the registers on the controller*/
	INT adapterTag_i		   /* adapter number of this controller */
    );

/******************************************************************************/
/*                                                                            */
/* FUNCTION     ControllerIntHardReset()                                      */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function resets the SATA interface of all ports         */
/*               and bring the PHY online. Should be called in order to bring */
/*               PHY online. The BIST done by software will call this after   */
/*               test                                                         */
/*                                                                            */
/* PRECONDITIONS    The port is ready for BIST at initialization              */
/*                                                                            */
/* POST-CONDITIONS  The PHY is offline or in BIST mode                        */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to be used.                              */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T ControllerIntHardReset(INT controllerTag_i);

#endif /* SATA_CONTROLLER_HEADER */
