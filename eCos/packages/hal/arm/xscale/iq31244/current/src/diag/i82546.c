//=============================================================================
//
//      i82546.c - Diagnostic support for i82546 NIC
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

extern void diag_wait(void);
extern int diag_ishex(char theChar);
extern int diag_hex2dec(char hex);
extern CYG_ADDRWORD decIn(void);
static void pci_ether_test (unsigned busno, unsigned devno, unsigned funcno);

#define READMEM8(   _reg_, _val_ ) ((_val_) = *((volatile CYG_BYTE   *)(_reg_)))
#define WRITEMEM8(  _reg_, _val_ ) { CYG_BYTE   *t = (CYG_BYTE   *)(_reg_); *t = (CYG_BYTE  )(_val_); }
#define READMEM16(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define WRITEMEM16( _reg_, _val_ ) { CYG_WORD16 *t = (CYG_WORD16 *)(_reg_); *t = (CYG_WORD16)(_val_); }
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
#define EE_REQ          0x40            // EEPROM request (82546 only)
#define EE_GNT          0x80            // EEPROM grant   (82546 only)
#define EE_PRES         0x100           // EEPROM present (82546 only)
#define EE_SIZE         0x200           // EEPROM size    (82546 only)
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
	// i82546 has EE_PRES bit and requires REQ/GNT before EEPROM access
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
//    diag_printf( "eeprom address %4x: data %4x\n", address, tmp );
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

// Get MAC address from user.
// Acceptable formats are:
//   nn.nn.nn.nn.nn.nn
//   nn:nn:nn:nn:nn:nn
//   nn-nn-nn-nn-nn-nn
//   nn nn nn nn nn nn
//   nnnnnnnnnnnn
static void
get_mac_address(char *addr_buf)
{
    char input[40], mac[6], *p;
    int got, i;

    do {
	got = 0;
	diag_printf ("\nCurrent MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		     addr_buf[0], addr_buf[1], addr_buf[2],
		     addr_buf[3], addr_buf[4], addr_buf[5]);
	diag_printf ("Enter desired MAC address: ");
	while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
	    ;

	p = input;
	while (*p && *p == ' ');

	if (p[0] == '\0' || p[1] == '\0')
	    continue;

	for (; got < 6; got++) {
	    if (!diag_ishex(p[0]) || !diag_ishex(p[1]))
		break;
	    mac[got] = (diag_hex2dec(p[0]) * 16) + diag_hex2dec(p[1]);
	    p += 2;
	    if (*p == '.' || *p == ':' || *p == ' ' || *p == '-')
		p++;
	}
	if (mac[5] & 1) {
	    diag_printf ("MAC address LSB must be even!\n"
                         "Entered address + 1 will be used for second ethernet port.\n\n");
	    got = 0;
	}
    } while (got != 6);

    for (i = 0; i < 6; i++)
	*addr_buf++ = mac[i];
}


static cyg_uint16 eeprom_defaults[] = {
/* halfword addresses! */
/*  0: */  0x0000,  0x0000,  0x0000,  0x0000,
/*  4: */  0x0000,  0x0000,  0x0000,  0x0000,
/*  8: */  0xffff,  0xffff,  0x6402,  0x0701,
/*  C: */  0x113C,  0xffff,  0xffff,  0xf06c,
/* 10: */  0xffff,  0xffff,  0xffff,  0x0000,
/* 14: */  0x14c8,  0xffff,  0xffff,  0xffff,
/* 18: */  0xffff,  0xffff,  0xffff,  0xffff,
/* 1C: */  0xffff,  0xffff,  0xffff,  0xffff,
/* 20: */  0xffff,  0x0021,  0x280c,  0x0000,
/* 24: */  0x04c8,  0xffff,  0xffff,  0xffff,
/* 28: */  0xffff,  0xffff,  0xffff,  0xffff,
/* 2C: */  0xffff,  0xffff,  0xffff,  0x0602,
/* 30: */  0xffff,  0xffff,  0xffff,  0xffff,
/* 34: */  0xffff,  0xffff,  0xffff,  0xffff,
/* 38: */  0xffff,  0xffff,  0xffff,  0xffff,
/* 3C: */  0xffff,  0xffff,  0xffff,  0x0000,
};

