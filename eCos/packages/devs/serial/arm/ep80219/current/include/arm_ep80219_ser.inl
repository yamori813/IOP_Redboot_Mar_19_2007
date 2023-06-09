//==========================================================================
//
//      io/serial/arm/arm_ep80219_ser.inl
//
//      EP80219 Serial I/O definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter, dmoseley
// Date:         2003-03-05
// Purpose:      EP80219 Serial I/O module (interrupt driven version)
// Description: 
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/ep80219.h>            // platform definitions

//-----------------------------------------------------------------------------
// Baud rate specification

static unsigned short select_baud[] = {
    0,    // Unused
    0,    // 50
    0,    // 75
    1047, // 110
    0,    // 134.5
    768,  // 150
    0,    // 200
    384,  // 300
    192,  // 600
    96,   // 1200
    24,   // 1800
    48,   // 2400
    0,    // 3600
    24,   // 4800
    16,   // 7200
    12,   // 9600
    8,    // 14400
    6,    // 19200
    3,    // 38400
    2,    // 57600
    1,    // 115200
};

#ifdef CYGPKG_IO_SERIAL_ARM_EP80219_SERIAL0
static pc_serial_info ep80219_serial_info0 = {EP80219_UART_ADDR,
                                          CYGNUM_HAL_INTERRUPT_UART};
#if CYGNUM_IO_SERIAL_ARM_EP80219_SERIAL0_BUFSIZE > 0
static unsigned char ep80219_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_EP80219_SERIAL0_BUFSIZE];
static unsigned char ep80219_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_EP80219_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(ep80219_serial_channel0,
                                       pc_serial_funs, 
                                       ep80219_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EP80219_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &ep80219_serial_out_buf0[0], sizeof(ep80219_serial_out_buf0),
                                       &ep80219_serial_in_buf0[0], sizeof(ep80219_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(ep80219_serial_channel0,
                      pc_serial_funs, 
                      ep80219_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EP80219_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(ep80219_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_EP80219_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &ep80219_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_EP80219_SERIAL0

// EOF arm_ep80219_ser.inl
