//==========================================================================
//
//      xscale_iop_redboot.c
//
//      HAL RedBoot command extensions for Intel XScale I/O Processors.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003, 2004, 2005 Red Hat, Inc.
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    drew.moseley@intel.com
// Contributors:
// Date:         2003-09-17
// Purpose:      XScale RedBoot Extensions
// Description:  Additional RedBoot Command for XScale I/O Processors
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_xscale.h>
#include <cyg/hal/hal_xscale_iop.h>
#include <cyg/hal/var_io.h>

#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT) && !defined(HAL_PCI_MINIMAL_INIT)
#define HAL_PCI_MINIMAL_INIT()
#endif

#ifdef CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_I2C_API
#include <cyg/hal/hal_spd.h>

static void
do_spd(int argc, char *argv[])
{
    #define SPD_NUM_BYTES 64
    char i2c_bytes[SPD_NUM_BYTES], cksum = 0, buf;
    int i;

    i2c_init();

    // Write 0 to the SDRAM SPD to reset the EEPROM pointer
    buf = 0;
    if (i2c_write_bytes(SDRAM_DEVID, &buf, 1) == -1) {
        diag_printf("Error timeout writing to I2C device\n");
        return;
    }

    // Read the bytes
    if (i2c_read_bytes(SDRAM_DEVID, i2c_bytes, SPD_NUM_BYTES) == -1) {
        diag_printf("Error timeout scanning SPD\n");
        return;
    }

    i = 0;
    while (i < SPD_NUM_BYTES)
    {
        diag_printf("0x%02x: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    i,
                    i2c_bytes[i+0],  i2c_bytes[i+1],  i2c_bytes[i+2],  i2c_bytes[i+3],
                    i2c_bytes[i+4],  i2c_bytes[i+5],  i2c_bytes[i+6],  i2c_bytes[i+7]);
        i+=8;
    }
    for (i = 0; i < SPD_NUM_BYTES; i++)
        if (i != SPD_CHECKSUM)
            cksum += i2c_bytes[i];

    if (cksum == i2c_bytes[SPD_CHECKSUM])
        diag_printf("Checksum test passed: 0x%02x\n", cksum);
    else
        diag_printf("Checksum test failed: calculated: 0x%02x, expected: 0x%02x\n", cksum, i2c_bytes[SPD_CHECKSUM]);
}

RedBoot_cmd("spd",
            "Dump SDRAM SPD Contents",
            "",
            do_spd
    );

static void
do_spdw(int argc, char *argv[])
{
    struct option_info opts[2];
    unsigned addr, pattern;
    unsigned char buf[2];
    bool addr_set, pattern_set;

    init_opts(&opts[0], 'a', true, OPTION_ARG_TYPE_NUM,
              (void **)&addr, (bool *)&addr_set, "address");
    init_opts(&opts[1], 'p', true, OPTION_ARG_TYPE_NUM,
              (void **)&pattern, (bool *)&pattern_set, "pattern");
    if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
        return;
    }
    if (!addr_set || !pattern_set) {
        diag_printf("usage: spdw -a <address> -p <pattern>\n");
        return;
    }
    buf[0] = (unsigned char)(addr & 0xff);
    buf[1] = (unsigned char)(pattern & 0xff);
    i2c_write_bytes(SDRAM_DEVID, &buf[0], 2);
}

RedBoot_cmd("spdw",
            "Write to SDRAM SPD",
            "-a <address> -p <pattern>",
            do_spdw
    );
#endif // CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_I2C_API

