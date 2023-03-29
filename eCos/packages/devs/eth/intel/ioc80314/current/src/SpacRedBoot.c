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
* FILENAME: 1.	SpacRedBoot.c
*
* DESCRIPTION: This file contains the Spac synchronization routines for RedBoot.
*
* HISTORY:
*
*    Rev. Date       Author  Description
*    ---- ---------  ------  ------------------------------------------
*    0.01 Aug 08,02  on     draft 
*    0.01 Sept 08,02  on     modified synchronization function signatures.
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

/* SPac chip base address. */
extern  UINT32 SpacBaseAddr;

/*******************************************************************************
* SpacInterruptDisable
* DESCRIPTION:
* Call this routine in order to disable interrupts.
*
*
* RETURNS:
*    int - the interrupt key.
*
******************************************************************************/
int SpacInterruptDisable(void)
{
  return 0;
}

/*******************************************************************************
* SpacInterruptEnable
* DESCRIPTION:
* Call this routine in order to enable interrupts.
*
*
* RETURNS:
*    N/A
*
******************************************************************************/
void SpacInterruptEnable(int interruptKey)
{}

/*******************************************************************************
* SpacSchedulerDisable
* DESCRIPTION:
* Call this routine in order to disable task/thread preemption.
*
*
* RETURNS:
*    int - the preemption key.
*
******************************************************************************/
int SpacSchedulerDisable(void)
{
  return 0;
}

/*******************************************************************************
* SpacSchedulerEnable
* DESCRIPTION:
* Call this routine in order to enable task/thread preemption.
*
*
* RETURNS:
*    N/A
*
******************************************************************************/
void SpacSchedulerEnable(int preemptionKey)
{}

/*******************************************************************************
* SpacSemCreate
* DESCRIPTION:
* Call this routine in order to create semaphores.
*
*
* RETURNS:
*    ETH_OK - operation suceded.
*
******************************************************************************/
int SpacSemCreate(SpacSemType semType, int count, int * semIdPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  return retCode;

}

/*******************************************************************************
* SpacSemDelete
* DESCRIPTION:
* Call this routine in order to delete previously created semaphores.
*
*
* RETURNS:
*    ETH_OK - operation suceded.
*
******************************************************************************/
int SpacSemDelete(int * semIdPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  return retCode;

}

/*******************************************************************************
* SpacSemWait
* DESCRIPTION:
* Call this routine in order to take a semaphore.
*
*
* RETURNS:
*    ETH_OK - operation suceded.
*
******************************************************************************/
int SpacSemWait(int * semIdPtr, int timeout)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  return retCode;

}

/*******************************************************************************
* SpacSemTryWait
* DESCRIPTION:
* Call this routine in order to take a semaphore, if available.
*
*
* RETURNS:
*    ETH_OK - operation suceded..
*
******************************************************************************/
int SpacSemTryWait(int * semIdPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  return retCode;

}

/*******************************************************************************
* SpacSemSignal
* DESCRIPTION:
* Call this routine in order to signal a semaphore.
*
*
* RETURNS:
*    ETH_OK - operation suceded.
*
*
******************************************************************************/
int SpacSemSignal(int * semIdPtr)
{

  SpacEthRtnCode_t retCode = ETH_OK;

  return retCode;

}

/*******************************************************************************
* SpacNanosleep
* DESCRIPTION:
* Call this routine in order to relinquish the CPU for the parameter-specified
* number of nanoseconds.
*
*
* RETURNS:
*    N/A
*
* Note:
*     This guaranties to sleep for at least but not exact the parameter's value
*     time. To avoid unsigned overflowing, we split the integer division, thus
*     forcing the parameter value to at least 100. 
******************************************************************************/
void SpacNanosleep(unsigned int nanoseconds)
{
  unsigned int counter = 0;
  unsigned int threshold = (ETH_XSCALE_CLK / 10000000) * (nanoseconds / 100);
  
  do{
    counter++;
  }while(counter < threshold);

}

