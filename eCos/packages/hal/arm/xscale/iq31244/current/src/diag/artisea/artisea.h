/************************************************************************;
;*                                                                      
;*      Filename: artisea.h  								 
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

#ifndef ARTISEA_HEADER
#define ARTISEA_HEADER


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

#define ARTISEA_PORT_COUNT            (4)
#define ARTISEA_COMMON_PORT_REG_SIZE  (0x200)
#define ARTISEA_SATA_PORT_REG_SIZE    (0x200)

/******************************************************************************/
/*                                                                            */
/*                 ARTISEA DPA Mode register Offsets                          */
/*                                                                            */
/******************************************************************************/

/***************************************************/
/*                                                 */
/* Artisea DPA Mode Configuration space registers  */
/* (Offsets into the PCI Config Space)             */
/*                                                 */
/***************************************************/
#define SUDBAR0   (0x10) /* SU PCI DPA Base Addr Reg 0                 */
#define SUPDUBAR0 (0x14) /* SU PCI DPA Upper Base Addr Reg 0           */

/*********************************************/
/*                                           */
/* Artisea DPA Mode Common Registers         */
/* (Offsets from SU PCI DPA Base Addr Reg 0) */
/*                                           */
/*********************************************/
#define SUPDIPR   (0x000) /* Interrupt pending Register                 */
#define SUPDIMR   (0x004) /* Interrupt Mask Register                    */


/******************************************************************************/
/*                                                                            */
/* Artisea Port Specific Registers                                            */
/* Offsets from the base of SATA Port Registers                               */
/* Compute as SUDBAR0 + ARTISEA_COMMON_PORT_REG_SIZE +                        */
/*            Port_Number * ARTISEA_SATA_PORT_REG_SIZE                        */
/*                                                                            */
/* SCRs Registers   0x000h - 0x03fh                                           */
/* BIST Registers   0x040h - 0x05fh                                           */
/* DMA Setup Reg    0x060h - 0x09bh                                           */
/* Reserved Reg     0x09ch - 0x0cbh                                           */
/* Test Registers   0x0cch - 0x0d3h                                           */
/* Reserved Red     0x0d4h - 0x1ffh                                           */
/*                                                                            */
/******************************************************************************/


/*********************************************/
/*                                           */
/* Artisea DPA Mode Command Block Registers  */
/*                                           */
/*********************************************/

/* Artisea Taskfile Register                                                */
#define SUPDDR     (0x000) /* Data Port Register                         */
#define SUPDER     (0x004) /* Error Register                             */
#define SUPDFR     (0x006) /* Features Register                          */
#define SUPDSCR    (0x008) /* Sector Count Register                      */
#define SUPDSNR    (0x00c) /* Sector Number Register                     */
#define SUPDCLR    (0x010) /* Cylinder Low Register                      */
#define SUPDCHR    (0x014) /* Cylinder High Register                     */
#define SUPDDHR    (0x018) /* Device / Head Register                     */
#define SUPDSR     (0x01c) /* Status Register                            */
#define SUPDCR     (0x01d) /* Command Register                           */

/*********************************************/
/*                                           */
/* Artisea DPA Mode Control Block Registers  */
/*                                           */
/*********************************************/
#define SUPDASR    (0x028) /* Alternate Status Register                   */
#define SUPDDCTLR  (0x029) /* Device Control Register                     */

/*********************************************/
/*                                           */
/* Artisea DPA Mode DMA Registers            */
/*                                           */
/*********************************************/
#define SUPDUDDTPR (0x064) /* Upper DMA Descriptor Table Pointer           */
#define SUPDDRBPR  (0x068) /* DMA Read Back Pointer Register               */
#define SUPDUDDBPR (0x06c) /* Upper DMA Data Buffer Pointer Reg            */
#define SUPDDCMDR  (0x070) /* DMA Command Register                         */
#define SUPDDSR    (0x072) /* DMA Status Register                          */
#define SUPDDDTPR  (0x074) /* DMA Descriptor Table Pointer Reg             */

