#ifndef CYGONCE_HAL_PLATFORM_EXTRAS_H
#define CYGONCE_HAL_PLATFORM_EXTRAS_H

/*=============================================================================
//
//      hal_platform_extras.h
//
//      Platform specific MMU table.
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
// Contributors: msalter, dkranak, cebruns
// Date:         2001-12-03
// Purpose:      Intel XScale IQ80315 platform specific mmu table
// Description:
// Usage:        #include <cyg/hal/hal_platform_extras.h>
//     Only used by "vectors.S"
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#ifdef PKG_HAL_HAS_MULT_CPU
#if !defined(CYG_HAL_STARTUP_ROM)
#define  mmu_table              0x40034000
#define _MMU_TABLE_SDRAM_WBRWA  0x000
#define _MMU_TABLE_SDRAM_UNCUNB 0x600 * 4
#endif

/*----------                CPU 1 Starup Code                      ----------*/
        .text
        .code   32
        .globl  Boot_init_cpu_1

Boot_init_cpu_1:
        mov     r0,  #(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0
        mov     r0,  #0x2000                    // enable coprocessor access  CP13,CP0
        orr     r0,  r0, #01
        mcr     p15, 0, r0, c15, c1, 0
        mcr     p15, 0, r0, c7, c10, 4          // Drain write and fill buffer
        CPWAIT  r0
        mrc     p15, 0, r0, c1, c0, 0           // Enable the Icache
        orr     r0,  r0, #MMU_Control_I
        mcr     p15, 0, r0, c1, c0, 0
        CPWAIT  r0

		// Set the TTB register to PT in Flash for now so ECC can be enabled
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

    // If CPU ECC enabled
        ldr     r7,  =(HAL_IOC80314_CIU_BASE+TS_CIU_CFG);                 /* 0x50103038      */
        ldr     r8,  [r7]
        ands    r8,  r8, #TS_CIU_CFG_ECC_EN
        beq     Boot_init_cpu_1_01

		// Enable on 80200
	   mcr   p15, 0, r0, c7, c10, 4 /* Drain write buffers                   */
wait_until_idle1:
	   mrc   p13, 0, r15, c0, c1, 0 /* Wait until the bus is idle 		     */
	   bmi   wait_until_idle1
	   mov   r0, #0xA				/* Enable ECC and Single bit correction  */
	   mcr   p13, 0, r0, c0, c1, 0
	   mrc   p15, 0, r0, c1, c0, 1  /* Enable ECC checking of the MMU tables */
	   orr   r0, r0, #2 
	   mcr   p15, 0, r0, c1, c0, 1
	   ldr   r3, =(0x80000000)      /* Set DWE bit in 80200 to disable RMW    */
	   mcr   p13, 0, r3, c8, c1, 0  /* 8 byte accesses to ECC protected space */
       mrc   p13, 0, r0, c0, c1, 0  // BCU_WAIT --> wait until the BCU isn't busy
       submi pc, pc, #0xc
       CPWAIT	r0

            .globl Boot_init_cpu_1_01       // Make sure interrupts disabled and enable ICache
Boot_init_cpu_1_01:
        // Copy first 4K of flash into first 4K of RAM. This includes the vecotor code
        ldr     r1, =NUM_HAL_CPU_1_BASE_PHY_ADDR // base address of SDRAM for CPU 1
        ldr     r2, =HAL_IOC80314_FLASH_BASE
        mov     r3, #0x1000
            .globl Boot_init_cpu_1_02
Boot_init_cpu_1_02:
        ldr     r4, [r2], #4
        str     r4, [r1], #4
        subs    r3, r3, #4
        bne     Boot_init_cpu_1_02

    // Move mmu tables into RAM so page table walks by the cpu
    // don't interfere with FLASH programming.
    // !!!! Change this to use SRAM once assured Hardware works
        ldr     r0, =mmu_table
        add     r2, r0, #0x4000     // End of tables
        ldr     r1, =(NUM_HAL_CPU_1_BASE_PHY_ADDR + 0x4000)

            .globl Boot_init_cpu_1_03
Boot_init_cpu_1_03:
        ldr     r3, [r0], #4
        str     r3, [r1], #4
        cmp     r0, r2
        bne     Boot_init_cpu_1_03

		// Copy Data section from CPU0
		ldr		r0, =0x8000
		ldr		r1, =(NUM_HAL_CPU_1_BASE_PHY_ADDR + 0x8000)
		ldr		r2, =(NUM_HAL_CPU_1_VAR_SIZE - 0x8000)
