/***********************************************************************************
* artisea_diag.c - Diagnostic routines for the Intel Artisea Serial ATA Controller
* Copyright:    (C) 2003-2004 Intel Corporation.
* modification history
* --------------------
* 08/30/03  ejb  Created for the EP80219 Intel Artisea Evaluation Board
* 
*/


#include <redboot.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#include <cyg/hal/hal_verde.h>     // Hardware definitions
#include <cyg/hal/ep80219.h>       // Platform specifics
#include <cyg/hal/hal_intr.h>      // Platform specifics
#include "sata_types.h"
#include "sata_adapter.h"
#include "sata_controller.h"
#include "sata_globals.h"


#define TEST_DATA_SIZE	   SATA_SECTOR_SIZE * 10	/* size of test buffer */
#define TEST_ITERATIONS		1000					/* number of read / write iterations for testing */	
#define TEST_START_SECTOR	10						/* SATA start sector for read / write test */
#define ATU_PCI_OFFSET		0xA0000000				/* add this from local address to get PCI address */
#define DMA_TIMEOUT_MS		5000					/* 5 second dma timeout */


/* function declarations */
extern long decIn(void);
extern long hexIn(void);
extern void hex32out(unsigned long num);
extern void diag_wait(void);
extern void hal_delay_us(cyg_int32 delay);
extern void hal_interrupt_unmask(int vector);
static void print_local_data_struct(void);
static int do_read_write(int adapter_num, int drive_num, int sector);
static STATUS_CODE_T idesata_non_data_isr(int drive);
static void idesata_dma_handler(int adapterNum);
static int idesata_identify(int devTag);
static STATUS_CODE_T DeviceTest(INT devTag_i);
static unsigned long get_drive_size(INT devTag_i);
static void artesia_Isr(int arg);
void delay_ms(int ms);
void artisea_test(void);
void BuzzerTest(void);



/* Artisea library adapterInfo structures */
ADAPTER_INFO_T adapters_g[MAX_ADAPTERS];
ADAPTER_INFO_T *adapters_gp[MAX_ADAPTERS];

/* Artisea library controllerInfo structures */
CONTROLLER_INFO_T controllers_g[MAX_CONTROLLERS];
CONTROLLER_INFO_T *controllers_gp[MAX_CONTROLLERS];

/* Artisea library portInfo structures */
PORT_INFO_T ports_g[MAX_PORTS];
PORT_INFO_T *ports_gp[MAX_PORTS];

/* Artisea library devInfo structures */
DEV_INFO_T devices_g[MAX_DEVICES];
DEV_INFO_T *devices_gp[MAX_DEVICES];

/* Artisea initialization flag */
int alreadyInitialized = FALSE;

/* DMA Descriptor Table */
DDT_ENTRY_T dmaDescTable[DDT_MAX_ENTRIES];

/* Global debug counter */
INT sataDebugCounter;

/* Global default features list */
SATA_FEATURES_T devFeatures_g;

/* device indentify information */
unsigned short dev_identify[SATA_SECTOR_SIZE / 2];

/* statically allocated test buffer */
UINT8 data_buffer_addr[TEST_DATA_SIZE];


