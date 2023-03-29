#ifndef CYGONCE_HAL_ARM_XSCALE_IQ8134X_IQ8134X_H
#define CYGONCE_HAL_ARM_XSCALE_IQ8134X_IQ8134X_H

/*=============================================================================
//
//      iq8134x.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Contributors: drew.moseley@intel.com
// Date:         2004-12-10
// Purpose:      Intel IQ8134 CRB platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/iq8134x.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_iq8134x.h>
#include <cyg/hal/hal_iop34x.h>  // IO Processor defines

// ------------------------------------------------------------------------
// SDRAM configuration
#define SDRAM_MAX_MB                     2048
#define SDRAM_MAX                        (SDRAM_MAX_MB << 20)
#define SDRAM_MAX_UNCACHED_MB            512
#define SDRAM_MAX_UNCACHED               (SDRAM_MAX_UNCACHED_MB << 20)
#define SDRAM_BASE                       0x00000000
#define SDRAM_PHYS_BASE                  CYGNUM_HAL_ARM_XSCALE_IOP_SDRAM_PHYS_BASE

#define SRAM_BASE 						 0xffe00000
#define SRAM_SIZE                        SZ_1M
// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_UNCACHED_BASE              0xa0000000
#define SDRAM_32BIT_ECC_BASE             0xd0000000
#define DCACHE_FLUSH_AREA                0xc0000000
#define CREATE_L2_RAM                    1
#define L2_RAM_AREA	                     (DCACHE_FLUSH_AREA + SZ_32K)
#define L2_RAM_SIZE                      (SZ_256K)
// Address used for battery backup test
#define SDRAM_BATTERY_TEST_ADDR  (SDRAM_BASE + 0x100000)

// ------------------------------------------------------------------------
// FLASH configuration
/* Flash Max is based on Product Version.  MC board has 32Meg where all others will have 8 Meg */
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
#define FLASH_MAX                        ((((*IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK) == IQ8134X_PROD_CODE_IQ8134xMC) || \
								((*IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK) == IQ8134X_PROD_CODE_IQ8134xMC_MODE2))? \
                                          (SZ_32M) : CYGNUM_HAL_ARM_XSCALE_IOP_FLASH_SIZE)
#else
#define FLASH_MAX                        SZ_8M
#endif
#define FLASH_MAX_MB                     (FLASH_MAX >> 20)
												  
// ------------------------------------------------------------------------
// PBI configuration
// PCE0 is a CDL option.  PCE1 is 32MB above PCE0
#define FLASH_DEFAULT_PHYS_BASE           0x0
#define PCE0_PHYS_BASE                    CYGNUM_HAL_ARM_XSCALE_IOP_FLASH_PHYS_BASE
#define PCE1_PHYS_BASE                    (PCE0_PHYS_BASE + 0x02000000)

#define IQ8134X_FLASH_WIDTH               ((*PBIU_PBAR0 & 0x1) == 0x1) ? 16 : 8
#define IQ8134X_FLASH_ADDR                (PCE0_PHYS_BASE)
#define IQ8134X_PRODUCT_CODE_ADDR         (PCE1_PHYS_BASE + 0x00000)
#define IQ8134X_BOARD_STEPPING_ADDR       (PCE1_PHYS_BASE + 0x10000)
#define IQ8134X_CPLD_FIRMWARE_REV_ADDR    (PCE1_PHYS_BASE + 0x20000)
#define IQ8134X_DISCRETE_LED_ADDR         (PCE1_PHYS_BASE + 0x30000)
#define IQ8134X_DISPLAY_LEFT_ADDR         (PCE1_PHYS_BASE + 0x40000)
#define IQ8134X_DISPLAY_RIGHT_ADDR        (PCE1_PHYS_BASE + 0x50000)
#define IQ8134X_BUZZER_CONTROL_ADDR       (PCE1_PHYS_BASE + 0x60000)
#define IQ8134X_32k_NVRAM_ADDR            (PCE1_PHYS_BASE + 0x70000)
#define IQ8134X_256k_NVRAM_ADDR           (PCE1_PHYS_BASE + 0x80000)
#define IQ8134X_ROTARY_SWITCH_ADDR        (PCE1_PHYS_BASE + 0xd0000)
#define IQ8134X_BATTERY_STATUS_ADDR       (PCE1_PHYS_BASE + 0xf0000)
#define IQ8134X_COMPACT_FLASH_ADDR        (PCE1_PHYS_BASE + 0x01000000)
#define MCM2_ETHERNET_BASE		  (PCE1_PHYS_BASE + 0xa0000)
#define MCM2_ETHERNET_IRQ		  CYGNUM_HAL_INTERRUPT_HPI
// ------------------------------------------------------------------------
// I2C configuration
// I2C slave address to which the unit responds when in slave-receive mode
#define I2C_DEVID      0x02
#define SDRAM_DEVID    0xA0

