/************************************************************************;
;*                                                                      
;*      Filename: sata_device.h 								 
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
/*   DESCRIPTION sata_device.h                                                */
/*                                                                            */
/*         This file includes definations related to SATA devices. At this    */
/*         time, only SATA hard disks are supported. The definations in this  */
/*         file should be independent of the controller.                      */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_DEVICE_HEADER
#define SATA_DEVICE_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/
#include <sata_types.h> /* Common data types used in SATA driver */
#include <sata_lib.h>   /* Definations of structures internal to SATA Lib */
#include <sata_dma.h>

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
#if 0
#define UPPER_32_BITS(addr) (UINT32)(( (addr) >> 32) & 0xFFFFFFFF)
#define LOWER_32_BITS(addr) (UINT32)(  (addr)        & 0xFFFFFFFF)
#endif

/* defines to use for the DMA Command Register */

#define DMA_DIRECTION_HOST_TO_DEV (RESET_BIT(0, SUPDDCMDR_DIR_BIT))        /* = 0 */
#define DMA_DIRECTION_DEV_TO_HOST   (SET_BIT(0, SUPDDCMDR_DIR_BIT))
#define DMA_START                   (SET_BIT(0, SUPDDCMDR_START_STOP_BIT))
#define DMA_STOP                  (RESET_BIT(0, SUPDDCMDR_START_STOP_BIT)) /* = 0 */

/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/

typedef struct SATA_COMMAND_S * PSATA_COMMAND_T;

typedef struct SATA_COMMAND_S
{
    SATA_COMMAND_SLOT_STATE_T slotState; /* Command slot state in a TCQ       */
    SATA_TRANSFER_TYPE_T    txType;   /* Type of data transfer                */
    SATA_COMMAND_TYPE_T     cmdType;  /* I/O or special command               */
    SATA_TASKFILE_T         taskfile; /* taskfile for this command            */
    INT                sectRemaining; /* Count of sectors yet to be transfered*/
    DMA_INFO_T              dma;      /* associated dmaInfo                   */
    void * buffer;         /* The Source/Destination address for the transfer */
    UINT8                   status;   /* Copy of SATA Altstatus reg (SUPDASR) */
    UINT32                  sStatus;  /* Copy of SATA SStatus reg (SUPDSSSR)  */
    UINT8                   error;    /* Copy of SATA error reg (SUPDER)      */
    UINT32                  sError;   /* Copy of SATA SError reg (SUPDSSER)   */
    PSATA_COMMAND_T         prevCmd;  /* previous command in list */
    PSATA_COMMAND_T         nextCmd;  /* next command in list */
} SATA_COMMAND_T;

/* Note: The devices could be RSMs rather than end devices. */
/* Currently RSM will not be supported in this design.      */