/* artisea_test - Main diagnostic routine for the 31244 Artisea Serial ATA controller */
void artisea_test()
{
	int i, num_artisea;
	cyg_uint32 data32;
    int is_artisea_in_dpa = 0;
	STATUS_CODE_T retval;
	int testDevNum;
    cyg_pci_device_id  devid[MAX_CONTROLLERS];
    cyg_pci_device	dev_info[MAX_CONTROLLERS];
	cyg_uint32 oldmask, newmask;


	diag_printf ("\n\nIntel 31244 (Artisea) PCI-X to Serial ATA Controller Diagnostics\n");
	diag_printf ("----------------------------------------------------------------\n\n");

	diag_printf ("    *************************************************\n");
	diag_printf ("    WARNING: ANY DATA ON CONNECTED DRIVES MAY BE LOST\n");
	diag_printf ("    *************************************************\n");


	// Initialize data structures
	if (InitLib() != SATA_SUCCESS)
	{
		// this should never happen...
		diag_printf ("ERROR: Can not initialize SATA data stuctures\n\n");
		diag_printf ("\nHit <CR> to Continue...\n");
		(void)hexIn();
		return;
	}

	// Find all SATA artisea devices on PCI bus 
	for (i = 0, num_artisea = 0; i < MAX_CONTROLLERS; i++, num_artisea++)
	{
		if (i == 0)
			devid[0] = CYG_PCI_NULL_DEVID;
		else
			devid[i] = devid[i-1]; // start from last one found   

		if (!cyg_pci_find_device(0x8086, 0x3200, &devid[i]))
			break;

	}

	if (num_artisea == 0)
	{
		diag_printf ("ERROR: No supported Artisea Serial ATA devices found\n");
		diag_printf ("\nHit <CR> to Continue...\n");
		(void)hexIn();
		return;
	}
	else
	{
		diag_printf ("\n    Artisea Serial ATA Controllers Found: \n");
	    diag_printf (" Unit#  Bus#  Device#  Function#\n");
	    diag_printf (" -----  ----  -------  ---------\n");
	    for (i = 0; i < num_artisea; i++)
		diag_printf ("   %d     %d       %d          %d\n", i,
		     CYG_PCI_DEV_GET_BUS(devid[i]),
		     CYG_PCI_DEV_GET_DEV(CYG_PCI_DEV_GET_DEVFN(devid[i])),
		     CYG_PCI_DEV_GET_FN(CYG_PCI_DEV_GET_DEVFN(devid[i]))); 
	}


	// unmask SATA interrupts
	INTCTL_READ(oldmask);
	newmask = oldmask | (1 << CYGNUM_HAL_INTERRUPT_SATA);
	INTCTL_WRITE(newmask);


	/* Initialize controllers and ports */
	for (i = 0; i < num_artisea; i++)
	{

		diag_printf ("\nInitializing Adapter %d...", i);

		// check the mode of the artisea - our test only works in DPA mode
		cyg_pci_read_config_uint32(devid[i], 0x08, &data32);

		// are the top bits of the class code 0106h ? 
		is_artisea_in_dpa = ( (data32 & 0xffff0000) == 0x01060000) ? 1 : 0;

		if (!is_artisea_in_dpa)
		{
			diag_printf ("\nERROR: artisea controller %d is NOT in DPA Mode!\n",i);
			continue;
		}

		// get the PCI base address of this controller 
		cyg_pci_get_device_info(devid[i], &(dev_info[i]));
		data32 = dev_info[i].base_map[0];
		data32 &= 0xfffffff0;
		
		// Initialize this controller and all ports associated with it
		retval = AdapterInit(i, (SATA_REG_ADDR_T)data32);       
		if ( retval == SATA_COMPLETED_WITH_ERRORS )
        {
            /* Some of the drives failed during init */
            diag_printf("\nERROR: AdapterInit returned SATA_COMPLETED_WITH_ERRORS (%d)\n", retval);

        }
        else if ( retval != SATA_SUCCESS )
        {
            /* Failed to initialize the library */
            diag_printf("ERROR: AdapterInit Failed (%d). Disabling adapter %d\n", retval, i);
        }
 

		/* Enable the interrpts on the adapter */
        if (AdapterEnableInterrupts(i) != SATA_SUCCESS)
			diag_printf ("\nERROR: Can not enable artisea %d Interrupts\n", i);

		/* connect the interrupt handler */
//		HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SATA, &artesia_Isr, CYGNUM_HAL_INTERRUPT_SATA, 0);
//	    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SATA);
//		hal_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SATA);

	}

	/* print out which ports are connected to drives */
	diag_printf("\n\nDrive Status:\n");
	diag_printf("-------------\n");

	for (i = 0; i < MAX_PORTS; i++)
	{
		if (ports_gp[i]->connected)
			diag_printf ("Device %02d:  artisea %d, Port %d:  CONNECTED!\n", i, ports_gp[i]->controller, ports_gp[i]->portNumber);
		else
			diag_printf ("Device %02d:  artisea %d, Port %d:  NOT CONNECTED!\n", i, ports_gp[i]->controller, ports_gp[i]->portNumber);
	}

#if 0
	diag_printf ("\nTest all connected ports? (y/n): ");
    sgets(answer);
    diag_printf("\n");

	// manufacturing test - all connected ports
    if ((answer[0] == 'y') || (answer[0] == 'Y'))
   	{
		for (testDevNum = 0; testDevNum < MAX_PORTS; testDevNum++)
		{
			if (ports_gp[testDevNum]->connected)
			{
				diag_printf ("\nTesting artisea %d, Port %d...\n", ports_gp[testDevNum]->controller, ports_gp[testDevNum]->portNumber);
				
				if (DeviceTest(testDevNum) == SATA_SUCCESS)
				{	
					ports_gp[testDevNum]->testResults = TEST_PASSED;
					diag_printf ("\n\nDevice %d: TEST PASSED\n\n", testDevNum);
				}

				// debug
				// print_local_data_struct();

			}
		}

		for (i = 0; i < MAX_PORTS; i++)
		{
			// get the status register info
			diag_printf ("Device %02d:  artisea %d, Port %d:  ", i, ports_gp[i]->controller, ports_gp[i]->portNumber);

			switch (ports_gp[i]->testResults)
			{
			case TEST_PASSED:
				diag_printf ("TEST PASSED!\n");
				break;
			case NOT_CONNECTED:
				diag_printf ("PORT NOT TESTED (NO DRIVE CONNECTED)\n");
				break;
			case DMA_TIMEOUT:
				diag_printf ("*** TEST FAILED (DMA TIMEOUT ERROR) ***\n");
				break;
			case DATA_ERROR:
				diag_printf ("*** TEST FAILED (DATA ERROR) ***\n");
				break;
			case DRIVER_ERROR:
				diag_printf ("*** TEST FAILED (DRIVER SETUP ERROR) ***\n");
				break;
			default:
				diag_printf ("*** TEST FAILED (UNKNOWN ERROR) ***\n");
				break;
			}
		}	
	}

#endif

	// manual test - individual ports
//	else
//	{
		testDevNum = 0;  // init value
		while (testDevNum < MAX_PORTS)
		{
			diag_printf ("Enter Device Number (99 to quit): ");
			testDevNum = decIn();
			if (testDevNum < MAX_PORTS)
			{
				if (ports_gp[testDevNum]->connected)
				{
					diag_printf ("\nPress ENTER to test artisea %d, Port %d...\n", ports_gp[testDevNum]->controller, ports_gp[testDevNum]->portNumber);
					hexIn();
				
					if (DeviceTest(testDevNum) == SATA_SUCCESS)
						diag_printf ("\n\nDevice %d: TEST PASSED\n\n", testDevNum);
				}
				else
					diag_printf ("\n\nDevice %d NOT Connected!\n", testDevNum);
			}

		}
//	}

	// mask SATA interrupts and return
	INTCTL_WRITE(oldmask);
	diag_wait();
}