// Timeout limit for SDRAM EEPROM to respond
#define I2C_TIMOUT	0x1000000

// ------------------------------------------------------------------------
// CPLD Registers
// BAT_STAT
#define IQ8134X_BATTERY_STATUS ((volatile unsigned char *)IQ8134X_BATTERY_STATUS_ADDR)
#define IQ8134X_BATTERY_PRESENT                  0x01
#define IQ8134X_BATTERY_FULLY_CHARGED            0x02
#define IQ8134X_BATTERY_FULLY_DISCHARGED         0x04
#define IQ8134X_BATTERY_ENABLE                   0x08

// PROD_CODE
#define IQ8134X_PRODUCT_CODE ((volatile unsigned char *)IQ8134X_PRODUCT_CODE_ADDR)
#define IQ8134X_PROD_SKU_MASK                    0x0F
#define IQ8134X_PROD_CODE_MASK                   0xF0
#define IQ8134X_PROD_CODE_IQ8134xSC              0x01
#define IQ8134X_PROD_CODE_IQ8134xBC              0x02
#define IQ8134X_PROD_CODE_IQ8134xRC              0x03
#define IQ8134X_PROD_CODE_IQ8134xMC              0x04
#define IQ8134X_PROD_CODE_IQ8134xAC              0x05
#define IQ8134X_PROD_CODE_IQ8134xMC_MODE2        0x08

// MACH TYPE For Linux Boot
#define IQ8134X_MACH_TYPE_SC                     799
#define IQ8134X_MACH_TYPE_MC                     801

// BOARD_STEPPING
#define IQ8134X_BOARD_STEPPING ((volatile unsigned char *)IQ8134X_BOARD_STEPPING_ADDR)
#define IQ8134X_BOARD_REWORK_LEVEL_MASK          0x0F
#define IQ8134X_BOARD_STEPPING_MASK              0xF0

// CPLD_FW
#define IQ8134X_CPLD_FIRMWARE_REV ((volatile unsigned char *)IQ8134X_CPLD_FIRMWARE_REV_ADDR)
#define IQ8134X_CPLD_CODE_REVISON_MASK           0xFF

// BUZZER_CTRL
#define IQ8134X_BUZZER_CONTROL ((volatile unsigned char *)IQ8134X_BUZZER_CONTROL_ADDR)
#define IQ8134X_BUZZER_ON                        0x01

// DISCRETE_LED
#define IQ8134X_DISCRETE_LED ((volatile unsigned char *)IQ8134X_DISCRETE_LED_ADDR)
#define IQ8134X_DISCRETE_LED_MASK                0xFF

// ROT_SW
#define IQ8134X_ROTARY_SWITCH ((volatile unsigned char *)IQ8134X_ROTARY_SWITCH_ADDR)
#define IQ8134X_ROTARY_SWITCH_MASK               0x0F

// ------------------------------------------------------------------------
// Rotary Switch Bit Settings
#define IQ8134X_ROTARY_SWITCH_DEV_HIDING         0x1
#define IQ8134X_ROTARY_SWITCH_PCI_PRIVMEM        0x2