1:
		ldr		r3, [r0], #4
		str		r3, [r1], #4
		subs	r2, r2, #4
		bne		1b

		// Copy HAL_DRAM_SIZE from CPU0
		ldr		r0, =hal_dram_size
		ldr		r1, =(NUM_HAL_CPU_1_BASE_PHY_ADDR)
		add		r1, r0, r1
		ldr		r2, [r0]
		str		r2, [r1]

    //  Adjust the MMU table for privet space for each processor
        ldr     r1, =(NUM_HAL_CPU_1_BASE_PHY_ADDR + 0x4000)   	// PT entries
        ldr     r2, =(NUM_HAL_CPU_1_BASE_PHY_ADDR + 0x4000)		// PT entries
        mov     r3, r2
        ldr     r5, =_MMU_TABLE_SDRAM_UNCUNB					// UNCUNB Entry in Flash
        ldr     r6, =_MMU_TABLE_SDRAM_WBRWA						// CACHED Entry in Flash
        sub     r5, r5, r6										// Offset to UNCUNB entry
        add     r2, r2, r5										// Offset to UNCUNB entry in RAM
        ldr     r4, =(NUM_HAL_CPU_1_VAR_SIZE>>20)     			// Size of private space
        mov     r0, r4,LSL #2                              		// Word offset for privet space entry
            .globl Boot_init_cpu_1_04
Boot_init_cpu_1_04:
        ldr     r5, [r1]										//1st PT Entry of CPU1 (CPU0's BASE address Entry)
        ldr     r6, [r1, r0]									//CPU1 BASE Address PT Entry
        str     r5, [r1, r0]									//Store CPU0's Private space entry to CPU1's BASE_PHY_ADDRESS
        str     r6, [r1], #4									//Store CPU1's Private space entry to CPU1's 1st entry
        ldr     r5, [r2]										//UNCUNB Entry for CPU0
        ldr     r6, [r2, r0]									//UNCUNB Entry for CPU1
        str     r5, [r2, r0]									//Store CPU0's Private UNCUNB entry to CPU1's Base_phy_address
        str     r6, [r2], #4									//Store CPU1's Private UNCUNB entry to CPU1's UNCUNB entry
        subs    r4, r4, #1
        bne     Boot_init_cpu_1_04

        // Enable branch target buffer
        mrc p15, 0, r0, c1, c0, 0
        orr r0, r0, #MMU_Control_BTB
        mcr p15, 0, r0, c1, c0, 0
        CPWAIT  r0

        // Set the TTB register
        ldr     r0, =(NUM_HAL_CPU_1_BASE_PHY_ADDR | 0x4000) // CPU 1 MMU table
        mcr     p15, 0, r0, c2, c0, 0
        // Enable permission checks in all domains
        ldr     r0, =0x55555555
        mcr     p15, 0, r0, c3, c0, 0
        // Enable the MMU
        mrc     p15, 0, r0, c1, c0, 0
        orr     r0, r0, #MMU_Control_M
        orr     r0, r0, #MMU_Control_R
        mcr     p15, 0, r0, c1, c0, 0
        CPWAIT  r0

		// Invalidate All
        mcr    p15,0,r1,c8,c7,0;  /* flush I+D TLBs */             

// Set CCLKCFG Multiplier to 9 (733MHz)
		mov		r0, #9
		mcr		p14, 0, r0, c6, c0, 0

       .globl Boot_init_cpu_1_05
Boot_init_cpu_1_05:
        b      warm_reset
    // end of CPU 1 Init
#endif                                  // PKG_HAL_HAS_MULT_CPU


#if defined(CYG_HAL_STARTUP_ROM)
        .section .mmu_tables, "a"

    mmu_table:
