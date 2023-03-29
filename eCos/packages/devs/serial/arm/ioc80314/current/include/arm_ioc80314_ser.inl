//==========================================================================
//
//      io/serial/arm/arm_ioc80314_ser.inl
//
//      IOC80314 Serial I/O definitions
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
// Contributors: msalter
// Date:         2000-10-10
// Purpose:      IOC80314 Serial I/O module (interrupt driven version)
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/ioc80314.h>            // platform definitions

//-----------------------------------------------------------------------------
// Baud rate specification
#define BAUD_MULTIPLIER(BAUD_RATE)  ((5000000 / (BAUD_RATE*2)) + 1)
static unsigned short select_baud[] = {
    0,    // Unused
    0,    // 50
    0,    // 75
    BAUD_MULTIPLIER(110),          // 110
                        0,         // 134.5
    BAUD_MULTIPLIER(150),          // 150
                       0,          // 200
    BAUD_MULTIPLIER(300   ),       // 300
    BAUD_MULTIPLIER(600   ),       // 600
    BAUD_MULTIPLIER(1200  ),       // 1200
    BAUD_MULTIPLIER(1800  ),       // 1800
    BAUD_MULTIPLIER(2400  ),       // 2400
    BAUD_MULTIPLIER(3600  ),       // 3600
    BAUD_MULTIPLIER(4800  ),       // 4800
    BAUD_MULTIPLIER(7200  ),       // 7200
    BAUD_MULTIPLIER(9600  ),       // 9600
    BAUD_MULTIPLIER(14400 ),       // 14400
    BAUD_MULTIPLIER(19200 ),       // 19200
    BAUD_MULTIPLIER(38400 ),       // 38400
    BAUD_MULTIPLIER(57600 ),       // 57600
    BAUD_MULTIPLIER(115200),       // 115200
};

#ifdef CYGPKG_IO_SERIAL_ARM_IOC80314_SERIAL0
static pc_serial_info ioc80314_serial_info0 = {IOC80314_UART_ADDR,
                                          CYGNUM_HAL_INTERRUPT_UART};
#if CYGNUM_IO_SERIAL_ARM_IOC80314_SERIAL0_BUFSIZE > 0
static unsigned char ioc80314_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_IOC80314_SERIAL0_BUFSIZE];
static unsigned char ioc80314_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_IOC80314_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(ioc80314_serial_channel0,
                                       pc_serial_funs,
                                       ioc80314_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_IOC80314_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &ioc80314_serial_out_buf0[0], sizeof(ioc80314_serial_out_buf0),
                                       &ioc80314_serial_in_buf0[0], sizeof(ioc80314_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(ioc80314_serial_channel0,
                      pc_serial_funs,
                      ioc80314_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_IOC80314_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(ioc80314_serial_io0,
             CYGDAT_IO_SERIAL_ARM_IOC80314_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio,
             pc_serial_init,
             pc_serial_lookup,     // Serial driver may need initializing
             &ioc80314_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_IOC80314_SERIAL0

// EOF arm_ioc80314_ser.inl
