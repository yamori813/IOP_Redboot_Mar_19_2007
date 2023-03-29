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

#ifndef _SYS_UTILS_H
#define _SYS_UTILS_H

/*----------------------------------------------------------------------------
 * FILENAME: sys_utils.h
 *
 * ORIGINATOR: Alex Bounine
 * COMPILER(S): GNUPro (gcc)
 * COMPILER VERSION: xscale-020523
 *
 *  $Revision: 1.7 $
 *      $Date: 2003/03/13 19:44:50 $
 *
 * DESCRIPTION: Contains definitions for system level routines.
 *
 *   History  :
 *   $Log: sys_utils.h,v $
 *   Revision 1.7  2003/03/13 19:44:50  alexb
 *   Moved shareable definitions
 *
 *   Revision 1.6  2003/01/15 20:21:01  alexb
 *   Update
 *
 *   Revision 1.5  2002/12/31 19:09:08  alexb
 *   Update
 *
 *   Revision 1.4  2002/12/19 18:41:00  alexb
 *   Intermediate check-in
 *
 *   Revision 1.3  2002/12/11 14:16:41  alexb
 *   Update for DMA tests
 *
 *   Revision 1.2  2002/11/18 21:59:31  alexb
 *   Updated
 *
 *   Revision 1.1  2002/10/10 20:11:04  alexb
 *   Initial revision
 *
 *
 *---------------------------------------------------------------------------*/


#ifndef DATA_CACHE_SIZE
    #define DATA_CACHE_SIZE 0x8000
#endif

#ifndef DCACHE_LINE_SIZE
    #define DCACHE_LINE_SIZE 32
#endif

#ifndef DATA_CACHE_FLUSH_ADDR
    #define DATA_CACHE_FLUSH_ADDR 0x50300000
#endif


extern
unsigned sysVirtualToOcn(
    void* xs_addr, unsigned* ocn_addr_m, unsigned* ocn_addr_l, unsigned* ocn_port
    );

extern
void* sysOcnToVirtual(
    unsigned ocn_addr_m, unsigned ocn_addr_l, unsigned ocn_port
    );

extern void cleanDCache(void);

#endif /* _SYS_UTILS_H */
