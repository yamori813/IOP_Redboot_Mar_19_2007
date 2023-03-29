/************************************************************************;
;*                                                                      
;*      Filename: sata.h  								 
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
#ifndef SATA_HEADER
#define SATA_HEADER


/******************************************************************************/
/*                                                                            */
/*                  I N C L U D E   S T A T E M E N T S                       */
/*                                                                            */
/******************************************************************************/

#include <sata_types.h> /* Common data types used in SATA driver */

/******************************************************************************/
/*                                                                            */
/*                C O N S T A N T   D E F I N I T I O N S                     */
/*                                                                            */
/******************************************************************************/

/* General Driver Configuration Constants */

/* ATA currently has a hard coded standard block size of 512 */
#define SATA_SECTOR_SIZE      (512)



/* SATA commands to be sent to the Drive */
/* Non Data commands */
#define ATA_NOP               (0x00)
#define ATA_DEVICE_RESET      (0x08)
#define ATA_READ_VERIFY	      (0x40)
#define ATA_READ_VERIFY_EXT   (0x42)
#define ATA_SEEK		      (0x70)
#define ATA_EXEC_DIAGNOSTIC	  (0x90)
#define ATA_INIT_DEV_PARAM    (0x91)	/* set drive geometry translation */
#define ATA_QUEUED_SERVICE    (0xA2)	/* */
#define ATA_SET_MULTIPLE	  (0xC6)	/* enable/disable multiple mode */
#define ATA_GET_MEDIA_STATUS  (0xDA)
#define ATA_STANDBY_IMMEDIATE (0xE2)
#define ATA_IDLE_IMMEDIATE    (0xE1)	/* force drive to become "ready" */
#define ATA_STANDBY    	      (0xE2)
#define ATA_IDLE    	      (0xE3)
#define ATA_CHECK_POWER_MODE  (0xE5)
#define ATA_SLEEP             (0xE6)
#define ATA_FLUSH_CACHE       (0xE7)
#define ATA_FLUSH_CACHE_EXT   (0xEA)    /* 48-Bit */
#define ATA_IDENTIFY_DEVICE	  (0xEC)	/* ask drive to identify itself	*/
#define ATA_SET_FEATURES      (0xEF)

/* Data In commands */
#define ATA_READ_SECTORS            (0x20)
#define ATA_READ_SECTORS_EXT        (0x24)
#define ATA_READ_MULTIPLE	        (0xC4)	/* read sectors using multiple mode */
#define ATA_READ_MULTIPLE_EXT       (0x29)  /* LBA48 read multiple */
#define ATA_READ_DMA_QUEUED	        (0xC7)	/* read sectors using Queued DMA transfers */
#define ATA_READ_DMA_QUEUED_EXT     (0x26)  /* LBA48 */
#define ATA_READ_DMA    	        (0xC8)	
#define ATA_READ_DMA_EXT            (0x25)  /* LBA48 DMA read */
#define ATA_READ_VERIFY_SECTORS     (0x40)  /* LBA48 rvs */
#define ATA_READ_VERIFY_SECTORS_EXT (0x42)    /* LBA48 read verify */

/* Data Out commands */
#define ATA_WRITE_SECTORS           (0x30)
#define ATA_WRITE_SECTORS_EXT       (0x34)
#define ATA_WRITE_VERIFY	        (0x3C)
#define ATA_WRITE_MULTIPLE	        (0xC5)	/* write sectors using multiple mode */
#define ATA_WRITE_MULTIPLE_EXT      (0x39)
#define ATA_WRITE_DMA               (0xCA)
#define ATA_WRITE_DMA_EXT           (0x35)    /* LBA48 DMA write */
#define ATA_WRITE_DMA_QUEUED        (0xCC)	/* write sectors using Queued DMA transfers */
#define ATA_WRITE_DMA_QUEUED_EXT    (0x36)

/* ATA Status Register Masks */
#define ATA_ERR_BIT_MASK	  (0x01)

/*
 * Timeouts for various operations: in msec
 */
#define WAIT_DRQ              (50)      /* 50msec - spec allows up to 20ms  */
#define WAIT_READY            (30)      /* 30msec - should be instantaneous */
#define WAIT_PIDENTIFY	      (3)       /* 3msec  - should be less than 3ms */
#define WAIT_WORSTCASE        (30000)   /* 31sec  - worst case when spinning up */
#define WAIT_CMD              (10000)   /* 10sec  - maximum wait for an IRQ to happen */
#define WAIT_MIN_SLEEP        (20)      /* 20msec - minimum sleep time */
#define WAIT_EXEC_DIAGNOSTIC  (2000)    /* 2 sec */


/* Bit Position definitions */
#define DEVICE_HEAD_LBA_BIT   (6)

