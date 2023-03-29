/*=============================================================================
//
//      iq8033x_setup_mcu.c
//
//      Platform specific MCU Initialization for HAL (C code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// Date:         2003-08-04
// Purpose:      Intel XScale iq80331/iq80332 CRB platform specific MCU initialization
// Description: 
//     This file contains low-level MCU initialization code called by
//     hal_platform_setup.h.
//     Since we may be using "alternative" memory (ie different stack and heap than
//     we were compiled for), we cannot use any global data or C Library calls.
// Usage:        
//     Only called by "hal_platform_setup.c"
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/iq8033x.h>             // Platform specific hardware definitions
#include <cyg/hal/hal_iop33x.h> // Variant specific hardware definitions
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_spd.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_diag.h>

//
// Delay for some time.
// These delays are used for the SDRAM initialization sequence below.
// They are manually tweaked using a DDR-II analyzer.  If the core
// speed changes, these may need to be tweaked however they have
// been calibrated with some margin.
//
#define SDRAM_INIT_DELAY(w)                          \
{                                                    \
    register unsigned long _val_ = (w) >> 1;         \
    if (_val_)  {                                    \
       asm volatile (                                \
           "0:  subs %0,%0,#1  \n"                   \
           "    bne  0b        \n"                   \
           "1:                 \n" : "+r" (_val_) ); \
    }                                                \
}


//
// Convert a divisor in ns to a count of MCLKs by
// "dividing" by the ns_per_cycle.
// Using a local divide algorithm so we don't have to
// call into the C library
//
static unsigned ns_to_cycles(unsigned divisor, unsigned dividend)
{
    unsigned rc = 0;

    if (divisor >= dividend)
    {
        do {
            divisor -= dividend;
            rc++;
        } while (divisor >= dividend);
    }

    // Since we need to convert to clocks, make sure we have one more if there
    // is a remainder so that we will delay at least "divisor" ns
    if (divisor)
        rc++;

    return rc;
}


//
// Throw a Memory Parameter Error
//
#define MEM_PARAM_ERROR(m, n) {HEX_DISPLAY_QUICK(m, n); while (1);}


// Wait for a DCAL operation to complete
static void wait_for_dcal(void) {
	int timeout = 100000;
	unsigned int dcalcsr_value;
	do { 
		HAL_READ_UINT32(MCU_DCALCSR, dcalcsr_value);
		// wait for DCAL operation to complete or timeout
		timeout--;
	} while ((dcalcsr_value & DCALCSR_OPERATION_IN_PROGRESS) && \
			(timeout > 0));

	if (((dcalcsr_value & DCALCSR_PASS_FAIL_MASK) != 0) || \
	     (timeout == 0)) {
		MEM_PARAM_ERROR(D, F);
	} 
}

#define SPD_NUM_BYTES 64
//
// Perform MCU Initialization
//
void mcu_initialization(cyg_uint32 *sdram_size,
                        cyg_bool *sdram_installed,
                        cyg_bool *ecc_cap)
{
    cyg_uint32 ddr_type, num_banks, num_rows, num_cols;
    cyg_uint32 refresh = 0, sdram_width = 0, tCAS = 0;
    cyg_uint32 ns_per_cycle, tRP, tRCD, tRAS, tRC, tWTR, tRFC, tREG, tWR, tRTW, tEDP, tRTCMD;
    cyg_uint32 bank_size_Mbytes = 0, addr_tx, sbr_base;
    cyg_uint32 sdcr0_val = 0, sdcr1_val = 0, sbr0_val = 0, sbr1_val = 0, rcvdly_val = 0;
    cyg_uint16 t;
    cyg_uint8 i2c_bytes[SPD_NUM_BYTES], cksum = 0;
    int i;
    char buf;

    HEX_DISPLAY_QUICK(0, 6);
    hal_platform_lowlevel_serial_debug_puts("In mcu_initialization()\r\n");

    i2c_reset();
    i2c_init();

    // Write 0 to the SDRAM SPD to reset the EEPROM pointer
    buf = 0;
    if (i2c_write_bytes(SDRAM_DEVID, &buf, 1) == -1) {
        // Timeout Assume no device in system
#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        *sdram_size = HAL_DCACHE_LOCKABLE_SIZE;
        *sdram_installed = false;
        return;
#else
        MEM_PARAM_ERROR(0, 7);
#endif
    }

    // Read the bytes
    if (i2c_read_bytes(SDRAM_DEVID, i2c_bytes, SPD_NUM_BYTES) == -1) {
        // Timeout Assume no device in system
#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        *sdram_size = HAL_DCACHE_LOCKABLE_SIZE;
        *sdram_installed = false;
        return;
#else
        // hit the leds if an error occurred
        MEM_PARAM_ERROR(0, 8);
#endif
    }

    // Verify the checksum
    for (i = 0; i < SPD_NUM_BYTES; i++)
        if (i != SPD_CHECKSUM)
            cksum += i2c_bytes[i];
    if (cksum != i2c_bytes[SPD_CHECKSUM])
        MEM_PARAM_ERROR(0, 9);

    // Verify/Determine the DDR type
    ddr_type = i2c_bytes[SPD_MEMTYPE];
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR)
        ns_per_cycle = 6;
    else if (ddr_type == SPD_MEMTYPE_SDRAM_DDRII)
        ns_per_cycle = 5;
    else
        MEM_PARAM_ERROR(0, A);

    // Verify the number of row addresses
    num_rows = i2c_bytes[SPD_NUM_ROWS];
    if ((num_rows < SBR_ROW_ADDRESS_MIN) ||
        (num_rows > SBR_ROW_ADDRESS_MAX))
        MEM_PARAM_ERROR(0, B);

    // Verify the number of column addresses
    num_cols = i2c_bytes[SPD_NUM_COLS];
    if ((num_cols < SBR_COL_ADDRESS_MIN) ||
        (num_cols > SBR_COL_ADDRESS_MAX))
        MEM_PARAM_ERROR(0, C);

    // Verify/Determine the number of banks
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR)
        num_banks = i2c_bytes[SPD_BANKCNT];
    else
        // DDR-II
        num_banks = (i2c_bytes[SPD_BANKCNT] & SPD_DDRII_BANKCNT_MASK) + 1;
    if (num_banks > 2)
        MEM_PARAM_ERROR(0, D);

    // Verify/Determine the data bus width
    if (i2c_bytes[SPD_MOD_WIDTH1] == 32)
        // 32-bit data
        sdcr0_val |= SDCR_BUS_WIDTH_32_BITS;
    else if (i2c_bytes[SPD_MOD_WIDTH1] == 40)
        // 32-bit data + 8-bit ECC
        sdcr0_val |= SDCR_BUS_WIDTH_32_BITS;
    else if (i2c_bytes[SPD_MOD_WIDTH1] == 64)
        // 64-bit data
        sdcr0_val |= SDCR_BUS_WIDTH_64_BITS;
    else if (i2c_bytes[SPD_MOD_WIDTH1] == 72)
        // 64-bit data + 8-bit ECC
        sdcr0_val |= SDCR_BUS_WIDTH_64_BITS;
    else 
        MEM_PARAM_ERROR(0, E);

    if (i2c_bytes[SPD_MOD_WIDTH2] != 0)
        MEM_PARAM_ERROR(0, F);

    // Verify/Determine ECC capabilities
#ifdef CYGSEM_HAL_ARM_IOP33X_ENABLE_ECC
    if (i2c_bytes[SPD_CONFIG] == SPD_CONFIG_NONE)
        // No error correction
        *ecc_cap = 0;
    else if (i2c_bytes[SPD_CONFIG] == SPD_CONFIG_ECC)
        // ECC error correction
        *ecc_cap = 1;
    else
        MEM_PARAM_ERROR(1, 0);
#else
    *ecc_cap = 0;
#endif

    // Verify/Determine refresh rate
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR)
    {
        if ((i2c_bytes[SPD_REFRESH] & SPD_RFR_MASK) == SPD_RFR_15_625us)
            refresh = RFR_333_MHZ_15_6us;
        else if ((i2c_bytes[SPD_REFRESH] & SPD_RFR_MASK) == SPD_RFR_7_8us)
            refresh = RFR_333_MHZ_7_8us;
        else
            MEM_PARAM_ERROR(1, 1);
    } else {
        if ((i2c_bytes[SPD_REFRESH] & SPD_RFR_MASK) == SPD_RFR_15_625us)
            refresh = RFR_400_MHZ_15_6us;
        else if ((i2c_bytes[SPD_REFRESH] & SPD_RFR_MASK) == SPD_RFR_7_8us)
            refresh = RFR_400_MHZ_7_8us;
        else
            MEM_PARAM_ERROR(1, 2);
    }

    // Verify/Determine SDRAM Width
    sdram_width = i2c_bytes[SPD_SDRAM_WIDTH];
    if ((sdram_width != 16) && (sdram_width != 8))
        MEM_PARAM_ERROR(1, 3);

    // Verify/Determine tCAS Setting
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR
        if (i2c_bytes[SPD_TCAS] & SPD_DDR_tCAS_LAT_2_5) {
            sdcr0_val |= (SDCR_tCAS_MCLK_2_5 | SDCR_tWDL_MCLK(0));
            tCAS = 3;
        } else {
            MEM_PARAM_ERROR(1, 4);
        }
    } else {
        // DDR-II
        if (i2c_bytes[SPD_TCAS] & SPD_DDRII_tCAS_LAT_4) {
            sdcr0_val |= (SDCR_tCAS_MCLK_4 | SDCR_tWDL_MCLK(2));
            tCAS = 4;
        } else {
            MEM_PARAM_ERROR(1, 5);
        }
    }

    // Verify/Determine Memory Module Attributes
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR
        if ((i2c_bytes[SPD_MOD_ATTRIB] & (SPD_ATTRIB_REG_CTL|SPD_ATTRIB_BUF_CTL)) ==
            SPD_ATTRIB_REG_CTL) {
            sdcr0_val |= SDCR_DIMM_TYPE_REGISTERED;
            tREG = 1;
        } else if ((i2c_bytes[SPD_MOD_ATTRIB] & (SPD_ATTRIB_REG_CTL|SPD_ATTRIB_BUF_CTL)) ==
                   SPD_ATTRIB_BUF_CTL) {
            sdcr0_val &= ~SDCR_DIMM_TYPE_REGISTERED;
            tREG = 0;
        } else if ((i2c_bytes[SPD_MOD_ATTRIB] & (SPD_ATTRIB_REG_CTL|SPD_ATTRIB_BUF_CTL)) == 0) {
            sdcr0_val &= ~SDCR_DIMM_TYPE_REGISTERED;
            tREG = 0;
        } else {
            MEM_PARAM_ERROR(1, 6);
        }
    } else {
        // DDR-II
        // Only Registered Supported and no status bits in the MOD_ATTRIB bytes
        // to tell us whether our DIMM is registered or not.
        // Assume it is and hope for the best.
        sdcr0_val |= SDCR_DIMM_TYPE_REGISTERED;
        tREG = 1;
    }

    // Verify/Determine tRP
    tRP = ns_to_cycles(SPD_tRP_to_ns(i2c_bytes[SPD_TRP]), ns_per_cycle) - 1;
    if (tRP <= SDCR_tRP_MAX)
        sdcr0_val |= SDCR_tRP_MCLK(tRP);
    else
        MEM_PARAM_ERROR(1, 7);

    // Verify/Determine tRCD
    tRCD = ns_to_cycles(SPD_tRCD_to_ns(i2c_bytes[SPD_TRCD]), ns_per_cycle) - 1;
    if (tRCD <= SDCR_tRCD_MAX)
        sdcr0_val |= SDCR_tRCD_MCLK(tRCD);
    else
        MEM_PARAM_ERROR(1, 8);

    // Verify/Determine tRAS
    tRAS = ns_to_cycles(SPD_tRAS_to_ns(i2c_bytes[SPD_TRAS]), ns_per_cycle) - 1;
    if (tRAS <= SDCR_tRAS_MAX)
        sdcr0_val |= SDCR_tRAS_MCLK(tRAS);
    else
        MEM_PARAM_ERROR(1, 9);

    // Verify/Determine Bank sizes
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR - SPD supports bank sizes up to 2GB but iq80331 and iq80332
        // only support a max of 1GB banks for DDR
        if (i2c_bytes[SPD_BANKSZ] == SPD_DDR_BANKSZ_1GB)
            bank_size_Mbytes = 1024;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDR_BANKSZ_512MB)
            bank_size_Mbytes = 512;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDR_BANKSZ_256MB)
            bank_size_Mbytes = 256;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDR_BANKSZ_128MB)
            bank_size_Mbytes = 128;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDR_BANKSZ_64MB)
            bank_size_Mbytes = 64;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDR_BANKSZ_32MB)
            bank_size_Mbytes = 32;
        else
            MEM_PARAM_ERROR(1, A);
    } else {
        // DDR-II - SPD supports bank sizes up to 16GB but iq80331 and iq80332
        // only support a max of 512MB banks for DDR-II
        if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_512MB)
            bank_size_Mbytes = 512;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_256MB)
            bank_size_Mbytes = 256;
        else if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_128MB)
            bank_size_Mbytes = 128;
        else
            MEM_PARAM_ERROR(1, B);
    }
    *sdram_size = (((bank_size_Mbytes * num_banks)
                    - CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE*2) << 20);

    // Verify/Determine tRC
    t = (cyg_uint16)SPD_tRC_to_ns(i2c_bytes[SPD_TRC]);
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDRII) {
        // DDR-II decoding based on JEDEC Spec dated 1/13/2003
        cyg_uint8 subfield_c = (i2c_bytes[SPD_TRFC_TRC_EXT] & 0xF0) >> 4;
        if (subfield_c == 0) {
            // No additional fractional ns part to tRC.
            ;
        } else if ((subfield_c >= 1) && (subfield_c <= 5)) {
            // There is an additional fractional ns part to tRFC.
            t += 1;
        } else {
            MEM_PARAM_ERROR(1, C);
        }
    }
    tRC = ns_to_cycles(t, ns_per_cycle) - 1;
    if (tRC <= SDCR_tRC_MAX)
        sdcr1_val |= SDCR_tRC_MCLK(tRC);
    else
        MEM_PARAM_ERROR(1, C);

    // Verify/Determine tWTR
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR -- tWTR == 1
        tWTR = 1;
    } else {
        // DDR-II -- read tWTR from the SPD
        tWTR = ns_to_cycles(SPD_tWTR_to_ns(i2c_bytes[SPD_TWTR]), ns_per_cycle);
    }

    // Verify/Determine tRFC
    t = (cyg_uint16)SPD_tRFC_to_ns(i2c_bytes[SPD_TRFC]);
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDRII) {
        // DDR-II decoding based on JEDEC Spec dated 1/13/2003
        cyg_uint8 subfield_a = ((i2c_bytes[SPD_TRFC_TRC_EXT] & 0x01) |
                                (i2c_bytes[SPD_TRFC_TRC_EXT] & 0x80) >> 6);
        cyg_uint8 subfield_b = (((i2c_bytes[SPD_TRFC_TRC_EXT] & 0x0E) >> 1) |
                                (i2c_bytes[SPD_TRFC_TRC_EXT] & 0x80) >> 6);

        if (subfield_a == 1) {
            t += 256;
        }

        if (subfield_b == 0) {
            // No additional fractional ns part to tRC.
            ;
        } else if ((subfield_b >= 1) && (subfield_b <= 5)) {
            // There is an additional fractional ns part to tRFC.
            t += 1;
        } else {
            MEM_PARAM_ERROR(1, D);
        }
    }
    tRFC = ns_to_cycles(t, ns_per_cycle) - 1;
    if (tRFC <= SDCR_tRFC_MAX)
        sdcr1_val |= SDCR_tRFC_MCLK(tRFC);
    else
        MEM_PARAM_ERROR(1, D);

    // Determine ODT Field
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDRII) {
        // DDR-II
        sdcr0_val |=SDCR_ODT_TERMINATION_75_OHM;
    }

    // Determine tWR Field
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR    333 = 0 -- (per JEDEC spec)
        sdcr1_val |= SDCR_tWR_MCLK_0;
    } else {
        // DDR-II 400 = 3 -- (per JEDEC spec)
        sdcr1_val |= SDCR_tWR_MCLK_3;
    }
    tWR = ns_to_cycles(SDCR_tWR_JEDEC_NS, ns_per_cycle);

    // Determine DQS# Disable Field
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR
        // Use single-ended operation for DDR-I
        sdcr1_val |= SDCR_DQS_ENABLE_MASK;
    } else {
        // DDR-II
        // Use differential operation for DDR-II
        sdcr1_val &= ~SDCR_DQS_ENABLE_MASK;
    }
        
    // Determine tWTRD -- tCAS + tWTR + tREG
    sdcr1_val |= SDCR_tWTRD_MCLK(tCAS + tWTR + tREG);
        
    // Determine tEDP
    tEDP = 2;
    sdcr0_val |= SDCR_tEDP_MCLK(tEDP);

    // Determine tRTCMD
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR
        tRTCMD = 2;
    } else {
        // DDR-II
        tRTCMD = 3;
    }
    sdcr1_val |= SDCR_tRTCMD_MCLK(tRTCMD);

    // Determine tRTW  -- tCAS + tREG + tEDP + 1
    tRTW = tCAS + tREG + tEDP + 1;
    if (tRTW == 8)
        tRTW = 0;
    sdcr1_val |= SDCR_tRTW_MCLK(tRTW);
        
    // Determine tWTCMD -- tCAS + tWR + tREG

    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR

        // For DDR, we know that both tCAS and tWR
        // as calculated by ns_to_cycles() will have
        // been rounded up resulting in a double rounding.
        // Go ahead and subtract 1 here to compensate.
        tWR -= 1;
    }
    sdcr1_val |= SDCR_tWTCMD_MCLK(tCAS + tWR + tREG);

    // Determine RCVDLY
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        // DDR
        rcvdly_val = 0x7;
    } else {
        // DDR-II
        rcvdly_val = 0x2;
    }

    // Assumption: All banks are the same size.

    // Set up bank 0 register
    if (bank_size_Mbytes == 32)
        sbr_base = SBR_32MEG;
    else if (bank_size_Mbytes == 64)
        sbr_base = SBR_64MEG;
    else if (bank_size_Mbytes == 128)
        sbr_base = SBR_128MEG;
    else if (bank_size_Mbytes == 256)
        sbr_base = SBR_256MEG;
    else if (bank_size_Mbytes == 512)
        sbr_base = SBR_512MEG;
    else if (bank_size_Mbytes == 1024)
        sbr_base = SBR_1GIG;
    else
        MEM_PARAM_ERROR(1, E);

    // Determine the proper address translation bits
    if ((num_rows == 12) && (num_cols == 9))
        addr_tx = SBR_AddrTX_1;
    else if ((num_rows == 12) && (num_cols == 10))
        addr_tx = SBR_AddrTX_1;
    else if ((num_rows == 13) && (num_cols == 9))
        addr_tx = SBR_AddrTX_2;
    else if ((num_rows == 13) && (num_cols == 10))
        addr_tx = SBR_AddrTX_1;
    else if ((num_rows == 13) && (num_cols == 11))
        addr_tx = SBR_AddrTX_1;
    else if ((num_rows == 14) && (num_cols == 10))
        addr_tx = SBR_AddrTX_3;
    else if ((num_rows == 14) && (num_cols == 11))
        addr_tx = SBR_AddrTX_1;
    else
        MEM_PARAM_ERROR(1, F);

    // Determine the actual SBR values
    sbr0_val = (sbr_base + ((SDRAM_PHYS_BASE >> 25) & 0x7f)) | addr_tx;
    if (num_banks == 2)
        sbr1_val = sbr0_val + sbr_base;
    else
        sbr1_val = sbr0_val;

    // Set the MCU MMRs
    HAL_WRITE_UINT32(MCU_SDBR, SDRAM_PHYS_BASE);
    HAL_WRITE_UINT32(MCU_SDCR0, sdcr0_val);
    HAL_WRITE_UINT32(MCU_SDCR1, sdcr1_val);
    HAL_WRITE_UINT32(MCU_SBR0, sbr0_val);
    HAL_WRITE_UINT32(MCU_SBR1, sbr1_val);
    HAL_WRITE_UINT32(MCU_S32SR, S32SR_MEG(CYGSEM_HAL_ARM_IOP33X_32BIT_ECC_REGION_SIZE));
    HAL_WRITE_UINT32(MCU_RCVDLY, rcvdly_val);

    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDBR);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDCR0);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDCR1);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SBR0);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SBR1);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_S32SR);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_RCVDLY);

    // Update the DLL registers per specification change #19.
    if (ddr_type == SPD_MEMTYPE_SDRAM_DDR) {
        HAL_WRITE_UINT32(MCU_SLVLMIX0, MCU_SLVLMIX0_DEFAULT_VALUE_DDR);
        HAL_WRITE_UINT32(MCU_SLVLMIX1, MCU_SLVLMIX1_DEFAULT_VALUE_DDR);
        HAL_WRITE_UINT32(MCU_SLVHMIX0, MCU_SLVHMIX0_DEFAULT_VALUE_DDR);
        HAL_WRITE_UINT32(MCU_SLVHMIX1, MCU_SLVHMIX1_DEFAULT_VALUE_DDR);
        HAL_WRITE_UINT32(MCU_SLVLEN, MCU_SLVLEN_DEFAULT_VALUE_DDR);
        HAL_WRITE_UINT32(MCU_MASTMIX, MCU_MASTMIX_DEFAULT_VALUE_DDR);
        HAL_WRITE_UINT32(MCU_MASTLEN, MCU_MASTLEN_DEFAULT_VALUE_DDR);
    } else {
        HAL_WRITE_UINT32(MCU_SLVLMIX0, MCU_SLVLMIX0_DEFAULT_VALUE_DDRII);
        HAL_WRITE_UINT32(MCU_SLVLMIX1, MCU_SLVLMIX1_DEFAULT_VALUE_DDRII);
        HAL_WRITE_UINT32(MCU_SLVHMIX0, MCU_SLVHMIX0_DEFAULT_VALUE_DDRII);
        HAL_WRITE_UINT32(MCU_SLVHMIX1, MCU_SLVHMIX1_DEFAULT_VALUE_DDRII);
        HAL_WRITE_UINT32(MCU_SLVLEN, MCU_SLVLEN_DEFAULT_VALUE_DDRII);
        HAL_WRITE_UINT32(MCU_MASTMIX, MCU_MASTMIX_DEFAULT_VALUE_DDRII);
        HAL_WRITE_UINT32(MCU_MASTLEN, MCU_MASTLEN_DEFAULT_VALUE_DDRII);
    }
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SLVLMIX0);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SLVLMIX1);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SLVHMIX0);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SLVHMIX1);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_SLVLEN);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_MASTMIX);
    hal_platform_lowlevel_serial_debug_put_value32(*MCU_MASTLEN);

    // Read-back certain MCU registers after writing, but before
    // accessing memory per C-Spec
    {
        volatile cyg_uint32 ign;
        HAL_READ_UINT32(MCU_SDIR, ign);
        HAL_READ_UINT32(MCU_SDBR, ign);
        HAL_READ_UINT32(MCU_SBR0, ign);
        HAL_READ_UINT32(MCU_SBR1, ign);
        HAL_READ_UINT32(MCU_S32SR, ign);
    }

    HEX_DISPLAY_QUICK(2, 0);

    {
        cyg_uint32 dev_id;
        cyg_uint32 biucr_val;
        asm volatile("mrc\tp15, 0, %0, c0, c0, 0" : "=r" (dev_id));
        HAL_READ_UINT32(BIU_BIUCR, biucr_val);

        //
        // Enable the MCU Core port on D0 and above
        //
        if ((dev_id == IOP331_DEVICE_ID_A_1) ||
            (dev_id == IOP331_DEVICE_ID_B_0) ||
            (dev_id == IOP331_DEVICE_ID_C_0) ||
            (dev_id == IOP331_DEVICE_ID_C_1) ||
            (dev_id == IOP332_DEVICE_ID_A_1)  ||
            (dev_id == IOP332_DEVICE_ID_A_2)  ||
            (dev_id == IOP332_DEVICE_ID_B_0)  ||
            (dev_id == IOP332_DEVICE_ID_C_0)  ||
            (dev_id == IOP332_DEVICE_ID_C_1)) {
            hal_platform_lowlevel_serial_debug_puts("Disabling MCU-Core Port.\r\n");
            HAL_WRITE_UINT32(BIU_BIUCR, biucr_val & ~BIUCR_MCU_PORT_ENABLE);
        } else {
            hal_platform_lowlevel_serial_debug_puts("Enabling MCU-Core Port.\r\n");
            HAL_WRITE_UINT32(BIU_BIUCR, biucr_val | BIUCR_MCU_PORT_ENABLE);
        }
    }

    HEX_DISPLAY_QUICK(2, 1);

    // Clear any pending MCU interrupts
    HAL_WRITE_UINT32(MCU_MCISR, 0x1F);

    //
    // SDRAM Initialization Sequence as specified by the JEDEC Spec
    // Measured values are based on 800MHz core clock
    // Updated Feb21'05 to comply with JESD79-2 (Nov'04) DDR2 spec
	// Steps are numbered based on JESD79-2 (DDR2 Spec)
	// STEPs 1-2 of JEDEC are done in Hardware up to this point
    HAL_WRITE_UINT32(MCU_RFR, 0);                                           // Disable the refresh counter

	// STEP 3
    SDRAM_INIT_DELAY(0x20000);                                              // 200us device deselect -- measured 800us
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_NOP);                               // One NOP cycle

    // STEP 4
    SDRAM_INIT_DELAY(100);                                                  // Delay for 400nS minimum
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_PRECHARGE_ALL);                     // Precharge-All command

	// STEP 5 and 6 - EMRS(2) and EMRS(3) programming - DDR2 Only
	/* Feb21'05: Added EMRS(2) and EMRS(3) commands per JESD79-2 (Nov'04) - cebruns */
	// Verified on Logic Analzyer - cebruns
	if (ddr_type == SPD_MEMTYPE_SDRAM_DDRII) {
		HAL_WRITE_UINT32(MCU_DCALADDR, DCALADDR_EMRS2);
		HAL_WRITE_UINT32(MCU_DCALCSR,  DCALCSR_EMRS_CS_0);
		wait_for_dcal();
		HAL_WRITE_UINT32(MCU_DCALCSR,  DCALCSR_EMRS_CS_1);
		wait_for_dcal();
		// STEP 6
		HAL_WRITE_UINT32(MCU_DCALADDR, DCALADDR_EMRS3);
		HAL_WRITE_UINT32(MCU_DCALCSR,  DCALCSR_EMRS_CS_0);
		wait_for_dcal();
		HAL_WRITE_UINT32(MCU_DCALCSR,  DCALCSR_EMRS_CS_1);
		wait_for_dcal();
	}
	/* End of EMRS(2) and EMRS(3) */

	// STEP 7
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_EMRS_DLL_ENABLED);                  // EMRS to enable the DLL
    SDRAM_INIT_DELAY(2);                                                    // Delay for Tmrd cycles -- 2 mclk -- 10 ns -- measured 115ns

    // STEP 8
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_MRS_DLL_IS_RESET_CAS_4_BURST_4);    // MRS to program the SDRAM parameters and DLL reset
    SDRAM_INIT_DELAY(2);                                                    // Delay for Tmrd cycles -- 2 mclk -- 10 ns -- measured 115ns

	// STEP 9
	// Added Feb21'05: Added Precharge All Command - cebruns
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_PRECHARGE_ALL);                     // Precharge-All command
    SDRAM_INIT_DELAY(SDCR_tRP_MAX);                                         // Delay for Trp cycles -- 7 mclk -- 35 ns -- measured 165ns

	// STEP 10 - Issue at least two AutoRefresh Cycles
