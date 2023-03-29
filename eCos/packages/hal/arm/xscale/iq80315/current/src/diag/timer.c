//=============================================================================
//
//      timer.c
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter, dmoseley
// Date:        2002-02-20
// Purpose:
// Description:
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_ioc80314.h>          // Hardware definitions
#include <cyg/hal/iq80315.h>            // Platform specifics
#include "test_menu.h"

extern void hal_delay_us(cyg_int32 delay);
extern CYG_ADDRWORD		decIn(void);
extern cyg_bool cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch);

void
timer_test(MENU_ARG arg)
{
    int j, i;

    diag_printf ("\nTimer test. Dots should appear at 1 second intervals:\n");

    for (j = 0; j < 20; j++) {
	diag_printf (".");
	for (i = 0; i < 1000; i++)
	    hal_delay_us(1000);
    }
}

void
timer_test2(MENU_ARG arg)
{
    int                      done=FALSE;
    cyg_bool                 linestate = 1;
    hal_virtual_comm_table_t *_curChan = CYGACC_CALL_IF_CONSOLE_PROCS();
    cyg_uint8                keyCh;
    volatile cyg_uint8       *gpioBase;
	cyg_uint32				 input;

    diag_printf ("\nTimer test 2: This test will drive GPIO pin 0\n"
                 "The loop addes about 100 usec of over head.\n"
				 "The width of a pulse represents the delay selected.\n"
                 "On IQ80315, switch xS5 must be on to desabel com port 1 and\n"
                 "the output will be on header p17 pin 16.\n"
                  "=> Hit \"AnyKey\" to teminate.\n");

	diag_printf ("What delay do you want (in usec)>\n");
	input = decIn();

	diag_printf ("Delaying for %dusec\n", input);
    // Initialize GPIO
    gpioBase = (volatile cyg_uint8*)(HAL_IOC80314_GPIO_BASE);
    *(((volatile cyg_uint8*)(gpioBase+GPIO_CNTRL_EN_ADDR)))          = 1;                              //Enable GPIO
    *(((volatile cyg_uint8*)(gpioBase+GPIO_CNTRL_DIRECTION_ADDR)))   = 0x01;                           //Set bit 1 to Output

    // Loop until AnyKey
    while (!done) {
        if ( linestate == 0)
          {
             *(((volatile cyg_uint8*)(gpioBase+GPIO_DATA_OUT_ADDR))) |= 0x01;
             linestate = 1;
          }
        else
          {
             *(((volatile cyg_uint8*)(gpioBase+GPIO_DATA_OUT_ADDR))) &= 0xFE;
             linestate = 0;
          }
        hal_delay_us(input);
        if (cyg_hal_plf_serial_getc_nonblock(*(*_curChan), &keyCh) == TRUE) {
           done = TRUE;
        }
    }
}








