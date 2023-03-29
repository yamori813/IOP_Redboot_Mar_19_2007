#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005 Red Hat, Inc.
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
// Date:         2003-07-10
// Purpose:      Intel XScale IQ80331/IQ80332 CRB platform specific support routines
// Description:
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_iop33x.h> // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/hal_cache.h>          // Cache definitions
#include <cyg/hal/iq8033x.h>             // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1  _platform_setup1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

// ------------------------------------------------------------------------
// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.
#define CYGHWR_LED_MACRO                                     \
        b       667f                                        ;\
   666:                                                     ;\
        .byte   DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3  ;\
        .byte   DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7  ;\
        .byte   DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B  ;\
        .byte   DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F  ;\
   667:                                                     ;\
        ldr     r0, =666b                                   ;\
        add     r0, r0, #\x                                 ;\
        ldrb    r1, [r0]                                    ;\
        ldr     r0, =DISPLAY_RIGHT                          ;\
        strb    r1, [r0]

// This sets up the base address that the init code
// uses for locking the cache down as sram during
// the initialization process
// This is an arbitrary address but we want to pick
// an otherwise unused address so we can still access
// SDRAM properly
#ifdef DCACHE_FLUSH_AREA
#define XSCALE_CACHE_SRAM_BASE_ADDRESS     DCACHE_FLUSH_AREA
#else
#define XSCALE_CACHE_SRAM_BASE_ADDRESS     0xf0000000
#endif

// ------------------------------------------------------------------------
// This macro locks the CACHE as SRAM at address \sram_base_addr
.macro _enable_and_lock_cache_as_sram sram_base_addr
        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        mrc     p15, 0, r0, c1, c0, 0   // Enable the Dcache
        orr     r0, r0, #MMU_Control_C
        mcr     p15, 0, r0, c1, c0, 0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        ldr     r0, =1                  // enable the data cache lock mode
        mcr     p15, 0, r0, c9, c2, 0
        CPWAIT  r0

        // What address to lock into cache
        ldr     r0, =\sram_base_addr
        // How many lines to lock
        ldr     r1, =(HAL_DCACHE_LOCKABLE_SIZE/HAL_DCACHE_LINE_SIZE)

        ldr     r2, =0                  // Used
        ldr     r3, =0                  // to
        ldr     r4, =0                  // zero
        ldr     r5, =0                  // initialize
        ldr     r6, =0                  // the
        ldr     r7, =0                  // cache
        ldr     r8, =0                  // region
        ldr     r9, =0                  //

889:    mcr     p15, 0, r0, c7, c2, 5   // allocate DCache line at address r0
        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        stmia   r0!, {r2-r9}            // Zero initialize
        subs    r1, r1, #1              // Check next line
        bne     889b

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        ldr     r0, =0
        mcr     p15, 0, r0, c9, c2, 0   // disable the data cache lock mode
        CPWAIT  r0
.endm

// ------------------------------------------------------------------------
// This macro unlocks the CACHE
        .macro _unlock_and_disable_cache
        mcr     p15, 0, r0, c9, c2, 1   // unlock the data cache
        mcr     p15, 0, r0, c7, c6, 0   // invalidate entire DCache
        mrc     p15, 0, r0, c1, c0, 0   // Disable the Dcache
        bic     r0, r0, #MMU_Control_C
        mcr     p15, 0, r0, c1, c0, 0
        CPWAIT  r0
.endm

