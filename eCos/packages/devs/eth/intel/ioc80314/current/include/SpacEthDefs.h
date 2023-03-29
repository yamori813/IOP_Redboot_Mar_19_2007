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
* FILENAME: 1.	SpacEthDefs.h
*
* DESCRIPTION: Contains general-usage defines for SPac's GigE block.  
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     added register definitions. 
*----------------------------------------------------------------------------*/

#ifndef _SPAC_ETH_DEFS_H_
#define _SPAC_ETH_DEFS_H_

#ifndef __tuntypes
#include <cyg/devs/eth/tuntypes.h>
#endif

//#ifndef _TS_BASIC_TYPES_H_
//#include <cyg/devs/eth/ts_basic_types.h>
//#endif

#ifndef _SPAC_ETH_PUBLIC_H_
#include <cyg/devs/eth/SpacEthPublic.h>
#endif

#ifndef _SPAC_ETH_HW_DEFS_H_
#include <cyg/devs/eth/SpacEthHardwareDefs.h>
#endif

/* In S/V mode, we use simplified TX/RX schemes. Undefine this when using 
 * an OS.  
 */
/* Platform defines. */
#define ETH_REDBOOT
    
/* Port0 and Port1 ids. */
#define ETH_PORT0_ID (0)
#define ETH_PORT1_ID (1)

/* CPU0 and CPU1 ids. */
#define ETH_CPU0_ID (0)
#define ETH_CPU1_ID (1)

/* Enabled/Disabled type. */
typedef enum {
  ETH_BIT_CLEARED = 0,   
  ETH_BIT_SET = 1,   
} EthBitField_t;

/* TX/RX direction flow type. */
typedef enum {
  ETH_TX = 0,   
  ETH_RX = 1,   
} EthDirFlow_t;

/* TX/RX cell frame type. */
typedef enum {
  ETH_FRM_UNKNOWN = 0,   
  ETH_FRM_SOF = 1,   
  ETH_FRM_SOFEOF = 2, 
  ETH_FRM_CELL = 3,   
  ETH_FRM_EOF = 4,   
} EthCellFrm_t;

/* TX/RX descriptor owner type. */
typedef enum {
  ETH_DESCR_SYS = 0, /* descriptor owned by the system. */  
  ETH_DESCR_DMA = 1, /* descriptor owned by the DMA. */  
} EthDescrOwner_t;

/* TX/RX buffer descriptor scheme type. */
typedef enum {
  ETH_SINGLE_DESCR = 0, /* Single descriptor frames. */    
  ETH_MULTI_DESCR = 1, /* Multiple descriptor frames. */    
  ETH_RING_DESCR = 2, /* Ring descriptor frames. */    
  ETH_MFRAME_LIST_DESCR = 3, /* Multi-frame descriptor list. */    
} EthDescr_t;

/* TX/RX buffer descriptor terminator type. */
typedef enum {
  ETH_DESCR_TERM_LAST = 0, /* "Last" bit set in NEXT_CPP. */    
  ETH_DESCR_TERM_NULL = 1, /* NEXT_CPP is NULL. */    
  ETH_DESCR_TERM_OWN = 2, /* "Owner" bit set in DCSW. */    
} EthDescrTerm_t;

/* TX/RX buffer size type. We recommend to use MTU dividers. */
typedef enum {
  ETH_BUFF_SIZE_64 = 64, 
  ETH_BUFF_SIZE_96 = 96, 
  ETH_BUFF_SIZE_128 = 128,     
  ETH_BUFF_SIZE_256 = 256,    
  ETH_BUFF_SIZE_512 = 512,    
  ETH_BUFF_SIZE_MTU = 1536, /* Ethernet frame size + alignment on 64-bit. */ 
  ETH_BUFF_SIZE_1600 = 1600, 
  ETH_BUFF_SIZE_2048 = 2048, 
  ETH_BUFF_SIZE_4096 = 4096, 
  ETH_BUFF_SIZE_8192 = 8192, 
  ETH_BUFF_SIZE_16384 = 16384, 
} EthBuffSize_t;

/* TX/RX OCN data burst type. */
typedef enum {
  ETH_OCN_BURST_8 = 0, /* 8-bytes burst. */    
  ETH_OCN_BURST_32 = 1, /* 32-bytes burst. */    
  ETH_OCN_BURST_128 = 2, /* 128-bytes burst. */    
  ETH_OCN_BURST_256 = 3, /* 256-bytes burst. */    
} EthOcnBurst_t;

/* Interface(to which MAC is connected) mode type. */
typedef enum {
  ETH_MAC_IF_RESERVED1 = 0, /* Reserved. */     
  ETH_MAC_IF_NIBLE = 1, /* 10/100Mb/s. */     
  ETH_MAC_IF_BYTE = 2, /* 1000Mb/s. */    
  ETH_MAC_IF_RESERVED2 = 3,     
} EthMacIfMode_t;

/* MII management cycle type. */
typedef enum {
  ETH_MAC_MII_SCAN = 0, /* Scan cycle. */     
  ETH_MAC_MII_READ = 1, /* Read cycle. */   
  ETH_MAC_MII_WRITE = 2, /* Write cycle. */   
} EthMacMiiCycle_t;

/* TX/RX VLAN/Byte count type. */
typedef struct {
  VUINT32 byteCount:16; /* TX/RX byte count. */
  VUINT32 vlanTag:16; /* TX/RX VLAN info. */
} EthVlanBCnt_t, * EthVlanBCntPtr_t;

/* TX/RX buffer descriptor type. MUST be 64-bit aligned in memory. */
typedef struct {
  VUINT32 startAddr0; /* buffer address, least significant bytes. */
  VUINT32 startAddr1;    /* buffer address, most significant bytes. */
  VUINT32 nextDescrAddr0; /* next descriptor address, least significant bytes. 
                            Must be 64-bit aligned. */
  VUINT32 nextDescrAddr1; /* next descriptor address, most significant bytes. */
  VUINT32 vlanBCnt;  /* VLAN tag(MS2B)/byte count(LS2B). */
  VUINT32 cfgStatus;     /* Configuration/Status. */
  VUINT32 reserved1;     /* reserved. make the descriptor cache line aligned. */
  VUINT32 reserved2;     /* reserved. make the descriptor cache line aligned. */
} EthBuffDescr_t, * EthBuffDescrPtr_t;

/* TX/RX descriptor info type. */
typedef struct {
  volatile EthDescr_t descrMode;
  volatile EthDescrTerm_t descrTermMode; 
  VUINT16 numDescr; /* Limit the number of buffers/descriptors per queue 
                      to ETH_NUM_DESCR. */
  VUINT16 buffSize; /* Assign EthBuffSize_t to this field. Used only for RX. */
  volatile  BOOL descrIntr; /* Allows per-descriptor interrupts. */
} EthDescrInfo_t, * EthDescrInfoPtr_t;

/* TX frame status/configuration type. */
typedef struct {
/* Beginning of configuration fields. */    
  UINT32 eof:1; /* End of frame field. */
  UINT32 sof:1; /* Start of frame field. */
  UINT32 pfVlan:1; /* Per frame VLAN field. */
  UINT32 hugeFrame:1; /* Huge frame field. */
  UINT32 padFrame:1; /* Pad frame field. */
  UINT32 crc:1; /* Generate CRC field. */
  UINT32 reserved1:8; /* reserved field. */
  UINT32 descrIntr:1; /* Descriptor interrupt field. */
  UINT32 reserved2:1; /* reserved field. */
/* End of configuration fields. */  
/* Beginning of status fields. */   
  UINT32 retryCount:4; /* Retry count field. */ 
  UINT32 collision:1; /* Collision field. */
  UINT32 reserved3:3; /* reserved field. */  
  UINT32 lateCollision:1; /* Late collision field. */
  UINT32 underrunAbort:1; /* Underrun abort field. */
  UINT32 retryLimReached:1; /* Retry limit reached field. */  
  UINT32 reserved4:3; /* reserved field. */
  UINT32 frmTxOk:1; /* Frame transmit ok field. */
/* End of status fields. */  
  UINT32 owner:1; /* Owner(0 = system, 1 = user) field. */  
} EthTxFrmStatusCfg_t, * EthTxFrmStatusCfgPtr_t;

