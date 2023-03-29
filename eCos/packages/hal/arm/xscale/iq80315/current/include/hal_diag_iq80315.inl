/*=============================================================================
//
//      hal_diag_iq80315.inl
//
//      Platform specific HAL diagnostic output code
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
// Author(s):   dkranak
// Date:        2003-05-22
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
// Copyright:    (C) 2003-2004 Intel Corporation.
// 
//####DESCRIPTIONEND####
//
//===========================================================================*/

//-----------------------------------------------------------------------------

// Based on 1.8432 MHz xtal
#define BAUD_MULTIPLIER(BAUD_RATE, FUDGE_V0, FUDGE_V1)  BAUD_RATE                                                                ,       \
                                                        {((((CYGNUM_HAL_SFN_SPEED       / (BAUD_RATE*2)) + FUDGE_V0)>>8)&0xFF),          \
                                                         ((((CYGNUM_HAL_PROCESSOR_SPEED / (BAUD_RATE*2)) + FUDGE_V1)>>8)&0xFF)  },       \
                                                        { (((CYGNUM_HAL_SFN_SPEED       / (BAUD_RATE*2)) + FUDGE_V0)    &0xFF),          \
                                                          (((CYGNUM_HAL_PROCESSOR_SPEED / (BAUD_RATE*2)) + FUDGE_V1)    &0xFF)  }


struct baud_config {
    cyg_int32 baud_rate;
    cyg_uint8 msb[2];
    cyg_uint8 lsb[2];
    };


/*!!!!   Correct the baude rate values for 80314 */
struct baud_config baud_conf[] = {
    {BAUD_MULTIPLIER(9600  , -6, -6)},               /* 0x */
    {BAUD_MULTIPLIER(19200 , -6, -6)},
    {BAUD_MULTIPLIER(38400 , -6, -6)},
    {BAUD_MULTIPLIER(57600 , -6, -6)},
    {BAUD_MULTIPLIER(115200, -6, -6)},
    {BAUD_MULTIPLIER(230400, -4, -4)},
    {BAUD_MULTIPLIER(460800, -4, -4)},
    {BAUD_MULTIPLIER(921600, -2, -2)},
                                   };             /* 0x */

//-----------------------------------------------------------------------------