/********************************************/
/*                                          */
/* Artisea DPA Mode Superset Registers      */
/*                                          */
/********************************************/

#define SUPDSSSR   (0x100) /* SATA SStatus Register                        */
#define SUPDSSER   (0x104) /* SATA SError Register                         */
#define SUPDSSCR   (0x108) /* SControl Register                            */
#define SUPDSDBR   (0x10c) /* Set Device Bits Register                     */

#define SUPDPFR    (0x140) /* PHY Features Register                        */
#define SUPDBFCSR  (0x144) /* BIST FIS Ctrl & Status Reg                   */
#define SUPDBER    (0x148) /* BIST Errors Register                         */
#define SUPDBFR    (0x14c) /* BIST Frames Register                         */
#define SUPDHBDLR  (0x150) /* Host BIST Data Low Register                  */
#define SUPDHBDHR  (0x154) /* Host BIST Data High Register                 */
#define SUPDDDBDLR (0x158) /* Device BIST Data Low Register                */
#define SUPDDBDHR  (0x15c) /* Device BIST Data High Register               */

#define SUPDDSFCSR (0x168) /* DMA Setup FIS Ctrl & Status                  */
#define SUPDHDBILR (0x16c) /* Host DMA Buffer Identifier Low Reg           */
#define SUPDHDBIHR (0x170) /* Host DMA Buffer Identifier High Reg          */
#define SUPDHRDR0  (0x174) /* Host Reserved DWORD Reg 0                    */
#define SUPDHDBOR  (0x178) /* Host DMA Buffer Offset Register              */
#define SUPDHDTCR  (0x17c) /* Host DMA Transfer Count Register             */
#define SUPDHRDR1  (0x180) /* Host Reserved DWORD Register 1               */
#define SUPDDDBILR (0x184) /* Device DMA Buffer Idenfifier Low Register    */
#define SUPDDDBIHR (0x188) /* Device DMA Buffer Idenfifier High Register   */
#define SUPDDRDR0  (0x18c) /* Device Reserved DWORD Register 0             */
#define SUPDDDBOR  (0x190) /* Device DMA Buffer Offset Register            */
#define SUPDDTCR   (0x194) /* Device DMA Transfer Count Register           */
#define SUPDDRDR1  (0x198) /* Device Reserved DWORD Register 1             */

#define SUPDTR0    (0x1cc) /* Test Register 0                              */
#define SUPDTR1    (0x1d0) /* Test Register 1                              */



/******************************************************************************/
/*                                                                            */
/* Mask Definations for varoius registers                                     */
/*                                                                            */
/******************************************************************************/

#define SUPDIPR_BITS_PER_PORT (8) /* Number of bits per port */
#define SUPDIPR_BITS_PER_PORT_MASK (0xff) /* 8 bits per port. shift + mask */
#define SUPDIPR_IDE_INTR_MASK (0x80808080) /* Mask to isolate IDE intr bit */
#define SUPDIPR_CRC_INTR_MASK (0x40404040) /* Mask to isolate IDE intr bit */
/* TBD: Masks for other 6 intr bits in SUPDIPR */


#define SUPDSSSR_IPM_STATE_MASK (0x00000f00) /*1=Active,2=Partial,3=Slumber*/

/* 0 = no device, 1 = Present but NO communication, 3 = Ready, 4 = offline */
#define SUPDSSSR_DEV_DET_MASK (0x0000000f)


/******************************************************************************/
/*                                                                            */
/* Bit position Definations for varoius registers                             */
/* Use READ_BIT RESET_BIT SET_BIT Macros to access them.                      */
/* RO = Read Only, RC = Read Clear, RS = Read Set, RW = Read Write            */
/*                                                                            */
/******************************************************************************/