/* RX frame status/configuration type. */
typedef struct {
/* Beginning of status fields. */     
  UINT32 eof:1; /* End of frame field. */
  UINT32 sof:1; /* Start of frame field. */
  UINT32 vtf:1; /* In frame VLAN Tag field. */
  UINT32 frmIsType:1; /* Ethernet frame type field. */
  UINT32 shortFrame:1; /* Short frame field. */
  UINT32 reserved1:2; /* reserved field. */
  UINT32 hashTableMatch:1; /* Hash table match found field. */
  UINT32 badFrame:1; /* frame collision occured field. */
  UINT32 overRun:1; /* RX FIFO overrun. */
  UINT32 reserved2:1; /* reserved field. */
  UINT32 maxFrmLeng:1; /* Frame truncated to max length received 
                                from MAC. */ 
  UINT32 crcError:1; /* CRC error field. */
/* End of status fields. */    
  UINT32 descrIntr:1; /* Descriptor interrupt field. */
  UINT32 reserved3:1; /* reserved field. */  
  UINT32 owner:1; /* Owner(0 = system, 1 = user) field. */  
  UINT32 byteCount:16; /* RX buffer size. */  
} EthRxFrmStatusCfg_t, * EthRxFrmStatusCfgPtr_t;

/* TX/RX memory pool type. */
typedef struct {
  EthBuffDescrPtr_t descrPoolPtr; /* Descriptor pool handle for deallocation 
                                     time. */ 
  VINT32 putInIdx; /* TX/RX put-in index. 
                                 Used to add buffers to descriptor rings. */     
  VINT32 takeOutIdx; /* TX/RX take-out index. 
                                 Used to remove buffers from descriptor rings. */  
  VINT32 freeEntries; /* number of free TX/RX entries in ring. */                                  
/* This accomodate for maintaining a list of system-specific buffer headers.
   The ring descriptor store only reference to buffers. The(safe) assumption
   is that the header struct has a "next" type pointer. */  
  volatile void * qHead; /* "PUT-IN" pointer. */
  volatile void * qTail; /* "TAKE-OUT" pointer. */  
} EthMemPool_t, * EthMemPoolPtr_t;

/* Simplified SV frame type. */
typedef struct EthSVFrame_tag{
  VUINT8 * buffPtr; /* buffer pointer. */
  struct EthSVFrame_tag * nextPtr; /* next pointer. */
  VUINT16 buffLen;  /* data length. */
  struct EthSVFrame_tag * nextFrm; /* next pointer. */
  VUINT16 value; /* auxiliary data. */
} EthSVFrame_t, * EthSVFramePtr_t;

/* Ethernet header type. */
typedef struct {
  UINT8 destAddr[ETH_MAC_ADDR_LEN]; /* destination address. */
  UINT8 sourceAddr[ETH_MAC_ADDR_LEN]; /* source address. */
  UINT8 typeLen[ETH_MAC_TYPE_LEN];  /* type/length. */
} EthHeader_t, * EthHeaderPtr_t;

/* TX "PUT" frame configuration info type. */
typedef struct {
  EthTxFrmStatusCfg_t txFrmStatusCfg; /* Frame-specific configuration info. */
  VUINT16 vlanData; /* Frame-specific VLAN tag. */
} EthTxFrmCfgInfo_t, * EthTxFrmCfgInfoPtr_t;

/******************************************************************************
 * Info structures. They are used to convey initialization info to the 
 * driver init routines. 
 */

/* TX/RX queue info types. */
/* ETH_TX_Q#_CFG_REG info type. */
typedef struct {
  UINT32 ocnPort:6; 
  UINT32 reserved1:4; 
  UINT32 reserved2:2; 
  UINT32 reserved3:2; 
  UINT32 autoMode:1; 
  UINT32 gVlanInsert:1; 
  UINT32 enableEofIntr:1; 
  UINT32 enableLastIntr:1; 
  UINT32 enableNullIntr:1; 
  UINT32 enableOwnerIntr:1; 
  UINT32 enableDescrIntr:1; 
  UINT32 reserved4:11; 
} EthTxQCfg_t, * EthTxQCfgPtr_t;

/* ETH_TX_Q#_BUFF_CFG_REG info type. */
typedef struct {
  UINT32 ocnPort:6; 
  UINT32 reserved1:2; 
  UINT32 burst:2; 
  UINT32 byteSwap:1; 
  UINT32 wordSwap:1; 
  UINT32 reserved2:20; 
} EthTxQBuffCfg_t, * EthTxQBuffCfgPtr_t;

/* ETH_TX/RX_Q#_PTR_HI_REG type. */
typedef struct {
  UINT32 descrAddr:12; 
  UINT32 reserved1:19; 
  UINT32 valid:1; 
} EthQPtrHi_t, * EthQPtrHiPtr_t;

typedef struct {
  EthDescrInfo_t descrInfo;
  EthTxQCfg_t txQCfgInfo;
  EthTxQBuffCfg_t txQBuffCfgInfo;
} EthTxQInfo_t, * EthTxQInfoPtr_t;

/* ETH_RX_Q#_CFG_REG info type. */
typedef struct {
  UINT32 ocnPort:6; 
  UINT32 reserved1:4; 
  UINT32 reserved2:2; 
  UINT32 reserved3:2; 
  UINT32 autoMode:1; 
  UINT32 reserved4:1; 
  UINT32 enableEofIntr:1; 
  UINT32 enableLastIntr:1; 
  UINT32 enableNullIntr:1; 
  UINT32 enableOwnerIntr:1; 
  UINT32 enableDescrIntr:1; 
  UINT32 reserved5:11; 
} EthRxQCfg_t, * EthRxQCfgPtr_t;

/* ETH_RX_Q#_BUFF_CFG_REG info type. */
typedef struct {
  UINT32 ocnPort:6; 
  UINT32 reserved1:2; 
  UINT32 burst:2; 
  UINT32 byteSwap:1; 
  UINT32 wordSwap:1; 
  UINT32 reserved2:20; 
} EthRxQBuffCfg_t, * EthRxQBuffCfgPtr_t;

typedef struct {
  EthDescrInfo_t descrInfo;
  EthRxQCfg_t rxQCfgInfo;
  EthRxQBuffCfg_t rxQBuffCfgInfo;
} EthRxQInfo_t, * EthRxQInfoPtr_t;

/* ETH_TX_CFG_REG info type. */
typedef struct {
  UINT32 startQNum:2; 
  UINT32 reserved1:20; 
  UINT32 enableHigherPriority:1; 
  UINT32 changePriority:1; 
  UINT32 reserved2:7;
  UINT32 reset:1; 
} EthTxCfg_t, * EthTxCfgPtr_t;

/* ETH_TX_CTRL_REG info type. */
typedef struct {
  UINT32 qEnables:4; 
  UINT32 reserved1:11;
  UINT32 go:1; 
  UINT32 reserved2:8;
  UINT32 manualPause:1; 
  UINT32 reserved3:4;
  UINT32 enableAbortIntr:1;
  UINT32 abort:1;
  UINT32 enableIdleIntr:1;
} EthTxCtrl_t, * EthTxCtrlPtr_t;

/* ETH_TX_STATUS_REG info type. */
typedef struct {
  UINT32 qUsable:4; 
  UINT32 reserved1:4;
  UINT32 currentQ:2; 
  UINT32 reserved2:5;
  UINT32 active:1;
  UINT32 qIdle:4; 
  UINT32 reserved3:4;
  UINT32 eoqPending:4; 
  UINT32 reserved4:4;
} EthTxStatus_t, * EthTxStatusPtr_t;

