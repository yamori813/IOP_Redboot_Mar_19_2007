#ifndef CYGONCE_HAL_MM_H
#define CYGONCE_HAL_MM_H

//=============================================================================
//
//      hal_mm.h
//
//      Intel XScale MM common definitions 
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
// Contributors: gthomas, dmoseley
// Date:         2001-12-03
// Purpose:      Intel XScale MM common definitions
// Description:  The macros defined here provide common definitions for
//		 memory management initialization.
// Usage:
//               #include <cyg/hal/hal_mm.h>
//               ...
//              
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#define PTE_SECTION_FLASH    ((3 << 10) | (1 << 3) | 2)
#define PTE_SECTION_RAM      ((1 << 12) | (3 << 10) | (1 << 3) | (1 << 2) | 2)
#define PTE_SECTION_UNCACHED ((3 << 10) | 2)

#ifdef __ASSEMBLER__

// form a first-level section entry
.macro FL_SECTION_ENTRY base,x,ap,p,d,c,b
.word (\base << 20) | (\x << 12) | (\ap << 10) | (\p << 9) |\
      (\d << 5) | (\c << 3) | (\b << 2) | 2
.endm

/* Super Section Descriptors Available on XScale Core gen. 3 */
#if (CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION >= 3)
// form a first-level supersection entry
.macro FL_SUPERSECTION_ENTRY base,upper_base,x,ap,p,d,c,b
.word (\base << 24) | (\upper_base << 20) | (1 << 18) | (\x << 12) | (\ap << 10) | (\p << 9) |\
      (\d << 5) | (\c << 3) | (\b << 2) | 2
.endm

#endif
// form a first-level unmapped entry
.macro FL_UNMAPPED_ENTRY
.word 0
.endm

// form a first-level page table entry
.macro FL_PT_ENTRY base,p,d
// I wanted to use logical operations here, but since I am using symbols later 
// to fill in the parameters, I had to use addition to force the assembler to
// do it right
.word \base + (\p << 9) + (\d << 5) + 1
.endm

// form a second level small page entry
.macro SL_SMPAGE_ENTRY base,ap3,ap2,ap1,ap0,c,b
.word (\base << 12) | (\ap3 << 10) | (\ap2 << 8) | (\ap1 << 6) |\
      (\ap0 << 4) | (\c << 3) | (\b << 2) | 2
.endm

// form a second level extended small page entry
.macro SL_XSMPAGE_ENTRY base,x,ap,c,b
.word (\base << 12) | (\x << 6) | (\ap << 4) | (\c << 3) | (\b << 2) | 3
.endm

#else

// -------------------------------------------------------------------------
// MMU initialization:
// 
// These structures are laid down in memory to define the translation
// table.
// 

/*
 * XScale Translation Table Base Bit Masks
 */
#define ARM_TRANSLATION_TABLE_MASK               0xFFFFC000

/*
 * XScale Domain Access Control Bit Masks
 */
#define ARM_ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << (domain_num)*2)
#define ARM_ACCESS_TYPE_CLIENT(domain_num)       (0x1 << (domain_num)*2)
#define ARM_ACCESS_TYPE_MANAGER(domain_num)      (0x3 << (domain_num)*2)

struct ARM_MMU_FIRST_LEVEL_FAULT {
    unsigned int id : 2;
    unsigned int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_FAULT_ID 0x0

struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
    unsigned int id : 2;
    unsigned int imp : 2;
    unsigned int domain : 4;
    unsigned int p : 1;
    unsigned int base_address : 23;
};
#define ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1

struct ARM_MMU_FIRST_LEVEL_SECTION {
    unsigned int id : 2;
    unsigned int b : 1;
    unsigned int c : 1;
    unsigned int imp : 1;
    unsigned int domain : 4;
    unsigned int p : 1;
    unsigned int ap : 2;
    unsigned int tex : 8;
    unsigned int base_address : 12;
};
#define ARM_MMU_FIRST_LEVEL_SECTION_ID 0x2

struct ARM_MMU_FIRST_LEVEL_RESERVED {
    unsigned int id : 2;
    unsigned int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_RESERVED_ID 0x3

#define ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
   (unsigned long *)((unsigned long)(ttb_base) + ((table_index) << 2))

#define ARM_FIRST_LEVEL_PAGE_TABLE_SIZE 0x4000

#define ARM_MMU_SECTION(ttb_base, actual_base, virtual_base,              \
                        cacheable, bufferable, perm)                      \
    CYG_MACRO_START                                                       \
        register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
                                                                          \
        desc.word = 0;                                                    \
        desc.section.id = ARM_MMU_FIRST_LEVEL_SECTION_ID;                 \
        desc.section.domain = 0;                                          \
        desc.section.c = (cacheable);                                     \
        desc.section.b = (bufferable);                                    \
        desc.section.ap = (perm);                                         \
        desc.section.base_address = (actual_base);                        \
        *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, (virtual_base)) \
                            = desc.word;                                  \
    CYG_MACRO_END

#define X_ARM_MMU_SECTION(abase,vbase,size,cache,buff,access)      \
    { int i; int j = abase; int k = vbase;                         \
      for (i = size; i > 0 ; i--,j++,k++)                          \
      {                                                            \
        ARM_MMU_SECTION(ttb_base, j, k, cache, buff, access);      \
      }                                                            \
    }

union ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARM_MMU_FIRST_LEVEL_FAULT fault;
    struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct ARM_MMU_FIRST_LEVEL_SECTION section;
    struct ARM_MMU_FIRST_LEVEL_RESERVED reserved;
};

#define ARM_UNCACHEABLE                         0
#define ARM_CACHEABLE                           1
#define ARM_UNBUFFERABLE                        0
#define ARM_BUFFERABLE                          1

#define ARM_ACCESS_PERM_NONE_NONE               0
#define ARM_ACCESS_PERM_RO_NONE                 0
#define ARM_ACCESS_PERM_RO_RO                   0
#define ARM_ACCESS_PERM_RW_NONE                 1
#define ARM_ACCESS_PERM_RW_RO                   2
#define ARM_ACCESS_PERM_RW_RW                   3

/*
 * Initialization for the Domain Access Control Register
 */
#define ARM_ACCESS_DACR_DEFAULT      (          \
        ARM_ACCESS_TYPE_MANAGER(0)    |         \
        ARM_ACCESS_TYPE_NO_ACCESS(1)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(2)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(3)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(4)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(5)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(6)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(7)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(8)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(9)  |         \
        ARM_ACCESS_TYPE_NO_ACCESS(10) |         \
        ARM_ACCESS_TYPE_NO_ACCESS(11) |         \
        ARM_ACCESS_TYPE_NO_ACCESS(12) |         \
        ARM_ACCESS_TYPE_NO_ACCESS(13) |         \
        ARM_ACCESS_TYPE_NO_ACCESS(14) |         \
        ARM_ACCESS_TYPE_NO_ACCESS(15)  )

#endif // ifdef __ASSEMBLER__

// ------------------------------------------------------------------------
// XScale extras.
#define MMU_Control_BTB 0x800


#endif // ifndef CYGONCE_HAL_MM_H
// End of hal_mm.h





