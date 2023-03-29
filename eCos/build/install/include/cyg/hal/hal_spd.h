#ifndef CYGONCE_HAL_HAL_SPD_H
#define CYGONCE_HAL_HAL_SPD_H

//=============================================================================
//
//      hal_spd.h
//
//      HAL header for SDRAM Serial Presence Detect support.
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
// Author(s):   msalter
// Contributors:msalter, dmoseley
// Date:        2002-01-17
// Purpose:     Generic HAL SPD header.
// Usage:       #include <cyg/hal/hal_spd.h>
// Description: This header provides byte numbers and bit definitions for
//              serial EEPROM containing SDRAM module information.
//                           
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//=============================================================================

// Commonly used bytes
#define SPD_MEMTYPE         2   // fundamental memory type
#define SPD_NUM_ROWS        3   // number of row addresses
#define SPD_NUM_COLS        4   // number of column addresses
#define	SPD_BANKCNT	    5	// number of module banks
#define	SPD_MOD_WIDTH1	    6	// module data width (byte 1)
#define	SPD_MOD_WIDTH2	    7	// module data width (byte 2)
#define SPD_CYCLE_TIME		9   // minimum cycle time;  High nibble=nS, Low=.1nS
#define	SPD_CONFIG	   11	// DIMM configuration type (Parity or not, EEC)
#define	SPD_REFRESH        12 	// Refresh rate
#define	SPD_SDRAM_WIDTH    13 	// DRAM width
#define	SPD_TCAS           18 	// tCAS Latency
#define	SPD_DIMM_TYPE      20 	// DIMM Type.  (bit0=Registered;bit1=Unbuffered)
#define	SPD_MOD_ATTRIB     21 	// DRAM module attribute
#define	SPD_TRP		   27	// minimum row precharge time
#define	SPD_TRCD	   29	// minimum RAS to CAS delay
#define	SPD_TRAS	   30	// minimum Active to Precharge time
#define	SPD_BANKSZ	   31	// module bank density
#define	SPD_TWR        36   // Write recovery time  
#define	SPD_TWTR	   37	// internal write to read command delay
#define SPD_TRTP       38   // Minimum refresh to active/refresh command period
#define	SPD_TRFC_TRC_EXT   40	// tRFC and tRC extension byte
#define	SPD_TRC		   41	// minimum active-to-active/auto refresh time
#define	SPD_TRFC           42	// minimum auto refresh-to-active/auto refresh time LSB
#define	SPD_CHECKSUM	   63 	// checksum for bytes 0-62

// SPD_MEMTYPE bits
#define SPD_MEMTYPE_SDRAM_DDR   0x7
#define SPD_MEMTYPE_SDRAM_DDRII 0x8

// SPD_BANKCNT bits
#define SPD_DDRII_BANKCNT_MASK  0x7

// SPD_CYCLE_TIME definitions
#define SPD_CYCLE_266MHZ   0x3D
#define SPD_CYCLE_200MHZ   0x50
// SPD_CONFIG bits
#define SPD_CONFIG_NONE    0x00
#define SPD_CONFIG_PARITY  0x01
#define SPD_CONFIG_ECC     0x02

// SPD_REFRESH_BITS
#define SPD_RFR_MASK       0x7F
#define SPD_RFR_15_625us   0x0
#define SPD_RFR_3_9us      0x1
#define SPD_RFR_7_8us      0x2
#define SPD_RFR_31_3us     0x3
#define SPD_RFR_62_5us     0x4
#define SPD_RFR_125us      0x5

// SPD_DIMM_TYPE
#define SPD_REGISTERED     0x01
#define SPD_UNBUFFERED     0x02

// SPD_MOD_ATTRIB bits
#define SPD_ATTRIB_BUF_CTL 0x01 // Buffered Addr/Control inputs
#define SPD_ATTRIB_REG_CTL 0x02 // Registered Addr/Control inputs
#define SPD_ATTRIB_PLL     0x04 // On-card PLL
#define SPD_ATTRIB_BUF_DQ  0x08 // Buffered DQMB inputs
#define SPD_ATTRIB_REG_DQ  0x10 // Registered DQMB inputs
#define SPD_ATTRIB_DIFF    0x20 // Differential clock input
#define SPD_ATTRIB_RRA     0x40 // Redundant Row Address

