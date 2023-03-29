/************************************************************************;
;*                                                                      
;*      Filename:  sata_device.c								 
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


#include <sata.h>
#include <sata_port.h>
#include <sata_globals.h>
#include <sata_device.h>
#include <sata_dma.h>
#include <artisea.h>


/******************************************************************************/
/*                                                                            */
/*            L O C A L   F U N C T I O N    P R O T O T Y P E S              */
/*                                                                            */
/******************************************************************************/

static STATUS_CODE_T DevProcessPioRequest(INT devTag_i,
                                          SATA_COMMAND_T * sataCmd_iop);
static STATUS_CODE_T DevProcessDmaRequest(INT devTag_i,
                                          SATA_COMMAND_T * sataCmd_iop);
static STATUS_CODE_T DevEndCommand(INT devTag_i,
                                   SATA_COMMAND_T * sataCmd_iop);
static void FixString(char *start, const int len);
static STATUS_CODE_T NonDataIsr(INT devTag_i, UINT32 ipr);

/******************************************************************************/
/*                                                                            */
/*       G L O B A L   F U N C T I O N   I M P L E M E N T A T I O N S        */
/*                                                                            */
/******************************************************************************/




/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function initializes the device.                         */
/*                                                                            */
/* PRECONDITIONS    The private memory for the device has been allocated.     */
/*                                                                            */
/* POST-CONDITIONS  The device is initialized and ready for I/O               */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to initialize.                         */
/*              devNumber_i                                                   */
/*                  The device number on the port represented by this device  */
/*                  RSM Not implemented. This Param is currently Ingored      */
/*              portTag_i                                                     */
/*                  Tag for port this device is connected to                  */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in devices_gp array        */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*              SATA_NOT_INITIALIZED    = Memory was not allocated.           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevInit(INT devTag_i, INT devNumber_i, INT portTag_i)
{
    DEV_INFO_T *devp = NULL_POINTER;
    int ctr;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if(devices_gp[devTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    devp = devices_gp[devTag_i];

    devp->port = portTag_i;

    devp->ioMode = VARIABLE; /* Use Command specified mode by default */
    for(ctr = 0; ctr < MAX_TCQ_DEPTH; ctr++)
    {
        devp->commandQueue[ctr].slotState = SLOT_FREE;
    }

    /* The following 4 elements are set during AdapterAttachDisk */
    devp->osDriveTag      = NULL_POINTER;
    devp->dmaHandler      = NULL_POINTER;
    devp->pioReadHandler  = NULL_POINTER;
    devp->pioWriteHandler = NULL_POINTER;

    /* The following 4 elements will be set during DeviceIdentify()*/
    devp->isDrive = TRUE;
    devp->lbaCapacity = 0;
    // devp->multisector = 0;
    MemSet(sizeof(IDENTIFY_DEVICE_T), 0, &(devp->devIdentify));

    //OsSpinlockInit(&devp->cmdQLock);
    //OsSpinlockInit(&devp->freeCmdQLock);

    /* set feature list to point to global default before OSDM sets it */
    devp->devFeatures = &devFeatures_g;

    /* need to change this when TCQ fully supported */
    devp->cmdFreeQueue = devp->commandQueue;

    /* Initialization complete. Mark device as ready for use. */
    devp->present = TRUE;
    devp->active = TRUE;

    /* Successfully initialized the device. Return success */
    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function performs a soft reset of the device.            */
/*                                                                            */
/* PRECONDITIONS    The private memory for the device has been allocated.     */
/*                                                                            */
/* POST-CONDITIONS  The device is initialized and ready for I/O               */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to initialize.                         */
/*              devNumber_i                                                   */
/*                  The device number on the port represented by this device  */
/*                  RSM Not implemented. This Param is currently Ingored      */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in devices_gp array        */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*              SATA_NOT_INITIALIZED    = Memory was not allocated.           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevSoftReset(INT devTag_i, INT devNumber_i)
{

/*     For a soft reset :                                                     */
/*         1. Set SRST bit in Device Control Reg.                             */
/*         2. Wait for 5 microseconds                                         */
/*         3. Clear SRSt bit.                                                 */
/*         4. wait for 2 milliseconds                                         */
/*         5. Read status / Alt Status register                               */
/*         6. Wait for the BSY bit to be cleared.                             */
/*         7. Check the Error Register and signature (pg 280)                 */
/*           (sect cnt=01h, sect #=01h, cyl_low=0h, cyl_high=0h, dev/head=0h) */
/*         8. Done.                                                           */
/*     We should expect the next command to be an Identify device             */

    /* Not yet implemented */
    return SATA_FAILURE;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function selects a device. Until RSM hardware exists,    */
/*                 this function simply returns SATA_SUCCESS                  */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success                                       */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T SelectDevice(INT devTag_i)
{

    /* we have one device per port at this stage (SATA 1.0)   */
    /* therefore the current device is it! no need to select. */

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }

    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    return SATA_SUCCESS;
}



/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function issues the command provided in the taskfile     */
/*              without any processing if the device is not busy. It writes   */
/*              all the taskfile parameters :                                 */
/*              Feature Reg, Sector Count Reg, Sector Number Reg, Cylinder    */
/*              Low Reg, Cylinder High Reg and Device Head Reg in that order. */
/*              It then writes to the Command Register and returns.           */
/*                                                                            */
/* PRECONDITIONS   The device has been initialized, and selected (caller is   */
/*                 expected to have called SelectDevice)                      */
/*                                                                            */
/* POSTCONDITIONS  The Interrupts on the drive will be *Enabled* if           */
/*                 waitOption_i was TRUE, un-modified otherwise.              */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              waitOption_i                                                  */
/*                  Should the API wait for the command to be completed ?     */
/*                  False = No wait. Write out the taskfile contents & return.*/
/*                  True = Wait.(Disable interrupts,Issue command,WAIT for dev*/
/*                      ready, Clear interrupts, Enable interrupts and return)*/
/*              taskfile_ip                                                   */
/*                  The exact contents of the taskfile to be written to the   */
/*                  device.                                                   */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_BUSY               = Device not ready (BSY bit set)      */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevExecuteRawCommand(
    INT             devTag_i,       /* DevTag of the device */
    boolean         waitOption_i,   /* F = Issue the command. Do nothing else */
                                    /* T = Wait for the command completion    */
                                    /*     with drive Interrupts disabled     */
    SATA_TASKFILE_T * taskfile_ip   /* Taskfile contents for the command      */
                                  )
{
    DEV_INFO_T     * devp    = NULL_POINTER;
    INT              portTag = INVALID_TAG;
    STATUS_CODE_T   retval   = SATA_SUCCESS;
    INT             ctr;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
    if(taskfile_ip == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if(DevIsReady(devTag_i) == FALSE) return SATA_BUSY;

    devp = devices_gp[devTag_i];
    portTag = devp->port;

    retval = PortWriteTaskfileParams( portTag, taskfile_ip );
    if(retval != SATA_SUCCESS)
    {
        diag_printf("Failed(%d) to write TaskfileParams. " 
                      "Command not yet issued", retval);
        return retval;
    }


    if(waitOption_i == TRUE) /* Wait for command completion */
    {
        /* Disable Interrupts on the drive.*/
        retval = DevDisableInterrupts(devTag_i);
        if(retval != SATA_SUCCESS)
        {
            diag_printf("Failed(%d) to disable device interrupts. "
                          "Command not yet issued", retval);

            /* Try to enable interrupts before we return ? */
            /* DevEnableInterrupts(devTag_i) */
            return retval;
        }
    }


 /* clear the interrupt and error status of the port before issuing a command */
    PortClearInterrupt (portTag);
    PortClearError (portTag);

    retval = PortWriteCommandReg(portTag, taskfile_ip->command);

    /* return if we are not asked to wait...*/
    if(waitOption_i == FALSE) return retval;

    /* return if there was an error issuing the command... */
    if(retval != SATA_SUCCESS)
    {
        /* We were asked to wait, so we must have disabled the interrupts */

        /* Clear the Interrupt */
        PortClearInterrupt (portTag);
        /* Enable interrupts before we return... */
        DevEnableInterrupts(devTag_i);

        return retval;
    }

    /* Wait for the drive to be ready. */

    /* Wait for some time to allow the drive to asserts Busy */
    /* spec allows drive 400ns to assert "BUSY" */

    /* Sleep for a Microsecond */
    MicrosecondSleep(1);

    /* Make sure the drive is ready... */
    for(ctr = 0; ctr < WAIT_DRQ; ctr ++)
    {
        /* is the drive ready for I/O ? */
        if(DevIsReady(devTag_i) == TRUE) break;

        /* Wait for 1 Millisecond more for the drive to be ready */
        MillisecondSleep(1);
    }

    /* Clear the Interrupt */
    PortClearInterrupt (portTag);

    /* Enable interrupts before we return... */
    DevEnableInterrupts(devTag_i);

    if(ctr >= WAIT_DRQ)
    {
        /* We Timed out ... */
        return SATA_BUSY;
    }

    return SATA_SUCCESS;
}



/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function adds a command to the device queue. This API    */
/*              should not be called from the OSDM (unless you know what you  */
/*              are doing)                                                    */
/*                                                                            */
/* PRECONDITIONS   The adapter has a command ready to send to the drive       */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              sataCmd_iop                                                   */
/*                  The command to be added to the commandQueue               */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_BUSY = No unused command slots found. Try again after a  */
/*                          command completes                                 */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevAddCommandToQueue(INT devTag_i, SATA_COMMAND_T * sataCmd_iop)
{
    DEV_INFO_T     * devp      = NULL_POINTER;
    SATA_COMMAND_T * sataCmd_p = NULL_POINTER;
    INT flags                  = 0;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
    
    if(sataCmd_iop == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    devp = devices_gp[devTag_i];

    /* NPG: TCQ Not yet supported. */
    /* We allow only ONE command to be issued at a time. */
    sataCmd_p = &devp->commandQueue[0];

    /* lock command queue for modification */
#if 0 /* not sure what to do with the spinlocks yet....... */
	OsSpinLock(&devp->cmdQLock, flags);
    if(sataCmd_p->slotState != SLOT_FREE)
    {
        //OsSpinUnlock(&devp->cmdQLock, flags);
        /* A command has already been queued and has not yet completed. */
        diag_printf("Command Slot is in use(%d). Returning Busy", 
                      sataCmd_p->slotState);
        return SATA_BUSY;
    }
    /* Fill out the command structure */

    sataCmd_p->slotState = SLOT_OCCUPIED;
    /* we can unlock it once we deem it occupied */
    //OsSpinUnlock(&devp->cmdQLock, flags);
#endif

    MemCopy(sizeof(SATA_COMMAND_T), sataCmd_iop, sataCmd_p);

    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function services the device's command queue.            */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_BUSY               = Drive not ready                     */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevServiceCommandQueue(INT devTag_i)
{
    /* when this function is called, the device issues any pending commands */
    /* that are currently in the command queue (we expect max of 1 command) */

    SATA_COMMAND_T * curCmd    = NULL_POINTER;
    DEV_INFO_T     * devp      = NULL_POINTER;
    STATUS_CODE_T retval       = SATA_SUCCESS;
    INT portTag                = INVALID_TAG;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    devp = devices_gp[devTag_i];
    portTag = devp->port;

    /* NPG: TCQ Not yet supported. */
    /*We allow only ONE command to be issued at a time. */
    curCmd = &devp->commandQueue[0];

    if(curCmd->slotState != SLOT_OCCUPIED)
    {
        /* No commands to be issued. */
        diag_printf("Dev %d: No Pending commands to be issued. State = %d",
                      devTag_i, curCmd->slotState);
        return SATA_SUCCESS;
    }

    /* Check if the drive is ready to accept the next command */
    if(DevIsReady(devTag_i) != TRUE)
    {
        /* Well, the Drive is not Ready. Return SATA_BUSY */
        return SATA_BUSY;
    }

    /* Drive is ready... write the task file parameters */
    retval = PortWriteTaskfileParams(portTag, &curCmd->taskfile);
    if(retval != SATA_SUCCESS) 
	{
		return retval;
	}

    /* From here on the steps differ for PIO transfers and DMA transfers */

    if ((curCmd->cmdType == SATA_WRITE) || (curCmd->cmdType == SATA_READ))
    {
        /* Is this a PIO or a DMA command ? */
        if(curCmd->txType == FIRST_PARTY_DMA)
        {
            retval = DevProcessDmaRequest(devTag_i, curCmd);
        }
        else if(curCmd->txType == PIO)
        {
            retval = DevProcessPioRequest(devTag_i, curCmd);
        }
        else
        {
            /* neither 1st party DMA nor PIO ... */
            /*3rd Party DMA not yet supported */
			return SATA_OUT_OF_RANGE_VALUE;
        }
    }
    else /* SATA_SPECIAL */
    {
      /* this is not a read or write command, so process this special command */
      /* But special commands should not have been queued in the first place! */
      /* TBD - Decide what is to be done for this case. */

        return SATA_OUT_OF_RANGE_VALUE;
    }

    /* act on the return value for the DevProcess???Request function calls */
    /* Did we successfully issue the command? If not, the command is left  */
    /* in the command queue as a pending command, else it is marked as an  */
    /* active command */
    if(retval != SATA_SUCCESS) return retval;

    /* Mark slotState as containing an Active Command. */
    curCmd->slotState = SLOT_ACTIVE;

    /* NOTE: An interrupt will be generated for every completed command. The */
    /*       ISR will change the slotstate to SLOT_FREE                      */

    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function determines whether the device is ready.         */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      TRUE / FALSE                                                  */
/*                                                                            */
/* ON ERRORS    returns FALSE on any errors                                   */
/*                                                                            */
/******************************************************************************/
boolean DevIsReady(INT devTag_i)
{
    UINT8 altStatus = 0;
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    /* check the altstatus register  */
    retval = PortReadAltStatusReg(devices_gp[devTag_i]->port, &altStatus);

    /* Failed to get register value. ASSUME device is NOT ready. */
    if(retval != SATA_SUCCESS) return FALSE;

    /*  if the BSY bit is 0, then we report that the device is ready  */
    /* The caller might need to check the DRQ bit to see if the drive */
    /* is ready for/with data                                         */

    /* What is DRDY used for ? (READ_BIT( altStatus, SUPDASR_DRDY_BIT) == 1 )*/
    if(READ_BIT( altStatus, SUPDASR_BSY_BIT)  == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function determines whether the DRQ is set.              */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      FALSE if DRQ-Bit is set,                                      */
/*              TRUE  if DRQ-Bit is 0,                                        */
/*                                                                            */
/* ON ERRORS    returns FALSE on any errors                                   */
/*                                                                            */
/******************************************************************************/
boolean DevDRQSet(INT devTag_i)
{
    UINT8 altStatus = 0;
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    /* check the altstatus register  */
    retval = PortReadAltStatusReg(devices_gp[devTag_i]->port, &altStatus);

    /* Failed to get register value. ASSUME device is NOT ready. */
    if(retval != SATA_SUCCESS) return FALSE;

    /* if the BSY bit is 0, then we report that the device is ready   */
    /* The caller might need to check the DRQ bit to see if the drive */
    /* is ready for/with data                                         */

    if((READ_BIT( altStatus, SUPDASR_DRQ_BIT) == 1 ))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function gets the status of the device by calling the    */
/*              PortReadStatusReg fcn. Calling this function will have all    */
/*              side effects of reading the taskfile status register.         */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              status_op                                                     */
/*                  pointer to a UINT8 variable that will hold the status.    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error codes on error                 */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevGetStatus(INT devTag_i, UINT8 * status_op )
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if ( !TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES) ) 
	{ 
		diag_printf("Value out of range: %d", devTag_i); 
		return SATA_OUT_OF_RANGE_VALUE ;
	}
    if ( devices_gp[devTag_i] == NULL_POINTER )     
		return SATA_NOT_INITIALIZED;
    if ( devices_gp[devTag_i]->present == FALSE )   
		return SATA_NOT_INITIALIZED;

    retval = PortReadStatusReg(devices_gp[devTag_i]->port, status_op);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function gets the Alt status of the device by calling    */
/*              the PortReadAltStatusReg fcn. Calling this function WILL NOT  */
/*              automatically clear the error on the port. Call DevClearError */
/*              to clear the errors.                                          */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              status_op                                                     */
/*                  pointer to a UINT8 variable that will hold the status.    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error codes on error                 */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevGetAltStatus(INT devTag_i, UINT8 * status_op )
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    retval = PortReadAltStatusReg(devices_gp[devTag_i]->port, status_op);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function gets the DMA status of the device by calling    */
/*              the PortReadDmaStatusReg fcn. Calling this function WILL NOT  */
/*              automatically clear the error on the port. Call DevClearError */
/*              to clear the errors.                                          */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              status_op                                                     */
/*                  pointer to a UINT8 variable that will hold the status.    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error codes on error                 */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevGetDmaStatus(INT devTag_i, UINT8 * status_op )
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    retval = PortReadDmaStatusReg(devices_gp[devTag_i]->port, status_op);

    return retval;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function gets the contents of the taskfile error reg.    */
/*              by calling the PortReadErrorReg fcn.                          */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              error_op                                                      */
/*                  pointer to a UINT8 variable that will hold the status.    */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error codes on error                 */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevGetError(INT devTag_i, UINT8 * error_op )
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if ( !TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES) )
	{ 
		diag_printf("Value out of range: %d", devTag_i); 
		return SATA_OUT_OF_RANGE_VALUE ;
	}
    if ( devices_gp[devTag_i] == NULL_POINTER )     
		return SATA_NOT_INITIALIZED;
    if ( devices_gp[devTag_i]->present == FALSE )   
		return SATA_NOT_INITIALIZED;

    retval = PortReadErrorReg(devices_gp[devTag_i]->port, error_op);

    return retval;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function gets both the SStatus and the Serror superset   */
/*              status registers along with a the error task file for the     */
/*              currently pending task.                                       */
/*              NOTE that it is up to the OSDM to parse this data.            */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*              An outstanding task with a pending error.                     */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              sStatus_op                                                    */
/*                  pointer to a UINT32 variable that will hold the sStatus.  */
/*              sError_op 	                                                  */
/*                  pointer to a UINT32 variable containing the sError data.  */
/*              errorTaskFile_op                                              */
/*                  pointer to the error task file                            */
/*                                                                            */
/* RETURNS      status plus the filled in data pointed to by the op pointers. */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevGetExtendedError(INT devTag_i,
						  UINT32 * sStatus_op,
						  UINT32 * sError_op,
						  SATA_TASKFILE_T * taskFile_op)
{
    STATUS_CODE_T retval1, retval2, retval3;
	retval1 = retval2 = retval3 = SATA_SUCCESS;


    /* Sanity check */
    if ( !TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES) )
	{ 
		diag_printf("Value out of range: %d", devTag_i); 
		return SATA_OUT_OF_RANGE_VALUE ;
	}
    if ( devices_gp[devTag_i] == NULL_POINTER )     return SATA_NOT_INITIALIZED;
    if ( devices_gp[devTag_i]->present == FALSE )   return SATA_NOT_INITIALIZED;

    retval1 = PortReadSStatusReg(devices_gp[devTag_i]->port, sStatus_op);
	if (retval1 != SATA_SUCCESS)
	{
		//.dh. fill in error message.
	}

    retval2 = PortReadSErrorReg(devices_gp[devTag_i]->port, sError_op);
	if (retval2 != SATA_SUCCESS)
	{
		//.dh. fill in error message.
	}

    retval3 = PortReadTaskfileParams(devices_gp[devTag_i]->port, taskFile_op);
    if(retval3 != SATA_SUCCESS)
    {
        //.dh. fill in error message.
    }


    if((retval1 | retval2 | retval3) == SATA_SUCCESS)
    {
        return SATA_SUCCESS;
    }
    else
    {
        return SATA_FAILURE;
    }
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function clears the errors on the port attached to the   */
/*              device                                                        */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      SATA_SUCCESS                                                  */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevClearError(INT devTag_i)
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;

    retval = PortClearError(devices_gp[devTag_i]->port);

    return retval;

}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function determines whether an interrupt is pending      */
/*                   for this device by calling the PortInterruptPending fcn. */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* RETURNS      TRUE / FALSE                                                  */
/*                                                                            */
/* ON ERRORS    returns FALSE on any errors (Can we do something better ?)    */
/*                                                                            */
/******************************************************************************/
boolean DevInterruptPending(INT devTag_i)
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) 
		return FALSE ;
    if(devices_gp[devTag_i] == NULL_POINTER)     
		return FALSE ;
    if(devices_gp[devTag_i]->present == FALSE)   
		return FALSE;

    /* Select the device, and then check the port */
    retval = SelectDevice(devTag_i);

    /* Return false for errors */
    if(retval != SATA_SUCCESS) return FALSE;

    return PortInterruptPending(devices_gp[devTag_i]->port);
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function determines whether a DMA interrupt is pending   */
/*                   for this device.                                         */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* RETURNS      TRUE / FALSE                                                  */
/*                                                                            */
/* ON ERRORS    returns FALSE on any errors (Can we do something better ?)    */
/*                                                                            */
/******************************************************************************/
boolean DevDmaInterruptPending(INT devTag_i)
{
    STATUS_CODE_T retval = SATA_SUCCESS;
    UINT8 dsr;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) 
		return FALSE;
    if(devices_gp[devTag_i] == NULL_POINTER)     
		return FALSE;
    if(devices_gp[devTag_i]->present == FALSE)   
		return FALSE;

    /* Select the device, and then check the port */
    retval = SelectDevice(devTag_i);
    /* Return false for errors */
    if(retval != SATA_SUCCESS) return FALSE;

    /* Check the DMA status register for a pending DMA Interrupt */
    retval = PortReadDmaStatusReg(devices_gp[devTag_i]->port, &dsr);
    if(retval != SATA_SUCCESS) return FALSE;

    if(READ_BIT(dsr, SUPDDSR_DMA_INTR_BIT)) return TRUE;

    return FALSE;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function disables the interrupts for the given drive     */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to use.                                */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevDisableInterrupts (INT devTag_i)
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) 
		return FALSE;
    if(devices_gp[devTag_i] == NULL_POINTER)     
		return FALSE;
    if(devices_gp[devTag_i]->present == FALSE)   
		return FALSE;

    /* Select the device, and then check the port */
    retval = SelectDevice(devTag_i);

    /* Return false for errors */
    if(retval != SATA_SUCCESS) return retval;

    return PortDisableInterrupts(devices_gp[devTag_i]->port);
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function enables the interrupts for the given device.    */
/*                                                                            */
/* PRECONDITIONS    port has been initialized.                                */
/*                                                                            */
/* POST-CONDITIONS  N/A                                                       */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to use.                                */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = Device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevEnableInterrupts(INT devTag_i)
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
		return FALSE;
    if(devices_gp[devTag_i] == NULL_POINTER)     
		return FALSE;
    if(devices_gp[devTag_i]->present == FALSE)   
		return FALSE;

    /* Select the device, and then check the port */
    retval = SelectDevice(devTag_i);

    /* Return false for errors */
    if(retval != SATA_SUCCESS) return retval;

    return PortEnableInterrupts(devices_gp[devTag_i]->port);
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function marks the given command slot as free.           */
/*              Currently, for debug purposes, the last completed command is  */
/*              left around in the device structure.                          */
/*                                                                            */
/* PRECONDITIONS  The device is ready for i/o, and the sataCmd_iop command    */
/*                has already been issued.                                    */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              sataCmd_iop                                                   */
/*                  The command to be processed                               */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_BUSY               = Not Ready for I/O                   */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
static STATUS_CODE_T DevEndCommand(INT devTag_i, SATA_COMMAND_T * sataCmd_iop)
{
    DEV_INFO_T     *devp       = NULL_POINTER;
    INT             portTag    = INVALID_TAG;
    SATA_COMMAND_T *cmdToFree  = NULL_POINTER;
    INT             flags      = 0;


    /* Sanity check - Could be called from the ISR */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
    if(sataCmd_iop == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }

    devp = devices_gp[devTag_i];
    portTag = devp->port;

  /* we are keep the last completed command in the system for some more time. */
  /* A debug function can then dump this information. This could help during  */
  /* the initial debug of the lib.                                            */

    MemCopy(sizeof(SATA_COMMAND_T), sataCmd_iop, &devp->lastCommand);

    /* We will clear the slot for reuse, */
    /* irrespective of the current state of the slot. */
    //OsSpinLock(&devp->cmdQLock, flags);    
    MemSet(sizeof(SATA_COMMAND_T), 0, sataCmd_iop);
    sataCmd_iop->slotState = SLOT_FREE;
    //OsSpinUnlock(&devp->cmdQLock, flags);
    /* with TCQ support, put command on free list */

    /* Record the status/error register values */
    PortReadAltStatusReg(portTag, &sataCmd_iop->status);
    PortReadSStatusReg  (portTag, &sataCmd_iop->sStatus);
    if(sataCmd_iop->txType == FIRST_PARTY_DMA)
    {
        PortReadDmaStatusReg(portTag, &(sataCmd_iop->dma.status));
    }
    PortReadErrorReg(portTag, &sataCmd_iop->error);
    PortReadSErrorReg(portTag, &sataCmd_iop->sError);

    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function will calculate how many bytes are to be         */
/*              transfered at this time, transfer the bytes (Rd or Wr),       */
/*              update the command to reflect the REMAINING data transfer.    */
/*              This function can be called in 2 contexts, either from an ISR */
/*              OR from DevProcessPioRequest. It can call DevEndCommand() to  */
/*              cleanup after the IO operation has completed.                 */
/*                                                                            */
/* PRECONDITIONS  The device is ready for i/o, and the sataCmd_iop command    */
/*                has already been issued.                                    */
/*                                                                            */
/* POST-CONDITIONS  One block of data is transfered. This block will be ONE   */
/*                  sector long if multisector is disabled, and "n" sectors   */
/*                  long if multisector transfers is set to "n" sectors. The  */
/*                  sataCmd_iop is modified to reflect the remaining part of  */
/*                  the data transfer.                                        */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              sataCmd_iop                                                   */
/*                  The command to be processed                               */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_BUSY               = Not Ready for I/O                   */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevPioMultiRdWr(INT devTag_i, SATA_COMMAND_T * sataCmd_iop)
{
    INT numBytes;
    DEV_INFO_T     * devp      = NULL_POINTER;
    STATUS_CODE_T retval       = SATA_SUCCESS;
    INT portTag                = INVALID_TAG;
    INT ctr = 0;

    /* Sanity check - Could be called from the ISR */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
    if(sataCmd_iop == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    if(sataCmd_iop->slotState != SLOT_ACTIVE)
    {
        diag_printf("Dev %d: Request to process command that's NOT Active(%d)!",
                    devTag_i, sataCmd_iop->slotState);
        return SATA_FAILURE ;
    }

    devp = devices_gp[devTag_i];
    portTag = devp->port;

    /* Make sure the drive is ready... */

    /* is the drive ready for I/O ? */
    while((DevIsReady(devTag_i) != TRUE) && (ctr < WAIT_DRQ))
    {
        ctr++;
        /* Wait for 1 Millisecond more for the drive to be ready */
        MillisecondSleep(1);
    }

    if(ctr >= WAIT_DRQ)
    {
        /* We Timed out ... */
        return SATA_BUSY;
    }

/* TBD - suppose I received a PIO write request for 1 sector. In this        */
/*       case, I would issue the command and transfer the sector immediately */
/*       Should I expect an interrupt from the controller  after this ?      */
/*       NPG - Looks like the answer is YES                                  */
/*       if yes, then I could expect to enter this function from an ISR      */
/*       with sectRemaining = 0, and only DevEndCommand needs to be done.    */
/*       and in that case, DevEndCommand should be called only in the ISR    */
/*       context.                                                            */

  /* If DRQ is set, the drive wants to transfer data, else it is telling us */
  /* That the data transfer is done. */

    if(DevDRQSet(devTag_i))
    {
        /* calculate the total number of bytes to transfer AT THIS TIME */
//        if(devp->multisector > 0)
        if(devp->devFeatures->ReadMultiple > 0)
        {
            /* Transfer a block of multiple sectors */
//            if(sataCmd_iop->sectRemaining > devp->multisector)
            if(sataCmd_iop->sectRemaining > devp->devFeatures->ReadMultiple)
            {
                /* Remaining Number of sectors is more than what we can xfer */
                /* At this time. */

//                numBytes = devp->multisector * SATA_SECTOR_SIZE;
                numBytes = devp->devFeatures->ReadMultiple * SATA_SECTOR_SIZE;
//                sataCmd_iop->sectRemaining -= devp->multisector;
                sataCmd_iop->sectRemaining -= devp->devFeatures->ReadMultiple;
            }
            else
            {
                /* Transfer all the remaining sectors at this time. */
                numBytes = sataCmd_iop->sectRemaining * SATA_SECTOR_SIZE;
                sataCmd_iop->sectRemaining = 0;
            }
        }
        else
        {
            /* Transfer One sector at a time. */ 
            /* sataCmd_iop->sectRemaining should be > 0 */
            //ASSERT(sataCmd_iop->sectRemaining > 0);

            numBytes = 1 * SATA_SECTOR_SIZE; /* 1 sector worth of data */
            sataCmd_iop->sectRemaining -= 1;
        }

        /* transfer the block of data to / from the drive */

        //ASSERT((sataCmd_iop->cmdType == SATA_WRITE) || 
          //     (sataCmd_iop->cmdType == SATA_READ));

        if(sataCmd_iop->cmdType == SATA_WRITE)
        {
            retval = PortPioWriteBytes(portTag, numBytes, sataCmd_iop->buffer);
        }
        else
        {
            retval = PortPioReadBytes(portTag, numBytes, sataCmd_iop->buffer);
        }

        /* Check for system errors on the transfer */
        if(retval != SATA_SUCCESS) return retval;

    }/* if ( DevDRQSet(devTag_i) ) */
    else
    {
        /* DRQ not set. Data Transfer done. */
        //ASSERT(sataCmd_iop->sectRemaining == 0);

        /* NPG: We will Always get an interrupt with DRQ = 0 to indicate  */
        /* completion of a Data Transfer command. */
        DevEndCommand(devTag_i, sataCmd_iop);
        return SATA_SUCCESS_COMPLETED;
    }

    return SATA_SUCCESS_NOT_COMPLETED;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function will process part of the PIO request.           */
/*              ONLY DevServiceCommandQueue is allowed to call this function. */
/*              It sets the correct Interrupt handler, issues the PIO command */
/*              and if required, transfers the first "n" sectors fo data.     */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              sataCmd_iop                                                   */
/*                  The command to be processed                               */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_BUSY               = Not Ready for I/O                   */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
static STATUS_CODE_T DevProcessPioRequest(INT devTag_i, 
                                          SATA_COMMAND_T * sataCmd_iop)
{
    INT           portTag = INVALID_TAG;
    STATUS_CODE_T retval  = SATA_SUCCESS;
    DEV_INFO_T *  devp    = NULL_POINTER;

    /* Sanity check - should not be required here... */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
    if(sataCmd_iop == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    devp = devices_gp[devTag_i];
    portTag = devp->port;

 /* We won't be checking here if the drive is ready. This was alredy done in */
 /* the DevServiceCommandQueue before we were called.                        */

 /* clear the interrupt and error status of the port before issuing a command */
    PortClearInterrupt(portTag);
    PortClearError(portTag);

    /* check to see if this is a read or a write */
    if(sataCmd_iop->cmdType == SATA_WRITE)
    {
        /* this is a write command */

        /* NOTE - The ISR needs to be set before we write the first block of */
        /*        data. We try to delay that as much as possible.            */

        /* issue the WRITE MULTIPLE or WRITE SECTOR command */
        retval = PortWriteCommandReg(portTag, sataCmd_iop->taskfile.command);
        /* Check for system errors on PortWrite */
        if(retval != SATA_SUCCESS) return retval;

        /* TBD - What state is the command in if it failed here ? */

        /* Wait for some time to allow the drive to asserts Busy */
        /* spec allows drive 400ns to assert "BUSY" */

        /* Sleep for a Microsecond */
        MicrosecondSleep(1);

        /* Set the interrupt handler for PIO transfers */
		/* TODO: INTERRUPT CONNECT HERE?  HOW DO I DO THAT??? */
    //    retval = SetISR( devp->osDriveTag, devp->pioWriteHandler );
        /* Check for system errors */
    //    if(retval != SATA_SUCCESS) return retval;

        /* Write out the first "n" sectors. */ 
        /* This function waits for the drive to be */
        /* Ready for data transfer */
        retval = DevPioMultiRdWr(devTag_i,sataCmd_iop);

        /* Check for system errors on PortWrite */
        if(retval != SATA_SUCCESS)
        {
            /* TODO: Set ISR to default ISR before returning. */
            //SetISR(devp->osDriveTag, NULL_POINTER);
            return retval;
        }
    }
    else if(sataCmd_iop->cmdType == SATA_READ)
    {
        /* this is a read command */

        /* TODO: Set the interrupt handler for PIO transfers */
      //  retval = SetISR( devp->osDriveTag, devp->pioReadHandler );
        /* Check for system errors */
      //  if(retval != SATA_SUCCESS) return retval;

        /* issue the READ MULTIPLE or READ SECTOR command */
        retval = PortWriteCommandReg(portTag, sataCmd_iop->taskfile.command);
        /* Check for system errors on PortWrite */
        if(retval != SATA_SUCCESS) return retval;
    }
    else
    {
        /* Neither Read nor Write. */
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function will process a DMA request for a device.        */
/* NOTE: Note that the SATA port does not generate an interrupt when a bus    */
/*       master operation is aborted. Software should time out.               */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              sataCmd_iop                                                   */
/*                  The command to be processed                               */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
static STATUS_CODE_T DevProcessDmaRequest(INT devTag_i, 
                                          SATA_COMMAND_T * sataCmd_iop)
{
    INT           portTag   = INVALID_TAG;
    STATUS_CODE_T retval    = SATA_SUCCESS;
    DEV_INFO_T   *devp      = NULL_POINTER;

    UINT32 ddtpr_lower32;
    UINT32 ddtpr_upper32;

    UINT16 cmd = 0x1;            /* DMA start */


    /* Sanity check - should not be required here... */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
    
    if(sataCmd_iop == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    devp = devices_gp[devTag_i];
    portTag = devp->port;

    /* Calculate the lower and upper parts of the address */
    ddtpr_upper32 = UPPER_32_BITS(sataCmd_iop->dma.ddt);
    ddtpr_lower32 = LOWER_32_BITS(sataCmd_iop->dma.ddt);

    /* clear the interrupt and error status of the port */
    PortClearInterrupt(portTag);
    PortClearError(portTag);

    /* Load the address of the DDT into the DDTPR registers */
    PortSetDTPR(portTag, ddtpr_upper32, ddtpr_lower32);

	/* Load the upper 32 bits of the addresses in the DDT entries into SUPDUDDBPR */
    PortSetDdtePage(portTag, sataCmd_iop->dma.ddtePage);

    /* write to the DMA command register to set direction, SUPDDCMDR*/
    if(sataCmd_iop->cmdType == SATA_READ)
    {
        //PortWriteDmaCommandReg(portTag, DMA_DIRECTION_DEV_TO_HOST);
		cmd = DMA_DIRECTION_DEV_TO_HOST;
    }
    else if(sataCmd_iop->cmdType == SATA_WRITE)
    {
//        PortWriteDmaCommandReg(portTag, DMA_DIRECTION_HOST_TO_DEV);
		cmd = DMA_DIRECTION_HOST_TO_DEV;
    }

    /* TODO: set the DMA interrupt handler */
  // retval = SetISR(devp->osDriveTag, devp->dmaHandler);
    /* Check for system errors */
  //  if(retval != SATA_SUCCESS) return retval;

   /* issue the READ/WRITE DMA command by writing to the taskfile command reg */
    retval = PortWriteCommandReg(portTag, sataCmd_iop->taskfile.command);
    /* Check for system errors on PortWrite */
    if(retval != SATA_SUCCESS)
    {
        /* TODO: Set ISR to default ISR before returning. */
        // SetISR(devp->osDriveTag, NULL_POINTER);
        return retval;
    }
 
	
	/* start the DMA engine by writing to the DMA command register, SUPDDCMDR */
	devices_gp[devTag_i]->waiting_for_dma = TRUE;
	retval = PortWriteDmaCommandReg(portTag, (UINT16)(DMA_START | cmd));    /* Check for system errors on PortWrite */
	
	if(retval != SATA_SUCCESS)
    {
        /* TODO Set ISR to default ISR before returning. */
        // SetISR( devp->osDriveTag, NULL_POINTER );
		devices_gp[devTag_i]->waiting_for_dma = FALSE;
        return retval;
    }

    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function sends the IDENTIFY DEVICE command to the drive  */
/*              and reads the 512 bytes of information returned by the drive. */
/*              This information is returned to the caller in buffer_op and   */
/*              a copy is saved for debug information.                        */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              buffer_op                                                     */
/*                  Pointer to a buffer of size 512 bytes to hold the         */
/*                  the reply packet.                                         */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              SATA_FAILURE            = drive not present                   */
/*              SATA_BUSY               = Timed out                           */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevIdentify(INT devTag_i, void * buffer_op)
{
    DEV_INFO_T     * devp      = NULL_POINTER;
    STATUS_CODE_T retval       = SATA_SUCCESS;
    INT portTag                = INVALID_TAG;
    INT ctr = 0;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) return SATA_OUT_OF_RANGE_VALUE;
    if(devices_gp[devTag_i] == NULL_POINTER)     
	{
		diag_printf ("devices_gp[devTag_i] == NULL_POINTER\n");
		return SATA_NOT_INITIALIZED;
	}
    if(devices_gp[devTag_i]->present == FALSE)   
	{
		diag_printf ("devices_gp[devTag_i]->present == FALSE\n");
		return SATA_NOT_INITIALIZED;
	}
    if(buffer_op == NULL_POINTER)                return SATA_OUT_OF_RANGE_VALUE;



    devp = devices_gp[devTag_i];
    portTag = devp->port;

    /* Check if drive is present */
    if(PortDevConnected(portTag) != TRUE)
    {
        /* Drive not connected to port ?!! This should not be happening !!!*/
        return SATA_FAILURE;
    }

    /* Disable Interrupts */
    retval = PortDisableInterrupts(portTag);
    /* Check for system errors */
    if(retval != SATA_SUCCESS) 
	{
		diag_printf ("PortDisableInterrupts returned %d\n", retval);
		return retval;
	}
    /* Write the ATA_IDENTIFY_DEVICE command */
    retval = PortWriteCommandReg(portTag, ATA_IDENTIFY_DEVICE);
    /* Check for system errors */
    if(retval != SATA_SUCCESS) 
	{
		diag_printf ("PortWriteCommandReg returned %d\n", retval);
		return retval;
	}

    MicrosecondSleep(1); /* Give drive 400 nsec to set the busy flag */

    /* Wait for some time Max = WAIT_WORSTCASE, if BUSY bit set */
    /* Make sure the drive is ready... */
    /* is the drive ready for I/O ? (not Busy?) */
    while((DevIsReady(devTag_i) != TRUE) && (ctr < WAIT_WORSTCASE))
    {
        ctr++;
        /* Wait for 1 Millisecond more for the drive to be ready */
        MillisecondSleep(1);
    }

    if(ctr >= WAIT_WORSTCASE)
    {
        return SATA_BUSY;
    }

    /* Check for DRQ, (wait if required)*/
    if(DevDRQSet(devTag_i) == FALSE)
    {
        /* wait 50 ms */
        MillisecondSleep(50);
        if(DevDRQSet(devTag_i) == FALSE)
        {
            return SATA_BUSY; /* We timed out... */
        }
    }

    /* Read PIO data, 512 bytes */
    retval = PortPioReadBytes(portTag, SATA_SECTOR_SIZE , buffer_op);
 
	/* Check for system errors */
    if(retval != SATA_SUCCESS) return retval;

	/* save the buffer in the devp structure */
    MemCopy(SATA_SECTOR_SIZE, buffer_op, &(devp->devIdentify));

    /* Fix the model, f/w rev, serial no strings */
    FixString(&devp->devIdentify.modelNumber[0], 
              sizeof(devp->devIdentify.modelNumber));
    FixString(&devp->devIdentify.firmwareRevision[0], 
              sizeof(devp->devIdentify.firmwareRevision));
    FixString(&devp->devIdentify.serialNumber[0], 
              sizeof(devp->devIdentify.serialNumber));

    /* What do we do of errors at this point ? (not that we expect any) */
    /* clear the drve interrupts. */
    PortClearInterrupt(portTag);

    /* Enable Interrupts */
    PortEnableInterrupts(portTag);

    /* Done. */
    return SATA_SUCCESS;
}


static void FixString (char *start, const int len)
{
    char *ptr = start;
    char *end, *ptr2 ;
	char temp;

    /* Even length for byte swapping */
    end = (len % 2) ? &start[len]: &start[len-1];

    ptr = start;

    /* strip leading space */
    while(ptr != end && *ptr == ' ')
    {
        ptr++;
    }

    ptr2 = start;

    /* ptr2 is at the start of the string, and ptr is at the start of*/
    /* the non-balnk part */

    /* compress internal blanks and strip trailing space */
    while((ptr != end) && (*ptr) != '\0')
    {
        if(*ptr != ' ')
        {
            *ptr2 = *ptr;
            ptr2++;
        }
        ptr++;
    }

    /* ptr is either at the end of the string, or a null.*/
    /* ptr2 is at the end of the string we copied */

    /* wipe out trailing garbage */
    while(ptr2 != end)
    {
        *ptr2++ = '\0';
    }

	/* now we need to swap the bytes in our string... */
	ptr = start;
	ptr2 = start+1;
	
	while((ptr != end) && (*ptr) != '\0')
	{
		temp = *ptr;
		*ptr = *ptr2;
		*ptr2 = temp;

		ptr+=2;
		ptr2+=2;
	}

}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function is called by the OSDM ISR to complete the data  */
/*              transfers. This function determines which command for devTag_i*/
/*              drive caused the interrupt (without TCQ, it will be the 1st   */
/*              command in the submittedQueue). For DMA transfers, It will    */
/*              check for errors, remove the command from the submitted       */
/*              commands queue, end the DMA transfer and return an appropriate*/
/*              status value. For PIO transfers, it will transfer the correct */
/*              number of bytes. If this transfer ends the ATA command, it    */
/*              will remove the command from the command queue and return     */
/*              the appropriate status. If this transfer doesn't end the ATA  */
/*              command, it will return SATA_SUCCESS_NOT_COMPLETED.           */
/*                                                                            */
/* PRECONDITIONS  In interrupt space                                          */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      SATA_SUCCESS_COMPLETED on success, error code on failure      */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_SUCCESS_NOT_COMPLETED = Part of ATA command completed.   */
/*                                           Command still in the Queue       */
/*              SATA_SUCCESS_COMPLETED  = ATA command completed, removed from */
/*                                        Command Queue.                      */
/*              SATA_FAILURE            = ATA Protocol level error (StatusReg)*/
/*              SATA_BUSY               = Device not Ready or device did not  */
/*                                        issue the Interrupt                 */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag / other value           */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevIsr ( INT devTag_i )
{
    STATUS_CODE_T retval       = SATA_SUCCESS;
    DEV_INFO_T     * devp      = NULL_POINTER;
    INT portTag                = INVALID_TAG;
    SATA_COMMAND_T * currCmd   = NULL_POINTER;
    UINT8 dmaStatReg;
    UINT8 statReg;
    INT controller;
    SATA_REG_ADDR_T base;
    UINT32 ipr;

#ifdef SATA_DEBUG
    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) return SATA_OUT_OF_RANGE_VALUE;
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devices_gp[devTag_i]->present == FALSE)   return SATA_NOT_INITIALIZED;
#endif

    devp = devices_gp[devTag_i];
    portTag = devp->port;

    /* Is the device Ready ? */
    if(DevIsReady(devTag_i) != TRUE)
    {
        /* Give the drive some time to clear the busy bit */
        MicrosecondSleep(1);
        if(DevIsReady(devTag_i) != TRUE) return SATA_BUSY;
    }


  /* Algo: This function will try to determine what caused the interrupt.     */
  /* If we think that a data transfer did not cause the interrupt, we will    */
  /* call the NonDataIsr function, which will handle all non-data interrupts  */
  /* and error interrupts                                                     */
  /* NPG: TBD: Check for errors on port Read/Write function calls             */

  /* TBD: Is there a sure-shot way to determine if the interrupt is not for a */
  /* Data Transfer ?                                                          */
    
    //ASSERT(ports_gp[portTag]);
    controller = ports_gp[portTag]->controller;
    
    //ASSERT(controllers_gp[controller]);
    base = controllers_gp[controller]->commonRegBase;

    /* Read the register */
    PciMemRead32(base + SUPDIPR, &ipr);

    /* Move the interrupt bits relevant to this port in lower byte */
    ipr >>= SUPDIPR_BITS_PER_PORT * ports_gp[portTag]->portNumber;
    /* Mask off all bits not applicable to this port */
    ipr &=  SUPDIPR_BITS_PER_PORT_MASK;


    /* if it's IDE interrupt */
    if(!(ipr & 0x7f))
    {
        /* Clear the interrupt */
        PortClearInterrupt(portTag);

        /* Determine which command caused the Interrupt...               */
        /* without TCQ, it's the 1st command in the commandQueue */
        currCmd = &devp->commandQueue[0];

        /* Figure out if it was a PIO command, */
        /* a DMA command or is this something else ? */
        if(currCmd->txType == FIRST_PARTY_DMA)
        {

#if 0 // If it's an error, there wouldn't be a interrupt

            /* First verify that the interrupt has been issued by the DMA engine */
            PortReadDmaStatusReg (portTag, &dmaStatReg);
            if(READ_BIT(dmaStatReg, SUPDDSR_DMA_INTR_BIT) != 1)
            {
                /* DMA did not cause the interrupt... Might be a non data command */
                /* Or an error */
				diag_printf ("Not a DMA Interrupt, calling NonDataIsr\n");
                return NonDataIsr(devTag_i);
            }
#endif

            /* OK, so a DMA caused the interrupt. First, let's stop the DMA */

            /* stop the DMA engine by writing to the DMA command register, SUPDDCMDR */
            retval = PortWriteDmaCommandReg(portTag, DMA_STOP);

            /* End the command */
            DevEndCommand(devTag_i, currCmd);

#if 0 // no sense to check since if it's completed it's not an error


            /* Check if the DMA completed successfully.  */
         /* NOTE: The SATA port does not generate an interrupt when a bus master */
         /* operation is aborted.                                                */
            PortReadAltStatusReg(portTag,&statReg);

            if((READ_BIT(dmaStatReg, SUPDDSR_DMA_ERROR_BIT) == 1) ||
               (READ_BIT(statReg, SUPDASR_ERR_BIT) == 1))
            {
                /* Error Bit is set. Report Protocol Error */
                return SATA_FAILURE;
            }
#endif

            return SATA_SUCCESS_COMPLETED;
        }
        else if(currCmd->txType == PIO)
        {
            /* We received an interrupt for a PIO command */
            return DevPioMultiRdWr(devTag_i, currCmd);
        }
        else
        {
            // third party DMA not supported
            /* Non data commands should not be queued... */
            diag_printf("SATA Command with wrong Transfer type (%d)", 
                          currCmd->txType);
            return SATA_OUT_OF_RANGE_VALUE;

        }
    } /* end if(no error) */
    else if(ipr > 0)
    {
        /* No command submitted. Is this an interrupt from Non-data command or */
        /* Was there an error ? */
      
		retval=NonDataIsr(devTag_i, ipr);

        /* Clear the interrupt */
        PortClearInterrupt(portTag);


        return retval;
    }
    else
    {
        /* None of the interrupt bits were set */
        return SATA_SUCCESS;
    }
    
    /* Should not Reach here !!! */
    return SATA_FAILURE;
}



/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function handles all the non-data interrupts when        */
/*              devIsr() function is called. This includes hotplug events.    */
/*                                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to check.                              */
/*              ipr                                                           */
/*                  Interrupts asserted for processing                        */
/*                                                                            */
/* RETURNS      SATA_HOTPLUG_EVENT if hotplug event                           */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
static STATUS_CODE_T NonDataIsr(INT devTag_i, UINT32 ipr)
{

    DEV_INFO_T     * devp      = NULL_POINTER;
    INT portTag                = INVALID_TAG;
    SATA_COMMAND_T * currCmd   = NULL_POINTER;
    STATUS_CODE_T retval       = SATA_SUCCESS;


#ifdef SATA_DEBUG


    /* Sanity check - should not be required here... */
    if (!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if (devices_gp[devTag_i] == NULL_POINTER)
    {
        return SATA_NOT_INITIALIZED;
    }
    if (devices_gp[devTag_i]->active == FALSE)
    {
        return SATA_NOT_INITIALIZED;
    }
#endif

    devp = devices_gp[devTag_i];
    portTag = devp->port;


    /* Looks like the ISR routine was called, but there is no submitted command */
    /* Check to verify that THIS device actually generated the interrupt */

    // something happened here....
    /* NPG: TBD: Non data commands not yet implemented */

    // PHY change, device hot-plug
    if (READ_BIT(ipr, SUPDIPR_STATE_INTR_BIT) == 1)
    {
        if (devp->present)    // Hot Unplug Event
        {
            // Stop DMA, end commands
            retval = PortWriteDmaCommandReg(portTag, DMA_STOP);

            currCmd = &devp->commandQueue[0];

            /* End the command */
            DevEndCommand(devTag_i, currCmd);

            devp->active = FALSE;
            devp->present = FALSE;
        }
        else
        {                       // Hotplug Event
            devp->present = TRUE;

        }

        return SATA_HOTPLUG_EVENT;
    }
    else
    {
     //   diag_printf("Non Data INT on dev %d. Error ?", devTag_i);
        return SATA_FAILURE;
    }

    return SATA_FAILURE;
}



/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function verbose dumps the contents of Alt Status reg,   */
/*              Err Reg, Super Set Status reg, Super set Err reg to the       */
/*              provided buffer.                                              */
/*                                                                            */
/*                                                                            */
/* PARAMETERS   devTag_i                                                      */
/*                  Tag for the device to check.                              */
/*              limit_i                                                       */
/*                  size of buffer_op                                         */
/*              buffer_ip                                                     */
/*                  buffer to hold the dump                                   */
/*                                                                            */
/* RETURNS      Number of bytes written to the buffer                         */
/*                                                                            */
/******************************************************************************/
INT DumpErrStat(int devTag_i, INT limit_i, void * buffer_op)
{
    DEV_INFO_T     * devp    = NULL_POINTER;
    PORT_INFO_T    * portp   = NULL_POINTER;
    INT              portTag = INVALID_TAG;

    char          * out = (char *) buffer_op;
    INT             len = 0;
    UINT8           data8;
    UINT32          data32;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(devices_gp[devTag_i] == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(buffer_op == NULL_POINTER)
    {
        diag_printf("Value out of range: %d", devTag_i); 
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* If less than a line's length is left, truncate dump and return */
    if(limit_i - len < 80)  return len;

    devp = devices_gp[devTag_i];
    portTag = devp->port;
    portp = ports_gp[portTag];

    len += sdiag_printf(out, "Status at the end of \"last\" command: 0x%02X\n", 
                   portp->lastSR);
    if(limit_i - len < 80)  return len;

    PortReadAltStatusReg(portTag, &data8);
    out = (char *) buffer_op + len;
    len += sdiag_printf(out, "AltStatus : 0x%02X\t\tlast: 0x%02X\n", data8, 
                   portp->lastASR);
    if(limit_i - len < 80)  return len;

    PortReadErrorReg(portTag, &data8);
    out = (char *) buffer_op + len;
    len += sdiag_printf(out, "Error Reg : 0x%02X\t\tlast: 0x%02X\n", data8, 
                   portp->lastER);
    if(limit_i - len < 80)  return len;

    PortReadSStatusReg(portTag, &data32);
    out = (char *) buffer_op + len;
    len += sdiag_printf(out, "SStatus   : 0x%08X\tlast: 0x%08X\n", data32, 
                   portp->lastSSSR);
    if(limit_i - len < 80)  return len;

    PortReadSErrorReg(portTag, &data32);
    out = (char *) buffer_op + len;
    len += sdiag_printf(out, "SError    : 0x%08X\tlast: 0x%08X\n", data32, 
                   portp->lastSSER);
    if(limit_i - len < 80)  return len;

    out = (char *) buffer_op + len;
    len += sdiag_printf(out, "All SErrors seen: 0x%08X\tlast: 0x%08X\n\n", 
                   data32, portp->cumulativeSSER);
    if(limit_i - len < 80)  return len;

    data32 = 0xffffffff;
    PciMemRead32( ports_gp[portTag]->portRegBase + SUPDSSSR, &data32);

    out = (char *) buffer_op + len;
/* 0 = no device, 1 = Present but NO communication, 3 = Ready, 4 = offline */
    len += sdiag_printf(out, "Dev is %s%s%s%s%s%s\n",
                   ((data32 & SUPDSSSR_DEV_DET_MASK)==0) ? 
                       "(0)Absent":"",
                   ((data32 & SUPDSSSR_DEV_DET_MASK)==1) ? 
                       "(1)Present but NO communication":"",
                   ((data32 & SUPDSSSR_DEV_DET_MASK)==2) ? 
                       "in (2)Unknown State":"",
                   ((data32 & SUPDSSSR_DEV_DET_MASK)==3) ? 
                       "(3)Ready":"",
                   ((data32 & SUPDSSSR_DEV_DET_MASK)==4) ? 
                       "(4)offline":"",
                   ((data32 & SUPDSSSR_DEV_DET_MASK) >4) ? 
                       "in Unknown State":"");
    return len;
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function sets the pointer to the device features passed  */
/*              in from the OSDM to a per device pointer. The features will   */
/*              be used by the library to determine certain behaviors of the  */
/*              commands depending on what is set. This MUST be called after  */
/*              the OS sets the features for the drive. If features are not   */
/*              changed on the drive this still needs to be called in order   */
/*              for the library to know what features are currently set.      */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*              The device features table must be filled out and valid!       */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*              features_p                                                    */
/*                  Pointer to the device features table                      */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
/*INLINE*/
STATUS_CODE_T DevSetFeatures(INT devTag_i, PSATA_FEATURES_T features_p)
{
    DEV_INFO_T     *devp      = devices_gp[devTag_i];
    STATUS_CODE_T retval      = SATA_SUCCESS;

   
    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) return SATA_OUT_OF_RANGE_VALUE;
    if(devp == NULL_POINTER)     return SATA_NOT_INITIALIZED;
    if(devp->present == FALSE)   return SATA_NOT_INITIALIZED;

    if(features_p)
    {
        devp->devFeatures = features_p;
    }
    else
    {
        return SATA_NOT_INITIALIZED;
    }

    return SATA_SUCCESS;
} /* End of DevSetFeatures() */

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function unsets the pointer to the device features       */
/*              This function should be called if device is removed when hot  */
/*              plug is being used.                                           */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              devTag_i                                                      */
/*                  Tag for the device to be used.                            */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
/*INLINE*/ 
STATUS_CODE_T DevUnsetFeatures(INT devTag_i)
{
    DEV_INFO_T     *devp      = devices_gp[devTag_i];
    STATUS_CODE_T retval      = SATA_SUCCESS;

  
    /* Sanity check */
    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES)) 
		return SATA_OUT_OF_RANGE_VALUE;
    if(devp == NULL_POINTER)     
		return SATA_NOT_INITIALIZED;
    if(devp->present == FALSE)   
		return SATA_NOT_INITIALIZED;

    devp->devFeatures = NULL_POINTER;

    return SATA_SUCCESS;
} /* End of DevUnsetFeatures() */