/* DeviceTest - Routine for testing a particular test once all devices have been initialized */
static STATUS_CODE_T DeviceTest(INT devTag_i)
{
    DEV_INFO_T     * devp      = NULL_POINTER;
    INT portTag                = INVALID_TAG;
	INT adapterTag;
	unsigned long drive_size;
	int i;
	

	devp = devices_gp[devTag_i];
    portTag = devp->port;

	adapterTag = ports_gp[portTag]->controller; 

	/* lets read the identity data from this drive, if possible */
	idesata_identify(devTag_i);
	drive_size = get_drive_size(devTag_i);


	/* print out the SATA drive make and model */
	diag_printf("\nSerial ATA Drive %d Identification:\n", devTag_i);
	diag_printf("-----------------------------------\n");
	diag_printf("  Model Number      : %s\n", devp->devIdentify.modelNumber); 
	diag_printf("  Firmware Revision : %s\n", devp->devIdentify.firmwareRevision);
	diag_printf("  Serial Number     : %s\n", devp->devIdentify.serialNumber);
	diag_printf("  Drive Size        : %d GBytes\n", drive_size);


	diag_printf ("\nPerforming Write / Read Test on Device %d...", devTag_i);
	for (i = TEST_START_SECTOR; i < TEST_ITERATIONS + TEST_START_SECTOR; i++)
	{
		/* perform a write read test on this drive... */
		if ((ports_gp[portTag]->testResults = do_read_write(adapterTag, devTag_i, TEST_START_SECTOR)) == TEST_PASSED)
		{
			if (!(i % 20))
				diag_printf (".");
		}
		else
		{
			diag_printf ("TEST FAILED!!! (cycle = %d)\n", i - TEST_START_SECTOR);
			break;
		}
	}

	if (i == TEST_ITERATIONS + TEST_START_SECTOR)
		diag_printf ("\nDEVICE %d:  PASSED!\n\n", devTag_i);
}



