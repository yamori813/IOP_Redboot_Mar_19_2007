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
// Author(s):    curt.e.bruns@intel.com
// Contributors: adam.j.brooks@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:      Intel XScale IQ8134 CRB platform specific support routines
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
#include <cyg/hal/hal_iop34x.h> // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/hal_cache.h>          // Cache definitions
#include <cyg/hal/iq8134x.h>             // Platform specific hardware definitions

#if defined (CYG_HAL_STARTUP_ROM) || defined (CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1  _platform_setup1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

// For ROMRAM startup, we need to copy the ROM data/code into RAM
// and jump to it.
#if defined(CYG_HAL_STARTUP_ROMRAM)
#define COPY_ROM_TO_RAM                       					         \
        ldr     r0,=(CYGMEM_REGION_rom)       					        ;\
        ldr     r1,=SDRAM_PHYS_BASE           						    ;\
		ldr		r2,=(__bss_end)          					    	    ;\
1:      ldr     r3,[r0],#4                    						    ;\
        str     r3,[r1],#4                    						    ;\
        cmp     r1,r2                         						    ;\
        bne     1b                            						    ;\
        ldr     r0,=2f                        						    ;\
        mcr     p15, 0, r1, c7, c5, 0   /* inval instruction cache */   ;\
        mcr     p15, 0, r1, c8, c7, 0;  /* flush I+D TLBs */            ;\
        mov     pc,r0                   /* Start SDRAM Execution */     ;\
