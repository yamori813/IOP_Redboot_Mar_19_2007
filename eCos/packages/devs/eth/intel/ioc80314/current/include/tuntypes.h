/*****************************************************************************
 *
 * Tundra Semiconductor Corporation
 * 
 * Copyright 2002, 2003, Tundra Semiconductor Corporation
 * 
 * This software is the property of Tundra Semiconductor Corporation
 * (Tundra), which specifically grants the user the right to use this
 * software in connection with the Intel(r) 80314 Companion Chip, provided
 * this notice is not removed or altered.  All other rights are reserved
 * by Tundra.
 * 
 * TUNDRA MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD
 * TO THIS SOFTWARE.  IN NO EVENT SHALL AMD BE LIABLE FOR INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING FROM THE
 * FURNISHING, PERFORMANCE, OR USE OF THIS SOFTWARE.
 * 
 *****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME: tuntypes.h
*
* DESCRIPTION: Contains type definitions used in Tundra libraries
*
* HISTORY:
*
*	Rev. Date       Author  Description
*	---- ---------  ------  ------------------------------------------
* 1.00 Feb.08,02  nvo     Initial creation.
* 1.01 Sept.25,02  on     Redefined BOOL types.
*---------------------------------------------------------------------------*/

#ifndef __tuntypes
#define __tuntypes

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __INCvxTypesOldh

typedef	char	INT8;
typedef	short	INT16;
typedef	int		 INT32;

typedef	unsigned char	 UINT8;
typedef	unsigned short	UINT16;
//typedef	unsigned int	  UINT32;

typedef	unsigned char	 UCHAR;
typedef unsigned short	USHORT;
typedef	unsigned int	  UINT;
typedef unsigned long	 ULONG;

/*
#ifndef TRUE
  typedef enum {
    FALSE = 0,
    TRUE = 1
  } BOOL;
#endif
*/
typedef	int		 STATUS;

typedef void		VOID;

#if 0
#ifdef __cplusplus
typedef int 		(*FUNCPTR) (...);     /* ptr to function returning int */
typedef void 		(*VOIDFUNCPTR) (...); /* ptr to function returning void */
typedef double 		(*DBLFUNCPTR) (...);  /* ptr to function returning double*/
typedef float 		(*FLTFUNCPTR) (...);  /* ptr to function returning float */
#else
typedef int 		(*FUNCPTR) ();	   /* ptr to function returning int */
typedef void 		(*VOIDFUNCPTR) (); /* ptr to function returning void */
typedef double 		(*DBLFUNCPTR) ();  /* ptr to function returning double*/
typedef float 		(*FLTFUNCPTR) ();  /* ptr to function returning float */
#endif			/* _cplusplus */
#endif // if 0

#endif

#ifndef __tunvolatile

#define __tunvolatile

typedef volatile UCHAR VCHAR;   /* shorthand for volatile UCHAR */
typedef volatile INT32 VINT32; /* volatile unsigned word */
typedef volatile INT16 VINT16; /* volatile unsigned halfword */
typedef volatile INT8 VINT8;   /* volatile unsigned byte */
//typedef volatile UINT32 VUINT32; /* volatile unsigned word */
typedef volatile UINT16 VUINT16; /* volatile unsigned halfword */
typedef volatile UINT8 VUINT8;   /* volatile unsigned byte */

#endif

#ifdef __cplusplus
}
#endif

#endif /* __tuntypes */
