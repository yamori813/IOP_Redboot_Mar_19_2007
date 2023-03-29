/*=============================================================================
//
//      iq8134x_setup_mcu.c
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
// Purpose:      Intel XScale IQ8134X CRB platform specific MCU initialization
// Description:
//     This file contains low-level MCU initialization code called by
//     hal_platform_setup.h.
//     Since we may be using "alternative" memory (ie different stack and heap than
//     we were compiled for), we cannot use any global data or C Library calls.
// Usage:
//     Only called by "hal_platform_setup.c"
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/cyg_type.h>
#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/iq8134x.h>            // Platform specific hardware definitions
#include <cyg/hal/hal_iop34x.h>         // Variant specific hardware definitions
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_spd.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_diag.h>

#undef INIT_MCU_ANYWAY

static inline void delay_mclks(unsigned int mclks);
int sampleDQS(int dllval,int reg);


// Delay for some time.
// These delays are used for the SDRAM initialization sequence below.
// They are manually tweaked using a DDR-II analyzer.  If the core
// speed changes, these may need to be tweaked however they have
// been calibrated with some margin.
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

static inline void delay_mclks(unsigned int mclks) {
	volatile unsigned int value;
	TMR1_READ(value);  // In case Terminal Status is set, read will clear it
	TMR1_WRITE(0);
	// Use DDR400MHz as calculation, since at DDR533 we'll only be
	// delaying a little longer than necessary which is not an issue.
	// Timer runs at IB frequency (400MHz), so we have to set timer to
	// desired MCLKS*2 since DDR400 runs at 200MHz
	TCR1_WRITE(mclks*2);
	TMR1_WRITE(TMR_TIMER_ENABLE);
	do {
		TMR1_READ(value);
	}while ((value & TMR_TERMINAL_COUNT) == 0);
}

// Convert a divisor in ps to a count of MCLKs by
// "dividing" by the ps_per_cycle.
// Using a local divide algorithm so we don't have to
// call into the C library
// Use pSec instead of nS because DDR2-533MHz has a 3.75nS cycle
// time.
static unsigned ps_to_cycles(unsigned divisor, unsigned dividend)
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

#define SPD_NUM_BYTES 64
//
// Perform MCU Initialization
//
void mcu_initialization(cyg_uint32 *sdram_size,
                        cyg_bool *sdram_installed,
                        cyg_bool *ecc_cap)
{
    cyg_uint32 ddr_type, num_banks, num_rows, num_cols, temp;
    cyg_uint32 refresh = 0, sdram_width = 0, tCAS = 0;
    cyg_uint32 ps_per_cycle = 0, tRP = 0, tRCD = 0, tRAS = 0, tRC = 0, tWTR = 0, tRFC = 0, tREG = 0, tWR = 0;
    cyg_uint32 tRTP = 0, tEDP = 0;
    cyg_uint32 bank_size_Mbytes = 0, sbsr_val, emrs = 0, mrs = 0;
    cyg_uint32 sdcr0_val = 0, sdcr1_val = 0, sdbr_val = 0;
    cyg_uint32 t;
    cyg_uint8 subfield_a, subfield_b, subfield_c, i2c_bytes[SPD_NUM_BYTES], cksum = 0;
    cyg_uint8 rev_id;
    volatile unsigned int scratch;
    int i;
    unsigned char buf;

    HEX_DISPLAY_QUICK(0, 6);
    hal_platform_lowlevel_serial_debug_puts("In mcu_initialization()\r\n");

    i2c_reset();
    i2c_init();

#ifndef CYGSEM_HAL_ARM_IQ8134X_HARDCODE_MCU
    // Write 0 to the SDRAM SPD to reset the EEPROM pointer
    buf = 0;
    unsigned char sdram_devid = SDRAM_DEVID;
    if (i2c_write_bytes(sdram_devid, &buf, 1) == -1) {
        sdram_devid+=2;   // try next IC address
        if (i2c_write_bytes(sdram_devid, &buf, 1) == -1) {
        // Timeout Assume no device in system
#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        *sdram_size = HAL_DCACHE_LOCKABLE_SIZE;
        *sdram_installed = false;
// Initialize the MCU even though we don't want to use it for RedBoot
#ifndef  INIT_MCU_ANYWAY
        return;
#endif
#else
        MEM_PARAM_ERROR(0, 7);
#endif
        }
    }

    // Read the bytes
    if (i2c_read_bytes(sdram_devid, i2c_bytes, SPD_NUM_BYTES) == -1) {
        // Timeout Assume no device in system
#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
        *sdram_size = HAL_DCACHE_LOCKABLE_SIZE;
        *sdram_installed = false;
#ifndef INIT_MCU_ANYWAY
        return;
#endif
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
	// Only support DDR-II for IQ8134x
	ddr_type = i2c_bytes[SPD_MEMTYPE];
	if (ddr_type != SPD_MEMTYPE_SDRAM_DDRII)
		MEM_PARAM_ERROR(0, A);
	
	// Verify the number of row addresses
	num_rows = i2c_bytes[SPD_NUM_ROWS];
	if ((num_rows < SBR_ROW_ADDRESS_MIN) || (num_rows > SBR_ROW_ADDRESS_MAX))
		MEM_PARAM_ERROR(0, B);
	
	// Verify the number of column addresses
	num_cols = i2c_bytes[SPD_NUM_COLS];
	if ((num_cols < SBR_COL_ADDRESS_MIN) || (num_cols > SBR_COL_ADDRESS_MAX))
		MEM_PARAM_ERROR(0, C);
	
	// Verify/Determine the number of banks
	num_banks = (i2c_bytes[SPD_BANKCNT] & SPD_DDRII_BANKCNT_MASK) + 1;
	if (num_banks > 2)
		MEM_PARAM_ERROR(0, D);
	
	// Verify/Determine the data bus width
	if (i2c_bytes[SPD_MOD_WIDTH1] == 32) 	// 32-bit data
		sdcr0_val |= SDCR_BUS_WIDTH_32_BITS;
	else if (i2c_bytes[SPD_MOD_WIDTH1] == 40) // 32-bit data + 8-bit ECC
		sdcr0_val |= SDCR_BUS_WIDTH_32_BITS;
	else if (i2c_bytes[SPD_MOD_WIDTH1] == 64) // 64-bit data
		sdcr0_val |= SDCR_BUS_WIDTH_64_BITS;
	else if (i2c_bytes[SPD_MOD_WIDTH1] == 72) // 64-bit data + 8-bit ECC
		sdcr0_val |= SDCR_BUS_WIDTH_64_BITS;
	else
		MEM_PARAM_ERROR(0, E);
	
	if (i2c_bytes[SPD_MOD_WIDTH2] != 0)
		MEM_PARAM_ERROR(0, F);

	// Verify/Determine ECC capabilities
#ifdef CYGSEM_HAL_ARM_IOP34X_ENABLE_ECC
	if (i2c_bytes[SPD_CONFIG] == SPD_CONFIG_NONE)	
		*ecc_cap = 0;
	else if (i2c_bytes[SPD_CONFIG] == SPD_CONFIG_ECC)
		*ecc_cap = 1;
	else
		MEM_PARAM_ERROR(1, 0);
#else
	*ecc_cap = 0;
#endif

	
	// Verify/Determine DDR Cycle Time.  266MHz = DDR2-533. 200MHz=DDDR2-400
	// Also make sure we're strapped for that setting
	if ((*PBIU_ESSTSR0 & MEM_FREQ_MASK) == MEM_FREQ_533)
	{
		ps_per_cycle = 3750;
		if (i2c_bytes[SPD_CYCLE_TIME] == SPD_CYCLE_200MHZ)
			MEM_PARAM_ERROR(1, 1);   // DIMM doesn't support 533MHz operation
	}
	else if ((*PBIU_ESSTSR0 & MEM_FREQ_MASK) == MEM_FREQ_400)
		ps_per_cycle = 5000;
	else
		MEM_PARAM_ERROR(1, 1);

	// Verify/Determine Refresh rate based on Refresh setting and
	// DDR cycle time.  Also set ps_per_cycle for later calculations.
	if ((i2c_bytes[SPD_REFRESH] & SPD_RFR_MASK) == SPD_RFR_15_625us) {
		if (ps_per_cycle == 3750)
			refresh = RFR_533_MHZ_15_6us;
		else
			refresh = RFR_400_MHZ_15_6us;
	}
	else if ((i2c_bytes[SPD_REFRESH] & SPD_RFR_MASK) == SPD_RFR_7_8us) {
		if (ps_per_cycle == 3750)
			refresh = RFR_533_MHZ_7_8us;
		else
			refresh = RFR_400_MHZ_7_8us;
	}
	else
		MEM_PARAM_ERROR(1, 2);

	// Verify/Determine SDRAM Width
	sdram_width = i2c_bytes[SPD_SDRAM_WIDTH];
	if ((sdram_width != 16) && (sdram_width != 8))
		MEM_PARAM_ERROR(1, 3);

	
	// IQ8134x only supports tCAS of 4 and higher
	if ((i2c_bytes[SPD_TCAS] & SPD_DDRII_tCAS_LAT_3) || (i2c_bytes[SPD_TCAS] & SPD_DDRII_tCAS_LAT_4))
		tCAS = 4;
	else if (i2c_bytes[SPD_TCAS] & SPD_DDRII_tCAS_LAT_5)
		tCAS = 5;
	else
		MEM_PARAM_ERROR(1, 4);

	
	// Verify/Determine Memory Module Attributes
	// SPD byte 20, bit0 = Registered, bit1 = Unbuffered
	// However, the SDCR0 on 8134x must ALWAYS BE SET for Registered
	// and then the DLLRCVER is set to Unbuffered or Registered (bit 16).
	if (i2c_bytes[SPD_DIMM_TYPE] & SPD_REGISTERED)
		tREG = 0;  // DLLRCVER uses 0 to mean registered
	else if (i2c_bytes[SPD_DIMM_TYPE] & SPD_UNBUFFERED)
		tREG = 1;  // and 1 to mean unbuffered
	else
		MEM_PARAM_ERROR(1, 5);
		
	
	// SPD_tRP_to_ps/SPD_tRCD_to_ps macro strips off bits 1:0 which are
	// <1nS increments and multiplies by 1000 to get psec because 533MHz
	// DDR2 has a 3.75ns cycle time and I don't want to mess w/ decimals
    	// Verify/Determine tRP
	tRP = ps_to_cycles(SPD_tRP_to_ps(i2c_bytes[SPD_TRP]), ps_per_cycle);
	if (tRP >SDCR_tRP_MAX)
		MEM_PARAM_ERROR(1, 6);

	// Verify/Determine tRCD
	tRCD = ps_to_cycles(SPD_tRCD_to_ps(i2c_bytes[SPD_TRCD]), ps_per_cycle);
	if (tRCD > SDCR_tRCD_MAX)
		MEM_PARAM_ERROR(1, 7);

	// Verify/Determine tRAS (tRAS reported in integer nS - no fractions)
	tRAS = ps_to_cycles(SPD_tRAS_to_ps(i2c_bytes[SPD_TRAS]), ps_per_cycle);
	if (tRAS > SDCR_tRAS_MAX)
		MEM_PARAM_ERROR(1, 8);


	// Verify/Determine Bank sizes
	// DDR-II - SPD supports bank sizes up to 16GB but iq8134x
	// only support a max of 1024MB banks for DDR-II
	if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_1GB)
		bank_size_Mbytes = 1024;
	else if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_512MB)
		bank_size_Mbytes = 512;
	else if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_256MB)
		bank_size_Mbytes = 256;
	else if (i2c_bytes[SPD_BANKSZ] == SPD_DDRII_BANKSZ_128MB)
		bank_size_Mbytes = 128;
	else
        	MEM_PARAM_ERROR(1, 9);

	// Verify/Determine tRC
	// DDR-II decoding based on JEDEC Spec dated 1/13/2003
	t = (cyg_uint16)SPD_tRC_to_ns(i2c_bytes[SPD_TRC]);
	subfield_c = (i2c_bytes[SPD_TRFC_TRC_EXT] & 0xF0) >> 4;
	
	t *= 1000;
	
	if (subfield_c == 1)
		t += 250;  // Add .25nS
	else if (subfield_c == 2)
		t += 330;  // Add .33nS
	else if (subfield_c == 3)
		t += 500;  // Add .5nS
	else if (subfield_c == 4)
		t += 660;  // Add .66nS
	else if (subfield_c == 5)
		t += 750;  // Add .75nS
	else if (subfield_c > 5)
		MEM_PARAM_ERROR(1, A);
	
	// Get cycles based on psec value of tRC (t is in nS)
	tRC = ps_to_cycles(t, ps_per_cycle);
	if (tRC > SDCR_tRC_MAX)
		MEM_PARAM_ERROR(1, B);

	// Verify/Determine tWTR
	// DDR-II -- read tWTR from the SPD
	tWTR = ps_to_cycles(SPD_tWTR_to_ps(i2c_bytes[SPD_TWTR]), ps_per_cycle);

    
	// Verify/Determine tRFC
	t = (cyg_uint16)SPD_tRFC_to_ns(i2c_bytes[SPD_TRFC]);
	
	// Change nS to pSec to help addition of subfield parameters
	t *= 1000;
	
	// DDR-II decoding based on JEDEC Spec dated 1/13/2003
	subfield_a = ((i2c_bytes[SPD_TRFC_TRC_EXT] & 0x01) |
				(i2c_bytes[SPD_TRFC_TRC_EXT] & 0x80) >> 6);
	subfield_b = (((i2c_bytes[SPD_TRFC_TRC_EXT] & 0x0E) >> 1) |
				(i2c_bytes[SPD_TRFC_TRC_EXT] & 0x80) >> 6);
	if (subfield_a == 1)
		t += 256*1000;
	
	//  The ROMRAM version of Redboot cannont utilize switch statements
	//  before jump to RAM.  Therefore we use a series of if statements
	// instead of the original switch statement.
	if (subfield_b == 1)
		t += 250;  // Add .25nS
	else if (subfield_b == 2)
		t += 330;  // Add .33nS
	else if (subfield_b == 3)
		t += 500;  // Add .5nS
	else if (subfield_b == 4)
		t += 660;  // Add .66nS
	else if (subfield_b == 5)
		t += 750;  // Add .75nS
	else if (subfield_b > 5)
		MEM_PARAM_ERROR(1, C);

	
	// Convert temp var, t into psec and get cycles back
	tRFC = ps_to_cycles(t, ps_per_cycle);
	if (tRFC > SDCR_tRFC_MAX)
		MEM_PARAM_ERROR(1, D);
	
	
	// Determine tWR from SPD (currently should always report 15nS)
	tWR = ps_to_cycles(SPD_tWR_to_ps(i2c_bytes[SPD_TWR]), ps_per_cycle);
	if (tWR > SDCR_tWR_MAX)
		MEM_PARAM_ERROR(1,E);
	
	// Determine tEDP - Equation TBD - just use default from C-spec
	tEDP = tEDP_FROM_DESIGN_8_INCH_TRACE;
	
	// Determine tRTCMD - Equation 9
	// From C-spec: Equation 9: (RTCMD = tRTP = X)
	// Where X == 2 for 400&533 MHz DDR-II
	// Grab tRTP from SPD, but this field should always be 2
	tRTP = ps_to_cycles(SPD_tRTP_to_ps(i2c_bytes[SPD_TRTP]), ps_per_cycle);
	if (tRTP != 2)
		MEM_PARAM_ERROR(1, F);


#else // HardCode MCU Values
	*ecc_cap = 0;

	if ((*PBIU_ESSTSR0 & MEM_FREQ_MASK) == MEM_FREQ_533)
	{	
		ps_per_cycle = 3750;
		refresh   = 0x820;
	}		  
	else
	{
		ps_per_cycle = 5000;
		refresh   = 0x618;
	}		  

	tRP = 0x4;
	tCAS = 0x4;
	tRCD = 0x4;
	tRAS = 0xC;
	tRC = 0x10;
	tWTR = 0x2;
	tRFC = 0x1C;
	tREG = 0x0;
	tWR = 0x4;
	tRTP = 0x2;
	tEDP = 0x4;
	
	num_banks = 2;
	bank_size_Mbytes = 512

			
#endif // Hardcode
	
	// Setup sdcr0
	sdcr0_val |= 0x91;
	sdcr0_val |= ((tCAS-1) << 8);
	sdcr0_val |= ((tCAS-2) << 12);
	sdcr0_val |= (tEDP << 16);
	sdcr0_val |= ((tRCD -1) << 20);
	sdcr0_val |= ((tRP -1) << 24);
	sdcr0_val |= ((tRAS -1) << 27);

	
	// Setup sdcr1
	sdcr1_val = tCAS - 1 + 2 + tWTR;
	sdcr1_val |= ((tRC - 1) << 4);
	sdcr1_val |= ((tWR) << 9);
	sdcr1_val |= ((tRFC - 1) << 12);
	sdcr1_val |= ((4) << 19);
	sdcr1_val |= ((tCAS - 1 + 2 + tWR) << 23);
	sdcr1_val |= ((tRTP) << 27);


	// Set up sbsr_val
	// Assumption: All banks are the same size.
	if (bank_size_Mbytes == 128)
		sbsr_val = SBR_128MEG;
	else if (bank_size_Mbytes == 256)
		sbsr_val = SBR_256MEG;
	else if (bank_size_Mbytes == 512)
		sbsr_val = SBR_512MEG;
	else if (bank_size_Mbytes == 1024)
		sbsr_val = SBR_1GIG;
	else
		MEM_PARAM_ERROR(2, 0);
	
	if (num_banks == 1)
		sbsr_val |= SBSR_ONE_BANK;
	else if (num_banks == 2)
		sbsr_val |= SBSR_TWO_BANKS;
	
	sbsr_val |= ((tCAS + 2 + tEDP) << 8);

	*sdram_size = (((bank_size_Mbytes * num_banks)
			- CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE*2) << 20);
	
	// SDBR is the Physical Base
	sdbr_val = (SDRAM_PHYS_BASE);

	
	//  Use the recommended non-default unbuffered drive strength settings
	//  if we are using an unbuffered DIMM
	if (tREG)
	{
		HAL_READ_UINT32(MCU_DRCR, scratch);
		scratch &= 0xFFFFFE7F;
		HAL_WRITE_UINT32(MCU_DRCR, scratch);
		
		HAL_READ_UINT32(MCU_RPDSR, scratch);
		scratch &= 0xFFFFF1C7;
		scratch |= ((3 << 9) | (3 << 3));
		HAL_WRITE_UINT32(MCU_RPDSR, scratch);
	}
	
	// Set the MCU MMRs
	HAL_WRITE_UINT32(MCU_SDBR, sdbr_val);
	HAL_WRITE_UINT32(MCU_SDUBR, 0);
	
	HAL_WRITE_UINT32(MCU_SDCR0, sdcr0_val);  // Write SDCR0
	HAL_READ_UINT32 (MCU_SDCR0, sdcr0_val);  // ReadBack to make sure it's written
	
	HAL_WRITE_UINT32(MCU_SDCR1, sdcr1_val);
	HAL_WRITE_UINT32(MCU_SBSR, sbsr_val);
	HAL_WRITE_UINT32(MCU_S32SR, S32SR_MEG(CYGSEM_HAL_ARM_IOP34X_32BIT_ECC_REGION_SIZE));
	
	
	// Read-back certain MCU registers after writing, but before
	// accessing memory per C-Spec
	HAL_READ_UINT32(MCU_SDCR0, scratch);
	HAL_READ_UINT32(MCU_SDCR1, scratch);
	HAL_READ_UINT32(MCU_SDBR, scratch);
	HAL_READ_UINT32(MCU_SDUBR, scratch);
	HAL_READ_UINT32(MCU_SBSR, scratch);
	HAL_READ_UINT32(MCU_S32SR, scratch);
	HAL_READ_UINT32(MCU_DLLRCVER, scratch);

	
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDBR);
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDUBR);
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDCR0);
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_SDCR1);
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_SBSR);
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_S32SR);

	
	
	hal_platform_lowlevel_serial_debug_put_value32(tRP);
	hal_platform_lowlevel_serial_debug_put_value32(tCAS);
	hal_platform_lowlevel_serial_debug_put_value32(tRCD);
	hal_platform_lowlevel_serial_debug_put_value32(tRAS);
	hal_platform_lowlevel_serial_debug_put_value32(tRC);
	hal_platform_lowlevel_serial_debug_put_value32(tWTR);
	hal_platform_lowlevel_serial_debug_put_value32(tRFC);
	hal_platform_lowlevel_serial_debug_put_value32(tREG);
	hal_platform_lowlevel_serial_debug_put_value32(tWR);
	hal_platform_lowlevel_serial_debug_put_value32(tRTP);
	hal_platform_lowlevel_serial_debug_put_value32(tEDP);
	
	
	HEX_DISPLAY_QUICK(2, 1);
	
	// Clear any pending MCU interrupts
	HAL_WRITE_UINT32(MCU_DMCISR, 0x30F);
	
	// SDRAM Initialization Sequence as specified by the JEDEC Spec
	// Steps are numbered based on JESD79-2 (DDR2 Spec)
	// STEPs 1-2 of JEDEC are done in Hardware up to this point
	HAL_WRITE_UINT32(MCU_RFR, 0);	// Disable the refresh counter
	
	// Call delay_mclks with either MClk delay desired or (delay time desired / 5nS )
	// Step 3 : Issue NOP cycle
	delay_mclks(40000);                                             // 200us for clocks to stabilize
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_NOP);                       // One NOP cycle
	HAL_READ_UINT32(MCU_SDIR, temp);
	
	
	// Step 4 : Issue Precharge all
	delay_mclks(80);                                                // Delay for 400nS minimum
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_PRECHARGE_ALL);             // Precharge-All command
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(tRP);                                               // Delay for tRP clocks
	
	
	// Step 5 : EMRS(2) programming
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_EMRS2_PROGRAM_TO_0);        // Program EMRS(2) to 0
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(10);                                                // Delay for tMRD (Min of 2)
	
	
	// Step 6 : EMRS(3) programming
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_EMRS3_PROGRAM_TO_0);        // Program EMRS(3) to 0
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(10);                                                // Delay for tMRD (Min of 2)
	
	
	// Step 7 : EMRS to enable DLL - make sure RTT field is written as well
	//          DQS enabled, RDQS disabled, Additive latency = 0, OutBuff enabled
	switch (sdcr0_val & SDCR_ODT_TERMINATION_MASK) {
		case SDCR_ODT_TERMINATION_DISABLED:
			emrs = SDIR_CMD_EMRS_DLL_ENABLED_NO_RTT;
			break;
		case SDCR_ODT_TERMINATION_75_OHM:
			emrs = SDIR_CMD_EMRS_DLL_ENABLED_75_OHM_RTT;
			break;
		case SDCR_ODT_TERMINATION_150_OHM:
			emrs = SDIR_CMD_EMRS_DLL_ENABLED_150_OHM_RTT;
			break;
	}
	HAL_WRITE_UINT32(MCU_SDIR, emrs);                               // EMRS to enable the DLL
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(10);                                                // Delay for tMRD (Min of 2)
	
	// Step 8 : MRS to reset DLL -> Setting operating parameters too
	if (tCAS == 3) {
		if (tWR == 3)
			mrs = SDIR_CMD_MRS_DLL_IS_RESET_CAS3_WR3;
		else
			mrs = SDIR_CMD_MRS_DLL_IS_RESET_CAS3_WR4;
	}
	else if (tCAS == 4) {
		if (tWR == 3)
			mrs = SDIR_CMD_MRS_DLL_IS_RESET_CAS4_WR3;
		else
			mrs = SDIR_CMD_MRS_DLL_IS_RESET_CAS4_WR4;
	}
	else if (tCAS == 5) {
		if (tWR == 3)
			mrs = SDIR_CMD_MRS_DLL_IS_RESET_CAS5_WR3;
		else
			mrs = SDIR_CMD_MRS_DLL_IS_RESET_CAS5_WR4;
	}
	HAL_WRITE_UINT32(MCU_SDIR, mrs);	// MRS to program the SDRAM parameters and DLL reset
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(10);			// Delay for tMRD (Min of 2)
	
	
	// Step 9 : Precharge All Command
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_PRECHARGE_ALL);	// Precharge-All command
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(SDCR_tRP_MAX + 100);			// Delay for Trp - no max spec'd

	
	// Step 10 : Minimum of two auto refresh cycles
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_AUTO_REFRESH);	// 1st of 2 required Auto Refresh Cycles
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(100);					// Delay at least tRFC cycles
	HAL_WRITE_UINT32(MCU_SDIR, SDIR_CMD_AUTO_REFRESH);	// 2nd of 2 required Auto Refresh Cycles
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(100);					// Delay at least tRFC cycles

	
	// Step 11 : MRS with DLL Not reset and operating parameters
	mrs &= ~(MRS_DLL_RESET);				// Clear DLL Reset Bit from Mode Register
	HAL_WRITE_UINT32(MCU_SDIR, mrs);			// MRS to program the SDRAM parameters and no DLL reset
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(300);					// Delay for Tmrd cycles -- 2 mclk

	

	// Step 12 : OCD Calibration - Set Default and then Exit command
	// Must be 200 clocks after step 8 (DLL Reset)
	delay_mclks(300);
	emrs |= SDIR_CMD_EMRS_OCD_DEFAULT;
	HAL_WRITE_UINT32(MCU_SDIR, emrs);			// EMRS OCD Default Command
	HAL_READ_UINT32(MCU_SDIR, temp);

	delay_mclks(300);
	emrs &= ~(SDIR_CMD_EMRS_OCD_DEFAULT);
	HAL_WRITE_UINT32(MCU_SDIR, emrs);			// EMRS OCD Exit Command
	HAL_READ_UINT32(MCU_SDIR, temp);
	delay_mclks(300);

	
	// If Delay is done Here - failing UNBUFFERED Dimms behave and start passing
	delay_mclks(188000);	// Works (equates to ~980uSec delay between DLL reset and refresh)

	// Step 13 : DDR2 is now ready for normal operation
	HAL_WRITE_UINT32(MCU_RFR, refresh);			// Re-enable the refresh counter.
	
	HEX_DISPLAY_QUICK(2, 2);

	HAL_WRITE_UINT32(MCU_DLLR4, DLLR4_B0_VALUE);
	
	hal_platform_lowlevel_serial_debug_put_value32(*ecc_cap);
	// B0 silicon samples DQS[8] which is only present on ECC DIMMs
	// If we aren't using a ECC DIMM we need to revert to using a table		
	if(*ecc_cap) 
	{
		int coarse, fine, lookfor, direction, dqs, dllvalue;
		
		dllvalue = 0x50; // Start value
		
		if(sampleDQS(dllvalue,tREG))
		{
			lookfor = 0;
			direction = -1;
		}
		else
		{
			lookfor = 1;
			direction = 1;
		}
		
		do
		{
			dllvalue += direction;
			
			if ((dllvalue == 0) || (dllvalue == 0xFF))
				MEM_PARAM_ERROR(6, 0);
			
			dqs = sampleDQS(dllvalue,tREG);
		}
		while(lookfor != dqs);
		
				
		if(dllvalue >= 0x30)
		{
			if(sampleDQS((dllvalue - 0x30),tREG))
				dllvalue -= 0x40;
		}
		
		
		// We want the DLLRCVER value to be 1/4 mclk (16 elements) before the detected edge
		dllvalue -= 0x10;
		
		// Make sure we don't go below zero
		if(dllvalue < 0)
			dllvalue = 0;
				
		fine = (dllvalue & 0x1f);
		coarse = ((dllvalue >> 5) & 0x7);
		
		HAL_WRITE_UINT32(MCU_DLLRCVER,((tREG << 16) | (coarse << 8) | fine));
		
		/* Reset the MCU FIFOs */
		HAL_READ_UINT32 (MCU_SDCR0, scratch);
		scratch &= 0xFFFFFF7F;			// Clear Reset FIFO bit
		HAL_WRITE_UINT32(MCU_SDCR0, scratch);
		scratch |= (1 << 7);
		HAL_WRITE_UINT32(MCU_SDCR0, scratch);  // Set Reset FIFO bit
	}
	else  //B0 but no ECC, use the values hardcoded values determined by the above algorithm
	{
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
		if ((*IQ8134X_PRODUCT_CODE & IQ8134X_PROD_SKU_MASK) ==  IQ8134X_PROD_CODE_IQ8134xSC)
			if (ps_per_cycle == 5000)
				HAL_WRITE_UINT32(MCU_DLLRCVER, (0x0011E | (tREG << 16)));	// SC 400Mhz
			else 
				HAL_WRITE_UINT32(MCU_DLLRCVER, (0x00206 | (tREG << 16)));	// SC 533Mhz
		else
			HAL_WRITE_UINT32(MCU_DLLRCVER, (0x00109 | (tREG << 16)));	// MC 400Mhz and 533Mhz
#else
		HAL_WRITE_UINT32(MCU_DLLRCVER, (0x00109 | (tREG << 16)));	// Volant 400Mhz and 533Mhz
#endif
	}
	hal_platform_lowlevel_serial_debug_put_value32(*MCU_DLLRCVER);

	*sdram_installed = true;
}


