/************************************************************************;
;*
;*      Filename: osl.h  								
;*
;*      Intel(r) Restricted Use
;*
;*      Intel Sample Serial ATA Code
;*
;*      SCD Advanced Engineering
;*
;*      Copyright (c) 2003, 2004 Intel Corp.
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
/*   DESCRIPTION: osl.h                                                       */
/*                                                                            */
/*         This file contains the declarations of the OS Service Layer (OSL)  */
/*		   API in prototype form.                                             */
/*         The OSL provides all the functionality that is either OS dependent */
/*         or architecture dependent as a set of utility/system APIs to the   */
/*         storage library. The OSL has a different implementation for each   */
/*         OS and possibly for different architectures, although this file    */
/*         and the interfaces contained herein should NOT change.             */
/*         IF these APIs are changed, it will require modification of the     */
/*         SATA library in order to maintain compatibility.                   */
/*		   Fundamentally, the OSL translates generic OS service requests from */
/*		   the SATA Library to OS specific functions.						  */
/*                                                                            */
/******************************************************************************/

#ifndef OSL_HEADER
#define OSL_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver */

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
#define UPPER_32_BITS(addr) (UINT32)(((addr) >> 32) & 0xFFFFFFFF)
#define LOWER_32_BITS(addr) (UINT32)( (addr)        & 0xFFFFFFFF)



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

typedef void (ide_set_handler_t) (void *drive, void *handler, unsigned int timeout, void *expiry);
extern ide_set_handler_t *ide_set_handler_fptr;

/******************************************************************************/
/*                                                                            */
/*                 F U N C T I O N    P R O T O T Y P E S                     */
/*                                                                            */
/******************************************************************************/

/*-------------------------------------*/
/*-------------------------------------*/
/*    Memory Allocation Functions      */
/*-------------------------------------*/
/*-------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION		GetMemory()                                                   */
/*                                                                            */
/* DESCRIPTION  This function gets relocatable memory                         */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   size_i                                                        */
/*                  The size in bytes of the memory block requested.          */
/*                                                                            */
/* RETURNS      Pointer to valid memory block on success,                     */
/*              NULL is returned if memory could not be allocated             */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void * GetMemory (
				  INT size_i		
				  );
				
/******************************************************************************/
/*                                                                            */
/* FUNCTION		GetLockedMemory()                                             */
/*                                                                            */
/* DESCRIPTION  This function gets non-relocatable/non-swappable memory with  */
/*              locked physical addresses.                                    */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   size_i                                                        */
/*                  The size in bytes of the memory block requested.          */
/*                                                                            */
/* RETURNS      Pointer to valid memory block on success,                     */
/*              NULL is returned if memory could not be allocated             */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void * GetLockedMemory (
						INT size_i
						);

/******************************************************************************/
/*                                                                            */
/* FUNCTION		FreeMemory()                                                  */
/*                                                                            */
/* DESCRIPTION  This function frees previously acquired memory by either      */
/*              GetMemory() or GetLockedMemory()                              */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   * pointer_i                                                   */
/*                  Pointer to the start of the previously allocated block.   */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T FreeMemory (
						  void * pointer_i
						  );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		VirtualToBus()                                                */
/*                                                                            */
/* DESCRIPTION  This function Returns the Bus address for the given location. */
/*              This is the address that other devices like a DMA controller  */
/*              can use the access that location.                             */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   * pointer_i                                                   */
/*                  Address of the location whose bus address is needed.      */
/*                                                                            */
/******************************************************************************/
BUS_ADDR_T VirtualToBus(
						  void * pointer_i
						  );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		MemCopy()                                                     */
/*                                                                            */
/* DESCRIPTION  This function copies the contents from one memory region to   */
/*              another.                                                      */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              length                                                        */
/*                  The number of bytes to be copied from "from_p" to "to_p"  */
/*              from_p                                                        */
/*                  Virtual Address of the Source                             */
/*              to_p                                                          */
/*                  Virtual Address of Destination                            */
/*                                                                            */
/******************************************************************************/
void MemCopy( INT length, void * from_ip, void * to_op );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		MemSet()                                                      */
/*                                                                            */
/* DESCRIPTION  This function initializes the contents of a memory region with*/
/*              the provided value.                                           */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS                                                                 */
/*              length                                                        */
/*                  The length og the memory region to be initialized.        */
/*              value                                                         */
/*                  The Byte value the memory is to be initialized to.        */
/*              address_op                                                    */
/*                  Virtual Address of memory region                          */
/*                                                                            */
/******************************************************************************/
void MemSet( INT length, UINT8 value, void * address_op );

