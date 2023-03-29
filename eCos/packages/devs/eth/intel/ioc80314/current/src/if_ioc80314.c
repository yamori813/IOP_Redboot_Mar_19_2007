//==========================================================================
//
//      if_ioc80314.c
//
//	Intel IOC80314 ethernet driver
//
//==========================================================================
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or
// other sources, and are covered by the appropriate copyright
// disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    cebruns
// Contributors: Ron Spence, Pacific Softworks, jskov, hmt, gthomas
// Date:         2003-04-29
// Purpose:      
// Description:  hardware driver for Intel IOC80314 gigabit ethernet
// Notes:        
//               I copied this from the i82544 (IQ80321 platform).  
//
// Copyright:    (C) 2003-2004 Intel Corporation.
//####DESCRIPTIONEND####
//
//==========================================================================
#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_intel_ioc80314.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
// So we can check the validity of the PCI window against the MLTs opinion,
// and thereby what the malloc heap consumes willy-nilly:
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif


#include <cyg/devs/eth/ioc80314_info.h>
#include CYGDAT_DEVS_ETH_INTEL_IOC80314_INL

// Added for FL_EMU Platform
extern EthPortInfo_t EthDriverInfo[];
extern int net_debug;
extern bool use_bootp;

#define QID 3

void ioc80314_check_wait(struct eth_drv_sc *sc);

// ------------------------------------------------------------------------


#ifdef CYGDBG_USE_ASSERTS
static struct {
    int can_send;
    int deliver;
    int stats;
    int waitcmd_timeouts;
    int waitcmd_timeouts_cu;
    int lockup_timeouts;
    int bad_cu_idles;
} missed_interrupt = { 0,0,0, 0,0, 0, 0 };
#endif

const EthSourceIsrPtr_t EthIsrTable[ETH_MAX_NUM_INTR_SOURCES] = 
  {
/* interrupt status bit 31. */ NULL,
/* interrupt status bit 30. */ EthOcnErrIsr,
/* interrupt status bit 29. */ EthRxIdleIsr,
/* interrupt status bit 28. */ EthRxAbortIsr,
/* interrupt status bit 27. */ EthRxErrorIsr,
/* interrupt status bit 26. */ EthRxOverrunIsr,
/* interrupt status bit 25. */ EthRxThresholdIsr,
/* interrupt status bit 24. */ EthRxWaitIsr,
/* interrupt status bit 23. */ NULL,
/* interrupt status bit 22. */ NULL,
/* interrupt status bit 21. */ NULL,
/* interrupt status bit 20. */ NULL,
/* interrupt status bit 19. */ EthRxQueueIsr,
/* interrupt status bit 18. */ EthRxQueueIsr,
/* interrupt status bit 17. */ EthRxQueueIsr,
/* interrupt status bit 16. */ EthRxQueueIsr,
/* interrupt status bit 15. */ EthStatsCarryIsr,
/* interrupt status bit 14. */ NULL,
/* interrupt status bit 13. */ EthTxIdleIsr,
/* interrupt status bit 12. */ EthTxAbortIsr,
/* interrupt status bit 11. */ EthTxErrorIsr,
/* interrupt status bit 10. */ EthTxUnderrunIsr,
/* interrupt status bit 9. */  EthTxThresholdIsr,
/* interrupt status bit 8. */  EthTxWaitIsr,
/* interrupt status bit 7. */  NULL,
/* interrupt status bit 6. */  NULL,
/* interrupt status bit 5. */  NULL,
/* interrupt status bit 4. */  NULL,
/* interrupt status bit 3. */  EthTxQueueIsr,
/* interrupt status bit 2. */  EthTxQueueIsr,
/* interrupt status bit 1. */  EthTxQueueIsr,
/* interrupt status bit 0. */  EthTxQueueIsr,
  };
/* CPU id. */
UINT32 GigeCpuId = 1;


// ------------------------------------------------------------------------
//
//                             MEMORY ADDRESSING
// 
// ------------------------------------------------------------------------
// Macros for writing shared memory structures - no need for byte flipping

#define READMEM8(   _reg_, _val_ ) ((_val_) = *((volatile CYG_BYTE *)(_reg_)))
#define WRITEMEM8(  _reg_, _val_ ) (*((volatile CYG_BYTE *)(_reg_)) = (CYG_BYTE)(_val_))
#define READMEM16(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define WRITEMEM16( _reg_, _val_ ) (*((volatile CYG_WORD16 *)(_reg_)) = (CYG_WORD16)(_val_))
#define READMEM32(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD32 *)(_reg_)))
#define WRITEMEM32( _reg_, _val_ ) (*((volatile CYG_WORD32 *)(_reg_)) = (CYG_WORD32)(_val_))
#define READMEM64(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD64 *)(_reg_)))
#define WRITEMEM64( _reg_, _val_ ) (*((volatile CYG_WORD64 *)(_reg_)) = (CYG_WORD64)(_val_))

#define OUTL( _v_, _a_ ) WRITEMEM32( _a_, _v_ )

static inline cyg_uint32 INL(cyg_uint32 io_address)
 {   cyg_uint32 _t_; READMEM32( io_address, _t_ ); return _t_;   }


// ------------------------------------------------------------------------
// Use arrays provided by platform header to verify pointers.

#ifdef CYGDBG_USE_ASSERTS
#define CHECK_NDP_SC_LINK()                                             \
    CYG_MACRO_START                                                     \
    int i, valid_netdev = 0, valid_sc = 0;                              \
    for(i = 0; i < CYGNUM_DEVS_ETH_INTEL_IOC80314_DEV_COUNT; i++) {       \
        if (ioc80314_netdev_array[i] == ndp) valid_netdev = 1;            \
        if (ioc80314_sc_array[i] == sc) valid_sc = 1;                     \
        if (valid_sc || valid_netdev) break;                            \
    }                                                                   \
    CYG_ASSERT( valid_netdev, "Bad ndp" );                              \
    CYG_ASSERT( valid_sc, "Bad sc" );                                   \
    CYG_ASSERT( (void *)p_ioc80314 == ioc80314_sc_array[i]->driver_private, \
                "sc pointer bad" );                                     \
    CYG_MACRO_END
#else
#define CHECK_NDP_SC_LINK()
#endif

#define IF_BAD_IOC80314( _p_ )                                             \
if (({                                                                  \
    int i, valid_p = 0;                                                 \
    for(i = 0; i < CYGNUM_DEVS_ETH_INTEL_IOC80314_DEV_COUNT; i++) {       \
        if (ioc80314_priv_array[i] == (_p_)) {                            \
            valid_p = 1;                                                \
            break;                                                      \
        }                                                               \
    }                                                                   \
    CYG_ASSERT(valid_p, "Bad pointer-to-ioc80314");                       \
    (!valid_p);                                                         \
}))

// ------------------------------------------------------------------------
//
//                FUNCTION PROTOTYPES
//
// ------------------------------------------------------------------------
static void ioc80314_reset (struct ioc80314* p_ioc80314);
static int  eth_set_mac_address (struct ioc80314* p_ioc80314, cyg_uint8 *addr); 
void start_timing(void);
void stop_timing (cyg_uint32 placeholder);

// ------------------------------------------------------------------------
//
//                NETWORK INTERFACE INITIALIZATION
//
//  Function : ioc80314_init
//
//  Description :
//       This routine resets, configures, and initializes the chip.
//       It also clears the ethernet statistics structure, and selects
//       which statistics are supported by this driver.
//
// ------------------------------------------------------------------------

static bool
ioc80314_init(struct cyg_netdevtab_entry * ndp)
{
    struct eth_drv_sc *sc;
    struct ioc80314 *p_ioc80314;
	unsigned int phy_address;

	sc = (struct eth_drv_sc *)(ndp->device_instance);
    p_ioc80314 = (struct ioc80314 *)(sc->driver_private);
	(p_ioc80314->port_id) ? (phy_address = 1) : (phy_address = 0);
    p_ioc80314->scPtr = &sc;
    p_ioc80314->gige_drv = &EthDriver;

#ifdef PKG_HAL_HAS_MULT_CPU
	// Eth driver already init'd by CPU0 - just return
	if (_curProc == 1) {
        (sc->funs->eth_drv->init)(sc, NULL );
		return 0;
	}
#endif
	if (net_debug == true) 
	    db_printf("<ioc80314_init> on port: %d\n", p_ioc80314->port_id);
    
    // Reset device
    ioc80314_reset(p_ioc80314);

    if (p_ioc80314->hardwired_esa) {
        // Hardwire the address without consulting the EEPROM.
        // When this flag is set, the p_ioc80314 will already contain
        // the ESA. Copy it to a mac_address for call to set_mac_addr
        eth_set_mac_address(p_ioc80314, p_ioc80314->mac_address);

    } else {
        // Acquire the ESA either from extenal means (probably RedBoot
        // variables) or from the attached EEPROM - if there is one.
        eth_set_mac_address(p_ioc80314, NULL);
    }

	if (net_debug == TRUE) {
    db_printf("<ioc80314_init> MAC Address = %02X %02X %02X %02X %02X %02X\n",
              p_ioc80314->mac_address[0], p_ioc80314->mac_address[1],
              p_ioc80314->mac_address[2], p_ioc80314->mac_address[3],
              p_ioc80314->mac_address[4], p_ioc80314->mac_address[5]);
	}

    // and record the net dev pointer
    p_ioc80314->ndp = (void *)ndp;
    p_ioc80314->within_send = 0; // init recursion level

    // Initialize upper level driver
    if ( p_ioc80314->mac_addr_ok )
        (sc->funs->eth_drv->init)(sc, &(p_ioc80314->mac_address[0]) );
    else
        (sc->funs->eth_drv->init)(sc, NULL );

#ifdef DEBUG
    db_printf("End of <ioc80314_init> on port: %d\n", p_ioc80314->port_id);
#endif

    return (1);
}


// ------------------------------------------------------------------------
//
//  Function : ioc80314_start
//
// ------------------------------------------------------------------------

static void 
ioc80314_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags )
{
    struct ioc80314 *p_ioc80314;
    cyg_uint32 status;
	
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

	if (net_debug == true) 
	    db_printf("<ioc80314_start> on port: %d\n", p_ioc80314->port_id);

    if ( ! p_ioc80314->mac_addr_ok ) {
#ifdef DEBUG
        db_printf("<ioc80314_start> %d: invalid MAC address, "
                  "can't bring up interface\n",
                  p_ioc80314->index );
        return;      
#endif
     }

    if ( p_ioc80314->active )
        ioc80314_stop( sc );

  status = EthDrvInit();
   if(status != ETH_OK){
#ifdef DEBUG
      db_printf("<ioc80314_start>, <EthDrvInit> returns %d\n", status);
  #endif
    return;
  } 


    // Initialize the Hardware
    status = EthConfigPort(p_ioc80314->port_id, &EthDriverInfo[p_ioc80314->port_id]);
    if (status != 0) 
    {
		if (net_debug == true)
	        db_printf("<ioc80314_start> return from <EthConfigPort> failed with Status = %d\n", status);
		db_printf ("Could not establish link - check cable?\n");
        p_ioc80314->active = 0;
        return;
    }    

	if (use_bootp) {
		db_printf ("Sending BOOTP Request....\n");
	}

    // Enable device
    p_ioc80314->active = 1;
    status = ioc80314intMpicInit(p_ioc80314);


    if (status != 0) 
    {
#ifdef DEBUG
        db_printf("<ioc80314_start> ioc80314intMpicInit return failed with Status = %d\n", status);
#endif
    }
    else
    {
#ifdef DEBUG
        db_printf("<ioc80314_start> ioc80314intMpicInit return passed\n");
#endif
    }

#ifdef DEBUG
    db_printf("End of <ioc80314_start> on port: %d\n", p_ioc80314->port_id);
#endif

}

// ------------------------------------------------------------------------
//
//  Function : ioc80314_stop
//
// ------------------------------------------------------------------------

static void
ioc80314_stop( struct eth_drv_sc *sc )
{
    cyg_uint32 status;
    struct ioc80314 *p_ioc80314;
    
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

#ifdef DEBUG
    db_printf("<ioc80314_stop> on port_id: %d\n", p_ioc80314->port_id);
#endif

#ifdef DEBUG
    db_printf("<ioc80314_stop> %d flg %x\n", p_ioc80314->index, *(int *)p_ioc80314 );
#endif


    // Reset Port
    status = EthResetPort(p_ioc80314->port_id);
    if (status != 0) 
    {
	if (net_debug == true)
	    db_printf("<ioc80314_stop> EthResetPort return failed with status: %d\n", status);
    }
    else
    {
         p_ioc80314->active = 0;               // stop people tormenting it
#ifdef DEBUG
        db_printf("<ioc80314_stop> EthResetPort return passed with Status = %d\n", status);
#endif
         
    }

/* Remove GigE ISR here. */
  status = intMpicReset();
  if(status != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("<ioc80314_stop>, <intMpicReset> returns %d\n", status);
    #endif // DEBUG_OVI
  } 

  status = EthDrvReset();
  if(status != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("<ioc80314_stop>, <EthDrvReset> returns %d\n", status);
    #endif // DEBUG_OVI
    return;
  }
    
#ifdef DEBUG
    db_printf("End of <ioc80314_stop> on port: %d\n", p_ioc80314->port_id);
#endif
    
}