/* ETH_TX_THRESH_REG info type. */
typedef struct {
  UINT32 startSending:8; 
  UINT32 reserved1:8;
  UINT32 stopFilling:8; 
  UINT32 reserved2:8;
} EthTxThresh_t, * EthTxThreshPtr_t;

/* ETH_TX_PRI_THRESH_REG info type. */
typedef struct {
  UINT32 underrunPriThresh:8; 
  UINT32 reserved1:24;
} EthTxPriThresh_t, * EthTxPriThreshPtr_t;

/* ETH_TX_Q_PRI_MAP_REG info type. */
typedef struct {
  UINT32 nextQAfter0:2; 
  UINT32 reserved1:1;
  UINT32 revertAfter0:1; 
  UINT32 frameLimitQ0:4; 
  UINT32 nextQAfter1:2; 
  UINT32 reserved2:1;
  UINT32 revertAfter1:1; 
  UINT32 frameLimitQ1:4; 
  UINT32 nextQAfter2:2; 
  UINT32 reserved3:1;
  UINT32 revertAfter2:1; 
  UINT32 frameLimitQ2:4; 
  UINT32 nextQAfter3:2; 
  UINT32 reserved4:1;
  UINT32 revertAfter3:1; 
  UINT32 frameLimitQ3:4; 
} EthTxPriMap_t, * EthTxPriMapPtr_t;

/* ETH_TX_PAUSE_CNT_REG info type. */
typedef struct {
  UINT32 pauseCount:16; 
  UINT32 reserved1:16;
} EthTxPauseCnt_t, * EthTxPauseCntPtr_t;

/* ETH_TX_GVLAN_TAG_REG info type. */
typedef struct {
  UINT32 gVlanTag:32; 
} EthTxGVlan_t, * EthTxGVlanPtr_t;

/* TX/RX block info types. */
typedef struct {
  EthTxCfg_t txCfgInfo;
  EthTxCtrl_t txCtrlInfo;
  EthTxThresh_t txThreshInfo;
  EthTxPriThresh_t txPriThreshInfo;
  EthTxPriMap_t txPriMapInfo;
  EthTxPauseCnt_t txPauseCntInfo;
  EthTxGVlan_t txGVlanInfo;
  EthTxQInfo_t txQInfo[ETH_NUM_TX_Q];
} EthTxInfo_t, * EthTxInfoPtr_t;

/* ETH_RX_CFG_REG info type. */
typedef struct {
  UINT32 defaultQNum:2; 
  UINT32 reserved1:6; 
  UINT32 enableMcastHashFilter:1; 
  UINT32 enableUcastHashFilter:1; 
  UINT32 bcastFrmEnable:1; 
  UINT32 mcastFrmEnable:1; 
  UINT32 ucastFrmEnable:1; 
  UINT32 stationEnable:1; 
  UINT32 reserved2:7; 
  UINT32 alowBadFrm:1; 
  UINT32 autoPauseEnable:1; 
  UINT32 changePriority:1; 
  UINT32 reserved3:7;
  UINT32 reset:1; 
} EthRxCfg_t, * EthRxCfgPtr_t;

/* ETH_RX_CTRL_REG info type. */
typedef struct {
  UINT32 qEnables:4; 
  UINT32 reserved1:11;
  UINT32 go:1; 
  UINT32 reserved2:13;
  UINT32 enableAbortIntr:1;
  UINT32 abort:1;
  UINT32 enableIdleIntr:1;
} EthRxCtrl_t, * EthRxCtrlPtr_t;

/* ETH_RX_STATUS_REG info type. */
typedef struct {
  UINT32 qUsable:4; 
  UINT32 reserved1:4;
  UINT32 currentQ:2; 
  UINT32 reserved2:4;
  UINT32 prbs:1;
  UINT32 active:1;
  UINT32 qIdle:4; 
  UINT32 reserved3:12;
} EthRxStatus_t, * EthRxStatusPtr_t;

/* ETH_RX_XSTATUS_REG info type. */
typedef struct {
  UINT32 eoq:4; 
  UINT32 reserved1:4;
  UINT32 eof:4; 
  UINT32 reserved2:4;
  UINT32 descrIntr:4;
  UINT32 reserved3:4;
  UINT32 errFlag:4;
  UINT32 reserved4:4;
} EthRxXStatus_t, * EthRxXStatusPtr_t;

/* ETH_RX_PAUSE_THRESH_REG info type. */
typedef struct {
  UINT32 unpauseThresh:9; 
  UINT32 reserved1:7;
  UINT32 pauseThresh:9; 
  UINT32 reserved2:7;
} EthRxPauseThresh_t, * EthRxPauseThreshPtr_t;

/* ETH_RX_THRESH_REG info type. */
typedef struct {
  UINT32 overrunWarningThresh:9; 
  UINT32 reserved1:23;
} EthRxThresh_t, * EthRxThreshPtr_t;

/* ETH_RX_VLAN_TAG_MAP_REG info type. */
typedef struct {
  UINT32 qNumForTag0:2; 
  UINT32 reserved1:2; 
  UINT32 qNumForTag1:2; 
  UINT32 reserved2:2; 
  UINT32 qNumForTag2:2; 
  UINT32 reserved3:2; 
  UINT32 qNumForTag3:2; 
  UINT32 reserved4:2; 
  UINT32 qNumForTag4:2; 
  UINT32 reserved5:2; 
  UINT32 qNumForTag5:2; 
  UINT32 reserved6:2; 
  UINT32 qNumForTag6:2; 
  UINT32 reserved7:2; 
  UINT32 qNumForTag7:2; 
  UINT32 reserved8:2; 
} EthRxVlanMap_t, * EthRxVlanMapPtr_t;

typedef struct {
  EthRxCfg_t rxCfgInfo;
  EthRxCtrl_t rxCtrlInfo;
  EthRxPauseThresh_t rxPauseThreshInfo;
  EthRxThresh_t rxThreshInfo;
  EthRxVlanMap_t rxVlanMapInfo;
  EthRxQInfo_t rxQInfo[ETH_NUM_RX_Q];
} EthRxInfo_t, * EthRxInfoPtr_t;

/* ETH_MAC_CFG1_REG info type. */
typedef struct {
  UINT32 txEnable:1; 
  UINT32 synchTxEnable:1; 
  UINT32 rxEnable:1; 
  UINT32 synchRxEnable:1; 
  UINT32 txFlowCtrl:1; 
  UINT32 rxFlowCtrl:1; 
  UINT32 reserved1:2; 
  UINT32 loopBack:1; 
  UINT32 reserved2:7; 
  UINT32 resetTxFnct:1; 
  UINT32 resetRxFnct:1; 
  UINT32 resetTxMac:1; 
  UINT32 resetRxMac:1; 
  UINT32 reserved3:10; 
  UINT32 simReset:1; 
  UINT32 softReset:1; 
} EthMacCfg1_t, * EthMacCfg1Ptr_t;

/* ETH_MAC_CFG2_REG info type. */
typedef struct {
  UINT32 fullDuplex:1; 
  UINT32 crcEnable:1; 
  UINT32 padCrc:1; 
  UINT32 reserved1:1; 
  UINT32 lenCheck:1; 
  UINT32 hugeFrame:1; 
  UINT32 reserved2:2; 
  UINT32 ifMode:2; 
  UINT32 reserved3:2; 
  UINT32 preambleLen:4; 
  UINT32 reserved4:16; 
} EthMacCfg2_t, * EthMacCfg2Ptr_t;

/* ETH_MAC_MAX_FRAME_REG info type. */
typedef struct {
  UINT32 maxFrmLen:16; 
  UINT32 reserved1:16; 
} EthMacMaxFrm_t, * EthMacMaxFrmPtr_t;