/* SATA Port IDE Interrupt - When set, this bit indicates that the SATA       */
/* device generated an interrupt.  This is the same as PCI IDE compatible     */
/* interrupt.  The source of this interrupt is based on the setting of the    */
/* "I" bit in the Device-to-Host Register FIS.  This interrupt is cleared by  */
/* reading the taskfile Status register.                                      */
#define SUPDIPR_IDE_INTR_BIT     (7) /* RO: SATA Port IDE Interrupt        */

/* SATA Port CRC Error Detect Interrupt - When set, this bit indicates that   */
/* a CRC error was detected on a previous data transfer.  The source of this  */
/* interrupt is from bit 21 (DIAG_C) of the SError register.  This interrupt  */
/* is cleared by writing a 1 to bit 21 of the SError register.                */
#define SUPDIPR_CRC_ERR_BIT      (6) /* RO: Port CRC Error Detect Interrupt*/

/* SATA Port Data Integrity Interrupt - When set, this bit indicates that a   */
/* CRC, disparity error was detected by the host, or an R_ERR primitive was   */
/* returned by the device in response to a Data FIS transfer.  The source of  */
/* this interrupt is from bit 8 (ERR_T) of the SError register.  This         */
/* interrupt is cleared by writing a 1 to bit 8 of the SError register.       */
#define SUPDIPR_DI_INTR_BIT      (5) /* RO: Port Data Integrity Interrupt  */

/* SATA Port Unrecognized FIS Reception Interrupt - When set, this bit        */
/* indicates that an unsupported FIS was detected.  The source of this        */
/* interrupt is from bit 10 (ERR_P) of the SError register.  This interrupt   */
/* is cleared by writing a 1 to bit 10 of the SError register.                */
#define SUPDIPR_UFIS_INTR_BIT    (4) /* RO: Unrecognized FIS Interrupt     */

/* SATA Port R_ERR Received Interrupt - When set, this bit indicates that an  */
/* R_ERR primitive was received during a Data FIS transfer.  The source of    */
/* this interrupt is from bit 22 (DIAG_H) of the SError register.  This       */
/* interrupt is cleared by writing a 1 to bit 22 of the SError register.      */
#define SUPDIPR_R_ERR_BIT        (3) /* RO: R_ERR Received Interrupt       */

/* SATA Port FIFO Error Interrupt - When set, a FIFO error occurred during    */
/* a Data FIS transfer. The source of this interrupt is from bit 11 (ERR_E)   */
/* of the SError register. This interrupt is cleared by writing a 1 to bit 11 */
/* of the SError register.                                                    */
#define SUPDIPR_FIFO_ERR_BIT     (2) /* RO: Port FIFO Error Interrupt      */

/* SATA Port PHY Ready Interrupt - When set, a SATA Port 0 PHY became READY   */
/* from NOT READY. OOB is done. The source of this interrupt is from bit 1    */
/* (ERR_M) of the SError register. This interrupt is cleared by writing a 1   */
/* to bit 1 of the SError register. The default value after  reset is 0, for  */
/* example the PHY will not be ready. If the PHY becomes ready as part of the */
/* initialization sequence, the value will change to 1.                       */
#define SUPDIPR_READY_INTR_BIT   (1) /* RO: SATA Port PHY Ready Interrupt  */

/* SATA Port PHY Change State Interrupt - When set, the PHY either went from  */
/* READY to NOT-READY, or from NOT-READY to READY. The source of this         */
/* interrupt is from bit 16 (DIAG_N) of the SError register. This interrupt   */
/* is cleared by writing a 1 to bit 16 of the SError register. The default    */
/* value after reset is 0, for example the PHY will not be ready. If the PHY  */
/* becomes ready (state change from not-ready to ready) as part of the        */
/* initialization sequence, the value will change to 1.                       */
#define SUPDIPR_STATE_INTR_BIT   (0) /* RO: PHY Change State Interrupt     */

