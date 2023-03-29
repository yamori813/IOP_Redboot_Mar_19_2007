//=============================================================================
//
//      i82541.c - Diagnostic support for i82541 NIC
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Mark Salter
// Contributors: dmoseley
// Date:         2003-03-17
// Purpose:     
// Description: 
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#include "test_menu.h"

#ifdef CYGPKG_IO_ETH_DRIVERS

extern int diag_ishex(char theChar);
extern int diag_hex2dec(char hex);
extern CYG_ADDRWORD decIn(void);
static void pci_ether_test (unsigned busno, unsigned devno, unsigned funcno);

#define READMEM8(   _reg_, _val_ ) ((_val_) = *((volatile CYG_BYTE   *)(_reg_)))
#define WRITEMEM8(  _reg_, _val_ ) { CYG_BYTE   *t = (CYG_BYTE   *)(_reg_); *t = (CYG_BYTE  )(_val_); }
#define READMEM16(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define WRITEMEM16( _reg_, _val_ ) {(CYG_WORD16 *t = (CYG_WORD16 *)(_reg_); *t = (CYG_WORD16)(_val_); }
#define READMEM32(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD32 *)(_reg_)))
#define WRITEMEM32( _reg_, _val_ ) { CYG_WORD32 *t = (CYG_WORD32 *)(_reg_); *t = (CYG_WORD32)(_val_); }
#define READMEM64(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD64 *)(_reg_)))
#define WRITEMEM64( _reg_, _val_ ) { CYG_WORD64 *t = (CYG_WORD64 *)(_reg_); *t = (CYG_WORD64)(_val_); }

#define OUTL( _v_, _a_ ) WRITEMEM32( _a_, _v_ )

static inline cyg_uint32 INL(cyg_uint32 io_address)
 {   cyg_uint32 _t_; READMEM32( io_address, _t_ ); return _t_;   }

// ------------------------------------------------------------------------
//
// Serial EEPROM access  - much like the other Intel ethernet
//

#define EE_SHIFT_CLK	0x01            // EEPROM shift clock.
#define EE_CS		0x02            // EEPROM chip select.
#define EE_DATA_WRITE	0x04            // EEPROM chip data in.
#define EE_DATA_READ	0x08            // EEPROM chip data out.
#define EE_REQ          0x40            // EEPROM request (82541 only)
#define EE_GNT          0x80            // EEPROM grant   (82541 only)
#define EE_PRES         0x100           // EEPROM present (82541 only)
#define EE_SIZE         0x200           // EEPROM size    (82541 only)
#define EE_ENB		(0x10|EE_CS)

#define EE_DELAY() do { int z; for ( z = 0; z < 10000; z++ ) ; } while (0)

#if 0
# define EE_PRINTF diag_printf
# define EE_STUFF "%4s | %4s | %4s | %4s [%08x]\n",     \
    (l & EE_SHIFT_CLK) ? "CLK"  : "clk",                      \
    (l & EE_CS) ? "eeCS" : "--",                       \
    (l & EE_DATA_WRITE) ? "eeDW" : "---",                      \
    (l & EE_DATA_READ) ? "eeDR" : "---",                      \
    l & 0xfffff
#else
# define EE_PRINTF( foo )
# define EE_STUFF
#endif

#define I82544_EECD     0x00010

static inline void
ee_select( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    if (l & EE_PRES) {
	// i82541 has EE_PRES bit and requires REQ/GNT before EEPROM access
	l |= EE_REQ;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
	while ((l & EE_GNT) == 0)
	    l = INL( ioaddr + I82544_EECD );
    }
    l &= ~0x3f;
    l |= EE_ENB;
    OUTL( l, ioaddr + I82544_EECD );
    EE_DELAY();
    l |= EE_CS;
    OUTL( l, ioaddr + I82544_EECD );
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_select       : " EE_STUFF  );
    EE_DELAY();
}

