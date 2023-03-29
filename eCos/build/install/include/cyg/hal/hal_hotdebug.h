/*=============================================================================
//
//      hal_hotdebug.h
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Intel Corporation
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
// Author(s):    drew.kranak@intel.com
// Contributors: drew.kranak@intel.com
// Date:         2004-01-04
// Purpose:      JTAG Hot Debug support code
// Description:  This file allows adding in JTAG Hot Debug support code to
//      to the RESET handler through a macro.  The code included here is a
//      derivative of the code found on the Intel(R) developers web site for
//      IOP processors in the White Paper:
//                     "Hot-Debug for Intel XScale(R) Core Debug"
//
// Usage:        #include <cyg/hal/hal_hotdebug.h>
// Copyright:    (C) 2003-2004 Intel Corporation.
// 
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_HOTDEBUG_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_HOTDEBUG_H

#ifdef __ASSEMBLER__

   .macro _xscale_hotdebug_enable
    //     This code is added to the FLASH reset code.
    mov   r13, #0x8000001c        //  set MOE bits to enable JTAG hot debug
    mcr   p14, 0, r13, c10, c0, 0
   .endm

   .macro _xscale_hotdebug
    //     This macro is added to the beginning of the RAM RESET or warm_reset code
        .globl reset_hotdebug
reset_hotdebug:
                                  // Check whether this is a JTAG debug exception or a real RESET event.
                                  //
                                  // NOTE: r13 is only safe register to use.
                                  // - For RESET, don’t really care about which register is used
                                  // - For debug exception, r13=DBG_r13, prevents application registers
                                  // - from being corrupted, before debug handler can save.
    mrs   r13, cpsr               //  and continue with the reset handler
    and   r13, r13, #0x1F         // Mask procrocessor mode bits
    cmp   r13, #0x15              // are we in DBG mode?
    beq   reset_hotdebug_stub     // if so, go to thehotdebug stub
    mov   r13, #0x8000001c        //  otherwise, enable debug, set MOE bits
    mcr   p14, 0, r13, c10, c0, 0 //  and continue with the reset handler
    mrc   p14, 0, r12, c10, c0, 0 //  and continue with the reset handler
    b     reset_hotdebug_x        //  branch to the reset handler.

    .align 5                      // align code to a cache line boundary.

        .globl reset_hotdebug_stub
reset_hotdebug_stub:
                                  // Save the state of the IC enable/disable bit in DBG_LR[0].
    mrc   p15, 0, r13, c1, c0, 0
    and   r13, r13, #0x1000
    orr   r14, r14, r13, lsr #12
                                  // Enable the IC to reduce fill buffer confict when JTAG is down loading
    mrc   p15, 0, r13, c1, c0, 0    // mini I cache.
    orr   r13, r13, #0x1000
    mcr   p15, 0, r13, c1, c0, 0
                                  // Perform a sync operation to ensure all outstanding instr fetches have
                                  // completed before continuing. The invalidate cache line function
                                  // serves as a synchronization operation, that’s why it is used
                                  // here. The target line is some scratch address in memory.
    adr   r13, reset_hotdebug_scratch
    mcr   p15, 0, r13, c7, c5, 1
                                  // invalidate BTB. make sure downloaded vector table does not hit one of
                                  // the application’s branches cached in the BTB, branch to the wrong place
    mcr   p15, 0, r13, c7, c5, 6
                                  // Now, send ‘ready for download’ message to debugger, indicating debugger
                                  // can begin the download. ‘ready for download’ = 0x00B00000.

        .globl reset_hotdebug_TX
reset_hotdebug_TX:
    mrc   p14, 0, r15, c14, c0, 0 // Waite for TX reg. availability
    bvs   reset_hotdebug_TX
    mov   r13, #0x00B00000
    mcr   p14, 0, r13, c8, c0, 0  // write to TX
                                  // Wait for JTAG debugger to indicate that the download is complete.
        .globl reset_hotdebug_RX
reset_hotdebug_RX:
    mrc   p14, 0, r15, c14, c0, 0 // Waiting for data from the debugger in RX.
    bpl   reset_hotdebug_RX
                                  // before reading the RX regsiter to get the address to branch to, restore
                                  // the state of the IC (saved in DBG_r14[0]) to the value it had at the
                                  // start of the debug handler stub. Also, note it must be restored before
                                  // reading the RX register because of limited scratch registers (r13)
    mrc   p15, 0, r13, c1, c0, 0
                                  //# First, check DBG_LR[0] to see if the IC was enabled or disabled
    tst   r14, #0x1
                                  //# Then, if it was previously disabled, then disable it now, otherwise,
                                  //# there’s no need to change the state, because its already enabled.
    biceq r13, r13, #0x1000
    mcr   p15, 0, r13, c1, c0, 0
    bic   r14, r14, #0x1          // Restore the link register value
                                  // Now r13 can be used to read RX and get the target address to branch to.
    mrc   p14, 0, r13, c9, c0, 0  // Read RX and
    mov   pc, r13                 // branch to downloaded address.

reset_hotdebug_safty:             //  Continue with real RESET code  i.e. not doing hot debug.
    b     reset_hotdebug_safty

    .align 5
        .globl reset_hotdebug_scratch
reset_hotdebug_scratch:           // scratch memory space used by the invalidate IC line function above.
    .word 0                       // insure it starts at a cache line boundary, so nothing else is affected
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0

        .globl reset_hotdebug_x
reset_hotdebug_x:                  //  Continue with real RESET code  i.e. not doing hot debug.


   .endm        /* _xscale_hotdebug */


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                                                                           */
/* Purpose:  Clean Main Data Cache                                           */
/*                                                                           */
/* Input:    MemoryCleanArea      Virtural Cache address space defined       */
/*                                to be used for cache cleaning.  Uually     */
/*                                not physical memory.                       */
/*                                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
   .macro _xscale_hotdebug_clean_data_cache  MemoryCleanArea
        .globl _hotdebug_clean
_hotdebug_clean:
    ldr     r1, =\MemoryCleanArea          @ use a CACHEABLE area of memory
    mov     r0, #1024                       @ number of lines in the Dcache
101:
    mcr     p15, 0, r1, c7, c2, 5           @ allocate a Dcache line
    add     r1, r1, #32                     @ increment to the next cache line
    subs    r0, r0, #1                      @ decrement the loop count
    bne     101b
        .globl _hotdebug_clean_x
_hotdebug_clean_x:
    .endm


   .macro _xscale_hotdebug_lock_exceptions_into_icache _have_MMU_TABLE _pMMU_TABLE
            @preperation to lock vector table into icache line.
            @The icache lock function requiers that instruction cacheing be disabled.
            @When the MMU is on, this is accomplished by seting the discriptor C bit to 0
        .globl _hotdebug_lock
_hotdebug_lock:
   .if   \_have_MMU_TABLE > 0
    ldr      r8, = \_pMMU_TABLE
    mov      r9, pc, lsr #20                @get current address
    add      r8, r8, r9                     @pointer to current memory discriptor
    ldr      r9,[r8]                        @get discriptor and preserve value
   .else
    mrc      p15, 0, r8, c2, c0, 0          @Translation Table Base address
    ldr      r10, =0x0FFFFFFF               @
    and      r8, r8, r10                    @Convert to virtural
    mov      r9, pc, lsr #20                @get current address
    add      r8, r8, r9                     @pointer to current memory discriptor
    ldr      r9,[r8]                        @get discriptor and preserve value
   .endif
    ldr      r10, =0x3                      @Test if this is a first level discriptor
    and      r10, r9, r10
    cmp      r10, #0x2
    beq      201f                           @If this is a first level discriptor, continue
    ldr      r10, =0xFFFFFC00
    and      r8, r9, r10                    @Get pointer to second head of second level discriptor
    ldr      r9,[r8]                        @get second level discriptor and preserve value in r9
        .globl _hotdebug_lock_1
_hotdebug_lock_1:

 201:
    ldr      r10, =0xFFFFFFF7               @Mask C bit to disable instruction cache.
    and      r10, r9, r10
    str      r10,[r8]                       @Set new discriptor value

    mov     r0, #0                          @ Clear internal buffers so that new discriptor value will be used
    mcr     p15, 0, r0, c9, c1, 1           @ unlock all of icache
    mcr     p15, 0, r0, c7, c10, 4          @ drain the write & fill buffers
    mcr     p15, 0, r0, c7, c7, 0           @ invalidate Icache, Dcache and BTB
    mcr     p15, 0, r0, c8, c7, 0           @ invalidate instuction and data TLBs
    mcr     p15, 0, r0, c7, c10, 4          @ drain the write & fill buffers
    CPWAIT  r1

    mcr     p15, 0, r0, c9, c1, 0;          @Lock Vector table load instructions into icache
    CPWAIT  r1

    str     r9, [r8]                        @restore discriptor talbe value and clear all buffered vaules
    mcr     p15, 0, r8, c7, c10,1           @ clean Dcache so that discriptor is in RAM
    mcr     p15, 0, r0, c7, c10,4           @ drain the write & fill buffers
    mcr     p15, 0, r0, c7, c7, 0           @ invalidate Icache, Dcache and BTB
    mcr     p15, 0, r0, c8, c7, 0           @ invalidate instuction and data TLBs
    CPWAIT  r1
        .globl _hotdebug_lock_x
_hotdebug_lock_x:
   .endm

#endif /* __ASSEMBLER__ */

#endif // CYGONCE_HAL_ARM_XSCALE_HAL_HOTDEBUG_H