2:
#endif

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
#define XSCALE_CACHE_SRAM_BASE_ADDRESS     0xC0000000
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
        mcr     p15, 0, r0, c9, c6, 0   // New DCache Lock register write command
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
        mcr     p15, 0, r0, c9, c6, 0   // disable the data cache lock mode
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
	// CP15 - Control processor - access enabled by default
	// CP14 - SW Debug, PMU, Clock/Power - access enabled by default
	// CP07 - Error logging for Ext. Bus and L2 Cache Parity errors
	// CP06 - Interrupts/Timers
	// CP00 - Accumulator
	ldr     r0, =0x20c1              // CP7,CP6,CP0
	mcr     p15, 0, r0, c15, c1, 0
	CPWAIT  r0
	
	// Enable the Icache
	mrc     p15, 0, r0, c1, c0, 0
	orr     r0, r0, #MMU_Control_I
	mcr     p15, 0, r0, c1, c0, 0
	CPWAIT  r0
	
	// Setup PBIU chip select 1
	ldr     r8, =PBIU_PBCR
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
	ldr		r1, =PCE1_PHYS_BASE | PBAR_DDWAIT_20 | PBAR_RCWAIT_20 | PBAR_ADWAIT_20 | PBAR_BUS_8
	ldr		r2, =PBLR_SZ_32M
	str		r1, [r8, #0x10]  // PBIU_PBAR1
	str		r2, [r8, #0x14]  // PBIU_PBLR1

	// Check what board flavor to determine Flash limit
	ldr		r2, =IQ8134X_PRODUCT_CODE_ADDR
	ldrb		r1, [r2]
	
	// Just want lower nibble of product code register
	mov		r2, #IQ8134X_PROD_SKU_MASK
	and		r1, r1, r2
	mov		r2, #IQ8134X_PROD_CODE_IQ8134xMC_MODE2
	cmp		r2, r1
	ldreq		r1, =PCE1_PHYS_BASE | PBAR_DDWAIT_4 | PBAR_RCWAIT_1 | PBAR_ADWAIT_4 | PBAR_BUS_16
	streq		r1, [r8, #0x10]  // PBIU_PBAR1
#endif
	// ====================================================================
	HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_0, DISPLAY_1
	// ====================================================================

	// Read Core ID Register
	// Will return 0 or 1 in r2 depending on which core we are.
	// r2: 0 = transport core, 1 = application core
	mrc 	p6, 0, r2, c0, c0, 0

// If:   PC is has 0xFx2x.xxxx in it, then common_boot code has already remapped
//       Flash and we're at 0xF020.0000.
// This section of code is NOT run if we're a ROMRAM version w/ Transport FW running
#if !defined (CYG_HAL_STARTUP_ROMRAM) || \
    (defined (CYG_HAL_STARTUP_ROMRAM) && (CYGHWR_HAL_APPLICATION_CODE_OFFSET == 0x0))
		.globl CHECK_IF_COMMON_BOOT_RAN
CHECK_IF_COMMON_BOOT_RAN:
		mov 		r0, pc
		ldr		r1, =IQ8134X_FLASH_ADDR
		ands		r0, r0, r1
		bne		common_boot_already_ran

        // ====================================================================
        HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_0, DISPLAY_F
        // ====================================================================

		// Check if we are a Single Core device
		ldr		r0, =ATUE_ATUDID_ASSEMBLER
		ldrh		r1, [r0]
		ldr		r3, =DEVICE_ID_3380          // single core
		cmp		r1, r3
		beq		single_core_sku      // Single Core - other core gone

		// If ATUE_PCSR[0] or [1] is set, then the other core is in reset
		// and we can be the master processor
		ldr  	r0, =ATUE_PCSR_ASSEMBLER     // Controller Only Jumper Absent, Dual Core
		ldr		r0, [r0]
		ands	r0, r0, #0x3
		bne		common_boot_already_ran

		// If we want to run on only a single core, this is where RedBoot will
		// hang the other core.
		// If we're a MC, or we're not using PCE1, 
		// then we skip the rotary test and just boot core1
		
		cmp		r2, #0
		bne		common_boot_already_ran
		// Hanging Core0 if MC and both are running
		ldr		r0, =IMU_TSR_HALT  // Halted core grabs "HALT" semaphore
hang_core_0:
		ldrb	r1, [r0]
		b		hang_core_0
#endif  // !ROMRAM

		// For Dual-Core - only the master core will be executing this
		// code.  Get the Master TSR and work our way through board init.
		.globl common_boot_already_ran
common_boot_already_ran:
		// Obtain Master Semaphore
		ldr		r0, =IMU_TSR_MASTER
		ldrb		r1, [r0]
single_core_sku:
        // ====================================================================
        HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_1, DISPLAY_F
        // ====================================================================

#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
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
		// r8 loaded with PBIU_PBCR address
        ldr     r5, [r8, #0x8]  // PBIU_PBAR0
        and     r5, r5, #0x3    // Save the current bus width

        // value to write into PBIU_PBAR0 to establish runtime flash address
        ldr     r0, =PCE0_PHYS_BASE | PBAR_RCWAIT_20 | PBAR_ADWAIT_20
        orr     r5, r5, r0

#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
	ldr		r6, =PBLR_SZ_32M
#else
	ldr		r6, =PBLR_SZ_8M
#endif

		.globl LOAD_MMU_VALUE
LOAD_MMU_VALUE:
// ROMRAM will figure out where to jump based on Flash location and offset of label
// ROMRAM is linked at 0x0 (RAM default address), which is why this needs done
#if defined (CYG_HAL_STARTUP_ROMRAM)
		ldr		r7, =CYGMEM_REGION_rom
		ldr		r11, =jump_to_flash
		add		r7, r7, r11
        // value to load into the MMU Control register set the TTB register
        ldr     r11, =(mmu_table + CYGMEM_REGION_rom)
#else
        // value to load into pc to jump to real runtime address
        ldr     r7, =jump_to_flash
        // value to load into the MMU Control register set the TTB register
        ldr     r11, =(mmu_table - IQ8134X_FLASH_ADDR + PCE0_PHYS_BASE)
#endif

#ifdef CYGSEM_HAL_ENABLE_L2_CACHE
		// Allow table walks to load descriptors into L2
		ldr		r0, =OUTER_CACHEABLE_TABLE_WALKS
		add		r11, r0, r11
#endif
        // ====================================================================
        HEX_DISPLAY_NO_MMU r0, r1, DISPLAY_0, DISPLAY_3
        // ====================================================================

        // Enable permission checks in all domains
        ldr     r0, =0x55555555
        mcr     p15, 0, r0, c3, c0, 0

        // value to load into the ARM Control register to Enable the MMU
        mrc     p15, 0, r12, c1, c0, 0
        orr     r12, r12, #(MMU_Control_M)
        orr     r12, r12, #(MMU_Control_R)
#ifdef CYGSEM_HAL_ENABLE_L2_CACHE
		orr 	r12, r12, #(MMU_Control_L2)
#endif

// Check for other core being halted is not required if CB ran already
// This code is only needed for ROM version or ROMRAM which does not have
// Flash remapped yet.
#if !defined (CYG_HAL_STARTUP_ROMRAM) || \
    (defined (CYG_HAL_STARTUP_ROMRAM) && \
    (CYGHWR_HAL_APPLICATION_CODE_OFFSET == 0x0))


		// Check if we are single core
		ldr		r0, =ATUE_ATUDID_ASSEMBLER
		ldrh		r1, [r0]
		ldr		r2, =DEVICE_ID_3380          // single core
		cmp		r1, r2
		beq		icache_boundary      // Single Core - other core gone

		// Check if other core is halted
		// If ATUE_PCSR[0] or [1] is set, then the other core is in reset
		// and we can move the Flash
		ldr	r0, =ATUE_PCSR_ASSEMBLER
		ldr		r0, [r0]
		ands	r0, r0, #0x3
		bne		icache_boundary

		// Both cores enabled so we need to check that the other core has
		// been halted
		// Read Core ID Register
		// Will return 0 or 1 in r2 depending on which core we are.
		// r2: 0 = transport core, 1 = application core
		mrc 	p6, 0, r2, c0, c0, 0
		cmp		r2, #0
		// Set R2 to the Other Core's ID so we can check the TSR register
		moveq   r2, #2
		movne	r2, #1
		ldr		r0, =IMU_TSR_HALT

		// Store 0 into HALT TSR
		mov		r3, #0
1:
		strb	r3, [r0]
		// Put some	jumps in here to allow other core to grab semaphore if needed
		b		2f
3:
		b		4f
2:
		b		3b
4:
		ldrb	r1, [r0]
		cmp		r1, r2
		// Continue to check the HALT semaphore until other core grabs it
		bne		1b
#endif

        b       icache_boundary
        .p2align 5
icache_boundary:
        // Here is where we switch from boot address (0x000000000) to the
        // actual flash runtime address. We align to cache boundary so we
        // execute from cache during the switchover. Cachelines are 8 words.

		// The common_boot code for the ROMRAM version will have already setup
		// the PBIU_PBAR0 which is why the next two instructions are conditional
		// on the startup type.
#if !defined (CYG_HAL_STARTUP_ROMRAM) || \
    (defined (CYG_HAL_STARTUP_ROMRAM) && \
    (CYGHWR_HAL_APPLICATION_CODE_OFFSET == 0x0))
        str     r5, [r8, #0x08]         // PBIU_PBAR0
        str     r6, [r8, #0x0c]         // PBIU_PBLR0
#endif
        mcr     p15, 0, r11, c2, c0, 0  // Set the TTB
        mcr     p15, 0, r12, c1, c0, 0  // Enable the MMU
        CPWAIT  r0                      // 3 instructions, ie 3 words
#if !defined (CYG_HAL_STARTUP_ROMRAM) || \
    (defined (CYG_HAL_STARTUP_ROMRAM) && \
    (CYGHWR_HAL_APPLICATION_CODE_OFFSET == 0x0))
        mov     pc, r7                  // Not required if Common Boot has already run
#endif

    jump_to_flash:
        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_0, DISPLAY_4
        // ====================================================================

		.globl LOCKING_CACHE_AS_SRAM
LOCKING_CACHE_AS_SRAM:
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

#if defined (CYG_HAL_STARTUP_ROMRAM)
		// Since we are a ROMRAM version, transport FW has locked Icache lines.
		// We must unlock and invalidate so as to not fetch from common boot code
		// when SDRAM is running @ 0
		// Globally Unlock L1 ICache
		mov		r0, #0
		mcr 	p15, 0, r0, c9, c5, 1
		// Globally Invalidate L1 ICache and BTB
		mcr		p15, 0, r0, c7, c5, 0

		// ROMRAM copies ROM code to RAM and then jumps to it.  At this point DDR has been
		// initialized, so this copy can take place.
		.globl ROM_COPY
ROM_COPY:
		COPY_ROM_TO_RAM

		// Now that we've relocated everything to SDRAM at 0x0, we need to reload the
		// vectors into the debugger if we're using JTAG.  The Prefetch Abort handler
		// will ignore this exception if we're not using JTAG
		//bkpt 0x1234;  -- Do not need on XScale Core gen >= 3

#endif


	   // For memoryless boots - this CDL option can be enabled and will give us
	   // 24k of SRAM
#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        _enable_and_lock_cache_as_sram 0

        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_2, DISPLAY_D
        // ====================================================================
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

		.globl ENABLING_DATACACHE
ENABLING_DATACACHE:
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

		.globl SAVE_BOOT_FLAGS
SAVE_BOOT_FLAGS:
        // Save boot time PCI config flags
        ldr     r1, =hal_pci_cfg_flags
        str     r9, [r1]

        // Save SDRAM PCI size
        ldr     r1, =hal_dram_pci_size
        str     r10, [r1]

        // Save SDRAM size
        ldr     r1, =hal_dram_size
        str     r11, [r1]

		.globl MOVE_MMU_TABLES
MOVE_MMU_TABLES:
#ifndef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_2, DISPLAY_F
        // ====================================================================

        // Move mmu tables into RAM so page table walks by the cpu
        // don't interfere with FLASH programming.
#if !defined  (CYG_HAL_STARTUP_ROMRAM)
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

        // Read TTB Base
		mrc     p15, 0, r0, c2, c0, 0
		// Save OuterCacheable, P-bit, S-bit Attributes
		ldr		r1, =0x3FFF
		and		r1, r1, r0
        ldr     r0, =(SDRAM_PHYS_BASE | 0x4000) // RAM tables
#else	// Else - we're running ROMRAM version so our tables are already in RAM
	  	ldr		r0, =(mmu_table + SDRAM_PHYS_BASE)
#ifdef CYGSEM_HAL_ENABLE_L2_CACHE
		// Allow table walks to load descriptors into L2
		ldr		r1, =OUTER_CACHEABLE_TABLE_WALKS
#endif

#endif

#if defined(CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE)
        //
        // Convert Virtual to Physical
        //
        add     r0, r0, #(CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2 << 20)
#endif
		orr     r0, r0, r1
        mcr     p15, 0, r0, c2, c0, 0           // load TTB - page table pointer
		CPWAIT  r0

#if defined  (CYG_HAL_STARTUP_ROMRAM)
		// We are now executing out of SDRAM and our TTB will point to SDRAM, so
		// we can release our FAC1 semaphore
		ldr		r0, =IMU_TSR_FAC1
		mov		r1, #0
		strb	r1, [r0]
#endif
        // ====================================================================
        HEX_DISPLAY r0, r1, DISPLAY_3, DISPLAY_1
        // ====================================================================
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

        // ====================================================================
	HEX_DISPLAY r0, r1, DISPLAY_A, DISPLAY_D
        // ====================================================================

        .endm    // _platform_setup1
#else
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
// Contains PCI Config Flags (Master on PCIX, PCIe, etc)
        .globl  hal_pci_cfg_flags
hal_pci_cfg_flags:   .long   0

// Contains Flags for which processor is Master, Slave, memory installed, etc
        .globl  hal_dual_boot_flags
hal_dual_boot_flags: .long   0
        .endm     // _platform_vectors

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
