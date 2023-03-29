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

/*-----------------------------------------------------------------------------
* FILENAME: 1.	SpacRedBoot.h
*
* DESCRIPTION: This file contains the abstractions for various unix like 
*              library calls.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     removed SYS_Nanosleep macro definition.
*----------------------------------------------------------------------------*/

#ifndef _SPAC_REDBOOT_
#define _SPAC_REDBOOT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
//#include <netinet/in.h>		/* for ntoh and hton macros */
//#include <arpa/inet.h>		/* for inet_ntoa inet_aton */
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <sys/time.h>
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <malloc.h> 
//#include <fcntl.h>
//#include <errno.h>                                                                                
//#include <time.h>
//#include <net/if.h>
//#include <semaphore.h>

//#define PRINTF(format, args...)   printf(format,##args)
//#define SPRINTF(format, args...)  sprintf(format,##args)
#define STRNCPY(dst, src, size)   strncpy(dst, src, size)
#define STRNCMP(dst, src, size)   strncmp(dst, src, size)
#define STRCPY(dst, src)          strcpy(dst, src)
#define STRCAT(dst, src)          strcat(dst, src)
#define STRCMP(s1, s2)            strcmp(s1, s2)
#define STRLEN(s)                 strlen(s)
#define MEMCPY(dst, src, size)    memcpy(dst, src, size)
#define MEMCMP(s1, s2, size)      memcmp(s1, s2, size)
#define MEMSET(s, c, size)        memset(s, c, size)
#define ALLOC(size)               malloc(size)
#define MEMALIGN(alignment, size) malloc(size)
#define FREE_MEM(ptr,size)        free(ptr)
#define EXIT(rc)                  exit(rc)
//#define INET_ATON(str, IP)	      inet_aton(str, IP)
//#define INET_NTOA(IP)	            inet_ntoa(IP)


#endif /* _SPAC_REDBOOT_ */