// ------------------------------------------------------------------------
// 7 Segment Display
#define DISPLAY_LEFT  IQ8134X_DISPLAY_LEFT_ADDR
#define DISPLAY_RIGHT IQ8134X_DISPLAY_RIGHT_ADDR

//
// The 7-segment displays are controlled by segments corresponding to the
// following diagram:
//
//             a
//            ---
//          f|   |b
//           | g |
//            ---
//          e|   |c
//           |   |
//            ---
//             d
//
#define DISPLAY_SEGMENT_POINT 0x01
#define DISPLAY_SEGMENT_A     0x02
#define DISPLAY_SEGMENT_B     0x04
#define DISPLAY_SEGMENT_C     0x08
#define DISPLAY_SEGMENT_D     0x10
#define DISPLAY_SEGMENT_E     0x20
#define DISPLAY_SEGMENT_F     0x40
#define DISPLAY_SEGMENT_G     0x80
#define DISPLAY_0  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | \
                    DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_F)
#define DISPLAY_1  (DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C)
#define DISPLAY_2  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_G | \
                    DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_D)
#define DISPLAY_3  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_G | \
                    DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D)
#define DISPLAY_4  (DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G | DISPLAY_SEGMENT_B | \
                    DISPLAY_SEGMENT_C)
#define DISPLAY_5  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G | \
                    DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D)
#define DISPLAY_6  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_E | \
                    DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_G)
#define DISPLAY_7  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_B | \
                    DISPLAY_SEGMENT_C)
#define DISPLAY_8  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | \
                    DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_F | \
                    DISPLAY_SEGMENT_G)
#define DISPLAY_9  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | \
                    DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G)
#define DISPLAY_A  (DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_A | \
                    DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_G)
#define DISPLAY_B  (DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_D | \
                    DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_G)
#define DISPLAY_C  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_E | \
                    DISPLAY_SEGMENT_D)
#define DISPLAY_D  (DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D | \
                    DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_G)
#define DISPLAY_E  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G | \
                    DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_D)
#define DISPLAY_F  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G | \
                    DISPLAY_SEGMENT_E)
#define DISPLAY_I  (DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_E)
#define DISPLAY_P  (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_E | \
		    DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G)
#define DISPLAY_PERIOD      DISPLAY_SEGMENT_POINT
#define DISPLAY_S  DISPLAY_5
#define DISPLAY_L  (DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_D)
#define DISPLAY_OFF 0x00
#define DISPLAY_ON  0xFF


#ifdef __ASSEMBLER__
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
        // Display 'lvalue:rvalue' on the hex display
        // lvalue and rvalue must be of the form 'DISPLAY_x'
        // where 'x' is a hex digit from 0-F.
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue	
	ldr	\reg0, =DISPLAY_LEFT
	ldr	\reg1, =\lvalue
	strb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	ldr	\reg1, =\rvalue
	strb	\reg1, [\reg0]
	.endm
#else
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue
	.endm
#endif

#define HEX_DISPLAY_NO_MMU HEX_DISPLAY

#else // __ASSEMBLER__
int atue_safe(void);
int atux_safe(void);
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
#define HEX_DISPLAY_QUICK(l, r) {                    \
    *(unsigned char *)DISPLAY_LEFT = DISPLAY_ ## l;  \
    *(unsigned char *)DISPLAY_RIGHT = DISPLAY_ ## r; \
}

static inline void HEX_DISPLAY(int lval, int rval)
{
    int i;
    static unsigned char hchars[] = {
	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3,
	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B,
	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
    };
    volatile unsigned char *ldisp = (volatile unsigned char *)DISPLAY_LEFT;
    volatile unsigned char *rdisp = (volatile unsigned char *)DISPLAY_RIGHT;

    *ldisp = hchars[lval & 0xf];
    *rdisp = hchars[rval & 0xf];

    for (i = 0; i < 0x4000000; i++);
}
#else  /* Not using PCE1 */
#define HEX_DISPLAY(lval, rval)
#define HEX_DISPLAY_QUICK(l,r)
#endif

#endif // __ASSEMBLER__