/*-------------------------------------*/
/*-------------------------------------*/
/*    PIO Memory Read/Write Functions  */
/*-------------------------------------*/
/*-------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION		IoReadxx()                                                    */
/*                                                                            */
/* DESCRIPTION  These functions read memory from IO space in the quantity     */
/*              designated by the xx extension where xx represents bits.      */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   sourceDataReg_i                                               */
/*                  Architecture independent IO address of the data register  */
/*                  to be read.                                               */
/*              *data_op                                                      */
/*                  Pointer to the local memory address where the data will   */
/*                  be stored.                                                */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T IoRead8(
					  SATA_REG_ADDR_T sourceDataReg_i,      /* From  */
					  UINT8 *data_op                      /* To    */
					  );
STATUS_CODE_T IoRead16(
					   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
					   UINT16 *data_op                    /* To    */
					   );
STATUS_CODE_T IoRead32(
					   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
					   UINT32 *data_op                    /* To    */
					   );
STATUS_CODE_T IoRead64(
					   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
					   UINT64 *data_op                    /* To    */
					   );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		IOReadBytes()                                                 */
/*                                                                            */
/* DESCRIPTION  This function is designed primarily to perform a PIO read of  */
/*              the data register in the IDE command register block. Thus,    */
/*              the source register location will be read for the total number*/
/*              of bytes requested and the data read will be placed into a    */
/*              destination buffer.                                           */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   count_i                                                       */
/*                  Number of bytes to read.                                  */
/*              sourceDataReg_i                                               */
/*                  Architecture independent IO address of the data register  */
/*                  to be read.                                               */
/*              * destinationBuffer_op                                        */
/*                  Pointer to the start address of the local memory buffer   */
/*                  where the data will be stored.                            */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T IOReadBytes (
						   INT count_i,                         /* Count */
						   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
						   void * destinationBuffer_op          /* To    */
						   );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		IoWritexx()                                                   */
/*                                                                            */
/* DESCRIPTION  These functions write memory into IO space in the quantity    */
/*              designated by the xx extension where xx represents bits.      */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   destinationDataReg_i                                          */
/*                  Architecture independent IO address of the data register  */
/*                  to be written.                                            */
/*              value_i                                                       */
/*                  Value to write                                            */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T IoWrite8  (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT8  value_i                        /* From */
						 );
STATUS_CODE_T IoWrite16 (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT16 value_i                        /* From */
						 );	
STATUS_CODE_T IoWrite32 (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT32 value_i                        /* From */
						 );
STATUS_CODE_T IoWrite64 (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT64 value_i                        /* From */
						 );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		IOWritebytes()                                                */
/*                                                                            */
/* DESCRIPTION  This function is designed primarily to perform a PIO write to */
/*              the data register in the IDE command register block. Thus,    */
/*              the destination register location will be written for the     */
/*              total number of bytes requested and the data to be written is */
/*              pointed to by the source pointer.                             */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   count_i                                                       */
/*                  Number of bytes to write                                  */
/*              * sourceBuffer_ip                                             */
/*                  Pointer to the start address of the local memory buffer   */
/*                  where the data to transfer is located.                    */
/*              destinationDataReg_i                                          */
/*                  Architecture independent IO address of the data register  */
/*                  to be written.                                            */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T IOWriteBytes (
							INT count_i,                           /* Count  */
							void * sourceBuffer_ip,                /* From   */
							SATA_REG_ADDR_T destinationDataReg_i   /* To     */
							);

/*-------------------------------------*/
/*-------------------------------------*/
/*  Memory Mapped Read/Write Functions */
/*-------------------------------------*/
/*-------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION		PciMemReadxx()                                                */
/*                                                                            */
/* DESCRIPTION  These functions read memory mapped PCI space in the quantity  */
/*              designated by the xx extension where xx represents bits.      */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   sourceDataReg_i                                               */
/*                  Architecture independent address of the PCI memory        */
/*                  to be read.                                               */
/*              *data_op                                                      */
/*                  Pointer to the local memory address where the data will   */
/*                  be stored.                                                */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PciMemRead8  (
							SATA_REG_ADDR_T sourceDataReg_i,      /* From  */
							UINT8  *data_op                     /* To    */
							);
STATUS_CODE_T PciMemRead16 (
							SATA_REG_ADDR_T sourceDataReg_i,      /* From  */
							UINT16 *data_op                     /* To    */
							);
STATUS_CODE_T PciMemRead32 (
							SATA_REG_ADDR_T sourceDataReg_i,      /* From  */
							UINT32 *data_op                     /* To    */
							);
STATUS_CODE_T PciMemRead64 (
							SATA_REG_ADDR_T sourceDataReg_i,      /* From  */
							UINT64 *data_op                     /* To    */
							);