//  This page table sets up the preferred mapping:
//
//    Virtual Addr  Physical Addr  Size(MB)  OCN Base Addr          Port  Description         Cache   D AP
//    ------------  -------------  --------  ------.----.----.----  ----  ------------------- ------- - --
// 1  0x00000000   0x00000000       512                                   SD/DDRAM            WB  RWA 0 RWE
// 2  0x20000000   0x20000000       512+                                  SD/DDRAM  window    WB  RWA 0 RWE
// 3  0x40000000   0x40000000        16                                   FLASH - PBI         WT  RA  1 RE
// 3a 0x41000000   0x41000000       240                                   Peripherals - PBI   UnC UnB 1 RE
// 4  0x50000000   0x50000000         1                                   SRAM                WB  RWA 0 RWE
// 5  0x50100000   0x50100000        64KB                                 80314 Ctrl Reg      UnC UnB 2 RW
// 6  0x50110000   0x50110000       960KB                                 Part of Ctrl Reg
// 6a 0x50200000   0x50000000         1                                   SRAM      alias     UnC UnB 0 RW
// 7  0x50300000   0x50300000        13                                   No Access           UnC UnB 4 NoA
// 8  0x51000000   0x51000000        16                                   Data Cache Flush    WT  RA  3 R
// 8a 0x52000000   0x52000000        16                                   Mini-Data Cache     WB  RWA 0 R
// 8b 0x53000000   0x53000000        16                                   Mini-Data Flush     WB  RWA 0 R
// 9  0x54000000   0x54000000       194                                   No Access           UnC UnB 4 NoA
//10  0x60000000   0x00000000       512                                   SD/DDRAM alias      UnC UnB 0 RWE
//    0x60000000   0x00000000      removed to support >1G memories        -----------------   WT  RA  5 RWE
//11  0x80000000   0x80000000       495      0x0000.0000.0000.0000        PCI1 MEM32          UnC UnB 6 RWE
//12  0x9EFF0000   0x9EFF0000         1      0x0000.0000.1EFF.0000        PCI1 I/O            UnC UnB 7 RWE
//13  0x9F000000   0x9F000000        16      0x0000.0001.0000.0000        PCI1 CFG            UnC UnB 8 RWE
//14  0xA0000000   0xA0000000       256      0x0000.0000.4000.0000        PCI1 PFM1           UnC UnB 9 RWE
//15  0xB0000000   0xB0000000       256      0x0000.0000.8000.0000        PCI1 PFM2           UnC UnB A RWE
//16  0xC0000000   0xC0000000       495      0x0000.0000.0000.0000        PCI2 MEM32          UnC UnB B RWE
//17  0xDEFF0000   0xDEFF0000         1      0x0000.0000.1EFF.0000        PCI2 I/O            UnC UnB C RWE
//18  0xDF000000   0xDF000000        16      0x0000.0001.0000.0000        PCI2 CFG            UnC UnB D RWE
//19  0xE0000000   0xE0000000       256      0x0000.0000.4000.0000        PCI2 PFM1           UnC UnB E RWE
//20  0xF0000000   0xF0000000       256      0x0000.0000.8000.0000        PCI2 PFM2           UnC UnB F RWE


//----     512MB VA=0x0000.0000 PhyA=0x0000.0000   ECC DRAM  XCB=111 WB RWA
// 1  0x00000000   0x00000000       512                                   SD/DDRAM            WB  RWA 0 RWE
    .globl _MMU_TABLE_SDRAM_WBRWA
_MMU_TABLE_SDRAM_WBRWA:
    .set    __PhyAddr, 0x000
	.rept	0x200 - 0x000
   //First Level       Phy Addr, x, ap, p, d, c, b
 //   FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0, 0, 0
 //   FL_SECTION_ENTRY   __PhyAddr, 1,  3, 1, 0, 1, 1
    FL_SECTION_ENTRY   __PhyAddr, 1,  3, 1, 0, 1, 1
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     SD/DDRAM  Memory Window       512MB VA=0x2000.0000 PhyA=0x2000.0000   ECC DRAM  XCB=111 WB RWA
// 2  0x20000000   0x20000000       512                                   SD/DDRAM  window    WB  RWA 0 RWE
    .set    __PhyAddr, 0x200
    .rept   0x400 - 0x200
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 1,  3, 1, 0, 1, 1
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     FLASH                          16MB VA=0x4000.0000 PhyA=0x4000.0000             XCB=000   C UnB
// 3  0x40000000   0x40000000        16                                   FLASH               UnC  UnB  1 RE
    .globl _MMU_TABLE_FLASH
_MMU_TABLE_FLASH:
    .set    __PhyAddr, 0x400
    .rept   0x410 - 0x400
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 1, 1, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PBI - Peripherals             240MB VA=0x4100.0000 PhyA=0x4100.0000             XCB=000 UnC UnB
// 3a 0x41000000   0x41000000       240                                   PBI Peripherals     UnC  UnB  1 RE
    .globl _MMU_TABLE_PBI_PERIPH
_MMU_TABLE_PBI_PERIPH:
    .set    __PhyAddr, 0x410
    .rept   0x500 - 0x410
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 1, 0, 0		
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     SRAM                           1 MB VA=0x5000.0000 PhyA=0x5000.0000 Parity SRAM  XCB=111 WB RA
// 4  0x50000000   0x50000000         1                                   SRAM                WB  RWA 0 RWE
    .globl _MMU_TABLE_SRAM