int sampleDQS(int dllval, int reg)
{
	int fine,coarse,scratch;
	volatile unsigned int * readAddr = (unsigned int *)0xA0040000;
	
	fine = (dllval & 0x1f);
	coarse = ((dllval >> 5) & 0x7);
	
	/*	
	hal_platform_lowlevel_serial_debug_puts("coarse:");
	hal_platform_lowlevel_serial_debug_putn8(coarse);
	hal_platform_lowlevel_serial_debug_puts(",fine:");
	hal_platform_lowlevel_serial_debug_putn8(fine);
	hal_platform_lowlevel_serial_debug_puts(" ( ");
	hal_platform_lowlevel_serial_debug_putn32(((1<<17) | (1<<18) | (reg<<16) | (coarse << 8) | fine));
	*/
	
	HAL_WRITE_UINT32(MCU_DLLRCVER,((1<<17) | (1<<18) | (reg<<16) | (coarse << 8) | fine));
	HAL_READ_UINT32(MCU_DLLRCVER,scratch);
	
	
	// We need to do the reads in assembler to make sure that the same register is used for both.
	// This creates a data dependency that guarantees that the second read is not launched until
	// the first read completes.
	asm volatile (
			"   ldr	%[scratch], [%[readAddr]]\n"
			"   ldr	%[scratch], [%[dllrcvr]]"
			: /* output */
			: /* input */ [scratch] "r" (scratch), [dllrcvr] "r" (MCU_DLLRCVER), [readAddr] "r" (readAddr));
	
	
	if(scratch & (1<<24))
	{
		// hal_platform_lowlevel_serial_debug_puts("=1\n\r");
		return 1;
	}
	else
	{
		// hal_platform_lowlevel_serial_debug_puts("=0\n\r");
		return 0;
	}
}
	