/******************************************************************************/
/*                                                                            */
/* FUNCTION		PciMemReadDataPort()                                          */
/*                                                                            */
/* DESCRIPTION  This function is designed primarily to perform a read of      */
/*              the data register in the "shadow" IDE command register block. */
/*              Located in PCI memeory space. Thus,the source data register   */
/*              location will be read for the total number of bytes requested */
/*              and the data read will be placed into a destination buffer.   */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   count_i                                                       */
/*                  Number of bytes to read.                                  */
/*              sourceDataReg_i                                               */
/*                  Architecture independent IO address of the data register  */
/*                  to be read.                                               */
/*              * destinationBuffer_op                                        */
/*                  Pointer to the start address of the local memory buffer   */
/*                  where the data will be stored.                            */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PciMemReadDataPort (
								  INT count_i,
								  SATA_REG_ADDR_T sourceDataReg_i,
								  void * destinationBuffer_op
								  );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		PciMemWritexx()                                               */
/*                                                                            */
/* DESCRIPTION  These functions write memory into PCI memory space in the     */
/*              quantity designated by the xx extension where xx represents   */
/*              bits.                                                         */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   destinationDataReg_i                                          */
/*                  Architecture independent IO address of the data register  */
/*                  to be written.                                            */
/*              value_i                                                       */
/*                  Value to write                                            */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PciMemWrite8  (
							 SATA_REG_ADDR_T destinationDataReg_i,   /* To    */
							 UINT8  value_i                        /* From  */
							 );
STATUS_CODE_T PciMemWrite16 (
							 SATA_REG_ADDR_T destinationDataReg_i,   /* To    */
							 UINT16 value_i                        /* From  */
							 );
STATUS_CODE_T PciMemWrite32 (
							 SATA_REG_ADDR_T destinationDataReg_i,   /* To    */
							 UINT32 value_i                        /* From  */
							 );
STATUS_CODE_T PciMemWrite64 (
							 SATA_REG_ADDR_T destinationDataReg_i,   /* To    */
							 UINT64 value_i                        /* From  */
							 );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		PciMrmWriteDataPort()                                         */
/*                                                                            */
/* DESCRIPTION  This function is designed primarily to perform a  write into  */
/*              the data register in the IDE command register "shadow" block  */
/*              located in PCI memory. Thus, the destination register         */
/*              location will be written for the total number of bytes        */
/*              requested and the data to be written is pointed to by the     */
/*              source buffer pointer.                                        */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   count_i                                                       */
/*                  Number of bytes to write                                  */
/*              * sourceBuffer_ip                                             */
/*                  Pointer to the start address of the local memory buffer   */
/*                  where the data to transfer is located.                    */
/*              destinationDataReg_i                                          */
/*                  Architecture independent PCI address of the data register */
/*                  to be written.                                            */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PciMemWriteDataPort(
								  INT count_i,
								  void * sourceBuffer_ip,
								  SATA_REG_ADDR_T destinationDataReg_i
								  );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		SetISR()                                                      */
/*                                                                            */
/* DESCRIPTION  This function provides a method for the library to pass a     */
/*              OS specific ISR callback address into the OS. In some OS's,   */
/*              the interrupt for command completion is shared among multiple */
/*              controllers.  Thus, when the OSDM calls the SATA Library, the */
/*              library must have a way to call back into the OSDM in context.*/
/*              A pointer to an associated drive structure is passed down     */
/*              from the OSDM to the Library.  That pointer is then           */
/*              registered as a callback parameter by the library when it     */
/*              sends a command for that drive. The pointer passed down is    */
/*              *OS_Drive.  In addition, the handler pointer is sent from OSDM*/
/*              to library, and registered with the OS.  This routine may not */
/*              be used by all OS's.                                          */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   *OS_Drive                                                     */
/*                  Pointer to data structure associated to a drive by the OS */
/*              *Handler                                                      */
/*                  Pointer to interrupt handler in the OS                    */
/*                  Set the default Handler if this value is NULL_POINTER     */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T SetISR(
					 void * OS_Drive,
					 void * Handler
					 );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		MicroSecondSleep()                                            */
/*                                                                            */
/* DESCRIPTION  This function is used to intorduce smallMicrosecond level     */
/*              delays.                                                       */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   count_i                                                       */
/*                  Number of microseconds to sleep                           */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void MicrosecondSleep(
                      LONG count_i
                      );

/******************************************************************************/
/*                                                                            */
/* FUNCTION		MillisecondSleep()                                            */
/*                                                                            */
/* DESCRIPTION  This function is used to intorduce small Millisecond level    */
/*              delays.                                                       */
/*                                                                            */
/* PRECONDITIONS    none                                                      */
/*                                                                            */
/* POST-CONDITIONS  none                                                      */
/*                                                                            */
/* PARAMETERS   count_i                                                       */
/*                  Number of Milliseconds to sleep                           */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void MillisecondSleep(
                      LONG count_i
                      );
#endif /* OSL_HEADER */
