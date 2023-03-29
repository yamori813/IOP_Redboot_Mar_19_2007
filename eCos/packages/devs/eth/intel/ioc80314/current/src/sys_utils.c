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
 * FILENAME: sys_utils.c
 *
 * ORIGINATOR: Alex Bounine
 * COMPILER(S): GNUPro (gcc)
 * COMPILER VERSION: xscale-020523
 *
 *  $Revision: 1.8 $
 *      $Date: 2003/03/13 19:47:25 $
 *
 * DESCRIPTION: Contains system level routines used by test components.
 *
 *   History  :
 *   $Log: sys_utils.c,v $
 *   Revision 1.8  2003/03/13 19:47:25  alexb
 *   Moved shared definitions
 *
 *   Revision 1.7  2003/02/08 22:19:18  alexb
 *   Updated function cleanDCache()
 *
 *   Revision 1.6  2003/01/15 20:21:34  alexb
 *   Intermediate check-in
 *
 *   Revision 1.5  2002/12/31 19:08:45  alexb
 *   Intermediate check-in
 *
 *   Revision 1.4  2002/12/19 18:41:43  alexb
 *   Intermediate check-in
 *
 *   Revision 1.3  2002/12/11 14:16:18  alexb
 *   Update for the DMA tests
 *
 *   Revision 1.2  2002/11/18 21:11:23  alexb
 *   Update
 *
 *   Revision 1.1  2002/10/10 20:10:42  alexb
 *   Initial revision
 *
 *
 *---------------------------------------------------------------------------*/


#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_intel_ioc80314.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
// So we can check the validity of the PCI window against the MLTs opinion,
// and thereby what the malloc heap consumes willy-nilly:
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif

#include <cyg/devs/eth/ioc80314_info.h>


typedef struct _MEM_MAP_ENTRY
{
    UINT32 proc_va_start; // procesor virtual address
    UINT32 proc_va_end;   // procesor virtual address

    UINT32 ocn_sa_m;    // most significant part of OCN start address
    UINT32 ocn_sa_l;    // least significant part of OCN start address

    UINT32 ocn_ea_m;    // most significant part of OCN end address
    UINT32 ocn_ea_l;    // least significant part of OCN end address

    UINT32 ocn_port;      // OCN port number
} MEM_MAP_ENTRY;


MEM_MAP_ENTRY eb_memory_map[] = {
// SDRAM (normally mapped cacheable)
{
HAL_IOC80314_SDRAM_VIR_BASE, (HAL_IOC80314_SDRAM_VIR_BASE + SDRAM_SIZE - 1), // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x1FFFFFFF, // ocn end address
PORT_SDRAM              // ocn port number
},
// Non-cacheable SDRAM alias
{
0x20000000, 0x3FFFFFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x1FFFFFFF, // ocn end address
PORT_SDRAM              // ocn port number
},
// FLASH
{
0x40000000, 0x4007FFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x0007FFFF, // ocn end address
PORT_PBI                // ocn port number
},
// SRAM
/*{
0x50000000, 0x5000FFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x0000FFFF, // ocn end address
PORT_XSCALE             // ocn port number
},*/
// Non-cacheable SRAM alias
{
0x50200000, 0x5020FFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x0000FFFF, // ocn end address
PORT_XSCALE             // ocn port number
},
// Switching Fabric SDRAM alias
{
0x60000000, 0x7FFFFFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x1FFFFFFF, // ocn end address
PORT_SDRAM              // ocn port number
},
// PCI1 MEM32
{
0x80000000, 0x9FFFFFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x1FFFFFFF, // ocn end address
PORT_PCI1               // ocn port number
},
// PCI1 PFM1
{
0xA0000000, 0xAFFFFFFF, // xs start/end address
0x00000000, 0x40000000, // ocn start address
0x00000000, 0x4FFFFFFF, // ocn end address
PORT_PCI1               // ocn port number
},
// PCI1 PFM2
{
0xB0000000, 0xBFFFFFFF, // xs start/end address
0x00000000, 0x80000000, // ocn start address
0x00000000, 0x8FFFFFFF, // ocn end address
PORT_PCI1               // ocn port number
},

// PCI2 MEM32
{
0xC0000000, 0xDFFFFFFF, // xs start/end address
0x00000000, 0x00000000, // ocn start address
0x00000000, 0x1FFFFFFF, // ocn end address
PORT_PCI2               // ocn port number
},

// PCI2 PFM1
{
0xE0000000, 0xEFFFFFFF, // xs start/end address
0x00000000, 0x40000000, // ocn start address
0x00000000, 0x4FFFFFFF, // ocn end address
PORT_PCI2               // ocn port number
},

// PCI2
{
0xF0000000, 0xFFFFFFFF, // xs start/end address
0x00000000, 0x80000000, // ocn start address
0x00000000, 0x8FFFFFFF, // ocn end address
PORT_PCI2               // ocn port number
}

};