// ------------------------------------------------------------------------
//
//  Function : ioc80314_recv
//
// ------------------------------------------------------------------------
static void 
ioc80314_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len )
{
    struct ioc80314 *p_ioc80314;
    SpacEthRtnCode_t rtnCode = ETH_OK;
    volatile cyg_uint8*  dest;
    volatile cyg_uint8*  src;
    EthSVFramePtr_t frmPtr, oldHeadPtr;
    cyg_uint32  lenSrc;
    cyg_uint32  lenDst;

    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

    frmPtr = oldHeadPtr = (EthSVFramePtr_t)p_ioc80314->head;

    if (frmPtr == NULL) 
    {
#ifdef DEBUG
    db_printf("<ioc80314_recv> RX list integrity error or empty.\n");
#endif
    }

//#ifdef DEBUG_OVI
//	db_printf ("<Receive> on Port: %d with sg_Len of: %d\n", p_ioc80314->port_id, sg_len);
//#endif

    src = frmPtr->buffPtr;
    dest = (unsigned char *)sg_list->buf;
    lenSrc = frmPtr->buffLen;
    lenDst = sg_list->len;
	if (dest == 0x0) {
        frmPtr = frmPtr->nextPtr;
        p_ioc80314->head = frmPtr;
	}
	else {
	    while (sg_len > 0)  //    for (idx =0; idx < sg_len;) 
	    {
	        if (lenSrc > lenDst) 
	        {
	            memcpy ((char*)dest, (char*)src, lenDst);
	            sg_list++;
	            sg_len--;
	            dest = (unsigned char*)sg_list->buf;
	            lenSrc -= lenDst;
	            src += lenDst;
	            lenDst = sg_list->len;
	        }
	        else if (lenDst > lenSrc)
	        {
	            memcpy ((char *)dest, (char *)src, lenSrc);
	            frmPtr = frmPtr->nextPtr;
	            p_ioc80314->head = frmPtr;
	            src = frmPtr->buffPtr;
	            dest += lenSrc;
	            lenDst -= lenSrc;
	            lenSrc = frmPtr->buffLen;            
	        }
	        else
	        {
	            memcpy ((char *)dest, (char *)src, lenSrc);
	            frmPtr = frmPtr->nextPtr;
	            p_ioc80314->head = frmPtr;
	            lenSrc = frmPtr->buffLen;
	            sg_list++;
	            sg_len--;
	            lenDst = sg_list->len;
	            src = frmPtr->buffPtr;
	            dest = (unsigned char*)sg_list->buf;
	        }
	    }
	}    
/* return copied buffers to the proprietary pool manager. */
    rtnCode = EthReleaseFrm(oldHeadPtr);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <ioc80314_recv>, <EthReleaseFrm> returns %d.", rtnCode);
      #endif  // DEBUG_OVI
      return;
    } 

/* list maintainance */
    if (p_ioc80314->head == NULL ) 
    {
        p_ioc80314->tail = NULL;
    }
}





// ------------------------------------------------------------------------
//
//  Function : ioc80314_can_send
//
// ------------------------------------------------------------------------

static int 
ioc80314_can_send(struct eth_drv_sc *sc)
{
    struct ioc80314 *p_ioc80314;
    unsigned int freeEntries;

    
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

    if ( p_ioc80314->active )
    {
        // all QID are going to be 3 for now - cebruns
        freeEntries = p_ioc80314->gige_drv->port[p_ioc80314->port_id].mac.tx.txQ[QID].memPool.freeEntries;
        return (freeEntries > 5);
    }
    else 
    {
	if (net_debug == true) 
	    db_printf("<ioc80314_can_send> port: %d is not active.\n", p_ioc80314->port_id);
        return (0);
    }


    return false;

}

// ------------------------------------------------------------------------
//
//  Function : ioc80314_send
//
// ------------------------------------------------------------------------

static void 
ioc80314_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list, int sg_len, int total_len,
            unsigned long key)
{
    struct ioc80314 *p_ioc80314;
//    struct eth_drv_sg *last_sg;
//    cyg_int32 txt;
    cyg_int32 status;
    EthTxFrmCfgInfo_t txFrmCfgInfo;
    EthMacMaxFrm_t macMaxFrmInfo; 
    
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

    if ( ! p_ioc80314->active )
        return;                         // device inactive, no return

    txFrmCfgInfo.vlanData = 0x0000;
    *((UINT32 *)&(txFrmCfgInfo.txFrmStatusCfg)) = 0x00000000;
    
    txFrmCfgInfo.txFrmStatusCfg.crc = ETH_BIT_SET; 
    txFrmCfgInfo.txFrmStatusCfg.padFrame = ETH_BIT_SET;
    
    // If VLAN tag is contained in frame, the QID would be extracted from the priority field
    txFrmCfgInfo.txFrmStatusCfg.descrIntr = 
      p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->txInfo.txQInfo[QID].descrInfo.descrIntr;
    
//  If VLAN tag is contained in frame, 
//    if( VLAN tag contained in frame ) {
//      txFrmCfgInfo.vlanData = vLanTag;
//      txFrmCfgInfo.txFrmStatusCfg.pfVlan = ETH_BIT_SET;
//    }

    status = EthRegAccess(p_ioc80314->port_id, ETH_MAC_MAX_FRAME_REG, 
      (UINT32 *)&(macMaxFrmInfo), REG_READ);
    if(status != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <ioc80314_send>, <EthRegAccess> access %d to %08X \
          for port %d returns %d.", REG_READ, ETH_MAC_MAX_FRAME_REG, 
          p_ioc80314->port_id, status);
      #endif  // DEBUG_OVI
      return;
    }
    
    if(total_len > macMaxFrmInfo.maxFrmLen){
      txFrmCfgInfo.txFrmStatusCfg.hugeFrame = ETH_BIT_SET;
    }

    status = ioc80314PutFrame(p_ioc80314->port_id, QID, sg_list, sg_len, &txFrmCfgInfo, p_ioc80314, key );
    if (status != 0) {
		#ifdef DEBUG
		db_printf ("<ioc80314_send> PutFrame failed with status: %d\n", status);
    	#endif
        (sc->funs->eth_drv->tx_done)( sc, key, 1 );
    }
	else
	 	(sc->funs->eth_drv->tx_done)(sc, key, 0);
}

// ------------------------------------------------------------------------
//
//  Function : ioc80314_reset
//
// ------------------------------------------------------------------------
static void
ioc80314_reset(struct ioc80314* p_ioc80314)
{
    cyg_uint32 port_id = p_ioc80314->port_id;
    cyg_uint32 status;

	if (net_debug == true)
	    db_printf("<ioc80314_reset> on port_id: %d\n", port_id);

    // FL_EMU
    status =  EthResetPort(port_id);
    if (status != 0)  
    {
		if (net_debug == TRUE) {
	        db_printf("<ioc80314_reset> Reset Port fails return non-zero: %d\n", status);
		}
    }    

#ifdef DEBUG 
    db_printf("End of <ioc80314_reset>\n");
#endif


}


// ------------------------------------------------------------------------
//
//                       INTERRUPT HANDLERS
//
// ------------------------------------------------------------------------

extern UINT32 GigeCpuId;

/* IACK and EOI register. Discriminated at init time. */
static UINT32 GigeIrqIack = HAL_IOC80314_CSR_BASE + ETH_MPIC_IACK3_REG;
static UINT32 GigeIrqEoi =  HAL_IOC80314_CSR_BASE + ETH_MPIC_EOI3_REG;


/*=========================================================================
 *  Function    : SpacIrqIsr
 =========================================================================*/
static cyg_uint32
ioc80314eth_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    
  SpacEthRtnCode_t rtnCode = ETH_OK; 
  UINT32 irqVal = 0x00000000;
  UINT32 iAckVal = 0x00000000;  
  struct eth_drv_sc *sc;
  struct ioc80314* p_ioc80314 = (struct ioc80314 *)data;
  sc = p_ioc80314->scPtr;
  
 
/* IACK cycle. */
    iAckVal = *(REG32_PTR(GigeIrqIack));  
    irqVal = iAckVal & ETH_MPIC_IACK_MASK;  
    if(irqVal == ETH_MPIC_GIGE_VECTOR)
    {
/* Call a service function. */
     rtnCode = EthIsr( (void*)sc );
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI2
          db_printf("<ioc80314eth_isr>, ISR handler at vector %d returns %d.\n", 
            irqVal, rtnCode);
        #endif  // DEBUG_OVI
      } 
    }else{        
      #ifdef DEBUG_OVI2
        db_printf("<ioc80314eth_isr>, not handled interrupt, IACK is %08X.\n", 
          iAckVal);      
      #endif  // DEBUG_OVI
    }
/* EOI cycle. */
    *(REG32_PTR(GigeIrqEoi)) = ETH_MPIC_EOI_MASK;  

  return CYG_ISR_CALL_DSR;        // schedule DSR
  
}




// ------------------------------------------------------------------------

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
static void
ioc80314eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct ioc80314* p_ioc80314 = (struct ioc80314 *)data;
    struct cyg_netdevtab_entry *ndp =
        (struct cyg_netdevtab_entry *)(p_ioc80314->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );

    cyg_drv_interrupt_acknowledge(p_ioc80314->vector);


}
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                              
// ------------------------------------------------------------------------
// Deliver routine:

void
ioc80314_deliver(struct eth_drv_sc *sc)
{
    cyg_uint32 status;

    // Check all qIDs with RedBootRx
    status = EthRedBootRx ( sc );

#ifdef DEBUG
    if (status != 0) 
    {
        db_printf("<ioc80314_deliver> returned from EthRedbootRx with status %d\n", status);
    }
#endif
    
}

// ------------------------------------------------------------------------
// Device table entry to operate the chip in a polled mode.
// Only diddle the interface we were asked to!

void
ioc80314_poll(struct eth_drv_sc *sc)
{
    struct ioc80314 *p_ioc80314;
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;


    if (p_ioc80314->active == 0) 
    {
        return;
    }    
    // As it happens, this driver always requests the DSR to be called:
    ioc80314eth_isr( p_ioc80314->vector, (cyg_addrword_t)p_ioc80314 );


    // (no harm in calling this ints-off also, when polled)
    ioc80314_deliver( sc );

    // Now call ioc80314RedBootFreeTx to check for free Tx frames
    ioc80314RedBootFreeTx ( sc ); 

	/* We're polling the Eth port - check that we didn't get a WAIT condition while doing other commands in RedBoot */
	ioc80314_check_wait( sc );

}

// ------------------------------------------------------------------------
// We're polling the Eth port - check that we didn't get a WAIT condition 
// while doing other commands in RedBoot
// This can also happen if a ping flood comes in - the Rx Queue goes offline on us
void
ioc80314_check_wait(struct eth_drv_sc *sc)
{
	struct ioc80314 *p_ioc80314;
	unsigned int* p_q3ptr;

	p_ioc80314 = (struct ioc80314 *)sc->driver_private;

	if (p_ioc80314->port_id == 0)
		p_q3ptr = (unsigned int*)0x501063BC;
	else
		p_q3ptr = (unsigned int*)0x501067BC;

	if ((*p_q3ptr & 0x80000000) == 0) {
		if (net_debug == TRUE)
			db_printf ("RX Queue was offline due to resources (must have been blocked by serial port)\n");
		*p_q3ptr = 0x80000000;
	}
}


// ------------------------------------------------------------------------
// Determine interrupt vector used by a device - for attaching GDB stubs
// packet handler.

int
ioc80314_int_vector(struct eth_drv_sc *sc)
{
    struct ioc80314 *p_ioc80314;

    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

#ifdef DEBUG
    db_printf ("<ioc80314_int_vector> on portId: %d\n", p_ioc80314->port_id);
#endif
    return (p_ioc80314->vector);
}



// ------------------------------------------------------------------------
//
//  Function : eth_set_mac_address
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------