/* ETH_MAC_TEST_REG info type. */
typedef struct {
  UINT32 slotTime:1; 
  UINT32 pause:1; 
  UINT32 txHdFlowEnable:1; 
  UINT32 maxBackOff:1; 
  UINT32 reserved1:28; 
} EthMacTest_t, * EthMacTestPtr_t;

/* ETH_MAC_ADDR_PART1_REG info type. */
typedef struct {
  UINT32 octet3:8; 
  UINT32 octet4:8; 
  UINT32 octet5:8; 
  UINT32 octet6:8; 
} EthMacAddr1_t, * EthMacAddr1Ptr_t;

/* ETH_MAC_ADDR_PART2_REG info type. */
typedef struct {
  UINT32 reserved1:16; 
  UINT32 octet1:8; 
  UINT32 octet2:8; 
} EthMacAddr2_t, * EthMacAddr2Ptr_t;

/* ETH_MAC_MII_CFG_REG info type. */
typedef struct {
  UINT32 mngmClkSel:3; 
  UINT32 reserved1:1; 
  UINT32 noPreamble:1; 
  UINT32 scanIncr:1; 
  UINT32 reserved2:25; 
  UINT32 resetMngm:1; 
} EthMacMiiCfg_t, * EthMacMiiCfgPtr_t;

/* ETH_MAC_MII_CMD_REG info type. */
typedef struct {
  UINT32 readCycle:1; 
  UINT32 scanCycle:1;
  UINT32 reserved1:30; 
} EthMacMiiCmd_t, * EthMacMiiCmdPtr_t;

/* ETH_MAC_MII_ADDR_REG info type. */
typedef struct {
  UINT32 regAddress:5; 
  UINT32 reserved1:3; 
  UINT32 phyAddress:5; 
  UINT32 reserved2:19; 
} EthMacMiiAddr_t, * EthMacMiiAddrPtr_t;

/* ETH_MAC_MII_CTRL_REG info type. */
typedef struct {
  UINT32 phyCtrl:16; 
  UINT32 reserved1:16; 
} EthMacMiiCtrl_t, * EthMacMiiCtrlPtr_t;

/* ETH_MAC_MII_STATUS_REG info type. */
typedef struct {
  UINT32 phyStatus:16; 
  UINT32 reserved1:16; 
} EthMacMiiStatus_t, * EthMacMiiStatusPtr_t;

/* ETH_MAC_MII_INDIC_REG info type. */
typedef struct {
  UINT32 busy:1; 
  UINT32 scanning:1;
  UINT32 notValid:1;
  UINT32 reserved1:29; 
} EthMacMiiIndic_t, * EthMacMiiIndicPtr_t;

/* ETH_MAC_IF_CTRL_REG info type. */
typedef struct {
  UINT32 jabberProtectEnable:1; 
  UINT32 reserved1:6;
  UINT32 resetGpsi:1;
  UINT32 linkFailDisable:1;
  UINT32 noCipher:1;
  UINT32 forceQuiet:1;
  UINT32 reserved2:4;
  UINT32 resetPmd:1;
  UINT32 speed:1;
  UINT32 reserved3:6;
  UINT32 resetRmii:1;
  UINT32 phyMode:1;
  UINT32 lhdMode:1;
  UINT32 ghdMode:1;
  UINT32 ifMode:1;
  UINT32 reserved4:3;
  UINT32 resetIf:1;
} EthMacIfCtrl_t, * EthMacIfCtrlPtr_t;

/* ETH_MAC_IF_STATUS_REG info type. */
typedef struct {
  UINT32 jabberPe10t:1; 
  UINT32 sqeErr:1; 
  UINT32 lossCarrier:1; 
  UINT32 linkFail:1; 
  UINT32 speed:1; 
  UINT32 fullDuplex:1; 
  UINT32 linkOk:1; 
  UINT32 jabberMiiPhy:1; 
  UINT32 clash:1; 
  UINT32 excessDefer:1; 
  UINT32 reserved1:22; 
} EthMacIfStatus_t, * EthMacIfStatusPtr_t;

/* MII info type. */
typedef struct {
} EthMiiInfo_t, * EthMiiInfoPtr_t;

/* ETH_TBI_CTRL1_REG info type. */
typedef struct {
  UINT32 reserved1:6; 
  UINT32 speed1:1; 
  UINT32 reserved2:1; 
  UINT32 fullDuplex:1; 
  UINT32 resetAn:1; 
  UINT32 reserved3:2; 
  UINT32 anEnable:1; 
  UINT32 speed0:1; 
  UINT32 loopBack:1; 
  UINT32 phyReset:1; 
} EthTbiCtrl1_t, * EthTbiCtrl1Ptr_t;

/* ETH_TBI_STATUS_REG info type. */
typedef struct {
  UINT32 extdCapability:1; 
  UINT32 reserved1:1; 
  UINT32 linkStatus:1; 
  UINT32 anAbility:1; 
  UINT32 remoteFault:1; 
  UINT32 anComplete:1; 
  UINT32 noPreamble:1; 
  UINT32 reserved2:1; 
  UINT32 extdStatus:1; 
  UINT32 reserved3:7; 
} EthTbiStatus_t, * EthTbiStatusPtr_t;

/* ETH_TBI_AN_ADV_REG info type. */
typedef struct {
  UINT32 reserved1:5; 
  UINT32 fullDuplex:1; 
  UINT32 halfDuplex:1; 
  UINT32 pause:2; 
  UINT32 reserved2:3; 
  UINT32 remoteFault:2; 
  UINT32 reserved3:1; 
  UINT32 nextPage:1; 
} EthTbiAnAdv_t, * EthTbiAnAdvPtr_t;

/* ETH_TBI_AN_LP_BP_REG info type. */
typedef struct {
  UINT32 reserved1:5; 
  UINT32 fullDuplex:1; 
  UINT32 halfDuplex:1; 
  UINT32 pause:2; 
  UINT32 reserved2:3; 
  UINT32 remoteFault:2; 
  UINT32 reserved3:1; 
  UINT32 nextPage:1; 
} EthTbiAnLpBp_t, * EthTbiAnLpBpPtr_t;

/* ETH_TBI_AN_EXP_REG info type. */
typedef struct {
  UINT32 reserved1:1; 
  UINT32 pageReceived:1; 
  UINT32 nextPageAble:1; 
  UINT32 reserved2:13; 
} EthTbiAnExp_t, * EthTbiAnExpPtr_t;

/* ETH_TBI_AN_NP_TX_REG info type. */
typedef struct {
  UINT32 message:11; 
  UINT32 toggle:1; 
  UINT32 ack2:1; 
  UINT32 messagePage:1; 
  UINT32 reserved1:1; 
  UINT32 nextPage:1; 
} EthTbiAnNpTx_t, * EthTbiAnNpTxPtr_t;

/* ETH_TBI_AN_LP_ANP_REG info type. */
typedef struct {
  UINT32 message:11; 
  UINT32 toggle:1; 
  UINT32 ack2:1; 
  UINT32 messagePage:1; 
  UINT32 reserved1:1; 
  UINT32 nextPage:1; 
} EthTbiAnLpAnp_t, * EthTbiAnLpAnpPtr_t;

/* ETH_TBI_EXT_STATUS_REG info type. */
typedef struct {
  UINT32 reserved1:12; 
  UINT32 t1000Half:1; 
  UINT32 t1000Full:1; 
  UINT32 x1000Half:1; 
  UINT32 x1000Full:1; 
} EthTbiExtStatus_t, * EthTbiExtStatusPtr_t;

/* ETH_TBI_JITT_DIAG_REG info type. */
typedef struct {
  UINT32 jitterPattern:10; 
  UINT32 reserved1:2; 
  UINT32 jitterSelect:3; 
  UINT32 jitterEnable:1; 
} EthTbiJittDiag_t, * EthTbiJittDiagPtr_t;

