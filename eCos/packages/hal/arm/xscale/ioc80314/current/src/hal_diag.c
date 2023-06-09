/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Author(s):   dkranak
// Contributors:nickg, gthomas, msalter, dkranak, cebruns
// Date:        2002-10-06
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
//#include <pkgconf/system.h>
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>            // cyg_drv_interrupt_acknowledge
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <redboot.h>
#include <cyg/hal/hal_cache.h>
#include CYGBLD_HAL_DIAG_INL          // Platform specific code

static void cyg_hal_plf_serial_init(void);
void cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 c);

//=============================================================================
// Serial driver
//=============================================================================

//-----------------------------------------------------------------------------
// There are two serial ports.
//#define CYG_DEV_SERIAL_BASE_A    0xfe800000 // port A
//#define CYG_DEV_SERIAL_BASE_B    0xfe810000 // port B

/*---------------------------------------------------------------------------*/
/* From serial_16550.h */

//-----------------------------------------------------------------------------
// Define the serial registers. 80314 is similar to dual 16550 serial chip.

#define CYG_DEV_SERIAL_RBR   0x00  // receiver buffer register, read, dlab = 0
#define CYG_DEV_SERIAL_THR   0x00 // transmitter holding register, write, dlab = 0
#define CYG_DEV_SERIAL_DLL   0x00 // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_SERIAL_IER   0x01 // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_SERIAL_DLM   0x01 // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_SERIAL_IIR   0x02 // interrupt identification register, read, dlab = 0
#define CYG_DEV_SERIAL_FCR   0x02 // fifo control register, write, dlab = 0
#define CYG_DEV_SERIAL_LCR   0x03 // line control register, write
#define CYG_DEV_SERIAL_MCR   0x04 // modem control register, write
#define CYG_DEV_SERIAL_LSR   0x05 // line status register, read
#define CYG_DEV_SERIAL_MSR   0x06 // modem status register, read
#define CYG_DEV_SERIAL_SCR   0x07 // scratch pad register

// The interrupt enable register bits.
#define SIO_IER_ERDAI   0x01            // enable received data available irq
#define SIO_IER_ETHREI  0x02            // enable THR empty interrupt
#define SIO_IER_ELSI    0x04            // enable receiver line status irq
#define SIO_IER_EMSI    0x08            // enable modem status interrupt

// The interrupt identification register bits.
#define SIO_IIR_IP      0x01            // 0 if interrupt pending
#define SIO_IIR_ID_MASK 0x0e            // mask for interrupt ID bits
#define ISR_Tx  0x02
#define ISR_Rx  0x04

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// The FIFO control register
#define SIO_FCR_FCR0   0x01             // enable xmit and rcvr fifos
#define SIO_FCR_FCR1   0x02             // clear RCVR FIFO
#define SIO_FCR_FCR2   0x04             // clear XMIT FIFO

// Modem Control Register
#define SIO_MCR_DTR 0x01
#define SIO_MCR_RTS 0x02


//-----------------------------------------------------------------------------

channel_data_t channels[2] = {
    { (cyg_uint8*)HAL_IOC80314_UART1_BASE, 1000, CYGNUM_HAL_INTERRUPT_SERIAL_A},
    { (cyg_uint8*)HAL_IOC80314_UART2_BASE, 1000, CYGNUM_HAL_INTERRUPT_SERIAL_B}
};

#ifdef PKG_HAL_HAS_MULT_CPU
static cyg_uint32     channelSelected = 0;
static cyg_uint32* uBuC_ChannelSelected = (cyg_uint32 *)((int)(&channelSelected) + (int)SDRAM_UNCACHED_BASE);
static cyg_uint32* ptrChannelSelected = (cyg_uint32 *)((int)(&channelSelected) + (int)(NUM_HAL_CPU_1_BASE_PHY_ADDR + SDRAM_UNCACHED_BASE));
#endif

