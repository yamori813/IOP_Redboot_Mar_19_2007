/************************************************************************;
;*                                                                      
;*      Filename:  sata_port.c  								 
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

#include <sata_port.h>
#include <sata_globals.h>
#include <sata_device.h>
#include <artisea.h>


/******************************************************************************/
/*                                                                            */
/*       G L O B A L   F U N C T I O N   I M P L E M E N T A T I O N S        */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function initializes the port.                           */
/*                                                                            */
/* PRECONDITIONS    The private memory for the port has been allocated.       */
/*                                                                            */
/* POST-CONDITIONS  The port is initialized and ready for I/O                 */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to initialize.                           */
/*              portNumber_i                                                  */
/*                  The port number on the controller represented by this port*/
/*              controllerTag_i                                               */
/*                  The tag for the controller this port is on                */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in ports_gp array          */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*              SATA_NOT_INITIALIZED    = Memory was not allocated.           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortInit(INT portTag_i, INT portNumber_i, INT controllerTag_i)
{
    INT controller, currDev, ctr;
    SATA_REG_ADDR_T base;
    STATUS_CODE_T retval = SATA_SUCCESS;
    boolean deviceInitFailed = FALSE;
    PORT_INFO_T * currPort = NULL_POINTER;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if(ports_gp[portTag_i] == NULL_POINTER)      return SATA_NOT_INITIALIZED;

    /* Check if the portNumber_i is valid */
    if(portNumber_i >= MAX_PORTS)  // was MAX_PORTS_PER_CONTROLLER
    {
        return SATA_OUT_OF_RANGE_VALUE;
    }

    currPort = ports_gp[portTag_i];
    controller = currPort->controller = controllerTag_i;
    base = controllers_gp[controller]->commonRegBase;
    currPort->portNumber = portNumber_i;

    /* Calculate our starting offset. ASSUMES contiguous addresses.*/
    currPort->portRegBase = base + COMMON_PORT_REG_SIZE
                            + portNumber_i * SATA_PORT_REG_SIZE;

    /* Other Port initializations go here. */

    /* Init the error tracking information. */
    currPort->cumulativeSSER = 0;
    currPort->lastSSER       = 0;
    currPort->lastER         = 0;
    currPort->lastASR        = 0;
    currPort->lastSR         = 0;
    currPort->lastSSSR       = 0;

    /* initialize the devTag in the devices array */
    for(ctr = 0; ctr < MAX_DEVICES_PER_PORT; ctr++)
    {
        currPort->dev[ctr] = INVALID_TAG;
    }

    /* Initialization complete. Mark port as ready for use. */
    currPort->present = TRUE;

    /* Disable all the interrupts on the port. */
    /*The OSDM will enable these when it is ready */
    PortDisableInterrupts(portTag_i);

    /* reset the SATA interface and bring it online */
    if(PortIntHardReset(portTag_i) == SATA_SUCCESS)
    {
        currPort->connected = PortDevConnected(portTag_i);
    }
    else
    {

//      currPort->present = FALSE;
        currPort->connected = FALSE;
    }

    /* Now initialize all the devices connected to this port. */
    if(currPort->connected)
    {
   
        /* Now let's initialize each of the device */
        for(ctr = 0; ctr < MAX_DEVICES_PER_PORT; ctr++)
        {
			currDev = portTag_i;  /* we have a 1-1 mapping between portTag and devTag (SATA is point-to-point) */
			currPort->dev[ctr] = currDev; 

            retval = DevInit(currDev, ctr, portTag_i);
            
			if(retval != SATA_SUCCESS)
            {
                /* Treat device failures as non fatal. The device will be */
                /* disabled and the initialization will continue.         */
                deviceInitFailed = TRUE;
            }
        }

        if(deviceInitFailed == TRUE)
        {
            /* One or more devices failed during initialization */
            return SATA_COMPLETED_WITH_ERRORS;
        }
        /* Done with device initializations */
    }


    /* Successfully initialized the port. Return success */
    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function checks if a device is actively connected to the */
/*              port ( Dev present & communication established ). To check    */
/*              for other connection status call PortReadSStatusReg           */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to test.                                 */
/*                                                                            */
/* RETURNS      TRUE    if the device is connected and ready                  */
/*              FALSE   if the device is not ready.                           */
/*                                                                            */
/* ON ERRORS    Returns FALSE. Error condition ignored.                       */
/*                                                                            */
/******************************************************************************/
boolean PortDevConnected(INT portTag_i)
{
    UINT32 sssr = 0;
	INT i;
	SATA_REG_ADDR_T data32;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS) ||
       (ports_gp[portTag_i] == NULL_POINTER) ||   
       (ports_gp[portTag_i]->present != TRUE))
    {
        /* SATA_OUT_OF_RANGE_VALUE or SATA_NOT_INITIALIZED */
        return FALSE;
    }

    /* Set to not connected. Will be changed below... */
    ports_gp[portTag_i]->connected = FALSE;

    /* Read the register */
    PciMemRead32( ports_gp[portTag_i]->portRegBase + SUPDSSSR, &sssr);


