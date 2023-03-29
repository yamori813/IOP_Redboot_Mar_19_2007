/************************************************************************;
;*                                                                      
;*      Filename: sata_globals.h  								 
;*                                                                      
;*      Intel(r) Restricted Use                                      
;*                                                                      
;*      Intel Sample Serial ATA Code                          
;*                                                                      
;*      SCD Advanced Engineering                               
;*                                                                      
;*	Copyright:    (C) 2003-2004 Intel Corporation.          
;*                                                                      
;*      Version 1.00                                                    
;*                                                                      
;*      This program has been developed by Intel Corporation.           
;*      Licensee has Intel's permission to incorporate this source code 
;*      into their product, royalty free.  This source code may NOT be  
;*      redistributed to anyone without Intel's written permission.     
;*                                                                      
;*      Intel specifically disclaims all warranties, express or         
;*      implied, and all liability, including consequential and other   
;*      indirect damages, for the use of this code, including liability 
;*      for infringement of any proprietary rights, and including the   
;*      warranties of merchantability and fitness for a particular      
;*      purpose.  Intel does not assume any responsibility for any      
;*      errors which may appear in this code nor any responsibility to  
;*      update it.                                                      
;************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                                                            */
/*   DESCRIPTION                                                              */
/*                                                                            */
/*       This file contains all the global declarations for the SATA library. */
/*                                                                            */
/******************************************************************************/

#ifndef SATA_GLOBALS_HEADER
#define SATA_GLOBALS_HEADER

/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver */
#include <sata_adapter.h>
#include <sata_controller.h>
#include <sata_port.h>
#include <sata_device.h>

/******************************************************************************/
/*                                                                            */
/*    V A R I A B L E   A N D   S T R U C T U R E   D E F I N I T I O N S     */
/*                                                                            */
/******************************************************************************/
#if 0



typedef struct ADAPTER_INFO_S
{
    boolean present;        /* Adapter present and struct data valid ?        */
    INT controller[MAX_CONTROLLERS_PER_ADAPTER];/*Index #s into controllers_gp*/
    INT drives[MAX_DEVICES_PER_ADAPTER]; /* Index #s of drives in this adapter*/
  //  INT osdmTag;           /* Tag registered by the OSDM during initialization*/
    INT driveLastServiced; /* Index number of the drive last serviced.        */
} ADAPTER_INFO_T;



typedef struct CONTROLLER_INFO_S
{
    boolean  present;    /* controller present and struct data valid ?        */
    INT     port[MAX_PORTS_PER_CONTROLLER]; /* Index #s into ports_gp         */
    INT     adapter;     /* Index number of the parent adapter                */
    INT     controllerNumber; /* 0 - n controller on the adapter              */
    SATA_CONTROLLER_MODE_T mode; /* Mode of operation                         */
    SATA_REG_ADDR_T commonRegBase; /* Base address for common SATA registers  */
} CONTROLLER_INFO_T;


typedef struct PORT_INFO_S
{
    boolean   present;    /* port present and struct data valid ?             */
    INT       dev[MAX_DEVICES_PER_PORT]; /* Index #s into devices_gp          */
    INT       controller; /* Index number of the parent controller            */
    INT       portNumber; /* 0 - n port on the controller                     */
    SATA_REG_ADDR_T portRegBase; /* Base address for common SATA registers    */
    boolean   connected;  /* Reflects the last known connection status        */
    UINT32    cumulativeSSER; /* Cumulative value of SUPDSSER bits   till now */
    UINT32    lastSSER;       /* Value of SUPDSSER last read                  */
    UINT32    lastER;         /* Value of SUPDER last read                    */
    UINT32    lastASR;        /* Value of SUPDASR last read                   */
    UINT32    lastSR;         /* Value of SUPDSR last read                    */
    UINT32    lastSSSR;       /* Value of SUPDSSSR last read                  */
	UINT32    testResults;	  /* Status of testing                            */
} PORT_INFO_T;