/* Contants to support SATA_FEATURES_S */
#define ATA_ENABLE_PIO_TRANSFER	0x01
#define ATA_ENABLE_WRITE_CACHE	0x02
#define ATA_ENABLE_APM			0x05	//Advanced Power Mgt
#define ATA_ENABLE_PUSFS		0x06	// PowerUp Standby Feature Set.
#define ATA_STANDBY_TO_ACTIVE	0x07
#define ATA_ENABLE_CFA1			0x0A
#define ATA_ENABLE_ACOUSTIC		0x42
#define ATA_ENABLE_REL_INT		0x5D
#define ATA_ENABLE_RLA			0xAA	// Read look ahead.

#define ATA_DISABLE_RLA			 0x55
#define ATA_DISABLE_PIO_TRANSFER 0x81
#define ATA_DISABLE__WRITE_CACHE 0x82
#define ATA_DISABLE_APM			 0x85
#define ATA_DISABLE_PUSFS		 0x86
#define ATA_DISABLE_CFA1		 0x8A
#define ATA_DISABLE_ACOUSTIC 	 0xC2
#define ATA_DISABLE_REL_INT		 0xDD

/*
 * Identify Device constant definitions
 */
#define ATA_MODEL_NUMBER_LENGTH    (20*2) /* Model # is 20 Words or 40 bytes */
#define ATA_SERIAL_NUMBER_LENGTH   (10*2) /* Serial # is 10 Words or 20 bytes*/


/* Status and AltStatus register bits */
#define SR_BSY_BIT     (7)  /* RO: 1= the interface/device is busy.   */
#define SR_DRDY_BIT    (6)  /* RO: 1= indicates that the dev is ready.*/
#define SR_DRQ_BIT     (3)  /* RO: 1= dev is ready with/for PIO data  */
#define SR_ERR_BIT     (0)  /* RO: 1= indicates that an error occurred*/

/* Device Control Register bits  */
#define CTLR_HOB_BIT   (7)  /* High Order Byte                        */
#define CTLR_SRST_BIT  (2)  /* WO: 1 = Start Reset, 0 = Stop reset    */
#define CTLR_nIEN_BIT  (1)  /*     0=disable 1=enable Interrupt       */

/******************************************************************************/
/*                                                                            */
/*                   M A C R O   D E F I N I T I O N S                        */
/*                                                                            */
/******************************************************************************/

/* Returns the bit located at "bit_pos" in "val" */
#define READ_BIT(val, bit_pos) (( (val) >> (bit_pos) ) & 0x1 )

/* Resets the bit located at "bit_pos" in "val" to 0.DOES NOT assign the value to "val" */
#define RESET_BIT(val, bit_pos) ( (val) & ~( 0x1 << (bit_pos) ) )

/* Sets the bit located at "bit_pos" in "val" to 1. DOES NOT assign the value to "val" */
#define SET_BIT(val, bit_pos) ( (val) | ( 0x1 << (bit_pos) ) )


/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/

typedef enum SATA_CONTROLLER_MODE_E
{
    LEGACY  = 0, /* Legacy mode of operation             */
    PCI_IDE = 1, /* PCI_IDE mode of operation            */
    PCI_DPA = 2  /* Direct Port Access mode of operation */
} SATA_CONTROLLER_MODE_T;

typedef enum SATA_TRANSFER_TYPE_E
{
    FIRST_PARTY_DMA = 0, /* Use 1st party DMA for data transfer             */
    THIRD_PARTY_DMA = 1, /* Use 3rd party DMA for data transfer             */
    PIO             = 2, /* Use PIO for data transfer                       */
    VARIABLE        = 3  /* Internal to Lib, I/O command specifies the mode */
} SATA_TRANSFER_TYPE_T;

typedef enum SATA_COMMAND_TYPE_E
{
    SATA_READ    = 0, /* Data Read commands        */
    SATA_WRITE   = 1, /* Data Write commands       */
    SATA_SPECIAL = 4  /* Status / Control commands */
} SATA_COMMAND_TYPE_T;

typedef enum SATA_COMMAND_SLOT_STATE_E
{
    SLOT_FREE     = 0, /* Command slot in the Queue is free to be re-used    */
    SLOT_OCCUPIED = 1, /* Command slot is in use. Command added to the queue */
                       /* but not issued to the device */
    SLOT_ACTIVE   = 2  /* Command slot is in us. Command issued to the device*/
} SATA_COMMAND_SLOT_STATE_T;