#define SUPDDCMDR_DIR_VERIFY_BIT (9) /* RO: DMA dir bit 0 = dev to Host    */
#define SUPDDCMDR_ACTIVE_BIT     (8) /* RO: 1st party DMA active ?         */
#define SUPDDCMDR_DIR_BIT        (3) /* RW: 0 = Host to Dev, 1=Dev to Host */
#define SUPDDCMDR_START_STOP_BIT (0) /* RW: 1 = Start, 0 = Stop            */
#define SUPDDSR_DMA_INTR_BIT     (2) /* RC: Port Specific Intr bit.        */
#define SUPDDSR_DMA_ERROR_BIT    (1) /* RC: Port specific DMA Error Bit    */
#define SUPDDSR_DMA_ACTIVE_BIT   (0) /* RO: 0 = DMA Completed              */

/* Write a 1 to clear the SUPDSSER Bits */
#define SUPDSSER_DIAG_F_BIT (25) /* RC: 1= Unknown FIS                     */
#define SUPDSSER_DIAG_H_BIT (22) /* RC: 1= R_ERR recieved. Handshake err   */
#define SUPDSSER_DIAG_C_BIT (21) /* RC: 1= CRC error detected              */
#define SUPDSSER_DIAG_D_BIT (20) /* RC: 1= Disparity error detected        */
#define SUPDSSER_DIAG_W_BIT (18) /* RC: 1= Comm Wake detected              */
#define SUPDSSER_DIAG_N_BIT (16) /* RC: 1= PHYRDY Change state detected    */
#define SUPDSSER_ERR_E_BIT  (11) /* RC: 1= Internal err: FIFO over/underrun*/
#define SUPDSSER_ERR_P_BIT  (10) /* RC: 1= Protocol err: Corrupt FIS       */
#define SUPDSSER_ERR_C_BIT  (9)  /* RC: 1= Non Recovered Comm. No signal   */
#define SUPDSSER_ERR_T_BIT  (8)  /* RC: 1= Transient Data Integrity err    */
#define SUPDSSER_ERR_M_BIT  (1)  /* RC: 1= Recovered comm err PHY now Ready*/

#define SUPDSSCR_HARD_RESRET_BIT (0) /* RW: 1 = Perform hard reset         */
#define SUPDSSCR_DISABLE_BIT     (2) /* RW: 1 = Put PHY in Offline Mode    */

/* Status and AltStatus register bits */
#define SUPDASR_BSY_BIT     (7)  /* RO: 1= the interface/device is busy.   */
#define SUPDASR_DRDY_BIT    (6)  /* RO: 1= indicates that the dev is ready.*/
#define SUPDASR_DRQ_BIT     (3)  /* RO: 1= dev is ready with/for PIO data  */
#define SUPDASR_ERR_BIT     (0)  /* RO: 1= indicates that an error occurred*/

/* Device Control Register bits  */
#define SUPDDCTLR_HOB_BIT   (7)  /* Reserved ? - High Order Byte           */
#define SUPDDCTLR_SRST_BIT  (2)  /* WO: 1 = Start Reset, 0 = Stop reset    */
#define SUPDDCTLR_nIEN_BIT  (1)  /* Reserved ? 0=disable 1=enable Interrupt*/

/* BIST register definitions */
#define SUPDBFCSR_CLR_BIT       0x02000000
#define SUPDBFCSR_PATTERN       0xf0000000
#define SUPDBFCSR_RLOOPBACK     0x00000008
#define SUPDBFCSR_ALOOPBACK     0x00000002
#define SUPDBFCSR_PATGEN        0x01000000
#define SUPDBFCSR_PATCHK        0x00800000

#define SUPDSSSR_DEV_ONLINE     0x00000013

/******************************************************************************/
/*                                                                            */
/*                   M A C R O   D E F I N I T I O N S                        */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                    T Y P E   D E F I N I T I O N S                         */
/*                                                                            */
/******************************************************************************/


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


#endif /* ARTISEA_HEADER */