/* Setup Serial EEPROM for Ethernet Configuration */
void
enet_setup (MENU_ARG arg)
{
    cyg_pci_device dev_info;
    cyg_pci_device_id devid;
    cyg_uint16 cksum;
    cyg_uint16 eeprom_data[64];
    cyg_uint32 nic_addr;
    int i, bus;

    // First, find NIC at builtin address

    bus = (*ATU_PCIXSR >> 8) & 0xff;
    if (bus == 0xff)
	bus = 0;

    devid = CYG_PCI_DEV_MAKE_ID(bus, CYG_PCI_DEV_MAKE_DEVFN(__NIC, 0));
    cyg_pci_get_device_info(devid, &dev_info);

    if (dev_info.vendor != 0x8086) {
	diag_printf("No i82546 device found\n");
	return;
    }

    nic_addr = dev_info.base_map[0];

    // Now read complete EEPROM contents
    for (i = cksum = 0; i < 64; i++) {
	eeprom_data[i] = read_eeprom_word(nic_addr, 6, i);
	cksum += eeprom_data[i];
    }

    // If not valid, initialize to defaults
    if (cksum != 0xBABA || (eeprom_data[0x0a] & 0xC000) != 0x4000) {
	diag_printf ("Current EEPROM contents invalid. Restoring defaults.\n");
	for (i = 0; i < 63; i++)
	    eeprom_data[i] = eeprom_defaults[i];
    }
	
    get_mac_address((char *)eeprom_data);

    // Now compute checksum on complete EEPROM contents
    for (i = cksum = 0; i < 63; i++)
	cksum += eeprom_data[i];

    diag_printf ("Writing to the Serial EEPROM... ");
    program_eeprom(nic_addr, 6, eeprom_data);
    diag_printf ("Done\n"); 

    /* now that we have finished writing the configuration data, we must ask the
       operator to reset the board to have the configuration changes take effect.
       After the reset, the standard Enet. port diagnostics can be run on the
       board under test */
    
    diag_printf ("\n******** Reset The Board To Have Changes Take Effect ********\n");
}


// i82559 Ethernet test
void
enet_test (MENU_ARG arg)
{
    cyg_pci_device_id  devid[6];
    int	unit = 0;
    int i, num_enet;

    for (i = 0, num_enet = 0; i < 6; i++, num_enet++) {
	if (i == 0)
	    devid[0] = CYG_PCI_NULL_DEVID;
	else
	    devid[i] = devid[i-1]; // start from last one found
	if (!cyg_pci_find_device(0x8086, 0x1008, &devid[i]))
	    break;
    }

    for (; i < 6; i++, num_enet++) {
	if (i == 0)
	    devid[0] = CYG_PCI_NULL_DEVID;
	else
	    devid[i] = devid[i-1]; // start from last one found
	if (cyg_pci_find_device(0x8086, 0x1010, &devid[i]))
            continue;
	if (cyg_pci_find_device(0x8086, 0x1079, &devid[i]))
            continue;
        break;
    }
	
    if (num_enet == 0) {
	diag_printf ("No supported Ethernet devices found\n");
	return;
    }
	
    diag_printf ("Supported Ethernet Devices:\n\n");

    diag_printf (" Unit#  Bus#  Device#  Function#\n");
    diag_printf (" -----  ----  -------  ---------\n");
    for (i = 0; i < num_enet; i++)
	diag_printf ("   %d     %d       %d          %d\n", i,
		     CYG_PCI_DEV_GET_BUS(devid[i]),
		     CYG_PCI_DEV_GET_DEV(CYG_PCI_DEV_GET_DEVFN(devid[i])),
		     CYG_PCI_DEV_GET_FN(CYG_PCI_DEV_GET_DEVFN(devid[i]))); 

    diag_printf ("\nEnter the unit number to test : ");
    unit = decIn();
    diag_printf ("\n");

    pci_ether_test (CYG_PCI_DEV_GET_BUS(devid[unit]),
		    CYG_PCI_DEV_GET_DEV(CYG_PCI_DEV_GET_DEVFN(devid[unit])),
		    CYG_PCI_DEV_GET_FN(CYG_PCI_DEV_GET_DEVFN(devid[unit])));

    diag_printf("\n");
    diag_wait();
}


#define VIRT_TO_BUS( _x_ ) ((cyg_uint32)CYGARC_VIRT_TO_BUS(_x_))
#define BUS_TO_VIRT( _x_ ) ((cyg_uint32)CYGARC_BUS_TO_VIRT(_x_))

// ------------------------------------------------------------------------
//                                                                      
//                   82544 REGISTER OFFSETS
//                                                                      
// ------------------------------------------------------------------------

// General registers
#define I82544_CTRL     0x00000
#define I82544_STATUS   0x00008
#define I82544_EECD     0x00010
#define I82544_CTRL_EXT 0x00018
#define I82544_MDIC     0x00020
#define I82544_FCAL     0x00028
#define I82544_FCAH     0x0002c
#define I82544_FCT      0x00030
#define I82544_VET      0x00038
#define I82544_FCTTV    0x00170
#define I82544_TXCW     0x00178
#define I82544_RXCW     0x00180
#define I82544_PBA      0x01000

// Interrupt control registers
#define I82544_ICR      0x000c0
#define I82544_ICS      0x000c8
#define I82544_IMS      0x000d0
#define I82544_IMC      0x000d8

// Receive registers
#define I82544_RCTL     0x00100
#define I82544_FCRTL    0x02160
#define I82544_FCRTH    0x02168
#define I82544_RDBAL    0x02800
#define I82544_RDBAH    0x02804
#define I82544_RDLEN    0x02808
#define I82544_RDH      0x02810
#define I82544_RDT      0x02818
#define I82544_RDTR     0x02820
#define I82544_RXDCTL   0x02828
#define I82544_RXCSUM   0x05000
#define I82544_MTA      0x05200
#define I82544_RAT      0x05400
#define I82544_VFTA     0x05600

