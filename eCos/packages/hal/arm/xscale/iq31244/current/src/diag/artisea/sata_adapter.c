/************************************************************************;
;*
;*      Filename:  sata_adapter.c								
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

#include <sata_types.h>
#include <sata_lib.h>
#include <sata_adapter.h>
#include <sata_globals.h>


/******************************************************************************/
/*                                                                            */
/*            L O C A L   F U N C T I O N    P R O T O T Y P E S              */
/*                                                                            */
/******************************************************************************/

static INT DriveNumToPort(INT adapterNumber_i, INT deviceNumber_i);

/******************************************************************************/
/*                                                                            */
/*       G L O B A L   F U N C T I O N   I M P L E M E N T A T I O N S        */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function allocates and initializes the memory structures */
/*              internal to the library. This function should be called before*/
/*              any other function in the library is used.                    */
/*                                                                            */
/* PRECONDITIONS    adapters_gp array has been allocated.                     */
/*                                                                            */
/* POST-CONDITIONS  The next NULL pointer in adapters_gp is set.              */
/*                  *adapterTag_op is set to it's index number/INVALID_TAG    */
/*                                                                            */
/* PARAMETERS   osdmTag_i                                                     */
/*                  The Tag used by OSDM to identify this adapter.            */
/*              adapterTag_op                                                 */
/*                  Address of the INT variable to hold the adapterTag.       */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in controllers_gp array    */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/

#if 0
STATUS_CODE_T InitLib()
{
    INT ctr;
    static boolean alreadyInitialized = FALSE;

    /* initialize the debug counter value to zero */
    sataDebugCounter = 0;

    if(alreadyInitialized == TRUE)
    {
        return SATA_SUCCESS;
    }

    /* set all the elements in the global pointer array to NULL_POINTERs   */
    /* We do it this way rather than using memset so that if required, the */
    /* value of NULL_POINTER can be changed                                */

    for(ctr = 0; ctr < MAX_ADAPTERS; ctr++)
    {
        adapters_gp[ctr] = NULL_POINTER;
    }

    for(ctr = 0; ctr < MAX_CONTROLLERS; ctr++)
    {
        controllers_gp[ctr] = NULL_POINTER;
    }

    for(ctr = 0; ctr < MAX_PORTS; ctr++)
    {
        ports_gp[ctr] = NULL_POINTER;
    }

    for(ctr = 0; ctr < MAX_DEVICES; ctr++)
    {
        devices_gp[ctr] = NULL_POINTER;
    }

    /* setting all features to False */
    MemSet(sizeof(SATA_FEATURES_T), 0, &devFeatures_g);

    alreadyInitialized = TRUE;

    return SATA_SUCCESS;

}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function allocates the private memory for the adapter.   */
/*              it allocates a ADAPTER_INFO_T structure and records the       */
/*              pointer in the next available slot in the adapters_gp array   */
/*              This function also initializes the allocated memory structure */
/*              and sets the osdmData member to the passed osdmTag_i.         */
/*                                                                            */
/* PRECONDITIONS    adapters_gp array has been allocated.                     */
/*                                                                            */
/* POST-CONDITIONS  The next NULL pointer in adapters_gp is set.              */
/*                  *adapterTag_op is set to it's index number/INVALID_TAG    */
/*                                                                            */
/* PARAMETERS   osdmTag_i                                                     */
/*                  The Tag used by OSDM to identify this adapter.            */
/*              adapterTag_op                                                 */
/*                  Address of the INT variable to hold the adapterTag.       */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in controllers_gp array    */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/

STATUS_CODE_T AdapterAllocatePrivateMemory (INT osdmTag_i, INT * adapterTag_op)
{
    INT tag;
    ADAPTER_INFO_T *newAdapterInfo;

    *adapterTag_op = INVALID_TAG ; /* Will be assigned a valid value below */

    tag = getNextSlot((void**)adapters_gp, MAX_ADAPTERS); /* NULL_POINTER safe */

    /* Check if we got an empty slot in the array */
    if(tag == INVALID_TAG)
    {
        return SATA_OUT_OF_RANGE_VALUE;
    }

    /* Allocate the Memory */
    newAdapterInfo = GetMemory(sizeof(ADAPTER_INFO_T));

    if(newAdapterInfo == NULL_POINTER)
    {
        /* Failed to get memory */
        return SATA_GET_MEMORY_FAILED;
    }

    /* Record the pointer in the global array */
    adapters_gp[tag] = (ADAPTER_INFO_T *) newAdapterInfo;
    *adapterTag_op = tag ;

    /* Initialize the structure */
    MemSet(sizeof(ADAPTER_INFO_T),0,newAdapterInfo);
    newAdapterInfo->present = FALSE;  /* Mark as not yet completely initialized */
    newAdapterInfo->osdmTag = osdmTag_i;

    return SATA_SUCCESS;

}