/* get_drive_size - Uses the device identify information to estimate the drive size in GBytes */
static unsigned long get_drive_size(INT devTag_i)
{
	DEV_INFO_T     * devp      = NULL_POINTER;
	unsigned long long drive_size;
	unsigned long total_sectors;
	
	devp = devices_gp[devTag_i];

	// this will estimate the drive size in GBytes
	total_sectors = (devp->devIdentify.totalNumSectors[1] << 16) + devp->devIdentify.totalNumSectors[0];
	drive_size = (total_sectors / 1000);
	drive_size *= SATA_SECTOR_SIZE;
	drive_size = drive_size / 1000000;
	
	return (drive_size);
}


/* idesata_identify - Read the SATA device identify information from the drive */
static int idesata_identify(int devTag)
{
    STATUS_CODE_T retval = SATA_SUCCESS;

    retval = DevIdentify( devTag, dev_identify);

    if (retval != SATA_SUCCESS)
    {
        diag_printf ("ERROR: Drive %d: Failed(%d) to get Device Identify information\n", devTag, retval);
		return (retval);
    }

    return SATA_SUCCESS;
}



/* delay_ms - delay function */
void delay_ms(int ms)
{
	hal_delay_us(ms * 1000);
}



/* "Poor Man's bzero" - zero's a block of memory. */
static void artisea_bzero (volatile char *ptr, UINT32 num_bytes)	
{
    volatile long i;		/* loop counter */

    /* zero out space */
    for (i = 0; i < num_bytes; *ptr++ = 0, i++)
		;
} 


/* Create a block of data to write out to target disk. */
static void make_data (UINT8 *pData, int size)
{
    unsigned i;				/* Loop variable */

    for (i = 0; i < size; i++)		/* Fill buf with characters.  Start */
		*pData++ = (i & 0xff);		/* with 0 and increment */
}


/* Verify contents of buffer with the incrementing scheme make_data uses */
static int check_data (UINT8* pData, int size)		
{
    unsigned int i, j;				/* Loop variables */
    int status = 0;

    for (i = 0; i < size; i++)		/* Verify buffer byte by byte */
    {
		if (*pData != (i & 0xff))
		{
			diag_printf("\nERROR: Byte %d expected 0x%X got 0x%X\n", i,(i&0xff),*pData);
			status++;

			break;
			/* return (ERROR);		 Verify error */

		}
		pData++;
    }
    
	if (status)
	{
		diag_printf ("\nPrinting buffer around failed address:\n");
		pData-=16;
		for (j = 0; j < 16; j++)
		{
			diag_printf (" %x", *pData);
			pData++;
		}
		diag_printf ("\nByte %d:  %x\n", i, *pData);
		for (j = 0; j < 16; j++)
		{
			pData++;
			diag_printf (" %x", *pData);
		}
		diag_printf ("\n");
		return (ERROR);

	}
    else
    	return (OK);			/* Verify OK */
}