#define I82544_RCTL_EN  (1<<1)
#define I82544_RCTL_BAM (1<<15)

// Transmit registers
#define I82544_TCTL     0x00400
#define I82544_TIPG     0x00410
#define I82544_TBT      0x00448
#define I82544_AIT      0x00458
#define I82544_TXDMAC   0x03000
#define I82544_TDBAL    0x03800
#define I82544_TDBAH    0x03804
#define I82544_TDLEN    0x03808
#define I82544_TDH      0x03810
#define I82544_TDT      0x03818
#define I82544_TIDV     0x03820
#define I82544_TXDCTL   0x03828
#define I82544_TSPMT    0x03830


#define I82544_TCTL_EN  (1<<1)
#define I82544_TCTL_PSP (1<<3)


// ------------------------------------------------------------------------
//
//               82544 DEVICE CONTROL WORD DEFNITIONS
//
// ------------------------------------------------------------------------

#define I82544_CTRL_FD          (1<<0)
#define I82544_CTRL_BEM         (1<<1)
#define I82544_CTRL_LRST        (1<<3)
#define I82544_CTRL_ASDE        (1<<5)
#define I82544_CTRL_SLU         (1<<6)
#define I82544_CTRL_ILOS        (1<<7)
#define I82544_CTRL_SPEED       (3<<8)
#define I82544_CTRL_FRCSPD      (1<<11)
#define I82544_CTRL_FRCDPLX     (1<<12)
#define I82544_CTRL_SWDPINSLO   (15<<18)
#define I82544_CTRL_SWDPINSIO   (15<<22)
#define I82544_CTRL_RST         (1<<26)
#define I82544_CTRL_RFCE        (1<<27)
#define I82544_CTRL_TFCE        (1<<28)
#define I82544_CTRL_VME         (1<<30)
#define I82544_CTRL_PHY_RST     (1<<31)

#define I82544_CTRL_PHY_RESET           (1<<18)
#define I82544_CTRL_PHY_RESET_DIR       (1<<22)
#define I82544_CTRL_MDIO                (1<<20)
#define I82544_CTRL_MDIO_DIR            (1<<24)
#define I82544_CTRL_MDC                 (1<<21)
#define I82544_CTRL_MDC_DIR             (1<<25)

#define I82544_CTRL_EXT_PHY_RESET4      (1<<4)
#define I82544_CTRL_EXT_PHY_RESET_DIR4  (1<<8)

#define PHY_ADDRESS 1

// ------------------------------------------------------------------------
//
//               82544 DEVICE STATUS WORD DEFNITIONS
//
// ------------------------------------------------------------------------

#define I82544_STATUS_FD        0x0001
#define I82544_STATUS_LU        0x0002
#define I82544_STATUS_TXOFF     0x0010
#define I82544_STATUS_TBIMODE   0x0020
#define I82544_STATUS_SPEED     0x00C0
#define I82544_STATUS_ASDV      0x0300
#define I82544_STATUS_PCI_SPD   0x0800
#define I82544_STATUS_BUS64     0x1000
#define I82544_STATUS_PCIX_MODE 0x2000
#define I82544_STATUS_PCIXSPD   0xC000

// ------------------------------------------------------------------------
//
//               RECEIVE DESCRIPTORS
//
// ------------------------------------------------------------------------

#define I82544_RD_BUFFER        0
#define I82544_RD_LENGTH        8
#define I82544_RD_CSUM          10
#define I82544_RD_STATUS        12
#define I82544_RD_ERRORS        13
#define I82544_RD_SPECIAL       14
#define I82544_RD_SIZE          16

#define I82544_RD_STATUS_DD     (1<<0)
#define I82544_RD_STATUS_EOP    (1<<1)
#define I82544_RD_STATUS_IXSM   (1<<2)
#define I82544_RD_STATUS_VP     (1<<3)
#define I82544_RD_STATUS_TCPCS  (1<<5)
#define I82544_RD_STATUS_IPCS   (1<<6)
#define I82544_RD_STATUS_PIF    (1<<7)

// ------------------------------------------------------------------------
//
//               TRANSMIT DESCRIPTORS
//
// ------------------------------------------------------------------------

// Currently we only use the legacy Tx descriptor

#define I82544_TD_BUFFER        0
#define I82544_TD_LENGTH        8
#define I82544_TD_CSO           10
#define I82544_TD_CMD           11
#define I82544_TD_STATUS        12
#define I82544_TD_CSS           13
#define I82544_TD_SPECIAL       14
#define I82544_TD_SIZE          16

#define I82544_TD_CMD_EOP       (1<<0)
#define I82544_TD_CMD_IFCS      (1<<1)
#define I82544_TD_CMD_IC        (1<<2)
#define I82544_TD_CMD_RS        (1<<3)
#define I82544_TD_CMD_RPS       (1<<4)
#define I82544_TD_CMD_DEXT      (1<<5)
#define I82544_TD_CMD_VLE       (1<<6)
#define I82544_TD_CMD_IDE       (1<<7)