//-----------------------------------------------------------------------------
static int
set_baud( channel_data_t *chan )
{
    cyg_uint8* base = chan->base;
    cyg_uint8 i;
	// If board revision == 1, then use the baud rates based on CPU speed, else use baud rates based on SFN speed
    cyg_uint8 j = BAUD_RATE_TABELE_INDEX;

    for (i=0; i<(sizeof(baud_conf)/sizeof(baud_conf[0])); i++)
    {
        if (chan->baud_rate == baud_conf[i].baud_rate) {
            cyg_uint8 lcr;
            HAL_READ_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, lcr|SIO_LCR_DLAB);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_DLL, baud_conf[i].lsb[j]);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_DLM, baud_conf[i].msb[j]);
            HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, lcr);
            return 1;
        }
    }
    return -1;
}


static void
init_serial_channel(channel_data_t* __ch_data)
{
#if defined DEBUG_SERIAL_GETC && defined HAVE_LED_DISPLAY
    unsigned char *pLed;
                pLed = DISPLAY_LEFT;
               *pLed = 0;
                pLed = DISPLAY_RIGHT
               *pLed = 0;
#endif

    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    channel_data_t* chan = (channel_data_t*)__ch_data;

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);
    chan->baud_rate = CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD;
    set_baud( chan );
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_FCR, 0x01);  // Enable FIFO
    CYGACC_CALL_IF_DELAY_US(10);
    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_FCR, 0x07);  // Enable & clear FIFO
    CYGACC_CALL_IF_DELAY_US(10);
}

cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
#ifdef DEBUG_SERIAL_GETC
    static unsigned char firsterror  = 0;
    static unsigned char seconderror = 0;
    static unsigned char firstdata   = 0;
    static unsigned char seconddata  = 0;
#ifdef HAVE_LED_DISPLAY
    unsigned char *pLed;
#endif
#endif

             cyg_bool   done   = false;
             cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
             cyg_uint8  lsr;
             cyg_uint8  cch;
#ifdef DEBUG_SERIAL_ECHO
    channel_data_t* __ch_data2 = (__ch_data==channels[0])? __ch_data2=channels[1]: __ch_data2=channels[0];
#endif
    static   cyg_uint8  recBuf[SERIAL_REC_BUF];
    static   cyg_uint32 recBufHead  = 0;
    static   cyg_uint32 recBufTail  = 0;

#ifdef PKG_HAL_HAS_MULT_CPU
	if (!channelSelected)
	   {
	   	if ((_curProc == 1) && (base == ((((channel_data_t*)(*ptrChannelSelected)))->base)))
		  {
			return false;
		  }
	   }
#endif


    while(!done)
      {
       HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
#ifdef DEBUG_SERIAL_GETC
       if ((lsr & (SIO_LSR_OE | SIO_LSR_PE | SIO_LSR_FE | SIO_LSR_BI | SIO_LSR_ERR) ) != 0)
         {
           if (firsterror == 0)
             {
              firsterror = lsr;
#ifdef HAVE_LED_DISPLAY
              pLed = DISPLAY_LEFT;
              *pLed = firsterror;
#endif
              if ((lsr & SIO_LSR_DR) == SIO_LSR_DR)
                {
                HAL_READ_UINT8(base+CYG_DEV_SERIAL_RBR, cch);
                recBuf[recBufHead++] = cch;
                firstdata = cch;
                recBufHead %= SERIAL_REC_BUF;
#ifdef DEBUG_SERIAL_ECHO
                cyg_hal_plf_serial_putc(__ch_data2, cch)
#endif
                }
             }
           else                         /* have firsterror */
             {
             seconderror = lsr;
#ifdef HAVE_LED_DISPLAY
             pLed = DISPLAY_RIGHT;
             *pLed = seconderror;
#endif
             if ((lsr & SIO_LSR_DR) == SIO_LSR_DR)
               {
               HAL_READ_UINT8(base+CYG_DEV_SERIAL_RBR, cch);
               recBuf[recBufHead++] = cch;
               seconddata = cch;
               recBufHead %= SERIAL_REC_BUF;
#ifdef DEBUG_SERIAL_ECHO
               cyg_hal_plf_serial_putc(__ch_data2, cch)
#endif
               }
             }     /* end firsterror */
           }       /* end of error */
#endif
      if ((lsr & SIO_LSR_DR) == 0)      /* do we have char in buffer? */
        {
        done = true;
        }
      else
        {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_RBR, cch);
#ifdef DEBUG_SERIAL_ECHO
        cyg_hal_plf_serial_putc(__ch_data2, cch)
#endif
        recBuf[recBufHead++] = cch;
        recBufHead %= SERIAL_REC_BUF;
        }                                 /* end of if char in buffer */
      }                                   /* end of while */
    if ( recBufHead == recBufTail)
      {
      return false;
      }
    else
      {
#ifdef PKG_HAL_HAS_MULT_CPU
  	  if (!channelSelected)
 		{
			channelSelected       = (cyg_uint32)__ch_data;
 			*uBuC_ChannelSelected = (cyg_uint32)__ch_data;
 		}
#endif
	  *ch = recBuf[recBufTail++];

          recBufTail %= SERIAL_REC_BUF;
      }
    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