/* ETH_TBI_CTRL2_REG info type. */
typedef struct {
  UINT32 commaDetect:1; 
  UINT32 wrapEnable:1; 
  UINT32 reserved1:2; 
  UINT32 ifMode:1; 
  UINT32 rxClkSelect:1; 
  UINT32 reserved2:2; 
  UINT32 anSense:1; 
  UINT32 reserved3:3; 
  UINT32 txDisparDisable:1; 
  UINT32 rxDisparDisable:1; 
  UINT32 shortCutLinkTmr:1; 
  UINT32 softReset:1; 
} EthTbiCtrl2_t, * EthTbiCtrl2Ptr_t;

/* the first 15 PHY registers are standard. */

/* ETH_PHY_CTRL_REG info type. */
typedef struct {
  UINT32 reserved1:6; 
  UINT32 speed1:1; 
  UINT32 cte:1; 
  UINT32 fullDuplex:1; 
  UINT32 resetAn:1; 
  UINT32 isolate:1; 
  UINT32 powerDown:1; 
  UINT32 anEnable:1; 
  UINT32 speed0:1; 
  UINT32 loopBack:1; 
  UINT32 phyReset:1; 
} EthPhyCtrl_t, * EthPhyCtrlPtr_t;

/* ETH_PHY_STATUS_REG info type. */
typedef struct {
  UINT32 extdCapability:1; 
  UINT32 jabberDetect:1; 
  UINT32 linkStatus:1; 
  UINT32 anAbility:1; 
  UINT32 remoteFault:1; 
  UINT32 anComplete:1; 
  UINT32 noPreamble:1; 
  UINT32 reserved2:1; 
  UINT32 extdStatus:1; 
  UINT32 t2_100Half:1; 
  UINT32 t2_100Full:1; 
  UINT32 t10Half:1; 
  UINT32 t10Full:1; 
  UINT32 tx100Half:1; 
  UINT32 tx100Full:1; 
  UINT32 t4_100:1; 
} EthPhyStatus_t, * EthPhyStatusPtr_t;

/* ETH_PHY_AN_ADV_REG info type. */
typedef struct {
  UINT32 selectorField:5; 
  UINT32 t10Half:1; 
  UINT32 t10Full:1; 
  UINT32 tx100Half:1; 
  UINT32 tx100Full:1; 
  UINT32 t4_100:1; 
  UINT32 pause:1; 
  UINT32 asymPause:1; 
  UINT32 reserved1:1; 
  UINT32 remoteFault:1; 
  UINT32 reserved2:1; 
  UINT32 nextPage:1; 
} EthPhyAnAdv_t, * EthPhyAnAdvPtr_t;

/* ETH_PHY_1000BASE_T_REG info type. */
typedef struct {
  UINT32 reserved1:8; 
  UINT32 t1000Half:1; 
  UINT32 t1000Full:1;   
  UINT32 hubDte:1; 
  UINT32 masterSlaveVal:1; 
  UINT32 masterSlaveEnable:1; 
  UINT32 testMode:3; 
} EthPhy1000BaseTCtrl_t, * EthPhy1000BaseTCtrlPtr_t;

/* ETH_PHY_EXT_STATUS_REG info type. */
typedef struct {
  UINT32 reserved1:12; 
  UINT32 t1000Half:1; 
  UINT32 t1000Full:1; 
  UINT32 x1000Half:1; 
  UINT32 x1000Full:1; 
} EthPhyExtStatus_t, * EthPhyExtStatusPtr_t;

#ifdef ETH_PHY_BCM5421

  /* Start of BCM5421-specific code */ 
    
  /* ETH_BCM_EXT_CTRL_REG info type. */
typedef struct {
  UINT32 gmiiFifoElasticity:1; 
  UINT32 reserved1:1; 
  UINT32 reserved2:1; 
  UINT32 ledOff:1; 
  UINT32 ledOn:1; 
  UINT32 ledTrafficModeEnable:1; 
  UINT32 resetScrambler:1; 
  UINT32 bypassRsa:1; 
  UINT32 bypassMlt3:1; 
  UINT32 bypassScrambler:1; 
  UINT32 bypass4b5b:1; 
  UINT32 forceInterrupt:1; 
  UINT32 interruptDisable:1; 
  UINT32 txDisable:1; 
  UINT32 autoMdiCrossOverDisable:1; 
  UINT32 ifMode:1; 
  } EthBcmExtCtrl_t, * EthBcmExtCtrlPtr_t;
  
  /* ETH_BCM_SUM_STATUS_REG info type. */
typedef struct {
  UINT32 txPauseEnable:1; 
  UINT32 rxPauseEnable:1; 
  UINT32 linkStatus:1; 
  UINT32 linkPartnerNPAbility:1; 
  UINT32 linkPartnerANAbility:1; 
  UINT32 anPageReceived:1; 
  UINT32 remoteFault:1; 
  UINT32 paralelDirFault:1; 
  UINT32 reportedStatus:3; 
  UINT32 anNPWait:1; 
  UINT32 anAbilityDetect:1; 
  UINT32 anAckDetect:1; 
  UINT32 anCompleteAck:1; 
  UINT32 anComplete:1; 
  } EthBcmSumStatus_t, * EthBcmSumStatusPtr_t;

/* BCM5421 summary status type. */
typedef enum {
    ETH_BCM_NOT_OK = 0, /* AN not completed or no common denominator. */     
    ETH_BCM_10_HALF = 1, /* 10BASE-T half-duplex. */     
    ETH_BCM_10_FULL = 2, /* 10BASE-T full-duplex. */     
    ETH_BCM_100TX_HALF = 3, /* 100BASE-TX half-duplex. */   
    ETH_BCM_100T4 = 4, /* 100BASE-T4. */   
    ETH_BCM_100TX_FULL = 5, /* 100BASE-T full-duplex. */     
    ETH_BCM_1000T_HALF = 6, /* 1000BASE-T half-duplex. */     
    ETH_BCM_1000T_FULL = 7, /* 1000BASE-T full-duplex. */     
  } EthBcmStatusMode_t;

/* End of BCM5421-specific code */ 

#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E
  /* Start of MARVELL88E-specific code */ 
    
  /* ETH_MARV_EXT_CTRL1_REG info type. */
  typedef struct {
    UINT32 disableJabber:1; 
    UINT32 polarityReversal:1; 
    UINT32 sqeTest:1; 
    UINT32 reserved1:1; 
    UINT32 disable125Clk:1; 
    UINT32 mdiCrossoverMode:2; 
    UINT32 rxThresh10T:1; 
    UINT32 reserved2:1; 
    UINT32 reserved3:1; 
    UINT32 forceLinkOk:1; 
    UINT32 assertCrcTx:1; 
    UINT32 rxFifoDepth:2; 
    UINT32 txFifoDepth:2; 
  } EthMarvExtCtrl1_t, * EthMarvExtCtrl1Ptr_t;
  
  /* ETH_MARV_EXT_CTRL2_REG info type. */
  typedef struct {
    UINT32 reserved1:4; 
    UINT32 txClkSpeed:3; 
    UINT32 reserved2:8; 
    UINT32 lostLockDetect:1; 
  } EthMarvExtCtrl2_t, * EthMarvExtCtrl2Ptr_t;
  
  /* ETH_MARV_SUM_STATUS_REG info type. */
  typedef struct {
    UINT32 jabber:1; 
    UINT32 polarity:1; 
    UINT32 reserved1:4; 
    UINT32 mdiCrossoverStatus:1; 
    UINT32 cableLen:3; 
    UINT32 linkStatus:1; 
    UINT32 resolvedStatus:1; 
    UINT32 pageReceived:1; 
    UINT32 duplexStatus:1; 
    UINT32 speedStatus:2; 
  } EthMarvSumStatus_t, * EthMarvSumStatusPtr_t;
  
  /* MARVELL88E summary status type. */
  typedef enum {
    ETH_MARV_10 = 0, /* 10Mbs. */     
    ETH_MARV_100 = 1, /* 100Mbs. */     
    ETH_MARV_1000 = 2, /* 1000Mbs. */     
    ETH_MARV_RES = 3, /* reserved. */     
  } EthMarvSpeedStatus_t;
  
  /* MARVELL88E interface mode type. */
  typedef enum {
    ETH_MARV_SERDES2COOPER = 0, /* SERDES to Cooper. */     
    ETH_MARV_TBI2COOPER = 1, /* TBI to Cooper. */    
    ETH_MARV_RESERVED = 2, /* reserved. */     
    ETH_MARV_GMII2COOPER = 3, /* GMII to Cooper. */          
  } EthMarvIfMode_t;
  
  /* End of MARVELL88E-specific code */ 
  
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
  
