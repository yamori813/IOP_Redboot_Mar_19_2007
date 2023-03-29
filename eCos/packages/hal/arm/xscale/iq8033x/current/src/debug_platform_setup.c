/*=============================================================================
//
//      debug_platform_setup.c
//
//      Low-level platform-setup serial debug routines.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// Author(s):    drew.moseley@intel.com
// Contributors: 
// Date:         2003-08-04
// Purpose:      Intel XScale IQ80331/IQ80332 CRB platform specific low-level debug routines
// Description: 
//     This file contains low-level debug I/O routines used during system initialziation.
//     Since we may be using "alternative" memory (ie different stack and heap) then we
//     cannot use any global data or C Library calls.
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/iq8033x.h>             // Platform specific hardware definitions
#include <cyg/hal/hal_iop33x.h> // Variant specific hardware definitions
#include <cyg/hal/hal_io.h>

// Define the serial registers.
#define CYG_DEV_RBR   0x00              // receiver buffer register, read, dlab = 0
#define CYG_DEV_THR   0x00              // transmitter holding register, write, dlab = 0
#define CYG_DEV_DLL   0x00              // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_DLM   0x01              // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_IER   0x01              // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_FCR   0x02              // fifo control register, write, dlab = 0
#define CYG_DEV_LCR   0x03              // line control register, write
#define CYG_DEV_LSR   0x05              // line status register, read

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter empty

// The interrupt enable register bits.
#define SIO_IER_UUE     0x40            // UART Unit Enable

void
hal_platform_lowlevel_serial_debug_init(unsigned long baud)
{
    cyg_uint32* base = (cyg_uint32*)IOP33X_UART0_ADDR;
    cyg_uint8 lcr, dll;

    // Make sure to clear the gpod bits associated with the uart before using
    HAL_WRITE_UINT32(GPIO_GPOD, 0);

    // Disable port interrupts while changing hardware
    // and enable the UART function on the GPIO pins
    HAL_WRITE_UINT8(base+CYG_DEV_IER, SIO_IER_UUE);

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);

    // Set baud rate
    switch (baud) {
    case 115200: dll =  18; break;
    case 57600:  dll =  36; break;
    case 38400:  dll =  54; break;
    case 19200:  dll = 109; break;
    case 9600:   dll = 217; break;
    default:     dll =  18; break;
    }
    HAL_READ_UINT8(base+CYG_DEV_LCR, lcr);
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, lcr|SIO_LCR_DLAB);
    HAL_WRITE_UINT8(base+CYG_DEV_DLL, dll);
    HAL_WRITE_UINT8(base+CYG_DEV_DLM, 0);
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, lcr);

    // Enable & clear FIFO
    HAL_WRITE_UINT8(base+CYG_DEV_FCR, 0x07);
}

void
hal_platform_lowlevel_serial_debug_putc(char c)
{
    cyg_uint32* base = (cyg_uint32*)IOP33X_UART0_ADDR;
    cyg_uint8 lsr;

    do {
       HAL_READ_UINT8(base + CYG_DEV_LSR, lsr);
    } while ((lsr & SIO_LSR_TEMT) == 0);

    HAL_WRITE_UINT8(base+CYG_DEV_THR, c);

    do {
       HAL_READ_UINT8(base + CYG_DEV_LSR, lsr);
    } while ((lsr & SIO_LSR_TEMT) == 0);
}

void
hal_platform_lowlevel_serial_debug_puts(char *s)
{
    if (s) 
    {
        while (*s)
            hal_platform_lowlevel_serial_debug_putc(*s++);
    }
}

void
hal_platform_lowlevel_serial_debug_putn32(cyg_uint32 n)
{
    char digs[] = {'0', '1', '2', '3', '4', '5', '6', '7', \
                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    hal_platform_lowlevel_serial_debug_putc('0'); 
    hal_platform_lowlevel_serial_debug_putc('x'); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 28) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 24) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 20) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 16) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 12) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  8) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  4) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  0) & 0xf]); 
}

void
hal_platform_lowlevel_serial_debug_putn16(cyg_uint16 n)
{
    char digs[] = {'0', '1', '2', '3', '4', '5', '6', '7', \
                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    hal_platform_lowlevel_serial_debug_putc('0'); 
    hal_platform_lowlevel_serial_debug_putc('x'); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 12) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  8) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  4) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  0) & 0xf]); 
}

void
hal_platform_lowlevel_serial_debug_putn8(cyg_uint8 n)
{
    char digs[] = {'0', '1', '2', '3', '4', '5', '6', '7', \
                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    hal_platform_lowlevel_serial_debug_putc('0'); 
    hal_platform_lowlevel_serial_debug_putc('x'); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  4) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  0) & 0xf]); 
}
