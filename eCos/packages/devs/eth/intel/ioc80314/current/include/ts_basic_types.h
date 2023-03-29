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

/*****************************************************************************
 *****************************************************************************
 **
 **   Revision : $Revision: 1.1 $
 **   Author   : $Author: bwood $
 **   Date     : $Date: 2002/09/06 18:08:51 $
 **   Purpose  : This file contains the basic types for all Tundra
 **              deliverables used for StrongPac.
 **   History  :
 **   $Log: ts_basic_types.h,v $
 **   Revision 1.1  2002/09/06 18:08:51  bwood
 **   Initial revision
 **
 **
 **
 *****************************************************************************
 *****************************************************************************/


#ifndef _TS_BASIC_TYPES_H_
#define _TS_BASIC_TYPES_H_

#ifdef __ASSEMBLER__
#define UINT32
#else
typedef enum {FALSE=0, TRUE=1}          BOOL;
typedef unsigned int                UINT32;
#endif // __ASSEMBLER__

#endif /* _TS_BASIC_TYPES_H_ */