static void
do_cpsr(int argc, char *argv[])
{
    register unsigned _val_;
    struct option_info opts[2];
	bool enable_ints, disable_ints;

    init_opts(&opts[0], 'e', false, OPTION_ARG_TYPE_FLG,
              (void *)&enable_ints, (bool *)0, "Enable Interrupts");
    init_opts(&opts[1], 'd', false, OPTION_ARG_TYPE_FLG,
              (void *)&disable_ints, (bool *)0, "Disable Interrupts");

    if (!scan_opts(argc, argv, 1, opts, 2, 0, 0, "")) {
        return;
    }
if (enable_ints && disable_ints) {
		diag_printf ("Can't enable and disable - Choose one\n");
	}
	else if (enable_ints) {
		diag_printf ("Enabling FIQ/IRQ\n");
	    asm volatile("mrs\t%0, cpsr" : "=r" (_val_));
    	asm volatile("msr\tcpsr, %0" : : "r" (_val_ & ~(CPSR_IRQ_DISABLE | CPSR_FIQ_DISABLE)));
	} 
	else if (disable_ints) { 	 
		diag_printf ("Disabling FIQ/IRQ\n");
	    asm volatile("mrs\t%0, cpsr" : "=r" (_val_));
    	asm volatile("msr\tcpsr, %0" : : "r" (_val_ | (CPSR_IRQ_DISABLE | CPSR_FIQ_DISABLE)));
	}
    asm volatile("mrs\t%0, cpsr" : "=r" (_val_));
    diag_printf("cpsr = 0x%08lx\n", _val_);
}

RedBoot_cmd("cpsr",
            "Dump cpsr",
            "-e (enable interrupts) -d (disable interrupts)",
            do_cpsr
    );

static void
do_dcsr(int argc, char *argv[])
{
    register unsigned _val_;
    struct option_info opts[4];
    bool set_enable_hot_debug;
    bool set_clear_hot_debug;
    bool set_trace_on;
    bool set_trace_off;

    init_opts(&opts[0], 'h', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_enable_hot_debug, (bool *)0, "Enable Hot Debug");
    init_opts(&opts[1], 'c', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_clear_hot_debug, (bool *)0, "Clear Hot Debug");
    init_opts(&opts[2], 't', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_trace_on, (bool *)0, "Turn Trace Buffer on");
    init_opts(&opts[3], 'n', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_trace_off, (bool *)0, "Turn Trace Buffer off");
    if (!scan_opts(argc, argv, 1, opts, 4, 0, 0, "")) {
        return;
    }

    if (set_enable_hot_debug)
      {
        //asm volatile("mrc\tp14, 0, %0, c10, c0, 0" : "=r" (_val_));
        _val_ = 0x8000001c;
         asm volatile("mcr\tp14, 0, %0, c10, c0, 0" : : "r" (_val_));
      }
    if (set_clear_hot_debug)
      {
        //asm volatile("mrc\tp14, 0, %0, c10, c0, 0" : "=r" (_val_));
        _val_ = 0x80000000;
         asm volatile("mcr\tp14, 0, %0, c10, c0, 0" : : "r" (_val_));
      }
    if (set_trace_on)
      {
        asm volatile("mrc\tp14, 0, %0, c10, c0, 0" :   "=r" (_val_));
        _val_ |= 0x00000001;
        asm volatile("mcr\tp14, 0, %0, c10, c0, 0" : : "r" (_val_));
      }
    if (set_trace_off)
      {
        asm volatile("mrc\tp14, 0, %0, c10, c0, 0" :   "=r" (_val_));
        _val_ &= ~0x00000001;
        asm volatile("mcr\tp14, 0, %0, c10, c0, 0" : : "r" (_val_));
      }

    asm volatile("mrc\tp14, 0, %0, c10, c0, 0" : "=r" (_val_));
    diag_printf("dcsr = 0x%08lx\n", _val_);
}

RedBoot_cmd("dcsr",
            "Dump dcsr",
            "[-h] Hot Debug on;  [-c] off\n   "
            "[-t] Trace     on;  [-n] off",
            do_dcsr
    );



#if defined(CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_PCI_REDBOOT_FNS)
#if defined(CYGPKG_IO_PCI) || defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)

#include <cyg/io/pci_hw.h>

static void
do_pcr(int argc, char *argv[])
{
    struct option_info opts[8];
    unsigned bus, device, function, offset, length;
    bool bus_set, device_set, function_set, offset_set, length_set;
    bool set_32bit, set_16bit, set_8bit;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&bus, (bool *)&bus_set, "bus number");
    init_opts(&opts[1], 'd', true, OPTION_ARG_TYPE_NUM,
              (void **)&device, (bool *)&device_set, "device number");
    init_opts(&opts[2], 'f', true, OPTION_ARG_TYPE_NUM,
              (void **)&function, (bool *)&function_set, "function number");
    init_opts(&opts[3], 'o', true, OPTION_ARG_TYPE_NUM,
              (void **)&offset, (bool *)&offset_set, "offset");
    init_opts(&opts[4], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[5], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[6], '2', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[7], '1', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_8bit, (bool *)0, "fill 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 8, 0, 0, "")) {
        return;
    }
    if (!bus_set || !device_set || !function_set || !offset_set) {
        diag_printf("usage: pcr -b <bus> -d <device> -f <function> -o <offset> [-l <length>] [-1|-2|-4]\n");
        return;
    }
    if (!length_set) {
        length = 4;
    }
    if (set_16bit) {
        offset &= ~0x1;
    } else if (set_32bit) {
        offset &= ~0x3;
    }

