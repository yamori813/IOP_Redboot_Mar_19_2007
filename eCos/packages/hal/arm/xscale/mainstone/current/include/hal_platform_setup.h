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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-07-15
// Purpose:      Intel XScale Mainstone specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_bulverde.h>       // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/mainstone.h>          // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1  _platform_setup1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

.macro  NOPs count
        .rept \count
        nop
        nop
        .endr
.endm

// ------------------------------------------------------------------------
// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.
#define CYGHWR_LED_MACRO                                     \
	mov     r0, #MAINSTONE_REGS_BASE ;                   \
	ldr     r1, [r0, #MAINSTONE_LEDCTRL] ;               \
        orr     r1, r1, #0xff ;                              \
	bic     r1, r1, #\x;                                 \
	str     r1, [r0, #MAINSTONE_LEDCTRL] ;

#define PAUSE                            \
        ldr     r1,=0x8000;              \
  555:  sub     r1,r1,#1;                \
        cmp     r1,#0;                   \
        bne     555b;

#define DCACHE_SIZE (32 * 1024)

// ------------------------------------------------------------------------
// MCU Register Values

// ------------------------------------------------------------------------
// This macro represents the initial startup code for the platform        
	.macro _platform_setup1
	// This is where we wind up immediately after reset. At this point, we
	// are executing from the boot address (0x00000000), not the eventual
	// flash address. Do some basic setup using position independent code
	// then switch to real flash address using MMU.
        ldr     r0,=(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0

        // enable coprocessor access
	ldr	r0, =0x20c3              // CP13,CP7,CP6,CP1,CP0
        mcr     p15, 0, r0, c15, c1, 0

	// Enable the Icache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_I
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

	// Drain write and fill buffer
	mcr	p15, 0, r0, c7, c10, 4
	CPWAIT	r0

	// The Mainstone board has two "virtual" registers which are comprised
	// of GPIO lines. These two 16-bit registers must be read before GPIO
	// is configured for norma board operation. The two registers are:
	//
	// System Configuration Register - GPIO 73:58
	// LCD Configuration Register    - GPIO 87:86,77,75:74,19,14
	//
	
	ldr     r10, =POWER_BASE
	mov     r0, #(PSSR_PH | PSSR_RDH)
        str     r0, [r10, #POWER_PSSR]           // disable all GPIOs
        mov     r0, #0
        str     r0, [r10, #POWER_PSSR]           // enable all GPIOs

	// wait for newly enabled lines to settle
        mov     r0, #100
    0:  subs    r0, r0, #1
        bne     0b
 
        ldr     r11, =GPIO_BASE                   // GPIO registers base address
	
        ldr     r1, [r11, #GPIO_GPLR1]            //  GPIO 63:32
        ldr     r2, [r11, #GPIO_GPLR2]            //  GPIO 95:64
	
        mov     r1, r1, lsr #26
        mov     r1, r1, lsl #16
        orr     r1, r1, r2, lsl #22               // upper 16 bits now have GPIO 73:58
	and     r3, r2, #0x2000                   // GPIO bit 77
	and	r4, r2, #0xc00                    // GPIO bits 75:74
	orr     r3, r3, r4, lsl #1                // go just to the right of GPIO bit 77
        and     r4, r2, #0xc00000                 // GPIO bits 87:86
	orr     r3, r3, r4, lsr #8                // go just to the left of GPIO bit 77
        ldr     r2, [r11, #GPIO_GPLR0]            // GPIO 31:0
	and     r4, r2, #0x4000                   // GPIO 14
	orr     r3, r3, r4, lsr #14
	and     r4, r3, #0x80000                  // GPIO 19
	orr     r3, r3, r4, lsr #18
	orr	r1, r1, r3, lsr #9	
	
	// r1 now contains the SCR (bits 31:16) and LCDCR (bits 15:0)
	// save it temporarily in power manager scratchpad register
        str     r1, [r10, #POWER_PSPR]

	// Now it safe to set up the GPIO pins for normal operation.

	bl	1f
	// Table of GPIO output pins which should be set.
	.word   0x00008800    // GPIO [31:0]
	.word   0x03cf0002    // GPIO [63:32]
	.word   0x0021FC00    // GPIO [95:64]
	.word   0x00000000    // GPIO [120:96]

	// Table of GPIO output pins which should be cleared.
	.word   0x00000000    // GPIO [31:0]
	.word   0x00000000    // GPIO [63:32]
	.word   0x00000000    // GPIO [95:64]
	.word   0x00000000    // GPIO [120:96]

	// Table of GPIO pins which should be outputs.
	.word   0xC27B9C04    // GPIO [31:0]
	.word   0x00EFAA83    // GPIO [63:32]
	.word   0x0E23FC00    // GPIO [95:64]
	.word   0x001E1F81    // GPIO [120:96]

	// Table of GPIO pin "alternate function" settings. Each pin
	// may be "normal" or one of three alternate functions.
	.word   0x94F00000    // GPIO [15:0]
	.word   0x015A859A    // GPIO [31:16]
	.word   0x999A955A    // GPIO [47:32]
	.word   0x0005A4AA    // GPIO [63:48]
	.word   0x6AA00000    // GPIO [79:64]
	.word   0x55A8041A    // GPIO [95:80]
	.word   0x56AA955A    // GPIO [111:96]
	.word   0x00000001    // GPIO [120:112]
    1:
        ldr     r11, =GPIO_BASE
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPSR0]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPSR1]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPSR2]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPSR3]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPCR0]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPCR1]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPCR2]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPCR3]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPDR0]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPDR1]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPDR2]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GPDR3]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR0_L]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR0_U]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR1_L]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR1_U]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR2_L]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR2_U]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR3_L]
	ldr	r0, [r14], #4
	str	r0, [r11, #GPIO_GAFR3_U]
	
	ldr     r10, =POWER_BASE
	mov     r0, #(PSSR_PH | PSSR_RDH)
        str     r0, [r10, #POWER_PSSR]           // disable all GPIOs
        mov     r0, #0
        str     r0, [r10, #POWER_PSSR]           // enable all GPIOs

        // Turn on hex display
	mov     r0,#MAINSTONE_REGS_BASE
	mov     r1,#0xff
	str     r1, [r0, #MAINSTONE_LEDCTRL]

        HEX_DISPLAY r0,r1,0x25000FFF

	// ===============================================================
	// Now we initialize the memory controller as specified in
	// the Bulverde manual (Chapter 6, Section 6.4.9)

	// ======== Step 1 =======
	//
	// Delay ~200us
        ldr     r10, =OSTIMER_BASE
        mov     r0, #0 
        str     r0, [r10, #OSTIMER_OSCR0]
    1:
        ldr     r1, [r10, #OSTIMER_OSCR0]
        cmp     r1, #0x300
        ble     1b

        HEX_DISPLAY r8,r9,0x24000FFF
	
        ldr     r10, =MCU_BASE

	// Init MSCx registers. (Readback after write is necessary)
        ldr     r0, =((CS1_CONFIG << 16) | CS0_CONFIG)
        str     r0, [r10, #MCU_MSC0]
        ldr     r0, [r10, #MCU_MSC0]
        ldr     r0, =((CS3_CONFIG << 16) | CS2_CONFIG)
        str     r0, [r10, #MCU_MSC1]
        ldr     r0, [r10, #MCU_MSC1]
        ldr     r0, =((CS5_CONFIG << 16) | CS4_CONFIG)
        str     r0, [r10, #MCU_MSC2]
        ldr     r0, [r10, #MCU_MSC2]

	//  Init MECR, MCMEM0, MCMEM1, MCATT0, MCATT1, MCIO0, MCIO1
        ldr     r0, =MECR_CONFIG
        str     r0, [r10, #MCU_MECR]
        ldr     r0, =MCMEM0_CONFIG
        str     r0, [r10, #MCU_MCMEM0]
        ldr     r0, =MCMEM1_CONFIG
        str     r0, [r10, #MCU_MCMEM1]
        ldr     r0, =MCATT0_CONFIG
        str     r0, [r10, #MCU_MCATT0]
        ldr     r0, =MCATT1_CONFIG
        str     r0, [r10, #MCU_MCATT1]
        ldr     r0, =MCIO0_CONFIG
        str     r0, [r10, #MCU_MCIO0]
        ldr     r0, =MCIO1_CONFIG
        str     r0, [r10, #MCU_MCIO1]

	// Write to MDCNFG (only if waking up)
	// 
        HEX_DISPLAY r8,r9,0x24100FFF
	
	// ???
	mov	r0, #0
        str	r0, [r10, #32]
	
	// MDREFR settings
        ldr	r0, [r10, #MCU_MDREFR]
        mov	r0, r0, lsr #12               // Clear DRI
        mov	r0, r0, lsl #12
        ldr     r1, =MDREFR_CONFIG
        orr     r0, r0, r1
        str     r0, [r10, #MCU_MDREFR]
        orr     r0, r0, #MDREFR_K0RUN
        bic     r0, r0, #MDREFR_K0DB2
        orr     r0, r0, #MDREFR_K0DB4
        bic     r0, r0, #(MDREFR_K1FREE | MDREFR_K2FREE)
        orr     r0, r0, #MDREFR_K0FREE
        str     r0, [r10, #MCU_MDREFR]

        HEX_DISPLAY r8,r9,0x24200FFF
	// ======== Step 2 =======
	//
	ldr     r1, =SXCNFG_CONFIG
	str     r1, [r10, #MCU_SXCNFG]

	// ======== Step 3 =======
	//
        bic     r0, r0, #(MDREFR_K2FREE | MDREFR_K1FREE | MDREFR_K0FREE)
        orr     r0, r0, #MDREFR_K1RUN
	orr     r0, r0, #MDREFR_K1DB2
        str     r0, [r10, #MCU_MDREFR]
        ldr     r0, [r10, #MCU_MDREFR]
        bic     r0, r0, #MDREFR_SLFRSH
        str     r0, [r10, #MCU_MDREFR]
        ldr     r0, [r10, #MCU_MDREFR]
        orr     r0, r0, #MDREFR_E1PIN
        str     r0, [r10, #MCU_MDREFR]
        ldr     r0, [r10, #MCU_MDREFR]  // readback to confirm data latched
        nop
        nop
	
        HEX_DISPLAY r8,r9,0x24300FFF
	// ======== Step 4 =======
	//
        ldr     r0, =MDCNFG_CONFIG
        bic     r0, r0, #(MDCNFG_DE0 | MDCNFG_DE1)
        bic     r0, r0, #(MDCNFG_DE2 | MDCNFG_DE3)
        bic     r0, r0,  #MDCNFG_DWID0_16
        str     r0, [r10, #MCU_MDCNFG]

	// ======== Step 5 =======
	//
	// Delay ~200us
        ldr     r11, =OSTIMER_BASE
        mov     r0, #0 
        str     r0, [r11, #OSTIMER_OSCR0]
    1:
        ldr     r1, [r11, #OSTIMER_OSCR0]
        cmp     r1, #0x300
        ble     1b

        HEX_DISPLAY r8,r9,0x24400FFF
	// ======== Step 6 =======
	//
        mov     r0, #0x78
        mcr     p15, 0, r0, c1, c0, 0

	// ======== Step 7 =======
	//
        ldr     r0, =SDRAM_PHYS_BASE
        str     r0, [r0]
        str     r0, [r0]
        str     r0, [r0]
        str     r0, [r0]
        str     r0, [r0]
        str     r0, [r0]
        str     r0, [r0]
        str     r0, [r0]
	str     r0, [r0]  //  Fix for erratum #116.

        HEX_DISPLAY r8,r9,0x24500FFF
	// ======== Step 8 =======
	//
	// Re-enable D-cache if desired.

	// ======== Step 9 =======
	//
        ldr     r0, [r10, #MCU_MDCNFG]
        orr     r0, r0, #MDCNFG_DE0
        str     r0, [r10, #MCU_MDCNFG]

	// ======== Step 10 =======
	//
        ldr     r0,  =MDMRS_CONFIG
        str     r0,  [r10, #MCU_MDMRS]

        HEX_DISPLAY r8,r9,0x24600FFF
	// ======== Step 11 =======
	//
        ldr     r0, [r10, #MCU_MDREFR]
        orr     r0, r0, #MDREFR_APD
	str     r0, [r10, #MCU_MDREFR]

//        ldr     r0, =MDMRSLP_CONFIG
//        str     r0, [r10, #MCU_MDMRSLP]

	HEX_DISPLAY r0,r1,0x25800FFF

	// ====================================================================
	// Setup Clocks

	// turn off all clocks except MCU and OStimer
        ldr     r10, =CLOCK_BASE
	ldr     r0, =(CKEN_OSTIMER | CKEN_MCU)
        str     r0, [r10, #CLOCK_CKEN]

	// The following code gets the value of the low order hex switch,
	// shifts the data one bit to the left and replaces the LS bit with
	// a 1. This value is loaded onto the CCCR register as the value for
	// "L" which is multiplied by 13MHz to get the core processor speed.
	// The result is odd values of L in the range of 0x1 to 0x1F in odd
	// numbered steps. The value 0x1, however is an invalid value for L
	// (when the hex switch is set to zero). For this special case the
	// CCCR value for L is replaced with 0x7 which is the power on
	// default value for L

	// Run-mode frequency = 13MHz * L
	// Turbo-mode frequency = Run-mode frequency * N
	
        mov     r0,  #(OSCC_OON | OSCC_TOUT_EN | OSCC_PIO_EN)
        str     r0,  [r10, #CLOCK_OSCC]

	mov     r11, #MAINSTONE_REGS_BASE
        ldr	r0, [r11, #MAINSTONE_GPSWR]  // read switch
	and     r0, r0, #0xf                 // isolate SW9
        movs    r0, r0, lsl #1
	moveq   r0, #7
	orrne   r0, r0, #1
	orr     r0, r0, #(2 << CCCR_2N_SHIFT) // N=1
        str     r0, [r10, #CLOCK_CCCR]

        ldr     r10, =RTC_BASE
	mov     r2, #0
	str     r2, [r10, #RTC_RTSR]
	str     r2, [r10, #RTC_RCNR]
	str     r2, [r10, #RTC_RTAR]
	str     r2, [r10, #RTC_SWCR]
	str     r2, [r10, #RTC_SWAR1]
	str     r2, [r10, #RTC_SWAR2]
	str     r2, [r10, #RTC_PICR]
	str     r2, [r10, #RTC_PIAR]
	
	HEX_DISPLAY r0,r1,0x25820FFF

	// Start frequency change sequence
        mrc     p14, 0, r0, c6, c0, 0
        orr     r0, r0, #2
        mcr     p14, 0, r0, c6, c0, 0

	//  MDREFR must be rewritten after frequency change
        ldr     r10, =MCU_BASE
        ldr     r1, [r10, #MCU_MDREFR]
        str     r1, [r10, #MCU_MDREFR]

	HEX_DISPLAY r0,r1,0x25830FFF

	// ===============================================================
	// Page Table Setup

	// First, fill the first level page table with invalid entries.
	mov	r12, #SDRAM_PHYS_BASE
	add	r12, r12, #0x4000	// start of table

	// Set the TTB register
	mcr	p15, 0, r12, c2, c0, 0

	ldr	r0, =MMU_L1_TYPE_Fault
	mov	r1, #4096
	add	r2, r12, #0
    1:
	str	r0, [r2], #4
	subs	r1, r1, #1
	bne	1b
	
	// 64MB SDRAM
	ldr     r0, =PTE_SECTION_RAM
	orr	r0, r0, #SDRAM_PHYS_BASE
	mov	r1, #(SDRAM_SIZE>>20)
	add	r2, r12, #(SDRAM_BASE>>18)
    1:
	str	r0, [r2], #4
	add	r0, r0, #(1024*1024)
	subs	r1, r1, #1
	bne	1b

	// 64MB Uncached SDRAM alias
	ldr     r0, =PTE_SECTION_UNCACHED
	orr	r0, r0, #SDRAM_PHYS_BASE
	mov	r1, #(SDRAM_SIZE>>20)
	add	r2, r12, #(SDRAM_UNCACHED_BASE>>18)
    1:
	str	r0, [r2], #4
	add	r0, r0, #(1024*1024)
	subs	r1, r1, #1
	bne	1b

	// 64M Application Flash
	ldr     r0, =PTE_SECTION_FLASH
	orr	r0, r0, #MAINSTONE_APP_FLASH_PHYS_BASE
	mov	r1, #64
	add	r2, r12, #(MAINSTONE_APP_FLASH_BASE>>18)
    1:
	str	r0, [r2], #4
	add	r0, r0, #(1024*1024)
	subs	r1, r1, #1
	bne	1b

	// 64M Boot Flash
	ldr     r0, =PTE_SECTION_FLASH
	orr	r0, r0, #MAINSTONE_BOOT_FLASH_PHYS_BASE
	mov	r1, #64
	add	r2, r12, #(MAINSTONE_BOOT_FLASH_BASE>>18)
    1:
	str	r0, [r2], #4
	add	r0, r0, #(1024*1024)
	subs	r1, r1, #1
	bne	1b

	// 1M Board registers
	ldr     r0, =PTE_SECTION_UNCACHED
	orr	r0, r0, #MAINSTONE_REGS_PHYS_BASE
	add	r2, r12, #(MAINSTONE_REGS_BASE>>18)
	str	r0, [r2], #4

	// 2M SRAM
	ldr     r0, =PTE_SECTION_RAM
	orr	r0, r0, #MAINSTONE_SRAM_PHYS_BASE
	add	r2, r12, #(MAINSTONE_SRAM_BASE>>18)
	str	r0, [r2], #4
	str	r0, [r2], #4

	// 1M Ethernet I/F
	ldr     r0, =PTE_SECTION_UNCACHED
	orr	r0, r0, #MAINSTONE_ETHERNET_PHYS_BASE
	add	r2, r12, #(MAINSTONE_ETHERNET_BASE>>18)
	str	r0, [r2], #4

	// 1M DCache Flush Area
	ldr     r0, =PTE_SECTION_RAM
	orr	r0, r0, #DCACHE_FLUSH_PHYS_BASE
	add	r2, r12, #(DCACHE_FLUSH_AREA>>18)
	str	r0, [r2], #4

	// 512M PCMCIA space identity mapped
	ldr     r0, =PTE_SECTION_UNCACHED
	orr	r0, r0, #0x20000000
	mov	r1, #512
	add	r2, r12, #(0x20000000>>18)
    1:
	str	r0, [r2], #4
	add	r0, r0, #(1024*1024)
	subs	r1, r1, #1
	bne	1b

	// 256M Bulverde regs identity mapped
	ldr     r0, =PTE_SECTION_UNCACHED
	orr	r0, r0, #0x40000000
	mov	r1, #256
	add	r2, r12, #(0x40000000>>18)
    1:
	str	r0, [r2], #4
	add	r0, r0, #(1024*1024)
	subs	r1, r1, #1
	bne	1b

	HEX_DISPLAY r0,r1,0x25830FFF
	
	// Enable permission checks in all domains
	ldr	r0, =0x55555555
	mcr	p15, 0, r0, c3, c0, 0
	
	HEX_DISPLAY r0,r1,0x25840FFF
	
	// Value to enable the MMU
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_M
	orr	r0, r0, #MMU_Control_R

	// value to load into pc to jump to real runtime address
	ldr     r1, =1f
	b       icache_boundary

	.p2align 5
icache_boundary:
	// Here is where we turn on MMU and switch from executing from physical
        // boot address to the virtual flash runtime address. We align to cache
        // boundary so we execute from cache during the switchover. Cachelines
        // are 8 words.

	mcr	p15, 0, r0, c1, c0, 0
        mrc     p15, 0, r0, c2, c0, 0
	mov     r0, r0
	sub     pc, pc, #4
        nop
        nop
        nop
        mov     pc, r1
    1:

	HEX_DISPLAY r0,r1,0x25850FFF

	// Enable the Dcache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_C
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        // Enable branch target buffer
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_BTB
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c7, 0   // flush Icache, Dcache and BTB
        CPWAIT  r0

        mcr     p15, 0, r0, c8, c7, 0   // flush instuction and data TLBs
        CPWAIT  r0

        // clean, drain, flush the main Dcache
        ldr     r1, =DCACHE_FLUSH_AREA  // use a CACHEABLE area of
                                        // memory that's mapped above SDRAM
        mov     r0, #1024               // number of lines in the Dcache
    0:
        mcr     p15, 0, r1, c7, c2, 5   // allocate a Dcache line
        add     r1, r1, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

	// clean, drain, flush the mini Dcache
        ldr     r2, =DCACHE_FLUSH_AREA + DCACHE_SIZE
        mov     r0, #64                 // number of lines in the Dcache
    0:
        mcr     p15, 0, r2, c7, c2, 5   // allocate a Dcache line
        add     r2, r2, #32          // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

        mcr     p15, 0, r0, c7, c6, 0   // flush Dcache
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

	HEX_DISPLAY r0,r1,0x25860FFF
	
	// Save SDRAM size
        ldr     r1, =hal_dram_size  /* [see hal_intr.h] */
	mov	r8, #0x4000000
	str	r8, [r1]

	HEX_DISPLAY r0,r1,0x00000000
	
	.endm    // _platform_setup1

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