/******************************************************************************/
/* This structure will model the ATA Task file.  The task file is a set of    */
/* registers with specific purposes for relating commands between the         */
/* controller and the device.                                                 */
/*                                                                            */
/* There are a couple of registers that have duplex roles.  These registers   */
/* are the Feature/Error and Command/Status.  These roles change depending on */
/* whether you are writing to or reading from the task file. If you are       */
/* writing to the task file then you are using the feature and command        */
/* registers. Consequently if you are reading from them you are using the     */
/* Error and Status registers.                                                */
/*                                                                            */
/* Our implementation will use Logical Block Addressing (LBA) instead of      */
/* Cylinder/Head/Sector.  As per the ATA specification the sector number,     */
/* cylinder low, cylinder high, and part of the device head registers will be */
/* used to specify the LBA.							                          */
/* For LBA28:                                                                 */
/*   Sector Count - Maximum value of 0xFF                                     */
/*   Sector Number - LBA bits 7:0 (lower byte)                                */
/*   Cylinder Low  - LBA bits 15:8 (lower byte)                               */
/*   Cylinder High - LBA bits 23:16 (lower byte)                              */
/*   Device Head   - LBA bits 27:24 in lower nibble of device head (bits 3:0) */
/*                                                                            */
/* For LBA48:                                                                 */
/*   Sector Count - Maximum value of 0xFFFF                                   */
/*   Sector Number - LBA bits 31:24 & 7:0                                     */
/*   Cylinder Low  - LBA bits 39:32 & 15:8                                    */
/*   Cylinder High - LBA bits 47:40 & 23:16                                   */
/*   Device Head   - bottom nibble all 1's                                    */
/******************************************************************************/
#pragma pack(1)
typedef struct SATA_TASKFILE_S
{
    UINT16 feature;         /* Feature Register Values             */
    UINT16 sectorCount;     /* Sector Count Register Values        */
    UINT16 sectorNumber;    /* Sector Number Register Values       */
    UINT16 cylinderLow;     /* Cylinder Low Register Values        */
    UINT16 cylinderHigh;    /* Cylinder High Register Values       */
    UINT8 deviceHead;       /* Device Head Register Values         */
    UINT8 command;          /* Command Register Values             */
} SATA_TASKFILE_T;
#pragma pack()


#pragma pack(1)
/* TBD: NOTE: Check if you need to swap the order of UCHAR fields
   TBD: Verify that the size of the structure is 512 bytes.
        pack with byte alignment */
typedef struct IDENTIFY_DEVICE_S
{
 UINT16   configBits;                             /* word 00              */
 UINT16   numCylinders;                           /* word 01              */
 UINT16   reserved0;                              /* word 02              */
 UINT16   numHeads;                               /* word 03              */
 UINT16   retired1[2];                            /* word 04-05           */
 UINT16   sectorsPerTrack;                        /* word 06              */
 UINT16   vendorUnique0[2];                       /* word 07-08           */
 UINT16   retired2;                               /* word 09              */
 UINT8    serialNumber[ATA_SERIAL_NUMBER_LENGTH];/* word 10-19           */
 UINT16   retired3[2];                            /* word 20-21           */
 UINT16   obselete0;                              /* word 22              */
 UINT8    firmwareRevision[8];                    /* word 23-26           */
 UINT8    modelNumber[ATA_MODEL_NUMBER_LENGTH];  /* word 27-46           */
 UINT16   maxSectorsPerMultiple;                  /* word 47              */
 UINT16   reserved1;                              /* word 48              */
 UINT16   capabilities;                           /* word 49              */
 UINT16   extraCapabilities;                      /* word 50              */
 UINT16   obsolete[2];                            /* word 51-52           */
 UINT16   validityBits;                           /* word 53              */
 UINT16   currentCylinders;                       /* word 54              */
 UINT16   currentHeads;                           /* word 55              */
 UINT16   currentSectorsPerTrack;                 /* word 56              */
 UINT16   currentCapacity[2];                     /* word 57-58           */
 UINT16   currentMaxSectorsPerMultiple;           /* word 59              */
 UINT16   totalNumSectors[2];                     /* word 60-61           */
 UINT16   obselete1;                              /* word 62              */
 UINT16   multiWordDmaMode;                       /* word 63              */
 UINT16   advancedPioModesSupported;              /* word 64              */
 UINT16   minMultiDmaXferCycle;                   /* word 65              */
 UINT16   recMinMultiDmaXferCycle;                /* word 66              */
 UINT16   minPioXferNoFlowCtrl;                   /* word 67              */
 UINT16   minPioXferWithFlowCtrl;                 /* word 68              */
 UINT16   reserved2[2];                           /* word 69-70           */
 UINT16   reserved3[4];                           /* word 71-74           */
 UINT16   queueDepth;                             /* word 75              */
 UINT16   reserved4[4];                           /* word 76-79           */
 UINT16   majorVersionNumber;                     /* word 80              */
 UINT16   minorVersionNumber;                     /* word 81              */
 UINT16   commandSetSupported0;                   /* word 82              */
 UINT16   commandSetSupported1;                   /* word 83              */
 UINT16   commandSetSupportedExt;                 /* word 84              */
 UINT16   commandSetEnabled0;                     /* word 85              */
 UINT16   commandSetEnabled1;                     /* word 86              */
 UINT16   commandSetDefault;                      /* word 87              */
 UINT16   ultraDmaMode;                           /* word 88              */
 UINT16   securityEraseCompletion;                /* word 89              */
 UINT16   enhancedSecurityErase;                  /* word 90              */
 UINT16   currentPowerMgtValue;                   /* word 91              */
 UINT16   masterPasswordRevision;                 /* word 92              */
 UINT16   comResetResult;                         /* word 93              */
 UINT16   acousticManagementValue;                /* word 94      ATAPI-6 */
 UINT16   streamMinRequestSize;                   /* word 95      ATAPI-6 */
 UINT16   streamTransferTime;                     /* word 96      ATAPI-6 */
 UINT16   streamAccessLatency;                    /* word 97      ATAPI-6 */
 UINT16   streamPerformanceGranularity[2];        /* word 98-99   ATAPI-6 */
 UINT16   totalNumSectors48Bit[4];                /* word 100-103 ATAPI-6 */
 UINT16   reserved5[23];                          /* word 104-126         */
 UINT16   removableMediaStatus;                   /* word 127             */
 UINT16   securityStatus;                         /* word 128             */
 UINT16   vendorUnique1[31];                      /* word 129-159         */
 UINT16   cfaPowerMode1;                          /* word 160             */
 UINT16   reserved6[94];                          /* word 161-254         */
 UINT16   integrityWord;                          /* word 255             */

} IDENTIFY_DEVICE_T, *PIDENTIFY_DEVICE_T;
#pragma pack()