#ifndef __ASSEMBLER__
//
// Low-level platform-setup time debug serial output routines
//
#ifdef CYGSEM_HAL_ARM_IQ8134X_SETUP_DEBUG

extern void hal_platform_lowlevel_serial_debug_init(unsigned long baud);
extern void hal_platform_lowlevel_serial_debug_putc(char c);
extern void hal_platform_lowlevel_serial_debug_puts(char *s);
extern void hal_platform_lowlevel_serial_debug_putn32(cyg_uint32 n);
#define hal_platform_lowlevel_serial_debug_put_value32(v) { \
    hal_platform_lowlevel_serial_debug_puts(#v);            \
    hal_platform_lowlevel_serial_debug_puts(" = ");         \
    hal_platform_lowlevel_serial_debug_putn32(v);           \
    hal_platform_lowlevel_serial_debug_puts("\r\n");        \
}
extern void hal_platform_lowlevel_serial_debug_putn16(cyg_uint16 n);
#define hal_platform_lowlevel_serial_debug_put_value16(v) { \
    hal_platform_lowlevel_serial_debug_puts(#v);            \
    hal_platform_lowlevel_serial_debug_puts(" = ");         \
    hal_platform_lowlevel_serial_debug_putn16(v);           \
    hal_platform_lowlevel_serial_debug_puts("\r\n");        \
}
extern void hal_platform_lowlevel_serial_debug_putn8(cyg_uint8 n);
#define hal_platform_lowlevel_serial_debug_put_value8(v) {  \
    hal_platform_lowlevel_serial_debug_puts(#v);            \
    hal_platform_lowlevel_serial_debug_puts(" = ");         \
    hal_platform_lowlevel_serial_debug_putn8(v);            \
    hal_platform_lowlevel_serial_debug_puts("\r\n");        \
}

#else

#define hal_platform_lowlevel_serial_debug_init(baud)
#define hal_platform_lowlevel_serial_debug_putc(c)
#define hal_platform_lowlevel_serial_debug_puts(s)
#define hal_platform_lowlevel_serial_debug_putn32(n)
#define hal_platform_lowlevel_serial_debug_put_value32(v)
#define hal_platform_lowlevel_serial_debug_putn16(n)
#define hal_platform_lowlevel_serial_debug_put_value16(v)
#define hal_platform_lowlevel_serial_debug_putn8(n)
#define hal_platform_lowlevel_serial_debug_put_value8(v)

#endif // CYGSEM_HAL_ARM_IQ8134X_SETUP_DEBUG
#endif // __ASSEMBLER__


#if defined (CYG_HAL_STARTUP_ROMRAM)
#define GET_FAC_SEMAPHORES()                                               	\
CYG_MACRO_START															   	\
	volatile cyg_uint8  *fac0_p = IMU_TSR_FAC0;								\
	volatile cyg_uint8  *fac1_p = IMU_TSR_FAC1;								\
	volatile cyg_uint8  fac0, fac1;				  							\
	cyg_uint32 coreid;                                                      \
	/* Determine which core we will be waiting on to release the FACx  */   \
	CIDR_READ(coreid);                                                      \
	if (!coreid)                                                            \
		coreid = 2;                                                         \
	/* Try to get semaphores - could add timeout here */ 				   	\
	do {																   	\
		fac0 = *fac0_p;    													\
		fac1 = *fac1_p;													   	\
	} while ((fac0 == coreid) || (fac1 == coreid));                         \
CYG_MACRO_END															   

#define RELEASE_FAC_SEMAPHORES()                                            \
CYG_MACRO_START															    \
	volatile cyg_uint8  *fac0_p = IMU_TSR_FAC0;								\
	volatile cyg_uint8  *fac1_p = IMU_TSR_FAC1;								\
	*fac0_p = 0;                                                            \
	*fac1_p = 0;            											    \
CYG_MACRO_END															   

#else
#define GET_FAC_SEMAPHORES()
#define RELEASE_FAC_SEMAPHORES()
#endif

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_IQ8134X_IQ8134X_H
// EOF iq8134x.h