/* do_read_write - perform one read / write test cycle on inidicated port/drive */
static int do_read_write(int adapterTag_i, int devTag_i, int sector)
{
    STATUS_CODE_T retval = SATA_SUCCESS;
	int i;
	SATA_TRANSFER_TYPE_T curr_transfer_mode;
    UINT64  start_sector;
	INT nr_sectors;
	SATA_COMMAND_TYPE_T cmd;
	UINT64	dmaTableXlate;
	cyg_uint32 stat;

    /* Fill data buffer with incrementing pattern */
	make_data(data_buffer_addr, TEST_DATA_SIZE);

	/* for testing, only one dma table entry pointing at this one buffer */
	dmaDescTable[0].baseAddress= (BUS_ADDR_T)((unsigned long)&data_buffer_addr +  ATU_PCI_OFFSET);
	dmaDescTable[0].byteCount = TEST_DATA_SIZE;
	dmaDescTable[0].EOT = 0xFFFF;

	/* for now, hardcode logical block info to first sector */
	start_sector = sector; 
	curr_transfer_mode = FIRST_PARTY_DMA;
	nr_sectors = TEST_DATA_SIZE / SATA_SECTOR_SIZE;
	cmd = SATA_WRITE;

	dmaTableXlate = (UINT64)(&dmaDescTable[0]);
	dmaTableXlate += (UINT64)ATU_PCI_OFFSET;

	/* add the write command to the queue */
    retval = AdapterAddCommandToQueue(
               adapterTag_i,			/* Tag for the adapter to be used             */
               devTag_i,				/* The devTag of the target drive.            */
               start_sector,			/* Starting sector # for the transfer (LBA28) */
               nr_sectors,				/* Number of sectors to transfer.             */
               cmd,						/* (Read / Write)							  */
               curr_transfer_mode,		/* Requested mode of transfer ( DMA / PIO )   */
               dmaTableXlate,			/* Bus address of the DMA Descriptor Table    */
               0,			            /* Top 32 bits of the DMA Data address in DDT */
               data_buffer_addr			/* Virtual Address of the source/target buffer*/
               );
    if (retval != SATA_SUCCESS )
    {
        diag_printf("\nERROR: Drive %d: Failed(%d) to Queue SATA_WRITE command.\n", devTag_i, retval);
		return (DRIVER_ERROR);
    }

    /* We could call AdapterServiceCommandQueues which will cause all pending commands for */
    /* drives attached to this HBA to be issued. */
    retval =  DevServiceCommandQueue(devTag_i);

    if (retval != SATA_SUCCESS )
    {
        diag_printf("ERROR: Drive %d: Failed(%d) to service command Queue.", devTag_i, retval);
		devices_gp[devTag_i]->waiting_for_dma = FALSE;
		return (DRIVER_ERROR);
    }

	i = 0;
	while (/*(devices_gp[devTag_i]->waiting_for_dma ) && */(i < DMA_TIMEOUT_MS))
	{
		IINTSRC_READ(stat);
		if (stat & (1 << CYGNUM_HAL_INTERRUPT_SATA))
			break;

		delay_ms(1);
		i++;
	}

	if (i == DMA_TIMEOUT_MS)
	{
		diag_printf("\nERROR: WRITE DMA Timeout waiting for interrupt!\n");
		return (DMA_TIMEOUT); 		
	}
	else
	{
		// we got an interrupt, manually call the ISR 
		artesia_Isr(0);
	}
	
	
	/*** Clear the buffer ***/
	artisea_bzero((volatile char *)data_buffer_addr, (UINT)TEST_DATA_SIZE);


	// delay for a bit, for no real purpose
	//delay_ms (500);

	/*** now do the read back and verify the data ***/

	cmd = SATA_READ;
	/* add the write command to the queue */
    retval = AdapterAddCommandToQueue(
               adapterTag_i,			/* Tag for the adapter to be used             */
               devTag_i,				/* The devTag of the target drive.            */
               start_sector,			/* Starting sector # for the transfer (LBA28) */
               nr_sectors,				/* Number of sectors to transfer.             */
               cmd,						/* (Read / Write)         */
               curr_transfer_mode,		/* Requested mode of transfer ( DMA / PIO )   */
               dmaTableXlate,           /* Bus address of the DMA Descriptor Table */
               0,                       /* Top 32 bits of the DMA Data address in DDT */
               data_buffer_addr			/* Virtual Address of the source/target buffer*/
               );
    if (retval != SATA_SUCCESS )
    {
        diag_printf("Drive %s: Failed(%d) to Queue SATA_READ command.", devTag_i, retval);
		return (DRIVER_ERROR);
    }


    /* We could call AdapterServiceCommandQueues which will cause all pending commands for */
    /* drives attached to this HBA to be issued. */
    retval =  DevServiceCommandQueue(devTag_i);

    if (retval != SATA_SUCCESS )
    {
        diag_printf("Drive %d: Failed(%d) to service command Queue.", devTag_i, retval);
    	devices_gp[devTag_i]->waiting_for_dma = FALSE;
		return (DRIVER_ERROR); // possibly do something to handle this error
	}

	i = 0;
	while (/*(devices_gp[devTag_i]->waiting_for_dma ) && */(i < DMA_TIMEOUT_MS))
	{
		IINTSRC_READ(stat);
		if (stat & (1 << CYGNUM_HAL_INTERRUPT_SATA))
			break;

		delay_ms(1);
		i++;
	}

	if (i == DMA_TIMEOUT_MS)
	{
		diag_printf("\nERROR: READ DMA Timeout waiting for interrupt!\n");
		return (DMA_TIMEOUT);   
	}
	else
	{
		// we got an interrupt, manually call the ISR 
		artesia_Isr(0);
	}

	// dma complete, check the data to ensure it was read correctly
	if (check_data(data_buffer_addr, TEST_DATA_SIZE) == ERROR)
		return (DATA_ERROR);
	else
	{
		return (TEST_PASSED);
	}
}



