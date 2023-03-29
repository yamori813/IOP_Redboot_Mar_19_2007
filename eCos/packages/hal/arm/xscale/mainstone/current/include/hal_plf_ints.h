#ifndef CYGONCE_HAL_PLF_INTS_H
#define CYGONCE_HAL_PLF_INTS_H
//==========================================================================
//
//      hal_plf_ints.h
//
//      HAL Platform Interrupt support
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
// Date:         2002-07-15
// Purpose:      Define Interrupt support
// Description:  The interrupt details for a specific platform is defined here.
// Usage:
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// start with variant ints
#include CYGBLD_HAL_VAR_INTS_H

#define CYGNUM_HAL_INTERRUPT_MMC_CARD       (CYGNUM_HAL_VAR_ISR_MAX + 1)
#define CYGNUM_HAL_INTERRUPT_USIM_CARD      (CYGNUM_HAL_VAR_ISR_MAX + 2)
#define CYGNUM_HAL_INTERRUPT_USB_CABLE      (CYGNUM_HAL_VAR_ISR_MAX + 3)
#define CYGNUM_HAL_INTERRUPT_ETHERNET       (CYGNUM_HAL_VAR_ISR_MAX + 4)
#define CYGNUM_HAL_INTERRUPT_AC97_CODEC     (CYGNUM_HAL_VAR_ISR_MAX + 5)
#define CYGNUM_HAL_INTERRUPT_PEN            (CYGNUM_HAL_VAR_ISR_MAX + 6)
#define CYGNUM_HAL_INTERRUPT_MS_INSERT      (CYGNUM_HAL_VAR_ISR_MAX + 7)
#define CYGNUM_HAL_INTERRUPT_EXBRD          (CYGNUM_HAL_VAR_ISR_MAX + 8)
#define CYGNUM_HAL_INTERRUPT_RSVD8          (CYGNUM_HAL_VAR_ISR_MAX + 9)
#define CYGNUM_HAL_INTERRUPT_PCMCIA0_CARD   (CYGNUM_HAL_VAR_ISR_MAX + 10)
#define CYGNUM_HAL_INTERRUPT_PCMCIA0_STATUS (CYGNUM_HAL_VAR_ISR_MAX + 11)
#define CYGNUM_HAL_INTERRUPT_PCMCIA0        (CYGNUM_HAL_VAR_ISR_MAX + 12)
#define CYGNUM_HAL_INTERRUPT_RSVD12         (CYGNUM_HAL_VAR_ISR_MAX + 13)
#define CYGNUM_HAL_INTERRUPT_PCMCIA1_CARD   (CYGNUM_HAL_VAR_ISR_MAX + 14)
#define CYGNUM_HAL_INTERRUPT_PCMCIA1_STATUS (CYGNUM_HAL_VAR_ISR_MAX + 15)
#define CYGNUM_HAL_INTERRUPT_PCMCIA1        (CYGNUM_HAL_VAR_ISR_MAX + 16)

#undef  CYGNUM_HAL_ISR_MAX
#define CYGNUM_HAL_ISR_MAX      CYGNUM_HAL_INTERRUPT_PCMCIA1

#undef  CYGNUM_HAL_ISR_COUNT
#define CYGNUM_HAL_ISR_COUNT    (CYGNUM_HAL_ISR_MAX+1)

// Platform ints get passed in through GPIO0
#define CYGNUM_HAL_INTERRUPT_PLATFORM CYGNUM_HAL_INTERRUPT_GPIO0

extern int   hal_plf_IRQ_handler(void);
extern void  hal_plf_interrupt_mask(int vector);
extern void  hal_plf_interrupt_unmask(int vector);
extern void  hal_plf_interrupt_acknowledge(int vector);

#define HAL_PLF_INTERRUPT_MASK(v)          hal_plf_interrupt_mask(v)
#define HAL_PLF_INTERRUPT_UNMASK(v)        hal_plf_interrupt_unmask(v)
#define HAL_PLF_INTERRUPT_ACKNOWLEDGE(v)   hal_plf_interrupt_acknowledge(v)
#define HAL_PLF_INTERRUPT_CONFIGURE(v,l,u) // Nothing to do.
#define HAL_PLF_INTERRUPT_SET_LEVEL(v)     // Nothing to do.

#define HAL_PLATFORM_RESET()                                               \
    CYG_MACRO_START                                                        \
    *MAINSTONE_MSCWR1 = 1;                                                 \
    *MAINSTONE_MSCWR1 = 0;                                                 \
    CYG_MACRO_END

#endif // CYGONCE_HAL_PLF_INTS_H