#if 0
    // Issuing AUTO_REFRESHES via the MCU_SDIR because there is no way
    // to decouple the precharge all and auto refresh commands in the MCU.
    // Each of these commands would actually result in AUTO_REFRESH 
    // followed by PRECHARGE_ALL.
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_AUTO_REFRESH);                      // First of two auto-refresh cycles
    SDRAM_INIT_DELAY(SDCR_tRFC_MAX);                                        // Delay for Trfc cycles -- 31 mclk -- 155 ns -- measured 14us
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_AUTO_REFRESH);                      // Second of two auto-refresh cycles
#else
    // For both DDRI and DDRII initialization sequences, there are 2 back to
    // back AUTO_REFRESH commands issued to the DIMM.. Instead of using MCU_SDIR
    // to do this use DCAL by writing to the DCALCSR to issue an AUTO_REFRESH 
    // to each CS.  Thus there will be 4 writes to the DCALCSR as opposed to
    //the previous 2 writes to MCU_SDIR.  The exact address and data values are:
    //
    //    ADDR        WRITE_DATA       NOTE
    //    DCALCSR     0x8000_0001      -- ARF to CS[0]
    //    DCALCSR     0x8000_0001      -- ARF to CS[0]
    //    DCALCSR     0x8010_0001      -- ARF to CS[1]
    //    DCALCSR     0x8010_0001      -- ARF to CS[1]
    HAL_WRITE_UINT32(MCU_DCALCSR, DCALCSR_AUTO_REFRESH_CS_0);
	wait_for_dcal();
    HAL_WRITE_UINT32(MCU_DCALCSR, DCALCSR_AUTO_REFRESH_CS_1);
	wait_for_dcal();
    SDRAM_INIT_DELAY(SDCR_tRFC_MAX);                                        // Delay for Trfc cycles -- 31 mclk -- 155 ns -- measured 14us
    HAL_WRITE_UINT32(MCU_DCALCSR, DCALCSR_AUTO_REFRESH_CS_0);
	wait_for_dcal();
    HAL_WRITE_UINT32(MCU_DCALCSR, DCALCSR_AUTO_REFRESH_CS_1);
	wait_for_dcal();