/* artesia_Isr - Interrupt service routine for diagnostic */
static void artesia_Isr(int arg)
{
	int i;

	for (i = 0; i < MAX_CONTROLLERS; i++)
	{
		idesata_dma_handler(i);
	}

	return;
}


/* This function will handle all Non-data interrupts.					*/
/* Note: We issue all Non-data commands with drive interrupts disabled. */
/*       Complete handling for the ND command is done in one place. We  */
/*       Don't expect ND interrupts.                                    */
/* Support for Hot-plug should be implemented here...                   */
static STATUS_CODE_T idesata_non_data_isr(int drive)
{

    diag_printf("Drive %d: Non Data ISR called. Did not expect this call.\n", drive);
   // DumpErrStat( drive_tag, DUMP_BUFFER_SZ, dump_buffer);
    //DEBUG_ERROR("%s",dump_buffer);
    return ERROR;
}


/* This function will handle all DMA related interrupts. */
/* The Lib sets the handler to this function for all DMA data transfers */
static void idesata_dma_handler(int adapterNum)
{
    STATUS_CODE_T retval    = SATA_SUCCESS;
    int drive_tag;
    int error_detected      = 0;
    UINT8 dma_stat, stat ;
    int i; /* Loop counter */

	// figure out which port / drive is interrupting */
	for (i = 0; i < MAX_DEVICES_PER_ADAPTER; i++)
	{
		drive_tag = adapters_gp[adapterNum]->drives[i];

		/* Check that there is an interrupt pending on the drive */
		if ( DevDmaInterruptPending(drive_tag) == FALSE)
		{
			/* The interrupt was not a DMA interrupt... Check if we have other interrupt pending...*/
			if ( DevInterruptPending(drive_tag) == FALSE)
			{
				continue;  // this device isn't interrupting, move on
			}
			else
			{
				/* OK, we have some other interrupt */
				idesata_non_data_isr(drive_tag);
			}
		}
		else
		{
			devices_gp[drive_tag]->waiting_for_dma = 0;	// we received the dma interrupt we were waiting for
		}

		/* this device is interrupting.... */
		/* Call the Lib's ISR. This will check for errors, Clear the interrupt & end the DMA transfer */
		retval = DevIsr(drive_tag);

		/* Mark DMA as no longer active */
		//drive_info->sg_dma_active = 0;

		/* Check the status if the DevIsr */
		switch (retval)
		{
		case SATA_SUCCESS_COMPLETED: /* ATA command completed */
			break;

		case SATA_FAILURE: /* ATA Protocol level error (StatusReg:ERR bit) */
			diag_printf("Drive %d: SATA command failed(%d). ATA protocol Error.", drive_tag, retval);
			//DumpErrStat( drive_tag, DUMP_BUFFER_SZ, dump_buffer);
			//diag_printf("%s",dump_buffer);

			/* Error handling done below... */
			error_detected = 1;
			break;

		case SATA_BUSY:    /* Device not Ready */
		default:
			diag_printf("Drive %d: DevIsr failed(%d)", drive_tag, retval);
			//DumpErrStat( drive_tag, DUMP_BUFFER_SZ, dump_buffer);
			//DEBUG_ERROR("%s",dump_buffer);

			/* Error handling done below... */
			error_detected = 1;
		}

		/* Get the DMA status */
		dma_stat = 0;
		DevGetDmaStatus(drive_tag, &dma_stat);

		/* Check the Status.
		1. Make sure that SUPDDSR_DMA_ACTIVE_BIT = 0, i.e. DMA transfer has completed.
		2. SUPDDSR_DMA_ERROR_BIT = 0, i.e. No parity errors and no aborts on the PCI bus
		*/
		if ( READ_BIT(dma_stat, SUPDDSR_DMA_ACTIVE_BIT) == 1 ||
			 READ_BIT(dma_stat, SUPDDSR_DMA_ERROR_BIT)  == 1
		   )
		{
			/* DMA error */
			diag_printf("Drive %d: DMA error (0x%0X)", drive_tag, dma_stat );
			error_detected = 1;
		}
	

		if ( (DevGetStatus(drive_tag, &stat) != SATA_SUCCESS) ||
			 (READ_BIT(stat, SR_ERR_BIT) == 1)                || /* Error bit is set */
			 (READ_BIT(stat, SR_DRQ_BIT) == 1)                   /* DRQ is set */
		   )
		{
			diag_printf("Drive %d: Bad Status (0x%0X)", drive_tag, stat );
			error_detected = 1;
		}

		if (error_detected != 0)
		{
			/* Do error handling here... */
			//ide_action = ide_error(drive, "dma_intr", stat);

			/* NPG: TBD: Clear the errors if there were any... */
			DevClearError(drive_tag);

			return ;
		}
	} 
}