typedef struct DEV_INFO_S
{
    boolean present;             /* device present and struct data valid ?    */
    boolean active;              /* device active? */
    INT port;                    /* Index number of the connected port        */
    SATA_TRANSFER_TYPE_T ioMode; /* force type of data transfer pio/dma/cmd dependent */
    SATA_COMMAND_T commandQueue[MAX_TCQ_DEPTH]; /* simple Command Queue */
    PSATA_COMMAND_T cmdFreeQueue;    /* pointer to free Command list */
//    OS_SPINLOCK_T   cmdQLock; /* spinlock for command queue */
//    OS_SPINLOCK_T   freeCmdQLock; /* spinlock for free command list */
    void * osDriveTag;      /* The OS's Drive Tag.                            */
    void * dmaHandler;      /* OS specific Dma Handler's address              */
    void * pioReadHandler;  /* OS specific PIO Handler's address              */
    void * pioWriteHandler; /* OS specific PIO Handler's address              */
    boolean isDrive;        /* Is this sata device a drive or something else? */
    UINT64 lbaCapacity;     /* Can hold LBA 48 values                         */
//  INT multisector;        /* Current multisector values 0 = disabled        */
    IDENTIFY_DEVICE_T devIdentify; /* Store devIdentify info for debug uses   */
    PSATA_FEATURES_T  devFeatures; /* Store devFeatures info */
    SATA_COMMAND_T lastCommand; /* For Debug purposes */
	boolean	waiting_for_dma; /* we're waiting for dma to complete on this drive */  
} DEV_INFO_T;


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
/*              This function allocates the private memory for the device. It */
/*              allocates a DEV_INFO_T structure and records the pointer in   */
/*              the next available slot in the devices_gp array. This         */
/*              function also initializes the allocated memory structure      */
/*              and sets the port member to the passed portTag.               */
/*                                                                            */
/* PRECONDITIONS    devices_gp array has been allocated.                      */
/*                                                                            */
/* POST-CONDITIONS  The next NULL pointer in device_gp is set.                */
/*                  *devTag_op is set to it's index number or INVALID_TAG     */
/*                                                                            */
/* PARAMETERS   portTag_i                                                     */
/*                  The Tag of the parent port.                               */
/*              devTag_op                                                     */
/*                  Address of the INT variable to hold the devTag.           */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in devices_gp array        */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevAllocatePrivateMemory (INT portTag_i, INT *devTag_op);

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
;
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
STATUS_CODE_T DevSoftReset(INT devTag_i, INT devNumber_i);

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
STATUS_CODE_T SelectDevice(INT devTag_i);

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function adds a command to the device queue.             */
/*              is used by the Adapter object, and should not be called from  */
/*              the OSDM (unless you know what you are doing)                 */
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
/*              Other Error codes on System errors                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T DevAddCommandToQueue(INT devTag_i, SATA_COMMAND_T * sataCmd_iop);

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
STATUS_CODE_T DevServiceCommandQueue(INT devTag_i);

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
boolean DevIsReady(INT devTag_i);

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
boolean DevDRQSet(INT devTag_i);

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function gets the status of the device by calling the    */
/*                   by calling the PortReadAltStatusReg fcn                  */
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
STATUS_CODE_T DevGetStatus(INT devTag_i, UINT8 * status_op );

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
STATUS_CODE_T DevGetAltStatus(INT devTag_i, UINT8 * status_op );

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
STATUS_CODE_T DevGetDmaStatus(INT devTag_i, UINT8 * status_op );

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
STATUS_CODE_T DevGetError(INT devTag_i, UINT8 * error_op );

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
						  SATA_TASKFILE_T * taskFile_op);

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
STATUS_CODE_T DevClearError(INT devTag_i );

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
boolean DevInterruptPending(INT devTag_i);

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
boolean DevDmaInterruptPending(INT devTag_i);

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
STATUS_CODE_T DevDisableInterrupts (INT devTag_i);

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
STATUS_CODE_T DevEnableInterrupts (INT devTag_i);

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
STATUS_CODE_T DevPioMultiRdWr(INT devTag_i, SATA_COMMAND_T * sataCmd_iop);

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
STATUS_CODE_T DevIdentify(INT devTag_i, void * buffer_op);

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
/* PRECONDITIONS                                                              */
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
STATUS_CODE_T DevIsr ( INT devTag_i );

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
        INT             devTag_i,      /* DevTag of the device */
        boolean         waitOption_i,  /* F = Issue the command. nothing else */
                                       /* T = Wait for the command completion */
                                       /*     with drive Interrupts disabled  */
        SATA_TASKFILE_T * taskfile_ip  /* Taskfile contents for the command   */
        );


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
INT DumpErrStat(int devTag_i, INT limit_i, void * buffer_op);

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
STATUS_CODE_T DevSetFeatures(INT devTag_i, PSATA_FEATURES_T features_p);

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
STATUS_CODE_T DevUnsetFeatures(INT devTag_i);


#endif /* SATA_DEVICE_HEADER */