#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
    // If we are not building a full RedBoot then it is unlikely that the
    // ATU has been configured yet.
    // Do a minimal configuration here.
    HAL_PCI_MINIMAL_INIT();
#endif

    {
        unsigned end_offset = offset + length;
        int num_this_line = 0;

        if (end_offset > 4096)
            end_offset = 4096;

        while (offset < end_offset)
        {
            if (num_this_line == 0)
                diag_printf("0x%03x: ", offset);

            if (set_8bit) {
                cyg_uint8 val;
                HAL_PCI_CFG_READ_UINT8(bus,
                                       CYG_PCI_DEV_MAKE_DEVFN(device, function),
                                       offset,
                                       val);
                diag_printf("%02x ", val);
                offset += 1;
                num_this_line += 1;
            } else if (set_16bit) {
                cyg_uint16 val;
                HAL_PCI_CFG_READ_UINT16(bus,
                                        CYG_PCI_DEV_MAKE_DEVFN(device, function),
                                        offset,
                                        val);
                diag_printf("%04x ", val);
                offset += 2;
                num_this_line += 2;
            } else {
                cyg_uint32 val;
                HAL_PCI_CFG_READ_UINT32(bus,
                                        CYG_PCI_DEV_MAKE_DEVFN(device, function),
                                        offset,
                                        val);
                diag_printf("%08x ", val);
                offset += 4;
                num_this_line += 4;
            }

            if (num_this_line == 16)
            {
                diag_printf("\n");
                num_this_line = 0;
            }
        }
        if (num_this_line != 0)
            diag_printf("\n");
    }
}

RedBoot_cmd("pcr",
            "Issue a PCI Config Read",
            "-b <bus> -d <device> -f <function> -o <offset> [-l <length>] [-1|-2|-4]",
            do_pcr
    );

static void
do_pcw(int argc, char *argv[])
{
    struct option_info opts[9];
    unsigned bus, device, function, offset, pattern, length;
    bool bus_set, device_set, function_set, offset_set, pattern_set, length_set;
    bool set_32bit, set_16bit, set_8bit;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&bus, (bool *)&bus_set, "bus number");
    init_opts(&opts[1], 'd', true, OPTION_ARG_TYPE_NUM,
              (void **)&device, (bool *)&device_set, "device number");
    init_opts(&opts[2], 'f', true, OPTION_ARG_TYPE_NUM,
              (void **)&function, (bool *)&function_set, "function number");
    init_opts(&opts[3], 'o', true, OPTION_ARG_TYPE_NUM,
              (void **)&offset, (bool *)&offset_set, "offset");
    init_opts(&opts[4], 'p', true, OPTION_ARG_TYPE_NUM,
              (void **)&pattern, (bool *)&pattern_set, "pattern");
    init_opts(&opts[5], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[6], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[7], '2', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[8], '1', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_8bit, (bool *)0, "fill 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 9, 0, 0, "")) {
        return;
    }
    if (!bus_set || !device_set || !function_set || !offset_set || !pattern_set) {
        diag_printf("usage: pcw -b <bus> -d <device> -f <function> -o <offset> -p <pattern> [-l <length>] [-1|-2|-4]\n");
        return;
    }
    if (!length_set) {
        if (set_8bit)
            length = 1;
        else if (set_16bit)
            length = 2;
        else
            length = 4;
    }
    if (set_16bit) {
        offset &= ~0x1;
    } else if (set_32bit) {
        offset &= ~0x3;
    }


#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
    // If we are not building a full RedBoot then it is unlikely that the
    // ATU has been configured yet.
    // Do a minimal configuration here.
    HAL_PCI_MINIMAL_INIT();