#define I82544_TD_STATUS_DD     (1<<0)
#define I82544_TD_STATUS_EC     (1<<1)
#define I82544_TD_STATUS_LC     (1<<2)
#define I82544_TD_STATUS_TU     (1<<3)

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------

#define MAX_RX_PACKET_SIZE  1536        // maximum Rx packet size
#define MAX_TX_PACKET_SIZE  1536        // maximum Tx packet size

static void
udelay(int delay)
{
    CYGACC_CALL_IF_DELAY_US(delay);
}


// ------------------------------------------------------------------------
//
//                     MDIO
//
// Device-specific bit-twiddling and line driving side-effects

// CYGACC_CALL_IF_DELAY_US() drags in huge amounts of scheduler locking and
// the like 'cos it's a VV call!  We only want a delay of 1uS tops, so:

#define MII_DELAY() do { int z; for ( z = 0; z < 100; z++ ) ; } while (0)

#if 0
# define MII_PRINTF diag_printf
# define MII_STUFF "%4s | %4s | %4s | %4s [%08x]\n",    \
    (*_ctrl & (1<<20)) ? "MDIO" : "---",                     \
    (*_ctrl & (1<<24)) ? "Wr" : "Rd",                      \
    (*_ctrl & (1<<21)) ? "CLK" : "clk",                      \
    *_ctrl
#else
# define MII_PRINTF( foo )
# define MII_STUFF
#endif

static inline cyg_uint32
mii_init( int ioaddr )
{
    cyg_uint32 ctrl;
    cyg_uint32 *_ctrl = &ctrl;
    *_ctrl = INL( ioaddr + I82544_CTRL );    
    *_ctrl &=~ I82544_CTRL_MDC;    
    *_ctrl |= I82544_CTRL_MDC_DIR;
    *_ctrl &= ~I82544_CTRL_MDIO_DIR;
    *_ctrl &= ~I82544_CTRL_MDIO;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );    
    MII_PRINTF( "mii_init  : " MII_STUFF  );
    MII_DELAY();
    return *_ctrl;
}

static inline void
mii_clock_up( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl |= I82544_CTRL_MDC;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_clock_up  : " MII_STUFF  );
    MII_DELAY();
}

static inline void
mii_clock_down( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl &=~ I82544_CTRL_MDC;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_clock_down: " MII_STUFF  );
    MII_DELAY();
}

static inline void
mii_read_mode( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl &= ~I82544_CTRL_MDIO_DIR;
    *_ctrl &= ~I82544_CTRL_MDIO;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_read_mode : " MII_STUFF  );
    MII_DELAY();
}

