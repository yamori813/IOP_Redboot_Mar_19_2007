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
// Author(s):    curt.e.bruns@intel.com
// Contributors: drew.moseley@intel.com
// Date:         2003-07-10
// Purpose:      HAL diagnostic output
// Description:  Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/iq8134x.h>             // platform definitions

/*---------------------------------------------------------------------------*/
/* From serial_16550.h */

struct baud_config {
    cyg_int32 baud_rate;
    cyg_uint8 msb;
    cyg_uint8 lsb;
};


// Working Config - CSPEC is incorrect!
struct baud_config baud_conf[] = {
    {9600,   0, 217},
    {19200,  0, 109},
    {38400,  0,  54},
    {57600,  0,  36},
    {115200, 0,  18}};

/*
// C-spec Config
struct baud_config baud_conf[] = {
    {9600,   0, 108},
    {19200,  0, 54},
    {38400,  0, 27},
    {57600,  0, 18},
    {115200, 0, 9}};
*/
    
// Define the serial registers.
#define CYG_DEV_RBR   0x00 // receiver buffer register, read, dlab = 0
#define CYG_DEV_THR   0x00 // transmitter holding register, write, dlab = 0
#define CYG_DEV_DLL   0x00 // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_IER   0x01 // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_DLM   0x01 // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_IIR   0x02 // interrupt identification register, read, dlab = 0
#define CYG_DEV_FCR   0x02 // fifo control register, write, dlab = 0
#define CYG_DEV_LCR   0x03 // line control register, write
#define CYG_DEV_MCR   0x04 // modem control register, write
#define CYG_DEV_LSR   0x05 // line status register, read
#define CYG_DEV_MSR   0x06 // modem status register, read
#define CYG_DEV_SCR   0x07 // scratch pad register

// Interrupt Enable Register
#define SIO_IER_RCV 0x01
#define SIO_IER_XMT 0x02
#define SIO_IER_LS  0x04
#define SIO_IER_MS  0x08
#define SIO_IER_UUE 0x40

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

// Modem Control Register
#define SIO_MCR_DTR 0x01
#define SIO_MCR_RTS 0x02


//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint32* base;
    cyg_int32 msec_timeout;
    int isr_vector;
    cyg_int32 baud_rate;
    int irq_state;
} channel_data_t;

// IQ8134x boards have two serial ports.  Transport core
// reserves UART0 in SL Mode.  If the RedBoot build includes two
// Comm channels, then we are ignoring Transport core's
// ownership and we'll use UART0 for Core0 and 
// UART1 for Core1.
static channel_data_t plf_ser_channels[CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS] = {
#if !defined (CYG_HAL_STARTUP_RAMDUAL) 
    {
        (cyg_uint8*)IOP34X_UART1_ADDR,
        1000, 
        CYGNUM_HAL_INTERRUPT_UART1,
        CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD,
        0
    },
#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    {
        (cyg_uint8*)IOP34X_UART0_ADDR,
        1000, 
        CYGNUM_HAL_INTERRUPT_UART0,
        CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD,
        0
    },
#endif
#else
    {
        (cyg_uint8*)IOP34X_UART0_ADDR,
        1000, 
        CYGNUM_HAL_INTERRUPT_UART0,
        CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD,
        0
    },
#endif
};


//-----------------------------------------------------------------------------

static int
set_baud( channel_data_t *chan )
{
    cyg_uint32* base = chan->base;
    cyg_uint8 i;

    for (i=0; i<(sizeof(baud_conf)/sizeof(baud_conf[0])); i++)
    {
        if (chan->baud_rate == baud_conf[i].baud_rate) {
            cyg_uint8 lcr;
            HAL_READ_UINT8(base+CYG_DEV_LCR, lcr);
            HAL_WRITE_UINT8(base+CYG_DEV_LCR, lcr|SIO_LCR_DLAB);
            HAL_WRITE_UINT8(base+CYG_DEV_DLL, baud_conf[i].lsb);
            HAL_WRITE_UINT8(base+CYG_DEV_DLM, baud_conf[i].msb);
            HAL_WRITE_UINT8(base+CYG_DEV_LCR, lcr);
            return 1;
        }
    }
    return -1;
}

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint32* base = ((channel_data_t*)__ch_data)->base;
    channel_data_t* chan = (channel_data_t*)__ch_data;

    // Enable the UART function 
    HAL_WRITE_UINT8(base+CYG_DEV_IER, SIO_IER_UUE);

    // 8-1-no parity.
    HAL_WRITE_UINT8(base+CYG_DEV_LCR, SIO_LCR_WLS0 | SIO_LCR_WLS1);
    set_baud( chan );
    HAL_WRITE_UINT8(base+CYG_DEV_FCR, 0x07);  // Enable & clear FIFO
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    cyg_uint32* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    do {
        HAL_READ_UINT8(base+CYG_DEV_LSR, lsr);
    } while ((lsr & SIO_LSR_THRE) == 0);

    HAL_WRITE_UINT8(base+CYG_DEV_THR, c);

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint32* base = ((channel_data_t*)__ch_data)->base;
    cyg_uint8 lsr;

    HAL_READ_UINT8(base+CYG_DEV_LSR, lsr);
    if ((lsr & SIO_LSR_DR) == 0)
        return false;

    HAL_READ_UINT8(base+CYG_DEV_RBR, *ch);

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

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        chan->irq_state = 1;
	HAL_WRITE_UINT8(chan->base+CYG_DEV_IER, SIO_IER_RCV|SIO_IER_UUE);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = chan->irq_state;
        chan->irq_state = 0;
	HAL_WRITE_UINT8(chan->base+CYG_DEV_IER, SIO_IER_UUE);
        HAL_INTERRUPT_MASK(chan->isr_vector);
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
    char c;
    cyg_uint8 lsr;
    CYGARC_HAL_SAVE_GP();

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    HAL_READ_UINT8(chan->base+CYG_DEV_LSR, lsr);
    if ( (lsr & SIO_LSR_DR) != 0 ) {

        HAL_READ_UINT8(chan->base+CYG_DEV_RBR, c);
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

	// Initialize UART1 for App Core usage
    // Disable interrupts.
    HAL_INTERRUPT_MASK(plf_ser_channels[0].isr_vector);

    // Init channels
    cyg_hal_plf_serial_init_channel(&plf_ser_channels[0]);


    // Setup procs in the vector table
    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &plf_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);


#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
	// Initialize UART0 if it was enabled by the build
    HAL_INTERRUPT_MASK(plf_ser_channels[1].isr_vector);
    cyg_hal_plf_serial_init_channel(&plf_ser_channels[1]);
    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &plf_ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
#endif

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
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

/*---------------------------------------------------------------------------*/

cyg_uint8 cyg_hal_led_segment[16] = {
    DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3,
    DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
    DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B,
    DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F };

void
hal_diag_led(int n)
{
#ifdef CYGSEM_HAL_ARM_IQ8134X_USE_PCE1
    if (n <= 0xf)
        HAL_WRITE_UINT8(DISPLAY_RIGHT, cyg_hal_led_segment[n & 0x0f]);
    else if (n == 0x10) {
        cyg_uint32 left_led, right_led;
        // Toggle Period
        HAL_READ_UINT8(DISPLAY_LEFT, left_led);
        HAL_READ_UINT8(DISPLAY_RIGHT, right_led);
        left_led ^= DISPLAY_PERIOD;
        right_led ^= DISPLAY_PERIOD;
        HAL_WRITE_UINT8(DISPLAY_RIGHT, right_led);
        HAL_WRITE_UINT8(DISPLAY_LEFT, left_led);
    }
#endif
}

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