#endif

    {
        unsigned int end_offset = offset + length;

        if (end_offset > 4096)
            end_offset = 4096;

        while (offset < end_offset)
        {
            if (set_8bit) {
                cyg_uint8 val = (cyg_uint8)(pattern & 0xff);
                HAL_PCI_CFG_WRITE_UINT8(bus,
                                        CYG_PCI_DEV_MAKE_DEVFN(device, function),
                                        offset,
                                        val);
                offset += 1;
            } else if (set_16bit) {
                cyg_uint16 val = (cyg_uint16)(pattern & 0xffff);
                HAL_PCI_CFG_WRITE_UINT16(bus,
                                         CYG_PCI_DEV_MAKE_DEVFN(device, function),
                                         offset,
                                         val);
                offset += 2;
            } else {
                cyg_uint32 val = (cyg_uint32)(pattern & 0xffffffff);
                HAL_PCI_CFG_WRITE_UINT32(bus,
                                         CYG_PCI_DEV_MAKE_DEVFN(device, function),
                                         offset,
                                         val);
                offset += 4;
            }
        }
    }
}

#ifndef ATU_OMWTVR0
#if defined(POMWVR_REG)
#define ATU_OMWTVR0 POMWVR_REG
#else
#error UNKNOWN VALUE FOR ATU_OMWTVR0
#endif
#endif

#ifndef ATU_OUMWTVR0
#if defined(PODWVR_REG)
#define ATU_OUMWTVR0 PODWVR_REG
#else
#error UNKNOWN VALUE FOR ATU_OUMWTVR0
#endif
#endif

#ifndef ATU_OIOWTVR
#if defined(POIOWVR_ADDR)
#define ATU_OIOWTVR POIOWVR_ADDR
#elif defined(ATUX_OIOWTVR)
#define ATU_OIOWTVR ATUX_OIOWTVR
#else
#error UNKNOWN VALUE FOR ATU_OUMWTVR0
#endif
#endif

RedBoot_cmd("pcw",
            "Issue a PCI Config Write",
            "-b <bus> -d <device> -f <function> -o <offset> -p <pattern> [-l <length>] [-1|-2|-4]",
            do_pcw
    );

static void
do_pmr(int argc, char *argv[])
{
    struct option_info opts[6];
    unsigned base_address, length;
    unsigned upper_base_address;
    bool base_address_set, base_address_set_1, length_set;
    bool set_32bit, set_16bit, set_8bit;
    char _size = 1;
    cyg_uint32 old_oumwtvr;
    cyg_uint32 old_omwtvr;

    init_opts(&opts[0], 'u', true, OPTION_ARG_TYPE_NUM,
              (void **)&upper_base_address, (bool *)&base_address_set, "base address");
    init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base_address, (bool *)&base_address_set_1, "base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[3], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[4], '2', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[5], '1', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_8bit, (bool *)0, "fill 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 6, 0, 0, "")) {
        return;
    }

    if (!base_address_set_1) {
        diag_printf("usage: pmr [-u <upper base address>] -b <lower base address> [-l <length>] [-1|-2|-4]\n");
        return;
    }
    if (!base_address_set) {
        upper_base_address = 0;
    }

    if (set_8bit) {
        _size = 1;
    } else if (set_16bit) {
        _size = 2;
        base_address &= ~0x1;
    } else if (set_32bit) {
        _size = 4;
        base_address &= ~0x3;
    } else {
        // Default to 32-bit
        _size = 4;
        base_address &= ~0x3;
    }

    if (!length_set) {
        length = _size;
    }

#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
    // If we are not building a full RedBoot then it is unlikely that the
    // ATU has been configured yet.
    // Do a minimal configuration here.
    HAL_PCI_MINIMAL_INIT();
#endif

    HAL_READ_UINT32(ATU_OUMWTVR0, old_oumwtvr);
#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using IOP ATU setup from Gen1-3
    HAL_READ_UINT32(ATU_OMWTVR0, old_omwtvr);