#endif 

#define ETH_PHY_UNRESOLVED (0)
/* PHY summary speed status type. */
typedef enum {
  ETH_PHY_SPEED_10 = 1, /* 10 Mbs. */     
  ETH_PHY_SPEED_100 = 2, /* 100 Mbs. */     
  ETH_PHY_SPEED_1000 = 3, /* 1000 Mbs. */     
} EthPhySpeed_t;

/* PHY summary link status type. */
typedef enum {
  ETH_PHY_LINK_DOWN = 1, /* link down. */     
  ETH_PHY_LINK_UP = 2, /* link up. */     
} EthPhyLink_t;

/* PHY summary duplex status type. */
typedef enum {
  ETH_PHY_DUPLEX_HALF = 1, /* half duplex. */     
  ETH_PHY_DUPLEX_FULL = 2, /* full duplex. */     
} EthPhyDuplex_t;

/* PHY summary AN status type. */
typedef enum {
  ETH_PHY_AN_NOT_COMPLETE = 1, /* AN not complete. */     
  ETH_PHY_AN_COMPLETE = 2, /* AN complete. */     
} EthPhyANstatus_t;

/* ETH_PHY_SUM_STATUS info type. */
typedef struct {
  UINT32 linkStatus:2; /* 0 = unresolved, 1 = down, 2 = up. */
  UINT32 speedStatus:2; /* 0 = unresolved, 1 = 10, 2 = 100, 3 = 1000 (Mbs). */
  UINT32 duplexStatus:2; /* 0 = unresolved, 1 = half, 2 = full. */
  UINT32 anStatus:2; /* 0 = unresolved, 1 = not complete (not enabled), 2 = complete. */
  UINT32 reserved:24; /* reserved. */
} EthPhySumStatus_t, * EthPhySumStatusPtr_t;


/* PHY info type. */
typedef struct {
  EthTbiCtrl1_t tbiCtrl1Info; /* TBI Control1 register info. */
  EthTbiAnAdv_t tbiAnAdvInfo; /* TBI AN Advertisement register info. */
  EthTbiCtrl2_t tbiCtrl2Info; /* TBI Control2 register info. */
  
/* the first 15 PHY registers are standard. */
  EthPhyCtrl_t phyCtrlInfo; /* PHY Control register info. */
  EthPhyAnAdv_t phyAnAdvInfo; /* PHY AN Advertisement register info. */
  EthPhy1000BaseTCtrl_t phy1000BaseTInfo; /* PHY 1000BASE-T register info. */
  
#ifdef ETH_PHY_BCM5421  
/* Start of BCM5421-specific code */ 
  EthBcmExtCtrl_t bcmExtCtrlInfo; /* BCM5421 PHY Extended Control register info. */
/* End of BCM5421-specific code */ 
#else /* ETH_PHY_BCM5421 */
  #ifdef ETH_PHY_MARV88E  
  /* Start of MARVELL88E-specific code */ 
    EthMarvExtCtrl1_t marvExtCtrl1Info; /* MARVELL88E Extended Control1 register info. */
    EthMarvExtCtrl2_t marvExtCtrl2Info; /* MARVELL88E Extended Control2 register info. */
    EthMarvIfMode_t marvIfMode; /* MARVELL88E interface mode info. */
  /* End of MARVELL88E-specific code */ 
  #else /* ETH_PHY_MARV88E */
    you must define the current PHY!
  #endif /* no PHY defined! */
#endif

} EthLinkInfo_t, * EthLinkInfoPtr_t;



/* MAC info type. */  
typedef struct {
  EthMacCfg1_t cfg1Info; /* config1 register info. */
  EthMacCfg2_t cfg2Info; /* config2 register info. */
  EthMacMaxFrm_t maxFrmInfo; /* max frame register info. */
  EthMacMiiCfg_t miiCfgInfo; /* MII config register info. */
  EthMacIfCtrl_t ifCtrlInfo; /* interface control register info. */
  EthMacAddr1_t addrPart1Info; /* address part1 register info. */
  EthMacAddr2_t addrPart2Info; /* address part2 register info. */
  EthLinkInfo_t linkInfo; /* Link partner info. */
} EthMacInfo_t, * EthMacInfoPtr_t;

/* ETH_PORT_CTRL_REG info type. */
typedef struct {
  UINT32 portPriority:1; 
  UINT32 reserved1:15; 
  UINT32 backPressureType:1; 
  UINT32 reserved2:1; 
  UINT32 speed:1; 
  UINT32 rbcMode:1; 
  UINT32 reserved3:1; 
  UINT32 prbsEnable:1; 
  UINT32 disableTxCgo:1; 
  UINT32 tbiOutputEnable:1; 
  UINT32 reserved4:4; 
  UINT32 statisticsEnable:1; 
  UINT32 zeroOnRead:1; 
  UINT32 clearStatistics:1; 
  UINT32 statusReset:1; 
} EthPortCtrl_t, * EthPortCtrlPtr_t;

/* ETH_PORT_INT_MASK_REG info type. */
typedef struct {
  UINT32 txQueues:4; 
  UINT32 reserved1:4; 
  UINT32 txWait:1; 
  UINT32 txThreshold:1; 
  UINT32 txUnderrun:1; 
  UINT32 txError:1; 
  UINT32 txAbort:1; 
  UINT32 txIdle:1; 
  UINT32 reserved2:1; 
  UINT32 statsCarry:1; 
  UINT32 rxQueues:4; 
  UINT32 reserved3:4; 
  UINT32 rxWait:1; 
  UINT32 rxThreshold:1; 
  UINT32 rxOverrun:1; 
  UINT32 rxError:1; 
  UINT32 rxAbort:1; 
  UINT32 rxIdle:1; 
  UINT32 ocnError:1;
  UINT32 reserved4:1; 
} EthPortIntrMask_t, * EthPortIntrMaskPtr_t;

/* ETH_PORT_INT_STATUS_REG info type. */
typedef struct {
  UINT32 txQueues:4; 
  UINT32 reserved1:4; 
  UINT32 txWait:1; 
  UINT32 txThreshold:1; 
  UINT32 txUnderrun:1; 
  UINT32 txError:1; 
  UINT32 txAbort:1; 
  UINT32 txIdle:1; 
  UINT32 reserved2:1; 
  UINT32 statsCarry:1; 
  UINT32 rxQueues:4; 
  UINT32 reserved3:4; 
  UINT32 rxWait:1; 
  UINT32 rxThreshold:1; 
  UINT32 rxOverrun:1; 
  UINT32 rxError:1; 
  UINT32 rxAbort:1; 
  UINT32 rxIdle:1; 
  UINT32 ocnError:1;
  UINT32 intrStatus:1; 
} EthPortIntrStatus_t, * EthPortIntrStatusPtr_t;

/* ETH_PORT_OCN_STATUS_REG info type. */
typedef struct {
  UINT32 ocnResponseHdr:24; 
  UINT32 ocnErrCount:8; 
} EthPortOcnStatus_t, * EthPortOcnStatusPtr_t;