static inline int
mii_read_data_bit( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl = INL( ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_read_data : " MII_STUFF  );
    return I82544_CTRL_MDIO == (I82544_CTRL_MDIO & *_ctrl);
}

static inline void
mii_write_data_bit( int ioaddr, int databit, cyg_uint32 *_ctrl )
{
    if ( databit )
        *_ctrl |= I82544_CTRL_MDIO;
    else
        *_ctrl &= ~I82544_CTRL_MDIO;
    *_ctrl |= I82544_CTRL_MDIO_DIR; // drive the mdio line
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_write_data: " MII_STUFF  );
    MII_DELAY();
}

// Pass ioaddr around "invisibly"
#define MII_INIT()                cyg_uint32 _ctrl_val = mii_init(ioaddr); \
                                  cyg_uint32 *_ctrl = &_ctrl_val;

#define MII_CLOCK_UP()            mii_clock_up(ioaddr, _ctrl)
#define MII_CLOCK_DOWN()          mii_clock_down(ioaddr, _ctrl)
#define MII_READ_MODE()           mii_read_mode(ioaddr, _ctrl)
#define MII_READ_DATA_BIT()       mii_read_data_bit(ioaddr, _ctrl)
#define MII_WRITE_DATA_BIT( _d_ ) mii_write_data_bit(ioaddr,(_d_),_ctrl)

static int
mii_read_register( cyg_uint32 ioaddr, int phy, int regnum )
{
    int value = 0;
    cyg_uint32 mdic = (2<<26) | (phy<<21) | (regnum<<16);

    OUTL( mdic, ioaddr + I82544_MDIC );

    // Wait for ready
    do {
	mdic = INL( ioaddr + I82544_MDIC );
    } while( (mdic & (1<<28)) == 0 );

    value = mdic & 0xFFFF;

    return value;
}

static void
mii_write_register( cyg_uint32 ioaddr, int phy, int regnum, int value )
{
    cyg_uint32 mdic = (1<<26) | (phy<<21) | (regnum<<16) | (value&0xFFFF);

    OUTL( mdic, ioaddr + I82544_MDIC );

    // Wait for ready
    do {
	mdic = INL( ioaddr + I82544_MDIC );
    } while( (mdic & (1<<28)) == 0 );
}



// ------------------------------------------------------------------------
//
//  Function : i82544_reset
//
// ------------------------------------------------------------------------
static void
i82544_reset(cyg_uint32 ioaddr)
{
    cyg_uint32 ctrl;

    ctrl = INL( ioaddr + I82544_CTRL );

    // reset controller
    OUTL( ctrl | I82544_CTRL_RST, ioaddr + I82544_CTRL );
    udelay(20);
    ctrl = INL( ioaddr + I82544_CTRL );
}

static CYG_ADDRESS _heap_base, _heap_free;
static cyg_uint32  _heap_size;

static CYG_ADDRESS
pciwindow_mem_alloc(int size)
{
    CYG_ADDRESS p_memory;
    int _size = size;

    p_memory = 0;
    size = (size + 3) & ~3;
    if ((_heap_free+size) < (_heap_base+_heap_size) ) {
        cyg_uint32 *p;
        p_memory = (CYG_ADDRESS)_heap_free;
        _heap_free += size;
        for ( p = (cyg_uint32 *)p_memory; _size > 0; _size -= 4 )
            *p++ = 0;
    }

    return p_memory;
}


CYG_ADDRESS rx_ring;                // location of Rx descriptors
CYG_ADDRESS tx_ring;                // location of Tx descriptors

#define MAX_RX_DESCRIPTORS 8
#define MAX_TX_DESCRIPTORS 8
#define MAX_RX_PACKET_SIZE 1536
#define MAX_TX_PACKET_SIZE 1536

static cyg_uint32 next_rx_descriptor;
static cyg_uint32 rx_pointer;
static cyg_int32  tx_pointer;

static cyg_uint8  mac_addr[6];


// ------------------------------------------------------------------------
//
//  Function : InitRxRing
//
// ------------------------------------------------------------------------

static void
InitRxRing(cyg_uint32 ioaddr)
{
    int i;
    cyg_uint32 rxctl;
    
    // Allocate array of Rx desriptors
    rx_ring = pciwindow_mem_alloc(MAX_RX_DESCRIPTORS * I82544_RD_SIZE + 32 );

    // assign ring structure, aligning it on a 16 byte boudary.
    rx_ring = (rx_ring + 15) & ~15;
    
    // Allocate and fill in buffer pointers
    for ( i = 0; i < MAX_RX_DESCRIPTORS; i++) {
        CYG_ADDRESS rd = rx_ring + (i*I82544_RD_SIZE);
        CYG_ADDRESS buf = pciwindow_mem_alloc(MAX_RX_PACKET_SIZE);
        WRITEMEM64( rd + I82544_RD_BUFFER, VIRT_TO_BUS(buf) );
    }

    // Set the receiver queue registers

    OUTL( VIRT_TO_BUS(rx_ring), ioaddr + I82544_RDBAL );
    OUTL( 0, ioaddr + I82544_RDBAH );
    OUTL( MAX_RX_DESCRIPTORS * I82544_RD_SIZE, ioaddr + I82544_RDLEN );
    OUTL( 0, ioaddr + I82544_RDH );
    OUTL( MAX_RX_DESCRIPTORS - 5, ioaddr + I82544_RDT );

    // zero out RAT
    for( i = 0; i < 32; i++ )
        OUTL( 0, ioaddr + I82544_RAT +(i*4) );

    // Zero out MTA
    for( i = 0; i < 128; i++ )
        OUTL( 0, ioaddr + I82544_MTA +(i*4) );
    
    // Set up receiver to accept broadcasts
    rxctl = INL( ioaddr + I82544_RCTL );
    rxctl |= I82544_RCTL_BAM;
    OUTL( rxctl, ioaddr + I82544_RCTL );

    next_rx_descriptor = 0;
    rx_pointer = 0;
}

// ------------------------------------------------------------------------
//
//  Function : InitTxRing
//
// ------------------------------------------------------------------------

static void
InitTxRing(cyg_uint32 ioaddr)
{
    int i;
    cyg_uint32 txctl;
    
    // Allocate array of Tx desriptors
    tx_ring = pciwindow_mem_alloc(
        MAX_TX_DESCRIPTORS * I82544_TD_SIZE + 32 );

    // assign ring structure, aligning it on a 16 byte boudary.
    tx_ring = (tx_ring + 15) & ~15;
    
    // Allocate and fill in buffer pointers
    for ( i = 0; i < MAX_TX_DESCRIPTORS; i++) {
        CYG_ADDRESS td = tx_ring + (i*I82544_TD_SIZE);
        WRITEMEM64( td + I82544_TD_BUFFER, 0 );
    }

    // Set the transmitter queue registers

    OUTL( VIRT_TO_BUS(tx_ring), ioaddr + I82544_TDBAL );
    OUTL( 0, ioaddr + I82544_TDBAH );
    OUTL( MAX_TX_DESCRIPTORS * I82544_TD_SIZE, ioaddr + I82544_TDLEN );
    OUTL( 0, ioaddr + I82544_TDH );
    OUTL( 0, ioaddr + I82544_TDT );

    // Set IPG values
    OUTL( 8 | (8<<10) | (6<<20), ioaddr + I82544_TIPG );
    
    // Program tx ctrl register
    txctl = INL( ioaddr + I82544_TCTL );
    txctl |= (15<<4);   // collision threshold
    txctl |= (64<<12);  // collision distance
    txctl |= I82544_TCTL_PSP;
    OUTL( txctl, ioaddr + I82544_TCTL );

    tx_pointer = 0;
}


static void
read_mac_address(cyg_uint32 ioaddr, unsigned char *mac)
{
    int addr_length, i, count;
    cyg_uint16 checksum;

    // read eeprom and get 82544's mac address
    addr_length = 6;

    for (checksum = 0, i = 0, count = 0; count < 64; count++) {
	cyg_uint16 value;
	// read word from eeprom
	value = read_eeprom_word(ioaddr, 6, count);
	checksum += value;
	if (count < 3) {
	    mac[i++] = value & 0xFF;
	    mac[i++] = (value >> 8) & 0xFF;
	}
    }
}


static int
send_packet(cyg_uint32 ioaddr, cyg_uint8 *pkt, cyg_uint16 len)
{
    cyg_int32 txt;
    CYG_ADDRESS dp;
    cyg_uint8 cmd, status;
    int delay;

    // Get the tx tail pointer
    txt = INL( ioaddr + I82544_TDT );

    // Get the descriptor address from the tail pointer.
    dp = tx_ring + (txt * I82544_TD_SIZE);

    HAL_DCACHE_STORE( ((CYG_ADDRESS)pkt) &~(HAL_DCACHE_LINE_SIZE-1),
		      len + (HAL_DCACHE_LINE_SIZE-1) );

    WRITEMEM64( dp + I82544_TD_BUFFER, VIRT_TO_BUS(pkt) );
    WRITEMEM16( dp + I82544_TD_LENGTH, len );

    cmd = I82544_TD_CMD_EOP;
    cmd |= I82544_TD_CMD_IFCS;
    cmd |= I82544_TD_CMD_RS;

    // Write command byte
    WRITEMEM8( dp + I82544_TD_CMD, cmd );
    
    // Zero out rest of fields
    WRITEMEM8( dp + I82544_TD_STATUS, 0 );
    WRITEMEM8( dp + I82544_TD_CSO, 0 );
    WRITEMEM8( dp + I82544_TD_CSS, 0 );
    WRITEMEM16( dp + I82544_TD_SPECIAL, 0 );
    
    txt = (txt + 1) % MAX_TX_DESCRIPTORS;

    // And finally, cause the transmission to happen by setting the
    // tail pointer in the hardware.
    OUTL( txt, ioaddr + I82544_TDT );

    for (delay = 30000; (INL( ioaddr + I82544_TDH ) == tx_pointer) && delay > 0; delay--)
	udelay(1000);

    if (delay <= 0)
	return 0;

    dp = tx_ring + (tx_pointer * I82544_TD_SIZE);

    READMEM8( dp + I82544_TD_CMD, cmd );
    READMEM8( dp + I82544_TD_STATUS, status );

    // Zero out buffer and command
    WRITEMEM64( dp + I82544_TD_BUFFER, 0 );
    WRITEMEM8( dp + I82544_TD_CMD, 0 );

    if( cmd & I82544_TD_CMD_EOP )
	tx_pointer = (tx_pointer + 1) % MAX_TX_DESCRIPTORS;

    return 1;
}


static int
rcv_packet(cyg_uint32 ioaddr, cyg_uint8 *pkt, cyg_uint32 len)
{
    int delay;
    CYG_ADDRESS dp, from_p;
    cyg_uint8 status;
    cyg_uint16 total_len;
    cyg_uint32 rxt;

    for (delay = 2000; (INL( ioaddr + I82544_RDH ) == rx_pointer) && delay > 0; delay--)
	udelay(1000);

    if (delay <= 0)
	return 0;

    // Form packet address
    dp = rx_ring + (rx_pointer * I82544_RD_SIZE);

    // Get status
    READMEM8( dp + I82544_RD_STATUS, status );

    if( status & I82544_RD_STATUS_EOP ) {
	cyg_uint16 length;

	READMEM16( dp + I82544_RD_LENGTH, length );

	next_rx_descriptor = rx_pointer;


	READMEM64( dp + I82544_RD_BUFFER, from_p );
	from_p = BUS_TO_VIRT(from_p);
	READMEM16( dp + I82544_RD_LENGTH, total_len );

	if (total_len > len)
	    total_len = len;

        memcpy(pkt, (unsigned char *)from_p, total_len);
	
    }

    // Advance rxp pointer
    rx_pointer = (rx_pointer + 1 ) % MAX_RX_DESCRIPTORS;

    // We can now also advance the tail pointer by 1
    rxt = INL( ioaddr + I82544_RDT );
    dp = rx_ring + (rxt * I82544_RD_SIZE);
    WRITEMEM8( dp + I82544_RD_STATUS, status );
    rxt = (rxt + 1) % MAX_RX_DESCRIPTORS;
    OUTL(rxt, ioaddr + I82544_RDT);

    return 1;
}

typedef struct {
    cyg_uint8     destination[6];
    cyg_uint8     source[6];
    cyg_uint16    type;
#define _TYPE_IP   0x800
#define _TYPE_ARP  0x806
#define _TYPE_RARP 0x8053
    char          data[1000];
} packet_t;

static void
dump_regs(cyg_uint32 ioaddr)
{
    cyg_uint16 u16_1, u16_2;

    diag_printf("\n");
    diag_printf("CTRL: %08x  STATUS: %08x  CTRL_EXT: %08x\n", 
		INL(ioaddr+I82544_CTRL), INL(ioaddr+I82544_STATUS),
		INL(ioaddr+I82544_CTRL_EXT));
    diag_printf("TXCW: %08x  RXCW: %08x  RCTL: %08x\n",
		INL(ioaddr+I82544_TXCW),INL(ioaddr+I82544_RXCW),
		INL(ioaddr+I82544_RCTL));

    u16_1 = mii_read_register( ioaddr, PHY_ADDRESS, 0 );  // ctrl
    u16_2 = mii_read_register( ioaddr, PHY_ADDRESS, 1 );  // status
    diag_printf("PHY_CTRL: %04x  PHY_STATUS: %04x\n", 
		u16_1, u16_2);

    u16_1 = mii_read_register( ioaddr, PHY_ADDRESS, 16 ); // pscon
    u16_2 = mii_read_register( ioaddr, PHY_ADDRESS, 17 ); // psstat
    diag_printf("PHY_PSCON: %04x  PHY_PSSTAT: %04x\n", 
		u16_1, u16_2);
}


static int
setup_loopback(cyg_uint32 ioaddr, int ext)
{
    cyg_uint32 ctrl;
    cyg_uint16 phy_ctrl;
    int delay;

    OUTL( 0, ioaddr + I82544_TXCW );

#if 1
    ctrl = INL( ioaddr + I82544_CTRL_EXT);
    ctrl |= (1 << 15);
    OUTL( ctrl, ioaddr + I82544_CTRL_EXT );
#endif

    ctrl = INL( ioaddr + I82544_CTRL );
    
    // Set link up bit
    ctrl |= (I82544_CTRL_SLU | I82544_CTRL_FRCSPD | I82544_CTRL_FRCDPLX);
    ctrl &= ~I82544_CTRL_ASDE;   // No speed detect
    ctrl &= ~I82544_CTRL_SPEED;  // 10Mb/s
    ctrl |= I82544_CTRL_FD;      // full-duplex
    ctrl &= ~I82544_CTRL_ILOS;
    OUTL( ctrl, ioaddr + I82544_CTRL );
    udelay(20000);

    ctrl |= I82544_CTRL_PHY_RST;
    OUTL( ctrl, ioaddr + I82544_CTRL );
    udelay(200000);
    ctrl &= ~I82544_CTRL_PHY_RST;
    OUTL( ctrl, ioaddr + I82544_CTRL );
    udelay(200000);

    phy_ctrl = mii_read_register( ioaddr, PHY_ADDRESS, 0 );
    phy_ctrl &= ~((1 << 6) | (1 << 13));  // 10Mb/s
    phy_ctrl |= (1 << 8);                 // full-duplex
    phy_ctrl &= ~((1 << 9) | (1 << 12));
    phy_ctrl |= (1 << 15) ;               // reset
    mii_write_register( ioaddr, PHY_ADDRESS, 0, phy_ctrl );
    udelay(20000);
    while (mii_read_register( ioaddr, PHY_ADDRESS, 0 ) & (1 << 15))
	;

    if (!ext) {
	phy_ctrl &= ~(1 << 15) ;              // reset
	phy_ctrl |= (1 << 14) ;               // loopback
	mii_write_register( ioaddr, PHY_ADDRESS, 0, phy_ctrl );
	udelay(20000);

	phy_ctrl = mii_read_register( ioaddr, PHY_ADDRESS, 16 );
	phy_ctrl |= (1 << 10);   // force link good
	mii_write_register( ioaddr, PHY_ADDRESS, 16, phy_ctrl );
	udelay(20000);

	ctrl = INL( ioaddr + I82544_RCTL );
	ctrl |= 0xc0;   // loopback
	OUTL( ctrl, ioaddr + I82544_RCTL );
    } else {
	phy_ctrl = mii_read_register( ioaddr, PHY_ADDRESS, 16 );
	phy_ctrl |= (1 << 10);   // force link good
	mii_write_register( ioaddr, PHY_ADDRESS, 16, phy_ctrl );
    }

    for (delay = 2000; !(mii_read_register(ioaddr, PHY_ADDRESS, 1) & 4) && delay > 0; delay--)
	udelay(1000);

    if (delay < 0)
	return 0;

    return 1;
}


static void
pci_ether_test (unsigned busno, unsigned devno, unsigned funcno)
{
    cyg_pci_device dev_info;
    cyg_pci_device_id  devid;
    cyg_uint32 ioaddr, ctrl;
    cyg_uint16 phy_stat;
    cyg_uint32 maclo, machi;
    packet_t *tx_pkt, *rx_pkt;
    int i, passed = 1;

    tx_pkt = (packet_t *)0x200000;
    rx_pkt = (packet_t *)0x300000;

    for (i = 0; i < sizeof(tx_pkt->data); i++)
	tx_pkt->data[i] = i;
    
    devid = CYG_PCI_DEV_MAKE_ID(busno, CYG_PCI_DEV_MAKE_DEVFN(devno,funcno));
    cyg_pci_get_device_info(devid, &dev_info);

    ioaddr = dev_info.base_map[0];

    diag_printf ("PCI Base Address  = 0x%X\n", ioaddr);

    diag_printf("Resetting chip... ");
    i82544_reset(ioaddr);
    diag_printf("done.\n");

    // Clear any pending interrupts
    ctrl = INL( ioaddr + I82544_ICR );

    // clear all mask bits
    OUTL( 0xFFFFFFFF, ioaddr + I82544_IMC );

    // Set interrupt bits for:
    // 1 = Transmit queue empty
    // 7 = Receiver timeout interrupt
    OUTL( (1<<1)|(1<<7), ioaddr + I82544_IMS );

    _heap_free = _heap_base = CYGARC_UNCACHED_ADDRESS(0x100000);
    _heap_size = 0x100000;

    InitRxRing(ioaddr);
    InitTxRing(ioaddr);

    read_mac_address(ioaddr, mac_addr);
    
    maclo = mac_addr[0] | (mac_addr[1]<<8) | (mac_addr[2]<<16) | (mac_addr[3]<<24);
    machi = (1<<31) | (0<<16) | mac_addr[4] | (mac_addr[5]<<8);

    OUTL( maclo , ioaddr + I82544_RAT );
    OUTL( machi, ioaddr + I82544_RAT + 4 );

    // enable receiver
    ctrl = INL( ioaddr + I82544_RCTL );
    ctrl |= I82544_RCTL_EN;
    OUTL( ctrl, ioaddr + I82544_RCTL );

    // Enable transmitter
    ctrl = INL( ioaddr + I82544_TCTL );
    ctrl |= I82544_TCTL_EN;
    OUTL( ctrl, ioaddr + I82544_TCTL );

    // PHY loopback test
    diag_printf("PHY loopback test...");

    if (!setup_loopback(ioaddr, 0)) {
	diag_printf("PHY setup failed\n");
	passed = 0;
	dump_regs(ioaddr);
	i82544_reset(ioaddr);
    } else {

	memcpy(tx_pkt->destination, mac_addr, sizeof(mac_addr));
	tx_pkt->type = _TYPE_IP;

	if (!send_packet(ioaddr, (cyg_uint8 *)tx_pkt, sizeof(*tx_pkt))) {
	    diag_printf("tx timeout!\n");
	    passed = 0;
	    dump_regs(ioaddr);
	    i82544_reset(ioaddr);
	} else {
	    memset(rx_pkt, 0, sizeof(*rx_pkt));
	    if (!rcv_packet(ioaddr, (cyg_uint8 *)rx_pkt, sizeof(*rx_pkt))) {
		diag_printf("rx timeout!\n");
		passed = 0;
		dump_regs(ioaddr);
		i82544_reset(ioaddr);
	    } else {
		if (memcmp(tx_pkt, rx_pkt, sizeof(*rx_pkt))) {
		    diag_printf("packet mismatch!\n");
		    i82544_reset(ioaddr);
		    passed = 0;
		}
	    }
	}
    }

    if (passed)
	diag_printf ("done\n");

    // External loopback test
    diag_printf ("External loopback test.  ");
    diag_wait();

    if (!setup_loopback(ioaddr, 1)) {
	diag_printf("PHY setup failed\n");
	passed = 0;
	dump_regs(ioaddr);
	i82544_reset(ioaddr);
    } else {
	memcpy(tx_pkt->destination, mac_addr, sizeof(mac_addr));
	tx_pkt->type = _TYPE_IP;

	if (!send_packet(ioaddr, (cyg_uint8 *)tx_pkt, sizeof(*tx_pkt))) {
	    phy_stat = mii_read_register( ioaddr, PHY_ADDRESS, 1 );
	    diag_printf("tx timeout!\n");
	    passed = 0;
	    dump_regs(ioaddr);
	    i82544_reset(ioaddr);
	} else {
	    memset(rx_pkt, 0, sizeof(*rx_pkt));
	    if (!rcv_packet(ioaddr, (cyg_uint8 *)rx_pkt, sizeof(*rx_pkt))) {
		diag_printf("rx timeout!\n");
		passed = 0;
		dump_regs(ioaddr);
		i82544_reset(ioaddr);
	    } else {
		if (memcmp(tx_pkt, rx_pkt, sizeof(*rx_pkt))) {
		    diag_printf("packet mismatch!\n");
		    passed = 0;
		    dump_regs(ioaddr);
		    i82544_reset(ioaddr);
		}
	    }
	}
    }
    
    diag_printf ("Ethernet controller tests %s.  ", passed ? "passed" : "failed" );
    diag_wait();
}