#endif
    while (length) {
        unsigned long base, dump_len;
        void *window_addr, *offset;

        base = base_address & ATU_OMWTV_MASK;
        window_addr = (void*)(PCI_OUTBOUND_MEM_WINDOW + base_address - base);
        offset = (void*)((unsigned long)window_addr - base_address);

        if (((unsigned long)window_addr + length) > PCI_OUTBOUND_MEM_WINDOW_TOP) {
            dump_len = PCI_OUTBOUND_MEM_WINDOW_TOP - (unsigned long)window_addr;
        } else {
            dump_len = length;
        }

#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using IOP ATU setup from Gen1-3
        HAL_WRITE_UINT32(ATU_OMWTVR0, base);
#endif
        HAL_WRITE_UINT32(ATU_OUMWTVR0, upper_base_address);

        switch( _size ) {
        case 1:
            diag_dump_buf_with_offset(window_addr, dump_len, offset);
            break;
        case 2:
            diag_dump_buf_with_offset_16bit(window_addr, dump_len, offset);
            break;
        case 4:
            diag_dump_buf_with_offset_32bit(window_addr, dump_len, offset);
            break;
        }

        length -= dump_len;
        base_address += dump_len;
    }

#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using IOP ATU setup from Gen1-3
    HAL_WRITE_UINT32(ATU_OMWTVR0, old_omwtvr);
#endif
    HAL_WRITE_UINT32(ATU_OUMWTVR0, old_oumwtvr);
}

RedBoot_cmd("pmr",
            "Issue a PCI Memory Read",
            " [-u <upper base address>] -b <base address> [-l <length>] [-1|-2|-4]",
            do_pmr
    );

static void
do_pmw(int argc, char *argv[])
{
    struct option_info opts[7];
    unsigned base_address, length, pattern;
    unsigned upper_base_address;
    bool base_address_set, length_set, pattern_set;
    bool base_address_set_1;
    bool set_32bit, set_16bit, set_8bit;
    char _size = 1;
    cyg_uint32 old_omwtvr;
    cyg_uint32 old_oumwtvr;

    init_opts(&opts[0], 'u', true, OPTION_ARG_TYPE_NUM,
              (void **)&upper_base_address, (bool *)&base_address_set, "upper base address");
    init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base_address, (bool *)&base_address_set_1, "base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[3], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[4], '2', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[5], '1', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_8bit, (bool *)0, "fill 8 bit units");
    init_opts(&opts[6], 'p', true, OPTION_ARG_TYPE_NUM,
              (void **)&pattern, (bool *)&pattern_set, "pattern");
    if (!scan_opts(argc, argv, 1, opts, 7, 0, 0, "")) {
        return;
    }

    if (!base_address_set_1 || !pattern_set) {
        diag_printf("usage: pmw [-u <upper base address>] -b <lower base address> -p <pattern> [-l <length>] [-1|-2|-4]\n");
        return;
    }
    if (!base_address_set) {
        upper_base_address = 0;
    }

    if (set_8bit) {
        _size = 1;
    } else if (set_16bit) {
        _size = 2;
        base_address &= ~0x1;
    } else if (set_32bit) {
        _size = 4;
        base_address &= ~0x3;
    } else {
        // Default to 32-bit
        _size = 4;
        base_address &= ~0x3;
    }

    if (!length_set) {
        length = _size;
    }

#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
    // If we are not building a full RedBoot then it is unlikely that the
    // ATU has been configured yet.
    // Do a minimal configuration here.
    HAL_PCI_MINIMAL_INIT();
#endif

#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using IOP ATU setup from Gen1-3
    HAL_READ_UINT32(ATU_OMWTVR0, old_omwtvr);
#endif
    HAL_READ_UINT32(ATU_OUMWTVR0, old_oumwtvr);

    while (length) {
        unsigned long base;
        long dump_len;
        void *window_addr, *offset;

        base = base_address & ATU_OMWTV_MASK;
        window_addr = (void*)(PCI_OUTBOUND_MEM_WINDOW + base_address - base);
        offset = (void*)((unsigned long)window_addr - base_address);

        if (((unsigned long)window_addr + length) > PCI_OUTBOUND_MEM_WINDOW_TOP) {
            dump_len = PCI_OUTBOUND_MEM_WINDOW_TOP - (unsigned long)window_addr;
        } else {
            dump_len = length;
        }

#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using IOP ATU setup from Gen1-3
        HAL_WRITE_UINT32(ATU_OMWTVR0, base);
#endif
        HAL_WRITE_UINT32(ATU_OUMWTVR0, upper_base_address);

        switch( _size ) {
        case 1:
        {
            // Fill 8 bits at a time
            cyg_uint8 *b = (cyg_uint8 *)window_addr;
            cyg_uint8 *p = (cyg_uint8 *)&pattern;
            long d = dump_len;

            while ((d -= sizeof(cyg_uint8)) >= 0) {
                diag_copy_mem8_safe(b, p, sizeof(*p));
                b++;
            }
            break;
        }
        case 2:
        {
            // Fill 16 bits at a time
            cyg_uint16 *b = (cyg_uint16 *)window_addr;
            cyg_uint16 *p = (cyg_uint16 *)&pattern;
            long d = dump_len;

            while ((d -= sizeof(cyg_uint16)) >= 0) {
                diag_copy_mem16_safe(b, p, sizeof(*p));
                b++;
            }
            break;
        }
        case 4:
        {
            // Fill 32 bits at a time
            cyg_uint32 *b = (cyg_uint32 *)window_addr;
            cyg_uint32 *p = (cyg_uint32 *)&pattern;
            long d = dump_len;

            while ((d -= sizeof(cyg_uint32)) >= 0) {
                diag_copy_mem32_safe(b, p, sizeof(*p));
                b++;
            }
            break;
        }
        }

        length -= dump_len;
        base_address += dump_len;
    }

#if CYGNUM_HAL_ARM_XSCALE_IOP_GENERATION < 4  // Using IOP ATU setup from Gen1-3
    HAL_WRITE_UINT32(ATU_OMWTVR0, old_omwtvr);
#endif
    HAL_WRITE_UINT32(ATU_OUMWTVR0, old_oumwtvr);
}

