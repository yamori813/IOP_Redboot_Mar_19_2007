/************************************************************************;
;*                                                                      
;*      Filename: sata_adapter.h  								 
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
/*   DESCRIPTION                                                              */
/*                                                                            */
/*        This file contains all the definations related to the "Adapter"     */
/*        object. It also contains the prototypes and definations related to  */
/*        any generic SATA logic.                                             */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_ADAPTER_HEADER
#define SATA_ADAPTER_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver          */
#include <sata_lib.h>   /* Definations of structures internal to SATA Lib */
#include <sata.h>       /* Stat command / mode types */

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

/* returns val + 1 if val + 1 < max, else returns 0. */
#define NEXT_IN_LOOP(val, max) ((val) >= ((max) - 1) ? 0 : ((val) + 1))

/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/

typedef struct ADAPTER_INFO_S
{
    boolean present;        /* Adapter present and struct data valid ?        */
    INT controller[MAX_CONTROLLERS_PER_ADAPTER];/*Index #s into controllers_gp*/
    INT drives[MAX_DEVICES_PER_ADAPTER]; /* Index #s of drives in this adapter*/
    INT osdmTag;           /* Tag registered by the OSDM during initialization*/
    INT driveLastServiced; /* Index number of the drive last serviced.        */
} ADAPTER_INFO_T;

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

static STATUS_CODE_T InitLib ( void );

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

STATUS_CODE_T AdapterAllocatePrivateMemory (INT osdmTag_i, INT * adapterTag_op);

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
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterInit (INT adapterTag_i, SATA_REG_ADDR_T baseAddr_i);


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
/*              devTag_op                                                     */
/*                  The Device Tag for the corresponding drive                */
/*                                                                            */
/* RETURNS      SATA_TRUE on success, error code on failure                   */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE, SATA_NOT_INITIALIZED, SATA_FALSE     */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T AdapterDevicePresent (
                INT adapterTag_i,
                INT deviceNumber_i,
                INT * devTag_op
                );

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
                                    void * buffer_op);

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
    );

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function adds the I/O command to the commandQueue of the */
/*              specified drive. This function accepts a sector range for     */
/*              transfer and computes the taskfile contents baed on this      */
/*              information.                                                  */
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
/*              numSectors_i                                                  */
/*                  Number of sectors to transfer (Max 256).                  */
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
    );

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
STATUS_CODE_T AdapterServiceCommandQueues(INT adapterTag_i);

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
STATUS_CODE_T AdapterEnableInterrupts(INT adapterTag_i);

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
STATUS_CODE_T AdapterDisableInterrupts(INT adapterTag_i);

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterGetMaxDdtEntries()                                     */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function returns the number of scatter gather segments  */
/*               the library will support.                                    */
/*                                                                            */
/* PRECONDITIONS    The adapter is ready for use.                             */
/*                                                                            */
/* POST-CONDITIONS                                                            */
/*                                                                            */
/* PARAMETERS                                                                 */
/*                                                                            */
/* RETURNS      ULONG number of segments                                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
ULONG AdapterGetMaxDdtEntries(void);

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterGetMaxTransferLength()                                 */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function returns the maximum buffer length that         */
/*               the library will support.                                    */
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
ULONG AdapterGetMaxTransferLength( void );

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
UCHAR AdapterGetMaxDevices(void);

/******************************************************************************/
/*                                                                            */
/* FUNCTION     AdapterIntHardReset()                                         */
/*                                                                            */
/* DESCRIPTION                                                                */
/*               This function resets the SATA interface of all controllers   */
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
STATUS_CODE_T AdapterIntHardReset(INT adapterTag_i);

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
UINT AdapterMemorySize(void);

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
STATUS_CODE_T AdapterPortBist(INT adapterTag_i, INT deviceNumber_i);

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
                                    PSATA_FEATURES_T features_p);

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
STATUS_CODE_T AdapterDevUnsetFeatures(INT adapterTag_i, INT deviceNumber_i);

#endif /* SATA_ADAPTER_HEADER */