static int
eth_set_mac_address(struct ioc80314* p_ioc80314, cyg_uint8 *addr) {
	cyg_uint32 *mac_addr_reg_lower, *mac_addr_reg_upper;

	// Default config - Port0 being used
	mac_addr_reg_upper = (cyg_uint32 *)(HAL_IOC80314_CSR_BASE + ETH_MAC_ADDR_PART1_REG);
	mac_addr_reg_lower = (cyg_uint32 *)(HAL_IOC80314_CSR_BASE + ETH_MAC_ADDR_PART2_REG);

	
	if (p_ioc80314->active)  {
#ifdef DEBUG
		db_printf("<eth_set_mac_address> : Port %d is active: must stop first\n", p_ioc80314->port_id);
#endif
		return (1);
	}
	if (addr == NULL) {  /* Need to verify MAC was in EEPROM */
		if (p_ioc80314->port_id) {	/* We're using Port1 - use the proper offset to get the MAC_ADDR */
			mac_addr_reg_upper = (cyg_uint32 *)(HAL_IOC80314_CSR_BASE + ETH_MAC_ADDR_PART1_REG + ETH_PORT_OFFSET);
			mac_addr_reg_lower = (cyg_uint32 *)(HAL_IOC80314_CSR_BASE + ETH_MAC_ADDR_PART2_REG + ETH_PORT_OFFSET);
		}
		if (!(*mac_addr_reg_upper | (*mac_addr_reg_lower >> 16))) { /* No MAC address read in from EEPROM */
			if (net_debug == TRUE)
				db_printf ("MAC Address not in EEPROM - can't enable port: %d\n", p_ioc80314->port_id);
			return (1);
		}
		else {	 /* MAC address read from EEPROM - put it in the driver */
#ifdef DEBUG
			db_printf ("<eth_set_mac_address> MAC address in EEPROM for Port: %d\n", p_ioc80314->port_id);
			db_printf ("<eth_set_mac_address> MAC address upper: 0x%0x, MAC address lower: 0x%0x\n", *mac_addr_reg_upper, (*mac_addr_reg_lower >> 16));
#endif
			p_ioc80314->mac_address[5]=p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet6 = (cyg_uint8)(*mac_addr_reg_upper >> 24);
			p_ioc80314->mac_address[4]=p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet5 = (cyg_uint8)(*mac_addr_reg_upper >> 16);
			p_ioc80314->mac_address[3]=p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet4 = (cyg_uint8)(*mac_addr_reg_upper >> 8);
			p_ioc80314->mac_address[2]=p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet3 = (cyg_uint8)(*mac_addr_reg_upper);
			p_ioc80314->mac_address[1]=p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart2Info.octet2 = (cyg_uint8)(*mac_addr_reg_lower >> 24);
			p_ioc80314->mac_address[0]=p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart2Info.octet1 = (cyg_uint8)(*mac_addr_reg_lower >> 16);
			EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet6 = p_ioc80314->mac_address[5];
			EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet5 = p_ioc80314->mac_address[4];
			EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet4 = p_ioc80314->mac_address[3];
			EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet3 = p_ioc80314->mac_address[2];
			EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart2Info.octet2 = p_ioc80314->mac_address[1];
			EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart2Info.octet1 = p_ioc80314->mac_address[0];
			p_ioc80314->mac_addr_ok = 1;
		}
	}
	else {
		if (net_debug == TRUE)  {
			db_printf ("<eth_set_mac_address> Mac Address defined by Redboot on Port: %d\n", p_ioc80314->port_id);
			db_printf ("<eth_set_mac_address> MAC address upper: 0x%0x, MAC address lower: 0x%0x\n", *mac_addr_reg_upper, (*mac_addr_reg_lower >> 16));
		}
		// record the MAC address in the device structure
		p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet6 = addr[5];
		p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet5 = addr[4];
		p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet4 = addr[3];
		p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart1Info.octet3 = addr[2];
		p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart2Info.octet2 = addr[1];
		p_ioc80314->gige_drv->portInfo[p_ioc80314->port_id]->macInfo.addrPart2Info.octet1 = addr[0];
		EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet6 = p_ioc80314->mac_address[5];
		EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet5 = p_ioc80314->mac_address[4];
		EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet4 = p_ioc80314->mac_address[3];
		EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart1Info.octet3 = p_ioc80314->mac_address[2];
		EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart2Info.octet2 = p_ioc80314->mac_address[1];
		EthDriverInfo[p_ioc80314->port_id].macInfo.addrPart2Info.octet1 = p_ioc80314->mac_address[0];

		p_ioc80314->mac_addr_ok = 1;

	}

if (net_debug == TRUE) {
	db_printf( "<eth_set_mac_address> Set MAC Address = %02X %02X %02X %02X %02X %02X (ok %d)\n",
	p_ioc80314->mac_address[5],
	p_ioc80314->mac_address[4],
	p_ioc80314->mac_address[3],
	p_ioc80314->mac_address[2],
	p_ioc80314->mac_address[1],
	p_ioc80314->mac_address[0],
	p_ioc80314->mac_addr_ok       );
}
#ifdef DEBUG
	db_printf("END of <eth_set_mac_address>\n");
#endif

	return p_ioc80314->mac_addr_ok ? 0 : 1;
}

// ------------------------------------------------------------------------
//
//  Function : eth_get_mac_address
//
// ------------------------------------------------------------------------
#ifdef ETH_DRV_GET_MAC_ADDRESS
static int
eth_get_mac_address(struct ioc80314* p_ioc80314, char *addr)
{
#ifdef DEBUG
    db_printf("<eth_get_mac_address>\n");
#endif
    
    memcpy( addr, (char *)(&p_ioc80314->mac_address[0]), 6 );

#ifdef DEBUG
	db_printf("END of <eth_get_mac_address>\n");
#endif

    return 0;
}
#endif
// ------------------------------------------------------------------------
//
//  Function : ioc80314_ioctl
//
// ------------------------------------------------------------------------
static int
ioc80314_ioctl(struct eth_drv_sc *sc, unsigned long key,
             void *data, int data_length)
{
    struct ioc80314 *p_ioc80314;
    
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;

#ifdef DEBUG
    db_printf("<ioc80314_ioctl> on port_id: %d\n", p_ioc80314->port_id);
#endif

#ifdef DEBUG
    db_printf( "<ioc80314_ioctl>: device eth%d at %x; key is 0x%x, data at %x[%d]\n",
               p_ioc80314->index, p_ioc80314, key, data, data_length );
#endif

    switch ( key ) {

#ifdef ETH_DRV_SET_MAC_ADDRESS
    case ETH_DRV_SET_MAC_ADDRESS:
        if ( 6 != data_length )
            return -2;
        return eth_set_mac_address( p_ioc80314, data ); // 1 /* do write eeprom */ );
#endif

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        return eth_get_mac_address( p_ioc80314, data );
#endif

    default:
        break;
    }
    return -1;
}

// ------------------------------------------------------------------------
//
//  Function : ioc80314PutFrame
//
// ------------------------------------------------------------------------
SpacEthRtnCode_t ioc80314PutFrame(
  unsigned portId, 
  unsigned qId,
  struct eth_drv_sg *sg_list, 
  int sg_len,
  EthTxFrmCfgInfoPtr_t txFrmCfgInfoPtr,
  struct ioc80314 *p_ioc80314,
  unsigned long key
  
){
  struct eth_drv_sg *last_sg;
  SpacEthRtnCode_t rtnCode = ETH_OK;
  int idx = 0;   

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("<ioc80314PutFrame>, invalid port id = %d.\n", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(qId >= ETH_NUM_TX_Q){
    #ifdef DEBUG_OVI
      db_printf("<ioc80314PutFrame>, invalid queue id = %d\n", qId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(txFrmCfgInfoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("<ioc80314PutFrame>, \
             invalid TX config/status pointer = %p.\n", txFrmCfgInfoPtr);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(sg_list == NULL){
    #ifdef DEBUG_OVI
      db_printf("<ioc80314PutFrame>, \
             invalid sg_list pointer = %p.\n", sg_list);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(sg_len == 0){
	#ifdef DEBUG_OVI
      db_printf("In <ioc80314PutFrame>, \
             invalid sg_len parameter = %d.\n", sg_len);
	#endif
    return ETH_PARAM_ERR;
  }   

  
  { 
    EthTxFrmStatusCfgPtr_t txFrmCfgPtr = NULL;
    EthVlanBCntPtr_t txVlanBCntPtr = NULL;
    EthMemPoolPtr_t memPoolPtr = 
      &(EthDriver.port[portId].mac.tx.txQ[qId].memPool);
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
    UINT32 ocnPortId = 0xFFFFFFFF;      
    EthQPtrHi_t qPtrHi;
    *((UINT32 *)&qPtrHi) = 0x00000000;
    
    if (memPoolPtr->freeEntries < sg_len)
    {
    #ifdef DEBUG_OVI
      db_printf("<ioc80314PutFrame> freeEntries < sg_len (fe: %d, sg_len:%d)\n", memPoolPtr->freeEntries, sg_len);
    #endif  // DEBUG_OVI
    return ETH_Q_FULL_ERR;
    
    }
    for(last_sg = &sg_list[sg_len]; sg_list < last_sg; idx++, sg_list++){  

/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx]), ETH_DESCR_SIZE, ETH_RX);      
/* Point to the descriptor's Config dword. */    
      txFrmCfgPtr = 
        (EthTxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].cfgStatus);        
/* If the "owner" bit of the pointed descriptor is own by DMA, or the 
   data buffer address is not NULL, the ring is full. It shouldn't happen if 
   checking for available space has ben done prior to call this routine. */
      if((txFrmCfgPtr->owner == ETH_DESCR_DMA) || 
         (memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0 != 0x00000000) || 
         ((memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000)){
        #ifdef DEBUG_OVI
          db_printf("In <ioc80314PutFrame>, TX queue full.\n");
        #endif  // DEBUG_OVI
        return ETH_Q_FULL_ERR;    
      }else{
/* Cache synchronization. This may be done before reaching this point. */    
        EthSynchCache((void *)sg_list->buf, sg_list->len, ETH_TX); 
/* Virtual-to-OCN address translation. */    
        rtnCode = sysVirtualToOcn((void *)sg_list->buf,
          (unsigned int*)&memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1, 
          (unsigned int*)&memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0, &ocnPortId);

/*
		db_printf ("<PUTFRAME> sysVirtToOCN for buf: 0x%08x returns: Start0: 0x%08x, Start1: 0x%08x, Port: 0x%08x\n", \
					sg_list->buf, memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr0, \
					memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].startAddr1, ocnPortId);
*/
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("In <ioc80314PutFrame>, <sysVirtualToOcn> returns %d.\n", rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
   
/* This should be atomic. */
        memPoolPtr->freeEntries--;

/* Frame configuration. */    
        txFrmCfgPtr->descrIntr = txFrmCfgInfoPtr->txFrmStatusCfg.descrIntr;
        txVlanBCntPtr = 
          (EthVlanBCntPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx].vlanBCnt);
        txVlanBCntPtr->byteCount = sg_list->len;    
                 
        if(idx == 0){ /* sof. */
        
          txFrmCfgPtr->padFrame = txFrmCfgInfoPtr->txFrmStatusCfg.padFrame;
          txFrmCfgPtr->hugeFrame = txFrmCfgInfoPtr->txFrmStatusCfg.hugeFrame;
          txFrmCfgPtr->crc = txFrmCfgInfoPtr->txFrmStatusCfg.crc;          
          txFrmCfgPtr->pfVlan = txFrmCfgInfoPtr->txFrmStatusCfg.pfVlan;
          txFrmCfgPtr->sof = ETH_BIT_SET;   
          if(txFrmCfgPtr->pfVlan){
            txVlanBCntPtr->vlanTag = txFrmCfgInfoPtr->vlanData;
          }   
        }         
        if(idx == sg_len - 1){ /* eof. */
          txFrmCfgPtr->eof = ETH_BIT_SET;
          p_ioc80314->tx_keys[qId][memPoolPtr->putInIdx] = key;
        }

/* This should be the last step for each descriptor. */      
        txFrmCfgPtr->owner = ETH_DESCR_DMA;       
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->putInIdx]), ETH_DESCR_SIZE, ETH_TX);      
/* Update "PUT-IN" index. */
        memPoolPtr->putInIdx = (++memPoolPtr->putInIdx) % numDescr;
       }
    }
    
/* Re-validate the queue. */  
    qPtrHi.valid = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, 
      ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), 
      (UINT32 *)&qPtrHi, REG_OR);

    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("<ioc80314PutFrame>, <EthRegAccess> access %d to %08X \
          returns %d.\n", REG_OR, 
          ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
      #endif  // DEBUG_OVI
      
      return rtnCode;
    }

  }  
  return rtnCode;
}


/*=========================================================================
 *  Function    : ioc80314TxDone
 *
 =========================================================================*/
SpacEthRtnCode_t ioc80314TxDone(
  UINT32 portId, 
  UINT32 qId,  
  EthMemPoolPtr_t memPoolPtr,
  struct ioc80314* p_ioc80314
){
  
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314TxDone>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(qId >= ETH_NUM_TX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314TxDone>, invalid queue id = %d.", qId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(memPoolPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314TxDone>, port id %d, queue id %d, \
             invalid memory pool pointer = %p.", portId, qId, memPoolPtr);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

    ndp = (struct cyg_netdevtab_entry *)(p_ioc80314->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

  {
  
/* Point to the descriptor's Config/Status dword. */    
    EthTxFrmStatusCfgPtr_t txFrmStatusCfgPtr = 
      (EthTxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].cfgStatus);
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
    
/* Cache synchronization. */    
    EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);          
/* If the "owner" bit of the pointed descriptor is not own by DMA and the 
   data buffer address is not NULL, the ring has entries to be freed. */
    while((txFrmStatusCfgPtr->owner != ETH_DESCR_DMA) && 
       ((memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr0 != 0x00000000) || 
       ((memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000))){        
/* Update statistics, if EOF. */  
      if(txFrmStatusCfgPtr->eof == ETH_BIT_SET){
        EthGetTxFrameStatus(txFrmStatusCfgPtr);
        (sc->funs->eth_drv->tx_done)( sc, p_ioc80314->tx_keys[qId][memPoolPtr->takeOutIdx], 0 );
      } 
/* Clear descriptor configuration/status. */      
      *((UINT32 *)txFrmStatusCfgPtr) = 0x00000000;       
/* Clear VLAN/Byte count configuration. */      
      memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].vlanBCnt = 0x00000000;
/* This should be the last clean-up step for each descriptor. */      
      memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr0 = 0x00000000;   
      memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].startAddr1 &= ~ETH_DESCR_ADDR_HI_MASK;   
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx]), ETH_DESCR_SIZE, ETH_TX);           
/* This should be atomic. */
      memPoolPtr->freeEntries++;
            