RedBoot_cmd("pmw",
            "Issue a PCI Memory Write",
            " [-u <upper base address>] -b <base address> -p <pattern> [-l <length>] [-1|-2|-4]",
            do_pmw
    );

static void
do_pir(int argc, char *argv[])
{
    struct option_info opts[5];
    unsigned base_address, length;
    bool base_address_set, length_set;
    bool set_32bit, set_16bit, set_8bit;
    char _size = 1;
    cyg_uint32 old_omwtvr;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base_address, (bool *)&base_address_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[2], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[3], '2', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[4], '1', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_8bit, (bool *)0, "fill 8 bit units");
    if (!scan_opts(argc, argv, 1, opts, 5, 0, 0, "")) {
        return;
    }

    if (!base_address_set) {
        diag_printf("usage: pir -b <base address> [-l <length>] [-1|-2|-4]\n");
        return;
    }

    if (set_8bit) {
        _size = 1;
    } else if (set_16bit) {
        _size = 2;
        base_address &= ~0x1;
    } else if (set_32bit) {
        _size = 4;
        base_address &= ~0x3;
    } else {
        // Default to 32-bit
        _size = 4;
        base_address &= ~0x3;
    }

    if (!length_set) {
        length = _size;
    }

#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
    // If we are not building a full RedBoot then it is unlikely that the
    // ATU has been configured yet.
    // Do a minimal configuration here.
    HAL_PCI_MINIMAL_INIT();
#endif

    HAL_READ_UINT32(ATU_OIOWTVR, old_omwtvr);

    while (length) {
        unsigned long base, dump_len;
        void *window_addr, *offset;

        base = base_address & ATU_OIOWTV_MASK;
        window_addr = (void*)(PCI_OUTBOUND_IO_WINDOW + base_address - base);
        offset = (void*)((unsigned long)window_addr - base_address);

        if (((unsigned long)window_addr + length) > PCI_OUTBOUND_IO_WINDOW_TOP) {
            dump_len = PCI_OUTBOUND_IO_WINDOW_TOP - (unsigned long)window_addr;
        } else {
            dump_len = length;
        }

        HAL_WRITE_UINT32(ATU_OMWTVR0, base);

        switch( _size ) {
        case 1:
            diag_dump_buf_with_offset(window_addr, dump_len, offset);
            break;
        case 2:
            diag_dump_buf_with_offset_16bit(window_addr, dump_len, offset);
            break;
        case 4:
            diag_dump_buf_with_offset_32bit(window_addr, dump_len, offset);
            break;
        }

        length -= dump_len;
        base_address += dump_len;
    }

    HAL_WRITE_UINT32(ATU_OIOWTVR, old_omwtvr);
}

