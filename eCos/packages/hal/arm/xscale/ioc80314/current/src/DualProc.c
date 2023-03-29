//==========================================================================
//
//      DualProc.c
//
//      RedBoot dump support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    dkranak
// Date:         2003-06-02
// Purpose:      Dual processor control
// Description:
//
// This code is part of RedBoot (tm).
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <pkgconf/hal.h>
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_ioc80314.h>
#include CYGBLD_HAL_BOARD_H
#include <cyg/hal/hal_diag.h>           // Diag IO
#include <cyg/hal/hal_cache.h>


RedBoot_init(dual_proc_prep, RedBoot_INIT_LAST);

#ifdef PKG_HAL_HAS_MULT_CPU
extern unsigned long * Boot_init_cpu_1;
extern volatile long _curProc;		  // Only run init from CPU0
volatile int procrunning = 0;
extern  void do_go(int argc, char *argv[]);

RedBoot_cmd("Dual",
            "Control the dual processor behavior.",
            "[-e(nable) | -s(top) | -g(o : executes loaded image)] [-0<Timer0>] [-1<Timer1>]",
            do_DualProc
    );

void
do_DualProc(int argc, char *argv[])
{
    struct                   option_info opts[6];
    unsigned long            ab_timer0;
	unsigned long            ab_timer1;
    static unsigned long     _ab_timer0 = 0x400;
    static unsigned long     _ab_timer1 = 0x400;
	static unsigned long 	 _wd_timer  = 0x1000;
    bool                     ab_timer0_set, ab_timer1_set, _enable_dual_set, _disable_dual_set, _status_set, _dual_go;
    unsigned long            procID, clockCfg;
    volatile unsigned long * ciu_base = (unsigned long *)(HAL_IOC80314_CIU_BASE);
    volatile unsigned long * reset_base = (unsigned long *)SDRAM_BASE;

    init_opts(&opts[0], 'e', false, OPTION_ARG_TYPE_FLG,
              (void **)&_enable_dual_set, 0, "Enable both processors");
    init_opts(&opts[1], 's', false, OPTION_ARG_TYPE_FLG,
              (void **)&_disable_dual_set, 0, "Disable processor 1");
    init_opts(&opts[2], '0', true, OPTION_ARG_TYPE_NUM,
              (void **)&ab_timer0, (bool *)&ab_timer0_set, "Processor 0 Timer");
    init_opts(&opts[3], '1', true, OPTION_ARG_TYPE_NUM,
              (void **)&ab_timer1, (bool *)&ab_timer1_set, "Processor 1 Timer");
    init_opts(&opts[4], 'x', false, OPTION_ARG_TYPE_FLG,
              (void **)&_status_set, 0, "show status");
    init_opts(&opts[5], 'g', false, OPTION_ARG_TYPE_FLG,
              (void **)&_dual_go, 0, "Execute image and do stack fixup");
    if (!scan_opts(argc, argv, 1, opts, 6, 0, 0, "")) {
        return;
    }

    if (ab_timer0_set) { _ab_timer0 = ab_timer0; ciu_base[TS_CIU_TIMER0/4] = ab_timer0;}
    if (ab_timer1_set) { _ab_timer1 = ab_timer1; ciu_base[TS_CIU_TIMER1/4] = ab_timer1;}

    if (_enable_dual_set) {
        if ((_curProc == 0) && (!(ciu_base[TS_CIU_CFG/4] & 0x1))) {
            diag_printf("Enabling Processor 1: T0=%d T1=%d\n", _ab_timer0, _ab_timer1);
		    ciu_base[TS_CIU_TIMER0/4] = _ab_timer0;
			ciu_base[TS_CIU_TIMER1/4] = _ab_timer1;
			ciu_base[TS_CIU_WDTIMER/4] = _wd_timer;
			reset_base[0x20/4]        = (unsigned long)&Boot_init_cpu_1;
			HAL_DELAY_US(2);
			HAL_DCACHE_SYNC();
			HAL_DCACHE_INVALIDATE_ALL();
            ciu_base[TS_CIU_CFG/4]   |= (TS_CIU_CFG_ARB_EN + TS_CIU_CFG_WD_EN);
			HAL_DELAY_US(1000000);
            ciu_base[TS_CIU_TIMER0/4] = _ab_timer0;
            return;
        }
        else {
            diag_printf("!!!! Processor 1 already enabled.\n");
            return;
        }
    }
	
	if (_dual_go) {
		cyg_uint32 other_load_add;
		other_load_add = *(unsigned int*)(&load_address + (NUM_HAL_CPU_1_BASE_PHY_ADDR/4));   // Get load add from other CPU
		if (load_address == 0x0) {
			diag_printf ("You haven't loaded an image to execute - aborting...\n");
		}
		else {
			if (other_load_add <= 0x200000) {
				diag_printf ("My Load address:  %p\n", load_address);
				diag_printf ("My Workspace_end: %p\n", workspace_end);
				diag_printf ("My ram_end:       %p\n", ram_end);
				diag_printf ("Load address of other Proc is <= 0x200000 -  you don't want your stack there\n");
				diag_printf ("Not executing....\n");
			}
			else {
				diag_printf ("Executing image and doing stack fixup...\n");
				diag_printf ("My Image start: %p  Image end: %p\n", load_address, load_address_end);
				diag_printf ("My workspace end: %p\n", workspace_end);
				diag_printf ("My ram_end:       %p\n", ram_end);
				diag_printf ("Load address of other Proc: %p\n", other_load_add);
				workspace_end = (char *)other_load_add;
				ram_end = (char *)other_load_add;
				HAL_DCACHE_SYNC();
				diag_printf ("My new workspace end: %p\n", workspace_end);
				do_go(0, NULL);
			}
		}
	}
	else if (_disable_dual_set) {
        if (_curProc == 1) {
            diag_printf("!!!! Processor 1 can only be disabled by Processor 0.\n");
            return;
        }
        else {
            diag_printf("Freezing processor 1.\n");
            ciu_base[TS_CIU_CFG/4]   &= ~(TS_CIU_CFG_ARB_EN + TS_CIU_CFG_WD_EN);
			return;
        }
    }
	else {
   // Else report current status
      procID     = (unsigned long)_Read_ID();
      clockCfg   = (unsigned long)_Read_CCLKCFG();
      ab_timer0  = ciu_base[TS_CIU_TIMER0/4];
      ab_timer1  = ciu_base[TS_CIU_TIMER1/4];

      diag_printf("   Current Processor      = %d.\n", _curProc);
      diag_printf("   Processor ID           = %08X.\n", procID);
      diag_printf("   Processor Clock Config = %08X.\n", clockCfg);
      diag_printf("   Cpu Timer 0            = %d.\n", ab_timer0);
      diag_printf("   Cpu Timer 1            = %d.\n", ab_timer1);
	}
}

void dual_proc_prep (void) {
	workspace_start = (char *)0x200000;  // Start User Workspace at 2Meg Offset - 0-1Meg for CPU0, 1-2Meg for CPU1

	// If CPU0 - no fixup needed
	if (_curProc ==0) { 
		return;
	}												     
	else {
		workspace_end = *(unsigned int*)(&workspace_end + (NUM_HAL_CPU_1_BASE_PHY_ADDR/4));   // Get workspace end from CPU0
	}
}


#else   // We only have one CPU - so only need 0-1Meg reserved
void dual_proc_prep (void) {
	workspace_start = (char *)0x100000;  // Start User Memory at 2Meg Offset - 0-1Meg for CPU0, 1-2Meg for CPU1
}

#endif //PKG_HAL_HAS_MULT_CPU
