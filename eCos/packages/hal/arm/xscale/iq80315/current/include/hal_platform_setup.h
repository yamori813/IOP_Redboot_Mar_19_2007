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
// Contributors: dkranak, cebruns
// Date:         2001-12-03
// Purpose:      Intel XScale IQ80315 platform specific support routines
// Description:
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"
//
//  Modified:
//  2002Sep01   dkranak    Derivitive of IQ80321 for IQ80315
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/
#define MEMORY_DMA_INIT   1              // Use DMA to initialze memory
#define ENABLE_ECC                       // Enables ECC in the XScale, CIU, and MCU
#define ENABLE_PARITY                    // Enables PARITY in the SRAM 
#undef  DEBUG_OVERLAY_DDR_WITH_SRAM      // Use SRAM as Memory - the first 1Meg
#define USE_SDRAM                        // Do SPD scan and init. memory - if it's undef'd we need to use SRAM

#ifndef USE_SDRAM
#ifndef DEBUG_OVERLAY_DDR_WITH_SRAM
error - You must either use SDRAM or SRAM - cannot go memoryless ****
#endif
#endif
								  
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_ioc80314.h>         // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/iq80315.h>            // Platform specific hardware definitions
#include <cyg/hal/hal_ioc80314_pci.h>

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1  _platform_setup1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

.macro  X_NOPs count
        .rept \count
        nop
        nop
        .endr
.endm

// SFabric init address , value
#define TCSR_INIT(sfadr, sfval)               \
Boot_##sfadr: .word  sfadr, sfval

//CSR_ADDR(reg_comp,reg_name)     (HAL_IOC80314_CSR_BASE + reg_comp##BASE   + reg_comp##reg_name)

// SFabric init componet class, reg name , value
#define TCSR_REG(sfclass, sfreg, sfval)               \
Boot_##sfclass##_##sfreg##: .word  (HAL_IQ80315_##sfclass##_BASE + TS_##sfclass##_##sfreg), sfval

#define TCSR_DMA(sfchnl, sfreg, sfval)               \
               .word  (DMA_REG_ADDR(sfchnl, sfreg)), sfval

#define TCSR_REG1(sfclass, sfreg, sfval)               \
Boot1_##sfclass##_##sfreg##: .word  (HAL_IQ80315_##sfclass##_BASE + TS_##sfclass##_##sfreg), sfval

#define TCSR_PAUSE(pPAUSE)             \
Boot_##pPAUSE##: .word  0x0, 0x0

#define  SetCSRfromTable(pTable, LableCnt)               \
	ldr		r1, =Boot_loc##LableCnt  ;					 \
	ldr		r2, =pTable - 4 ;							 \
	sub		r1, r1, r2  ;							     \
	sub		r0, pc, r1 ;								 \
    .set Boot_loc##LableCnt, . ;                         \
    ldr    r1,[r0],#4   ; /* get address            */   \
    ldr    r2,[r0],#4   ; /* get data               */   \
    cmp    r1,#0        ; /* while (address != 0)   */   \
    strne  r2,[r1]      ;                                \
    /* mcr     p15, 0, r0, c7, c10, 4;  / * drain the write & fill buffers */    \
    /* CPWAIT  r0   */  ;                                \
    bne    Boot_loc##LableCnt     ;

#define  SetCSRfromNext(pNext, LableCnt)                 \
    sub    r0,pc,#Boot_loc##LableCnt - pNext + 4  ;      \
    .set Boot_loc##LableCnt, .                    ;      \
    ldr    r1,[r0],#4   ; /* get address            */   \
    ldr    r2,[r0],#4   ; /* get data               */   \
    str    r2,[r1]      ;

#define  SetCSRfromNextEntry                             \
    ldr    r1,[r0],#4   ; /* get address            */   \
    ldr    r2,[r0],#4   ; /* get data               */   \
    str    r2,[r1]      ;
#define  SetCSRfromNext(pNext, LableCnt)                 \
    sub    r0,pc,#Boot_loc##LableCnt - pNext + 4  ;      \
    .set Boot_loc##LableCnt, .                    ;      \
    ldr    r1,[r0],#4   ; /* get address            */   \
    ldr    r2,[r0],#4   ; /* get data               */   \
    str    r2,[r1]      ;

#define  SetCSRfromNextEntry                             \
    ldr    r1,[r0],#4   ; /* get address            */   \
    ldr    r2,[r0],#4   ; /* get data               */   \
    str    r2,[r1]      ;