/* Update "TAKE-OUT" index. */
      memPoolPtr->takeOutIdx = (++memPoolPtr->takeOutIdx) % numDescr;
/* Point to the next descriptor's Config dword. */    
      txFrmStatusCfgPtr = 
        (EthTxFrmStatusCfgPtr_t)&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx].cfgStatus); 
/* Cache synchronization. */    
      EthSynchCache(&(memPoolPtr->descrPoolPtr[memPoolPtr->takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);           
    } 
     
  }

  return rtnCode;
    
}


/*=========================================================================
 *  Function    : ioc80314EthRxDemux
 =========================================================================*/
SpacEthRtnCode_t 	ioc80314EthRxDemux(
  struct eth_drv_sc *sc,
  UINT32 qId,
  UINT32 length,
  EthSVFramePtr_t frmPtr
)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  struct ioc80314* p_ioc80314 = (struct ioc80314 *)(sc->driver_private);
  UINT32 portId = p_ioc80314->port_id;

  if(frmPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314EthRxDemux>, invalid frame pointer = %p.", frmPtr);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314EthRxDemux>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    goto discard;
  } 

  if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314EthRxDemux>, invalid queue id = %d.", qId);
    #endif  // DEBUG_OVI
    goto discard;
  } 

  if((EthDriver.portInfo[portId]->txInfo.txCtrlInfo.qEnables & (1 << qId)) == 
    FALSE){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314EthRxDemux>, disabled queue id = %d.", qId);
    #endif  // DEBUG_OVI
    goto discard;
  }
  
  if ( p_ioc80314->head == NULL ) 
  {
    p_ioc80314->head = p_ioc80314->tail = frmPtr;
    ((EthSVFramePtr_t)(p_ioc80314->head))->nextFrm = ((EthSVFramePtr_t)(p_ioc80314->tail))->nextFrm = NULL;
  }
  else
  {   
    if (((EthSVFramePtr_t)(p_ioc80314->head))->nextFrm == NULL) 
    {
        ((EthSVFramePtr_t)(p_ioc80314->head))->nextFrm = frmPtr;
    }
    ((EthSVFramePtr_t)(p_ioc80314->tail))->nextFrm = frmPtr;
     p_ioc80314->tail = frmPtr;
    ((EthSVFramePtr_t)(p_ioc80314->tail))->nextFrm = NULL;
  }
  (sc->funs->eth_drv->recv)( sc, length );


  discard:
  return rtnCode;
    
}


/*=========================================================================
 *  Function    : ioc80314QChk
 =========================================================================*/
SpacEthRtnCode_t ioc80314QChk(
  struct eth_drv_sc *sc 
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  struct ioc80314* p_ioc80314 = (struct ioc80314 *)(sc->driver_private);
  
  if(p_ioc80314->port_id >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314QChk>, invalid port id = %d.", p_ioc80314->port_id);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  {
    int qIdx;
    EthRxCtrlPtr_t rxCtrlInfoPtr = &(EthDriver.portInfo[p_ioc80314->port_id]->rxInfo.rxCtrlInfo);
    
    for(qIdx = (ETH_NUM_RX_Q - 1); qIdx >= 0; qIdx--){
      if(rxCtrlInfoPtr->qEnables & (1 << qIdx)){            
/* Get new received frames. */      
        rtnCode = EthQRx(p_ioc80314->port_id, qIdx );
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <ioc80314QChk>, <EthQRx> for queue %d \
              returns %d.", qIdx, rtnCode);
          #endif  // DEBUG_OVI
/* Give a chance to service next queue.       
        return rtnCode; */
        }
      }
     else
      {
      #ifdef DEBUG_OVI 
        db_printf("\nIn <ioc80314QChk>, port id %d, queue id %d disabled", 
                     p_ioc80314->port_id, qIdx );  
      #endif  // DEBUG_OVI
        
      }
    }
  }
  return rtnCode;
    
}


/*=========================================================================
 *  Function    : EthQRx
 =========================================================================*/
SpacEthRtnCode_t ioc80314EthQRx(
 struct eth_drv_sc *sc,
 UINT32 qId
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  struct ioc80314* p_ioc80314 = (struct ioc80314 *)(sc->driver_private);

  if(p_ioc80314->port_id >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314EthQRx>, invalid port id = %d.", p_ioc80314->port_id);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314EthQRx>, invalid queue id = %d.", qId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  {
    EthSVFramePtr_t framePtr = NULL;   
    BOOL descrIntr = EthDriver.portInfo[p_ioc80314->port_id]->rxInfo.rxQInfo[qId].descrInfo.descrIntr;
 
/* Get new received frames. */      
    rtnCode = ioc80314EthGetFrame(sc, qId);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <ioc80314EthQRx>, port id %d, queue id %d, <EthGetFrame> returns %d.", 
          p_ioc80314->port_id, qId, rtnCode);
      #endif  // DEBUG_OVI
//      return rtnCode; 
    }
   
/* Feed with new frame buffers. */      
    rtnCode = EthGetNewBuffers(&(EthDriver.port[p_ioc80314->port_id].mac.rx.rxQ[qId].memPool), 
      &framePtr);
    if((rtnCode == ETH_OK) && (framePtr != NULL)){
      rtnCode = EthFeedRxQ(p_ioc80314->port_id, qId, &framePtr, descrIntr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <ioc80314EthQRx>, port id %d, queue id %d, <EthFeedRxQ> returns %d.", 
            p_ioc80314->port_id, qId, rtnCode);
        #endif  // DEBUG_OVI
        if(framePtr != NULL){
          rtnCode = EthReleaseFrm(framePtr);
          if(rtnCode != ETH_OK){ /* shouldn't happen. */
            #ifdef DEBUG_OVI
            db_printf("\nIn <ioc80314EthQRx>, port id %d, queue id %d, <EthReleaseFrm> returns %d.", 
              p_ioc80314->port_id, qId, rtnCode);
            #endif  // DEBUG_OVI
            return rtnCode; 
          }    
        }
// do not return here. give EthGetFrame a chance.        
        return rtnCode;  
      }
    }else if(framePtr != NULL){
      rtnCode = EthReleaseFrm(framePtr);
      if(rtnCode != ETH_OK){ /* shouldn't happen. */
        #ifdef DEBUG_OVI
        db_printf("\nIn <ioc80314EthQRx>, port id %d, queue id %d, <EthReleaseFrm> returns %d.", 
          p_ioc80314->port_id, qId, rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode; 
      }    
    }
  }
  return rtnCode;
    
}


/*=========================================================================
 *  Function    : ioc80314EthGetFrame
 =========================================================================*/
SpacEthRtnCode_t ioc80314EthGetFrame(
struct eth_drv_sc *sc,
UINT32 qId
){

	SpacEthRtnCode_t rtnCode = ETH_OK;
	EthRxQPtr_t rxQPtr = NULL;
	struct ioc80314* p_ioc80314 = (struct ioc80314 *)(sc->driver_private);
	UINT32 portId = p_ioc80314->port_id;
	UINT32 length = 0;

//db_printf ("<ioc80314EthGetFrame>\n");
	if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
		db_printf("\nIn <ioc80314EthGetFrame>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
	}

	if(qId >= ETH_NUM_RX_Q){
    #ifdef DEBUG_OVI
		db_printf("\nIn <ioc80314EthGetFrame>, invalid queue id = %d.", qId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
	}

	rxQPtr = &(EthDriver.port[portId].mac.rx.rxQ[qId]);

	if(rxQPtr->memPool.qHead == NULL){ /* Ring empty or header list insane. */

    #ifdef DEBUG_OVI
		db_printf("\nIn <ioc80314EthGetFrame>, port id %d, queue id %d, list integrity error.", portId, qId);
    #endif  // DEBUG_OVI
    
		return ETH_Q_INTEGRITY_ERR;
	}

	{
		/* Point to the descriptor's Config dword. */
		EthRxFrmStatusCfgPtr_t rxFrmStatusCfgPtr =
		(EthRxFrmStatusCfgPtr_t)&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].cfgStatus);
		EthSVFramePtr_t walkingFrmPtr = NULL;
		EthSVFramePtr_t qHeadFrmPtr = NULL;
		EthVlanBCntPtr_t vlanBCntPtr = NULL;
		BOOL foundEof = FALSE;
		UINT16 numDescr =
		EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;

		if(rxQPtr->prevFrmCellPtr != NULL){
			walkingFrmPtr = rxQPtr->prevFrmCellPtr->nextPtr;
		}else{
			walkingFrmPtr = (EthSVFramePtr_t)(rxQPtr->memPool.qHead);
		}

		/* Cache synchronization. */
		EthSynchCache(&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);
		/* If the "owner" bit of the pointed descriptor is not own by DMA and the 
		data buffer address is not NULL, the ring has entries to be reassembled. */
		while((rxFrmStatusCfgPtr->owner != ETH_DESCR_DMA) &&
		((rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr0 != 0x00000000) ||
		((rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr1 & ETH_DESCR_ADDR_HI_MASK) != 0x000))){

			/* Point to the descriptor's VlanBcount dword. */
			vlanBCntPtr =
			(EthVlanBCntPtr_t)&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].vlanBCnt);

			/* SOF or SOFEOF */
			if(rxFrmStatusCfgPtr->sof == ETH_BIT_SET){
				if((rxQPtr->invalidFrm == TRUE) ||
				(rxQPtr->prevFrmCell == ETH_FRM_CELL) ||
				(rxQPtr->prevFrmCell == ETH_FRM_SOF)){ /* Invalid cell sequence. */
					if(rxQPtr->prevFrmCell != ETH_FRM_EOF){
						EthIncrRxErr();

					}
					if(rxQPtr->prevFrmCellPtr != NULL){

						qHeadFrmPtr = (EthSVFramePtr_t)(rxQPtr->memPool.qHead);
						/* Update head/tail pointers. */
						rxQPtr->memPool.qHead = walkingFrmPtr;
						if(rxQPtr->memPool.qHead == NULL){
							rxQPtr->memPool.qTail = NULL;
						}
						rxQPtr->prevFrmCellPtr->nextPtr = NULL;
						/* Release frame. */
						rtnCode = EthReleaseFrm(qHeadFrmPtr);
						/* Clear the per-frame cached info. */
						rxQPtr->prevFrmCellPtr = NULL;
						rxQPtr->prevFrmCell = ETH_FRM_UNKNOWN;
						rxQPtr->invalidFrm = FALSE;
						rxQPtr->rxSofFrmStatus = 0x00000000;
						rxQPtr->vLanTag = 0x0000;

						if(rtnCode != ETH_OK){ /* shouldn't happen. */
              #ifdef DEBUG_OVI
							db_printf("\nIn <ioc80314EthGetFrame>, port id %d, queue id %d, <EthReleaseFrm> returns %d.", 
							portId, qId, rtnCode);
              #endif  // DEBUG_OVI
/* Do not return. Allow proceding with the next frame.      
              return rtnCode; */
						}
					}else{ /* shouldn't happen. */

            #ifdef DEBUG_OVI
						db_printf("\nIn <ioc80314EthGetFrame>, port id %d, queue id %d, RX queue integrity error.", 
						portId, qId);
            #endif  // DEBUG_OVI
            return ETH_Q_INTEGRITY_ERR;
					}
				}
				/* Save data in case this frame may be completed in a next call. */
				rxQPtr->rxSofFrmStatus = *((UINT32 *)rxFrmStatusCfgPtr);
				if(rxFrmStatusCfgPtr->vtf == ETH_BIT_SET){
					rxQPtr->vLanTag = vlanBCntPtr->vlanTag;
				}
				rxQPtr->prevFrmCell = ETH_FRM_SOF;
				if(rxFrmStatusCfgPtr->eof == ETH_BIT_SET){/* SOFEOF. */
					foundEof = TRUE;
					rxQPtr->prevFrmCell = ETH_FRM_SOFEOF;
				}
				rxQPtr->invalidFrm = FALSE;
				/* This could potentially invalidate the frame. */
				EthGetRxFrameStatus(rxQPtr);
				length = 0;
			}
			/* CELL or EOF */
			else if(rxFrmStatusCfgPtr->sof == ETH_BIT_CLEARED){
				if((rxQPtr->prevFrmCell != ETH_FRM_SOF) &&
				((rxQPtr->prevFrmCell != ETH_FRM_CELL) ||
				(rxQPtr->invalidFrm == TRUE))){ /* Invalid cell sequence. */
					rxQPtr->invalidFrm = TRUE;
				}
				if(rxFrmStatusCfgPtr->eof == ETH_BIT_CLEARED){ /* CELL */
					rxQPtr->prevFrmCell = ETH_FRM_CELL;
				}else{ /* EOF */
					if(rxQPtr->invalidFrm == TRUE){
						EthIncrRxErr();

					}
					foundEof = TRUE;
					rxQPtr->prevFrmCell = ETH_FRM_EOF;
				}
			}
			walkingFrmPtr->buffLen = vlanBCntPtr->byteCount;
			length += walkingFrmPtr->buffLen;

			/* Cache synchronization. */
			EthSynchCache((void *)walkingFrmPtr->buffPtr, walkingFrmPtr->buffLen, ETH_RX);
			/* Clear frame configuration. */
			*((UINT32 *)rxFrmStatusCfgPtr) = 0x00000000;
			/* Clear VLAN/Byte count configuration. */
			rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].vlanBCnt = 0x00000000;
			/* This should be the last clean-up step for each descriptor. */
			rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr0 = 0x00000000;
			rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].startAddr1 &= ~ETH_DESCR_ADDR_HI_MASK;
			/* Cache synchronization. */
			EthSynchCache(&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx]), ETH_DESCR_SIZE, ETH_TX);
			/* This should be atomic. */
			rxQPtr->memPool.freeEntries++;

			/* Update "TAKE-OUT" index. */
			rxQPtr->memPool.takeOutIdx = (++(rxQPtr->memPool.takeOutIdx)) % numDescr;
			/* Cache synchronization. */
			EthSynchCache(&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx]), ETH_DESCR_SIZE, ETH_RX);
			/* Point to the descriptor's Config dword. */
			rxFrmStatusCfgPtr =
			(EthRxFrmStatusCfgPtr_t)&(rxQPtr->memPool.descrPoolPtr[rxQPtr->memPool.takeOutIdx].cfgStatus);
			/* Update "previous" and "current" pointers. */
			rxQPtr->prevFrmCellPtr = walkingFrmPtr;
			walkingFrmPtr = walkingFrmPtr->nextPtr;
			/* Call a "demux" routine to present the extracted frame to a registered 
			client. */
			if((foundEof == TRUE) && (rxQPtr->invalidFrm == FALSE)){
				EthIncrRx();
				qHeadFrmPtr = (EthSVFramePtr_t)rxQPtr->memPool.qHead;
				/* Update head/tail pointers. */
				rxQPtr->memPool.qHead = walkingFrmPtr;
				if(rxQPtr->memPool.qHead == NULL){

					rxQPtr->memPool.qTail = NULL;
				}
				rxQPtr->prevFrmCellPtr->nextPtr = NULL;

				/* Demux frame. */
				rtnCode = ioc80314EthRxDemux(sc, qId, length, qHeadFrmPtr);
				/* Clear the per-frame cached info. */
				rxQPtr->prevFrmCellPtr = NULL;
				rxQPtr->prevFrmCell = ETH_FRM_UNKNOWN;
				rxQPtr->rxSofFrmStatus = 0x00000000;
				rxQPtr->vLanTag = 0x0000;
				if(rtnCode != ETH_OK){ /* shouldn't happen. */
          #ifdef DEBUG_OVI
					db_printf("\nIn <ioc80314EthGetFrame>, port id %d, queue id %d, <EthRxDemux> returns %d.", 
					portId, qId, rtnCode);
          #endif  // DEBUG_OVI
/* Do not return. Allow proceding with the next frame.              
          return rtnCode; */
				}
			}
		}
	}

	return rtnCode;

}


