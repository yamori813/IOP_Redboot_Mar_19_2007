#ifndef CYGONCE_HAL_ARM_XSCALE_EP80219_EP80219_H
#define CYGONCE_HAL_ARM_XSCALE_EP80219_EP80219_H

/*=============================================================================
//
//      ep80219.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    msalter
// Contributors: msalter, dmoseley
// Date:         2003-03-05
// Purpose:      Intel EP80219 platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/ep80219.h>
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_ep80219.h>
#include <cyg/hal/hal_verde.h>  // IO Processor defines

#define EP80219_FLASH_ADDR          0xf0000000   // Verde PBIU CS0
#define EP80219_UART_ADDR           0xfe800000   // Verde PBIU CS1
#define EP80219_DISPLAY_RIGHT_ADDR  0xfe850000   // Verde PBIU CS2
#define EP80219_DISPLAY_LEFT_ADDR   0xfe840000   // Verde PBIU CS3
#define EP80219_ROTARY_SWITCH_ADDR  0xfe8d0000   // Verde PBIU CS4
#define EP80219_COMPACTFLASH_ADDR   0xfe8f0000   // Verde PBIU CS5

#define SDRAM_PHYS_BASE     0xa0000000
#define SDRAM_BASE          0x00000000
#define SDRAM_SIZE          0x08000000  // 128MB
#define SDRAM_MAX           0x20000000  // 512MB

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_UNCACHED_BASE 0xc0000000
#define DCACHE_FLUSH_AREA   0xe0000000

// Pin used to enable Gigabit Ethernet NIC
// The GPIO pin only has effect when switch S7E1-5 is closed (ON).
#define EP80219_GBE_GPIO_PIN 4

// ------------------------------------------------------------------------
// SDRAM configuration

// I2C slave address to which the unit responds when in slave-receive mode
#define I2C_DEVID   0x02
#define SDRAM_DEVID 0xA4

// Timeout limit for SDRAM EEPROM to respond
#define I2C_TIMOUT	0x1000000

// ------------------------------------------------------------------------
// 7 Segment Display
#define DISPLAY_LEFT  EP80219_DISPLAY_LEFT_ADDR
#define DISPLAY_RIGHT EP80219_DISPLAY_RIGHT_ADDR

#define DISPLAY_OFF 0xFF

#define DISPLAY_0  0x03
#define DISPLAY_1  0x9F
#define DISPLAY_2  0x25
#define DISPLAY_3  0x0D
#define DISPLAY_4  0x99
#define DISPLAY_5  0x49
#define DISPLAY_6  0x41
#define DISPLAY_7  0x1F
#define DISPLAY_8  0x01
#define DISPLAY_9  0x19
#define DISPLAY_A  0x11
#define DISPLAY_B  0xC1
#define DISPLAY_C  0x63
#define DISPLAY_D  0x85
#define DISPLAY_E  0x61
#define DISPLAY_F  0x71
#define DISPLAY_I  0xF3
#define DISPLAY_L  0xE3
#define DISPLAY_P  0x31
#define DISPLAY_S  0x48

#define DISPLAY_PERIOD  0xFE
#define DISPLAY_ERROR   0x60  /* Displays "E." */


#ifdef __ASSEMBLER__
        // Display 'lvalue:rvalue' on the hex display
        // lvalue and rvalue must be of the form 'DISPLAY_x'
        // where 'x' is a hex digit from 0-F.
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue	
	ldr	\reg0, =DISPLAY_LEFT		// display left digit
	ldr	\reg1, =\lvalue
	strb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	ldr	\reg1, =\rvalue			// display right digit
	strb	\reg1, [\reg0]
#if 0
	// delay
        ldr     \reg0, =0x7800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
#endif
	.endm

	.macro REG_DISPLAY reg0, reg1, reg2
	b	667f
   666:
	.byte	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3 
	.byte	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7
	.byte	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B
	.byte	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
   667:
	ldr	\reg0, =666b
	add	\reg0, \reg0, \reg2, lsr #4
	ldrb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_LEFT
	str	\reg1, [\reg0]
	ldr	\reg0, =666b
	and     \reg2, \reg2, #0xf
	add	\reg0, \reg0, \reg2
	ldrb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	str	\reg1, [\reg0]

	// delay
        ldr     \reg0, =0x7800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
	.endm
#else
static inline void HEX_DISPLAY(int lval, int rval)
{
    int i;
    static unsigned char hchars[] = {
	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3,
	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B,
	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
    };
    volatile unsigned int *ldisp = (volatile unsigned int *)DISPLAY_LEFT;
    volatile unsigned int *rdisp = (volatile unsigned int *)DISPLAY_RIGHT;

    *ldisp = hchars[lval & 0xf];
    *rdisp = hchars[rval & 0xf];

    for (i = 0; i < 0x10000000; i++);
}
#endif // __ASSEMBLER__

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_EP80219_EP80219_H
// EOF ep80219.h