#ifdef ERIC_DEBUG
	diag_printf ("PortDevConnected: Port %d SSSR = 0x%X\n", portTag_i, sssr);
#endif


	/* 0 = no device, 1 = Present but NO communication, 3 = Ready, 4 = offline */
    if ((sssr & SUPDSSSR_DEV_DET_MASK) == 0x3)
    {
        ports_gp[portTag_i]->connected = TRUE;
        // mark all devices active since we are connected
        for (i=0; i < MAX_DEVICES_PER_PORT; i++)
        {
            INT devTag = ports_gp[portTag_i]->dev[i];
            DEV_INFO_T * devp = (devTag == INVALID_TAG) ? NULL_POINTER : devices_gp[devTag];

            if (devp != NULL_POINTER)
            {
#ifdef ERIC_DEBUG
                diag_printf("PTag %d : Setting devTag %d active.",
                              portTag_i, devTag);
#endif
                devp->active = TRUE;
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE ;
    }
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of AltStatus register      */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadAltStatusReg(INT portTag_i, UINT8 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead8(ports_gp[portTag_i]->portRegBase + SUPDASR, data_op);

    ports_gp[portTag_i]->lastASR = *data_op;

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of Status register         */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadStatusReg(INT portTag_i, UINT8 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if(ports_gp[portTag_i] == NULL_POINTER || 
       ports_gp[portTag_i]->present != TRUE)
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead8( ports_gp[portTag_i]->portRegBase + SUPDSR, data_op );

    ports_gp[portTag_i]->lastSR = *data_op;

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of Control register        */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadControlReg(INT portTag_i, UINT8 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) || 
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead8(ports_gp[portTag_i]->portRegBase + SUPDDCTLR, data_op);

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of Error register          */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadErrorReg(INT portTag_i, UINT8 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS))
    {
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead8( ports_gp[portTag_i]->portRegBase + SUPDER, data_op );

    ports_gp[portTag_i]->lastER = *data_op;

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of Super Set Status        */
/*              SStatus register                                              */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadSStatusReg(INT portTag_i, UINT32 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead32(ports_gp[portTag_i]->portRegBase + SUPDSSSR, data_op);

    ports_gp[portTag_i]->lastSSSR = *data_op;

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of SError register         */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadSErrorReg(INT portTag_i, UINT32 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) || 
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead32(ports_gp[portTag_i]->portRegBase + SUPDSSER, data_op);

    ports_gp[portTag_i]->lastSSER = *data_op;
    ports_gp[portTag_i]->cumulativeSSER |= *data_op;

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of SControl register       */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadSControlReg (INT portTag_i, UINT32 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead32(ports_gp[portTag_i]->portRegBase + SUPDSSCR, data_op);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function returns the contents of Dma Status register     */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              data_op                                                       */
/*                  Address of the variable to hold the register value        */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadDmaStatusReg(INT portTag_i, UINT8 *data_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Read the register */
    PciMemRead8(ports_gp[portTag_i]->portRegBase + SUPDDSR, data_op);

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function reads "count_io"  bytes of data from the data   */
/*              port register (SUPDDR), putting them in the destination       */
/*              buffer.                                                       */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              count_i                                                       */
/*                  The number of bytes to be read from the data port reg.    */
/*              buffer_op                                                     */
/*                  Address of the destination buffer. Should be large enough */
/*                  to hold count_i bytes.                                    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortPioReadBytes(INT portTag_i, INT count_i, void * buffer_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Call the OS Service layer to transfer the data */
    PciMemReadDataPort(count_i, ports_gp[portTag_i]->portRegBase + SUPDDR, 
                       buffer_op);
    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function writes "count_io"  bytes of data to the data    */
/*              port register (SUPDDR).                                       */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              count_i                                                       */
/*                  The number of bytes to be written to the data port reg.   */
/*              buffer_ip                                                     */
/*                  Address of the Source buffer.                             */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortPioWriteBytes(INT portTag_i, INT count_i, void * buffer_ip)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Call the OS Service layer to transfer the data */
    PciMemWriteDataPort(count_i, buffer_ip,
                        ports_gp[portTag_i]->portRegBase + SUPDDR);
    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function writes the various parameters of the taskfile   */
/*              to the corresponding port specific registers. This function   */
/*              supports ONLY CHS/LBA28 addressing for now.                   */
/*              Only the lower 8 bits of sectorNumber_i, cylinderLow_i and    */
/*              cylinderHigh_i should be used at this time.                   */
/*                                                                            */
/*   T B D      Decide on parameter contents for LBA48. (not yet supported)   */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              taskfile_ip                                                   */
/*                  Contents of the task file registers. The command register */
/*                  is NOT written.                                           */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortWriteTaskfileParams(INT portTag_i,
                                      SATA_TASKFILE_T * taskfile_ip)
{

    STATUS_CODE_T retval = SATA_SUCCESS;
    SATA_REG_ADDR_T base = ports_gp[portTag_i]->portRegBase;

    /* Sanity check */
    if(taskfile_ip == NULL_POINTER) return SATA_OUT_OF_RANGE_VALUE;
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

#if 0 /* Don't think this is needed with LBA48 support */
    /* ONLY CHS / LBA28 addressing is supported. Clear all the bits except the*/
    /* lower 8 bits of sectorNumber_i, cylinderLow_i and cylinderHigh_i       */
    taskfile_ip->sectorNumber &= 0xff;
    taskfile_ip->cylinderLow  &= 0xff;
    taskfile_ip->cylinderHigh &= 0xff;
#endif

    /* Write to the register */
    PciMemWrite16(base + SUPDFR,  taskfile_ip->feature);
    PciMemWrite16(base + SUPDSCR, taskfile_ip->sectorCount);
    PciMemWrite16(base + SUPDSNR, taskfile_ip->sectorNumber);
    PciMemWrite16(base + SUPDCLR, taskfile_ip->cylinderLow);
    PciMemWrite16(base + SUPDCHR, taskfile_ip->cylinderHigh);
    PciMemWrite8(base + SUPDDHR, taskfile_ip->deviceHead);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function reads the parameters of the taskfile            */
/*              from the corresponding port specific registers.               */
/*                                                                            */
/*                                                                            */
/*   T B D      Decide on parameter contents for LBA48. (not yet supported)   */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              taskfile_ip                                                   */
/*                  Location to write the task file registers.                */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortReadTaskfileParams(INT portTag_i,
                                     SATA_TASKFILE_T * taskfile_op)
{

    STATUS_CODE_T retval = SATA_SUCCESS;
    SATA_REG_ADDR_T base = ports_gp[portTag_i]->portRegBase;

    /* Sanity check */
    if(taskfile_op == NULL_POINTER) return SATA_OUT_OF_RANGE_VALUE;
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }
    
    /* Read from the register */

    PciMemRead16(base + SUPDFR, &taskfile_op->feature);
    PciMemRead16(base + SUPDSCR, &taskfile_op->sectorCount);
    PciMemRead16(base + SUPDSNR, &taskfile_op->sectorNumber);
    PciMemRead16(base + SUPDCLR, &taskfile_op->cylinderLow);
    PciMemRead16(base + SUPDCHR, &taskfile_op->cylinderHigh);
    PciMemRead8(base + SUPDDHR, &taskfile_op->deviceHead);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function sets the value of the Command register          */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              cmd_i                                                         */
/*                  The SATA command to be written to the Command register    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortWriteCommandReg (INT portTag_i, UINT8 cmd_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Write to the register */
    PciMemWrite8(ports_gp[portTag_i]->portRegBase + SUPDCR, cmd_i);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function sets the value of the Control register          */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              value_i                                                       */
/*                  The new value to be written to the control register       */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortWriteControlReg (INT portTag_i, UINT8 value_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Write to the register */
    PciMemWrite8(ports_gp[portTag_i]->portRegBase + SUPDDCTLR, value_i);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function sets the Descriptor Table Pointer Registers     */
/*              ( Both Upper and lower DTPRs - SUPDUDDTPR, SUPDDDTPR )        */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              upperDTPR_i                                                   */
/*                   The value for SUPDUDDTPR (Upper DDTPR Register) which    */
/*                   contains the upper 32-bit PCI address of the 64-bit PCI  */
/*                   address. In PCI IDE mode, the Upper DDTPR Register is    */
/*                   not used.                                                */
/*              lowerDTPR_i                                                   */
/*                   The value for SUPDDDTPR (Lower DDTPR Register) which     */
/*                   contains the lower 32-bit PCI address.                   */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortSetDTPR(INT portTag_i, UINT32 upperDTPR_i, UINT32 lowerDTPR_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Write to the register */
    PciMemWrite32(ports_gp[portTag_i]->portRegBase + SUPDUDDTPR, upperDTPR_i);
    PciMemWrite32(ports_gp[portTag_i]->portRegBase + SUPDDDTPR, lowerDTPR_i);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              Sets the upper 32 bits of the addresses in the PRDT entries   */
/*              in the SUPDUDDBPR register                                    */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              value_i                                                       */
/*                   The value of SUPDUDDBPR which contains the upper 32-bit  */
/*                   PCI address of the 64-bit PCI address. All the           */
/*                   descriptors in the descriptor table share this register. */
/*                   All the DDT Entries share the same upper 32 bits of      */
/*                   their 64 bit addresses.                                  */
/*                   For example, all the data buffers must be located in the */
/*                   same 4-GByte memory space.                               */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortSetDdtePage(INT portTag_i, UINT32 value_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Write to the register */
    PciMemWrite32(ports_gp[portTag_i]->portRegBase + SUPDUDDBPR, value_i);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              writes to the DMA command register (SUPDDCMDR) for that port. */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*              command_i                                                     */
/*                  The value of SUPDDCMDR which enables/disables the DMA     */
/*                  engine (bus master capability) and also provides          */
/*                  direction control for DMA transfers.                      */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortWriteDmaCommandReg(INT portTag_i, UINT16 command_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Write to the register */
    PciMemWrite16(ports_gp[portTag_i]->portRegBase + SUPDDCMDR, command_i);

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function checks if an interrupt is pending on the port.  */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*                                                                            */
/* RETURNS      TRUE / FALSE                                                  */
/*                                                                            */
/* ON ERRORS    returns FALSE                                                 */
/*                                                                            */
/******************************************************************************/
boolean PortInterruptPending(INT portTag_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;
    INT controller;
    SATA_REG_ADDR_T base;
    UINT32 ipr;
    UINT8 dsr;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    controller = ports_gp[portTag_i]->controller;
    base = controllers_gp[controller]->commonRegBase;

    /* Read the register */
    PciMemRead32(base + SUPDIPR, &ipr);

    /* Move the interrupt bits relevant to this port in lower byte */
    ipr >>= SUPDIPR_BITS_PER_PORT * ports_gp[portTag_i]->portNumber;
    /* Mask off all bits not applicable to this port */
    ipr &=  SUPDIPR_BITS_PER_PORT_MASK;

    if(ipr > 0)
    {
        return TRUE; /* Some interrupt bits were set */
    }

    /* Check the DMA status register for a pending DMA Interrupt */
    retval = PortReadDmaStatusReg(portTag_i, &dsr);
    if(retval != SATA_SUCCESS) return FALSE;

    if(READ_BIT(dsr, SUPDDSR_DMA_INTR_BIT)) return TRUE;

    /* No indication of a pending interrupt.... */
    return FALSE;

}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function clears the interrupt for the given port.        */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortClearInterrupt(INT portTag_i)
{

    STATUS_CODE_T retval = SATA_SUCCESS;
    INT controller;
    SATA_REG_ADDR_T base;
    UINT32 ipr, serr;
    UINT8 stat;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    controller = ports_gp[portTag_i]->controller;
    base = controllers_gp[controller]->commonRegBase;

#ifdef SATA_DEBUG

    /* Read the register */
    PciMemRead32(base + SUPDIPR, &ipr);

    /* Move the interrupt bits relevant to this port in lower byte */
    ipr >>= SUPDIPR_BITS_PER_PORT * ports_gp[portTag_i]->portNumber;
    /* Mask off all bits not applicable to this port */
    ipr &=  SUPDIPR_BITS_PER_PORT_MASK;

    if(ipr <= 0) /* None of the interrupt bits were set */
    {
        return SATA_SUCCESS;
    }
#endif


    /* Clear the SATA Port IDE Interrupt by reading the taskfile status register */
    retval = PortReadStatusReg(portTag_i, &stat);
    if(retval != SATA_SUCCESS) return retval;

    /* Reset the DMA interrupt */
    stat = 0;
    stat = SET_BIT( 0, SUPDDSR_DMA_INTR_BIT);
    PciMemWrite8(ports_gp[portTag_i]->portRegBase + SUPDDSR, stat);

    /* Clear the rest of the interrupts by writing to SError register. */
    serr = 0; /* reset all bits to 0 */
    serr = SET_BIT(serr, SUPDSSER_DIAG_N_BIT);
    serr = SET_BIT(serr, SUPDSSER_ERR_M_BIT);
    serr = SET_BIT(serr, SUPDSSER_ERR_E_BIT);
    serr = SET_BIT(serr, SUPDSSER_DIAG_H_BIT);
    serr = SET_BIT(serr, SUPDSSER_ERR_P_BIT);
    serr = SET_BIT(serr, SUPDSSER_ERR_T_BIT);
    serr = SET_BIT(serr, SUPDSSER_DIAG_C_BIT);
    PciMemWrite32(ports_gp[portTag_i]->portRegBase + SUPDSSER, serr);

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function disables the interrupts for the given port.     */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortDisableInterrupts(INT portTag_i)
{
    PORT_INFO_T * port = NULL_POINTER;
    INT controller;
    SATA_REG_ADDR_T base;
    UINT32 mask, imr;
    UINT8 dctlr;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if(ports_gp[portTag_i] == NULL_POINTER ||
       ports_gp[portTag_i]->present != TRUE)
    {
        return SATA_NOT_INITIALIZED;
    }

    port = ports_gp[portTag_i];
    controller = ports_gp[portTag_i]->controller;
    base = controllers_gp[controller]->commonRegBase;

    /* Interrupts are enabled/disabled in a different way in PCI IDE mode       */
    /* and in DPA mode. In PCI IDE mode, you set the nIEN bit in Device Control */
    /* Register (SUPDCTLR) to mask the interrupts, and in DPA mode, you set the */
    /* correct bits in the interrupt mask registers (SUPDIMR)                   */

    if(controllers_gp[controller]->mode == PCI_DPA)
    {
        /* set the correct bits in the interrupt mask registers (SUPDIMR)      */
        /* 8 bits per port, set the bits to 0 to mask them. Set all the rest   */
        /* of the bits to 1. C-Spec Ver 0.94 page 171                          */

        /* Step 1 - Set the correct bit to 1 */
        mask = SET_BIT(0, SUPDIPR_IDE_INTR_BIT);

        /* 8 interrupt bits per port. Look at C-Spec 0.94, pg 167 for defn.    */
        /* Step 2 - Move the mask bit to the right posn for the current port   */
        mask = mask << ( 8 * port->portNumber);

        /* Step 3 - Bit inverse as 1 = enabled, 0 = disabled */
        /* mask ^= 0xffffffff; */
        mask = ~mask;

        /* Read the current values in IMR.... We don't want to enable interrupts*/
        /* for other ports by mistake !!! */

        /* Read the register */
        PciMemRead32(base + SUPDIMR, &imr);

        /* Step 4 - Modify the current values so that the 0s stay  */
        mask = mask & imr;

        /* write to the register */
        PciMemWrite32(base + SUPDIMR, mask);
    }
    else
    {
        /* PCI IDE mode / Legacy mode, write a 1 to the nIEN bit to disable the  */
        /* interrupt, and 0s to rest of the bits so that you don't change their  */
        /* values... Remember, SUPDDCTLR is a WRITE_ONLY register. You can't     */
        /* read-modify-write to it.                                              */
        dctlr = SET_BIT(0, SUPDDCTLR_nIEN_BIT);

        /* Write to the register */
        PciMemWrite8(base + SUPDDCTLR, dctlr);
    }

    /* Done.*/
    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function enables the interrupts for the given port.      */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortEnableInterrupts(INT portTag_i)
{
    PORT_INFO_T * port   = NULL_POINTER;
    INT controller;
    SATA_REG_ADDR_T base;
    UINT32 mask, imr ;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    port = ports_gp[portTag_i];
    controller = ports_gp[portTag_i]->controller;
    base = controllers_gp[controller]->commonRegBase;

    /* Interrupts are enabled/disabled in a different way in PCI IDE mode       */
    /* and in DPA mode. In PCI IDE mode, you set the nIEN bit in Device Control */
    /* Register (SUPDCTLR) to mask the interrupts, and in DPA mode, you set the */
    /* correct bits in the interrupt mask registers (SUPDIMR)                   */

    if(controllers_gp[controller]->mode == PCI_DPA)
    {
        /* set the correct bits in the interrupt mask registers (SUPDIMR)      */
        /* 8 bits per port, set the bits to 0 to mask them. Set all the rest   */
        /* of the bits to 1. C-Spec Ver 0.94 page 171                          */
        mask = SET_BIT(0, SUPDIPR_IDE_INTR_BIT);

        /* 8 interrupt bits per port. Look at C-Spec 0.94, pg 167 for defn.    */
        /* Move the mask bit to the right position for the current port number */
        mask = mask << ( 8 * port->portNumber);

        /* Read the current values in IMR.... We don't want to enable interrupts*/
        /* for other ports by mistake !!! */

        /* Read the register */
        PciMemRead32(base + SUPDIMR, &imr);

        /* Step 4 - Modify the current values so that the 1s stay  */
        mask = mask | imr;

        /* write to the register */
        PciMemWrite32(base + SUPDIMR, mask);
    }
    else
    {
        /* PCI IDE mode / Legacy mode, write a 0 to the nIEN bit to enable the  */
        /* interrupt again, and 0s to rest of the bits so that you don't change */
        /* their values... Remember, SUPDDCTLR is a WRITE_ONLY register. You    */
        /* can't read-modify-write to it.                                       */

        /* Write to the register */
        PciMemWrite8(base + SUPDDCTLR, 0);
    }

    /* Done.*/
    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function clears the errors for the given port.           */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to use.                                  */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Port not initialized                */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortClearError(INT portTag_i)
{

    INT controllerTag;
    SATA_REG_ADDR_T base;
    UINT32 serr;
    UINT8 stat;
    PORT_INFO_T * currPort = NULL_POINTER;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS)) return SATA_OUT_OF_RANGE_VALUE;
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    currPort = ports_gp[portTag_i];
    controllerTag = currPort->controller;
    base = controllers_gp[controllerTag]->commonRegBase;


    /* Clear the SUPDSSER */
    /* Read the SError reg so that the Last read value is recorded. */
    PortReadSErrorReg(portTag_i,&serr);
    // Now, let's clear all the non-reserved error bits by writing 1s to them    
    serr = 0;
    serr = SET_BIT(serr,SUPDSSER_DIAG_F_BIT); // Unknown FIS                     
    serr = SET_BIT(serr,SUPDSSER_DIAG_H_BIT); // R_ERR recieved. Handshake err   
    serr = SET_BIT(serr,SUPDSSER_DIAG_C_BIT); // CRC error detected              
    serr = SET_BIT(serr,SUPDSSER_DIAG_D_BIT); // Disparity error detected        
    serr = SET_BIT(serr,SUPDSSER_DIAG_W_BIT); // Comm Wake detected              
    serr = SET_BIT(serr,SUPDSSER_DIAG_N_BIT); // PHYRDY Change state detected    
    serr = SET_BIT(serr,SUPDSSER_ERR_E_BIT);  // Inter err: FIFO over/underrun
    serr = SET_BIT(serr,SUPDSSER_ERR_P_BIT);  // Protocol err: Corrupt FIS       
    serr = SET_BIT(serr,SUPDSSER_ERR_C_BIT);  // Non Recovered Comm. No signal   
    serr = SET_BIT(serr,SUPDSSER_ERR_T_BIT);  // Transient Data Integrity err    
    serr = SET_BIT(serr,SUPDSSER_ERR_M_BIT);  // Recovered comm err PHY Ready

    /* Write the register */
    PciMemWrite32(currPort->portRegBase + SUPDSSER, serr);

    /* Clear the SUPDDSR Bit 01 DMA Err by writing a 1 to it. */
    stat = 0;
    stat = SET_BIT(0, SUPDDSR_DMA_ERROR_BIT);

    /* Write the register */
    PciMemWrite8(currPort->portRegBase + SUPDDSR, stat);

    /* Clear SUPDSR (Bit 0 ERR ) - Auto cleared when the next command issued */
    /* Clear SUPDER - Automatically cleared when the next command is issued. */
    /* don't bother with BIST FIS CSR - SUPDBFCSR(Bit 25 )? */

    /* All errors cleared.*/
    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION     PortIntHardReset()                                            */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function resets the SATA interface and bring            */
/*               the PHY online. Should be called in order to bring PHY       */
/*               online. The BIST done by software will call this after test  */
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
STATUS_CODE_T PortIntHardReset(INT portTag_i)
{
    UINT32 reset = 0;
    INT controllerTag;
    PORT_INFO_T * currPort = NULL_POINTER;
    SATA_REG_ADDR_T base;
    UINT32 sssr = 0;
    INT count = 0;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS))
    {
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    currPort = ports_gp[portTag_i];
    controllerTag = currPort->controller;
    base = currPort->portRegBase; // Address of the port specific region


    /* bring PHY online and start COMRESET init sequence */
    reset = 0x1;
    PciMemWrite32(currPort->portRegBase + SUPDSSCR, reset);

#ifdef ERIC_DEBUG 
	diag_printf("Initiate COMRESET...");
#endif
    MillisecondSleep(500);

    /*  stop initialization and bring into online state */
    reset = 0;
    PciMemWrite32(currPort->portRegBase + SUPDSSCR, reset);

    count = 2;
    do
    {
        MillisecondSleep(500);
        /* read the status */
        PciMemRead32(currPort->portRegBase + SUPDSSSR, &sssr);
    }
    while (!(sssr & SUPDSSSR_DEV_ONLINE) && --count);

    /* if device status not present and online */
    if (!(sssr & SUPDSSSR_DEV_ONLINE))
    {
#ifdef ERIC_DEBUG

        diag_printf("COMRESET failed for controller %d port %d. sssr = 0x%0x ",
                      controllerTag, portTag_i, sssr);
#endif
        return SATA_NOT_INITIALIZED;
    }

#ifdef ERIC_DEBUG
    diag_printf("COMRESET Succeeded for controller %d port %d\n",
                  controllerTag, portTag_i);
#endif

    return SATA_SUCCESS;
} /* end of PortIntHardReset() */

/******************************************************************************/
/*                                                                            */
/* FUNCTION     PortBistStatus()                                              */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function starts the built in self test (BIST) by        */
/*               doing the far end retimed loopback test using the 16bit      */
/*               counting pattern. The function puts the hardware into PHY    */
/*               online mode after doing a COMRESET                           */
/*                                                                            */
/* PRECONDITIONS    The port is ready for BIST at initialization              */
/*                                                                            */
/* POST-CONDITIONS  The PHY is online and ready                               */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  Tag for the port to be used.                              */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortBistStatus(INT portTag_i)
{
    STATUS_CODE_T retval = SATA_SUCCESS;
    UINT32 reset = 0;
    INT controllerTag;
    PORT_INFO_T * currPort = NULL_POINTER;
    SATA_REG_ADDR_T base;
    UINT32 bistCsr = 0;
    UINT32 bistErr = 0;
    UINT32 bistCount = 0;
    INT timeout = 0;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(portTag_i, MAX_PORTS))
    {
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if((ports_gp[portTag_i] == NULL_POINTER) ||
       (ports_gp[portTag_i]->present != TRUE))
    {
        return SATA_NOT_INITIALIZED;
    }

    currPort = ports_gp[portTag_i];
    controllerTag = currPort->controller;
    base = controllers_gp[controllerTag]->commonRegBase;

    /* set bit 25 to clear frame and error */
    bistCsr = SUPDBFCSR_CLR_BIT;

    PciMemWrite32(base + SUPDBFCSR, bistCsr);

    bistCsr = SUPDBFCSR_PATTERN |   // set the test pattern
              SUPDBFCSR_RLOOPBACK |       // set retimed loopback
              SUPDBFCSR_PATGEN |          // set pattern generator
              SUPDBFCSR_PATCHK;           // set pattern checker

    PciMemWrite32(base + SUPDBFCSR, bistCsr);

    do
    {
        MillisecondSleep(100);  // delay for 100ms to let the BIST run
        PciMemRead32(base + SUPDBFR, &bistCount);

        if(timeout++ > 10)
        {
            diag_printf("BIST timed out without starting...bail\n");
            return SATA_FAILURE;
        }

    } while(bistCount < 10); // as long as frame count < 10, we keep running

    /* read the error count to check for errors */
    PciMemRead32(base + SUPDBER, &bistErr);

    /* bist detected failure */
    if(bistErr)
    {
        diag_printf("BIST failed with count: %d\n", bistErr);
        return SATA_FAILURE;
    }

    /* Is this needed if we are resetting the interface? */
    bistCsr = 0;
    PciMemWrite32(base + SUPDBFCSR, bistCsr);

    // Now we need to do a COMRESET/COMINIT sequence to conclude the test

    /* hardreset to stop BIST */
    if((retval = PortIntHardReset(portTag_i)) != SATA_SUCCESS)
    {
        return retval;
    }

    return SATA_SUCCESS;
} /* end of PortBistStatus() */