static inline void
ee_deselect( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD ) & ~0x3f;
    l |= EE_ENB;
    OUTL( l, ioaddr + I82544_EECD );
    EE_PRINTF( "ee_deselect 1   : " EE_STUFF  );
    EE_DELAY();
    EE_DELAY();
    EE_DELAY();
    l &= ~EE_CS;
    OUTL( l, ioaddr + I82544_EECD );
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_deselect 2   : " EE_STUFF  );
    EE_DELAY();
    EE_DELAY();
    EE_DELAY();
    if (l & EE_REQ) {
	l &= ~EE_REQ;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
    }
}

static inline void
ee_clock_up( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    l |= EE_SHIFT_CLK;
    OUTL( l, ioaddr + I82544_EECD );
    EE_DELAY();
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_clock_up     : " EE_STUFF  );
    EE_DELAY();
}

static inline void
ee_clock_down( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    l &=~ EE_SHIFT_CLK;
    OUTL( l, ioaddr + I82544_EECD );
    EE_DELAY();
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_clock_down   : " EE_STUFF  );
    EE_DELAY();
}

static inline int
ee_read_data_bit( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_read_data    : " EE_STUFF  );
    return EE_DATA_READ == (EE_DATA_READ & l);
}

static inline void
ee_write_data_bit( int ioaddr, int databit )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    if ( databit )
        l |= EE_DATA_WRITE;
    else
        l &= ~EE_DATA_WRITE;
    OUTL( l, ioaddr + I82544_EECD );
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_write_data   : " EE_STUFF  );
    EE_DELAY();
}

// Pass ioaddr around "invisibly"
#define EE_SELECT()              ee_select(ioaddr)
#define EE_DESELECT()            ee_deselect(ioaddr)
#define EE_CLOCK_UP()            ee_clock_up(ioaddr)
#define EE_CLOCK_DOWN()          ee_clock_down(ioaddr)
#define EE_READ_DATA_BIT()       ee_read_data_bit(ioaddr)
#define EE_WRITE_DATA_BIT( _d_ ) ee_write_data_bit(ioaddr,(_d_))

// ------------------------------------------------------------------------

static int
read_eeprom_word( cyg_uint32 ioaddr, int addrbits, int address )
{
    int i, tmp;

    // Should already be not-selected, but anyway:
    EE_SELECT();

    // Shift the read command bits out.
    for (i = 3; i >= 0; i--) { // Doc says to shift out a zero then:
        tmp = (6 & (1 << i)) ? 1 : 0; // "6" is the "read" command.
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }

    // Now clock out address
    for ( i = addrbits - 1; i >= 0 ; i-- ) {
        tmp = (address & (1<<i)) ? 1 : 0;
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        tmp = EE_READ_DATA_BIT();
        EE_CLOCK_DOWN();

//        CYG_ASSERT( (0 == tmp) == (0 == i), "Looking for zero handshake bit" );
    }

    // read in the data
    tmp = 0;
    for (i = 15; i >= 0; i--) {
        EE_CLOCK_UP();
        if ( EE_READ_DATA_BIT() )
            tmp |= (1<<i);
        EE_CLOCK_DOWN();
    }

    // Terminate the EEPROM access.
    EE_DESELECT();
 
#ifdef DEBUG_EE
    diag_printf( "eeprom address %4x: data %4x\n", address, tmp );
#endif

    return tmp;
}

// ------------------------------------------------------------------------
static void
write_eeprom_word( cyg_uint32 ioaddr, int addrbits, int address, cyg_uint16 value)
{
    int i, tmp;
    
    // Should already be not-selected, but anyway:
    EE_SELECT();

    // Shift the write command bits out.
    for (i = 3; i >= 0; i--) {          // Doc says to shift out a zero then:
        tmp = (5 & (1 << i)) ? 1 : 0;   // "5" is the "write" command.
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }

    // Now clock out address
    for ( i = addrbits - 1; i >= 0 ; i-- ) {
        tmp = (address & (1<<i)) ? 1 : 0;
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        tmp = EE_READ_DATA_BIT();
        EE_CLOCK_DOWN();
    }

    
    // write the data
    for (i = 15; i >= 0; i--) {
        tmp = (value & (1 << i)) ? 1 : 0;
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }

    // Terminate the EEPROM access.
    EE_DESELECT();

    CYGACC_CALL_IF_DELAY_US((cyg_int32)15*1000);
}