#endif
    SDRAM_INIT_DELAY(SDCR_tRFC_MAX);                                        // Delay for Trfc cycles -- 31 mclk -- 155 ns -- measured 310ns 

	// STEP 11
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_MRS_DLL_NOT_RESET_CAS_IN_SDCR0);    // MRS to program the SDRAM parameters and no DLL reset
    SDRAM_INIT_DELAY(2);                                                    // Delay for Tmrd cycles -- 2 mclk -- 10 ns -- measured 29us
    HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_NORMAL_OPERATION);                  // DDR normal operation

	// STEP 12 - DDR2 Only
	/* Feb21'05: Added step 12 of JEDEC init sequence per JESD79-2 (Nov'04) - cebruns */
	// Write OCD_CAL_DEFAULT and then OCD_CAL_EXIT 
	if (ddr_type == SPD_MEMTYPE_SDRAM_DDRII) {
		cyg_uint32  dcaladdr_value = (DCALADDR_EMRS1 | DCALADDR_OCD_CAL_DEFAULT);

		SDRAM_INIT_DELAY(200);
		switch (sdcr0_val & SDCR_ODT_TERMINATION_MASK) {
			case SDCR_ODT_TERMINATION_DISABLED:
				break;
			case SDCR_ODT_TERMINATION_75_OHM:
				dcaladdr_value |= (DCALADDR_RTT_75_OHM);
				break;
			case SDCR_ODT_TERMINATION_150_OHM:
				dcaladdr_value |= (DCALADDR_RTT_150_OHM);
				break;
		}
		// Write OCD Cal Default.  From Logic Analyzer it appears the DCAL logic writes
		// OCD Cal Mode Exit 9 clocks after OCD Cal Default is written
		HAL_WRITE_UINT32(MCU_DCALADDR, dcaladdr_value);
		HAL_WRITE_UINT32(MCU_DCALCSR,  DCALCSR_EMRS_CS_0);
		wait_for_dcal();
		HAL_WRITE_UINT32(MCU_DCALCSR,  DCALCSR_EMRS_CS_1);
		wait_for_dcal();
		/* End of OCD Calibration setting */
	}

	// STEP 13
    HAL_WRITE_UINT32(MCU_RFR, refresh);                                     // Re-enable the refresh counter.
    HEX_DISPLAY_QUICK(2, 2);

    *sdram_installed = true;
    return;
}