RedBoot_cmd("pir",
            "Issue a PCI I/O Read",
            "-b <base address> [-l <length>] [-1|-2|-4]",
            do_pir
    );

static void
do_piw(int argc, char *argv[])
{
    struct option_info opts[6];
    unsigned base_address, length, pattern;
    bool base_address_set, length_set, pattern_set;
    bool set_32bit, set_16bit, set_8bit;
    char _size = 1;
    cyg_uint32 old_omwtvr;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base_address, (bool *)&base_address_set, "base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM,
              (void **)&length, (bool *)&length_set, "length");
    init_opts(&opts[2], '4', false, OPTION_ARG_TYPE_FLG,
              (void *)&set_32bit, (bool *)0, "fill 32 bit units");
    init_opts(&opts[3], '2', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_16bit, (bool *)0, "fill 16 bit units");
    init_opts(&opts[4], '1', false, OPTION_ARG_TYPE_FLG,
              (void **)&set_8bit, (bool *)0, "fill 8 bit units");
    init_opts(&opts[5], 'p', true, OPTION_ARG_TYPE_NUM,
              (void **)&pattern, (bool *)&pattern_set, "pattern");
    if (!scan_opts(argc, argv, 1, opts, 6, 0, 0, "")) {
        return;
    }

    if (!base_address_set || !pattern_set) {
        diag_printf("usage: piw -b <base address> -p <pattern> [-l <length>] [-1|-2|-4]\n");
        return;
    }

    if (set_8bit) {
        _size = 1;
    } else if (set_16bit) {
        _size = 2;
        base_address &= ~0x1;
    } else if (set_32bit) {
        _size = 4;
        base_address &= ~0x3;
    } else {
        // Default to 32-bit
        _size = 4;
        base_address &= ~0x3;
    }

    if (!length_set) {
        length = _size;
    }

#if defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
    // If we are not building a full RedBoot then it is unlikely that the
    // ATU has been configured yet.
    // Do a minimal configuration here.
    HAL_PCI_MINIMAL_INIT();
#endif

    HAL_READ_UINT32(ATU_OIOWTVR, old_omwtvr);

    while (length) {
        unsigned long base;
        long dump_len;
        void *window_addr, *offset;

        base = base_address & ATU_OIOWTV_MASK;
        window_addr = (void*)(PCI_OUTBOUND_IO_WINDOW + base_address - base);
        offset = (void*)((unsigned long)window_addr - base_address);

        if (((unsigned long)window_addr + length) > PCI_OUTBOUND_IO_WINDOW_TOP) {
            dump_len = PCI_OUTBOUND_IO_WINDOW_TOP - (unsigned long)window_addr;
        } else {
            dump_len = length;
        }

        HAL_WRITE_UINT32(ATU_OMWTVR0, base);

        switch( _size ) {
        case 1:
        {
            // Fill 8 bits at a time
            cyg_uint8 *b = (cyg_uint8 *)window_addr;
            cyg_uint8 *p = (cyg_uint8 *)&pattern;
            long d = dump_len;

            while ((d -= sizeof(cyg_uint8)) >= 0) {
                diag_copy_mem8_safe(b, p, sizeof(*p));
                b++;
            }
            break;
        }
        case 2:
        {
            // Fill 16 bits at a time
            cyg_uint16 *b = (cyg_uint16 *)window_addr;
            cyg_uint16 *p = (cyg_uint16 *)&pattern;
            long d = dump_len;

            while ((d -= sizeof(cyg_uint16)) >= 0) {
                diag_copy_mem16_safe(b, p, sizeof(*p));
                b++;
            }
            break;
        }
        case 4:
        {
            // Fill 32 bits at a time
            cyg_uint32 *b = (cyg_uint32 *)window_addr;
            cyg_uint32 *p = (cyg_uint32 *)&pattern;
            long d = dump_len;

            while ((d -= sizeof(cyg_uint32)) >= 0) {
                diag_copy_mem32_safe(b, p, sizeof(*p));
                b++;
            }
            break;
        }
        }

        length -= dump_len;
        base_address += dump_len;
    }

    HAL_WRITE_UINT32(ATU_OIOWTVR, old_omwtvr);
}