#endif

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function initializes the adapter.                        */
/*                                                                            */
/* PRECONDITIONS    The private memory for the adapter has been allocated.    */
/*                                                                            */
/* POST-CONDITIONS  The adapter, the controller and all the attached ports    */
/*                  are initialized and ready for I/O                         */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be initialize.                     */
/*              baseAddr_i                                                    */
/*                  The base address to the registers on the single           */
/*                  controller present on this adapter.                       */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in controllers_gp array    */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*              SATA_NOT_INITIALIZED    = Memory was not allocated.           */
/*              SATA_COMPLETED_WITH_ERRORS = a device init failed             */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterInit (INT adapterTag_i, SATA_REG_ADDR_T baseAddr_i)
{
    INT currController, currPort, currDev, currDriveIndex;
    INT numControllers;  /* Number of controllers on this adapter */
    INT ctr, ctr2, ctr3; /* counters used in a for loop */

    ADAPTER_INFO_T *adapterp = NULL_POINTER;

    STATUS_CODE_T retval = SATA_SUCCESS;
    boolean deviceInitFailed = FALSE;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    adapterp = adapters_gp[adapterTag_i];

    /* Detect how many controllers we have - hard coded to One controller Only for now.*/
    numControllers = MAX_CONTROLLERS_PER_ADAPTER;

    /* Let's perform similar operations together rather than performing */
    /* all operations for a controller at one go. Helps in Debug */

    /* Ask all the controllers to allocate their private memory */
    for(ctr = 0; ctr < numControllers; ctr++)
    {
        
        //adapterp->controller[ctr] = ctr;
		adapterp->controller[ctr] = MAX_CONTROLLERS_PER_ADAPTER * adapterTag_i + ctr;

        currController = adapterp->controller[ctr];
        retval = ControllerInit(currController, ctr, baseAddr_i, adapterTag_i);

        if(retval == SATA_COMPLETED_WITH_ERRORS)
        {
            /* SATA_COMPLETED_WITH_ERRORS implies that a device init failed */
            /* Non fatal error */
            deviceInitFailed = TRUE;
        }
        else if(retval != SATA_SUCCESS)
        {
            /* Treat controller / port failures as fatal.  */
            return retval;
        }
    }


    if(deviceInitFailed == TRUE)
    {
        /* One or more devices failed during initialization */
        diag_printf("Adapter %d: One or more devices failed during initialization", adapterTag_i);
        return SATA_COMPLETED_WITH_ERRORS;
    }

    /* Collect the list of drives (devTags) downstream to this adapter. This algo will */
    /* have to change to support RSMs */

    /* But first, initialise the array with all INVALID_TAGs */
    for(ctr = 0; ctr < MAX_DEVICES_PER_ADAPTER; ctr++)
    {
        adapterp->drives[ctr] = INVALID_TAG;
    }

    currDriveIndex = 0;

    for(ctr = 0; ctr < numControllers; ctr++)
    {
        currController = adapterp->controller[ctr];

        /* INVALID_TAG indicates that we have looked at all the valid controllers */
        if(currController == INVALID_TAG) break;
        if(controllers_gp[currController] == NULL_POINTER) break;

        for(ctr2 = 0; ctr2 < MAX_PORTS_PER_CONTROLLER ; ctr2++)
        {
            currPort = controllers_gp[currController]->port[ctr2];

            /* INVALID_TAG indicates that we have looked at all the valid ports */
            if(currPort == INVALID_TAG) break;
            if(ports_gp[currPort] == NULL_POINTER) break;

            for(ctr3 = 0; ctr3 < MAX_DEVICES_PER_PORT; ctr3++)
            {
                /* collect all the devTags */
                currDev = ports_gp[currPort]->dev[ctr3];

                /* INVALID_TAG indicates that we have looked at all the valid devices */
                if(currDev == INVALID_TAG) break;
                if(devices_gp[currDev] == NULL_POINTER) break;

                /* Check if the device is a drive that can have command queues. */
                if (!(devices_gp[currDev]->present))
                    break; /* not initialized OR not a drive */

                adapterp->drives[currDriveIndex] = currDev;  /* record the driveTag */
                currDriveIndex++;
            }
        }
    }

    diag_printf("  %d drives found! ", currDriveIndex);

    /* start from drive 0 next time... */
    adapterp->driveLastServiced = MAX_DEVICES_PER_ADAPTER - 1;

    /* Initialization complete. Mark controller as ready for use. */
    adapterp->present = TRUE;

    /* Done with adapter, controller & port initializations */

    /* Successfully initialized. Return success */
    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function accepts as drive number (0 to n) that might be  */
/*              connected to this adapter. It uses this number to locate the  */
/*              right port the drive should be attached to. It then checks    */
/*              the port status to determine if any drive is connected to the */
/*              port. This function will have to change to support more than  */
/*              one drive per port.                                           */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*              deviceNumber_i                                                */
/*                  the drive number (0 to n) that might be connected to a    */
/*                  port on a controller on this adapter.                     */
/*                                                                            */
/* RETURNS      SATA_TRUE on success, error code on failure                   */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE, SATA_NOT_INITIALIZED, SATA_FALSE     */
/******************************************************************************/
STATUS_CODE_T AdapterDevicePresent(INT adapterTag_i, INT deviceNumber_i,
                                   INT * devTag_op)
{
    INT portTag;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    /* check if the deviceNumber_i is within a valid range */
    if(deviceNumber_i < 0 || deviceNumber_i > MAX_DEVICES)
    {
        /* Report out of range error */
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* The adapterTag gives us the adapter number. We are looking for the portTag*/
    /* corresponding to the nth drive connected to this adapter */
    portTag = DriveNumToPort(adapterTag_i, deviceNumber_i);

    /* Which Device on this port ? */
    if(devTag_op != NULL_POINTER)
    {
        *devTag_op = ports_gp[portTag]->dev[deviceNumber_i % MAX_DEVICES_PER_PORT];
    }

    /* Assumes that if the port is connected, the drive is present. This will */
    /* change when RSMs are used. (RSMs are Not yet supported) */
    if (PortDevConnected(portTag) == TRUE)
    {
        if ((devTag_op != NULL_POINTER) && (*devTag_op == INVALID_TAG))
        {
            diag_printf("ERROR: portTag %d, Internal Error - Device Present, but Invalid devTag!",portTag);
            diag_printf("ports_gp[%d]->dev[%d mod %d] = %d",(int)portTag, (int)deviceNumber_i, (int)MAX_DEVICES_PER_PORT,(int) *devTag_op);
            return SATA_FALSE;
        }
        return SATA_TRUE;
    }
    else
    {
        return SATA_FALSE;
    }
}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function accepts as drive number (0 to n) that might be  */
/*              connected to this adapter. It uses this number to locate the  */
/*              right port the drive should be attached to. It then sends the */
/*              IDENTIFY DEVICE command to the drive and reads the 512 bytes  */
/*              of information returned by the drive. This information is     */
/*              returned to the caller in buffer_op and a copy is saved for   */
/*              debug information. The Drive will NOT generate an interrupt   */
/*              during this operation.                                        */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*              deviceNumber_i                                                */
/*                  the drive number (0 to n) that might be connected to a    */
/*                  port on a controller on this adapter.                     */
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
STATUS_CODE_T AdapterDeviceIdentify(INT adapterTag_i, INT deviceNumber_i,
                                    void * buffer_op)
{
    INT portTag, devTag;
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
    {
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(adapters_gp[adapterTag_i] == NULL_POINTER)
    {
        return SATA_NOT_INITIALIZED;
    }

    /* check if the deviceNumber_i is within a valid range */
    if((deviceNumber_i < 0) || (deviceNumber_i > MAX_DEVICES))
    {
        /* Report out of range error */
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* The adapterTag gives us the adapter number. We are looking for the portTag*/
    /* corresponding to the nth drive connected to this adapter */
    portTag = DriveNumToPort(adapterTag_i, deviceNumber_i);

    /* Which Device on this port ? */
    devTag = ports_gp[portTag]->dev[ deviceNumber_i % MAX_DEVICES_PER_PORT];

    SelectDevice(devTag);
    retval = DevIdentify( devTag, buffer_op);

    PortClearInterrupt(portTag);

    return retval;

}

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION  This function establishes the 1:1 relation between the OS's   */
/*              view of the hard disks and the internal representation in the */
/*              SATA library.                                                 */
/*                                                                            */
/* PRECONDITIONS    The ports have been initialized.                          */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*              deviceNumber_i                                                */
/*                  the drive number (0 to n) that might be connected to a    */
/*                  port on a controller on this adapter.                     */
/*              The next Three parameters are have OS dependent values that   */
/*              are used passed on as parameters to other System dependent    */
/*              service calls.                                                */
/*              OsDriveTag_ip                                                 */
/*                  The OS's Drive Tag. To be returned when requested.        */
/*              DmaHandler_ip                                                 */
/*                  OS specific Dma Handler's address.                        */
/*              PioReadeHandler_ip                                            */
/*                  OS specific PIO Handler's address.                        */
/*              PioWriteHandler_ip                                            */
/*                  OS specific PIO Handler's address.                        */
/*              devTag_op                                                     */
/*                  The address of the variable to hold the devTag            */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE, SATA_NOT_INITIALIZED                 */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterAttachDisk(
                               INT adapterTag_i,      /* Tag for the adapter to be used.                */
                               INT driveNumber_i,     /* the drive number (0 to n) on Adapter           */
                               void * OsDriveTag_ip,  /* The OS's Drive Tag.                            */
                               void * DmaHandler_ip,  /* OS specific Dma Handler's address              */
                               void * PioReadHandler_ip,  /* OS specific PIO Handler's address         */
                               void * PioWriteHandler_ip,  /* OS specific PIO Handler's address         */
                               INT * devTag_op        /* The address of the variable to hold the devTag */
                               )
{
    INT portTag, driveNum, devTag;
    DEV_INFO_T *curDrive;

    *devTag_op = INVALID_TAG; /* Will be set to correct value below */

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    /* check if the deviceNumber_i is within a valid range */
    if(driveNumber_i < 0 || driveNumber_i > MAX_DEVICES)
    {
        /* Report out of range error */
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* The adapterTag gives us the adapter number. We are looking for the portTag*/
    /* corresponding to the nth drive connected to this adapter */
    portTag = DriveNumToPort(adapterTag_i, driveNumber_i);

    /* Which drive on this port ? (currently only One drive per port)*/
    driveNum = driveNumber_i % MAX_DEVICES_PER_PORT;

    /* Check the portTag and drive number we calculated. */
    /* There is no reason why we will ever get invalid values here !!! */
    if(!TAG_WITHIN_RANGE(portTag, MAX_PORTS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(ports_gp[portTag] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    /* Now, let's get the devTag for this drive from the portInfo*/
    devTag = ports_gp[portTag]->dev[driveNum];

    /* Check the devTag we calculated... */
    /* There is no reason why we will ever get invalid values here !!! */
    if(!TAG_WITHIN_RANGE(devTag, MAX_DEVICES))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(devices_gp[devTag] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    /* And finally, here is the pointer to the correct devInfo... */
    curDrive = devices_gp[devTag];

    /* Let's set the devTag. */
    *devTag_op = devTag;

    /* Now, let's record all the parameters in the devInfo */
    curDrive->osDriveTag      = OsDriveTag_ip;
    curDrive->dmaHandler      = DmaHandler_ip;
    curDrive->pioReadHandler  = PioReadHandler_ip;
    curDrive->pioWriteHandler = PioWriteHandler_ip;

    /* all done. return success */
    return SATA_SUCCESS;

}
/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function adds the I/O command to the commandQueue of the */
/*              specified drive. This function accepts a sector range for     */
/*              transfer and computes the taskfile contents baed on this      */
/*              information. Currently ONLY LBA28 is supported                */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*              devTag_i                                                      */
/*                  The devTag of the target drive.                           */
/*              startSector_i                                                 */
/*                  Starting sector number for the transfer. (LBA28 Only)     */
/*                  LBA48 supported, actually LBA32                           */
/*              numSectors_i                                                  */
/*                  Number of sectors to transfer (Max 256 in LBA28).         */
/*                  Max 65536 in LBA48                                        */
/*              cmd_i                                                         */
/*                  Type of command (Read / Write)                            */
/*              mode_i                                                        */
/*                  Requested mode of transfer ( DMA / PIO )                  */
/*              ddt_i                                                         */
/*                  Bus address of the DMA Descriptor Table.                  */
/*                  Set to 0 for PIO mode of data transfer.                   */
/*              ddtePage_i                                                    */
/*                  Top 32 bits of the DMA Data address in DDT.               */
/*                  Set to 0 for PIO mode of data transfer.                   */
/*              buffer_iop                                                    */
/*                  Virtual Address of the source/target buffer.              */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE, SATA_NOT_INITIALIZED                 */
/*              SATA_BUSY = No unused command slots found. Try again after a  */
/*                          command completes                                 */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterAddCommandToQueue(
                                      INT adapterTag_i,           /* Tag for the adapter to be used             */
                                      INT devTag_i,               /* The devTag of the target drive.            */
                                      INT startSector_i,          /* Starting sector # for the transfer (LBA28) */
                                      INT numSectors_i,           /* Number of sectors to transfer.             */
                                      SATA_COMMAND_TYPE_T cmd_i,  /* Type of command (Read / Write)             */
                                      SATA_TRANSFER_TYPE_T mode_i,/* Requested mode of transfer ( DMA / PIO )   */
                                      UINT64  ddt_i,              /* Bus address of the DMA Descriptor Table    */
                                      UINT32  ddtePage_i,         /* Top 32 bits of the DMA Data address in DDT */
                                      void * buffer_iop           /* Virtual Address of the source/target buffer*/
                                      )
{

    SATA_COMMAND_T *sataCmd_p;
    DEV_INFO_T * currDrive = NULL_POINTER;
    boolean lba48 = FALSE;


    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    if(!TAG_WITHIN_RANGE(devTag_i, MAX_DEVICES))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(devices_gp[devTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    /* Check that start sector and end sectors are within range. 
    if(!WITHIN_RANGE(startSector_i, 0, devices_gp[devTag_i]->lbaCapacity ))
    {
        diag_printf("ERROR: Invalid value for startSector(%d)."
                      " Must be lesser than %lld",
                      startSector_i, devices_gp[devTag_i]->lbaCapacity);
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    if(!WITHIN_RANGE(startSector_i + numSectors_i, 0,
                     devices_gp[devTag_i]->lbaCapacity ))
    {
        diag_printf("ERROR: Invalid value for End Sector(%d)."
                      " Must be lesser than %lld",
                      startSector_i + numSectors_i,
                      devices_gp[devTag_i]->lbaCapacity);
        return SATA_OUT_OF_RANGE_VALUE ;
    }
*/

	currDrive = devices_gp[devTag_i];

//    ASSERT(currDrive->devFeatures);

    /* if no features set by default we are assuming LBA28 */
    lba48 = currDrive->devFeatures->Lba48 ? TRUE : FALSE;

    /* Max 256 sectors of transfer at a time under LBA 28. */
    /* Max 65536 sectors of transfer at a time under LBA 48. */
    if (lba48 && (numSectors_i > 0xffff))
    {
        diag_printf("ERROR: LBA48 numSectors(%d) > 0xffff not allowed",
                      numSectors_i);
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    else if(!lba48 && (numSectors_i > 0xff))
    {
        diag_printf("ERROR: LBA28 numSectors(%d) > 0xff not allowed",
                      numSectors_i);
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* Check the command */
    if((cmd_i != SATA_READ) && (cmd_i != SATA_WRITE))
    {
        diag_printf("ERROR: I/O Command(%d) is neither READ nor WRITE", cmd_i);
        return SATA_OUT_OF_RANGE_VALUE;
    }

    /* Check the DDT params. These can be NULL for PIO data transfers */
    if((mode_i == FIRST_PARTY_DMA) && (ddt_i == 0))
    {
        diag_printf("ERROR: DMA Command with DDT = 0%s", "");
        return SATA_OUT_OF_RANGE_VALUE;
    }
    /* Check the buffer pointer. for DMA commands, we won't be using the pointer, BUT */
    /* for PIO mode of data transfer, we will need the buffer pointer. */
    /* NOTE: PIO mode can be FORCED even if DMA mode is requested.     */
    if(buffer_iop == NULL_POINTER)
	{
		diag_printf("ERROR: Tag %d, buffer_iop is NULL",adapterTag_i);
		return SATA_NOT_INITIALIZED;
	}


    /* NPG: TCQ Not yet supported. We allow only ONE command to be issued at a time. */
    sataCmd_p = &currDrive->commandQueue[0];

    if(sataCmd_p->slotState != SLOT_FREE)
    {
        /* A command has already been queued and has not yet completed. */
        diag_printf("Command Slot is in use(%d). Returning Busy", sataCmd_p->slotState);
        return SATA_BUSY;
    }
    /* Fill out the command structure */

    sataCmd_p->slotState = SLOT_OCCUPIED;

    /* The ioMode in the devInfo is used to FORCE a particular mode. */
    /* If it has a value of VARIABLE then the mode specified in the  */
    /* command will be used                                          */


    if(currDrive->ioMode != VARIABLE)
    {
        /* Force the drive specific mode */
        mode_i = currDrive->ioMode;
    }

    /* We record the Actual mode that will be used for the command */
    sataCmd_p->txType = mode_i;
    sataCmd_p->cmdType = cmd_i;

    /* Fill out the taskfile */

    sataCmd_p->taskfile.feature = 0;
    /* Keep track of the number of sectors yet to be transfered separately. */
    /* Makes it easier to know when the Tx is over in the case when         */
    sataCmd_p->sectRemaining = numSectors_i;

    /* if we are doing LBA 48 */
    if(lba48)
    {
        sataCmd_p->taskfile.sectorCount = numSectors_i & 0xffff;

        /* bits 31:24 7:0  */
        sataCmd_p->taskfile.sectorNumber =
        (startSector_i & 0xff) | ((startSector_i >> 16) & 0xff00);
        startSector_i >>= 8;
        sataCmd_p->taskfile.cylinderLow  = startSector_i & 0xff;
        startSector_i >>= 8;
        sataCmd_p->taskfile.cylinderHigh = startSector_i & 0xff;
        /* always master */
        sataCmd_p->taskfile.deviceHead = 0xf;
    }
    else /* LBA 28 */
    {
        /* sectorCount = 0     */
        /* max 255, 0 == 256 */
        // sataCmd_p->taskfile.sectorCount = numSectors_i % 256 ;
        sataCmd_p->taskfile.sectorCount = numSectors_i & 0xff;

        /* compute the LBA values */
        /* All devices supporting ATA/ATAPI-5 and above support LBA */
        sataCmd_p->taskfile.sectorNumber = startSector_i & 0xFF; /* Lower 8 bits */
        startSector_i >>= 8;
        sataCmd_p->taskfile.cylinderLow  = startSector_i & 0xFF; /* Next 8 bits */
        startSector_i >>= 8;
        sataCmd_p->taskfile.cylinderHigh = startSector_i & 0xFF; /* Next 8 bits */
        startSector_i >>= 8;
        sataCmd_p->taskfile.deviceHead   = startSector_i & 0xF;  /* Last 4 bits */

    }
    /* Mark the startSector address as LBA address, Dev as 0 */
    sataCmd_p->taskfile.deviceHead =
        (UINT8)SET_BIT(sataCmd_p->taskfile.deviceHead, DEVICE_HEAD_LBA_BIT);

//    diag_printf("taskfile.deviceHead = %0x, Verify that Bit %d is SET.",
//                  sataCmd_p->taskfile.deviceHead, DEVICE_HEAD_LBA_BIT);

    /* The actual command depends on 1.READ/Write, 2.DMA/PIO, 3. Multisector */
    if(cmd_i == SATA_READ)
    {/* Read */
        /* Which mode ? 3rd Party DMA not yet implemented */
        if(mode_i == FIRST_PARTY_DMA)
        {
            /* 1st Party DMA Read.ignore Multisector */
            sataCmd_p->taskfile.command =
            lba48 ? ATA_READ_DMA_EXT : ATA_READ_DMA;
        }
        else
        { /* if not 1st party DMA then Assume PIO */
            /* Multisector transfers enabled ? */
            if(currDrive->devFeatures->ReadMultiple == 0)
            {
                /* PIO Read without multiple sectors */
                sataCmd_p->taskfile.command =
                lba48 ? ATA_READ_SECTORS_EXT : ATA_READ_SECTORS;
            }
            else
            {
                /* PIO Read with multiple sectors */
                sataCmd_p->taskfile.command =
                lba48 ? ATA_READ_MULTIPLE_EXT : ATA_READ_MULTIPLE;
            }/* Multisector */
        } /* mode */
    }/* Read/Write */
    else
    {/* Write */
        /* Which mode ? 3rd Party DMA not yet implemented */
        if(mode_i == FIRST_PARTY_DMA)
        {
            /* 1st Party DMA Write. ignore Multisector */
            sataCmd_p->taskfile.command =
            lba48 ? ATA_WRITE_DMA_EXT : ATA_WRITE_DMA;
        }
        else
        { /* if not 1st party DMA then Assume PIO */
            /* Multisector transfers enabled ? */
//            if(currDrive->multisector == 0)
            if(currDrive->devFeatures->ReadMultiple == 0)
            {
                /* PIO Write without multiple sectors */
                sataCmd_p->taskfile.command =
                lba48 ? ATA_WRITE_SECTORS_EXT : ATA_WRITE_SECTORS;
            }
            else
            {
                /* PIO Write with multiple sectors */
                sataCmd_p->taskfile.command =
                lba48 ? ATA_WRITE_MULTIPLE_EXT : ATA_WRITE_MULTIPLE;
            } /* Multisector */
        }/* mode */
    }/* Read/Write */
    /* That takes care of the taskfile. 
    diag_printf("Taskfile:\n"
                  "feature      %04x\n"
                  "sectorCount  %04x\n"
                  "sectorNumber %04x\n"
                  "cylinderLow  %04x\n"
                  "cylinderHigh %04x\n"
                  "deviceHead   %02x\n"
                  "command      %02x\n",
                  sataCmd_p->taskfile.feature,
                  sataCmd_p->taskfile.sectorCount,
                  sataCmd_p->taskfile.sectorNumber,
                  sataCmd_p->taskfile.cylinderLow,
                  sataCmd_p->taskfile.cylinderHigh,
                  sataCmd_p->taskfile.deviceHead,
                  sataCmd_p->taskfile.command);

	*/
    /* Initialize the other elements to 0 */
    sataCmd_p->dma.ddt = ddt_i;
    sataCmd_p->dma.ddtePage = ddtePage_i;
    sataCmd_p->status  = 0;
    sataCmd_p->sStatus = 0;
    sataCmd_p->error   = 0;
    sataCmd_p->sError  = 0;
    sataCmd_p->buffer  = buffer_iop;

    /* All done. Return success */
    return SATA_SUCCESS;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterServiceCommandQueues                                   */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function initiatiates servicing of commandQueues for    */
/*               all the drives under this adapter. It implements a simple    */
/*               fairness algorithm, where it starts servicing the queues     */
/*               from the next drive from a circular list of drives. This     */
/*               should not be a critical issue as all I/O transactions are   */
/*               expected to be 1st party DMA operations, coupled with the    */
/*               fact that each port on the SATA controller (e.g. Artisea)    */
/*               would have it's own DMA controller.                          */
/*                                                                            */
/* NOTE: Note that the SATA port does not generate an interrupt when a bus    */
/*       master operation is aborted. Software should time out.               */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    three types of errors are possible,                           */
/*              1. one or more drives was busy and will have pending commands */
/*                 in the queue after this call.                              */
/*                 The error value in this case is SATA_COMPLETED_WITH_ERRORS */
/*              2. one or more drives failed to issue pending commands for    */
/*                 other non-trivial reasons.                                 */
/*                 The error value in this case is SATA_FAILED.               */
/*                 The detailed error value returned by the drive is discarded*/
/*              3. Other generic errors could return error values of          */
/*                 SATA_OUT_OF_RANGE_VALUE, SATA_NOT_INITIALIZED              */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterServiceCommandQueues (INT adapterTag_i)
{

    INT currDriveIndex, driveTag, ctr;
    ADAPTER_INFO_T *adapterp = NULL_POINTER;
    STATUS_CODE_T retval = SATA_SUCCESS;
    boolean someDrivesBusy = FALSE;
    boolean someDrivesFailed = FALSE;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    adapterp = adapters_gp[adapterTag_i];

    /* decide which drive to service now. */
    currDriveIndex =
    NEXT_IN_LOOP(adapterp->driveLastServiced,MAX_DEVICES_PER_ADAPTER);

    /* Sanity check on the currDriveIndex value. (to protect against -ve numbers)*/
    if(!WITHIN_RANGE(currDriveIndex, 0, MAX_DEVICES_PER_ADAPTER))
        currDriveIndex = 0;

    /* OK, Now we know which driveindex to look at next, but is it a valid */
    /* drive ? If not, start from index 0*/
    driveTag = adapterp->drives[currDriveIndex];
    if(driveTag == INVALID_TAG)
        currDriveIndex = 0;

    adapterp->driveLastServiced = currDriveIndex;

    for(ctr = 0; ctr < MAX_DEVICES_PER_ADAPTER; ctr++)
    {
        /* loop through the entire array once.*/
        driveTag = adapterp->drives[currDriveIndex];

        /* What's the next drive index ? */
        currDriveIndex =
        NEXT_IN_LOOP(currDriveIndex,MAX_DEVICES_PER_ADAPTER);

        if(driveTag == INVALID_TAG) continue;

        /* Now ask the drive to issue any pending requests */
        retval = DevServiceCommandQueue(driveTag);

        if(retval == SATA_BUSY)
        {
            /* The drive was busy and could not issue all commands.       */
            /* some commands are pending in the queue and the OSDM should */
            /* service the commandQueues again after some time.           */
            someDrivesBusy = TRUE;
        }
        else if(retval != SATA_SUCCESS)
        {
            /* The call failed for some other reason. */
            someDrivesFailed = TRUE;
        }
    }

    /* Called the DevServiceCommandQueue API once for all the drives */
    /* Return the status of the operation. */

    if(someDrivesFailed == TRUE) return SATA_FAILURE;
    if(someDrivesBusy   == TRUE) return SATA_COMPLETED_WITH_ERRORS;

    /*  All went well */
    return SATA_SUCCESS;

}


/******************************************************************************/
/*                                                                            */
/*                 P R I V A T E    F U N C T I O N S                         */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function accepts as drive number (0 to n) and the adapter*/
/*              number. It uses this numbers to locate the right port the     */
/*              drive should be attached to. It returns the portTag for that  */
/*              port.                                                         */
/*                                                                            */
/* PARAMETERS   deviceNumber_i                                                */
/*                  the drive number (0 to MAX_DEVICES_PER_ADAPTER) that      */
/*                  might be connected to a                                   */
/*                  port on a controller on an adapter.                       */
/*              adapterNumber_i                                               */
/*                  The adapterNumber od the adapter the drive is connected to*/
/*                                                                            */
/* RETURNS      portTag corresponding to the drive number.                    */
/*                                                                            */
/* ON ERRORS    PortTag will have a value of INVALID_TAG on errors.           */
/*                                                                            */
/******************************************************************************/
static INT DriveNumToPort(INT adapterNumber_i, INT deviceNumber_i)
{
    INT port, portTag;

    INT maxPortsPerAdapter =
    MAX_CONTROLLERS_PER_ADAPTER * MAX_PORTS_PER_CONTROLLER ;

    /* Sanitize the parameters... The deviceNumber_i should be between 0 to
       MAX_DEVICES_PER_ADAPTER. Assert an error.*/

    //ASSERT( WITHIN_RANGE(deviceNumber_i, 0, MAX_DEVICES_PER_ADAPTER));

    if(!WITHIN_RANGE(deviceNumber_i, 0, MAX_DEVICES_PER_ADAPTER))
    {
        return INVALID_TAG;
    }

    /* Which port on the adapter is this device connected to ? */
    port = (INT)deviceNumber_i / (INT)MAX_DEVICES_PER_PORT;

    /* Which port is that in the global list ? */
    portTag = port + adapterNumber_i * maxPortsPerAdapter;

    return portTag;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterEnableInterrupts                                       */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function enables the interrupts on the HBA once all     */
/*               initialization is completed and the driver is ready to       */
/*               receive / transmit data.                                     */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterEnableInterrupts(INT adapterTag_i)
{
    INT currController, currPort;
    INT numControllers;  /* Number of controllers on this adapter */
    INT ctr, ctr2;      /* counters used in a for loop */

    ADAPTER_INFO_T *adapterp = NULL_POINTER;

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    adapterp = adapters_gp[adapterTag_i];

    /* Detect how many controllers we have */
    numControllers = MAX_CONTROLLERS_PER_ADAPTER;

    for(ctr = 0; ctr < numControllers; ctr++)
    {
        currController = adapterp->controller[ctr];

        /* INVALID_TAG indicates that we have looked at all the valid controllers */
        if(currController == INVALID_TAG) break;

        for(ctr2 = 0; ctr2 < MAX_PORTS_PER_CONTROLLER ; ctr2++)
        {
            currPort = controllers_gp[currController]->port[ctr2];

            /* INVALID_TAG indicates that we have looked at all the valid ports */
            if(currPort == INVALID_TAG) break;

			// clear the pending interrupts before enabling
			retval = PortClearInterrupt(currPort);
            if(retval != SATA_SUCCESS) return retval;

            /* Enable the Interrupts for the port */
            retval = PortEnableInterrupts(currPort);
            /* Check for system errors */
            if(retval != SATA_SUCCESS) return retval;

        }
    }

    /* Completed Successfully. Return success */
    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterDisableInterrupts                                      */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function enables the interrupts on the HBA once all     */
/*               initialization is completed and the driver is ready to       */
/*               receive / transmit data.                                     */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterDisableInterrupts(INT adapterTag_i)
{
    INT currController, currPort;
    INT numControllers;  /* Number of controllers on this adapter */
    INT ctr, ctr2;      /* counters used in a for loop */

    ADAPTER_INFO_T *adapterp = NULL_POINTER;

    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
        return SATA_OUT_OF_RANGE_VALUE ;
    if(adapters_gp[adapterTag_i] == NULL_POINTER) return SATA_NOT_INITIALIZED;

    adapterp = adapters_gp[adapterTag_i];

    /* Detect how many controllers we have - hard coded to One controller Only for now.*/
    numControllers = MAX_CONTROLLERS_PER_ADAPTER;

    for(ctr = 0; ctr < numControllers; ctr++)
    {
        currController = adapterp->controller[ctr];

        /* INVALID_TAG indicates that we have looked at all the valid controllers */
        if(currController == INVALID_TAG) break;

        for(ctr2 = 0; ctr2 < MAX_PORTS_PER_CONTROLLER ; ctr2++)
        {
            currPort = controllers_gp[currController]->port[ctr2];

            /* INVALID_TAG indicates that we have looked at all the valid ports */
            if(currPort == INVALID_TAG) break;

            /* Disable the Interrupts for the port */
            retval = PortDisableInterrupts(currPort);
            /* Check for system errors */
            if(retval != SATA_SUCCESS) return retval;

        }
    }

    /* Completed Successfully. Return success */
    return SATA_SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterGetMaxDdtEntries()                                     */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function returns the number of scatter gather segments  */
/*               the library will support.                                    */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*                                                                            */
/* RETURNS      ULONG number of segments                                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
ULONG AdapterGetMaxDdtEntries()
{
    return DDT_MAX_ENTRIES;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterGetMaxTransferLength()                                 */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function returns the maximum buffer length that         */
/*               the library will support.                                    */
/*               This the the maximum size that the Library can transfer at   */
/*               one time.                                                    */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*                                                                            */
/* RETURNS      ULONG length                                                  */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
ULONG AdapterGetMaxTransferLength()
{

    /* DDT_ENTRY_MAX_COUNT is word aligned (LSB = 0), */
    /* adjust the value to get he correst number */

    return DDT_MAX_ENTRIES * ( DDT_ENTRY_MAX_COUNT + 1);
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterGetMaxDevices()                                        */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function returns the number of devices that             */
/*               the library will support.                                    */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*                                                                            */
/* RETURNS      UCHAR number of devices                                       */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
UCHAR AdapterGetMaxDevices()
{

    return MAX_DEVICES_PER_ADAPTER;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterIntHardReset()                                         */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function resets the SATA interface and bring            */
/*               the PHY online. Should be called in order to bring PHY       */
/*               online. The BIST done by software will call this after test  */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for BIST at initialization           */
/*                                                                            */
/* POST-CONDITIONS  The PHY is offline or in BIST mode                        */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
//INLINE 
STATUS_CODE_T AdapterIntHardReset(INT adapterTag_i)
{
    ADAPTER_INFO_T *adapterp = adapters_gp[adapterTag_i];
    STATUS_CODE_T retval = SATA_SUCCESS;
    INT currController;
    INT numControllers;  /* Number of controllers on this adapter */
    INT ctr; /* counters used in a for loop */

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
    {
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(!adapterp)
    {
        return SATA_NOT_INITIALIZED;
    }

    /* Detect how many controllers we have */
    /* hard coded to One controller Only for now.*/
    numControllers = MAX_CONTROLLERS_PER_ADAPTER;

    /* Ask all the controllers COMRESET their interface */
    for(ctr = 0; ctr < numControllers; ctr++)
    {
        currController = adapterp->controller[ctr];

        /* hardreset to bring PHY online */
        if((retval = ControllerIntHardReset(currController)) != SATA_SUCCESS)
        {
            return retval;
        }
    }

    return SATA_SUCCESS;
} /* end of AdapterHardReset() */

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterIntHardReset()                                         */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function calculates total memory needed for all data    */
/*               structures that needs to be allocated. This function is only */
/*               needed for windows due to miniport cannot dynamically alloc  */
/*               or free memory after certain initialization stage            */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*                                                                            */
/* RETURNS      total size of memory needed                                   */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
//INLINE 
UINT AdapterMemorySize()
{
    UINT memSize = 0;
    UINT total = 0;

    /* calculate all adapters */
    total = MAX_ADAPTERS;
    memSize += total * sizeof(ADAPTER_INFO_T);

    /* calculate all controllers */
    total *= MAX_CONTROLLERS_PER_ADAPTER;
    memSize += total * sizeof(CONTROLLER_INFO_T);

    /* calcuate all ports */
    total *= MAX_PORTS_PER_CONTROLLER;
    memSize += total * sizeof(PORT_INFO_T);

    /* calculate all devices */
    total *= MAX_DEVICES_PER_PORT;
    memSize += total * sizeof(DEV_INFO_T);

    return memSize;
} /* end of AdapterMemorySize() */

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function accepts as drive number (0 to n) that might be  */
/*              connected to this adapter. It uses this number to locate the  */
/*              right port the drive should be attached to. It then performs  */
/*              BIST on the port interface. BIST is only done against first   */
/*              device on each port.                                          */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS   adapterTag_i                                                  */
/*                  Tag for the adapter to be used.                           */
/*              deviceNumber_i                                                */
/*                  the drive number (0 to n) that might be connected to a    */
/*                  port on a controller on this adapter.                     */
/*                                                                            */
/* RETURNS      returns error from PortBistStatus                             */
/******************************************************************************/
STATUS_CODE_T AdapterPortBist(INT adapterTag_i, INT deviceNumber_i)
{
    INT portTag;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
    {
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(adapters_gp[adapterTag_i] == NULL_POINTER)
    {
        return SATA_NOT_INITIALIZED;
    }

    /* check if the deviceNumber_i is device 0 */
    /* we don't do BIST except with device 0 */
    if((deviceNumber_i % MAX_DEVICES_PER_PORT) != 0)
    {
        /* Report out of range error */
        diag_printf("BIST Skipped for deviceNumber_i %d", deviceNumber_i);
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* The adapterTag gives us the adapter number. We are looking for the portTag*/
    /* corresponding to the nth drive connected to this adapter */
    portTag = DriveNumToPort(adapterTag_i, deviceNumber_i);

    return PortBistStatus(portTag);
} /* end of AdapterPortBist() */

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
/*              This function calls DevSetFeatures() to set feature for       */
/*              individual devices.                                           */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*              The device features table must be filled out and valid!       */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              adapterTag_i                                                  */
/*                  Tag for the adapter that the device resides on            */
/*              deviceNumber_i                                                */
/*                  The device no relative to the adapter to pass the feature */
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
STATUS_CODE_T AdapterDevSetFeatures(INT adapterTag_i, INT deviceNumber_i,
                                    PSATA_FEATURES_T features_p)
{
    INT portTag;
    INT devTag;
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
    {
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(adapters_gp[adapterTag_i] == NULL_POINTER)
    {
        return SATA_NOT_INITIALIZED;
    }

    /* check if the deviceNumber_i is within a valid range */
    if((deviceNumber_i < 0) || (deviceNumber_i > MAX_DEVICES))
    {
        /* Report out of range error */
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* The adapterTag gives us the adapter no. We are looking for the portTag */
    /* corresponding to the nth drive connected to this adapter */
    portTag = DriveNumToPort(adapterTag_i, deviceNumber_i);

    /* Which Device on this port ? */
    devTag = ports_gp[portTag]->dev[ deviceNumber_i % MAX_DEVICES_PER_PORT];

    return DevSetFeatures(devTag, features_p);
} /* end of AdapterPortSetFeatures() */

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function unsets the pointer to the device features       */
/*              This function is called by the OSDM during hot-plug cleanup   */
/*                                                                            */
/* PRECONDITIONS                                                              */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              adapterTag_i                                                  */
/*                  Tag for the adapter that the device resides on            */
/*              deviceNumber_i                                                */
/*                  The device no relative to the adapter to pass the feature */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = Invalid Tag                         */
/*              SATA_NOT_INITIALIZED    = device not initialized              */
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterDevUnsetFeatures(INT adapterTag_i, INT deviceNumber_i)
{
    INT portTag;
    INT devTag;
    STATUS_CODE_T retval = SATA_SUCCESS;

    /* Sanity check */
    if(!TAG_WITHIN_RANGE(adapterTag_i, MAX_ADAPTERS))
    {
        return SATA_OUT_OF_RANGE_VALUE ;
    }
    if(adapters_gp[adapterTag_i] == NULL_POINTER)
    {
        return SATA_NOT_INITIALIZED;
    }

    /* check if the deviceNumber_i is within a valid range */
    if((deviceNumber_i < 0) || (deviceNumber_i > MAX_DEVICES))
    {
        /* Report out of range error */
        return SATA_OUT_OF_RANGE_VALUE ;
    }

    /* The adapterTag gives us the adapter no. We are looking for the portTag */
    /* corresponding to the nth drive connected to this adapter */
    portTag = DriveNumToPort(adapterTag_i, deviceNumber_i);

    /* Which Device on this port ? */
    devTag = ports_gp[portTag]->dev[ deviceNumber_i % MAX_DEVICES_PER_PORT];

    return DevUnsetFeatures(devTag);
} /* end of AdapterPortUnsetFeatures() */