/* ETH_MSTAT_CAR1_REG info type. */
typedef struct {
  UINT32 rdrp:1; 
  UINT32 rjbr:1; 
  UINT32 rfrg:1; 
  UINT32 rovr:1; 
  UINT32 rund:1; 
  UINT32 rcse:1; 
  UINT32 rcde:1; 
  UINT32 rflr:1; 
  UINT32 raln:1; 
  UINT32 rxuo:1; 
  UINT32 rxpf:1; 
  UINT32 rxcf:1; 
  UINT32 rbca:1; 
  UINT32 rmca:1; 
  UINT32 rfcs:1; 
  UINT32 rpkt:1; 
  UINT32 rbyt:1; 
  UINT32 reserved:8; 
  UINT32 trmgv:1; 
  UINT32 trmax:1; 
  UINT32 tr1k:1; 
  UINT32 tr511:1; 
  UINT32 tr255:1; 
  UINT32 tr127:1; 
  UINT32 tr64:1; 
} EthMstatCar1_t, * EthMstatCar1Ptr_t;

/* ETH_MSTAT_CARM1_REG info type. */
typedef struct {
  UINT32 rdrp:1; 
  UINT32 rjbr:1; 
  UINT32 rfrg:1; 
  UINT32 rovr:1; 
  UINT32 rund:1; 
  UINT32 rcse:1; 
  UINT32 rcde:1; 
  UINT32 rflr:1; 
  UINT32 raln:1; 
  UINT32 rxuo:1; 
  UINT32 rxpf:1; 
  UINT32 rxcf:1; 
  UINT32 rbca:1; 
  UINT32 rmca:1; 
  UINT32 rfcs:1; 
  UINT32 rpkt:1; 
  UINT32 rbyt:1; 
  UINT32 reserved:8; 
  UINT32 trmgv:1; 
  UINT32 trmax:1; 
  UINT32 tr1k:1; 
  UINT32 tr511:1; 
  UINT32 tr255:1; 
  UINT32 tr127:1; 
  UINT32 tr64:1; 
} EthMstatCarm1_t, * EthMstatCarm1Ptr_t;

/* ETH_MSTAT_CAR2_REG info type. */
typedef struct {
  UINT32 tdrp:1; 
  UINT32 tpfh:1; 
  UINT32 tncl:1; 
  UINT32 txcl:1; 
  UINT32 tlcl:1; 
  UINT32 tmcl:1; 
  UINT32 tscl:1; 
  UINT32 tedf:1; 
  UINT32 tdfr:1; 
  UINT32 txpf:1; 
  UINT32 tbca:1; 
  UINT32 tmca:1; 
  UINT32 tpkt:1; 
  UINT32 tbyt:1; 
  UINT32 tfrg:1; 
  UINT32 tund:1;   
  UINT32 tovr:1; 
  UINT32 txcf:1; 
  UINT32 tfcs:1; 
  UINT32 tjbr:1; 
  UINT32 reserved:12; 
} EthMstatCar2_t, * EthMstatCar2Ptr_t;

/* ETH_MSTAT_CARM2_REG info type. */
typedef struct {
  UINT32 tdrp:1; 
  UINT32 tpfh:1; 
  UINT32 tncl:1; 
  UINT32 txcl:1; 
  UINT32 tlcl:1; 
  UINT32 tmcl:1; 
  UINT32 tscl:1; 
  UINT32 tedf:1; 
  UINT32 tdfr:1; 
  UINT32 txpf:1; 
  UINT32 tbca:1; 
  UINT32 tmca:1; 
  UINT32 tpkt:1; 
  UINT32 tbyt:1; 
  UINT32 tfrg:1; 
  UINT32 tund:1;   
  UINT32 tovr:1; 
  UINT32 txcf:1; 
  UINT32 tfcs:1; 
  UINT32 tjbr:1; 
  UINT32 reserved:12; 
} EthMstatCarm2_t, * EthMstatCarm2Ptr_t;

/* MSTAT info type. */
typedef struct {
  EthMstatCarm1_t carm1Info;
  EthMstatCarm2_t carm2Info;
} EthMstatInfo_t, * EthMstatInfoPtr_t;

/* GigE port info type. */  
typedef struct {
  EthPortCtrl_t portCtrlInfo;
  EthPortIntrMask_t portIntrMaskInfo;
  EthMacInfo_t macInfo; /* MAC block info. */
  EthMstatInfo_t mstatInfo; /* MSTAT block info. */
  EthTxInfo_t txInfo; /* TX block info. */
  EthRxInfo_t rxInfo; /* RX block info. */
} EthPortInfo_t, * EthPortInfoPtr_t;

/******************************************************************************
 * Driver structures. They are used to object-oriented model the device. 
 */

/* TX/RX queue types. */
typedef struct {
  EthMemPool_t memPool; 
} EthTxQ_t, * EthTxQPtr_t;

typedef struct {
  EthMemPool_t memPool; 
  volatile EthCellFrm_t prevFrmCell; /* State-machine. Tracks the type of the previous
                               frame cell type. */
  EthSVFramePtr_t prevFrmCellPtr; /* Previous cell frame pointer. */                                                                                                 
  VUINT32 rxSofFrmStatus; /* The last SOF status info. */    
  VUINT16 vLanTag; /* VLAN tag info of the last SOF. */    
  volatile BOOL invalidFrm; /* Frame status info. */ 
} EthRxQ_t, * EthRxQPtr_t;

/* TX/RX block types. */
typedef struct {
  EthTxQ_t txQ[ETH_NUM_TX_Q];
} EthTx_t, * EthTxPtr_t;

typedef struct {
  EthRxQ_t rxQ[ETH_NUM_RX_Q];
} EthRx_t, * EthRxPtr_t;

/* MAC type. */  
typedef struct {
  EthTx_t tx; /* TX block. */
  EthRx_t rx; /* RX block. */
} EthMac_t, * EthMacPtr_t;

/* GigE port type. */  
typedef struct {
  EthMac_t mac; /* MAC block. */
  volatile BOOL portEnabled; /* Port status flag. */  
} EthPort_t, * EthPortPtr_t;

/* GigE block type. */
typedef struct {
  EthPort_t port[ETH_NUM_PORTS] /* port(0, 1). */; 
  EthPortInfoPtr_t portInfo[ETH_NUM_PORTS]; /* Port(0, 1) info, passed on at 
                                            init time. */  
  volatile BOOL drvEnabled; /* Driver status flag. */
  unsigned int intEnabled; /* Interrupt status flag. */
} Eth_t, * EthPtr_t;

/* Begins user-configurable section. 
*************************************/

/* wait loop counter, for now. */
#define ETH_MII_MNGM_WAIT_COUNT (100)   

/* User modifiable display flag. */
#define ETH_DISPLAY (1)

/* User modifiable configuration information id. */
#define ETH_CFG_INFO_IDX (0)

/* User modifiable memory space id. */
#define ETH_MEM_ALLOC_ID (ETH_OCN_SDRAM_ID)

/* Max number of PCI1/2 and SDRAM descriptors per queue. */
#define ETH_MAX_NUM_PCI_DESCR_PER_Q (32)

/* Max number of SRAM descriptors per queue. */
#define ETH_MAX_NUM_SRAM_DESCR_PER_Q (5)
  
/* Max number of PCI1/2 and SDRAM buffers per descriptor. */
#define ETH_NUM_PCI_BUFF_PER_DESCR (3)

/* Max number of SRAM buffers per descriptor. */
#define ETH_NUM_SRAM_BUFF_PER_DESCR (3)

/* memory buffer size on PCI1/2 and SDRAM. */
#define ETH_MAX_PCI_BUFF_SIZE (ETH_BUFF_SIZE_256)

/* memory buffer size on SRAM. */
#define ETH_MAX_SRAM_BUFF_SIZE (ETH_BUFF_SIZE_96)

/* Number of descriptors per queue. */
#define ETH_NUM_DESCR_PER_Q (ETH_MAX_NUM_PCI_DESCR_PER_Q)