/* Note: The devices could be RSMs rather than end devices. */
/* Currently RSM will not be supported in this design.      */

typedef struct DEV_INFO_S
{
    boolean present;             /* device present and struct data valid ?    */
    boolean active;              /* device active? */
	INT port;                    /* Index number of the connected port        */
    SATA_TRANSFER_TYPE_T ioMode; /* force type of data transfer pio/dma/cmd dependent */
    SATA_COMMAND_T commandQueue[MAX_TCQ_DEPTH]; /* simple Command Queue */
    PSATA_COMMAND_T cmdFreeQueue;    /* pointer to free Command list */
   // OS_SPINLOCK_T   cmdQLock; /* spinlock for command queue */
   // OS_SPINLOCK_T   freeCmdQLock; /* spinlock for free command list */
    void * osDriveTag;      /* The OS's Drive Tag.                            */
    void * dmaHandler;      /* OS specific Dma Handler's address              */
    void * pioReadHandler;  /* OS specific PIO Handler's address              */
    void * pioWriteHandler; /* OS specific PIO Handler's address              */
    boolean isDrive;        /* Is this sata device a drive or something else? */
    UINT64 lbaCapacity;     /* Can hold LBA 48 values                         */
//  INT multisector;        /* Current multisector values 0 = disabled        */
    IDENTIFY_DEVICE_T devIdentify; /* Store devIdentify info for debug uses   */
    PSATA_FEATURES_T  devFeatures; /* Store devFeatures info */
    SATA_COMMAND_T lastCommand; /* For Debug purposes */
	boolean	waiting_for_dma; /* we're waiting for dma to complete on this drive */  
} DEV_INFO_T;



typedef struct SATA_COMMAND_S * PSATA_COMMAND_T;

typedef struct SATA_COMMAND_S
{
    SATA_COMMAND_SLOT_STATE_T slotState; /* Command slot state in a TCQ       */
    SATA_TRANSFER_TYPE_T    txType;   /* Type of data transfer                */
    SATA_COMMAND_TYPE_T     cmdType;  /* I/O or special command               */
    SATA_TASKFILE_T         taskfile; /* taskfile for this command            */
    INT						sectRemaining; /* Count of sectors yet to be transfered*/
    DMA_INFO_T              dma;      /* associated dmaInfo                   */
    void *					buffer;   /* The Source/Destination address for the transfer */
    UINT8                   status;   /* Copy of SATA Altstatus reg (SUPDASR) */
    UINT32                  sStatus;  /* Copy of SATA SStatus reg (SUPDSSSR)  */
    UINT8                   error;    /* Copy of SATA error reg (SUPDER)      */
    UINT32                  sError;   /* Copy of SATA SError reg (SUPDSSER)   */
    PSATA_COMMAND_T         prevCmd;  /* previous command in list */
    PSATA_COMMAND_T         nextCmd;  /* next command in list */
} SATA_COMMAND_T;

#endif

//const INT INVALID_TAG = -1; /* invalid {adapter,controller,port,device}Tag    */
#define INVALID_TAG -1

/* The following pointer arrays are Initialized to NULL_POINTERs in InitLib() */

/* Array of pointers to adapterInfo structures.                               */
extern ADAPTER_INFO_T *adapters_gp[MAX_ADAPTERS];

/* Array of pointers to controllerInfo structures.                            */
extern CONTROLLER_INFO_T *controllers_gp[MAX_CONTROLLERS];

/* Array of pointers to portInfo structures.                                  */
extern PORT_INFO_T *ports_gp[MAX_PORTS];

/* Array of pointers to devInfo structures.                                   */
extern DEV_INFO_T *devices_gp[MAX_DEVICES];

/* Global debug counter */
extern INT sataDebugCounter;

/* Global device feature list */
extern SATA_FEATURES_T devFeatures_g;

#endif /* SATA_GLOBALS_HEADER */