/* InitLib - allocates and initializes the memory structures internal to the library. 
This function should be called before any other function in the library is used. */
static STATUS_CODE_T InitLib(void)
{
    INT ctr;
    static boolean alreadyInitialized = FALSE;

    /* initialize the debug counter value to zero */
    sataDebugCounter = 0;

    if(alreadyInitialized == TRUE)
    {
        return SATA_SUCCESS;
    }

    /* set all the elements in the global pointer arrays to actual structures statically declared
	   in arrays above.  This is done so that these structures can be statically
	   declared in the diagnostic and still keep compatibility with the Intel driver code */

    for(ctr = 0; ctr < MAX_ADAPTERS; ctr++)
	{
       adapters_gp[ctr] = &adapters_g[ctr];
	   adapters_gp[ctr]->present = FALSE;  // Mark as not yet completely initialized 
    }

    for(ctr = 0; ctr < MAX_CONTROLLERS; ctr++)
    {
        controllers_gp[ctr] = &controllers_g[ctr];
		controllers_gp[ctr]->present = FALSE;  // Mark as not yet completely initialized 
		controllers_gp[ctr]->adapter = 0;
    }

    for(ctr = 0; ctr < MAX_PORTS; ctr++)
    {
        ports_gp[ctr] = &ports_g[ctr];
		ports_gp[ctr]->present = FALSE;		// Mark as not yet completely initialized 
		ports_gp[ctr]->testResults = NOT_CONNECTED;
    }

    for(ctr = 0; ctr < MAX_DEVICES; ctr++)
    {
        devices_gp[ctr] = &devices_g[ctr];
		devices_gp[ctr]->present = FALSE;		// Mark as not yet completely initialized 
    }

    alreadyInitialized = TRUE;

    return SATA_SUCCESS;
}