/* Number of buffers per descriptor. */
#define ETH_NUM_BUFF_PER_DESCR (ETH_NUM_PCI_BUFF_PER_DESCR)

/* Buffer size. */
#define ETH_MAX_BUFF_SIZE (ETH_MAX_PCI_BUFF_SIZE)

/* more for buffer allocation on PCI1/2 and SDRAM. 
   less for buffer allocation on SRAM. */
#define ETH_BUFF_MEM_SIZE (ETH_NUM_PORTS * (ETH_NUM_TX_Q + ETH_NUM_RX_Q) * \
                           ETH_NUM_DESCR_PER_Q * ETH_NUM_BUFF_PER_DESCR * \
                           ETH_MAX_BUFF_SIZE)
                           
/* Number of alocated buffers. */
#define ETH_NUM_BUFFS (ETH_BUFF_MEM_SIZE / ETH_MAX_BUFF_SIZE)
                           
/* more for descriptor allocation on PCI1/2 and SDRAM.
   less for descriptor allocation on SRAM. */
#define ETH_DESCR_MEM_SIZE (ETH_NUM_PORTS * (ETH_NUM_TX_Q + ETH_NUM_RX_Q) * \
                           ETH_NUM_DESCR_PER_Q * sizeof(EthBuffDescr_t))

/* User modifiable OCN TX/RX burst size. */
#define ETH_OCN_BURST_SIZE (ETH_OCN_BURST_256)

/* PCI1/2 and SRAM descriptor memory types. */ 
 
typedef struct {
  EthBuffDescr_t descrTable[ETH_NUM_DESCR_PER_Q]; 
} EthDescrQ_t, * EthDescrQPtr_t;

typedef struct {
  EthDescrQ_t descrQ[ETH_NUM_TX_Q]; 
} EthDirectDescr_t, * EthDirectDescrPtr_t;

typedef struct {
  EthDirectDescr_t directDescrQ[ETH_NUM_PORT_DIRECTIONS]; 
} EthPortDescr_t, * EthPortDescrPtr_t;

typedef struct {
  EthPortDescr_t portDescr[ETH_NUM_PORTS]; 
} EthMemDescr_t, * EthMemDescrPtr_t;

/* Ends user-configurable section. 
*************************************/


#ifdef ETH_REDBOOT  
  
  #ifndef _SPAC_REDBOOT_
  #include <cyg/devs/eth/SpacRedBoot.h>
  #endif
 
  #ifndef _SPAC_QMNG_H_
  #include <cyg/devs/eth/SpacQMng.h>
  #endif
  
/* Demux type. */
typedef enum {
  ETH_DEMUX_LOOPBACK = 0,   
  ETH_DEMUX_DISCARD = 1,   
} EthDemux_t;
  
/* Test status type. */
typedef enum {
  ETH_TEST_FAILED = -1,
  ETH_TEST_NOT_RUN = 0,
  ETH_TEST_RUN = 1,
  ETH_TEST_PASSED = 2,
} EthTestStatus_t, * EthTestStatusPtr_t;

/* Port (test) done type. */
typedef struct {
  volatile EthTestStatus_t direction[ETH_NUM_PORT_DIRECTIONS]; /* TX = 0, RX = 1. */
} EthTestDonePort_t, * EthTestDonePortPtr_t;


/* Test done type. */
typedef struct {
  EthTestDonePort_t port[ETH_NUM_PORTS]; /* port0 = 0, port1 = 1. */
} EthTestDone_t, * EthTestDonePtr_t;

/* Queue interrupt (test) done type. */
typedef enum {
  ETH_Q_DESCR_INTR = 0,
  ETH_Q_EOF_INTR = 1, 
  ETH_Q_EOQ_INTR = 2, 
} EthQIntr_t, * EthQIntrPtr_t;

/* Test id type. */
typedef enum {
  ETH_TEST_2_2 = 0,
  ETH_TEST_2_3 = 1,
  ETH_TEST_3_2and3_3 = 2,
  ETH_TEST_3_4 = 3,
  ETH_TEST_3_5 = 4,
  ETH_TEST_3_7 = 5,
  ETH_TEST_4_1 = 6,
  ETH_TEST_5_1 = 7,
  ETH_TEST_5_2 = 8,
  ETH_TEST_5_3 = 9,
  ETH_TEST_5_4and6_1 = 10,
  ETH_TEST_6_2and6_7 = 11,
  ETH_TEST_6_5 = 12,
  ETH_TEST_6_6 = 13,
  ETH_TEST_6_8 = 14,
  ETH_TEST_7_9 = 15,
  ETH_TEST_6_4and7_8 = 16,
  ETH_TEST_7_1 = 17,
  ETH_TEST_7_2 = 18,
  ETH_TEST_8_3and6_3 = 19,
  ETH_TEST_8_4and6_3 = 20,
  ETH_TEST_9_1 = 21,
  ETH_TEST_LAST, 
} EthTestId_t, * EthTestIdPtr_t;

/* Begins user-configurable section. 
*************************************/

/* Max number of buffer bytes to be dumped. */
#define ETH_NUM_DUMPED_BYTES (100)

/* User modifiable queue manager size. */
#define ETH_QMNG_SIZE (ETH_NUM_BUFFS)

/* User modifiable queue manager cell size. */
#define ETH_MAX_QMNG_CELL_SIZE (ETH_MAX_BUFF_SIZE)

/* define/undefine own allocator in the SDRAM high memeory. */
//#define HIGH_MEM_ALLOC

#define SDRAM_HIGH_MEM_CACHE (0x0C000000)

#define SDRAM_HIGH_MEM_UNCACHE (0x2C000000)

#define SDRAM_HIGH_MEM (SDRAM_HIGH_MEM_CACHE)

/*  #define ETH_DIAG_EXTENDED    */
  
  #ifdef ETH_DIAG_EXTENDED
    #define ETH_DESCR_WORD_CLEAR_VAL (0x0C00400f)
  #else 
    #define ETH_DESCR_WORD_CLEAR_VAL (0x00000000)
  #endif
  
/* Added to conditionally compile debug code. */  

/*    #define ETH_TX_4DEBUG */

/*  This is a SW workaround for the race between DMA read descriptor and
    XScale cache line flush operations. 
        
*/  
  #define ETH_SW_WORK_AROUND_148   


  #ifdef ETH_SW_WORK_AROUND_148
/* Flush line cache after updating the data address, and after update the ownership. 
    #define ETH_DESCR_DOUBLE_FLUSH
*/  


/* Higher the DMA priority for operations via OCN. 

*/
    #define ETH_RISE_DMA_PRIORITY
    
    #ifdef ETH_RISE_DMA_PRIORITY
    
      #define ETH_SDRAM_BASE_ADDR_LO (0x4000)      
      #define ETH_SDRAM_CTRL_REG (ETH_SDRAM_BASE_ADDR_LO + 0x004) 
      #define ETH_SDRAM_CTRL_PORT_PRI (3 << 8)      
      #define ETH_SDRAM_CTRL_OCN_PRI (1 << 8)      
    
    #endif  /*  ETH_RISE_DMA_PRIORITY  */

  
  #endif  /*  ETH_SW_WORK_AROUND_148  */

/*  This is a SW workaround for the lack of EOQ signaling when a RX ERR 
    condition arise. 
        
*/  
  #define ETH_SW_WORK_AROUND_143   


/*  Capture undersized frames. Debug code only. 
  #define ETH_CAPTURE_UNDERSIZE   

*/  

  #ifdef ETH_CAPTURE_UNDERSIZE
    #define ETH_RX_UNDERSIZE_FRM (64)
  #endif

/* Count before LinkConfig times out */
#define MAX_THRESHOLD_COUNT 500000  

/* Ends user-configurable section. 
*************************************/

#endif  /* ETH_REDBOOT */


#endif  /* _SPAC_ETH_DEFS_H_ */