// ------------------------------------------------------------------------
// This macro represents the initial startup code for the platform
        .macro _platform_setup1

        // This is where we wind up immediately after reset. At this point, we
        // are executing from the boot address (0x00000000), not the eventual
        // flash address. Do some basic setup using position independent code
        // then switch to real flash address

        ldr     r0,=(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0

        // enable coprocessor access
        ldr     r0, =0x20c1              // CP13,CP7,CP6,CP0
        mcr     p15, 0, r0, c15, c1, 0
        CPWAIT  r0

        // Setup PBIU chip selects
        ldr     r8, =PBIU_PBCR

        ldr     r2, =PBLR_SZ_1M
        ldr     r1, =PCE1_PHYS_BASE | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_8
        str     r1, [r8, #0x10]  // PBIU_PBAR1
        str     r2, [r8, #0x14]  // PBIU_PBLR1

        // ====================================================================
        HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_0, DISPLAY_1
        // ====================================================================

        // Enable the Icache
        mrc     p15, 0, r0, c1, c0, 0
        orr     r0, r0, #MMU_Control_I
        mcr     p15, 0, r0, c1, c0, 0
        CPWAIT  r0

#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        //
        // Lock two lines into the I-Cache at address 0
        // This is so that exception fetches will succeed since they
        // come from the I-Cache rather than the D-Cache.
        //
        ldr     r0, =0                       // Address to lock
        mcr     p15, 0, r0, c9, c1, 0        // lock it
        ldr     r0, =HAL_ICACHE_LINE_SIZE    // Next ICache Line
        mcr     p15, 0, r0, c9, c1, 0        // lock it
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

        // ====================================================================
        HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_0, DISPLAY_2
        // ====================================================================

        // value to write into PBIU_PBAR0 to establish runtime flash address
        ldr     r5, [r8, #0x8]  // PBIU_PBAR0
        ands    r5, r5, #0x3    // Save the current bus width

        // Set to read only to insure the flash is not accidently overwritten by a runaway applications.
        ldr     r0, =PCE0_PHYS_BASE | PBAR_READONLY |PBAR_RCWAIT_20 | PBAR_ADWAIT_20
        orr     r5, r5, r0

        // value to write into PBIU_PBLR0 to establish runtime flash address
        ldr     r6, =PBLR_SZ_8M

        // value to load into pc to jump to real runtime address
        ldr     r7, =1f

        // ====================================================================
        HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_0, DISPLAY_3
        // ====================================================================

        // value to load into the MMU Control register set the TTB register
        ldr     r11, =(mmu_table - IQ8033X_FLASH_ADDR + PCE0_PHYS_BASE)

        // Enable permission checks in all domains
        ldr     r0, =0x55555555
        mcr     p15, 0, r0, c3, c0, 0

        // value to load into the MMU Control register to Enable the MMU
        mrc     p15, 0, r12, c1, c0, 0
        orr     r12, r12, #(MMU_Control_M)
        orr     r12, r12, #(MMU_Control_R)

        b       icache_boundary
        .p2align 5
icache_boundary:
        // Here is where we switch from boot address (0x000000000) to the
        // actual flash runtime address. We align to cache boundary so we
        // execute from cache during the switchover. Cachelines are 8 words.
        str     r5, [r8, #0x08]         // PBIU_PBAR0
        str     r6, [r8, #0x0c]         // PBIU_PBLR0
        mcr     p15, 0, r11, c2, c0, 0  // Set the TTB
        mcr     p15, 0, r12, c1, c0, 0  // Enable the MMU
        CPWAIT  r0                      // 3 instructions, ie 3 words
        mov     pc, r7

    1:
        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_4
        // ====================================================================

        // Setup the CACHE as SRAM at an arbitrary address
        // So that we can call the C function for system initialization
        _enable_and_lock_cache_as_sram XSCALE_CACHE_SRAM_BASE_ADDRESS

        ldr     sp, =(XSCALE_CACHE_SRAM_BASE_ADDRESS+HAL_DCACHE_LOCKABLE_SIZE-12-4)
        add     r0, sp, #12             // parm1 == &sdram_size
        add     r1, sp, #8              // parm2 == &sdram_pci_size
        add     r2, sp, #4              // parm3 == &config_flags
        bl      hal_platform_setup
        ldmib   sp!, {r9-r11}           // r9 == config_flags, r10 == sdram_pci_size, r11 == sdram_size

        // Unlock the CACHE
        _unlock_and_disable_cache
        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_2, DISPLAY_C
        // ====================================================================

#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        _enable_and_lock_cache_as_sram 0

        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_2, DISPLAY_D
        // ====================================================================
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        mcr     p15, 0, r0, c7, c7, 0   // flush Icache, Dcache and BTB
        mcr     p15, 0, r0, c8, c7, 0   // flush instuction and data TLBs
        mrc     p15, 0, r0, c1, c0, 0   // Enable the Dcache and BTBs
        orr     r0, r0, #(MMU_Control_C)
        orr     r0, r0, #(MMU_Control_BTB)
        mcr     p15, 0, r0, c1, c0, 0
        CPWAIT r0

        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_2, DISPLAY_E
        // ====================================================================

        // Save boot time PCI config flags
        ldr     r1, =hal_pci_cfg_flags
        str     r9, [r1]

        // Save SDRAM PCI size
        ldr     r1, =hal_dram_pci_size
        str     r10, [r1]

        // Save SDRAM size
        ldr     r1, =hal_dram_size
        str     r11, [r1]

#ifndef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_2, DISPLAY_F
        // ====================================================================

        // Move mmu tables into RAM so page table walks by the cpu
        // don't interfere with FLASH programming.
        ldr     r0, =mmu_table               // Start of ROM tables
        add     r2, r0, #0x4000              // End of ROM tables
        ldr     r1, =(SDRAM_BASE | 0x4000)   // Start of RAM tables

    1:
        ldmia   r0, {r3-r10}
        stmia   r1, {r3-r10}
        mcr     p15, 0, r1, c7, c10, 1         // clean the dcache line
        add     r0, r0, #HAL_DCACHE_LINE_SIZE
        add     r1, r1, #HAL_DCACHE_LINE_SIZE
        cmp     r0, r2
        bne     1b

        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_3, DISPLAY_0
        // ====================================================================

        // Set the TTB register to DRAM mmu_table
        ldr     r0, =(SDRAM_PHYS_BASE | 0x4000) // RAM tables
#if defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE) && \
    !defined(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE_0)
        //
        // Convert Virtual to Physical
        //
        add     r0, r0, #(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2 << 20)
#endif
        mcr     p15, 0, r0, c2, c0, 0           // load page table pointer
        CPWAIT  r0

        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_3, DISPLAY_1
        // ====================================================================
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

        // ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_D
        // ====================================================================
        .endm    // _platform_setup1

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  hal_pci_cfg_flags
hal_pci_cfg_flags:   .long   0
        .endm     // _platform_vectors

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
