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
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-04
// Purpose:      Intel XScale IQ8134 CRB platform specific low-level debug routines
// Description: 
//     This file contains low-level debug I/O routines used during system initialziation.
//     Since we may be using "alternative" memory (ie different stack and heap) then we
//     cannot use any global data or C Library calls.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/iq8134x.h>            // Platform specific hardware definitions
#include <cyg/hal/hal_iop34x.h>         // Variant specific hardware definitions
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


/*********************************************************
* For low level debug output during system configuration
* a UART can be enabled.  
**********************************************************/
void
hal_platform_lowlevel_serial_debug_init(unsigned long baud)
{
	// IQ8134x boards have two serial ports.  Transport core
	// reserves UART0.  If the RedBoot build includes two
	// Comm channels, then we are ignoring Transport core's
	// ownership and we'll use UART0 for Core0 and 
	// UART1 for Core1.
    cyg_uint32* base;
    cyg_uint8 lcr, dll;

#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1 
	// Use UART1 for App core and UART0 for Transport core
	if (_RUNNING_ON_CORE1) 
		base = (cyg_uint32*)IOP34X_UART1_ADDR;
	else
		base = (cyg_uint32*)IOP34X_UART0_ADDR;
#else
		base = (cyg_uint32*)IOP34X_UART1_ADDR;
#endif

    // Disable port interrupts while changing hardware
    // and enable the UART function 
    HAL_WRITE_UINT8(base+CYG_DEV_IER, SIO_IER_UUE);

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);

    // Set baud rate
    switch (baud) {
    case 115200: dll =   18; break;
    case 57600:  dll =   36; break;
    case 38400:  dll =   54; break;
    case 19200:  dll =  109; break;
    case 9600:   dll =  217; break;
    default:     dll =   18; break;
    }
    HAL_READ_UINT8(base+CYG_DEV_LCR, lcr);
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, lcr|SIO_LCR_DLAB);
    HAL_WRITE_UINT8(base+CYG_DEV_DLL, dll);
    HAL_WRITE_UINT8(base+CYG_DEV_DLM, 0);
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, lcr);

    // Enable & clear FIFO
    HAL_WRITE_UINT8(base+CYG_DEV_FCR, 0x07);
}

/*********************************************************
* Function to output a character to the debug UART during
* system initialization.  It waits for an 'Empty' condition
* on the UART and then transfers its character and then
* waits for the char to be transmitted by waiting for the
* 'Empty' condition again.
**********************************************************/
void
hal_platform_lowlevel_serial_debug_putc(char c)
{

    cyg_uint32* base;
    cyg_uint8 lsr;

#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
	// Use UART1 for App core and UART0 for Transport core
	if (_RUNNING_ON_CORE1) 
		base = (cyg_uint32*)IOP34X_UART1_ADDR;
	else
		base = (cyg_uint32*)IOP34X_UART0_ADDR;
#else
	base = (cyg_uint32*)IOP34X_UART1_ADDR;
#endif

    do {
       HAL_READ_UINT8(base + CYG_DEV_LSR, lsr);
    } while ((lsr & SIO_LSR_TEMT) == 0);

    HAL_WRITE_UINT8(base+CYG_DEV_THR, c);

    do {
       HAL_READ_UINT8(base + CYG_DEV_LSR, lsr);
    } while ((lsr & SIO_LSR_TEMT) == 0);
}

/*********************************************************
* Push a string of characters through the putc routine.
* String must be null-terminated.
**********************************************************/
void
hal_platform_lowlevel_serial_debug_puts(char *s)
{
char * temp;
#ifdef CYG_HAL_STARTUP_ROMRAM
asm volatile("	movs	r9,pc\n"
		"	ldr  	r9,=0xf0200000\n"
		"	addmi	%0,%1,r9\n"
		: "=r" (temp): "r" (s): "r9");
#else
	temp = s;
#endif
	
    if (temp) 
    {
        while (*temp)
            hal_platform_lowlevel_serial_debug_putc(*temp++);
    }
}

/*********************************************************
* Outputs a Hex 32-bit number.
**********************************************************/
void
hal_platform_lowlevel_serial_debug_putn32(cyg_uint32 n)
{
#ifdef CYG_HAL_STARTUP_ROMRAM
	char digs[16];
	asm volatile("	ldr	r9,=0x33323130\n"
	             "	str	r9,[%0],#4\n"
	             "	ldr	r9,=0x37363534\n"
	             "	str	r9,[%0],#4\n"
	             "	ldr	r9,=0x42413938\n"
	             "	str	r9,[%0],#4\n"
	             "	ldr	r9,=0x46454443\n"
	             "	str	r9,[%0],#4\n"
	             ::"r" (digs):"r9");
#else
	char digs[] = {'0', '1', '2', '3', '4', '5', '6', '7', \
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
#endif
	
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

/*********************************************************
* Outputs a Hex 16-bit number.
**********************************************************/
void
hal_platform_lowlevel_serial_debug_putn16(cyg_uint16 n)
{
#ifdef CYG_HAL_STARTUP_ROMRAM
	char digs[16];
	asm volatile("	ldr	r9,=0x33323130\n"
	             "	str	r9,[%0],#4\n"
	             "	ldr	r9,=0x37363534\n"
	             "	str	r9,[%0],#4\n"
	             "	ldr	r9,=0x42413938\n"
	             "	str	r9,[%0],#4\n"
	             "	ldr	r9,=0x46454443\n"
	             "	str	r9,[%0],#4\n"
	             ::"r" (digs):"r9");
#else
	char digs[] = {'0', '1', '2', '3', '4', '5', '6', '7', \
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
#endif

    hal_platform_lowlevel_serial_debug_putc('0'); 
    hal_platform_lowlevel_serial_debug_putc('x'); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >> 12) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  8) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  4) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  0) & 0xf]); 
}

/*********************************************************
* Outputs a Hex 8-bit number.
**********************************************************/
void
hal_platform_lowlevel_serial_debug_putn8(cyg_uint8 n)
{
#ifdef CYG_HAL_STARTUP_ROMRAM
	char digs[16];
	asm volatile("	ldr	r9,=0x33323130\n"
             	"	str	r9,[%0],#4\n"
             	"	ldr	r9,=0x37363534\n"
             	"	str	r9,[%0],#4\n"
             	"	ldr	r9,=0x42413938\n"
             	"	str	r9,[%0],#4\n"
             	"	ldr	r9,=0x46454443\n"
             	"	str	r9,[%0],#4\n"
             	::"r" (digs):"r9");
#else
	char digs[] = {'0', '1', '2', '3', '4', '5', '6', '7', \
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
#endif
	
    hal_platform_lowlevel_serial_debug_putc('0'); 
    hal_platform_lowlevel_serial_debug_putc('x'); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  4) & 0xf]); 
    hal_platform_lowlevel_serial_debug_putc(digs[(n >>  0) & 0xf]); 
}
