/************************************************************************;
;*                                                                      
;*      Filename:  sata_controller.c								 
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
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_controller.h>
#include <sata_globals.h>
#include <sata_lib.h>



/******************************************************************************/
/*                                                                            */
/*       G L O B A L   F U N C T I O N   I M P L E M E N T A T I O N S        */
/*                                                                            */
/******************************************************************************/



/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function initializes the controller.                     */
/*                                                                            */
/* PRECONDITIONS    The private memory for the controller has been allocated. */
/*                                                                            */
/* POST-CONDITIONS  The controller and attached ports are initialized and     */
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
/*              SATA_NOT_INITIALIZED    = Memory was not allocated.           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T ControllerInit (
    INT controllerTag_i,       /* Tag for the controller to initialize.*/
    INT controllerNumber_i,    /* The controller number on the adapter */
    SATA_REG_ADDR_T baseAddr_i, /* base addr to the registers on the controller*/
    INT adapterTag_i
	)
{
    INT portTag_i, ctr;
    STATUS_CODE_T retval = SATA_SUCCESS;
    boolean deviceInitFailed = FALSE;


    /* Sanity check */
    if ( !TAG_WITHIN_RANGE(controllerTag_i, MAX_CONTROLLERS) )
        return SATA_OUT_OF_RANGE_VALUE ;
    if ( controllers_gp[controllerTag_i] == NULL_POINTER ) return SATA_NOT_INITIALIZED;

    /* Check if the controllerNumber_i is valid */
    if (controllerNumber_i >= MAX_CONTROLLERS_PER_ADAPTER) return SATA_OUT_OF_RANGE_VALUE;
    controllers_gp[controllerTag_i]->controllerNumber = controllerNumber_i;

    controllers_gp[controllerTag_i]->commonRegBase = baseAddr_i;

	/* if we got this far, the controller is in DPA mode (tests currently only work in DPA mode) */
	controllers_gp[controllerTag_i]->mode = PCI_DPA;

	/* set the adapter number for this controller */
	controllers_gp[controllerTag_i]->adapter = adapterTag_i;

    /* Initialization complete. Mark controller as ready for use. */
    controllers_gp[controllerTag_i]->present = TRUE;

    /* Any other controller initializations go here. None for Artisea */
 
	
    /* Now let's initialize each of the ports on this controller */
    for ( ctr = 0; ctr < MAX_PORTS_PER_CONTROLLER; ctr++ )
    {
		portTag_i = MAX_PORTS_PER_CONTROLLER * controllerTag_i + ctr;

		controllers_gp[controllerTag_i]->port[ctr] = portTag_i;

        retval = PortInit(portTag_i, ctr, controllerTag_i);
		
        if (retval == SATA_COMPLETED_WITH_ERRORS)
        {
            /* SATA_COMPLETED_WITH_ERRORS implies that a device init failed */
            /* Non fatal error */
            deviceInitFailed = TRUE;
        }
        else if ( retval != SATA_SUCCESS)
        {
            /* Treat port failures as fatal.  */
            return retval;
        }
    }

    if ( deviceInitFailed == TRUE )
    {
        /* One or more devices failed during initialization */
        return SATA_COMPLETED_WITH_ERRORS;
    }

    /* Done with controller & port initializations */

    /* Successfully initialized. Return success */
    return SATA_SUCCESS;
}

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
/*INLINE*/
STATUS_CODE_T ControllerIntHardReset(INT controllerTag_i)
{
    INT ctr;
    INT currPort;

    STATUS_CODE_T retval = SATA_SUCCESS;

    for(ctr = 0; ctr < MAX_PORTS_PER_CONTROLLER; ctr++)
    {
        //ASSERT(controllers_gp[controllerTag_i]);
        currPort = controllers_gp[controllerTag_i]->port[ctr];

        if((retval = PortIntHardReset(currPort)) != SATA_SUCCESS )
        {
            return retval;
        }

    }

    return SATA_SUCCESS;
} /* end of ControllerIntHardReset() */