/*=========================================================================
 *  Function    : EthSynchCache
 *
 *  Arguments   : address - input, address of DMA buffer.
 *              : direction - input, TX/RX direction flag.
 *              : len - input, length of DMA buffer. 
 * 
 *  Description : Synchronize precessor's cache with the DMA buffers.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : Before reading from DMA memory we call this routine with 
 *              : direction = ETH_RX. After writing to DMA memory we call 
 *              : this routine with direction = ETH_TX.
 =========================================================================*/
int EthSynchCache(void * address, unsigned len, 
  unsigned direction){

  SpacEthRtnCode_t retCode = ETH_OK;
  unsigned lineAddr, endAddr, status;

	status = _Read_ARM_Control;
	if (!(status & 0x4)) 
		return retCode;
  
  if(address == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSynchCache>, \
             invalid address parameter = %p.", address);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(len == 0x00000000){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSynchCache>, \
             invalid length parameter = %d.", len);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  lineAddr = 
    (unsigned)address - (((unsigned)address) % ETH_XSCALE_CACHE_LINE_LEN);
  endAddr = (unsigned)address + len;

  if(direction == ETH_TX){
/* Clean/flush cache. */    
    do{
      _Clean_D_Cache_Line(lineAddr);
      lineAddr += ETH_XSCALE_CACHE_LINE_LEN;
    }while(lineAddr < endAddr);
    _Drain_Write_Buffer;
    _WaitForNoBCU();
  }else if(direction == ETH_RX){
    do{
      _Clean_D_Cache_Line(lineAddr);
      _Invalidate_D_Cache_Line(lineAddr);
      lineAddr += ETH_XSCALE_CACHE_LINE_LEN;
    }while(lineAddr < endAddr);
    _WaitForNoBCU();
  }else{
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthSynchCache>, \
             invalid direction parameter = %d.", direction);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  }

  return retCode;
  
}
  
/*=========================================================================
 *  Function    : EthVirt2Bus
 *
 *  Arguments   : virtAddr - input, virtual address.
 *              : busAddrPtr - output, buss address pointer.
 * 
 *  Description : Translate a virtual address into a buss address.
 * 
 *  return      : ETH_PARAM_ERR - Invalid parameter.
 *              : ETH_OK - Operation succeded.
 *
 *  Note        : 
 =========================================================================*/

int EthVirt2Bus(unsigned virtAddr, unsigned * busAddrPtr){

  SpacEthRtnCode_t retCode = ETH_OK;
  
  if(virtAddr == 0x00000000){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthVirt2Bus>, \
             invalid virtual address parameter = %08X.", virtAddr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  if(busAddrPtr == NULL){
    #ifdef DEBUG_OVI
      db_printf("\nIn <EthVirt2Bus>, \
             invalid buss address pointer parameter = %p.", busAddrPtr);
    #endif // DEBUG_OVI
    return ETH_PARAM_ERR;
  } 

  *busAddrPtr = virtAddr;

  return retCode;
  
}
  
/*=========================================================================
 *  Function    : EthHaltSystem
 *  Arguments   : N/A.
 * 
 *  Description : Disables RX. Halts the DMA test processing.  
 *              : Disables xScale interrupts.
 * 
 *  return      : N/A. 
 *
 *  Note        : This is intended for debugging purposes.
 =========================================================================*/

void EthHaltSystem(void){
  
  UINT32 cpsr;
  
  *(REG32_PTR(SpacBaseAddr + ETH_MAC_CFG1_REG + (ETH_PORT0_ID * ETH_PORT_OFFSET))) &= ~0x00000004;
  *(REG32_PTR(SpacBaseAddr + ETH_MAC_CFG1_REG + (ETH_PORT1_ID * ETH_PORT_OFFSET))) &= ~0x00000004;
      
  *((UINT32*)(0x50107400 + 0x2B0)) = 0xAA55AA55;
  
  cpsr = _Read_CPSR;
  _Write_CPSR(cpsr | _IRQ_DISABLE | _FIQ_DISABLE);
   
//  db_printf("\nIn <EthHaltSystem>, halting the system...\n"); 
//    
  return;
  
}
  