/*=========================================================================
 *  Function    : ioc80314RedBootFreeTx
 =========================================================================*/
SpacEthRtnCode_t ioc80314RedBootFreeTx(
struct eth_drv_sc *sc
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  struct ioc80314* p_ioc80314 = (struct ioc80314 *)(sc->driver_private);
  UINT32 portId = p_ioc80314->port_id;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314RedBootFreeTx>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  {
    int qIdx;
    EthTxCtrlPtr_t txCtrlInfoPtr = &(p_ioc80314->gige_drv->portInfo[portId]->txInfo.txCtrlInfo);
    
    for(qIdx = (ETH_NUM_TX_Q - 1); qIdx >= 0; qIdx--){
      if(txCtrlInfoPtr->qEnables & (1 << qIdx)){            
/* Free transmitted frames. */      
        rtnCode = ioc80314TxDone(portId, qIdx, &(p_ioc80314->gige_drv->port[portId].mac.tx.txQ[qIdx].memPool), p_ioc80314);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <ioc80314RedBootFreeTx>, <ioc80314TxDone> for queue %d \
              returns %d.", qIdx, rtnCode);
          #endif  // DEBUG_OVI
/* Give a chance to service next queue.       
        return rtnCode; */
        }
      }
    }
  }
  return rtnCode;
    
}



/*=========================================================================
 *  Function    : EthIsr
 *
 *  Arguments   : infoPtr - input, user info passed in.
 * 
 *  Description : GigE's ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : Port0 will get first chance to service its interrupts.
 *              
 =========================================================================*/
SpacEthRtnCode_t EthIsr(
  void * infoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  int idx;
  EthPortIntrStatus_t gigeIntrStatus[ETH_NUM_PORTS];
  static int portIdx = ETH_PORT0_ID;
  
  
  if(infoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthIsr>, invalid info pointer = %p.", infoPtr);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }
  
  for(idx = 0; idx < ETH_NUM_PORTS; idx++){  
  /* Read both ports' interrupt status register, to avoid spurious interrupts. */      
    rtnCode = EthRegAccess(idx, ETH_PORT_INT_STATUS_REG, 
      (UINT32 *)&(gigeIntrStatus[idx]), REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthIsr>, <EthRegAccess> access %d to %08X \
          for port %d returns %d.", REG_READ, ETH_PORT_INT_STATUS_REG, 
          idx, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
    if(gigeIntrStatus[idx].intrStatus == ETH_BIT_SET){
      rtnCode = EthRegAccess(idx, ETH_PORT_INT_STATUS_REG, 
        (UINT32 *)&(gigeIntrStatus[idx]), REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthIsr>, <EthRegAccess> access %d to %08X, \
            port %d, returns %d.", REG_WRITE, ETH_PORT_INT_STATUS_REG, 
            idx, rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;   
      } 
    }    
  } 
      
  for(idx = 0; idx < ETH_NUM_PORTS; idx++){
    if(gigeIntrStatus[portIdx].intrStatus == ETH_BIT_SET){
#ifdef DEBUG
        db_printf ("<EthIsr> Have an interrupt with idx: %d and portIdx: %d\n", idx, portIdx);    
#endif        
      rtnCode = EthPortIsr(portIdx, 
        *((UINT32 *)&(gigeIntrStatus[portIdx])) & ~ETH_PORT_INT_STATUS_MASK, 
        infoPtr);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthIsr>, <EthPortIsr> port %d returns %d.", 
            portIdx, rtnCode);
        #endif  // DEBUG_OVI
/* Give a chance to service next port.       
        return rtnCode; */  
      }   
    } 
    portIdx = (portIdx == ETH_PORT0_ID ? ETH_PORT1_ID : ETH_PORT0_ID);
  }

  portIdx = (portIdx == ETH_PORT0_ID ? ETH_PORT1_ID : ETH_PORT0_ID);

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthPortIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : intrStatus - Port interrupt status.
 *              : infoPtr - input, user info passed in. 
 * 
 *  Description : GigE's ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : Prior to returning, this routine, attempts to clear the 
 *              : interrupt sources.
 =========================================================================*/
SpacEthRtnCode_t EthPortIsr(
  UINT32 portId, 
  UINT32 intrStatus,   
  void * infoPtr
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 intrMask;
  intData interruptData;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPortIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if(infoPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPortIsr>, invalid info pointer = %p.", infoPtr);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  interruptData.sc = (struct eth_drv_sc *)infoPtr;
    
  rtnCode = EthRegAccess(portId, ETH_PORT_INT_MASK_REG, &intrMask, REG_READ);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthPortIsr>, <EthRegAccess> access %d to %08X \
        returns %d.", REG_READ, ETH_PORT_INT_MASK_REG, rtnCode);
    #endif  // DEBUG_OVI
    return rtnCode;
  } 

  {
    register unsigned idx, isrParam, loopCounter = 0;
    unsigned int statusBit;
/* If there are any interrupts, dispatch them. */      
    do{
/* Get the index into the EthIsrTable. */ 
      __asm volatile ("CLZ %0, %1": "=r" (idx) : "r" (intrStatus));
  		
      if(idx == ETH_ARM_CLZ_BIT_NOT_SET){
        break;
      }
      statusBit = ETH_MAX_NUM_INTR_SOURCES - (idx + 1);
/* If the interrupt is enabled, call its handler. */      
      if((intrMask & (1 << statusBit)) == 
        ETH_BIT_CLEARED){
        if(EthIsrTable[idx] != NULL){
          if((intrStatus & ETH_TX_Q_WAIT_INTR_MASK) ||
            (intrStatus & ETH_RX_Q_WAIT_INTR_MASK)){
                interruptData.statusPtr = &intrStatus;
            isrParam = (UINT32)(&interruptData);
          }else{
            interruptData.statusPtr = &statusBit;
            isrParam = (UINT32)(&interruptData);
          }
          rtnCode = EthIsrTable[idx](portId, isrParam);
          if(rtnCode != ETH_OK){
            #ifdef DEBUG_OVI
              db_printf("\nIn <EthPortIsr>, <EthIsrTable[%d]> returns %d.", 
                idx, rtnCode);
            #endif  // DEBUG_OVI
/* Give a chance to service next interrupt source.       
            return rtnCode; */  
          }
        }else{
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthPortIsr>, could not found handler for GigE \
              interrupt source %d.", statusBit);
          #endif  // DEBUG_OVI
/* Give a chance to service next interrupt source.       
          return rtnCode; */  
        }
      }
/* Clear the serviced interrupt source bit. */      
      intrStatus &= ~(1 << statusBit);
/* Paranoid redundancy mechanism, in case something is VERY wrong with the 
   CLZ instruction. */      
      loopCounter++;
    }while(loopCounter < ETH_MAX_NUM_INTR_SOURCES);
  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthOcnErrIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's OCN ERROR ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthOcnErrIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthOcnErrIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

/* The interrupt for Port1 should be masked, to reduce system overhead. */  
  if(portId == ETH_PORT1_ID){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthOcnErrIsr>, warning, OCN ERROR interrupt for port \
        id = %d is not masked.", portId);
    #endif  // DEBUG_OVI
//    return ETH_PARAM_ERR;
  } 
  
  {
    EthPortOcnStatus_t portOcnStatus;
    int idx;
    for(idx = 0; idx < ETH_NUM_PORTS; idx++){
      rtnCode = EthRegAccess(idx, ETH_PORT_OCN_STATUS_REG, 
        (UINT32 *)&portOcnStatus, REG_READ);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthOcnErrIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_READ, ETH_PORT_OCN_STATUS_REG, rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      }
      
      #ifdef DEBUG_OVI
      db_printf("\nIn <EthOcnErrIsr>, OCN error count = %d, %s OCN response \
        header is %08X.", portOcnStatus.ocnErrCount,
        (idx == ETH_PORT0_ID ? "first" : "last"), 
        portOcnStatus.ocnResponseHdr);
      #endif  // DEBUG_OVI
            
  /* Clear error count field(s). */  
      portOcnStatus.ocnErrCount = 0x00;
      rtnCode = EthRegAccess(idx, ETH_PORT_OCN_STATUS_REG, 
        (UINT32 *)&portOcnStatus, REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthOcnErrIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_PORT_OCN_STATUS_REG, rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      }
    }
  }
  
/* T.B.D what else to do here. */  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxIdleIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX IDLE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxIdleIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxIdleIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

  {
    EthRxStatus_t rxStatus;
/* Get the idle queue(s). */      
    rtnCode = EthRegAccess(portId, ETH_RX_STATUS_REG, (UINT32 *)&rxStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxIdleIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_STATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxIdleIsr>, RX queue idle field = %02X.", 
          rxStatus.qIdle);
      #endif  // DEBUG_OVI
    }
  }
        
/* Signal a semaphore, if needed. */  

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxAbortIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX ABORT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxAbortIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxAbortIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  #ifdef DEBUG_OVI
    db_printf("\nIn <EthRxAbortIsr>, port id %d, RX abort completed.", portId);
  #endif  // DEBUG_OVI
  
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxErrorIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX ERROR ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxErrorIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxErrorIsr>, invalid port id = %d.", portId);
    #endif
    return ETH_PARAM_ERR;
  } 
  
  {
  UINT32 qErrXStatus, errStatus; 
  int idx; 
/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &qErrXStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_XSTATUS_REG, rtnCode);
      #endif
      return rtnCode;
    }
    
/* Keep only the error status bits. */      
    qErrXStatus &= ETH_RX_Q_ERR_MASK;

/* Get the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_ERR_STATUS_REG, &errStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_ERR_STATUS_REG, rtnCode);
      #endif
      return rtnCode;
    }
    
    for(idx = (ETH_NUM_RX_Q - 1); idx >= 0; idx--){
      if(qErrXStatus & (ETH_RX_Q0_ERR_MASK << idx)){
        
#ifdef ETH_SW_WORK_AROUND_143 

/* Signal the queue's counting semaphore, if using a RTOS. Call the RX handler 
   if running RedBoot. */ 
#ifdef ETH_REDBOOT
        rtnCode = EthQRx(portId, idx);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, <EthQRx> for port %d, queue %d returns %d.", 
              portId, idx, rtnCode);
          #endif
  /* Give a chance to re-validate queue.       
          return rtnCode; */  
        } 
            
#else
        SpacSemSignal(&rxQCountSem[qId]);
#endif  
/* Account for the unsignaled EOQ condition. */      
        rtnCode = EthRxQueueIsr(portId, idx + ETH_RX_Q_INTR_STATUS_LO);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, <EthRxQueueIsr> for port %d, queue %d returns %d.", 
              portId, idx, rtnCode);
          #endif
  /* Give a chance to re-validate queue.       
          return rtnCode; */  
        } 
        

#endif  /*  ETH_SW_WORK_AROUND_143  */
                