//
// Perform Memory Scrub
//
void mcu_memory_scrub(cyg_uint32 base, cyg_uint32 size)
{
#ifdef CYGSEM_HAL_ARM_IQ8033X_BATTERY_TEST
    cyg_uint32 battery_back_test_value;
#endif

    HEX_DISPLAY_QUICK(S, L);
    hal_platform_lowlevel_serial_debug_puts("In mcu_memory_scrub()\r\n");

    // clear test register
    HAL_WRITE_UINT32(MCU_ECTST, 0);

    // disable ECC, disable reporting
    HAL_WRITE_UINT32(MCU_ECCR, 0);

#ifdef CYGSEM_HAL_ARM_IQ8033X_BATTERY_TEST
    HAL_DCACHE_INVALIDATE(SDRAM_BATTERY_TEST_ADDR, 4);
    HAL_READ_UINT32(SDRAM_BATTERY_TEST_ADDR, battery_back_test_value);
#endif

    if (size != 0) {
        asm volatile ("   mov     r0, #0\n"
                      "   mov     r1, #0\n"
                      "   mov     r2, #0\n"
                      "   mov     r3, #0\n"
                      "   mov     r4, #0\n"
                      "   mov     r5, #0\n"
                      "   mov     r6, #0\n"
                      "   mov     r7, #0\n"
                      "   mov     r8, %[size]\n"
                      "   mov     r9, %[base]\n"
                      "0: stmia   r9, {r0-r7}\n"
                      "   mcr     p15, 0, r9, c7, c10, 1\n"
                      "   add     r9, r9, #32\n"
                      "   subs    r8, r8, #32\n"
                      "   bne     0b" 
                      : /* output */ 
                      : /* input */ [size] "r" (size), [base] "r" (base) 
                      : /* clobber */ "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9");
    }

    hal_platform_lowlevel_serial_debug_puts("DONE\r\n");

#ifdef CYGSEM_HAL_ARM_IQ8033X_BATTERY_TEST
    HAL_WRITE_UINT32(SDRAM_BATTERY_TEST_ADDR, battery_back_test_value);
    HAL_DCACHE_STORE(SDRAM_BATTERY_TEST_ADDR, sizeof(battery_back_test_value));
#endif
    HEX_DISPLAY_QUICK(S, E);
}

//
// Enable ECC
//
void mcu_enable_ecc(void)
{
    // Enable ECC and all reporting
    HAL_WRITE_UINT32(MCU_ECCR, (ECCR_SINGLE_BIT_REPORT_ENABLE | ECCR_MULTI_BIT_REPORT_ENABLE | 
                                ECCR_SINGLE_BIT_CORRECT_ENABLE | ECCR_ECC_ENABLE));
}