_MMU_TABLE_SRAM:
    .set    __PhyAddr, 0x500
    .rept   0x501 - 0x500
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 1,  3, 1, 0, 1, 1
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     Ctrl Reg                       1 MB VA=0x5010.0000 PhyA=0x5010.0000             XCB=000 WB RWA
// 5  0x50100000   0x50100000        64KB                                 80314 Ctrl Reg      UnC UnB 2 RW
// 6  0x50110000   0x50110000       960KB                                 Part of Ctrl Reg   (!!!should add Second Level Table)
    .set    __PhyAddr, 0x501
    .rept   0x502 - 0x501
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 2, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     SRAM    alias                  1 MB VA=0x5000.0000 PhyA=0x5000.0000 Parity SRAM  XCB=000 UnC UnB
// 6a 0x50200000   0x50000000         1                                   SRAM                UnC UnB 0
    .set    __PhyAddr, 0x500
    .rept   0x503 - 0x502
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr


//----     No Acess                      253MB VA=0x0000.0000 PhyA=0x0000.0000   Blank     XCB=000 RnC UnB
// 7  0x50600000   0x50600000        13                                   No Access           UnC UnB 4 NoA
    .set    __PhyAddr, 0x503
    .rept   0x510 - 0x503
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  0, 0, 4, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     Data  Cache Flus               1 MB VA=0x5020.0000 PhyA=0x5020.0000             XCB=010 WB RWA
// 8  0x51000000   0x51000000        16                                   Data Cache Flush    WT  RA  3 R
    .globl _MMU_TABLE_CACHE_FLUSH
_MMU_TABLE_CACHE_FLUSH:
    .set    __PhyAddr, 0x510
    .rept   0x520 - 0x510
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 0, 3, 1, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     Mini-Data Cache                1 MB VA=0x5030.0000 PhyA=0x0000.0000   ECC DRAM  XCB=110 WB RWA
// 8a 0x52000000   0x52000000        16                                   Mini-Data Cache     WB  RWA 0 R
    .set    __PhyAddr, 0x000
    .rept   0x530 - 0x520
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 1,  3, 1, 0, 1, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     Mini-Data Cache Flush          1 MB VA=0x5040.0000 PhyA=0x5040.0000   ECC DRAM  XCB=110 WB RWA
// 8b 0x53000000   0x53000000        16                                   Mini-Data Fulsh     WB  RWA 0 R
    .globl _MMU_TABLE_MINI_CACHE_FLUSH
_MMU_TABLE_MINI_CACHE_FLUSH:
    .set    __PhyAddr, 0x530
    .rept   0x540 - 0x530
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 1,  3, 0, 0, 1, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     No Acess                      253MB VA=0x0000.0000 PhyA=0x0000.0000   Blank     XCB=000 RnC UnB
// 9  0x50600000   0x50600000       194                                   No Access           UnC UnB 4 NoA
    .set    __PhyAddr, 0x540
    .rept   0x600 - 0x540
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  0, 0, 4, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     SD/DDRAM   alias              512MB VA=0x0000.0000 PhyA=0x0000.0000   ECC DRAM  XCB=010 Wt RA
//10  0x60000000   0x00000000       512                                   SD/DDRAM  alias     WT  RA  5 RWE
//    .globl _MMU_TABLE_SDRAM_WTRA
//_MMU_TABLE_SDRAM_WTRA:
//    .set    __PhyAddr, 0x000
//    .rept   0x800 - 0x600
    //First Level       Phy Addr, x, ap, p, d, c, b
//    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 0, 5, 1, 0
//    .set    __PhyAddr,__PhyAddr+1
//	.endr

//----     SD/DDRAM   UnC UnB           512MB VA=0x0000.0000 PhyA=0x0000.0000   ECC DRAM  XCB=000 UnC UnB
//10  0x60000000   0x00000000       512                                   SD/DDRAM  alias     WT  RA  5 RWE
    .globl _MMU_TABLE_SDRAM_UNCUNB
_MMU_TABLE_SDRAM_UNCUNB:
    .set    __PhyAddr, 0x000
    .rept   0x800 - 0x600
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 5, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr


//----     PCI1 MEM32                    495MB VA=0x0000.0000 PhyA=0x0000.0000    PCI 1    XCB=000 UnC UnB
//11  0x80000000   0x80000000       495      0x0000.0000.0000.0000        PCI1 MEM32          UnC UnB 6 RWE
    .globl _MMU_TABLE_PCI1