/* Shouldn't happen, for now. */        
        if(errStatus & (ETH_RX_Q0_DESC_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, descriptor error in queue %d.", idx);
          #endif
        }
        if(errStatus & (ETH_RX_Q0_TIMEOUT_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, timeout error in queue %d.", idx);
          #endif
        }
        if(errStatus & (ETH_RX_Q0_READ_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, read error in queue %d, target \
              response is %02X", idx, 
              (errStatus & (ETH_RX_Q0_TRESP_ERR_MASK << (idx * ETH_BYTE_WIDTH))));
          #endif
        }
        if(errStatus & (ETH_RX_Q0_TEA_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxErrorIsr>, TEA error in queue %d.", idx);
          #endif
        }
      }
    }
    
/* Clear the que error extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &qErrXStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_XSTATUS_REG, rtnCode);
      #endif
      return rtnCode;
    }
/* Clear the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_ERR_STATUS_REG, &errStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_ERR_STATUS_REG, rtnCode);
      #endif
      return rtnCode;
    }
  }
    
/* According to the specs, chip reset may be needed. TBD. 
  EthReset(); 
*/    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxOverrunIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX OVERRUN ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxOverrunIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxOverrunIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  #ifdef DEBUG_OVI
    db_printf("\nIn <EthRxOverrunIsr>, port id %d, overrun condition.", portId);
  #endif  // DEBUG_OVI

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);  
  
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxOverrunIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
    return rtnCode;
  }

/* Increase DMA RX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_CLEARED, ETH_BIT_CLEARED);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxOverrunIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
    return rtnCode;
  }  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxThresholdIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX THRESHOLD ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthRxThresholdIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxThresholdIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);  
  
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxThresholdIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
    return rtnCode;
  }

/* Increase DMA RX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_CLEARED, ETH_BIT_CLEARED);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxThresholdIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
    return rtnCode;
  }
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxWaitIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's RX WAIT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : A WAIT condition is expected to happen if a frame was 
 *              : bigger than the space provided in the queue. 
 *              : if there are no more space to receive frames, abort the  
 *              : frame, clean-up and restart the queue. else, advance the 
 *              : DMA pointer and restart the queue.
 *              : Also, this handler clears the EOQ interrupt associated
 *              : with.
 =========================================================================*/
SpacEthRtnCode_t EthRxWaitIsr(
  UINT32 portId, 
  UINT32 info
){
     
  SpacEthRtnCode_t  rtnCode;
  struct eth_drv_sc *sc;

  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxWaitIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   
    
  if (((intData *)info) == NULL) {
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxWaitIsr>, invalid info pointer = %d.", info);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   

  {
    sc = ((intData *)info)->sc;
    EthRxStatus_t rxStatus;
    EthRxCtrl_t rxCtrl;
    EthQPtrHi_t qPtrHi;
    UINT32 qId;
    EthBuffDescrPtr_t descrPoolPtr = NULL;
    UINT16 numDescr = 0xFFFF;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0;
    INT32 sofBuffDescrIdx = 0;
    INT32 walkingBuffDescrIdx = 0;
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;
    SpacEthRegOps_t regOp = REG_OR;
    *((UINT32 *)&qPtrHi) = 0x00000000;  
    UINT32 loopExitCounter = 0;
    BOOL sysTermination = FALSE;
    UINT32 extIntrStatus;

/* Get the current queue. */      
    rtnCode = EthRegAccess(portId, ETH_RX_STATUS_REG, (UINT32 *)&rxStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_STATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 

    qId = rxStatus.currentQ;
    
/* Get the control info. */          
    rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&rxCtrl, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_CTRL_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }   
        
/* Because a WAIT is also a EOQ, clear the queue and EOQ interrupt bits. */      
    *(((intData *)info)->statusPtr) &= ~(ETH_RX_Q0_INTR_MASK << qId);
    extIntrStatus = (ETH_RX_EOQ_Q_X_INTR_MASK << qId);
/* Clear the EOQ extended status bit. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &extIntrStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 

/* only service enabled queueus. */
  if((rxCtrl.qEnables & (1 << qId)) != 0){
    descrPoolPtr = EthDriver.port[portId].mac.rx.rxQ[qId].memPool.descrPoolPtr;
    numDescr = EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;
        
/*  First, try to make some space, to reduce the chance of getting another WAIT 
    immediatelly. Signal the queue's counting semaphore, if using a RTOS. Call the 
    queue RX handler if running RedBoot. */ 
    rtnCode = ioc80314EthQRx( sc, qId );   // (portId, qId);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthQRx> for port %d, queue %d returns %d.", 
          portId, qId, rtnCode);
      #endif  // DEBUG_OVI
/* Give a chance to re-validate queue.       
        return rtnCode; */  
    } 
        
/* Find the SOF and last descriptor (processed by DMA). */  
    lastBuffDescrIdxIncr = sofBuffDescrIdx = 
      EthDriver.port[portId].mac.rx.rxQ[qId].memPool.takeOutIdx;  
    lastBuffDescrIdx = sofBuffDescrIdx - 1;
    if(lastBuffDescrIdx < 0){
      lastBuffDescrIdx = numDescr - 1;
    }          
/* Cache synchronization. */    
    EthSynchCache(&(descrPoolPtr[sofBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);   
    EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);   
                
    if(EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.descrTermMode == 
      ETH_DESCR_TERM_OWN){    
      if(((*((EthRxFrmStatusCfgPtr_t)&(descrPoolPtr[sofBuffDescrIdx].
        cfgStatus))).owner == ETH_DESCR_SYS) || 
        (descrPoolPtr[sofBuffDescrIdx].vlanBCnt == 0x00000000)){
        sysTermination = TRUE;
      }else if(descrPoolPtr[sofBuffDescrIdx].vlanBCnt != 0x00000000){
        lastBuffDescrIdx = sofBuffDescrIdx;
        lastBuffDescrIdxIncr = (lastBuffDescrIdx + 1) % numDescr;
  /* Cache synchronization. */    
        EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX); 
        loopExitCounter = 1;
        while((descrPoolPtr[lastBuffDescrIdxIncr].vlanBCnt != 0x00000000) && 
          (loopExitCounter < numDescr)){
          lastBuffDescrIdx = lastBuffDescrIdxIncr;
          lastBuffDescrIdxIncr = (lastBuffDescrIdxIncr + 1) % numDescr;
  /* Cache synchronization. */    
          EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX); 
          loopExitCounter++;                         
        }
        if(loopExitCounter == numDescr){
          lastBuffDescrIdx = sofBuffDescrIdx - 1;
          if(lastBuffDescrIdx < 0){
            lastBuffDescrIdx = numDescr - 1;
          }
          lastBuffDescrIdxIncr = sofBuffDescrIdx;
        }
      }
    }
      
/* If the queue was SYSTEM terminated or to small, for now, abort. Else, there may 
   be a chance that more available RX descriptors are chained. */ 
    if((sysTermination == TRUE) || (loopExitCounter != 0)){          
//      if(loopExitCounter1 < numDescr){
      if(loopExitCounter != 0){
/* Return the aborted buffers to DMA. */ 
        walkingBuffDescrIdx = sofBuffDescrIdx;
        do{
          (*((EthRxFrmStatusCfgPtr_t)&(descrPoolPtr[walkingBuffDescrIdx].
            cfgStatus))).owner = ETH_DESCR_DMA;
/* Clear VLAN/Byte count configuration. Needed in the WAIT ISR. */                
          descrPoolPtr[walkingBuffDescrIdx].vlanBCnt = 0x00000000;  
/* Cache synchronization. */    
          EthSynchCache(&(descrPoolPtr[walkingBuffDescrIdx]), ETH_DESCR_SIZE, ETH_TX);                                             
          walkingBuffDescrIdx = (walkingBuffDescrIdx + 1) % numDescr;
        }while(walkingBuffDescrIdx != lastBuffDescrIdxIncr);          
        lastBuffDescrIdx = sofBuffDescrIdx - 1;
        if(lastBuffDescrIdx < 0){
          lastBuffDescrIdx = numDescr - 1;
        }
        lastBuffDescrIdxIncr = sofBuffDescrIdx;        
      } 
            
      rxCtrl.enableAbortIntr = ETH_BIT_SET;   
      rxCtrl.abort = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&rxCtrl, 
        REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_RX_CTRL_REG, rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      } 
    }
   
/* Cache synchronization. */    
   EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);                          
   
/* Manually update the descriptor pointer, if NULL terminated, or aborted buffers
   were returned to DMA. */   
    if(((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
     ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)) ||
//     ((loopExitCounter1 < numDescr)))
     ((loopExitCounter != 0))){

      rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
        (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
      if(rtnCode != 0){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxWaitIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      } 
      rtnCode = EthRegAccess(portId, 
        ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), 
        (UINT32 *)&descrAddrLo, REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, 
            ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      } 
      regOp = REG_WRITE;
    }

    /* Re-validate the queue. If NULL terminated, qPtrHi contains the HI bits. */    
    qPtrHi.valid = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), (UINT32 *)&qPtrHi, regOp);

    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", regOp, 
          ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
          
  }else{
    rxCtrl.enableAbortIntr = ETH_BIT_SET;   
    rxCtrl.abort = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, ETH_RX_CTRL_REG, (UINT32 *)&rxCtrl, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_CTRL_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }   
  }

  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthRxQueueIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, used to retrieve the qeue id. 
 * 
 *  Description : GigE's RX QUEUE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : An EOQ condition is expected to happen if a frame was 
 *              : received, and DMA has encountered a LAST/NULL EOQ 
 *              : condition on the last processed descriptor. Update the 
 *              : queue pointer, clean-up and restart the queue. 
 =========================================================================*/
SpacEthRtnCode_t EthRxQueueIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  UINT32 extIntrStatus, rxQCfg, qId;
  struct eth_drv_sc *sc;

  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxQueueIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  
  if((intData*)info == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxQueueIsr>, invalid info pointer = %d.", info);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   
  
  if ((*(((intData *)info)->statusPtr) > ETH_RX_Q_INTR_STATUS_HI) || (*(((intData *)info)->statusPtr) < ETH_RX_Q_INTR_STATUS_LO)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRxQueueIsr>, invalid interrupt status bit = %d.", *(((intData *)info)->statusPtr));
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }else{
    qId = *(((intData *)info)->statusPtr) - ETH_RX_Q_INTR_STATUS_LO;
  } 
  
  {
    sc = ((intData *)info)->sc;
    EthQPtrHi_t qPtrHi;
    EthBuffDescrPtr_t descrPoolPtr = 
      EthDriver.port[portId].mac.rx.rxQ[qId].memPool.descrPoolPtr;
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->rxInfo.rxQInfo[qId].descrInfo.numDescr;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0;
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;    
    SpacEthRegOps_t regOp = REG_OR;    
    *((UINT32 *)&qPtrHi) = 0x00000000;
    UINT32 loopExitCounter = 0;      

/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &extIntrStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_RX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
/* Keep only the extended status bits specific to a particular queue. */  
    extIntrStatus &= (ETH_RX_Q_X_INTR_MASK << qId);
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_RX_XSTATUS_REG, &extIntrStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_RX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 
/* Get the config enable bits. */      
    rtnCode = EthRegAccess(portId, 
      ETH_RX_Q0_CFG_REG + (qId * ETH_RX_Q_REG_OFFSET), &rxQCfg, REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, 
          ETH_RX_Q0_CFG_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 
      
/* Descriptor interrupt. */  
    if((extIntrStatus & (ETH_RX_DESCR_Q_X_INTR_MASK << qId)) && 
      (rxQCfg & ETH_RX_DESCR_Q_INTR_EN_MASK)){

    }
/* EOF interrupt. */  
    if((extIntrStatus & (ETH_RX_EOF_Q_X_INTR_MASK << qId)) &&
      (rxQCfg & ETH_RX_EOF_Q_INTR_EN_MASK)){                
        
/* Signal the queue's counting semaphore, if using a RTOS. Call the RX handler 
   if running RedBoot. */ 
      rtnCode = ioc80314EthQRx(sc, qId);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxQueueIsr>, <EthQRx> for port %d, queue %d returns %d.", 
            portId, qId, rtnCode);
        #endif  // DEBUG_OVI
/* Give a chance to re-validate queue.       
        return rtnCode; */  
      } 
            
        

    }
/* EOQ interrupt. */  
    if((extIntrStatus & (ETH_RX_EOQ_Q_X_INTR_MASK << qId)) &&
      (rxQCfg & ETH_RX_EOQ_Q_INTR_EN_MASK)){
  
    }
            
    if((extIntrStatus & (ETH_RX_EOQ_Q_X_INTR_MASK << qId)) &&
      (rxQCfg & ETH_RX_EOQ_Q_INTR_EN_MASK)){    
        
  /* Find the last descriptor (processed by DMA). */  
      lastBuffDescrIdx = 
        EthDriver.port[portId].mac.rx.rxQ[qId].memPool.takeOutIdx;
  /* Cache synchronization. */    
      EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);            
        
      if(descrPoolPtr[lastBuffDescrIdx].vlanBCnt == 0x00000000){
        lastBuffDescrIdxIncr = lastBuffDescrIdx;
        lastBuffDescrIdx--;
        if(lastBuffDescrIdx < 0){
          lastBuffDescrIdx = numDescr - 1;
        }
      }else{
        loopExitCounter = 1;            
        lastBuffDescrIdxIncr = (lastBuffDescrIdx + 1) % numDescr;
  /* Cache synchronization. */    
        EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX);            
        while((descrPoolPtr[lastBuffDescrIdxIncr].vlanBCnt != 0x00000000) && 
         (loopExitCounter < numDescr)){
          lastBuffDescrIdx = lastBuffDescrIdxIncr;
          lastBuffDescrIdxIncr = (lastBuffDescrIdxIncr + 1) % numDescr;
  /* Cache synchronization. */    
          EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX);         
          loopExitCounter++;
         }
         if(loopExitCounter == numDescr){
           lastBuffDescrIdxIncr = EthDriver.port[portId].mac.rx.rxQ[qId].memPool.takeOutIdx;        
           lastBuffDescrIdx--;
           if(lastBuffDescrIdx < 0){
             lastBuffDescrIdx = numDescr - 1;
          }
        }
      }        
        
