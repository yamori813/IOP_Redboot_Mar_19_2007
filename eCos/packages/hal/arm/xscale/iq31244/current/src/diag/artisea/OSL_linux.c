/************************************************************************;
;*
;*      Filename: osl.c
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
/*   DESCRIPTION: osl.c                                                       */
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

/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/
#include <OSL.h>
#include <sata_debug.h>
#include <linux/ide.h>
#include <linux/slab.h>
#include <linux/string.h>        /* memset, memcpy */
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/system.h>

/******************************************************************************/
/*                                                                            */
/*       G L O B A L   F U N C T I O N   I M P L E M E N T A T I O N S        */
/*                                                                            */
/******************************************************************************/

ide_set_handler_t *ide_set_handler_fptr;

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
				  )
{
    DEBUG_TRACE_4();

    /* NPG: TBD: Consider the use of vmalloc rather than kmalloc */
    /* NPG: vmalloc could cause page swapping -> disk activity   */
    /*      for a disk driver, this could cause system lockup    */

    return kmalloc(size_i, GFP_KERNEL);
}
				
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
						)
{
    DEBUG_TRACE_4();

/*    return kmalloc(size_i, GFP_BUFFER | GFP_DMA);*/
    return kmalloc(size_i, GFP_KERNEL | GFP_DMA);
}

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
						  )
{
    DEBUG_TRACE_4();
    kfree(pointer_i);
    return SATA_SUCCESS;
}

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
/*                Virtual Address of the location whose bus address is needed.*/
/*                                                                            */
/******************************************************************************/
BUS_ADDR_T VirtualToBus(
						  void * pointer_i
						  )
{
    unsigned long virt_addr ;

    DEBUG_TRACE_4();

    virt_addr = virt_to_bus((volatile void*)pointer_i);

    /* Typecast before return */
    return (BUS_ADDR_T) virt_addr;
}

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
void MemCopy( INT length, void * from_ip, void * to_op )
{
    memcpy(to_op, from_ip, length);
}

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
void MemSet( INT length, UINT8 value, void * address_op )
{
    memset(address_op, value, length);
}

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
/*                  Pointer to the local memory (Virtual address) where the   */
/*                  data will be stored.                                      */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T IoRead8(
					  SATA_REG_ADDR_T sourceDataReg_i,      /* From  */
					  UINT8 *data_op                      /* To    */
					  )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}


STATUS_CODE_T IoRead16(
					   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
					   UINT16 *data_op                    /* To    */
					   )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}


STATUS_CODE_T IoRead32(
					   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
					   UINT32 *data_op                    /* To    */
					   )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}


STATUS_CODE_T IoRead64(
					   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
					   UINT64 *data_op                    /* To    */
					   )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}

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
/*                  Pointer to the start address (Virtual memory address) of  */
/*                  the local memory buffer where the data will be stored.    */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T IOReadBytes (
						   INT count_i,                         /* Count */
						   SATA_REG_ADDR_T sourceDataReg_i,     /* From  */
						   void * destinationBuffer_op          /* To    */
						   )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}

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
						 )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}
STATUS_CODE_T IoWrite16 (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT16 value_i                        /* From */
						 )	
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}
STATUS_CODE_T IoWrite32 (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT32 value_i                        /* From */
						 )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}
STATUS_CODE_T IoWrite64 (
						 SATA_REG_ADDR_T destinationDataReg_i,   /* To   */
						 UINT64 value_i                        /* From */
						 )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}

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
/*                  Pointer to the start address (Virtual memory address) of  */
/*                  the local memory buffer where data to transfer is located */
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
							)
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}

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
/*                  Pointer to the start address (Virtual memory address) of  */
/*                  the local memory buffer where the data will be stored.    */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PciMemRead8  (
							SATA_REG_ADDR_T sourceDataReg_i,    /* From  */
							UINT8  *data_op                     /* To    */
							)
{
    DEBUG_TRACE_4();

    *data_op = readb(sourceDataReg_i);

    return SATA_SUCCESS;
}

