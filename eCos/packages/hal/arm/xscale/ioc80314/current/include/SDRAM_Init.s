#############################################################################
#
# Tundra Semiconductor Corporation
# 
# Copyright 2002, 2003, Tundra Semiconductor Corporation
# 
# This software is the property of Tundra Semiconductor Corporation
# (Tundra), which specifically grants the user the right to use this
# software in connection with the Intel(r) 80314 Companion Chip, provided
# this notice is not removed or altered.  All other rights are reserved
# by Tundra.
# 
# TUNDRA MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD
# TO THIS SOFTWARE.  IN NO EVENT SHALL AMD BE LIABLE FOR INCIDENTAL OR
# CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING FROM THE
# FURNISHING, PERFORMANCE, OR USE OF THIS SOFTWARE.
# 
#############################################################################

#########################################################################
#
# File Name: TsiSDRAM.s
# Description: This files contains the assembly code required to
# initialize the Power Pro Processor Bus Interface (PB), SDRAM banks 
# (reading and using SDRAM presence information), and the EE Banks.
#
# Date          Name    Description
# -------------------------------------------------------------
# Feb.27,2002   avb     Initial creation for FPGA Emulation Testing.
#
#########################################################################


.set  TSI_BASE,     0xC0000000   # Tsi Chip Base Address

.set  PB_BAR2,         0x820   # PB Base Address Register
.set  PB_MAB2,         0x824   # PB Address Mask Register
.set  PB_TAR2,         0x828   # PB Address Translation Register

.set  PB_BAR3,         0x830   # PB Base Address Register
.set  PB_MAB3,         0x834   # PB Address Mask Register
.set  PB_TAR3,         0x838   # PB Address Translation Register

.set  SD_REFRESH,      0x4000  # SDRAM Refresh Interval Register
.set  SD_CNTRL,        0x4004  # SDRAM Control Register
.set  SD_BANK_CNTRL,   0x4008  # SDRAM Memory Bank Control Register
.set  SD_B0_ADDR,      0x4010  # SDRAM Memory Bank 0 Address Register
.set  SD_B0_MASK,      0x4014  # SDRAM Memory Bank 0 Mask Register
.set  SD_ECC_STAT,     0x4058  # SDRAM ECC Status/Control Register



#========================================================================
# Function:       TsiSDRAMInit
# Arguments:      - none
# Return value:   - none
# Registers used: r3,r4
# Description:    This function is used to initialize the TsiSDRAM Controller. 
#
# --------------------------------------
# Date modified   Author  Description
# --------------------------------------
# Feb.27, 2002    avb     Implemented for FPGA Emulation Testing.
#
#========================================================================

.globl TsiSDRAMInit

TsiSDRAMInit:
         
   addis    r3, 0,%hi(TSI_BASE) # PowerPro Base Address (PB init part)
   ori      r3,r3,%lo(TSI_BASE)

# Clear ECC Control/Status Register to make sure that ECC is disabled
   addis    r4,0, 0x0000
   stw      r4, SD_ECC_STAT(r3)   

# Set SDRAM Refresh Interval
   addis    r4,0, 0x0000
   ori      r4,r4,0x0080
   stw      r4, SD_REFRESH(r3)   

# Set SDRAM Memory Bank Control Register
   addis    r4,0, 0x3a05
   ori      r4,r4,0x2244
   stw      r4, SD_BANK_CNTRL(r3)   

# Set SDRAM Memory Bank 0 Address Register
   addis    r4,0, 0x0010
   ori      r4,r4,0x0000
   stw      r4, SD_B0_ADDR(r3)   

# Set SDRAM Memory Bank 0 Address Mask Register
   addis    r4,0, 0x000F
   ori      r4,r4,0xE000
   stw      r4, SD_B0_MASK(r3)   

# Set SDRAM Control Register
   addis    r4,0, 0x9010  # ENABLE=1,DQM_EN=0,CL=2,
#   addis    r4,0, 0xD010   # as above but DQM_EN = 1 
   ori      r4,r4,0xc000
   stw      r4, SD_CNTRL(r3)   

   bclr  20,0  # return from subroutine