/* Manually update the descriptor pointer, if NULL terminated. */         
      if((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
       ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)){
        rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
          (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxQueueIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), 
          (UINT32 *)&descrAddrLo, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_RX_Q0_PTR_LO_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
        regOp = REG_WRITE;
      }
      /* Re-validate the queue. If NULL terminated, qPtrHi contains the HI bits. */    
      qPtrHi.valid = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, 
        ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), 
        (UINT32 *)&qPtrHi, regOp);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthRxQueueIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", regOp, 
            ETH_RX_Q0_PTR_HI_REG + (qId * ETH_RX_Q_REG_OFFSET), rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      }
    }
  }

  return rtnCode;   
    
}

/*=========================================================================
 *  Function    : EthStatsCarryIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's STATS CARRY ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthStatsCarryIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthStatsCarryIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  {
    UINT32 mstatCar1;
    UINT32 mstatCar2;
/* Get the Carry1 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR1_REG, &mstatCar1, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_MSTAT_CAR1_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          reads %08X.", REG_READ, ETH_MSTAT_CAR1_REG, *((UINT32 *)&mstatCar1));
      #endif  // DEBUG_OVI
    }
/* Clear the Carry1 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR1_REG, &mstatCar1, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MSTAT_CAR1_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
/* Get the Carry2 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR2_REG, &mstatCar2, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_MSTAT_CAR2_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          reads %08X.", REG_READ, ETH_MSTAT_CAR2_REG, *((UINT32 *)&mstatCar2));
      #endif  // DEBUG_OVI
    }
/* Clear the Carry2 status bits. */      
    rtnCode = EthRegAccess(portId, ETH_MSTAT_CAR2_REG, &mstatCar2, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthStatsCarryIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_MSTAT_CAR2_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }   
  }
  
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxIdleIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX IDLE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxIdleIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxIdleIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
 
  {
    EthTxStatus_t txStatus;
/* Get the idle queue(s). */      
    rtnCode = EthRegAccess(portId, ETH_TX_STATUS_REG, (UINT32 *)&txStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxIdleIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_STATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }else{
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxIdleIsr>, TX queue idle field = %02X.", 
          txStatus.qIdle);
      #endif  // DEBUG_OVI
    }
  }

/* Signal a semaphore, if needed.  */  


  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxAbortIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX ABORT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxAbortIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxAbortIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  #ifdef DEBUG_OVI
    db_printf("\nIn <EthTxAbortIsr>, port id %d, TX abort completed.", portId);
  #endif  // DEBUG_OVI
    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxErrorIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX ERROR ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxErrorIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxErrorIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
    
  {
  UINT32 xStatus, errStatus; 
  int idx; 
/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &xStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
/* Get the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_ERR_STATUS_REG, &errStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_ERR_STATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
        
    for(idx = (ETH_NUM_TX_Q - 1); idx >= 0; idx--){
      if(xStatus & (ETH_TX_Q0_ERR_MASK << idx)){
/* Missing/misplaced SOF. */        
        if(errStatus & (ETH_TX_Q0_DESC_ERR_MASK << (idx * ETH_BYTE_WIDTH))){          
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, descriptor error in queue %d.", idx);
          #endif  // DEBUG_OVI
          

        }
        if(errStatus & (ETH_TX_Q0_TIMEOUT_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
//          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, timeout error in queue %d.", idx);
//          #endif  // DEBUG_OVI
        }
        if(errStatus & (ETH_TX_Q0_READ_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
//          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, read error in queue %d, target \
              response is %02X", idx, 
              (errStatus & (ETH_TX_Q0_TRESP_ERR_MASK << (idx * ETH_BYTE_WIDTH))));
//          #endif  // DEBUG_OVI
        }
        if(errStatus & (ETH_TX_Q0_TEA_ERR_MASK << (idx * ETH_BYTE_WIDTH))){
//          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxErrorIsr>, TEA error in queue %d.", idx);
//          #endif  // DEBUG_OVI
        }
      }
    }
    
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &xStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
/* Clear the error status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_ERR_STATUS_REG, &errStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxErrorIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_ERR_STATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
  }
    
/* According to the specs, chip reset may be needed. TBD.  
  EthReset(); 
*/    
    
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxUnderrunIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX UNDERRUN ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxUnderrunIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;
  
  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxUnderrunIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  #ifdef DEBUG_OVI
    db_printf("\nIn <EthTxUnderrunIsr>, port id %d, underrun condition.", portId);
  #endif  // DEBUG_OVI
    

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED); 
   
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxUnderrunIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
  }

/* Increase DMA TX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_SET, ETH_BIT_SET);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxUnderrunIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
  }
  
  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxThresholdIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX THRESHOLD ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 =========================================================================*/
SpacEthRtnCode_t EthTxThresholdIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  EthBitField_t port0Pri; 
  EthBitField_t port1Pr;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxThresholdIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 
  

/* Increase this port priority. */      
  port0Pri = (portId == ETH_PORT0_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);
  port1Pr = (portId == ETH_PORT1_ID ? ETH_BIT_SET: ETH_BIT_CLEARED);  
  
  rtnCode = EthChangePortPri(port0Pri, port1Pr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxThresholdIsr>, <EthChangePortPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
  }

/* Increase DMA TX priority, on this port. */      
  rtnCode = EthChangeDmaPri(portId, ETH_BIT_SET, ETH_BIT_SET);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxThresholdIsr>, <EthChangeDmaPri> returns %d.", rtnCode);
    #endif  // DEBUG_OVI
  }
  

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxWaitIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, user info passed in. 
 * 
 *  Description : GigE's TX WAIT ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : As the driver checks for available space in the TX queue,
 *              : a WAIT condition is expected to happen if a frame was 
 *              : interrupted, and DMA has encountered an EOQ condition.
 *              : if the EOF was forgotten, abort the frame, clean-up
 *              : and restart the queue. if the frame continues, advance the 
 *              : DMA pointer and restart the queue.
 =========================================================================*/
SpacEthRtnCode_t EthTxWaitIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  struct eth_drv_sc *sc;
  struct ioc80314 *p_ioc80314;

  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxWaitIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if((intData *)info == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxWaitIsr>, invalid info pointer = %d.", info);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   

    
  {
    sc = ((intData *)info)->sc;
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;
    EthRxStatus_t txStatus;
    EthRxCtrl_t txCtrl;
    EthQPtrHi_t qPtrHi;
    UINT32 qId;
    EthBuffDescrPtr_t descrPoolPtr = NULL;
    UINT16 numDescr = 0xFFFF;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0;
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;    
    SpacEthRegOps_t regOp = REG_OR;        
    *((UINT32 *)&txCtrl) = 0x00000000;    
    *((UINT32 *)&qPtrHi) = 0x00000000;  
//    UINT16 loopExitCounter;  

/* Get the current queue. */      
    rtnCode = EthRegAccess(portId, ETH_TX_STATUS_REG, (UINT32 *)&txStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_STATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 
    qId = txStatus.currentQ;
    
/* Get the control info. */              
    rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&txCtrl, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_CTRL_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }   
    
  if((txCtrl.qEnables & (1 << qId)) != 0){
    descrPoolPtr = EthDriver.port[portId].mac.tx.txQ[qId].memPool.descrPoolPtr;
    numDescr = EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
              
/* Signal the queue's counting semaphore, if using a RTOS. Call the queue 
   RX handler if running RedBoot. */ 
    rtnCode = ioc80314TxDone (portId, qId, &(EthDriver.port[portId].mac.tx.txQ[qId].memPool), p_ioc80314);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <ioc80314TxDone> for port %d, queue %d \
          returns %d.", portId, qId, rtnCode);
      #endif  // DEBUG_OVI
/* give a chance to re-activate the queue        
        return rtnCode; */
    } 
/* Find the last descriptor. */  
// EthFreeTxFrame should give the lastBuffDescrIdx + 1.
    lastBuffDescrIdxIncr = 
      EthDriver.port[portId].mac.tx.txQ[qId].memPool.takeOutIdx;
    lastBuffDescrIdx = lastBuffDescrIdxIncr - 1;
    if(lastBuffDescrIdx < 0){
      lastBuffDescrIdx = numDescr - 1;
    }
      
/* Cache synchronization. */    
    EthSynchCache(&(descrPoolPtr[lastBuffDescrIdxIncr]), ETH_DESCR_SIZE, ETH_RX);      

/* If the queue was not NULL or LAST terminated, for now, abort. Else, there
   may be a chance that more available TX descriptors are chained. */ 
    if(
      ((*((EthTxFrmStatusCfgPtr_t)&(descrPoolPtr[lastBuffDescrIdxIncr].cfgStatus))).owner == 
        ETH_DESCR_SYS)){
      txCtrl.enableAbortIntr = ETH_BIT_SET;
      txCtrl.abort = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&txCtrl, 
        REG_WRITE);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", REG_WRITE, ETH_TX_CTRL_REG, rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      } 
    }else{
/* Cache synchronization. */    
      EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);      
/* Manually update the descriptor pointer, if NULL terminated. */   
      if((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
       ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)){
        rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
          (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxWaitIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&descrAddrLo, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
        regOp = REG_WRITE;
      }
/* Re-validate the queue. If NULL terminated, qPtrHi contains the HI bits. */    
      qPtrHi.valid = ETH_BIT_SET;   
      rtnCode = EthRegAccess(portId, 
        ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), 
        (UINT32 *)&qPtrHi, regOp);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
            returns %d.", regOp, 
            ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
        #endif  // DEBUG_OVI
        return rtnCode;
      }
    }
               
  
  }else{
    txCtrl.enableAbortIntr = ETH_BIT_SET;   
    txCtrl.abort = ETH_BIT_SET;   
    rtnCode = EthRegAccess(portId, ETH_TX_CTRL_REG, (UINT32 *)&txCtrl, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxWaitIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_CTRL_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }   
  }

  }

  return rtnCode;
    
}

/*=========================================================================
 *  Function    : EthTxQueueIsr
 *
 *  Arguments   : portId - Port ID (0, 1)
 *              : info - input, used to retrieve the qeue id. 
 * 
 *  Description : GigE's TX QUEUE ISR.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_BASE_ADDR_ERR - Invalid chip base address.
 *
 *  Note        : Use of "db_printf" macro to allow for non-blocking logging.
 *              : As the driver checks for available space in the TX queue,
 *              : a EOQ condition is expected to happen if  DMA has encountered
 *              : an ownership EOQ condition, after an EOF. Restart the queue.
 *              : Update the queue pointer if NULL termination.
 =========================================================================*/