/* debug print */
static void print_local_data_struct(void)
{
int ctr, i;

	for (ctr = 0; ctr < MAX_ADAPTERS; ctr++)
	{
		
		diag_printf ("\nadapters_gp %d:\n", ctr);
		diag_printf ("------------------\n");
		diag_printf ("present = %d\n", adapters_gp[ctr]->present);
		for (i = 0; i < MAX_CONTROLLERS_PER_ADAPTER; i++)
			diag_printf ("controllers[%d] = %d\n", i, adapters_gp[ctr]->controller[i] );
		for (i = 0; i < MAX_DEVICES_PER_ADAPTER; i++)
			diag_printf ("drives[%d] = %d\n", i, adapters_gp[ctr]->drives[i] );
		diag_printf ("driveLastServiced = %d\n", adapters_gp[ctr]->driveLastServiced);

	}

    for(ctr = 0; ctr < MAX_CONTROLLERS; ctr++)
    {

		diag_printf ("\ncontrollers_gp %d:\n", ctr);
		diag_printf ("------------------\n");
		diag_printf ("present = %d\n", controllers_gp[ctr]->present);
		for (i = 0; i < MAX_PORTS_PER_CONTROLLER; i++)
			diag_printf ("port[%d] = %d\n", i, controllers_gp[ctr]->port[i] );
		diag_printf ("adapter = %d\n", controllers_gp[ctr]->adapter);
		diag_printf ("mode = %d\n", (int)controllers_gp[ctr]->mode);
		diag_printf ("commonRegBase = 0x%X\n", (UINT32)controllers_gp[ctr]->commonRegBase);
		diag_printf ("\n\n");

    }


    for(ctr = 0; ctr < MAX_PORTS; ctr++)
    {
		diag_printf ("\nports_gp %d:\n", ctr);
		diag_printf ("------------\n");
		diag_printf ("present = %d\n", ports_gp[ctr]->present);
		for (i = 0; i < MAX_DEVICES_PER_PORT; i++)
			diag_printf ("dev[%d] = %d\n", i, ports_gp[ctr]->dev[i]);
		diag_printf ("controller = %d\n", ports_gp[ctr]->controller);		
		diag_printf ("portNumber = %d\n", ports_gp[ctr]->portNumber);		
		diag_printf ("portRegBase = 0x%X\n", (UINT32)ports_gp[ctr]->portRegBase);
		diag_printf ("connected = %d\n", ports_gp[ctr]->connected);
		diag_printf ("cumulativeSSER = 0x%X\n", ports_gp[ctr]->cumulativeSSER);
		diag_printf ("lastSSER = 0x%X\n", ports_gp[ctr]->lastSSER);
		diag_printf ("lastER = 0x%X\n", ports_gp[ctr]->lastER);
		diag_printf ("lastASR = 0x%X\n", ports_gp[ctr]->lastASR);
		diag_printf ("lastSR = 0x%X\n", ports_gp[ctr]->lastSR);
		diag_printf ("lastSSSR = 0x%X\n", ports_gp[ctr]->lastSSSR);
		diag_printf ("testResults = %d\n", ports_gp[ctr]->testResults);
    }

    for(ctr = 0; ctr < MAX_DEVICES; ctr++)
    {

		diag_printf ("\ndevices_gp %d:\n", ctr);
		diag_printf ("--------------\n");
		diag_printf ("present = %d\n", devices_gp[ctr]->present);
		diag_printf ("active = %d\n", devices_gp[ctr]->active);
		diag_printf ("port = %d\n", devices_gp[ctr]->port);
		diag_printf ("ioMode = %d\n", (int)devices_gp[ctr]->ioMode);
		diag_printf ("osDriveTag = 0x%X\n", (UINT32)devices_gp[ctr]->osDriveTag);
		diag_printf ("dmaHandler = 0x%X\n", (UINT32)devices_gp[ctr]->dmaHandler);
		diag_printf ("pioReadHandler = 0x%X\n", (UINT32)devices_gp[ctr]->pioReadHandler);
		diag_printf ("pioWriteHandler = 0x%X\n", (UINT32)devices_gp[ctr]->pioWriteHandler);
		diag_printf ("isDrive = %d\n", devices_gp[ctr]->isDrive);
		diag_printf ("lbaCapacity = %d\n", devices_gp[ctr]->lbaCapacity);
		diag_printf ("waiting_for_dma = %d\n", devices_gp[ctr]->waiting_for_dma);
		
    }
}



#define GPOE_ADDR		0xFFFFE7C4
#define GPID_ADDR		0xFFFFE7C8
#define GPOD_ADDR		0xFFFFE7CC

/* EP80219 Buzzer Test */
void BuzzerTest(void)
{
	/* make GPIO #2 data line a 1 so that the buzzer is off when
	   we make the signal an output */
	*(volatile cyg_uint8 *)GPOD_ADDR |= (1 << 2);

	/* make GPIO #2 an output */
	*(volatile cyg_uint8 *)GPOE_ADDR &= ~(1 << 2);

	diag_printf ("Enabling Buzzer, should sound for 2 seconds...\n\n");

	/* turn on buzzer for 2 seconds - active low output */
	*(volatile cyg_uint8 *)GPOD_ADDR &= ~(1 << 2);

	delay_ms(2000);

	/* turn off buzzer */
	*(volatile cyg_uint8 *)GPOD_ADDR |= (1 << 2);

	diag_printf ("Done!\n\n");

	return;
}


