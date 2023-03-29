/************************************************************************;
;*
;*      Filename: sata_types.h  								
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
/*       This file contains the definations of standard types to be used for  */
/*       development of the SATA code. The purpose of using these types       */
/*       throughout the program instead of the standard C data types is       */
/*       for easier portability to different architectures and OSes.          */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_TYPES_HEADER
#define SATA_TYPES_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                C O N S T A N T   D E F I N I T I O N S                     */
/*                                                                            */
/******************************************************************************/

#define DDT_MAX_ENTRIES   (8192) /* DDT can be max of 64Kb, 8 Bytes per entry */
#define DDT_ENTRY_EOT_BIT (15)   /* DMA Descriptor Table Entry EOT bit        */
#define DDT_ENTRY_MAX_COUNT (0xfffe) /* Even number of bytes, max 64K         */

/* Artisea DMA Specific Bit positions are defined in artisea.h */


/******************************************************************************/
/*                                                                            */
/*                   M A C R O   D E F I N I T I O N S                        */
/*                                                                            */
/******************************************************************************/
/*
 * All pointers should be initialized to the following macro
 * if there is no other value that it should be initialized too.
 * This will allow the pointer initialization value to be
 * easily changed to an address that can cause an exception
 * to aid debugging of runaway pointers.
 */
#define NULL_POINTER (void *)(0)


/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/

/* Type defines shared with other driver code */

/*************************************/
/*  common to all OSes               */
/*************************************/


#ifndef CHAR
typedef char            CHAR;
#endif

#ifndef UCHAR
typedef unsigned char   UCHAR;
#endif

#ifndef INT8
typedef signed char     INT8;
#endif

#ifndef UINT8
typedef unsigned char   UINT8;
#endif

#ifndef INT16
typedef short           INT16;
#endif

#ifndef UINT16
typedef unsigned short  UINT16;
#endif

#ifndef SHORT
typedef short			SHORT;
#endif

#ifndef USHORT
typedef unsigned short  USHORT;
#endif

#ifndef INT
typedef int             INT;
#endif

#ifndef UINT
typedef unsigned int    UINT;
#endif

#ifndef INT32
typedef int             INT32;
#endif

#ifndef UINT32
typedef unsigned int    UINT32;
#endif

#ifndef UINT64
typedef unsigned long long UINT64;
#endif

#ifndef LONG32
typedef long            LONG32;
#endif

#ifndef ULONG32
typedef unsigned long   ULONG32;
#endif

#ifndef LONG
typedef long            LONG;
#endif

#ifndef ULONG
typedef unsigned long   ULONG;
#endif

#ifndef ULONG_PTR
typedef unsigned long   ULONG_PTR;
#endif

#ifndef boolean
typedef int             boolean;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef OK
#define OK 1
#endif

#define UPPER_32_BITS(addr) (( (addr) >> 32) & 0xFFFFFFFF)
#define LOWER_32_BITS(addr) (  (addr)        & 0xFFFFFFFF)

/* The Addresses for Registers on the Artisea Controller */
typedef UINT8 * SATA_REG_ADDR_T;
/* Bus addresses of locations as seen by a DMAC */
typedef UINT32 BUS_ADDR_T;

/******************************************************************************/
/* This type will define the return code values that the SATA functions       */
/* will return.  Success and failure are predefined.  Any other error code    */
/* should be added to this enum.                                              */
/******************************************************************************/
typedef enum STATUS_CODE
{
   SATA_SUCCESS               = 0, /* Indicates the function succeeded        */
   SATA_SUCCESS_COMPLETED     = 0, /* Success, operation completed            */
   SATA_SUCCESS_NOT_COMPLETED = 1, /* Success, operation Not completed(PIO...)*/
   SATA_FAILURE               = 2, /* Indicates the Fatal function failed     */
   SATA_COMPLETED_WITH_ERRORS = 3, /* Task completed with non fatal errors    */
   SATA_GET_MEMORY_FAILED     = 4, /* Failed to get system memory             */
   SATA_FREE_MEMORY_FAILED    = 5, /* Failed to free system memory            */
   SATA_OUT_OF_RANGE_VALUE    = 6, /* Our of Range error                      */
   SATA_NOT_INITIALIZED       = 7, /* SATA Object not initialized / present   */
   SATA_BUSY                  = 8,  /* Destination was busy                    */
   SATA_FALSE                 = 0, /* Same as FALSE                           */
   SATA_TRUE                  = 9,  /* Unique STATUS_CODE. NOT same as FALSE   */
   SATA_HOTPLUG_EVENT		  = 10

} STATUS_CODE_T;


/******************************************************************************/
/*                                                                            */
/* DMA Dedcriptor Table (DDT) Entry structure.                                */
/*     The first DWORD in the entry is the address of a data buffer. Buffers  */
/*     are aligned on WORD boundaries.                                        */
/*     The second field is an even numbered 16 bit byte count of the buffer.  */
/*     The MSB of the last WORD marks the entry as the End-Of-Table entry.    */
/* In a given DDT, two consecutive entries are offset by 16 bytes and are     */
/* aligned on a 4-byte boundary. All entries share the same 4GB page.         */
/*                                                                            */
/******************************************************************************/

typedef struct DDT_ENTRY_S
{
    BUS_ADDR_T baseAddress; /* Base address of Buffer Region                  */
    UINT16   byteCount;   /* Byte count of data Buffer                        */
    UINT16   EOT;       /* Set the DDT_ENTRY_EOT_BIT to 1 on last table entry */
} DDT_ENTRY_T;

/* DDT is managed by OSDM */
/* Note: Zero Copy transfers require that the user allocated buffers be used  */
/* for the data transfers rather than making duplicate copies of the data.    */
/* Some OSes might not support Zero Copy at this time.  Our approach will be  */
/* to let the OS specific deiver decide what to do.  We will just use the     */
/* DDT that is passed down to us.                                             */

typedef struct DDT_S  /* Used by OSDM */
{
    UINT64       ddtPhysicalAddress; /* Physical bus address of the start of the DDT     */
	UINT32		 ddtePage; /* Upper 32 bits of the entries in the DDT */
    DDT_ENTRY_T *ddte;    /* CPU view of DDT */
} DDT_T;

typedef struct DMA_INFO_S /* Used internally by device */
{
    UINT64 ddt;      /* DMA view of the DMA Descriptor Table.     */
    UINT32 ddtePage; /* Top 32 bits of DDT_ENTRY baseAddress      */
    UINT8 status;    /* Copy of the DMA Status Register - SUPDDSR */
} DMA_INFO_T;


// test results
#define NOT_CONNECTED	0
#define TEST_PASSED	1
#define DMA_TIMEOUT		2
#define DATA_ERROR		3
#define DRIVER_ERROR	4


#endif /* SATA_TYPES_HEADER */