SpacEthRtnCode_t EthTxQueueIsr(
  UINT32 portId, 
  UINT32 info
){
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  UINT32 extIntrStatus, txQCfg, qId;
  struct eth_drv_sc *sc;
  struct ioc80314 *p_ioc80314;
  


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxQueueIsr>, invalid port id = %d.", portId);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if((intData*)info == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxQueueIsr>, invalid info pointer = %d.", info);
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }   

  if ((*(((intData *)info)->statusPtr) > ETH_TX_Q_INTR_STATUS_HI) || (*(((intData *)info)->statusPtr) < ETH_TX_Q_INTR_STATUS_LO))  
  {
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthTxQueueIsr>, invalid interrupt status bit = %d.", *(((intData *)info)->statusPtr));
    #endif  // DEBUG_OVI
    return ETH_PARAM_ERR;
  }else{
    qId = *(((intData *)info)->statusPtr) - ETH_TX_Q_INTR_STATUS_LO;
  }  
  
  {
    sc = ((intData *)info)->sc;  
    p_ioc80314 = (struct ioc80314 *)sc->driver_private;    
    EthQPtrHi_t qPtrHi;
    EthBuffDescrPtr_t descrPoolPtr = 
      EthDriver.port[portId].mac.tx.txQ[qId].memPool.descrPoolPtr;
    UINT16 numDescr = 
      EthDriver.portInfo[portId]->txInfo.txQInfo[qId].descrInfo.numDescr;
    INT32 lastBuffDescrIdx = 0;
    INT32 lastBuffDescrIdxIncr = 0; 
    UINT32 descrAddrLo = 0x00000000;
    UINT32 ocnPortId = 0xFFFFFFFF;    
    *((UINT32 *)&qPtrHi) = 0x00000000;  

/* Get the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &extIntrStatus, 
      REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, ETH_TX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    }
/* Keep only the extended status bits specific to a particular queue. */  
    extIntrStatus &= (ETH_TX_Q_X_INTR_MASK << qId);
/* Clear the extended status bits. */      
    rtnCode = EthRegAccess(portId, ETH_TX_XSTATUS_REG, &extIntrStatus, 
      REG_WRITE);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_WRITE, ETH_TX_XSTATUS_REG, rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 
    
/* Get the config enable bits. */      
    rtnCode = EthRegAccess(portId, 
      ETH_TX_Q0_CFG_REG + (qId * ETH_TX_Q_REG_OFFSET), &txQCfg, REG_READ);
    if(rtnCode != ETH_OK){
      #ifdef DEBUG_OVI
        db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
          returns %d.", REG_READ, 
          ETH_TX_Q0_CFG_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
      #endif  // DEBUG_OVI
      return rtnCode;
    } 
    
/* This implementations uses only EOF/EOQ interrupts. */  
    if(((extIntrStatus & (ETH_TX_EOF_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOF_Q_INTR_EN_MASK)) || 
      ((extIntrStatus & (ETH_TX_EOQ_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOQ_Q_INTR_EN_MASK))){
/* Signal the queue's counting semaphore, if using a RTOS. Call the TX 
   free frames handler if running RedBoot. */ 
    rtnCode = ioc80314TxDone (portId, qId, &(EthDriver.port[portId].mac.tx.txQ[qId].memPool), p_ioc80314);
      if(rtnCode != ETH_OK){
        #ifdef DEBUG_OVI
          db_printf("\nIn <EthTxQueueIsr>, <ioc80314TxDone> for port %d, queue %d \
            returns %d.", portId, qId, rtnCode);
        #endif  // DEBUG_OVI
/* give a chance to update the queue pointer       
        return rtnCode; */
      } 

/* Find the last descriptor. */  
// EthFreeTxFrame should give the lastBuffDescrIdx + 1.
      lastBuffDescrIdxIncr = 
        EthDriver.port[portId].mac.tx.txQ[qId].memPool.takeOutIdx;
      lastBuffDescrIdx = lastBuffDescrIdxIncr - 1;
      if(lastBuffDescrIdx < 0){
        lastBuffDescrIdx = numDescr - 1;
      }
    } 

/* Descriptor interrupt. */  
    if((extIntrStatus & (ETH_TX_DESCR_Q_X_INTR_MASK << qId)) && 
      (txQCfg & ETH_TX_DESCR_Q_INTR_EN_MASK)){
    }
/* EOF interrupt. */  
    if((extIntrStatus & (ETH_TX_EOF_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOF_Q_INTR_EN_MASK)){
    }
/* EOQ interrupt. */  
    if((extIntrStatus & (ETH_TX_EOQ_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOQ_Q_INTR_EN_MASK)){
    }
    
    if((extIntrStatus & (ETH_TX_EOQ_Q_X_INTR_MASK << qId)) &&
      (txQCfg & ETH_TX_EOQ_Q_INTR_EN_MASK)){   
/* Cache synchronization. */    
      EthSynchCache(&(descrPoolPtr[lastBuffDescrIdx]), ETH_DESCR_SIZE, ETH_RX);      

/* Manually update the descriptor pointer, if NULL terminated. */   
      if((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr0 == 0x00000000) && 
       ((descrPoolPtr[lastBuffDescrIdx].nextDescrAddr1 & ETH_DESCR_ADDR_HI_MASK) == 0x000)){
        rtnCode = sysVirtualToOcn(descrPoolPtr + lastBuffDescrIdxIncr,
          (UINT32 *)&qPtrHi, (UINT32 *)&descrAddrLo, &ocnPortId);
        if(rtnCode != 0){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxQueueIsr>, <sysVirtualToOcn> returns %d.", rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&descrAddrLo, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_PTR_LO_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        } 
        rtnCode = EthRegAccess(portId, 
          ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), 
          (UINT32 *)&qPtrHi, REG_WRITE);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthTxQueueIsr>, <EthRegAccess> access %d to %08X \
              returns %d.", REG_WRITE, 
              ETH_TX_Q0_PTR_HI_REG + (qId * ETH_TX_Q_REG_OFFSET), rtnCode);
          #endif  // DEBUG_OVI
          return rtnCode;
        }
      }
    }
  }

  return rtnCode;
    
}


static int
ioc80314_int_attach( struct ioc80314 *p_ioc80314 )
{

#ifdef DEBUG
    db_printf("ioc80314_int_attach\n");
#endif


    p_ioc80314->vector = GIGE_INTERRUPT_VECTOR;
    p_ioc80314->interrupt_handle = 0; // Flag not attached.
    // FIXME - need interrupt vector retrieval function
    //            if (cyg_pci_translate_interrupt(&dev_info, &p_i82544->vector)) { 
#ifdef DEBUG
    db_printf(" Wired to HAL vector %d\n", p_ioc80314->vector);
#endif

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    cyg_drv_interrupt_create(
        p_ioc80314->vector,
        0,                  // Priority - unused FIXME - do we need?
        (CYG_ADDRWORD)p_ioc80314, // Data item passed to ISR & DSR
        ioc80314eth_isr,            // ISR
        ioc80314eth_dsr,            // DSR
        &p_ioc80314->interrupt_handle, // handle to intr obj
        &p_ioc80314->interrupt_object ); // space for int obj

    cyg_drv_interrupt_attach(p_ioc80314->interrupt_handle);

#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE


    // This is the indicator for "uses an interrupt"
    if (p_ioc80314->interrupt_handle != 0) {
        cyg_drv_interrupt_acknowledge(p_ioc80314->vector);
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
        cyg_drv_interrupt_unmask(p_ioc80314->vector);
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE


    }
    return 0;
}

/*=========================================================================
 *  Function    : ioc80314intMpicInit
 =========================================================================*/
SpacEthRtnCode_t ioc80314intMpicInit(struct ioc80314 *p_ioc80314)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;
  UINT32 cpsr;

  
  GigeCpuId = 
    (*(REG32_PTR(SpacBaseAddr + ETH_CIU_CFG_REG)) & ETH_CIU_PID_MASK) >> ETH_BYTE_WIDTH;
  
  if((GigeCpuId != ETH_CPU0_ID) && (GigeCpuId != ETH_CPU1_ID)){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314intMpicInit>, invalid CPU id = %d.", GigeCpuId);
    #endif  // DEBUG_OVI
    return ETH_INVAL_CFG_ERR;
  } 

  /* If Driver already init'd */
   if (EthDriver.intEnabled == 1) 
   {
    EthDriver.intEnabled++;
    return rtnCode;
   }
   else if (EthDriver.intEnabled == 2) 
   {
    #ifdef DEBUG_OVI
      db_printf("<EthintMpicInit>, can't init MPIC three times\n");
    #endif // DEBUG_OVI
   
    return ETH_PARAM_ERR;
   }

    return ETH_OK;   /* cebruns - FIXME FIXME - don't register interrupt yet */  
/* Make sure that interrupts are disabled */
  cpsr = _Read_CPSR;
  _Write_CPSR(cpsr | _IRQ_DISABLE | _FIQ_DISABLE);

  rtnCode = ioc80314_int_attach (p_ioc80314);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314intMpicInit>, <ioc80314_int_attach> for eth0 returns %d.", rtnCode);
    #endif  // DEBUG_OVI
  } 

/*  rtnCode = ioc80314_int_attach (&ioc80314_eth1_priv_data); //(0, ETH_MPIC_GIGE_VECTOR, EthIsr);
  if(rtnCode != ETH_OK){
    #ifdef DEBUG_OVI
      db_printf("\nIn <ioc80314intMpicInit>, <ioc80314_int_attach> for eth1 returns %d.", rtnCode);
    #endif  // DEBUG_OVI
  } 
*/  

/* Set IRQ to be enabled later */  
  cpsr &= ~_IRQ_DISABLE;     
  
/* Enable IRQs. */  
  _Write_INT_Control(ETH_XSCALE_INTCTL_MASK);
    
  if(GigeCpuId == ETH_CPU0_ID){
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_CSR2_REG)) = (1 << 1);  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_TASK2_REG)) = ETH_MPIC_TASK_MASK;  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IDR16_REG)) = ETH_MPIC_IDR_OUT3_MASK;
    GigeIrqIack = SpacBaseAddr + ETH_MPIC_IACK2_REG;
    GigeIrqEoi = SpacBaseAddr + ETH_MPIC_EOI2_REG;
    
  }else{
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_CSR3_REG)) = (1 << 1);  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_TASK3_REG)) = ETH_MPIC_TASK_MASK;  
    *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IDR16_REG)) = ETH_MPIC_IDR_OUT4_MASK;
    GigeIrqIack = SpacBaseAddr + ETH_MPIC_IACK3_REG;
    GigeIrqEoi = SpacBaseAddr + ETH_MPIC_EOI3_REG;
    
  }
  
  *(REG32_PTR(SpacBaseAddr + ETH_MPIC_IVPR16_REG)) = ETH_MPIC_IVPR_PRIO_MASK | 
    ETH_MPIC_IVPR_S_MASK | ETH_MPIC_IVPR_P_MASK | ETH_MPIC_GIGE_VECTOR;  
   
/* Enable interrupts */
  _Write_CPSR(cpsr);
    
  #ifdef DEBUG_OVI
    db_printf("\nIn <ioc80314intMpicInit>, GIGE ISRs routed to CPU%d.\n", GigeCpuId);
  #endif  // DEBUG_OVI

  EthDriver.intEnabled = 1;    
  return rtnCode;
  
}


  
/*=========================================================================
 *  Function    : intMpicReset
 *
 *  Arguments   : N/A.
 * 
 *  Description : Resets the MPIC block.
 * 
 *  return      : ETH_OK - Operation succeded.
 *
 *  Note        : for now GigE-oriented. 
 *           
 =========================================================================*/
SpacEthRtnCode_t intMpicReset(void)
{
  
  SpacEthRtnCode_t rtnCode = ETH_OK;

  if (EthDriver.intEnabled == 2) 
  {
    EthDriver.intEnabled--;
    return rtnCode;
  }
/* Need to detach interrupts here - FIXME cebruns
  gigeHandlerDetachIRQ();
*/  
  
#ifdef __GIGE_SDRAM_FIQ_DEBUG__     
  gigeHandlerDetachFIQ();
#endif
  
/*  *(REG32_PTR(SpacBaseAddr + ETH_MPIC_GCR_REG)) = TS_MPIC_GCR_R; */
  EthDriver.intEnabled--; 
  return rtnCode;
  
}

/*=========================================================================
 *  Function    : EthRedBootRx
 *
 *  Description : Schedules the processing of RX queues.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *              : ETH_Q_FULL_ERR - Queue(RX ring) full condition.  
 *
 *  Note        : This implementation uses a simplified priority scheme.
 *              : It assumes the following priority Q3 > Q2 > Q1 > Q0. 
 *              : This is intended to be used on a RedBoot platform, in an 
 *              : interrupt context.
 =========================================================================*/
SpacEthRtnCode_t EthRedBootRx( struct eth_drv_sc *sc )
{
 
  struct ioc80314* p_ioc80314 = (struct ioc80314 *)(sc->driver_private);
  unsigned int portId = p_ioc80314->port_id;
  SpacEthRtnCode_t rtnCode = ETH_OK;


  if(portId >= ETH_NUM_PORTS){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthRedBootRx>, invalid port id = %d.", portId);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  {
    int qIdx;
    EthRxCtrlPtr_t rxCtrlInfoPtr = &(EthDriver.portInfo[portId]->rxInfo.rxCtrlInfo);

    for(qIdx = (ETH_NUM_RX_Q - 1); qIdx >= 0; qIdx--){    
      if(rxCtrlInfoPtr->qEnables & (1 << qIdx)){            
/* Get new received frames. */      
        rtnCode = ioc80314EthQRx(sc, qIdx);
        if(rtnCode != ETH_OK){
          #ifdef DEBUG_OVI
            db_printf("\nIn <EthRedBootRx>, <EthQRx> for queue %d \
              returns %d.", qIdx, rtnCode);
          #endif // DEBUG_OVI
/* Give a chance to service next queue.       
        return rtnCode; */
        }
      }
     else
      {
      #ifdef DEBUG_OVI 
        db_printf("\nIn <EthRedBootRx>, port id %d, queue id %d disabled", 
                     portId, qIdx );  
      #endif // DEBUG_OVI
        
      }
    }
  }

  return rtnCode;
    
}

#ifdef DEBUG_OVI
void
start_timing (void) {
	volatile cyg_uint32 *base_count1 = (volatile cyg_uint32 *)0x50107444;
	volatile cyg_uint32 *prio_count1 = (volatile cyg_uint32 *)0x50107448;

	*prio_count1 |= 0x80000000;  // Mask Int
	*base_count1 =  0xffffffff; // Max countdown - inhibit counter
	*base_count1 &= ~(0x80000000);  // Start Counting
}

void 
stop_timing (cyg_uint32 out) {
	volatile cyg_uint32 *base_count1 = (volatile cyg_uint32 *)0x50107444;
	volatile cyg_uint32 *curr_count1 = (volatile cyg_uint32 *)0x50107440;
	cyg_uint32 diff;
	 
	*base_count1 |= 0x80000000;  // Stop Counting
	diff = (*base_count1 & 0x7fffffff) - (*curr_count1 & 0x7fffffff);
	diag_printf ("This moment in time: %d is %u\n", out, diff);

} 
#endif

// EOF if_ioc80314.c