//
// Perform Memory Scrub
//
void mcu_memory_scrub(cyg_uint32 base, cyg_uint32 size)
{
#ifdef CYGSEM_HAL_ARM_IQ8134X_BATTERY_TEST
    cyg_uint32 battery_back_test_value;
#endif
#ifdef CYGSEM_HAL_ARM_IQ8134x_USE_ADMA_SCRUB
	cyg_uint32 adma_size, adma_base, desc_base;
	cyg_uint32 inval_size, inval_base;

	typedef struct  {
		cyg_uint32 nda;
		cyg_uint32 dc;
		cyg_uint32 block_fill;
		cyg_uint32 bc;
		cyg_uint32 dst_l;
		cyg_uint32 dst_u;
		cyg_uint32 src_l;
		cyg_uint32 src_u;
	} ADMA_DESC_T;
	ADMA_DESC_T *p_desc;

	// Only use ADMA to scrub >1M
	// and scrub base to 1M with core for descriptor space
	if (size > SZ_1M) {
		adma_size = inval_size = size - SZ_1M;
		size = SZ_1M;
		adma_base = inval_base = base + SZ_1M;
	}
	else
		adma_size = 0;
#endif

	// Scrub "A" for scrubbing ADMA Descriptor Region
    HEX_DISPLAY_QUICK(S, A);
    hal_platform_lowlevel_serial_debug_puts("In mcu_memory_scrub()\r\n");

    // clear test register
    HAL_WRITE_UINT32(MCU_DECTST, 0);

    // disable ECC, disable reporting
    HAL_WRITE_UINT32(MCU_DECCR, 0);

#ifdef CYGSEM_HAL_ARM_IQ8134X_BATTERY_TEST
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

#ifdef CYGSEM_HAL_ARM_IQ8134x_USE_ADMA_SCRUB
	// If we are scrubbing > 1M, then adma_size will be non-zero and we'll use ADMA
	if (adma_size != 0) {
		cyg_uint32 temp;

		// Clear Enable Bit
		*ACCR0  &= ~(ACR_ENABLE);

		// Setup first descriptor to be offset from 0 and aligned to 32-bytes
		desc_base = base + 0x20;
		desc_base &= ~(0x1F);

		// Point to where first descriptor will be
		*ANDAR0 = desc_base;

		// Setup 15M Descriptors until all memory is setup to be scrubbed
		while (adma_size != 0) {
			// Align 1st descriptor to 32-byte offset and away from 0x0
			p_desc = (ADMA_DESC_T*)desc_base;
			// Data to fill
			p_desc->block_fill = 0x0;
			// Destination
			p_desc->dst_l = adma_base;
			p_desc->dst_u = 0;
			// Sources are zeroed since it's a block-fill
			p_desc->src_l = 0;
			p_desc->src_u = 0;
			// Descriptor Control : Block Fill | InternalBus -> Local Memory XFer
			p_desc->dc    = (DC_MEMORY_BLOCK_FILL | DC_INTERNAL_TO_LOCAL);

			// Set BC and check if we're the last descriptor
			if (adma_size > (SZ_15M)) {
				adma_size -= SZ_15M;
				adma_base += SZ_15M;
				desc_base += sizeof(ADMA_DESC_T);
				p_desc->nda = desc_base;
				p_desc->bc = SZ_15M;
			}
			// Else we have < 15M to transfer and will be the last descriptor
			else  {
				p_desc->bc = adma_size;
				adma_size = 0;
				p_desc->nda = 0;
			}
		}

		// Descriptors are setup in SDRAM - we can enable the ADMA, but first check for any errors
		if (*ACSR0 & ASR_ERROR_MASK)
			MEM_PARAM_ERROR(S, 1);

		// Display SL for Scrub Loop on HEX LEDs
		HEX_DISPLAY_QUICK(S, L);

	    // Flush out descriptors to RAM
		HAL_DCACHE_SYNC();
		HAL_L2_CACHE_SYNC();

		// Start the engine
		*ACCR0 |= ACR_ENABLE;

		// Make sure Engine has started
		HAL_READ_UINT32(ACCR0, temp);

		// While ADMA is working - just wait
		while (*ACSR0 & ASR_ACTIVE);

		// Clear Enable Bit in Channel
		*ACCR0  &= ~(ACR_ENABLE);

		// Clean up our descriptor area so memory is truly all 0x0s
		mcu_memory_scrub(base, SZ_1M);

	} // If adma_size != 0
#endif

#ifdef CYGSEM_HAL_ARM_IQ8134X_BATTERY_TEST
    HAL_WRITE_UINT32(SDRAM_BATTERY_TEST_ADDR, battery_back_test_value);
    HAL_DCACHE_STORE(SDRAM_BATTERY_TEST_ADDR, sizeof(battery_back_test_value));
	HAL_L2_CACHE_STORE(SDRAM_BATTERY_TEST_ADDR, sizeof(battery_back_test_value));
#endif
    HEX_DISPLAY_QUICK(S, E);
}

//
// Enable ECC
//
void mcu_enable_ecc(void)
{
    // Enable ECC and all reporting
    HAL_WRITE_UINT32(MCU_DECCR, (ECCR_SINGLE_BIT_CORRECT_ENABLE | ECCR_SINGLE_BIT_REPORT_ENABLE | \
                                 ECCR_MULTI_BIT_REPORT_ENABLE | ECCR_ECC_ENABLE));
}

// Used to count Mclks during JEDEC init sequence