// SPD_BANKSZ bits
#define SPD_DDR_BANKSZ_512MB 0x80
#define SPD_DDR_BANKSZ_256MB 0x40
#define SPD_DDR_BANKSZ_128MB 0x20
#define SPD_DDR_BANKSZ_64MB  0x10
#define SPD_DDR_BANKSZ_32MB  0x08
#define SPD_DDR_BANKSZ_16MB  0x04
#define SPD_DDR_BANKSZ_2GB   0x02
#define SPD_DDR_BANKSZ_1GB   0x01

#define SPD_DDRII_BANKSZ_512MB 0x80
#define SPD_DDRII_BANKSZ_256MB 0x40
#define SPD_DDRII_BANKSZ_128MB 0x20
#define SPD_DDRII_BANKSZ_16GB  0x10
#define SPD_DDRII_BANKSZ_8GB   0x08
#define SPD_DDRII_BANKSZ_4GB   0x04
#define SPD_DDRII_BANKSZ_2GB   0x02
#define SPD_DDRII_BANKSZ_1GB   0x01

// SPD_tCAS bits
#define SPD_DDR_tCAS_LAT_1     0x01
#define SPD_DDR_tCAS_LAT_1_5   0x02
#define SPD_DDR_tCAS_LAT_2     0x04
#define SPD_DDR_tCAS_LAT_2_5   0x08
#define SPD_DDR_tCAS_LAT_3     0x10
#define SPD_DDR_tCAS_LAT_3_5   0x20

#define SPD_DDRII_tCAS_LAT_2   0x04
#define SPD_DDRII_tCAS_LAT_3   0x08
#define SPD_DDRII_tCAS_LAT_4   0x10
#define SPD_DDRII_tCAS_LAT_5   0x20

// SPD_tRP bits
#define SPD_tRP_ns(sec,qrt)    ((sec) << 2 | (qrt))
#define SPD_tRP_to_ns(tRP)     ((tRP) >> 2)
#define SPD_tRP_to_ps(tRP)     (((tRP) >> 2)*1000)

// SPD_tWTR bits
#define SPD_tWTR_ns(sec,qrt)   ((sec) << 2 | (qrt))
#define SPD_tWTR_to_ns(tWTR)   ((tWTR) >> 2)
#define SPD_tWTR_to_ps(tWTR)   (((tWTR) >> 2)*1000)

// SPD_tRCD bits
#define SPD_tRCD_ns(sec,qrt)   ((sec) << 2 | (qrt))
#define SPD_tRCD_to_ns(tRCD)   ((tRCD) >> 2)
#define SPD_tRCD_to_ps(tRCD)   (((tRCD) >> 2)*1000)

// SPD_tRAS bits
#define SPD_tRAS_ns(sec,qrt)   (sec)
#define SPD_tRAS_to_ns(tRAS)   (tRAS)
#define SPD_tRAS_to_ps(tRAS)   (tRAS*1000)

// SPD_tRC bits
#define SPD_tRC_ns(sec,qrt)    (sec)
#define SPD_tRC_to_ns(tRC)     (tRC)
#define SPD_tRC_to_ps(tRC)     (tRC*1000)

// SPD_tRFC bits
#define SPD_tRFC_ns(sec,qrt)   (sec)
#define SPD_tRFC_to_ns(tRFC)   (tRFC)
#define SPD_tRFC_to_ps(tRFC)   (tRFC*1000)

// SPD_tWR bits
#define SPD_tWR_to_ns(tWR)     ((tWR) >> 2)
#define SPD_tWR_to_ps(tWR)     (((tWR) >> 2)*1000)

// SPD_tRTP bits
#define SPD_tRTP_to_ps(tRTP)   (((tRTP) >> 2)*1000)
//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_SPD_H
// End of hal_spd.h