// ------------------------------------------------------------------------
#define IF_PROCESOR_ZERO(reg)                                                                    \
        ldr     reg, =(HAL_80314_CSR_BASE + HAL_80314_CIU_BASE+TS_CIU_CFG);                 /* 0xFFFE3038      */  \
        ldr     reg, [reg, #0];                                                                  \
        ands    reg, reg, #TS_CIU_CFG_PID;                                                       \



#define SPIP_IF_PROCESOR_ONE(reg0, reg1, brto)                                                   \
        ldr     reg1, =(HAL_80314_CSR_BASE + HAL_80314_CIU_BASE+TS_CIU_CFG);                 /* 0xFFFE3038     */  \
        ldr     reg0, [reg1, #0];                                                                \
        ands    reg0, reg0, #TS_CIU_CFG_PID;                                                     \
        bne     brto;

#define SPIP_IF_PROCESOR_ZER0(reg0, reg1, brto)                                                  \
        ldr     reg1, =(HAL_80314_CSR_BASE + HAL_80314_CIU_BASE+TS_CIU_CFG);                 /* 0xFFFE3038     */  \
        ldr     reg0, [reg1, #0];                                                                \
        ands    reg0, reg0, #TS_CIU_CFG_PID;                                                     \
        beq     brto;


// Read Spd requirements:
// byte   - byte location in SPD device - left-shifted by 16: e.g byte1 = 0x10000
// r0 = address of I2C_Cntrl1
// r1 = address of I2C_Cntrl2
// r2 = address of I2C_Rd_Data
// r3 = will return data read from I2C Byte location
// r4 - clobbered in routine
// r5 - clobbered in routine
// r10 - device 0, 1, 2 for DDR_x slot reading
#define read_spd(byte) 			\
	ldr	r4, =byte												;\
	cmp r10, #I2C_DDR0_SPD										;\
	bne	1f														;\
	ldr r3, =I2C_DDR0_SPD | I2C_READ | I2C_DDR_SPD_DEVICE 		;\
	b	3f														;\
1:																;\
	cmp r10, #1													;\
	bne 2f														;\
	ldr	r3, =I2C_DDR1_SPD | I2C_READ | I2C_DDR_SPD_DEVICE		;\
	b	3f														;\
2:																;\
	ldr r3, =I2C_DDR2_SPD | I2C_READ | I2C_DDR_SPD_DEVICE 	    ;\
3:																;\
	orr r3, r3, r4												;\
	str r3, [r0]												;\
	ldr r3, =I2C_START											;\
	str	r3, [r1]												;\
	mov r3, #0x100												;\
4:																;\
	subs r3, r3, #1												;\
 	bne 4b														;\
5:																;\
	ldr  r4, [r1]												;\
	mov	 r5, #0x100												;\
	ands r3, r4, r5												;\
	bne 5b														;\
	mov	r5, #0x10000000											;\
	ands r3, r4, r5												;\
	mov	r3, #1													;\
	beq	i2c_fail												;\
	ldr r3, [r2]												;\

// ddr_installed - check if bank has memory installed
// device - 0 or 1 for DDR0 or DDR1 SPD Device
// r0 = address of I2C_Cntrl1
// r1 = address of I2C_Cntrl2
// r2 = address of I2C_Rd_Data
// r3 = will return 0 if device installed, 1 if no device installed
// r4 - clobbered in routine
// r5 - clobbered in routine
#define ddr_installed(device) 			\
	ldr	r3, =device												;\
	ldr	r4, =I2C_BYTE12											;\
	cmp	r3, #0													;\
	bne	1f														;\
	ldr r3, =I2C_DDR0_SPD | I2C_READ | I2C_DDR_SPD_DEVICE 		;\
	b	3f														;\
1:																;\
	cmp r3, #1													;\
	bne	2f														;\
	ldr r3, =I2C_DDR1_SPD | I2C_READ | I2C_DDR_SPD_DEVICE 	    ;\
	b	3f														;\
2:																;\
	ldr	r3, =I2C_DDR2_SPD | I2C_READ | I2C_DDR_SPD_DEVICE		;\
3:																;\
	orr r3, r3, r4												;\
	str r3, [r0]												;\
	ldr r3, =I2C_START											;\
	str	r3, [r1]												;\
	mov r3, #0x100												;\
4:																;\
	subs r3, r3, #1												;\
 	bne 4b														;\
5:																;\
	ldr  r4, [r1]												;\
	mov	 r5, #0x100												;\
	ands r3, r4, r5												;\
	bne 5b														;\
	mov	r5, #0x04000000  										;\
	ands r3, r4, r5												;\
	moveq r3, #0  												;\
	movne r3, #1												;\


// ------------------------------------------------------------------------
// MCU Register Values

// ------------------------------------------------------------------------
// This macro represents the initial startup code for the platform
    .macro _platform_setup1
	// This is where we wind up immediately after reset. At this point, we
	// are executing from the boot address (0x00000000), not the eventual
	// flash address. Do some basic setup using position independent code
	// then switch to real flash address
		.globl Boot_init_cpu_0
Boot_init_cpu_0:

        ldr     r0,=(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0

    // enable coprocessor access
        ldr r0, =0x2001              // CP13,CP0
        mcr     p15, 0, r0, c15, c1, 0

	// Drain write and fill buffer
        mcr p15, 0, r0, c7, c10, 4
        CPWAIT  r0

    // Adjust HLP parameters
    ldr r1, =HAL_80314_CSR_BASE + HAL_80314_HLP_BASE
    ///ldr r2, =0x034422c0
    //str     r2, [r1, #0x08]  // HLP B0_CTRL0
    //ldr r2, =0xd0012000
    //str     r2, [r1, #0x0C]  // HLP B0_CTRL1

    /* Enable another window into FLASH to allow single stepping
     * through the relocation code.
     */
    ldr r1, =0xFFFE3028
    ldr r2, [r1, #0]
    orr r2, r2, #0x23
    str r2, [r1, #0]
    ldr r2, [r1, #0]

    // Set CPU Clock Ratio to the MAX value.
    // The emulation board is running on low speed.
    //mov     r0, #0x09
    //mcr     p14, 0, r0, c6, c0, 0
    nop
    nop


		.globl Boot_init_cpu_0_00
Boot_init_cpu_0_00:

	// Enable the Icache
       mrc p15, 0, r0, c1, c0, 0
       orr r0, r0, #MMU_Control_I
       mcr p15, 0, r0, c1, c0, 0
       CPWAIT  r0

    // Move FLASH to board Map location
	// value to load into pc to jump to real runtime address
       ldr     r7, =Boot_FlashBase

    SetCSRfromTable(CSR_Table, 0)
//!!!!--------
    add    r0,pc,#CSR_TableFlash - Boot_locxx -4;
Boot_locxx:
    ldr    r1,[r0],#4                   // Get CIU_BAR1 Address
    ldr    r2,[r0],#4                   // Get Flash Base Address
    b      Boot_icache_boundary
//------     Control Space Register Initialization Values
		.ltorg
        .globl CSR_Table
//    TCSR_REG (((),       ())

CSR_Table:
    TCSR_INIT(HAL_80314_CIU_CFG_BAR, HAL_IOC80314_CSR_BASE + TS_CIU_CFG_BAR_EN)    // Move CSR base registes    TCSR_REG1(CIU, SF_BAR4,        (0x00000000))  // Set Bar4 to 0x00000
//    TCSR_REG1(CIU, SF_SIZES,       (0x6b00006b))  // Enable Bar0 and Bar4
//    TCSR_PAUSE(CSR_HAL_ADJUST)
//    TCSR_REG (HLP, B0_CTRL0,       (0x034422c0))  // Adjust Flash processing time make flash acesses faster
//    TCSR_REG (HLP, B0_CTRL1,       (0xd0012000))  // This is needed to prevent the JTAG units from timing out
    TCSR_PAUSE(CSR_TableFlash)
CSR_TableFlash:
    TCSR_REG (CIU,  SF_BAR1,      (HAL_IOC80314_FLASH_BASE))       // Move FLASH from 0 to board map value
CSR_TableStart:
    TCSR_REG (CIU,  SRAM_BAR,     (HAL_IOC80314_SRAM_BASE       + TS_CIU_CFG_BAR_EN))
    TCSR_REG (CIU,  SF_BAR2,      (IQ80315_PCI1_ADDR))
    TCSR_REG (CIU,  SF_BAR3,      (IQ80315_PCI2_ADDR))
    TCSR_REG (CIU,  SF_BAR4,      (SDRAM_WINDOW_BASE))

    TCSR_REG (SDRAM, ECC_STATUS,  (0x00000000))                                  //Zero to clear
//    TCSR_REG (SDRAM, REFRESH,     (0x0000061A))                                  //0x80 Extreamly conservative refress rate
//    TCSR_REG (SDRAM, BANK_CTRL,   (0x1A042248))     //Unregister               //0x3A052244
//    TCSR_REG (SDRAM, BANK_CTRL,   (0x3A042248))       //Registered               //0x3A052244
//    TCSR_REG (SDRAM, B0_ADDR,     (0x00100000))                                  //ENable and Begin at addr 0   0x00100000
//    TCSR_REG (SDRAM, B0_MASK,     (0x000FF000))                                  //0x000FE000  mask for 512Meg
//    TCSR_REG (SDRAM, B1_ADDR,     (0x00101000))                                  //ENable and add 512Meg   0x00102000   A35 = Bit 19
//    TCSR_REG (SDRAM, B1_MASK,     (0x000FF000))                                  //0x000FE000  mask for 512Meg
//    TCSR_REG (SDRAM, CNTRL,       (0x3454C000))                                  //0x1814C000Set up the cotrol reg, but do not start yet.
    TCSR_REG (HLP, B0_ADDR,       (IQ80315_FLASH_BASE))                     // PBI PCE0# Addr
    TCSR_REG (HLP, B0_MASK,       (IQ80315_FLASH_MASK     ))                     // 32 Meg
    TCSR_REG (HLP, B0_CTRL0,      (IQ80315_FLASH_CTRL0    ))                     // Write disabled
    TCSR_REG (HLP, B0_CTRL1,      (IQ80315_FLASH_CTRL1    ))                     //
    TCSR_REG (HLP, B1_ADDR,       (IQ80315_FPGA_BASE      ))                     // PBI PCE1# Addr
    TCSR_REG (HLP, B1_MASK,       (IQ80315_FPGA_MASK      ))                     //  Meg
    TCSR_REG (HLP, B1_CTRL0,      (IQ80315_FPGA_CTRL0     ))                     // Write enabled
    TCSR_REG (HLP, B1_CTRL1,      (IQ80315_FPGA_CTRL1     ))                     //
    TCSR_REG (HLP, B2_ADDR,       (IQ80315_RESERVED_BASE  ))                     // PBI PCE2# Addr
    TCSR_REG (HLP, B2_MASK,       (IQ80315_RESERVED_MASK  ))                     //  Meg
    TCSR_REG (HLP, B2_CTRL0,      (IQ80315_RESERVED_CTRL0 ))                     // Write enabled
    TCSR_REG (HLP, B2_CTRL1,      (IQ80315_RESERVED_CTRL1 ))                     //
    TCSR_REG (HLP, B3_ADDR,       (IQ80315_IOREG_BASE     ))                     // PBI PCE3# Addr
    TCSR_REG (HLP, B3_MASK,       (IQ80315_IOREG_MASK     ))                     //  Meg
    TCSR_REG (HLP, B3_CTRL0,      (IQ80315_IOREG_CTRL0    ))                     // Write enabled
    TCSR_REG (HLP, B3_CTRL1,      (IQ80315_IOREG_CTRL1    ))                     //

    // The next entry enables DDRAM.  This enable must wait 200,000 ns after reset to insure proper operation.
//    TCSR_REG1(SDRAM, CNTRL,       (TS_SD_CNTRL_ENABLE + 0x3454C000))             //Enable and 0x9010C000
    TCSR_REG (CIU, SF_SIZES,      (IQ80315_BAR1_SIZE + IQ80315_BAR2_SIZE + IQ80315_BAR3_SIZE + IQ80315_BAR4_SIZE))

    TCSR_PAUSE(CSR_WD_TIMER)                               // Needed for JTAG when read to non valide memory address can cause CIU bus hang.
    TCSR_REG (CIU, WDTIMER,       (0x0000FFFF))                                  //Set WD time to ????
    TCSR_REG (CIU, CFG,           (TS_CIU_CFG_WD_EN))                            //Enable Bus Watch Dog Timmer

     TCSR_PAUSE(CSR_TableEnd)

    TCSR_INIT(HAL_IOC80314_SRAM_BASE, HAL_IOC80314_SRAM_SIZE)


BOOT_TABLE_COPY_MMU_TABLE:
    TCSR_DMA( 1,       GCSR, 0x01000000)    // Control Status
    TCSR_DMA( 1, SRC_ADDR_M, 0x00000000)
    TCSR_DMA( 1, SRC_ADDR_L, mmu_table)
    TCSR_DMA( 1, DST_ADDR_M, 0x0       )
    TCSR_DMA( 1, DST_ADDR_L, 0x00000000)  // Base Address
    TCSR_DMA( 1,       TCR1, 0x00000400)  // HLP to SDRAM
    TCSR_DMA( 1,       TCR2, (0x0C000000 +(mmu_table_end - mmu_table)))  // MMU Table Size
    TCSR_DMA( 1,  ND_ADDR_M, 0x0       )  // Next Discriptor
    TCSR_DMA( 1,  ND_ADDR_L, 0x00000001)
    TCSR_DMA( 1,     ND_TCR, 0x0       )  //Transfer Control
    TCSR_DMA( 1,       GCSR, 0x20000000)  //
    TCSR_DMA( 1,       GCSR, 0xA0000000)  //
     TCSR_PAUSE(COPY_MMU_TABLE)


	.p2align 5
	   .globl Boot_icache_boundary
Boot_icache_boundary:
	// Here is where we switch from boot address (0x000000000) to the
	// actual flash runtime address. We align to cache boundary so we
    // execute from cache during the switchover. Cachelines are 8 words.
        str     r2,[r1] // set new address for the CIU BAR1
        ldr     r2,[r1] // ensure that write operation was completed
        mov     r0, r2
        nop
        mov     pc, r7
        nop                  // We should never reach this point. If we do,
        nop                  // display FF and loop forever.
    0:  b       0b


		.globl Boot_FlashBase
Boot_FlashBase:
#if defined (CYGPKG_HAL_ARM_XSCALE_IQ80315_BOARD_FLAVOR_IQ80315)
    // Processor one can skip
    //init PCI1 reg if system is not HOST
        ldr     r0, =HAL_IOC80314_PCI1_BASE + PCI_PCI_CSR
        ldr     r1, =PCI1_CSR_INIT
        str     r1, [r0]
        ldr     r0, =HAL_IOC80314_PCI1_BASE + PCI_PCI_MISC_CSR
        ldr     r1, [r0]
        ldr     r2, =PCI1_MISC_CSR_INIT
        orr     r1,r1,r2
        str     r1, [r0]
#endif
    //Fill in BAR1 Look Up Table values to falcitate with hlp accesses
        ldr     r0, =HAL_IOC80314_CIU_BASE + TS_CIU_SF_BAR1_LUT0 //Pointer to PBC BAR1 Lookup Table base
        ldr     r1, =0                                           //Lower address value
        eor     r2, r2, r2                                       //Upper address value
        mov     r3, #32                                          //Counter: Number of look up registers pairs
		.globl Boot_BAR1_Fill
Boot_BAR1_Fill:
        str     r1, [r0], #+4                                    //TS_CIU_SF_BAR1_LUT0
        str     r2, [r0], #+4                                    //TS_CIU_SF_BAR1_LUT_UPPER0
        add     r1, r1,   #0x800000
        subs    r3, r3,   #1
        bne     Boot_BAR1_Fill

    //Fill in BAR4 Look Up Table values for Memory Window
        ldr     r0, =HAL_IOC80314_CIU_BASE + TS_CIU_SF_BAR4_LUT0 //Pointer to PBC BAR4 Lookup Table base
        ldr     r1, =SDRAM_WINDOW_BASE + OCN_PORT_SDRAM          //Lower address value
        eor     r2, r2, r2                                       //Upper address value
        mov     r3, #32                                          //Counter: Number of look up registers pairs
		.globl Boot_BAR4_Fill
Boot_BAR4_Fill:
        str     r1, [r0], #+4                                    //TS_CIU_SF_BAR4_LUT0
        str     r2, [r0], #+4                                    //TS_CIU_SF_BAR4_LUT_UPPER0
        add     r1, r1,   #0x1000000							 //16M Pages
        subs    r3, r3,   #1
        bne     Boot_BAR4_Fill

    //Set control registers from table values
    SetCSRfromTable(CSR_TableStart, 1)

#ifdef USE_SDRAM
	// SPD Scan to get SDRAM Register values from DDR Device(s) (cebruns)
	// r0 = I2C_CNTRL1, r1 = I2C_CNTRL2, r2 = RD_DATA, r3/4/5/6 = working reg, r7 = CNRL, r8 = BANK_CNTRL
		ldr 	r0, =SDRAM_I2C_CNTRL1
		ldr 	r1, =SDRAM_I2C_CNTRL2
		ldr 	r2, =SDRAM_I2C_RD_DATA
		mov		r7, #0  // SD_CNTRL
		mov 	r8, #0	// SD_BANK_CNTRL
		mov		r9, #0  // Total SDRAM Size in MB

// Only DDR Supported for FL
		mov 	r3, #DDR_EN
		orr 	r7, r7, r3
// Change Port_ARB setting if needed - defaults to 00b
		ldr		r3, =(HAL_IOC80314_PCI1_BASE + PCI_PCI_CLASS)
		ldrb	r4, [r3]
		cmp		r4, #0x2					// Check if it's B1 (Class=0x2) stepping
		ldreq	r3, =SDRAM_PORT1_PRIORITY   // CIU Priority for B1 (better XScale performance)
		ldrne   r3, =SDRAM_PORT2_PRIORITY	// SFN Priority - needed for 80200 lockup workaround on B0
		orr		r7, r7, r3
// Set Precharge bit to A10
		ldr 	r3, =SDRAM_APB_10
		orr 	r7, r7, r3
// Set Bank Management - Banks left open until miss or refresh
		ldr 	r3, =SDRAM_BANKS_OPEN
		orr 	r8, r8, r3
// Set Twr = 2 clocks
		ldr r3, 	=SDRAM_TWR_CLOCKS_2
		orr 	r8, r8, r3
		b		check_dimms

		.ltorg
		.globl check_dimms
check_dimms:
		ddr_installed(0)  // Check if device in ddr0
		.globl check_dimm0
check_dimm0:
		cmp		r3, #0
		moveq 	r10, #0
		beq		10f
		ddr_installed(1)  // Check if device in ddr1
		.globl check_dimm1
check_dimm1:
		cmp		r3, #0
		moveq 	r10, #1
		beq 	10f
		ddr_installed(2)  // Check if device in ddr2
		.globl check_dimm2
check_dimm2:
		cmp 	r3, #0
		moveq 	r10, #2
		beq 	10f
		mov		r3, #22
		b		i2c_fail
	// Get rest of values from I2C
	// First get Refresh Value
	// r10 has which DDR Device to read - 0, 1 or 2
10: 
 	 	read_spd(I2C_BYTE12)
		.globl i2c_byte_rfr
i2c_byte_rfr:
		cmp		 r3, #SPD_RFR_15_62us
		bne		1f
		ldr 	r3, =REFRESH_FOR_15_62us
		b		refresh_done
1:
		cmp		r3, #SPD_RFR_3_9us
		bne		2f
		ldr 	r3, =REFRESH_FOR_3_9us
		b		refresh_done
2:
		cmp 	r3, #SPD_RFR_7_8us
		bne		3f
		ldr 	r3, =REFRESH_FOR_7_8us
		b		refresh_done
3:
		cmp 	r3, #SPD_RFR_31_3us
		mov 	r3, #2
		bne		i2c_fail
		ldr 	r3, =REFRESH_FOR_31_3us
refresh_done:
		ldr r4, =SD_REFRESH
		str r3, [r4]

// Now get CAS Latency
		read_spd(I2C_BYTE18)
		.globl i2c_byte_cas
i2c_byte_cas:
		ands	 r4, r3, #SPD_CAS_25
		bne		cas_2_5
		ands 	r4, r3, #SPD_CAS_15
		bne 	cas_1_5
		mov		r3, #3
		b		i2c_fail  // Neither Cas2.5 or Cas1.5 found - memory not supported
cas_2_5:
		orr		r7, r7, #SDRAM_CAS_25
		b		cas_done
cas_1_5:
		orr 	r7, r7, #SDRAM_CAS_15
cas_done:

// Now get Burst Length Value
		read_spd(I2C_BYTE16)
		.globl i2c_byte_burst
i2c_byte_burst:
		ands 	r4, r3, #SPD_BURST_LENGTH2
		bne		burst_length2
		mov		r3, #4
		b		i2c_fail
burst_length2:
		ldr		r3, =SDRAM_BURST_LENGTH2
		orr		r7, r7, r3

// Now get DQS_SZ (x4/x8/x16)
		read_spd(I2C_BYTE13)
		.globl i2c_byte_dqs
i2c_byte_dqs:
		ands 	r4, r3, #SPD_X4
		bne		spd_x4
		ands 	r4, r3, #SPD_X8
		bne		spd_x8
		ands 	r4, r3, #SPD_X16
		bne		spd_x16
		mov 	r3, #5
		b		i2c_fail
spd_x4:
		ldr		r3, =SDRAM_X4
		b		dqs_size_done
spd_x8:
spd_x16:
		ldr 	r3, =SDRAM_X8_DQM_EN
dqs_size_done:
		orr	r7, r7, r3

		mov r14, #0
// Now check if ECC module inserted
		read_spd(I2C_BYTE11)
		.globl i2c_byte_ecc
i2c_byte_ecc:
		ands 	r4, r3, #SPD_ECC
		movne	r14, #1
		beq		not_ecc_module

// Now get ECC_SZ (x4/x8/x16)
		read_spd(I2C_BYTE14)
		.globl i2c_byte_ecc_sz
i2c_byte_ecc_sz:
		ands 	r4, r3, #SPD_X4
		bne		ecc_x4
		ands 	r4, r3, #SPD_X8
		bne		ecc_x8
		ands 	r4, r3, #SPD_X16
		bne		ecc_x16
		mov		r3, #6
		b		i2c_fail
ecc_x4:
		mov 	r3, #ECC_X4
		orr 	r7, r7, r3
		b		ecc_size_done
ecc_x8:
ecc_x16:
		ldr	 	r3, =ECC_X8 | SDRAM_DQM_EN
		orr		r7, r7, r3
ecc_size_done:


not_ecc_module:
		// Get values for SD_BANK_CNTRL
		// First get Buff/Unbuff value
		read_spd(I2C_BYTE21)
		ands 	r4, r3, #SPD_BUFFERED
		beq		1f
		mov		r3, #7
		b		i2c_fail
1:
		ands 	r4, r3, #SPD_REGISTERED
		beq		2f
		mov		r3, #SDRAM_BUFFERED
		b		buff_done
2:
		mov		r3, #SDRAM_UNBUFFERED
buff_done:
		orr 	r8, r8, r3

		// Get NBANK - Phy banks
		read_spd(I2C_BYTE5)
		cmp r3, #SPD_PHY_BANKS1
		bne	1f
		mov r3, #SDRAM_PHY_BANKS1
		b	phy_bank_done
1:
		cmp	r3, #SPD_PHY_BANKS2
		mov	r3, #8
		bne	i2c_fail
		mov	r3, #SDRAM_PHY_BANKS2
phy_bank_done:
		orr r8, r8, r3

		// Get NBANK - Log banks
		read_spd(I2C_BYTE17)
		cmp r3, #SPD_LOG_BANKS2
		bne	1f
		mov r3, #SDRAM_LOG_BANKS2
		b	log_bank_done
1:
		cmp	r3, #SPD_LOG_BANKS4
		mov	r3, #9
		bne	i2c_fail
		mov	r3, #SDRAM_LOG_BANKS4
log_bank_done:
		orr r8, r8, r3

		// Get Num Column Addresses
		read_spd(I2C_BYTE4)
		cmp r3, #SPD_A_MODE0
		bne	1f
		mov r3, #SDRAM_A_MODE0
		b	a_mode_done
1:
		cmp	r3, #SPD_A_MODE1
		bne	2f
		mov	r3, #SDRAM_A_MODE1
		b	a_mode_done
2:
		cmp	r3, #SPD_A_MODE2
		bne	3f
		mov	r3, #SDRAM_A_MODE2
		b	a_mode_done
3:
		cmp	r3, #SPD_A_MODE3
		bne	4f
		mov	r3, #SDRAM_A_MODE3
		b	a_mode_done
4:
		cmp	r3, #SPD_A_MODE4
		bne	5f
		mov	r3, #SDRAM_A_MODE4
		b	a_mode_done
5:
		cmp	r3, #SPD_A_MODE5
		bne	6f
		mov	r3, #SDRAM_A_MODE5
		b	a_mode_done
6:
		cmp	r3, #SPD_A_MODE6
		mov r3, #10
		b	i2c_fail
		mov	r3, #SDRAM_A_MODE6
		b	a_mode_done
a_mode_done:
		orr r8, r8, r3

		// Get tRAS value
		read_spd(I2C_BYTE30)
		mov	r4, #1
		mov	r5, #10
tras_loop:
		cmp	r3, r5
		ble	tras_done
		add	r4, r4, #1
		add	r5, r5, #10
		cmp	r5, #160
		beq tras_fail
		b	tras_loop
tras_fail:
		mov	r3, #11
		b	i2c_fail
tras_done:
		mov r3, r4, lsl #16
		orr r8, r8, r3
		str	r8, [r2]

		// Get tRCD value
		read_spd(I2C_BYTE29)
		mov	r4, r3, asr #2
		and	r3, r4, #0xff
		mov	r4, #1
		mov	r5, #10
trcd_loop:
		cmp	r3, r5
		ble	trcd_done
		add	r4, r4, #1
		add	r5, r5, #10
		cmp	r5, #80
		beq trcd_fail
		b	trcd_loop
trcd_fail:
		mov	r3, #12
		b	i2c_fail
trcd_done:
		mov r3, r4, lsl #12
		orr	r8, r8, r3

		// Get tRP value
		read_spd(I2C_BYTE27)
		mov	r4, r3, asr #2
		and	r3, r4, #0xff
		mov	r4, #1
		mov	r5, #10
trp_loop:
		cmp	r3, r5
		ble	trp_done
		add	r4, r4, #1
		add	r5, r5, #10
		cmp	r5, #80
		beq trp_fail
		b	trp_loop
trp_fail:
		mov	r3, #13
		b	i2c_fail
trp_done:
		mov r3, r4, lsl #8
		orr r8, r8, r3

		// Get trfc
		read_spd(I2C_BYTE42)
		mov	r4, #1
		mov	r5, #10
		cmp	r3, #0
		bne	trfc_loop
		mov	r4, #8		// if no Trfc byte - use 8 clocks
		b	trfc_done
trfc_loop:
		cmp	r3, r5
		ble	trfc_done
		add	r4, r4, #1
		add	r5, r5, #10
		cmp	r5, #260
		beq trfc_fail
		b	trfc_loop
trfc_fail:
		mov	r3, #14
		b	i2c_fail
trfc_done:
		mov r3, r4
		orr	r8, r8, r3

// Store values found in the SPD - afterwards, R8 is running total of ByteCount
		ldr r3, =SD_BANK_CNTRL
		str r8, [r3]
		ldr	r3, =SD_CNTRL
		str	r7, [r3]

// Start to read density/bank sizes here
		// ====================================================================
	    HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_1
		// ====================================================================
// Read Density of Dimmx, where x is the first DIMM we encountered - could be 0,1,2	
// r8 will now contain total byte count in all DIMMS >> 20 (so it contains MB)
	.globl read_density_bankx
read_density_bankx:
// Calculate RAM Size to store in BANKx Addr
// First Get Density
		read_spd(I2C_BYTE31)
		mov		r6, #0
		mov		r8, #0
		cmp		r3, #SPD_DENSITY_2G
		moveq 	r6, #SDRAM_DENSITY_2G
		cmp		r3, #SPD_DENSITY_1G
		moveq 	r6, #SDRAM_DENSITY_1G
		cmp 	r3, #SPD_DENSITY_512M
		moveq	r6, #SDRAM_DENSITY_512M
		cmp		r3, #SPD_DENSITY_256M
		moveq	r6, #SDRAM_DENSITY_256M
		cmp		r3, #SPD_DENSITY_128M
		moveq	r6, #SDRAM_DENSITY_128M
		cmp		r3, #SPD_DENSITY_64M					  
		moveq	r6, #SDRAM_DENSITY_64M
		cmp		r6, #0
		moveq	r3, #15
		beq		i2c_fail


// Get Num Banks
		read_spd(I2C_BYTE5)
		mul		r9, r6, r3	  // Now r9 has total MB from this DIMM
		mov		r8, r9, ror #20   // r8 has total MB from DIMMx (x = first DIMM found - could be 0,1,2)

		cmp		r9, #SDRAM_DENSITY_2G
		ldreq	r3, =SDRAM_MASK_2G
		cmp		r9, #SDRAM_DENSITY_1G
		ldreq	r3, =SDRAM_MASK_1G
		cmp		r9, #SDRAM_DENSITY_512M
		ldreq	r3, =SDRAM_MASK_512M
		cmp		r9, #SDRAM_DENSITY_256M
		ldreq	r3, =SDRAM_MASK_256M
		cmp		r9, #SDRAM_DENSITY_128M
		ldreq	r3, =SDRAM_MASK_128M
		cmp		r9, #SDRAM_DENSITY_64M
		ldreq	r3, =SDRAM_MASK_64M

	
		cmp		r10, #1       // Are we DIMM1?
		beq		1f
		cmp		r10, #2       // Are we DIMM2?
		beq		2f
		ldr		r4, =SD_BANK0_MASK  // We must be DIMM0
		str		r3, [r4]
		mov		r3, #SDRAM_DIMM_ENABLE
		ldr		r4, =SD_BANK0_ADDR
		str		r3, [r4]
		b		3f
1:
		ldr		r4, =SD_BANK1_MASK
		str		r3, [r4]
		mov		r3, #SDRAM_DIMM_ENABLE
		ldr		r4, =SD_BANK1_ADDR
		str		r3, [r4]
		b		3f
2:
		ldr		r4, =SD_BANK2_MASK
		str		r3, [r4]
		mov		r3, #SDRAM_DIMM_ENABLE
		ldr		r4, =SD_BANK2_ADDR
		str		r3, [r4]
		b		3f
3:
		cmp		r10, #2  // Already scanning DDR_2 - so we're done
		moveq	r10, #0  // Clear Byte Sum
		beq		i2c_done
		cmp		r10, #1  // We're scanning DDR_1 - goto DDR_2
		beq		read_density_bank2
		mov		r10, #0  // Else we're scanning DDRO - go check DDR1

	.globl read_density_bank1
read_density_bank1:
	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_2
	// ====================================================================
// Calculate RAM Size to store in BANK1 Addr - if Applicable
// First Get Density
		mov		r10, #0
		ldr		r3, =I2C_DDR1_SPD | I2C_READ | I2C_DDR_SPD_DEVICE | I2C_BYTE31
		str 	r3, [r0]
		ldr 	r3, =I2C_START
		str		r3, [r1]
		mov 	r3, #0x100
1:
		subs 	r3, r3, #1
		bne 	1b
0:
		ldr  	r4, [r1]
		mov	 	r5, #0x100
		ands 	r3, r4, r5
		bne 	0b

		mov		r5, #0x10000000
		ands 	r3, r4, r5
		bne		get_dimm_size
		mov		r5, #0x04000000  	// timeout?
		ands 	r3, r4, r5
		bne	 	read_density_bank2	// Check DDR_2
		mov		r3, #16
		b		i2c_fail

get_dimm_size:
		ldr r3, [r2]
		mov		r6, #0
		cmp		r3, #SPD_DENSITY_2G
		moveq 	r6, #SDRAM_DENSITY_2G
		cmp		r3, #SPD_DENSITY_1G
		moveq 	r6, #SDRAM_DENSITY_1G
		cmp 	r3, #SPD_DENSITY_512M
		moveq	r6, #SDRAM_DENSITY_512M
		cmp		r3, #SPD_DENSITY_256M
		moveq	r6, #SDRAM_DENSITY_256M
		cmp		r3, #SPD_DENSITY_128M
		moveq	r6, #SDRAM_DENSITY_128M
		cmp		r3, #SPD_DENSITY_64M
		moveq	r6, #SDRAM_DENSITY_64M
		cmp		r6, #0
		moveq	r3, #17
		beq		i2c_fail

// Get Num Banks
		ldr 	r3, =I2C_DDR1_SPD | I2C_READ | I2C_DDR_SPD_DEVICE | I2C_BYTE5
		str 	r3, [r0]
		ldr 	r3, =I2C_START
		str		r3, [r1]
		mov 	r3, #0x100
1:
		subs 	r3, r3, #1
		bne 	1b
0:
		ldr  	r4, [r1]
		ldr	 	r5, =0x100
		ands 	r3, r4, r5
		bne 	0b

		ldr		r5, =0x10000000
		ands 	r3, r4, r5
		mov		r3, #18
		beq		i2c_fail

		ldr 	r3, [r2]
		mul		r10, r6, r3	  // Now r10 has total MB from this DIMM
		.globl ADDING_SECOND_DIMM_SIZE
ADDING_SECOND_DIMM_SIZE:
		add		r8, r8, r10, ror #20   // Now r8 has total MB from DIMM0 (we know its 0 now) + DIMM1

		cmp		r10, #SDRAM_DENSITY_2G
		ldreq	r3, =SDRAM_MASK_2G
		cmp		r10, #SDRAM_DENSITY_1G
		ldreq	r3, =SDRAM_MASK_1G
		cmp		r10, #SDRAM_DENSITY_512M
		ldreq	r3, =SDRAM_MASK_512M
		cmp		r10, #SDRAM_DENSITY_256M
		ldreq	r3, =SDRAM_MASK_256M
		cmp		r10, #SDRAM_DENSITY_128M
		ldreq	r3, =SDRAM_MASK_128M
		cmp		r10, #SDRAM_DENSITY_64M
		ldreq	r3, =SDRAM_MASK_64M

		ldr		r4, =SD_BANK1_MASK
		str		r3, [r4]

		ldr		r4, =SD_BANK0_ADDR   // find where to continue SDRAM
		ldr		r3, [r4]
		ldr		r4, =0xfffff
		and		r3, r3, r4
		mov		r4, r9, ror #16  // Now r4 has bytes in Dimm0 >> 16 (for 30 bit addressing in Bank_Addr)
		add		r3, r3, r4		 // Now r3 has address_dimm0 + bytes_dimm0
		orr		r3, r3, #SDRAM_DIMM_ENABLE
		ldr		r4, =SD_BANK1_ADDR
		str		r3, [r4]        // Emable DIMM1 at Dimm0+Dimm0 bytes
		//		add 	r9, r9, r10     // r9 has DIMM0 + DIMM1 bytes
		mov		r9, r10				// r9 = Total Dimm1


	.globl read_density_bank2
read_density_bank2:
	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_3
	// ====================================================================
// Calculate RAM Size to store in BANK2 Addr - if Applicable
// First Get Density
		mov		r10, #0
		ldr 	r3, =I2C_DDR2_SPD | I2C_READ | I2C_DDR_SPD_DEVICE | I2C_BYTE31
		str 	r3, [r0]
		ldr 	r3, =I2C_START
		str		r3, [r1]
		mov 	r3, #0x100
1:
		subs 	r3, r3, #1
		bne 	1b
0:
		ldr  	r4, [r1]
		mov	 	r5, #0x100
		ands 	r3, r4, r5
		bne 	0b

		mov		r5, #0x10000000
		ands 	r3, r4, r5
		bne		get_dimm_size2
		mov		r5, #0x04000000  // timeout?
		ands 	r3, r4, r5
		bne		i2c_done // No other DIMM installed
		mov		r3, #19
		b		i2c_fail

get_dimm_size2:
		ldr 	r3, [r2]
		mov		r6, #0
		cmp		r3, #SPD_DENSITY_2G
		moveq 	r6, #SDRAM_DENSITY_2G
		cmp		r3, #SPD_DENSITY_1G
		moveq 	r6, #SDRAM_DENSITY_1G
		cmp 	r3, #SPD_DENSITY_512M
		moveq	r6, #SDRAM_DENSITY_512M
		cmp		r3, #SPD_DENSITY_256M
		moveq	r6, #SDRAM_DENSITY_256M
		cmp		r3, #SPD_DENSITY_128M
		moveq	r6, #SDRAM_DENSITY_128M
		cmp		r3, #SPD_DENSITY_64M
		moveq	r6, #SDRAM_DENSITY_64M
		cmp		r6, #0
		moveq	r3, #20
		beq		i2c_fail

// Get Num Banks
		ldr 	r3, =I2C_DDR2_SPD | I2C_READ | I2C_DDR_SPD_DEVICE | I2C_BYTE5
		str 	r3, [r0]
		ldr 	r3, =I2C_START
		str		r3, [r1]
		mov 	r3, #0x100
1:
		subs 	r3, r3, #1
		bne 	1b
0:
		ldr  	r4, [r1]
		ldr	 	r5, =0x100
		ands 	r3, r4, r5
		bne 	0b

		ldr		r5, =0x10000000
		ands 	r3, r4, r5
		mov		r3, #21
		beq		i2c_fail

		ldr 	r3, [r2]
		mul		r10, r6, r3	  // Now r10 has total MB from this DIMM
		.globl Adding_Third_DIMM
Adding_Third_DIMM:
		add		r8, r8, r10, ror #20	  // r8 has total MB for D0+D1+D2

		cmp		r10, #SDRAM_DENSITY_2G
		ldreq	r3, =SDRAM_MASK_2G
		cmp		r10, #SDRAM_DENSITY_1G
		ldreq	r3, =SDRAM_MASK_1G
		cmp		r10, #SDRAM_DENSITY_512M
		ldreq	r3, =SDRAM_MASK_512M
		cmp		r10, #SDRAM_DENSITY_256M
		ldreq	r3, =SDRAM_MASK_256M
		cmp		r10, #SDRAM_DENSITY_128M
		ldreq	r3, =SDRAM_MASK_128M
		cmp		r10, #SDRAM_DENSITY_64M
		ldreq	r3, =SDRAM_MASK_64M

		ldr		r4, =SD_BANK2_MASK
		str		r3, [r4]
	
		ldr		r4, =SD_BANK1_ADDR  // find where to continue SDRAM
		ldr		r3, [r4]
		cmp		r3, #0				// No DIMM in Bank1
		ldreq	r4, =SD_BANK0_ADDR	// So check Bank0
		ldreq	r3, [r4]			// And figure where to continue
		ldr		r4, =0xfffff
		and		r3, r3, r4
		mov		r4, r9, ror #16  // Now r4 has bytes in Dimm1 >> 16 (for 30 bit addressing in Bank_Addr)
		add		r3, r3, r4		 // Now r3 has address_dimm1 + bytes_dimm1
		orr		r3, r3, #SDRAM_DIMM_ENABLE
		ldr		r4, =SD_BANK2_ADDR
		str		r3, [r4]        // Enable DIMM2 at Dimm1+Dimm0 bytes

		b		i2c_done       // Skip over table - we're done with SPD scan

	.ltorg
i2c_fail:
		b	5151f
5150:
		.byte	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3
		.byte	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7
		.byte	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B
		.byte	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
5151:
		ldr		r1, =5150b
		mov		r5, r3, asr #4  // r5 has left display offset
		and		r6, r3, #0xf	// r6 has right display offset
		add		r1, r1, r5		// r5 has left address
		ldrb 	r5, [r1]
		ldr		r1, =5150b
		add		r1, r1, r6		// r6 has right address
		ldrb 	r6, [r1]
display_loop:
		ldr		r3, =IQ80315_DISPLAY_LEFT_ADDR
		ldr		r4, =DISPLAY_1
		strb 	r4, [r3]
		ldr 	r3, =IQ80315_DISPLAY_RIGHT_ADDR
		ldr 	r4, =DISPLAY_C
		strb 	r4, [r3]
		ldr		r7, =0x00FFFFFF
loop2:
		subs 	r7, r7, #1
		bne		loop2
		ldr 	r7, =0x00FFFFFF
		ldr		r3, =IQ80315_DISPLAY_LEFT_ADDR
		strb	r5, [r3]
		ldr		r3, =IQ80315_DISPLAY_RIGHT_ADDR
		strb	r6, [r3]
loop3:
	 	subs 	r7, r7, #1
		bne 	loop3
		b		display_loop

		.globl i2c_done
i2c_done:
		ldr		r4, =SD_CNTRL
		ldr 	r7, [r4]

/* Here is the check for the Battery Backup Test -
   i.e. Should I send a "Wake" command to come out of BBU mode, or
        should I send an "Enable" command and forget all the SDRAM knows?
		Byte2 of the BootE2 is reserved for this (mainly because I couldn't
		think of a better place for it).  - cebruns
*/
// r0 = address of I2C_Cntrl1
// r1 = address of I2C_Cntrl2
// r2 = address of I2C_Rd_Data
// r3 = will return data read from I2C Byte location
// r5 - clobbered in routine
// r10 - device 0, 1, 2 for DDR_x slot reading
	.globl Check_E2PROM_BBU
Check_E2PROM_BBU:
	ldr 	r0, =I2C_CNTRL1
	ldr 	r1, =I2C_CNTRL2
	ldr 	r2, =I2C_RD_DATA
	ldr 	r3, =(I2C_READ | 0xA | (2 << 16))  // Setup I2C_Cntrl1 to read byte2
	str 	r3, [r0]											   
	ldr 	r3, =I2C_START										   
	str		r3, [r1]											   
	mov 	r3, #0x100											   
4:															   
	subs 	r3, r3, #1											   
 	bne 	4b													   
5:															   
	ldr  	r0, [r1]											   
	mov	 	r5, #0x100											   
	ands 	r3, r0, r5											   
	bne 	5b													   
	mov		r5, #0x10000000										   
	ands 	r3, r0, r5											   
	beq		no_bbu		// Some error reading the BootE2 - NO BBU Available
	ldr 	r3, [r2]   // r3 now contains byte2 of the BootE2 

	cmp 	r3, #0xff  // Uninit'd BootE2
	beq		no_bbu
	cmp		r3, #0x0   // No BBU
	beq 	no_bbu
	cmp 	r3, #0x1   // BBU Testing Enabled
	bne		no_bbu		   // BNE because unexpected value in location 2

// Bailout option to keep from making doorstops - if WAKE won't wake the SDRAM, we'll
// use the rotary switch to use the ENABLE Command
	ldr		r0, =IQ80315_ROTARY_SWITCH_ADDR
	ldrb	r1, [r0]
	and		r1, r1, #0x0f
	cmp		r1, #0x01
	// Rotary position 'E' will give us ENABLE
	orreq	r7, r7, #SDRAM_CNTRL_ENABLE 
	beq		2f
	// Else we'll go for the WAKE command
	orr		r7, r7, #SDRAM_CNTRL_WAKE  // We're in BBU mode - use a wake command
	orr		r14, r14, #0x2 // R14, bit 0 = ECC Enable, bit 1 = BBU Enabled
	b		2f
no_bbu:
	orr		r7, r7, #SDRAM_CNTRL_ENABLE
2:

		.globl store_control
store_control:
		str		r7, [r4]
#endif // USE_SDRAM


	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_4
	// ====================================================================

	// Set the TTB register
	ldr	r0, =mmu_table
	mcr	p15, 0, r0, c2, c0, 0

	// Enable permission checks in all domains
	ldr	r0, =0x55555555
	mcr	p15, 0, r0, c3, c0, 0

	// Enable the MMU
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_M
	orr	r0, r0, #MMU_Control_R
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
    mcr p15, 0, r0, c7, c10, 4      // drain the write & fill buffers
	CPWAIT r0

	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_5
	// ====================================================================
#ifdef ENABLE_PARITY
// SRAM MEMORY INITIALIZATION
// Turn on Parity  : initialize SRAM and then clear Parity errors
// PE occur because CIU does RMW and parity bit(s) have not been init'd, yet
		ldr 	r0, =(HAL_IOC80314_CIU_BASE+TS_CIU_CFG);
		ldr		r1, [r0]
		orr		r1, r1, #TS_CIU_CFG_PAR_EN
		str		r1, [r0]     // Parity now enabled
#endif

// Scrub memory - either through DMA or directly
#ifndef MEMORY_DMA_INIT
        mov     r0, #0
        mov     r1, #0
        mov     r2, #0
        mov     r3, #0
        mov     r4, #0
        mov     r5, #0
        mov     r6, #0
        mov     r7, #0

        ldr     r11, =HAL_IOC80314_SRAM_BASE
        ldr     r12, =HAL_IOC80314_SRAM_SIZE     // Size of SRAM to Initialize

		.globl Boot_Sram_Init
   Boot_Sram_Init:
        stmia   r11!, {r0-r7}
        subs    r12, r12, #32
        bne     Boot_Sram_Init
#else                                   // Use DMA init
        SetCSRfromTable(BOOT_TABLE_SRAM_FILL, 2)
#endif                                  // MEMORY_DMA_INIT  SRAM

// Need to clear parity errors
		ldr		r0, =(HAL_IOC80314_CIU_BASE+TS_CIU_ERROR)
		mov		r1, #TS_CIU_ERROR_PARITY
		str		r1, [r0]
//      Initiaize SRAM Completed

#ifdef  USE_SDRAM
#ifdef  ENABLE_ECC
		.globl	ECC_ENABLE
ECC_ENABLE:
        // ECC for 80200
		ands r0, r14, #1
		beq no_ecc
		// Enable ECC for XScale and IOC80314
		// Enable on IOC80314 for SDRAM Block
		ldr		r0, =(HAL_IOC80314_SDRAM_BASE+TS_SDRAM_ECC_STATUS)
		ldr		r1, [r0]
		orr		r1, r1, #TS_ECC_STATUS_ECC_EN
		str		r1, [r0]

		// Enable on IOC80314 for CIU->XScale DBus
        ldr     r0,  =(HAL_IOC80314_CIU_BASE+TS_CIU_CFG);
        ldr     r1,  [r0]
        orr     r1,  r1, #TS_CIU_CFG_ECC_EN
		str		r1,  [r0]

		// Enable on 80200
	   mcr   p15, 0, r0, c7, c10, 4 /* Drain write buffers                   */
wait_until_idle0:
	   mrc   p13, 0, r15, c0, c1, 0 /* Wait until the bus is idle 		     */
	   bmi   wait_until_idle0
	   mov   r0, #0xA				/* Enable ECC and Single bit correction  */
	   mcr   p13, 0, r0, c0, c1, 0
	   mrc   p15, 0, r0, c1, c0, 1  /* Enable ECC checking of the MMU tables */
	   orr   r0, r0, #2 
	   mcr   p15, 0, r0, c1, c0, 1
	   ldr   r3, =(0x80000000)      /* Set DWE bit in 80200 to disable RMW    */
	   mcr   p13, 0, r3, c8, c1, 0  /* 8 byte accesses to ECC protected space */
       mrc   p13, 0, r15, c0, c1, 0  // BCU_WAIT --> wait until the BCU isn't busy
       submi pc, pc, #0xc
       CPWAIT	r0
#endif // ENABLE_ECC
#endif // USE_SDRAM
no_ecc:

  //---------               Memory Initialization                                                      ---------//
  	b past_tables
BOOT_TABLE_SRAM_FILL:
    TCSR_DMA( 0,       GCSR, 0x01000000)  // Control Status
    TCSR_DMA( 0, SRC_ADDR_M, 0x00000000)
    TCSR_DMA( 0, SRC_ADDR_L, 0x00000000)
    TCSR_DMA( 0, DST_ADDR_M, 0x0       )
    TCSR_DMA( 0, DST_ADDR_L, 0x50000000)  // Base Address
    TCSR_DMA( 0,       TCR1, 0x00000300)
    TCSR_DMA( 0,       TCR2, 0x0C100000)  // 1Meg SRAM  256 birsts
    TCSR_DMA( 0,  ND_ADDR_M, 0x0       )  // Next Discriptor
    TCSR_DMA( 0,  ND_ADDR_L, 0x00000001)
    TCSR_DMA( 0,     ND_TCR, 0x0       )  //Transfer Control
    TCSR_DMA( 0,       GCSR, 0x20000000)  //
    TCSR_DMA( 0,       GCSR, 0xA0000000)  //
     TCSR_PAUSE(SRAM_FILL)

BOOT_TABLE_SDRAM_FILL:
BOOT_TABLE_SDRAM_FILL_RESET:
    TCSR_DMA( 1,       GCSR, 0x01000000)    // Control Status
    TCSR_DMA( 1, SRC_ADDR_M, 0x00000000)
    TCSR_DMA( 1, SRC_ADDR_L, 0x00000000)
    TCSR_DMA( 1, DST_ADDR_M, 0x0       )
    TCSR_DMA( 1, DST_ADDR_L, 0x00000000)  // Base Address
    TCSR_DMA( 1,       TCR1, 0x00040400)
BOOT_TABLE_SDRAM_FILL_CNT:
    TCSR_DMA( 1,       TCR2, 0x0C800000)  // 8Meg Chunks
    TCSR_DMA( 1,  ND_ADDR_M, 0x0       )  // Next Discriptor
    TCSR_DMA( 1,  ND_ADDR_L, 0x00000001)
    TCSR_DMA( 1,     ND_TCR, 0x0       )  //Transfer Control
    TCSR_DMA( 1,       GCSR, 0x20000000)  //
BOOT_TABLE_SDRAM_FILL_CTRL:
    TCSR_DMA( 1,       GCSR, 0xA0000000)  //
     TCSR_PAUSE(SDRAM_FILL)

past_tables:
	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_S, DISPLAY_L
	// ====================================================================

        ldr     r11, =SDRAM_UNCACHED_BASE
#ifdef  USE_SDRAM
#ifndef MEMORY_DMA_INIT
        ldr     r12, =0x200000     // size of SD - DDRAM  memory to Initialize   :: Assume SD/DDRAM size 512Meg for now.
        //  DDRAM Initialization Loop
        mov     r0, #0
        mov     r1, #0
        mov     r2, #0
        mov     r3, #0
        mov     r4, #0
        mov     r5, #0
        mov     r6, #0
        mov     r7, #0

		.globl Boot_DDRam_Init
   Boot_DDRam_Init:
        stmia   r11!, {r0-r7}
        subs    r12, r12, #32
        bne     Boot_DDRam_Init
#else            // Use DMA init
        SetCSRfromTable(BOOT_TABLE_SDRAM_FILL, 3)   // Set registers and fill first Block
        ldr     r2,(BOOT_TABLE_SDRAM_FILL_CTRL +4)  // GCSR go CMD
		ands	r6, r14, #2
		movne	r6, #0x10   // If BBU enabled - we'll clear 16Meg
		moveq 	r6, r8	    // R8 has dram_size in MB - load into r6 as counter
        ldr     r7, =DMA_CHN_ADDR(1)                // Channel pointer
        ldr     r9, (BOOT_TABLE_SDRAM_FILL_CNT +4)  // Get Block Size: Count
        ldr     r10,(BOOT_TABLE_SDRAM_FILL_RESET +4)// GCSR reset CMD
		.globl Boot_DDRam_Init_DMA
Boot_DDRam_Init_DMA:
        ldr     r0, [r7, #CH_GCSR]                 // Wait for done
        ands    r0, r0,  #GCSR_DONE
        beq     Boot_DDRam_Init_DMA
        subs    r6, r6,  #0x8                   // Count-= 8Meg chunks
        beq     Boot_DDRam_Init_DMA_Done

        str     r10,[r7, #CH_GCSR]                 //  Reset DMA controler
        str     r9, [r7, #CH_TCR2]                 //  Reset Count Register
        str     r2, [r7, #CH_GCSR]                 //  Restart DMA
        b       Boot_DDRam_Init_DMA
		.globl Boot_DDRam_Init_DMA_Done
Boot_DDRam_Init_DMA_Done:
#endif // MEMORY_DMA_INIT  DDRAM
#endif // USE_SDRAM

#ifdef DEBUG_OVERLAY_DDR_WITH_SRAM
        ldr     r0, =(HAL_IOC80314_CIU_BASE  + TS_CIU_SRAM_BAR)    //Pointer to SRAM BAR
        ldr     r1, =(SDRAM_PHYS_BASE        + TS_CIU_CFG_BAR_EN)  // value = DDRAM base
        str     r1, [r0]
#else	// No Overlay
		mov		r0, r0
#endif

#ifdef  USE_SDRAM
		.globl MEMORY_TEST
MEMORY_TEST:
// Now that Memory is Init'd - Do a MemTest on 16Meg of SDRAM - check for ECC failures/general RAM Failures
// - cebruns - Oct. '03
		ldr		r2, =0x1000000			// Test 16Meg
		ldr		r0,  =SDRAM_BASE_UnCUnB   // Pointer
5:
		mov		r1, r0					// Store address as data into mem locations
		str		r1, [r0], #4
		subs	r2, r2, #4
		bne		5b
		ldr		r2, =0x1000000			// Test 16Meg
		ldr		r0, =SDRAM_BASE_UnCUnB
1:
		ldr		r1, [r0]
		cmp		r1, r0
		bne		2f						// Fail
		add		r0, r0, #4
		subs	r2, r2, #4
		beq		3f						// Pass
		b		1b						// Pass, continue looping
2:
		.globl SET_BREAK_HERE
SET_BREAK_HERE:
		ldr		r0, =DISPLAY_LEFT
		ldr		r1, =DISPLAY_F
		strb	r1, [r0]
		ldr		r0, =DISPLAY_RIGHT
		strb	r1, [r0]
		mov		r1, #0x10000000
1:
		subs	r1, r1, #1
		bne		1b
		ands	r6, r14, #2
		blne	DISABLE_BBU

byebye:
		b		byebye			//No need to continue if we fail

		.globl DISABLE_BBU
DISABLE_BBU:
	ldr 	r0, =I2C_CNTRL1
	ldr 	r1, =I2C_CNTRL2
	ldr 	r2, =I2C_WR_DATA
	ldr 	r3, =(I2C_WRITE | 0xA | (2 << 16))  // Setup I2C_Cntrl1 to read byte2
	mov		r4, #0x0
	str		r4, [r2]   // Clear BBU Byte in EEPROM
	str 	r3, [r0]											   
	ldr 	r3, =I2C_START										   
	str		r3, [r1]											   
	mov 	r3, #0x100											   
4:															   
	subs 	r3, r3, #1											   
 	bne 	4b													   
5:															   
	ldr  	r0, [r1]											   
	mov	 	r5, #0x100											   
	ands 	r3, r0, r5											   
	bne 	5b													   
	mov		pc, lr  // Done writing - just return


3:
// Check for SBE, MBE, or Parity Err
		.globl CHECK_ERRORS
CHECK_ERRORS:
		ldr		r0, =(HAL_IOC80314_CIU_BASE+TS_CIU_ERROR)
		ldr		r1, [r0]
		mov		r2, #(TS_CIU_ERROR_PARITY | TS_CIU_ERROR_UECC | TS_CIU_ERROR_CECC)
		ands	r0, r1, r2
		bne		ECC_FAILURE
		ldr		r0, =SD_ECC_STATUS
		ldr		r1, [r0]
		ldr		r2, =(SD_ECC_ERROR_MASK+SD_ECC_UERR)
		ands	r0, r1, r2
		bne		ECC_FAILURE
		b		END_MEM_TEST

		.globl ECC_FAILURE
ECC_FAILURE:
		ldr		r0, =DISPLAY_LEFT
		ldr		r1, =DISPLAY_E
		strb	r1, [r0]
		ldr		r1, =DISPLAY_C
		ldr		r0, =DISPLAY_RIGHT
		strb	r1, [r0]
		mov		r1, #0x10000000
1:
		subs	r1, r1, #1
		bne		1b
		b		ECC_FAILURE	// No need to continue if we fail


		.globl END_MEM_TEST  // Need to zero-out our test area
END_MEM_TEST:
		ldr		r2, =0x1000000			// Reset 16Meg
		ldr		r0,  =SDRAM_BASE_UnCUnB   // Pointer
5:
		mov		r1, #0
		str		r1, [r0], #4
		subs	r2, r2, #4
		bne		5b

#endif // USE_SDRAM

		.globl DONE_MEM_TEST
DONE_MEM_TEST:
	// Enable the Dcache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_C
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

// Copy Vectors Code to SDRAM
// Copy first 4K of flash into first 4K of RAM.
        ldr r1, =SDRAM_BASE // base address of SDRAM
        ldr     r2, =HAL_IOC80314_FLASH_BASE
        mov     r3, #0x1000
		.globl boot_copyVectorCode
boot_copyVectorCode:
        ldr     r4, [r2], #4
        str     r4, [r1], #4
        subs    r3, r3, #4
        bne     boot_copyVectorCode
//avb-end Copy Vectors Code to SDRAM
								   
    // clean, drain, flush the main Dcache
        ldr     r1, =DCACHE_FLUSH_BASE  // use a CACHEABLE area of
                                        // memory that's mapped above SDRAM
        mov     r0, #1024               // number of lines in the Dcache
0:
        mcr     p15, 0, r1, c7, c2, 5   // allocate a Dcache line
        add     r1, r1, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

	// ====================================================================
	HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_8
	// ====================================================================

	// clean, drain, flush the mini Dcache
        ldr     r2, =MINI_DCACHE_FLUSH_BASE
        mov     r0, #64                 // number of lines in the Dcache
    0:
        mcr     p15, 0, r2, c7, c2, 5   // allocate a Dcache line
        add     r2, r2, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

        mcr     p15, 0, r0, c7, c6, 0   // flush Dcache
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_0, DISPLAY_9
	// ====================================================================
	// Save SDRAM size from SPD Search - if we're OVERLAYED with SRAM, only report 1Meg
	// If SDRAM installed is > 512Meg, only report 512Meg
	// OSes/Apps can read SDRAM registers to see how much memory is installed
	// and use a 'windowing' technique to access memory above 512Meg thru the SFN
    ldr     r1, =hal_dram_size  /* [see hal_intr.h] */
	.globl	FIXUP_R8
FIXUP_R8:
	movs	r8, r8, lsl #20		// Put r8 back into total byte mode (instead of MB)
	ldrcs	r8, =0x20000000    // If r8 overflowed (for >=4Gig memory installed - it to 512Meg since RedBoot only uses 512

#ifdef DEBUG_OVERLAY_DDR_WITH_SRAM
    ldr     r8, =0x100000
#else
	cmp		r8, #0x20000000
	bls		store_size
	ldr		r8, =0x20000000
store_size:
#endif
	str	r8, [r1]

	// Move mmu tables into RAM so page table walks by the cpu
	// don't interfere with FLASH programming.
    // !!!! Change this to use SRAM once assured Hardware works
	.globl MMU_COPY_TO_SDRAM
MMU_COPY_TO_SDRAM:
	ldr	r0, =mmu_table
    add r2, r0, #0x4000     // End of tables
    mov r1, #SDRAM_BASE_UnCUnB
    orr r1,r1, #0x4000     // RAM tables

	// everything can go as-is
    1:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	cmp	r0, r2
	bne	1b

	// ====================================================================
    HEX_DISPLAY r3, r4, DISPLAY_1, DISPLAY_0
	// ====================================================================

        // clean, drain, flush the main Dcache
        ldr     r1, =DCACHE_FLUSH_BASE  // use a CACHEABLE area of memory
        mov     r0, #1024               // number of lines in the Dcache
    0:
        mcr     p15, 0, r1, c7, c2, 5   // allocate a Dcache line
        add     r1, r1, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

	// clean, drain, flush the mini Dcache
        ldr     r2, =MINI_DCACHE_FLUSH_BASE
        mov     r0, #64                 // number of lines in the Dcache
    0:
        mcr     p15, 0, r2, c7, c2, 5   // allocate a Dcache line
        add     r2, r2, #32             // increment to the next cache line
        subs    r0, r0, #1              // decrement the loop count
        bne     0b

        mcr     p15, 0, r0, c7, c6, 0   // flush Dcache
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

	// ====================================================================
      HEX_DISPLAY r3, r4, DISPLAY_1, DISPLAY_1
	// ====================================================================

	// Set the TTB register to DRAM mmu_table
        ldr     r0, =(SDRAM_PHYS_BASE | 0x4000) // RAM tables
        mov     r1, #0
        mcr     p15, 0, r1, c7, c5, 0       // flush I cache
        mcr     p15, 0, r1, c7, c10, 4      // drain WB
        mcr     p15, 0, r0, c2, c0, 0       // load page table pointer
        mcr     p15, 0, r1, c8, c7, 0       // flush TLBs
        CPWAIT  r0
            .globl Boot_Finish_Platform_Setup
Boot_Finish_Platform_Setup:

// Set CCLKCFG Multiplier to 9 (733MHz)
		mov		r0, #9
		mcr		p14, 0, r0, c6, c0, 0

	// ====================================================================
	HEX_DISPLAY r3, r4, DISPLAY_A, DISPLAY_1
	// ====================================================================

			.globl Boot_Platform_Complete
Boot_Platform_Complete:
	.endm    // _platform_setup1

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  hal_pcsr_cfg_retry
hal_pcsr_cfg_retry:   .long   0  // Boot-time value of PCSR Retry bit.
        .endm

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