void
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 c)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    volatile cyg_uint8 lsr;

#ifdef PKG_HAL_HAS_MULT_CPU
	if (!channelSelected)
	   {
	   	//if ((_curProc == 1) && (base == ((channel_data_t*)(*ptrChannelSelected)->base)))
	   	if ((_curProc == 1) && (base == ((((channel_data_t*)(*ptrChannelSelected)))->base)))
		  {
			return;
		  }
	   }
#endif
	CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    HAL_WRITE_UINT8(base+CYG_DEV_SERIAL_THR, c);

    // Hang around until the character has been safely sent.
    //!!!! remove once working.

    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}

void
cyg_hal_plf_serial_empty_fifo(void* __ch_data)
{
    cyg_uint8* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    // Hang around until the character has been safely sent.
    //!!!! remove once working.
    do {
        HAL_READ_UINT8(base+CYG_DEV_SERIAL_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    CYGARC_HAL_RESTORE_GP();
}


static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf,
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 1000; // delay in usec = ~1 second
    res = hal_delay_us_getc(delay_count, __ch_data, ch);
    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 ier;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_INTERRUPT_SET_LEVEL(chan->isr_vector, 1);
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        ier |= SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector);
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        ier &= ~SIO_IER_ERDAI;
        HAL_WRITE_UINT8(chan->base+CYG_DEV_SERIAL_IER, ier);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
    {
        va_list ap;
        va_start(ap, __func);
        chan->baud_rate = va_arg(ap, cyg_int32);
        va_end(ap);
        ret = set_baud(chan);
        break;
    }
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc,
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint8 _iir;
    CYGARC_HAL_SAVE_GP();

    HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;

    *__ctrlc = 0;

    if ( ISR_Rx == _iir ) {
        cyg_uint8 c, lsr;
        HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_LSR, lsr);
    if ( (lsr & SIO_LSR_DR) != 0 ) {

            HAL_READ_UINT8(chan->base+CYG_DEV_SERIAL_RBR, c);
            if( cyg_hal_is_break( (char *) &c , 1 ) )
                *__ctrlc = 1;
        }
        // Acknowledge the interrupt
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);
        res = CYG_ISR_HANDLED;
    }
    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm0;
    hal_virtual_comm_table_t* comm1;
#ifdef PKG_HAL_HAS_MULT_CPU
    //volatile unsigned long * ciu_base = (unsigned long *)(HAL_IOC80314_CIU_BASE);
    //unsigned long            curProc=ciu_base[TS_CIU_CFG/4] & TS_CIU_CFG_PID;
#endif

    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

#ifdef PKG_HAL_HAS_MULT_CPU
    if (_curProc == 0) {
#endif
    // Disable interrupts.
      HAL_INTERRUPT_MASK(channels[0].isr_vector);
      HAL_INTERRUPT_MASK(channels[1].isr_vector);

      // Init channels
      init_serial_channel(&channels[0]);
      init_serial_channel(&channels[1]);
      console_selected = false;
#ifdef PKG_HAL_HAS_MULT_CPU
    }
	else
	{
		channelSelected = 0;
	}
#endif

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm0 = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm0, &channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm0, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm0, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm0, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm0, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm0, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm0, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm0, cyg_hal_plf_serial_getc_timeout);

    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm1 = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm1, &channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm1, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm1, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm1, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm1, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm1, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm1, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm1, cyg_hal_plf_serial_getc_timeout);