// ------------------------------------------------------------------------
static void
write_enable_eeprom(cyg_uint32 ioaddr, int addrbits)
{
    int i, tmp;

    // Should already be not-selected, but anyway:
    EE_SELECT();

    // Shift the EWEN command bits out.
    for (i = 5; i >= 0; i--) {           // Doc says to shift out a zero then:
        tmp = (0x13 & (1 << i)) ? 1 : 0; // "0x13" is the "EWEN" command.
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }

    // Shift the rest of the address bits out. (ignored)
    // Two bits were used for the EWEN command above.
    tmp = 0;
    for (i = (addrbits-3); i >= 0; i--) {
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }

    // Terminate the EEPROM access.
    EE_DESELECT();
}


// ------------------------------------------------------------------------
static void
program_eeprom(cyg_uint32 ioaddr, int addrbits, cyg_uint16 *data)
{
    cyg_uint32 i;
    cyg_uint16 checksum = 0;

    // First enable erase/write operations on the eeprom.
    // This is done through the EWEN instruction.
    write_enable_eeprom(ioaddr, addrbits);

    for (i = 0; i < 63; i++, data++) {
	checksum += *data;
	write_eeprom_word(ioaddr, addrbits, i, *data);
    }
    checksum = 0xBABA - checksum;
    write_eeprom_word(ioaddr, addrbits, i, checksum);
}

extern void get_mac_address(char *addr_buf);

static cyg_uint16 eeprom_defaults[] = {
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF,	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

#if 0
static cyg_uint16 eeprom_defaults[] = {
    0xFFFF, 0xFFFF, 0xFFFF, 0x0B00, 0xFFFF, 0x2078, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0x640B, 0x1078, 0x8086, 0x1078, 0x8086, 0x3284,
    0x20DD, 0x5555, 0x0000, 0x2F90, 0x3200, 0x0012, 0x1E20, 0x0012,
    0x1E20, 0x0012, 0x1E20, 0x0012, 0x1E20, 0x0009, 0x0200, 0x0000,
    0x000C, 0x93A6, 0x280B, 0x0000, 0x0800, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0602,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};
#endif

/* Setup Serial EEPROM for Ethernet Configuration */
void
i8254x_enet_setup (cyg_uint32 nic_addr)
{
    cyg_uint16 cksum;
    cyg_uint16 eeprom_data[64];
    int i;

    // Now read complete EEPROM contents
    for (i = cksum = 0; i < 64; i++) {
	eeprom_data[i] = read_eeprom_word(nic_addr, 6, i);
	cksum += eeprom_data[i];
    }

    // If not valid, initialize to defaults
    if (eeprom_data[0x0] != 0x1000) {
	diag_printf ("Current EEPROM contents invalid. Restoring defaults.\n");
	for (i = 0; i < 63; i++)
	    eeprom_data[i] = eeprom_defaults[i];
    }

    get_mac_address((char *)eeprom_data);

    // Now compute checksum on complete EEPROM contents
    for (i = cksum = 0; i < 63; i++)
	cksum += eeprom_data[i];

    eeprom_data[63] = 0xBABA - cksum;

    diag_printf ("Writing to the Serial EEPROM... ");
    program_eeprom(nic_addr, 6, eeprom_data);
    diag_printf ("Done\n");

    /* now that we have finished writing the configuration data, we must ask the
       operator to reset the board to have the configuration changes take effect.
       After the reset, the standard Enet. port diagnostics can be run on the
       board under test */

    diag_printf ("\n******** Reset The Board To Have Changes Take Effect ********\n");
}
#endif // CYGPKG_IO_ETH_DRIVERS
