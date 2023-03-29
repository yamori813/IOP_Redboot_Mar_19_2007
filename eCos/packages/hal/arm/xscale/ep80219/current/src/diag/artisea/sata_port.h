/************************************************************************;
;*                                                                      
;*      Filename: sata_port.h  								 
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
/*         This file includes declarations related to the SATA port on the    */
/*         controller. Many of the declarations might be controller specific. */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_PORT_HEADER
#define SATA_PORT_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver          */
#include <sata_lib.h>   /* Definations of structures internal to SATA Lib */
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


typedef struct PORT_INFO_S
{
    boolean   present;    /* port present and struct data valid ?             */
    INT       dev[MAX_DEVICES_PER_PORT]; /* Index #s into devices_gp          */
    INT       controller; /* Index number of the parent controller            */
    INT       portNumber; /* 0 - n port on the controller                     */
    SATA_REG_ADDR_T portRegBase; /* Base address for common SATA registers    */
    boolean   connected;  /* Reflects the last known connection status        */
    UINT32    cumulativeSSER; /* Cumulative value of SUPDSSER bits   till now */
    UINT32    lastSSER;       /* Value of SUPDSSER last read                  */
    UINT32    lastER;         /* Value of SUPDER last read                    */
    UINT32    lastASR;        /* Value of SUPDASR last read                   */
    UINT32    lastSR;         /* Value of SUPDSR last read                    */
    UINT32    lastSSSR;       /* Value of SUPDSSSR last read                  */
	UINT32    testResults;	  /* Status of testing                            */
} PORT_INFO_T;


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
/*              This function allocates the private memory for the port. It   */
/*              allocates a PORT_INFO_T structure and records the pointer in  */
/*              the next available slot in the ports_gp array. This function  */
/*              also initializes the allocated memory structure and sets the  */
/*              controller member to the passed controllerTag.                */
/*                                                                            */
/* PRECONDITIONS    ports_gp array has been allocated.                        */
/*                                                                            */
/* POST-CONDITIONS  The next NULL pointer in ports_gp is set.                 */
/*                  *portTag_op is set to it's index number or INVALID_TAG    */
/*                                                                            */
/* PARAMETERS   controllerTag                                                 */
/*                  The Tag of the parent controller.                         */
/*              portTag_op                                                    */
/*                  Address of the int variable to hold the portTag.          */
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in ports_gp array          */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortAllocatePrivateMemory (INT controllerTag_i, INT *portTag_op);


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
/*                                                                            */
/* RETURNS      SATA_SUCCESS on success, error code on failure                */
/*                                                                            */
/* ON ERRORS    returns the following error codes                             */
/*              SATA_OUT_OF_RANGE_VALUE = no space in ports_gp array          */
/*              SATA_GET_MEMORY_FAILED  = Failed to allocate memory           */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PortInit(INT portTag_i, INT portNumber_i, INT controllerTag_i);

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
/* RETURNS      TRUE    if the device is connected and ready                  */
/*              FALSE   if the device is not ready.                           */
/* ON ERRORS    Returns FALSE. Error condition ignored.                       */
/*                                                                            */
/******************************************************************************/
boolean PortDevConnected (INT portTag_i);


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
STATUS_CODE_T PortReadAltStatusReg (INT portTag_i, UINT8 *data_op);

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
STATUS_CODE_T PortReadStatusReg (INT portTag_i, UINT8 *data_op);

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
STATUS_CODE_T PortReadControlReg (INT portTag_i, UINT8 *data_op);

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
STATUS_CODE_T PortReadErrorReg (INT portTag_i, UINT8 *data_op);

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
STATUS_CODE_T PortReadSStatusReg (INT portTag_i, UINT32 *data_op);

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
STATUS_CODE_T PortReadSErrorReg (INT portTag_i, UINT32 *data_op);

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
STATUS_CODE_T PortReadSControlReg (INT portTag_i, UINT32 *data_op);

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
STATUS_CODE_T PortReadDmaStatusReg (INT portTag_i, UINT8 *data_op);

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
STATUS_CODE_T PortPioReadBytes (INT portTag_i, INT count_i, void * buffer_op);

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
STATUS_CODE_T PortPioWriteBytes (INT portTag_i, INT count_i, void * buffer_ip);

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
STATUS_CODE_T PortWriteTaskfileParams(
    INT portTag_i,
    SATA_TASKFILE_T * taskfile_ip
    );

/******************************************************************************/
/*                                                                            */
/* DESCRIPTION                                                                */
/*              This function reads the parameters of the taskfile            */
/*              to the corresponding port specific registers.                 */
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
STATUS_CODE_T PortReadTaskfileParams(
    INT portTag_i,
    SATA_TASKFILE_T * taskfile_op
    );

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
STATUS_CODE_T PortWriteCommandReg (INT portTag_i, UINT8 cmd_i);

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
STATUS_CODE_T PortWriteControlReg (INT portTag_i, UINT8 value_i);


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
STATUS_CODE_T PortSetDTPR (INT portTag_i, UINT32 upperDTPR_i,\
                            UINT32 lowerDTPR_i );

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
STATUS_CODE_T PortSetDdtePage (INT portTag_i, UINT32 value_i);

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
STATUS_CODE_T PortWriteDmaCommandReg (INT portTag_i, UINT16 command_i);

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
boolean PortInterruptPending (INT portTag_i);

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
STATUS_CODE_T PortClearInterrupt (INT portTag_i);


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
STATUS_CODE_T PortDisableInterrupts (INT portTag_i);


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
STATUS_CODE_T PortEnableInterrupts (INT portTag_i);


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
STATUS_CODE_T PortClearError (INT portTag_i);

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
STATUS_CODE_T PortIntHardReset(INT portTag_i);

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
STATUS_CODE_T PortBistStatus(INT portTag_i);

#endif /* SATA_PORT_HEADER */