RedBoot_cmd("piw",
            "Issue a PCI I/O Write",
            "-b <base address> -p <pattern> [-l <length>] [-1|-2|-4]",
            do_piw
    );
#endif // defined(CYGPKG_IO_PCI) || defined(CYGSEM_HAL_BUILD_MINIMAL_REDBOOT)
#endif // defined(CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_PCI_REDBOOT_FNS)

#if defined(CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_ECC_REDBOOT_FNS)
#include <cyg/hal/hal_cache.h>

static void
do_eccf(int argc, char *argv[])
{
    struct option_info opts[1];
    volatile cyg_uint32 *base, readback;
    bool base_set;
    register unsigned cpsr;
    cyg_uint32 elog0, elog1, ecar0, ecar1, mcisr, eccr;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM,
              (void **)&base, (bool *)&base_set, "base address");
    if (!scan_opts(argc, argv, 1, opts, 1, 0, 0, "")) {
        return;
    }
    if (!base_set) {
        diag_printf("usage: eccf -b <base>\n");
        return;
    }

    //
    // Sync and flush all caches
    //
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
	HAL_L2_CACHE_DISABLE();
	HAL_L2_CACHE_SYNC();

    //
    // Disable single-bit correction; enable single and multi-bit reporting
    //
    HAL_READ_UINT32(MCU_ECCR, eccr);
    HAL_WRITE_UINT32(MCU_ECCR, ((eccr &~ MCU_ECCR_SINGLE_BIT_CORRECTION) |
                                (MCU_ECCR_SINGLE_BIT_REPORTING | MCU_ECCR_MULTI_BIT_REPORTING)));

    //
    // Write the value w/ invalid ECC
    //
    HAL_WRITE_UINT32(MCU_ECTST, 0x1);
    *base = 0xFEEDFACE;
    HAL_WRITE_UINT32(MCU_ECTST, 0x0);

    //
    // Enable IRQ/FIQ and trigger the ECC interrupt by reading back the value
    //
    asm volatile("mrs\t%0, cpsr" : "=r" (cpsr));
    asm volatile("msr\tcpsr, %0" : : "r" (cpsr & ~(CPSR_IRQ_DISABLE | CPSR_FIQ_DISABLE)));
    readback = *base;

    //
    // Wait for the interrupt to be serviced
    //
    do {
        HAL_READ_UINT32(MCU_MCISR, mcisr);
    } while (mcisr & 0x7);

    //
    // Disable IRQ/FIQ
    //
    asm volatile("msr\tcpsr, %0" : : "r" (cpsr));

    //
    // Restore ECCR
    //
    HAL_WRITE_UINT32(MCU_ECCR, eccr);

    //
    // Re-enable caches
    //
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
	HAL_L2_CACHE_ENABLE();
    //
    // Report out
    //
    HAL_READ_UINT32(MCU_ELOG0, elog0);
    HAL_READ_UINT32(MCU_ELOG1, elog1);
    HAL_READ_UINT32(MCU_ECAR0, ecar0);
    HAL_READ_UINT32(MCU_ECAR1, ecar1);
    HAL_READ_UINT32(MCU_MCISR, mcisr);
    diag_printf("ELOG0 = 0x%08lx, ELOG1 = 0x%08lx\n", elog0, elog1);
    diag_printf("ECAR0 = 0x%08lx, ECAR1 = 0x%08lx\n", ecar0, ecar1);
    diag_printf("MCISR = 0x%08lx\n", mcisr);
}

RedBoot_cmd("eccf",
            "Force an ECC failure",
            "-b <base_address>",
            do_eccf
    );
#endif // defined(CYGSEM_HAL_ARM_XSCALE_IOP_PROVIDE_ECC_REDBOOT_FNS)

/*------------------------------------------------------------------------*/
// Is Opaque memory enabled?
unsigned int hal_platform_linux_exec_capable(void)
{
#if !defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_FUNCTION_RAM) || !defined(CYGNUM_HAL_ARM_XSCALE_IOP_BAR2_SIZE_0)
    diag_printf("Cannot execute the Linux kernel.\n");
    diag_printf("Ensure that BAR2 is configured to share all of RAM.\n");
    return 0;
#else
    return 1;
#endif
}