#pragma pack(1)
/*******************************************************************
The SATA_FEATURES_T structure is a store for the existance of all of
the SATA features that may or may not be enabled through the Identify
Device - Set Features initialization sequence.

The values will be set to true for enabled, or false for
disabled, although a user may place other non-zero values in the
variable location representing a supported feature for implementation
specific purposes.

.dh. This structure currently ignores Packet commands/ATAPI capabilities.
********************************************************************/
typedef struct SATA_FEATURES_S
{
    // PIO: When Enabled, value shall be # Sectors to transfer per interrupt.
	UCHAR ReadMultiple; 	// word 59
    UCHAR MultiwordDma;     // word 63
    UCHAR QueueDepth;       // word 75
    UCHAR Atapi_6;     		// word 80
	// PIO: Support of READ_BUFFER Command.
    UCHAR ReadBuffer;       // word 82
	// PIO: Support of WRITE_BUFFER Command.
    UCHAR WriteBuffer;      // word 82
	UCHAR HostProtectedArea;// word 82
	UCHAR DeviceResetCmd;	// word 82
	UCHAR WriteCache;		// word 82 + SET FEATURES
	UCHAR LookAhead;	    // word 82 + SET FEATURES
    UCHAR PwrUpStandby;		// word 82 + SET FEATURES
    UCHAR SecurityMode;		// word 82, default enabled if supported.
	UCHAR SMARTMode;        // word 82, default enabled if supported.
	UCHAR MandatoryPM;		// word 82, Should always be enabled?

	UCHAR MandFlushCache;	// word 83,  Should always be enabled
	UCHAR FlushCacheExt;	// word 83,  Should always be enabled if lba48
	UCHAR Lba48;			// word 83, default enabled if supported.
	UCHAR AcousticMgt;      // word 83 + SET FEATURES.
	UCHAR SetMaxSecurity;	// word 83, enabled if HostProtectedArea on
	// if True, must call SET FEATURES to spinup drive after power up.
	UCHAR SetFeaturesSpinup;// word 83,	
	UCHAR AdvPowerMgt;		// word 83 + SET FEATURES
	UCHAR CFASupported;		// word 83 + SET FEATURES
	UCHAR DownLoadMicro;	// word 83
    UCHAR SmartSelfTest;    // word 84
    UCHAR SmartErrorLog;    // word 84
    UCHAR MediaSerialNo;    // word 84
    UCHAR DmaMode;          // Word 88
} SATA_FEATURES_T, *PSATA_FEATURES_T;
#pragma pack()

/******************************************************************************/
/*                                                                            */
/*    V A R I A B L E   A N D   S T R U C T U R E   D E F I N I T I O N S     */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                 F U N C T I O N    P R O T O T Y P E S                     */
/*                                                                            */
/******************************************************************************/

#endif /* SATA_HEADER */