_MMU_TABLE_PCI1:
    .set    __PhyAddr, 0x800
    .rept   0x9EF - 0x800
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 6, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI1 I/O                       1 MB VA=0x0000.0000 PhyA=0x0000.0000   PCI 1  XCB=000 UnC UnB
//    This is a 64 K region which using only first level discripters overlaps  pci1 MEM32 space
//12  0x9EFF0000   0x9EFF0000         1      0x0000.0000.1EFF.0000        PCI1 I/O            UnC UnB 7 RWE
    .set    __PhyAddr, 0x9EF
    .rept   0x9F0 - 0x9EF
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 7, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 1 Config                   16MB VA=0x9F00.0000 PhyA=0x9F00.0000   PCI 1     XCB=000 UnC UnB
//13  0x9F000000   0x9F000000         16      0x0000.0001.0000.0000        PCI1 CFG            UnC UnB 8 RWE
    .set    __PhyAddr, 0x9F0
    .rept   0xA00 - 0x9F0
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 8, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 1 PFM1                    256MB VA=0xA000.0000 PhyA=0xA000.0000   PCI 1     XCB=000 UnC UnB
//14  0xA0000000   0xA0000000       256      0x0000.0000.4000.0000        PCI1 PFM1           UnC UnB 9 RWE
    .set    __PhyAddr, 0xA00
    .rept   0xB00 - 0xA00
    //First Level       Phy Addr, x, ap, p, d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 9, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 1  PFM2                   256MB VA=0xB000.0000 PhyA=0xB000.0000   PCI 1     XCB=000 UnC UnB
//15  0xB0000000   0xB0000000       256      0x0000.0000.8000.0000        PCI1 PFM2           UnC UnB A RWE
    .set    __PhyAddr, 0xB00
    .rept   0xC00 - 0xB00
    //First Level       Phy Addr, x, ap, p,   d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0xA, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 2  MEM32                  495MB VA=0xC000.0000 PhyA=0xC000.0000   PCI 2     XCB=000 UnC UnB
//16  0xC0000000   0xC0000000       495      0x0000.0000.0000.0000        PCI2 MEM32          UnC UnB B RWE
    .globl _MMU_TABLE_PCI2
_MMU_TABLE_PCI2:
    .set    __PhyAddr, 0xC00
    .rept   0xDEF - 0xC00
    //First Level       Phy Addr, x, ap, p,   d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0xB, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 2  I/O                     1 MB VA=0xDEF0.0000 PhyA=0xDEF0.0000   PCI 2     XCB=000 UnC UnB
//    This is a 64 K region which using only first level discripters overlaps  pci2 MEM32 space
//17  0xDEFF0000   0xDEFF0000         1      0x0000.0000.1EFF.0000        PCI2 I/O            UnC UnB C RWE
    .set    __PhyAddr, 0xDEF
    .rept   0xDF0 - 0xDEF
    //First Level       Phy Addr, x, ap, p,   d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0xC, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 2  Config                  16MB VA=0xDF00.0000 PhyA=0xDF00.0000   PCI 2     XCB=000 UnC UnB
//18  0xDF000000   0xDF000000        16      0x0000.0001.0000.0000        PCI2 CFG            UnC UnB D RWE
    .set    __PhyAddr, 0xDF0
    .rept   0xE00 - 0xDF0
    //First Level       Phy Addr, x, ap, p,   d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0xD, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 2  PFM1                   256MB VA=0xE000.0000 PhyA=0xF000.0000   PCI 2     XCB=000 UnC UnB
//19  0xE0000000   0xE0000000       256      0x0000.0000.4000.0000        PCI2 PFM1           UnC UnB E RWE
    .set    __PhyAddr, 0xE00
    .rept   0xF00 - 0xE00
    //First Level       Phy Addr, x, ap, p,   d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0xE, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

//----     PCI 2  PFM2                   256MB VA=0xF000.0000 PhyA=0xF000.0000   PCI 2     XCB=000 UnC UnB
//20  0xF0000000   0xF0000000       256      0x0000.0000.8000.0000        PCI2 PFM2           UnC UnB F RWE
    .set    __PhyAddr, 0xF00
    .rept   0x1000 - 0xF00
    //First Level       Phy Addr, x, ap, p,   d, c, b
    FL_SECTION_ENTRY   __PhyAddr, 0,  3, 1, 0xF, 0, 0
    .set    __PhyAddr,__PhyAddr+1
	.endr

mmu_table_end:

#endif /* CYG_HAL_STARTUP_ROM */

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
