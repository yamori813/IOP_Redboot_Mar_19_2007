//==========================================================================
//
//      io/serial/arm/arm_iop33x_ser.inl
//
//      IOP331 and IOP332 Serial I/O definitions
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-07-10
// Purpose:      IOP331 and IOP332 Serial I/O module
//               (interrupt driven version)
// Description: 
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/iop33x.h>            // platform definitions

//-----------------------------------------------------------------------------
// Baud rate specification
static unsigned short select_baud[] = {
    0,    // Unused
    0,    // 50
    0,    // 75
    0,    // 110
    0,    // 134.5
    0,    // 150
    0,    // 200
    0,    // 300
    0,    // 600
    0,    // 1200
    0,    // 1800
    0,    // 2400
    0,    // 3600
    0,    // 4800
    0,    // 7200
    217,  // 9600
    0,    // 14400
    109,  // 19200
    54,   // 38400
    36,   // 57600
    18,   // 115200
};

//
// IER UART Unit Enable handling
//
#define IER_UUE 0x40
#define CYGOPT_IO_SERIAL_SUPPORT_EXTRA_IER_BITS IER_UUE

#ifdef CYGPKG_IO_SERIAL_ARM_IOP33X_SERIAL0
static pc_serial_info iop33x_serial_info0 = {
    IOP33X_UART0_ADDR,
    CYGNUM_HAL_INTERRUPT_UART
};
#if CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL0_BUFSIZE > 0
static unsigned char iop33x_serial_out_buf0[
    CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL0_BUFSIZE
    ];
static unsigned char iop33x_serial_in_buf0[
    CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL0_BUFSIZE
    ];

static SERIAL_CHANNEL_USING_INTERRUPTS(
    iop33x_serial_channel0,
    pc_serial_funs, 
    iop33x_serial_info0,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL0_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT,
    &iop33x_serial_out_buf0[0], sizeof(iop33x_serial_out_buf0),
    &iop33x_serial_in_buf0[0], sizeof(iop33x_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(
    iop33x_serial_channel0,
    pc_serial_funs, 
    iop33x_serial_info0,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL0_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(
    iop33x_serial_io0, 
    CYGDAT_IO_SERIAL_ARM_IOP33X_SERIAL0_NAME,
    0,                     // Does not depend on a lower level interface
    &cyg_io_serial_devio, 
    pc_serial_init, 
    pc_serial_lookup,     // Serial driver may need initializing
    &iop33x_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_IOP33X_SERIAL0

#ifdef CYGPKG_IO_SERIAL_ARM_IOP33X_SERIAL1
static pc_serial_info iop33x_serial_info1 = {
    IOP33X_UART1_ADDR,
    CYGNUM_HAL_INTERRUPT_UART
};
#if CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL1_BUFSIZE > 0
static unsigned char iop33x_serial_out_buf1[
    CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL1_BUFSIZE
    ];
static unsigned char iop33x_serial_in_buf1[
    CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL1_BUFSIZE
    ];

static SERIAL_CHANNEL_USING_INTERRUPTS(
    iop33x_serial_channel1,
    pc_serial_funs, 
    iop33x_serial_info1,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL1_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT,
    &iop33x_serial_out_buf1[0], sizeof(iop33x_serial_out_buf1),
    &iop33x_serial_in_buf1[0], sizeof(iop33x_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(
    iop33x_serial_channel1,
    pc_serial_funs, 
    iop33x_serial_info1,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_IOP33X_SERIAL1_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(
    iop33x_serial_io1, 
    CYGDAT_IO_SERIAL_ARM_IOP33X_SERIAL1_NAME,
    0,                     // Does not depend on a lower level interface
    &cyg_io_serial_devio, 
    pc_serial_init, 
    pc_serial_lookup,     // Serial driver may need initializing
    &iop33x_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_IOP33X_SERIAL1

// EOF arm_iop33x_ser.inl