STATUS_CODE_T PciMemRead16 (
							SATA_REG_ADDR_T sourceDataReg_i,    /* From  */
							UINT16 *data_op                     /* To    */
							)
{
    DEBUG_TRACE_4();

    *data_op = readw(sourceDataReg_i);

    return SATA_SUCCESS;
}


STATUS_CODE_T PciMemRead32 (
							SATA_REG_ADDR_T sourceDataReg_i,    /* From  */
							UINT32 *data_op                     /* To    */
							)
{
    DEBUG_TRACE_4();

    *data_op = readl(sourceDataReg_i);

    return SATA_SUCCESS;
}


STATUS_CODE_T PciMemRead64 (
							SATA_REG_ADDR_T sourceDataReg_i,    /* From  */
							UINT64 *data_op                     /* To    */
							)
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}

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
/*                  Pointer to the start address (Virtual memory address) of  */
/*                  the local memory buffer where the data will be stored.    */
/*                                                                            */
/* RETURNS      STATUS_CODE_T as defined in sata_types.h                      */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
STATUS_CODE_T PciMemReadDataPort (
								  INT count_i,
								  SATA_REG_ADDR_T sourceDataReg_i,
								  void * destinationBuffer_op
								  )
{
    int ctr;
    char * ptr = destinationBuffer_op;

    DEBUG_TRACE_4();

    /* dest, src, num */
    /* memcpy_fromio( destinationBuffer_op, sourceDataReg_i, count_i); */

    for (ctr = count_i; ctr > 0; ctr-- )
    {
        *ptr++ = readb(sourceDataReg_i);
    }
    return SATA_SUCCESS;
}

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
							 SATA_REG_ADDR_T destinationDataReg_i, /* To    */
							 UINT8  value_i                        /* From  */
							 )
{
    DEBUG_TRACE_4();

    writeb(value_i,destinationDataReg_i);

    return SATA_SUCCESS;
}

STATUS_CODE_T PciMemWrite16 (
							 SATA_REG_ADDR_T destinationDataReg_i, /* To    */
							 UINT16 value_i                        /* From  */
							 )
{
    DEBUG_TRACE_4();

    writew(value_i,destinationDataReg_i);

    return SATA_SUCCESS;
}

STATUS_CODE_T PciMemWrite32 (
							 SATA_REG_ADDR_T destinationDataReg_i, /* To    */
							 UINT32 value_i                        /* From  */
							 )
{
    DEBUG_TRACE_4();

    writel(value_i,destinationDataReg_i);

    return SATA_SUCCESS;
}

STATUS_CODE_T PciMemWrite64 (
							 SATA_REG_ADDR_T destinationDataReg_i, /* To    */
							 UINT64 value_i                        /* From  */
							 )
{   /* TBD : Not Yet Implemented */
    DEBUG_TRACE_4();
    WARN_STUB();
    return SATA_FAILURE;
}

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
								  )
{
    int ctr;
    char * ptr = sourceBuffer_ip;
    DEBUG_TRACE_4();

    /* dest, src, num */
    /*memcpy_toio( destinationDataReg_i, sourceBuffer_ip, count_i); */
    for (ctr = count_i; ctr > 0; ctr-- )
    {
        writeb( *(ptr++), destinationDataReg_i);

        /* Write memory Barrier. Do we have to have this ? Slows things down...*/
        wmb();
    }

    return SATA_SUCCESS;
}

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
					 )
{
    ide_drive_t *drive = (ide_drive_t *) OS_Drive;
    ide_handler_t *handler = (ide_handler_t *)Handler;

    DEBUG_TRACE_4();
    ASSERT(OS_Drive != NULL_POINTER);
    ASSERT(Handler != NULL_POINTER);

    if (ide_set_handler_fptr)
    {
        ide_set_handler_fptr(drive, handler, WAIT_CMD, NULL);
        return SATA_SUCCESS;
    }

    return SATA_FAILURE;
}

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
                      )
{
    DEBUG_TRACE_4();
    udelay(count_i);
}

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
                      )
{
    DEBUG_TRACE_4();
    mdelay(count_i);
}