#ifdef PKG_HAL_HAS_MULT_CPUXXXX
    if (_curProc == 0) {
      CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
	} else {
	  volatile CYG_ADDRWORD *vvtP0 = (CYG_ADDRWORD*)((CYG_ADDRWORD)hal_virtual_vector_table + (CYG_ADDRWORD)NUM_HAL_CPU_1_BASE_PHY_ADDR);
	  hal_virtual_comm_table_t*   call_if_console_vector = (hal_virtual_comm_table_t*)(vvtP0[CYGNUM_CALL_IF_CONSOLE_PROCS]);
	  if (call_if_console_vector == comm1){
        CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
        CYGACC_CALL_IF_SET_DEBUG_COMM(0);
	  } else {
        CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
        CYGACC_CALL_IF_SET_DEBUG_COMM(1);
	  }
	  console_selected = true;
	}
#else
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
#endif
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}


//=============================================================================
// Compatibility with older stubs
//=============================================================================
/*
cyg_uint8 cyg_hal_led_segment[16] = {
    DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3,
    DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
    DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B,
    DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F };


void
hal_diag_led(int n)
{
    HAL_WRITE_UINT8(DISPLAY_RIGHT, cyg_hal_led_segment[n & 0x0f]);
}
*/

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG


#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#include <cyg/hal/hal_stub.h>           // cyg_hal_gdb_interrupt
#endif

// Assumption: all diagnostic output must be GDB packetized unless this is a ROM (i.e.
// totally stand-alone) system.

#if defined(CYG_HAL_STARTUP_ROM) || !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
#define HAL_DIAG_USES_HARDWARE
#endif

/*---------------------------------------------------------------------------*/
#if CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL==0
// This is the base address of the A-channel
#define CYG_DEV_SERIAL_BASE      HAL_IOC80314_UART1_BASE
#define CYG_DEV_SERIAL_INT       CYGNUM_HAL_INTERRUPT_SERIAL_A
#else
// This is the base address of the B-channel
#define CYG_DEV_SERIAL_BASE      HAL_IOC80314_UART1_BASE
#define CYG_DEV_SERIAL_INT       CYGNUM_HAL_INTERRUPT_SERIAL_B
#endif

static channel_data_t ser_channel = { (cyg_uint8*)CYG_DEV_SERIAL_BASE, 0, 0};

#ifdef HAL_DIAG_USES_HARDWARE

void hal_diag_init(void)
{
    static int init = 0;
    char *msg = "\n\rARM eCos\n\r";

    if (init) return;
    init++;
    init_serial_channel(&ser_channel);

    while (*msg) hal_diag_write_char(*msg++);
}

#ifdef DEBUG_DIAG
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define DIAG_BUFSIZE 32
#else
#define DIAG_BUFSIZE 2048
#endif
static char diag_buffer[DIAG_BUFSIZE];
static int diag_bp = 0;
#endif

void hal_diag_write_char(char c)
{
    cyg_uint8 lsr;

    hal_diag_init();

    cyg_hal_plf_serial_putc(&ser_channel, c);

#ifdef DEBUG_DIAG
    diag_buffer[diag_bp++] = c;
    if (diag_bp == DIAG_BUFSIZE) {
        while (1) ;
        diag_bp = 0;
    }
#endif
}

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(&ser_channel);
}

#else // HAL_DIAG relies on GDB

// Initialize diag port - assume GDB channel is already set up
void hal_diag_init(void)
{
    if (0) init_serial_channel(&ser_channel); // avoid warning
}

// Actually send character down the wire
static void
hal_diag_write_char_serial(char c)
{
    cyg_hal_plf_serial_putc(&ser_channel, c);
}

static bool
hal_diag_read_serial(char *c)
{
    long timeout = 1000000000;  // A long time...
    while (!cyg_hal_plf_serial_getc_nonblock(&ser_channel, c))
        if (0 == --timeout) return false;

    return true;
}

void
hal_diag_read_char(char *c)
{
    while (!hal_diag_read_serial(c)) ;
}

void
hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif

        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;

            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            if (!hal_diag_read_serial(&c1))
                continue;   // No response - try sending packet again

            if( c1 == '+' )
                break;              // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            cyg_drv_interrupt_acknowledge(CYG_DEV_SERIAL_INT);
            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt (__builtin_return_address(0));
                break;
            }
#endif
            // otherwise, loop round again
        }

        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif

    }
}
#endif

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