/*============================================================================
 *  Function    : sysVirtualToOcn()
 *
 *  Arguments   : xs_addr - processor address for which the corresponding OCN
 *                          coordinates (address:port) have to be defined.
 *                ocn_addr_m - pointer to location (processor address) where
 *                             the most significant part of the OCN address will
 *                             returned.
 *                ocn_addr_l - pointer to location (processor address) where
 *                             the least significant part of the OCN address will
 *                             returned.
 *                ocn_port - pointer to location (processor address) where
 *                           the number of the corresponding OCN port will
 *                           returned.
 *
 *  Description : This function converts an address in the processor address
 *                space into an OCN adrress:port combination that can be
 *                used by independent masters on other OCN ports (DMA, GIGE)
 *
 *  Return      : 0 if the processor address was mapped, othrwise 0xFFFF_FFFF
 *
 ===========================================================================*/

UINT32 sysVirtualToOcn(
    void* xs_addr,
    UINT32* ocn_addr_m,
    UINT32* ocn_addr_l,
    UINT32* ocn_port
    )
{
unsigned int i;
UINT32 ret_val = 0xFFFFFFFF;

    // Walk through the memory map to find matching entry
    for (i = 0; i < (sizeof(eb_memory_map)/sizeof(MEM_MAP_ENTRY)); i++)
    {
        if (
            (UINT32)xs_addr >= eb_memory_map[i].proc_va_start &&
            (UINT32)xs_addr <= eb_memory_map[i].proc_va_end
            )
        {
            *ocn_addr_m = eb_memory_map[i].ocn_sa_m;
            *ocn_addr_l = eb_memory_map[i].ocn_sa_l +
                ((UINT32)xs_addr - eb_memory_map[i].proc_va_start);
            *ocn_port =   eb_memory_map[i].ocn_port;
            ret_val = 0;
            break;
        }
    }

    return ret_val;
}


/*============================================================================
 *  Function    : sysOcnToVirtual()
 *
 *  Arguments   : ocn_addr_m - most significant part of the OCN address.
 *                ocn_addr_l - least significant part of the OCN address.
 *                ocn_port   - number of the corresponding OCN port.
 *
 *  Description : This function converts an OCN address:port combination into
 *                an address in the processor address space.
 *
 *  Return      : An address in the processor space or 0xFFFFFFFF if unable
 *                to map specified OCN address.
 *
 ===========================================================================*/

void* sysOcnToVirtual(
    unsigned ocn_addr_m,
    unsigned ocn_addr_l,
    unsigned ocn_port
    )
{
UINT32 ret_val = 0xFFFFFFFF;
UINT32 xs_addr;
unsigned int i;

    // Walk through the memory map to find matching entry
    for (i = 0; i < (sizeof(eb_memory_map)/sizeof(MEM_MAP_ENTRY)); i++)
    {
        if (
            ocn_port == eb_memory_map[i].ocn_port &&
            ocn_addr_m == eb_memory_map[i].ocn_sa_m &&
            ocn_addr_l >= eb_memory_map[i].ocn_sa_l &&
            ocn_addr_l <= eb_memory_map[i].ocn_ea_l
           )
        {
            xs_addr = eb_memory_map[i].proc_va_start +
                        (ocn_addr_l - eb_memory_map[i].ocn_sa_l);

            if (xs_addr <= eb_memory_map[i].proc_va_end)
            {
                ret_val = xs_addr;
            }

            break;
        }
    }

    return (void*)ret_val;
}


/*============================================================================
 *  Function    : cleanDCache()
 *
 *  Arguments   : none
 *
 *  Description : Cleans and invalidates Data Cache.
 *
 *  Return      : none
 *
 ===========================================================================*/

void cleanDCache(void)
{
int i;
UINT32 address = DATA_CACHE_FLUSH_ADDR;

    for(i = 0; i < DATA_CACHE_SIZE/DCACHE_LINE_SIZE; i++)
    {
        _Allocate_D_Cache_Line(address);
        address += DCACHE_LINE_SIZE;
    }

    _Invalidate_D_Cache;
    _Drain_Write_Buffer;
}

