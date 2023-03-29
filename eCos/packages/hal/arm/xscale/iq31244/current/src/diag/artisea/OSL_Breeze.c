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

#include "sata_types.h"
extern void delay_ms(int num_ms);

/******************************************************************************/
/*                                                                            */
/*       G L O B A L   F U N C T I O N   I M P L E M E N T A T I O N S        */
/*                                                                            */
/******************************************************************************/

//ide_set_handler_t *ide_set_handler_fptr;


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
    *data_op = *(UINT8*)sourceDataReg_i;

    return SATA_SUCCESS;
}


STATUS_CODE_T PciMemRead16 (
							SATA_REG_ADDR_T sourceDataReg_i,    /* From  */
							UINT16 *data_op                     /* To    */
							)
{
    *data_op = *(UINT16*)sourceDataReg_i;

    return SATA_SUCCESS;
}


STATUS_CODE_T PciMemRead32 (
							SATA_REG_ADDR_T sourceDataReg_i,    /* From  */
							UINT32 *data_op                     /* To    */
							)
{
    *data_op = *(UINT32*)sourceDataReg_i;

    return SATA_SUCCESS;
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
unsigned short* ptr = destinationBuffer_op;


	/* in Breeze, we must do 16-bit PCI reads */
    for (ctr = count_i/(sizeof(unsigned short)); ctr > 0; ctr-- )
    {
        *ptr = *(unsigned short*)sourceDataReg_i;
		ptr++;
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

    *(UINT8*)destinationDataReg_i = value_i;

    return SATA_SUCCESS;
}

STATUS_CODE_T PciMemWrite16 (
							 SATA_REG_ADDR_T destinationDataReg_i, /* To    */
							 UINT16 value_i                        /* From  */
							 )
{
    *(UINT16*)destinationDataReg_i = value_i;

    return SATA_SUCCESS;
}

STATUS_CODE_T PciMemWrite32 (
							 SATA_REG_ADDR_T destinationDataReg_i, /* To    */
							 UINT32 value_i                        /* From  */
							 )
{
    *(UINT32*)destinationDataReg_i = value_i;

    return SATA_SUCCESS;
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

    /* dest, src, num */
    /*memcpy_toio( destinationDataReg_i, sourceBuffer_ip, count_i); */
    for (ctr = count_i; ctr > 0; ctr-- )
    {
	    *(UINT8*)destinationDataReg_i = *(ptr++);

    }

    return SATA_SUCCESS;
}




/* delay instead of sleep in breeze */
void MillisecondSleep(int num_ms)
{
	delay_ms(num_ms);
}


/* TODO: how can i make Breeze delay in the microsecond range?  for loop??? */
void MicrosecondSleep(int num_ms)
{
	delay_ms(1);
}
