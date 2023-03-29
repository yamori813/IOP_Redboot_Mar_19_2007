/*****************************************************************************
 *
 * Tundra Semiconductor Corporation
 * 
 * Copyright 2002, 2003, Tundra Semiconductor Corporation
 * 
 * This software is the property of Tundra Semiconductor Corporation
 * (Tundra), which specifically grants the user the right to use this
 * software in connection with the Intel(r) 80314 Companion Chip, provided
 * this notice is not removed or altered.  All other rights are reserved
 * by Tundra.
 * 
 * TUNDRA MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD
 * TO THIS SOFTWARE.  IN NO EVENT SHALL AMD BE LIABLE FOR INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING FROM THE
 * FURNISHING, PERFORMANCE, OR USE OF THIS SOFTWARE.
 * 
 *****************************************************************************/

/*-----------------------------------------------------------------------------
* FILENAME: 1.	SpacEthInfo.c
*
* DESCRIPTION: Contains the GigE driver configuration info.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Sept 08,02  on     draft. 
*----------------------------------------------------------------------------*/


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


/* SPac GigE driver's global config info object. */
EthPortInfo_t EthDriverInfo[] = 
{
/* Begins Config0 info. */
{
{
  ETH_BIT_CLEARED, /* portPriority:1 */ 
  0x00000000, /* reserved1:15 */ 
  ETH_BIT_CLEARED, /* backPressureType:1 */ 
  0x00000000, /* reserved2:1 */ 
  ETH_BIT_CLEARED, /* speed:1 */ 
  ETH_BIT_SET, /* rbcMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_CLEARED, /* prbsEnable:1 */ 
  ETH_BIT_CLEARED, /* disableTxCgo:1 */ 
  ETH_BIT_CLEARED, /* tbiOutputEnable:1 */ 
  0x00000000, /* reserved4:4 */ 
  ETH_BIT_SET, /* statisticsEnable:1 */ 
  ETH_BIT_CLEARED, /* zeroOnRead:1 */ 
  ETH_BIT_CLEARED, /* clearStatistics:1 */ 
  ETH_BIT_CLEARED /* statusReset:1 */ 
}, /* portCtrlInfo */
{
  0x0, /* txQueues:4 */ 
  0x0, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* txWait:1 */ 
  ETH_BIT_CLEARED, /* txThreshold:1 */ 
  ETH_BIT_SET, /* txUnderrun:1 */ 
  ETH_BIT_CLEARED, /* txError:1 */ 
  ETH_BIT_CLEARED, /* txAbort:1 */ 
  ETH_BIT_SET, /* txIdle:1 */ 
  0x0, /* reserved2:1 */ 
  ETH_BIT_CLEARED, /* statsCarry:1 */ 
  0x0, /* rxQueues:4 */ 
  0x0, /* reserved3:4 */ 
  ETH_BIT_CLEARED, /* rxWait:1 */ 
  ETH_BIT_CLEARED, /* rxThreshold:1 */ 
  ETH_BIT_CLEARED, /* rxOverrun:1 */ 
  ETH_BIT_CLEARED, /* rxError:1 */ 
  ETH_BIT_CLEARED, /* rxAbort:1 */ 
  ETH_BIT_SET, /* rxIdle:1 */ 
  ETH_BIT_CLEARED, /* ocnError:1 */
  0x0 /* reserved4:1 */ 
}, /* portIntrMaskInfo */
/* Begins MAC info. */
{
{
  ETH_BIT_SET, /* txEnable:1 */ 
  0x00000000, /* synchTxEnable:1 */ 
  ETH_BIT_SET, /* rxEnable:1 */ 
  0x00000000, /* synchRxEnable:1 */ 
  ETH_BIT_SET, /* txFlowCtrl:1 */ 
  ETH_BIT_SET, /* rxFlowCtrl:1 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_BIT_CLEARED, /* loopBack:1 */ 
  0x00000000, /* reserved2:7 */ 
  ETH_BIT_CLEARED, /* resetTxFnct:1 */ 
  ETH_BIT_CLEARED, /* resetRxFnct:1 */ 
  ETH_BIT_CLEARED, /* resetTxMac:1 */ 
  ETH_BIT_CLEARED, /* resetRxMac:1 */ 
  0x00000000, /* reserved3:10 */ 
  ETH_BIT_CLEARED, /* simReset:1 */ 
  ETH_BIT_CLEARED /* softReset:1 */ 
}, /* cfg1Info */
{
  0x00000000, /* fullDuplex:1 */ 
  ETH_BIT_CLEARED, /* crcEnable:1 */ 
  ETH_BIT_CLEARED, /* padCrc:1 */ 
  0x00000000, /* reserved1:1 */ 
  ETH_BIT_CLEARED, /* lenCheck:1 */ 
  ETH_BIT_SET, /* hugeFrame:1 */ 
  0x00000000, /* reserved2:2 */ 
  0x00000000, /* ifMode:2 */ 
  0x00000000, /* reserved3:2 */ 
  0x07, /* preambleLen:4 */ 
  0x00000000 /* reserved4:16 */ 
}, /* cfg2Info */
{
  0x0600, /* maxFrmLen:16 */ 
  0x00000000 /* reserved1:16 */ 
}, /* maxFrmInfo */
{
  0x00000007, /* mngmClkSel:3 - changed to 7 to slow clock to within spec - cebruns */ 
  0x00000000, /* reserved1:1 */ 
  ETH_BIT_SET, /* noPreamble:1 */ 
  ETH_BIT_CLEARED, /* scanIncr:1 */ 
  0x00000000, /* reserved2:25 */ 
  ETH_BIT_CLEARED /* resetMngm:1 */ 
}, /* miiCfgInfo */
{
  0x00000000, /* jabberProtectEnable:1 */ 
  0x00000000, /* reserved1:6 */
  0x00000000, /* resetGpsi:1 */
  0x00000000, /* linkFailDisable:1 */
  0x00000000, /* noCipher:1 */
  0x00000000, /* forceQuiet:1 */
  0x00000000, /* reserved2:4 */
  0x00000000, /* resetPmd:1 */
  0x00000000, /* speed:1 */
  0x00000000, /* reserved3:6 */
  0x00000000, /* resetRmii:1 */
  0x00000000, /* phyMode:1 */
  0x00000000, /* lhdMode:1 */
  0x00000000, /* ghdMode:1 */
  0x00000000, /* ifMode:1 */
  0x00000000, /* reserved4:3 */
  ETH_BIT_CLEARED /* resetIf:1 */
}, /* ifCtrlInfo */
{
  0x11, /* octet4:8 */ 
  0x11, /* octet3:8 */ 
  0x11, /* octet2:8 */ 
  0x11 /* octet1:8 */ 
}, /* addrPart1Info */
{
  0x0000, /* reserved1:16 */ 
  0x11, /* octet6:8 */ 
  0x11 /* octet5:8 */ 
}, /* addrPart2Info */
/* Begins Link info. */
{
{
  0x00000000, /* reserved1:6 */ 
  0x00000000, /* speed1:1 */ 
  0x00000000, /* reserved2:1 */ 
  0x00000000, /* fullDuplex:1 */ 
  ETH_BIT_CLEARED, /* resetAn:1 */ 
  0x00000000, /* reserved3:2 */ 
  ETH_BIT_CLEARED, /* anEnable:1 */ 
  0x00000000, /* speed0:1 */ 
  ETH_BIT_CLEARED, /* loopBack:1 */ 
  ETH_BIT_CLEARED /* phyReset:1 */ 
}, /* tbiCtrl1Info */
{
  0x00000000, /* reserved1:5 */ 
  ETH_BIT_SET, /* fullDuplex:1 */ 
  ETH_BIT_SET, /* halfDuplex:1 */ 
  0x00000000, /* pause:2 */ 
  0x00000000, /* reserved2:3 */ 
  0x00000000, /* remoteFault:2 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET /* nextPage:1 */ 
}, /* tbiAnAdvInfo */
{
  ETH_BIT_CLEARED, /* commaDetect:1 */ 
  ETH_BIT_CLEARED, /* wrapEnable:1 */ 
  0x00000000, /* reserved1:2 */ 
  0x00000000, /* ifMode:1 */ 
  ETH_BIT_SET, /* rxClkSelect:1 */ 
  0x00000000, /* reserved2:2 */ 
  ETH_BIT_CLEARED, /* anSense:1 */ 
  0x00000000, /* reserved3:3 */ 
  ETH_BIT_CLEARED, /* txDisparDisable:1 */ 
  ETH_BIT_CLEARED, /* rxDisparDisable:1 */ 
  ETH_BIT_CLEARED, /* shortCutLinkTmr:1 */ 
  ETH_BIT_CLEARED /* softReset:1 */ 
}, /* tbiCtrl2Info */

/* the first 15 PHY registers are standard. */
{
  0x00000000, /* reserved1:6 */ 
  ETH_BIT_CLEARED, /* speed1:1 */ 
  ETH_BIT_CLEARED, /* cte:1 */ 
  ETH_BIT_CLEARED, /* fullDuplex:1 */
  ETH_BIT_SET, /* resetAn:1 */ 
  ETH_BIT_CLEARED, /* isolate:1 */ 
  ETH_BIT_CLEARED, /* powerDown:1 */ 
  ETH_BIT_SET, /* anEnable:1 */ 
  ETH_BIT_CLEARED, /* speed0:1 */ 
  ETH_BIT_CLEARED, /* loopBack:1 */ 
  ETH_BIT_CLEARED /* phyReset:1 */ 
}, /* phyCtrlInfo */
{
  0x01, /* selectorField:5 */ 
  ETH_BIT_SET, /* t10Half:1 - updated for B0*/ 
  ETH_BIT_SET, /* t10Full:1 */ 
  ETH_BIT_SET, /* tx100Half:1 - updated for B0 */ 
  ETH_BIT_SET, /* tx100Full:1 */ 
  ETH_BIT_CLEARED, /* t4_100:1 */ 
  ETH_BIT_CLEARED, /* pause:1 */ 
  ETH_BIT_CLEARED, /* asymPause:1 */ 
  0x00000000, /* reserved1:1 */ 
  ETH_BIT_CLEARED, /* remoteFault:1 */ 
  0x00000000, /* reserved2:1 */ 
  ETH_BIT_CLEARED /* nextPage:1 */ 
}, /* phyAnAdvInfo */
{
  0x00000000, /* reserved1:8 */ 
  ETH_BIT_CLEARED, /* t1000Half:1 */ 
  ETH_BIT_SET, /* t1000Full:1 */   
  ETH_BIT_SET, /* hubDte:1 */ 
  ETH_BIT_SET, /* masterSlaveVal:1 */ 
  ETH_BIT_CLEARED, /* masterSlaveEnable:1 */ 
  0x00000000 /* testMode:3 */ 
}, /* phy1000BaseTInfo */

#ifdef ETH_PHY_BCM5421  
/* Start of BCM5421-specific code */ 

{
  ETH_BIT_CLEARED, /* gmiiFifoElasticity:1 */ 
  0x00000000, /* reserved1:1 */ 
  0x00000000, /* reserved2:1 */ 
  ETH_BIT_CLEARED, /* ledOff:1 */ 
  ETH_BIT_CLEARED, /* ledOn:1 */ 
  ETH_BIT_SET, /* ledTrafficModeEnable:1 */ 
  ETH_BIT_CLEARED, /* resetScrambler:1 */ 
  ETH_BIT_CLEARED, /* bypassRsa:1 */ 
  ETH_BIT_CLEARED, /* bypassMlt3:1 */ 
  ETH_BIT_CLEARED, /* bypassScrambler:1 */ 
  ETH_BIT_CLEARED, /* bypass4b5b:1 */ 
  ETH_BIT_CLEARED, /* forceInterrupt:1 */ 
  ETH_BIT_SET, /* interruptDisable:1 */ 
  ETH_BIT_CLEARED, /* txDisable:1 */ 
  ETH_BIT_CLEARED, /* autoMdiCrossOverDisable:1 */ 
  ETH_BIT_CLEARED /* ifMode:1 */ 
} /* bcmExtCtrlInfo */

/* End of BCM5421-specific code */ 
#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E  
  /* Start of MARVELL88E-specific code */ 

{
  ETH_BIT_CLEARED, /* disableJabber:1 */ 
  ETH_BIT_CLEARED, /* polarityReversal:1 */ 
  ETH_BIT_CLEARED, /* sqeTest:1 */ 
  ETH_BIT_SET, /* reserved1:1 (must be set!!!) */ 
  ETH_BIT_CLEARED, /* disable125Clk:1 */ 
  0x3, /* mdiCrossoverMode:2 */ 
  ETH_BIT_CLEARED, /* rxThresh10T:1 */ 
  0x00000000, /* reserved2:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_CLEARED, /* forceLinkOk:1 */ 
  ETH_BIT_CLEARED, /* assertCrcTx:1 */ 
  ETH_BIT_CLEARED, /* rxFifoDepth:2 */ 
  ETH_BIT_CLEARED /* txFifoDepth:2 */ 
}, /* marvExtCtrl1Info */
{
  0x00000000, /* reserved1:4 */ 
  0x6, /* txClkSpeed:3 */ 
  0x00000000, /* reserved2:8 */ 
  ETH_BIT_CLEARED /* lostLockDetect:1 */ 
}, /* marvExtCtrl2Info */
  ETH_MARV_GMII2COOPER, /* marvIfMode */

  /* End of MARVELL88E-specific code */ 
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
#endif

}
/* Ends Link info. */
},
/* Ends MAC info. */
/* Begins MSTAT info. */
{
{
  ETH_BIT_CLEARED, /* rdrp:1 */ 
  ETH_BIT_CLEARED, /* rjbr:1 */ 
  ETH_BIT_CLEARED, /* rfrg:1 */ 
  ETH_BIT_CLEARED, /* rovr:1 */ 
  ETH_BIT_CLEARED, /* rund:1 */ 
  ETH_BIT_CLEARED, /* rcse:1 */ 
  ETH_BIT_CLEARED, /* rcde:1 */ 
  ETH_BIT_CLEARED, /* rflr:1 */ 
  ETH_BIT_CLEARED, /* raln:1 */ 
  ETH_BIT_CLEARED, /* rxuo:1 */ 
  ETH_BIT_CLEARED, /* rxpf:1 */ 
  ETH_BIT_CLEARED, /* rxcf:1 */ 
  ETH_BIT_CLEARED, /* rbca:1 */ 
  ETH_BIT_CLEARED, /* rmca:1 */ 
  ETH_BIT_CLEARED, /* rfcs:1 */ 
  ETH_BIT_CLEARED, /* rpkt:1 */ 
  ETH_BIT_CLEARED, /* rbyt:1 */ 
  ETH_BIT_CLEARED, /* reserved:8 */ 
  ETH_BIT_CLEARED, /* trmgv:1 */ 
  ETH_BIT_CLEARED, /* trmax:1 */ 
  ETH_BIT_CLEARED, /* tr1k:1 */ 
  ETH_BIT_CLEARED, /* tr511:1 */ 
  ETH_BIT_CLEARED, /* tr255:1 */ 
  ETH_BIT_CLEARED, /* tr127:1 */ 
  ETH_BIT_CLEARED /* tr64:1 */ 
}, /* carm1Info */
{
  ETH_BIT_CLEARED, /* tdrp:1 */ 
  ETH_BIT_CLEARED, /* tpfh:1 */ 
  ETH_BIT_CLEARED, /* tncl:1 */ 
  ETH_BIT_CLEARED, /* txcl:1 */ 
  ETH_BIT_CLEARED, /* tlcl:1 */ 
  ETH_BIT_CLEARED, /* tmcl:1 */ 
  ETH_BIT_CLEARED, /* tscl:1 */ 
  ETH_BIT_CLEARED, /* tedf:1 */ 
  ETH_BIT_CLEARED, /* tdfr:1 */ 
  ETH_BIT_CLEARED, /* txpf:1 */ 
  ETH_BIT_CLEARED, /* tbca:1 */ 
  ETH_BIT_CLEARED, /* tmca:1 */ 
  ETH_BIT_CLEARED, /* tpkt:1 */ 
  ETH_BIT_CLEARED, /* tbyt:1 */ 
  ETH_BIT_CLEARED, /* tfrg:1 */ 
  ETH_BIT_CLEARED, /* tund:1 */   
  ETH_BIT_CLEARED, /* tovr:1 */ 
  ETH_BIT_CLEARED, /* txcf:1 */ 
  ETH_BIT_CLEARED, /* tfcs:1 */ 
  ETH_BIT_CLEARED, /* tjbr:1 */ 
  ETH_BIT_CLEARED /* reserved:12 */ 
} /* carm2Info */
},
/* Ends MSTAT info. */
/* Begins TX info. */
{
{
  0x3, /* startQNum:2 */ 
  0x00000000, /* reserved1:20 */ 
  ETH_BIT_CLEARED, /* enableHigherPriority:1 */ 
  ETH_BIT_CLEARED, /* changePriority:1 */ 
  0x00000000, /* reserved2:7 */
  ETH_BIT_CLEARED /* reset:1 */ 
}, /* txCfgInfo */
{
  0xF, /* qEnables:4 */ 
  0x00000000, /* reserved1:11 */
  ETH_BIT_SET, /* go:1 */ 
  0x00000000, /* reserved2:8 */
  ETH_BIT_CLEARED, /* manualPause:1 */ 
  0x00000000, /* reserved3:4 */
  ETH_BIT_SET, /* enableAbortIntr:1 */
  ETH_BIT_CLEARED, /* abort:1 */
  ETH_BIT_CLEARED /* enableIdleIntr:1 */
}, /* txCtrlInfo */
{
  0x60, /* 0x20 startSending:8 */ 
  0x00000000, /* reserved1:8 */
  0xD8, /* 0xE0 0xDF stopFilling:8 */ 
  0x00000000 /* reserved2:8 */
}, /* txThreshInfo */
{
  0x00, /* underrunPriThresh:8 */ 
  0x00000000 /* reserved1:24 */
}, /* txPriThreshInfo */
{
  0x3, /* nextQAfter0:2 */ 
  0x00000000, /* reserved1:1 */
  ETH_BIT_SET, /* revertAfter0:1 */ 
  0x1, /* frameLimitQ0:4 */ 
  0x0, /* nextQAfter1:2 */ 
  0x00000000, /* reserved2:1 */
  ETH_BIT_SET, /* revertAfter1:1 */ 
  0x2, /* frameLimitQ1:4 */ 
  0x1, /* nextQAfter2:2 */ 
  0x00000000, /* reserved3:1 */
  ETH_BIT_SET, /* revertAfter2:1 */ 
  0x4, /* frameLimitQ2:4 */ 
  0x2, /* nextQAfter3:2 */ 
  0x00000000, /* reserved4:1 */
  ETH_BIT_SET, /* revertAfter3:1 */ 
  0x8 /* frameLimitQ3:4 */ 
}, /* txPriMapInfo */
{
  0x00000000, /* pauseCount:16 */ 
  0x00000000 /* reserved1:16 */
}, /* txPauseCntInfo */
{
  0x8100abcd /* gVlanTag:32 */ 
}, /* txGVlanInfo */
/* Begins txQInfo info. */
{
/* Begins txQInfo[0] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
},
/* Ends txQInfo[0] info. */
/* Begins txQInfo[1] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
},
/* Ends txQInfo[1] info. */
/* Begins txQInfo[2] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
},
/* Ends txQInfo[2] info. */
/* Begins txQInfo[3] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
}
/* Ends txQInfo[3] info. */
}
/* Ends txQInfo info. */
},
/* Ends TX info. */
/* Begins RX info. */
{
{
  0x3, /* defaultQNum:2 */ 
  0x00000000, /* reserved1:6 */ 
  ETH_BIT_CLEARED, /* enableMcastHashFilter:1 */ 
  ETH_BIT_CLEARED, /* enableUcastHashFilter:1 */ 
  ETH_BIT_SET, /* bcastFrmEnable:1 */ 
  ETH_BIT_SET, /* mcastFrmEnable:1 */ 
  ETH_BIT_SET, /* ucastFrmEnable:1 */ 
  ETH_BIT_SET, /* stationEnable:1 */ 
  0x00000000, /* reserved2:7 */ 
  ETH_BIT_CLEARED, /* alowBadFrm:1 */ 
  ETH_BIT_CLEARED, /* autoPauseEnable:1 */ 
  ETH_BIT_CLEARED, /* changePriority:1 */ 
  0x00000000, /* reserved3:7 */
  ETH_BIT_CLEARED /* reset:1 */ 
}, /* rxCfgInfo */
{
  0xF, /* qEnables:4 */ 
  0x00000000, /* reserved1:11 */
  ETH_BIT_SET, /* go:1 */ 
  0x00000000, /* reserved2:13 */
  ETH_BIT_SET, /* enableAbortIntr:1 */
  ETH_BIT_CLEARED, /* abort:1 */
  ETH_BIT_CLEARED /* enableIdleIntr:1 */
}, /* rxCtrlInfo */
{
  0x180, /* unpauseThresh:9 */ 
  0x00000000, /* reserved1:7 */
  0x1C0, /* pauseThresh:9 */ 
  0x00000000 /* reserved2:7 */
}, /* rxPauseThreshInfo */
{
  0x1E0, /* overrunWarningThresh:9 */ 
  0x00000000 /* reserved1:23 */
}, /* rxThreshInfo */
{
  0x0, /* qNumForTag0:2 */ 
  0x00000000, /* reserved1:2 */ 
  0x0, /* qNumForTag1:2 */ 
  0x00000000, /* reserved2:2 */ 
  0x1, /* qNumForTag2:2 */ 
  0x00000000, /* reserved3:2 */ 
  0x1, /* qNumForTag3:2 */ 
  0x00000000, /* reserved4:2 */ 
  0x2, /* qNumForTag4:2 */ 
  0x00000000, /* reserved5:2 */ 
  0x2, /* qNumForTag5:2 */ 
  0x00000000, /* reserved6:2 */ 
  0x3, /* qNumForTag6:2 */ 
  0x00000000, /* reserved7:2 */ 
  0x3, /* qNumForTag7:2 */ 
  0x00000000 /* reserved8:2 */ 
}, /* rxVlanMapInfo */
/* Begins rxQInfo info. */
{
/* Begins rxQInfo[0] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
},
/* Ends rxQInfo[0] info. */
/* Begins rxQInfo[1] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
},
/* Ends rxQInfo[1] info. */
/* Begins rxQInfo[2] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
},
/* Ends rxQInfo[2] info. */
/* Begins rxQInfo[3] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
}
/* Ends rxQInfo[3] info. */
}
/* Ends rxQInfo info. */
}
/* Ends RX info. */
},
/* Ends Config0 info. */

/* Begins Config1 info. */
{
{
  ETH_BIT_CLEARED, /* portPriority:1 */ 
  0x00000000, /* reserved1:15 */ 
  ETH_BIT_CLEARED, /* backPressureType:1 */ 
  0x00000000, /* reserved2:1 */ 
  ETH_BIT_CLEARED, /* speed:1 */ 
  ETH_BIT_SET, /* rbcMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_CLEARED, /* prbsEnable:1 */ 
  ETH_BIT_CLEARED, /* disableTxCgo:1 */ 
  ETH_BIT_CLEARED, /* tbiOutputEnable:1 */ 
  0x00000000, /* reserved4:4 */ 
  ETH_BIT_SET, /* statisticsEnable:1 */ 
  ETH_BIT_CLEARED, /* zeroOnRead:1 */ 
  ETH_BIT_CLEARED, /* clearStatistics:1 */ 
  ETH_BIT_CLEARED /* statusReset:1 */ 
}, /* portCtrlInfo */
{
  0x0, /* txQueues:4 */ 
  0x0, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* txWait:1 */ 
  ETH_BIT_CLEARED, /* txThreshold:1 */ 
  ETH_BIT_SET, /* txUnderrun:1 */ 
  ETH_BIT_CLEARED, /* txError:1 */ 
  ETH_BIT_CLEARED, /* txAbort:1 */ 
  ETH_BIT_SET, /* txIdle:1 */ 
  0x0, /* reserved2:1 */ 
  ETH_BIT_CLEARED, /* statsCarry:1 */ 
  0x0, /* rxQueues:4 */ 
  0x0, /* reserved3:4 */ 
  ETH_BIT_CLEARED, /* rxWait:1 */ 
  ETH_BIT_CLEARED, /* rxThreshold:1 */ 
  ETH_BIT_CLEARED, /* rxOverrun:1 */ 
  ETH_BIT_CLEARED, /* rxError:1 */ 
  ETH_BIT_CLEARED, /* rxAbort:1 */ 
  ETH_BIT_SET, /* rxIdle:1 */ 
  ETH_BIT_CLEARED, /* ocnError:1 */
  0x0 /* reserved4:1 */ 
}, /* portIntrMaskInfo */
/* Begins MAC info. */
{
{
  ETH_BIT_SET, /* txEnable:1 */ 
  0x00000000, /* synchTxEnable:1 */ 
  ETH_BIT_SET, /* rxEnable:1 */ 
  0x00000000, /* synchRxEnable:1 */ 
  ETH_BIT_SET, /* txFlowCtrl:1 */ 
  ETH_BIT_SET, /* rxFlowCtrl:1 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_BIT_CLEARED, /* loopBack:1 */ 
  0x00000000, /* reserved2:7 */ 
  ETH_BIT_CLEARED, /* resetTxFnct:1 */ 
  ETH_BIT_CLEARED, /* resetRxFnct:1 */ 
  ETH_BIT_CLEARED, /* resetTxMac:1 */ 
  ETH_BIT_CLEARED, /* resetRxMac:1 */ 
  0x00000000, /* reserved3:10 */ 
  ETH_BIT_CLEARED, /* simReset:1 */ 
  ETH_BIT_CLEARED /* softReset:1 */ 
}, /* cfg1Info */
{
  0x00000000, /* fullDuplex:1 */ 
  ETH_BIT_CLEARED, /* crcEnable:1 */ 
  ETH_BIT_CLEARED, /* padCrc:1 */ 
  0x00000000, /* reserved1:1 */ 
  ETH_BIT_CLEARED, /* lenCheck:1 */ 
  ETH_BIT_SET, /* hugeFrame:1 */ 
  0x00000000, /* reserved2:2 */ 
  0x00000000, /* ifMode:2 */ 
  0x00000000, /* reserved3:2 */ 
  0x07, /* preambleLen:4 */ 
  0x00000000 /* reserved4:16 */ 
}, /* cfg2Info */
{
  0x0600, /* maxFrmLen:16 */ 
  0x00000000 /* reserved1:16 */ 
}, /* maxFrmInfo */
{
  0x00000007, /* mngmClkSel:3 - changed to 7 to slow clock to within spec - cebruns */ 
  0x00000000, /* reserved1:1 */ 
  ETH_BIT_SET, /* noPreamble:1 */ 
  ETH_BIT_CLEARED, /* scanIncr:1 */ 
  0x00000000, /* reserved2:25 */ 
  ETH_BIT_CLEARED /* resetMngm:1 */ 
}, /* miiCfgInfo */
{
  0x00000000, /* jabberProtectEnable:1 */ 
  0x00000000, /* reserved1:6 */
  0x00000000, /* resetGpsi:1 */
  0x00000000, /* linkFailDisable:1 */
  0x00000000, /* noCipher:1 */
  0x00000000, /* forceQuiet:1 */
  0x00000000, /* reserved2:4 */
  0x00000000, /* resetPmd:1 */
  0x00000000, /* speed:1 */
  0x00000000, /* reserved3:6 */
  0x00000000, /* resetRmii:1 */
  0x00000000, /* phyMode:1 */
  0x00000000, /* lhdMode:1 */
  0x00000000, /* ghdMode:1 */
  0x00000000, /* ifMode:1 */
  0x00000000, /* reserved4:3 */
  ETH_BIT_CLEARED /* resetIf:1 */
}, /* ifCtrlInfo */
{
  0x11, /* octet4:8 */ 
  0x11, /* octet3:8 */ 
  0x11, /* octet2:8 */ 
  0x11 /* octet1:8 */ 
}, /* addrPart1Info */
{
  0x0000, /* reserved1:16 */ 
  0x11, /* octet6:8 */ 
  0x11 /* octet5:8 */ 
}, /* addrPart2Info */
/* Begins Link info. */
{
{
  0x00000000, /* reserved1:6 */ 
  0x00000000, /* speed1:1 */ 
  0x00000000, /* reserved2:1 */ 
  0x00000000, /* fullDuplex:1 */ 
  ETH_BIT_CLEARED, /* resetAn:1 */ 
  0x00000000, /* reserved3:2 */ 
  ETH_BIT_CLEARED, /* anEnable:1 */ 
  0x00000000, /* speed0:1 */ 
  ETH_BIT_CLEARED, /* loopBack:1 */ 
  ETH_BIT_CLEARED /* phyReset:1 */ 
}, /* tbiCtrl1Info */
{
  0x00000000, /* reserved1:5 */ 
  ETH_BIT_SET, /* fullDuplex:1 */ 
  ETH_BIT_SET, /* halfDuplex:1 */ 
  0x00000000, /* pause:2 */ 
  0x00000000, /* reserved2:3 */ 
  0x00000000, /* remoteFault:2 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET /* nextPage:1 */ 
}, /* tbiAnAdvInfo */
{
  ETH_BIT_CLEARED, /* commaDetect:1 */ 
  ETH_BIT_CLEARED, /* wrapEnable:1 */ 
  0x00000000, /* reserved1:2 */ 
  0x00000000, /* ifMode:1 */ 
  ETH_BIT_SET, /* rxClkSelect:1 */ 
  0x00000000, /* reserved2:2 */ 
  ETH_BIT_CLEARED, /* anSense:1 */ 
  0x00000000, /* reserved3:3 */ 
  ETH_BIT_CLEARED, /* txDisparDisable:1 */ 
  ETH_BIT_CLEARED, /* rxDisparDisable:1 */ 
  ETH_BIT_CLEARED, /* shortCutLinkTmr:1 */ 
  ETH_BIT_CLEARED /* softReset:1 */ 
}, /* tbiCtrl2Info */

/* the first 15 PHY registers are standard. */
{
  0x00000000, /* reserved1:6 */ 
  ETH_BIT_CLEARED, /* speed1:1 */ 
  ETH_BIT_CLEARED, /* cte:1 */ 
  ETH_BIT_CLEARED, /* fullDuplex:1 */
  ETH_BIT_SET, /* resetAn:1 */ 
  ETH_BIT_CLEARED, /* isolate:1 */ 
  ETH_BIT_CLEARED, /* powerDown:1 */ 
  ETH_BIT_SET, /* anEnable:1 */ 
  ETH_BIT_CLEARED, /* speed0:1 */ 
  ETH_BIT_CLEARED, /* loopBack:1 */ 
  ETH_BIT_CLEARED /* phyReset:1 */ 
}, /* phyCtrlInfo */
{
  0x01, /* selectorField:5 */ 
  ETH_BIT_SET, /* t10Half:1 */ 
  ETH_BIT_SET, /* t10Full:1 */ 
  ETH_BIT_SET, /* tx100Half:1 */ 
  ETH_BIT_SET, /* tx100Full:1 */ 
  ETH_BIT_CLEARED, /* t4_100:1 */ 
  ETH_BIT_CLEARED, /* pause:1 */ 
  ETH_BIT_CLEARED, /* asymPause:1 */ 
  0x00000000, /* reserved1:1 */ 
  ETH_BIT_CLEARED, /* remoteFault:1 */ 
  0x00000000, /* reserved2:1 */ 
  ETH_BIT_CLEARED /* nextPage:1 */ 
}, /* phyAnAdvInfo */
{
  0x00000000, /* reserved1:8 */ 
  ETH_BIT_SET, /* t1000Half:1 */ 
  ETH_BIT_SET, /* t1000Full:1 */   
  ETH_BIT_SET, /* hubDte:1 */ 
  ETH_BIT_SET, /* masterSlaveVal:1 */ 
  ETH_BIT_CLEARED, /* masterSlaveEnable:1 */ 
  0x00000000 /* testMode:3 */ 
}, /* phy1000BaseTInfo */

#ifdef ETH_PHY_BCM5421  
/* Start of BCM5421-specific code */ 

{
  ETH_BIT_CLEARED, /* gmiiFifoElasticity:1 */ 
  0x00000000, /* reserved1:1 */ 
  0x00000000, /* reserved2:1 */ 
  ETH_BIT_CLEARED, /* ledOff:1 */ 
  ETH_BIT_CLEARED, /* ledOn:1 */ 
  ETH_BIT_SET, /* ledTrafficModeEnable:1 */ 
  ETH_BIT_CLEARED, /* resetScrambler:1 */ 
  ETH_BIT_CLEARED, /* bypassRsa:1 */ 
  ETH_BIT_CLEARED, /* bypassMlt3:1 */ 
  ETH_BIT_CLEARED, /* bypassScrambler:1 */ 
  ETH_BIT_CLEARED, /* bypass4b5b:1 */ 
  ETH_BIT_CLEARED, /* forceInterrupt:1 */ 
  ETH_BIT_SET, /* interruptDisable:1 */ 
  ETH_BIT_CLEARED, /* txDisable:1 */ 
  ETH_BIT_CLEARED, /* autoMdiCrossOverDisable:1 */ 
  ETH_BIT_CLEARED /* ifMode:1 */ 
} /* bcmExtCtrlInfo */

/* End of BCM5421-specific code */ 
#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E  
  /* Start of MARVELL88E-specific code */ 

{
  ETH_BIT_CLEARED, /* disableJabber:1 */ 
  ETH_BIT_CLEARED, /* polarityReversal:1 */ 
  ETH_BIT_CLEARED, /* sqeTest:1 */ 
  ETH_BIT_SET, /* reserved1:1 (must be set!!!) */ 
  ETH_BIT_CLEARED, /* disable125Clk:1 */ 
  0x3, /* mdiCrossoverMode:2 */ 
  ETH_BIT_CLEARED, /* rxThresh10T:1 */ 
  0x00000000, /* reserved2:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_CLEARED, /* forceLinkOk:1 */ 
  ETH_BIT_CLEARED, /* assertCrcTx:1 */ 
  ETH_BIT_CLEARED, /* rxFifoDepth:2 */ 
  ETH_BIT_CLEARED /* txFifoDepth:2 */ 
}, /* marvExtCtrl1Info */
{
  0x00000000, /* reserved1:4 */ 
  0x6, /* txClkSpeed:3 */ 
  0x00000000, /* reserved2:8 */ 
  ETH_BIT_CLEARED /* lostLockDetect:1 */ 
}, /* marvExtCtrl2Info */
  ETH_MARV_GMII2COOPER, /* marvIfMode */

  /* End of MARVELL88E-specific code */ 
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
#endif

}
/* Ends Link info. */
},
/* Ends MAC info. */
/* Begins MSTAT info. */
{
{
  ETH_BIT_CLEARED, /* rdrp:1 */ 
  ETH_BIT_CLEARED, /* rjbr:1 */ 
  ETH_BIT_CLEARED, /* rfrg:1 */ 
  ETH_BIT_CLEARED, /* rovr:1 */ 
  ETH_BIT_CLEARED, /* rund:1 */ 
  ETH_BIT_CLEARED, /* rcse:1 */ 
  ETH_BIT_CLEARED, /* rcde:1 */ 
  ETH_BIT_CLEARED, /* rflr:1 */ 
  ETH_BIT_CLEARED, /* raln:1 */ 
  ETH_BIT_CLEARED, /* rxuo:1 */ 
  ETH_BIT_CLEARED, /* rxpf:1 */ 
  ETH_BIT_CLEARED, /* rxcf:1 */ 
  ETH_BIT_CLEARED, /* rbca:1 */ 
  ETH_BIT_CLEARED, /* rmca:1 */ 
  ETH_BIT_CLEARED, /* rfcs:1 */ 
  ETH_BIT_CLEARED, /* rpkt:1 */ 
  ETH_BIT_CLEARED, /* rbyt:1 */ 
  ETH_BIT_CLEARED, /* reserved:8 */ 
  ETH_BIT_CLEARED, /* trmgv:1 */ 
  ETH_BIT_CLEARED, /* trmax:1 */ 
  ETH_BIT_CLEARED, /* tr1k:1 */ 
  ETH_BIT_CLEARED, /* tr511:1 */ 
  ETH_BIT_CLEARED, /* tr255:1 */ 
  ETH_BIT_CLEARED, /* tr127:1 */ 
  ETH_BIT_CLEARED /* tr64:1 */ 
}, /* carm1Info */
{
  ETH_BIT_CLEARED, /* tdrp:1 */ 
  ETH_BIT_CLEARED, /* tpfh:1 */ 
  ETH_BIT_CLEARED, /* tncl:1 */ 
  ETH_BIT_CLEARED, /* txcl:1 */ 
  ETH_BIT_CLEARED, /* tlcl:1 */ 
  ETH_BIT_CLEARED, /* tmcl:1 */ 
  ETH_BIT_CLEARED, /* tscl:1 */ 
  ETH_BIT_CLEARED, /* tedf:1 */ 
  ETH_BIT_CLEARED, /* tdfr:1 */ 
  ETH_BIT_CLEARED, /* txpf:1 */ 
  ETH_BIT_CLEARED, /* tbca:1 */ 
  ETH_BIT_CLEARED, /* tmca:1 */ 
  ETH_BIT_CLEARED, /* tpkt:1 */ 
  ETH_BIT_CLEARED, /* tbyt:1 */ 
  ETH_BIT_CLEARED, /* tfrg:1 */ 
  ETH_BIT_CLEARED, /* tund:1 */   
  ETH_BIT_CLEARED, /* tovr:1 */ 
  ETH_BIT_CLEARED, /* txcf:1 */ 
  ETH_BIT_CLEARED, /* tfcs:1 */ 
  ETH_BIT_CLEARED, /* tjbr:1 */ 
  ETH_BIT_CLEARED /* reserved:12 */ 
} /* carm2Info */
},
/* Ends MSTAT info. */
/* Begins TX info. */
{
{
  0x3, /* startQNum:2 */ 
  0x00000000, /* reserved1:20 */ 
  ETH_BIT_CLEARED, /* enableHigherPriority:1 */ 
  ETH_BIT_CLEARED, /* changePriority:1 */ 
  0x00000000, /* reserved2:7 */
  ETH_BIT_CLEARED /* reset:1 */ 
}, /* txCfgInfo */
{
  0xF, /* qEnables:4 */ 
  0x00000000, /* reserved1:11 */
  ETH_BIT_SET, /* go:1 */ 
  0x00000000, /* reserved2:8 */
  ETH_BIT_CLEARED, /* manualPause:1 */ 
  0x00000000, /* reserved3:4 */
  ETH_BIT_SET, /* enableAbortIntr:1 */
  ETH_BIT_CLEARED, /* abort:1 */
  ETH_BIT_CLEARED /* enableIdleIntr:1 */
}, /* txCtrlInfo */
{
  0x60, /* 0x20 startSending:8 */ 
  0x00000000, /* reserved1:8 */
  0xD8, /* 0xE0 0xDF stopFilling:8 */ 
  0x00000000 /* reserved2:8 */
}, /* txThreshInfo */
{
  0x00, /* underrunPriThresh:8 */ 
  0x00000000 /* reserved1:24 */
}, /* txPriThreshInfo */
{
  0x3, /* nextQAfter0:2 */ 
  0x00000000, /* reserved1:1 */
  ETH_BIT_SET, /* revertAfter0:1 */ 
  0x1, /* frameLimitQ0:4 */ 
  0x0, /* nextQAfter1:2 */ 
  0x00000000, /* reserved2:1 */
  ETH_BIT_SET, /* revertAfter1:1 */ 
  0x2, /* frameLimitQ1:4 */ 
  0x1, /* nextQAfter2:2 */ 
  0x00000000, /* reserved3:1 */
  ETH_BIT_SET, /* revertAfter2:1 */ 
  0x4, /* frameLimitQ2:4 */ 
  0x2, /* nextQAfter3:2 */ 
  0x00000000, /* reserved4:1 */
  ETH_BIT_SET, /* revertAfter3:1 */ 
  0x8 /* frameLimitQ3:4 */ 
}, /* txPriMapInfo */
{
  0x00000000, /* pauseCount:16 */ 
  0x00000000 /* reserved1:16 */
}, /* txPauseCntInfo */
{
  0x8100abcd /* gVlanTag:32 */ 
}, /* txGVlanInfo */
/* Begins txQInfo info. */
{
/* Begins txQInfo[0] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
},
/* Ends txQInfo[0] info. */
/* Begins txQInfo[1] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
},
/* Ends txQInfo[1] info. */
/* Begins txQInfo[2] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
},
/* Ends txQInfo[2] info. */
/* Begins txQInfo[3] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  ETH_BIT_CLEARED, /* gVlanInsert:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved3:11 */ 
}, /* txQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* txQBuffCfgInfo */
}
/* Ends txQInfo[3] info. */
}
/* Ends txQInfo info. */
},
/* Ends TX info. */
/* Begins RX info. */
{
{
  0x3, /* defaultQNum:2 */ 
  0x00000000, /* reserved1:6 */ 
  ETH_BIT_CLEARED, /* enableMcastHashFilter:1 */ 
  ETH_BIT_CLEARED, /* enableUcastHashFilter:1 */ 
  ETH_BIT_SET, /* bcastFrmEnable:1 */ 
  ETH_BIT_SET, /* mcastFrmEnable:1 */ 
  ETH_BIT_SET, /* ucastFrmEnable:1 */ 
  ETH_BIT_SET, /* stationEnable:1 */ 
  0x00000000, /* reserved2:7 */ 
  ETH_BIT_CLEARED, /* alowBadFrm:1 */ 
  ETH_BIT_CLEARED, /* autoPauseEnable:1 */ 
  ETH_BIT_CLEARED, /* changePriority:1 */ 
  0x00000000, /* reserved3:7 */
  ETH_BIT_CLEARED /* reset:1 */ 
}, /* rxCfgInfo */
{
  0xF, /* qEnables:4 */ 
  0x00000000, /* reserved1:11 */
  ETH_BIT_SET, /* go:1 */ 
  0x00000000, /* reserved2:13 */
  ETH_BIT_SET, /* enableAbortIntr:1 */
  ETH_BIT_CLEARED, /* abort:1 */
  ETH_BIT_CLEARED /* enableIdleIntr:1 */
}, /* rxCtrlInfo */
{
  0x180, /* unpauseThresh:9 */ 
  0x00000000, /* reserved1:7 */
  0x1C0, /* pauseThresh:9 */ 
  0x00000000 /* reserved2:7 */
}, /* rxPauseThreshInfo */
{
  0x1E0, /* overrunWarningThresh:9 */ 
  0x00000000 /* reserved1:23 */
}, /* rxThreshInfo */
{
  0x0, /* qNumForTag0:2 */ 
  0x00000000, /* reserved1:2 */ 
  0x0, /* qNumForTag1:2 */ 
  0x00000000, /* reserved2:2 */ 
  0x1, /* qNumForTag2:2 */ 
  0x00000000, /* reserved3:2 */ 
  0x1, /* qNumForTag3:2 */ 
  0x00000000, /* reserved4:2 */ 
  0x2, /* qNumForTag4:2 */ 
  0x00000000, /* reserved5:2 */ 
  0x2, /* qNumForTag5:2 */ 
  0x00000000, /* reserved6:2 */ 
  0x3, /* qNumForTag6:2 */ 
  0x00000000, /* reserved7:2 */ 
  0x3, /* qNumForTag7:2 */ 
  0x00000000 /* reserved8:2 */ 
}, /* rxVlanMapInfo */
/* Begins rxQInfo info. */
{
/* Begins rxQInfo[0] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
},
/* Ends rxQInfo[0] info. */
/* Begins rxQInfo[1] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
},
/* Ends rxQInfo[1] info. */
/* Begins rxQInfo[2] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
},
/* Ends rxQInfo[2] info. */
/* Begins rxQInfo[3] info. */
{
/* Begins descrInfo info. */
{
  ETH_RING_DESCR, /* descrMode */
  ETH_DESCR_TERM_OWN, /* descrTermMode */
  ETH_NUM_DESCR_PER_Q, /* numDescr */
  ETH_MAX_BUFF_SIZE, /* buffSize */
  FALSE /* descrIntr */
},
/* Ends descrInfo info. */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:4 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  ETH_BIT_CLEARED, /* autoMode:1 */ 
  0x00000000, /* reserved3:1 */ 
  ETH_BIT_SET, /* enableEofIntr:1 */ 
  ETH_BIT_CLEARED, /* enableLastIntr:1 */ 
  ETH_BIT_CLEARED, /* enableNullIntr:1 */ 
  ETH_BIT_SET, /* enableOwnerIntr:1 */ 
  ETH_BIT_CLEARED, /* enableDescrIntr:1 */ 
  0x00000000 /* reserved4:11 */ 
}, /* rxQCfgInfo */
{
  ETH_MEM_ALLOC_ID, /* ocnPort:6 */ 
  0x00000000, /* reserved1:2 */ 
  ETH_OCN_BURST_SIZE, /* burst:2 */ 
  ETH_BIT_CLEARED, /* byteSwap:1 */ 
  ETH_BIT_CLEARED, /* wordSwap:1 */ 
  0x00000000 /* reserved2:20 */ 
} /* rxQBuffCfgInfo */
}
/* Ends rxQInfo[3] info. */
}
/* Ends rxQInfo info. */
}
/* Ends RX info. */
}
/* Ends Config1 info. */


}
; 

