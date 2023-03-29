
RedBoot for Intel XScale evaluation boards
September 16, 2004
README
========================================================================


This ReadMe contains instructions for running Redboot on the following
Intel XScale based boards:

  - IQ80310
  - IQ80321
  - IQ80315
  - IQ80331
  - IQ80332
  - IQ31244
  - EP80219
  - Mainstone
  - IXDP425
  - GRG
  - Motorola PrPMC1100

You will need the GNUPro xscale-elf toolchain which should be installed
as per the GNUPro documentation.


Overview
--------
These implementations of RedBoot support several configurations:

  * RedBoot running from the board's FLASH boot sector.

  * RedBoot running from RAM with RedBoot in the FLASH boot sector.

Installing Initial RedBoot Image
--------------------------------
Initial installations of RedBoot require the use of a flash utility or device
programmer as indicated by the board manufacturer. Please see appropriate
documentation for details on initial flash programming. A set of prebuilt files
are provided. This set corresponds to each of the supported configurations and
includes an ELF file (.elf), a binary image (.bin), and an S-record file (.srec).

* RedBoot on IQ80310

  - Running from the FLASH boot sector:

  bin/iq80310/redboot_ROM.bin
  bin/iq80310/redboot_ROM.elf
  bin/iq80310/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/iq80310/redboot_RAM.bin
  bin/iq80310/redboot_RAM.elf
  bin/iq80310/redboot_RAM.srec


* RedBoot on IQ80321

  - Running from the FLASH boot sector:

  bin/iq80321/redboot_ROM.bin
  bin/iq80321/redboot_ROM.elf
  bin/iq80321/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/iq80321/redboot_RAM.bin
  bin/iq80321/redboot_RAM.elf
  bin/iq80321/redboot_RAM.srec


* RedBoot on IQ80315

  - Running from the FLASH boot sector:

  bin/iq80315/redboot_ROM.bin
  bin/iq80315/redboot_ROM.elf
  bin/iq80315/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/iq80315/redboot_RAM.bin
  bin/iq80315/redboot_RAM.elf
  bin/iq80315/redboot_RAM.srec


* RedBoot on EP80219

  - Running from the FLASH boot sector:

  bin/ep80219/redboot_ROM.bin
  bin/ep80219/redboot_ROM.elf
  bin/ep80219/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/ep80219/redboot_RAM.bin
  bin/ep80219/redboot_RAM.elf
  bin/ep80219/redboot_RAM.srec


* RedBoot on IQ31244

  - Running from the FLASH boot sector:

  bin/iq31244/redboot_ROM.bin
  bin/iq31244/redboot_ROM.elf
  bin/iq31244/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/iq31244/redboot_RAM.bin
  bin/iq31244/redboot_RAM.elf
  bin/iq31244/redboot_RAM.srec


* RedBoot on IQ80332/IQ80331

  - Running from the FLASH boot sector:

  bin/iq8033x/redboot_ROM.bin
  bin/iq8033x/redboot_ROM.elf
  bin/iq8033x/redboot_ROM.srec

  - Running from the FLASH boot sector, using locked data-cache as SRAM:

  bin/iq8033x/redboot_ROM_cache_sram.bin
  bin/iq8033x/redboot_ROM_cache_sram.elf
  bin/iq8033x/redboot_ROM_cache_sram.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/iq8033x/redboot_RAM.bin
  bin/iq8033x/redboot_RAM.elf
  bin/iq8033x/redboot_RAM.srec


* RedBoot on Mainstone

  - Running from the FLASH boot sector:

  bin/mainstone/redboot_ROM.bin
  bin/mainstone/redboot_ROM.elf
  bin/mainstone/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/mainstone/redboot_RAM.bin
  bin/mainstone/redboot_RAM.elf
  bin/mainstone/redboot_RAM.srec


* RedBoot on IXDP425

  - Running from the FLASH boot sector:

  bin/ixdp425/redboot_ROM.bin
  bin/ixdp425/redboot_ROM.elf
  bin/ixdp425/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/ixdp425/redboot_RAM.bin
  bin/ixdp425/redboot_RAM.elf
  bin/ixdp425/redboot_RAM.srec


* RedBoot on GRG

  - Running from the FLASH boot sector:

  bin/grg/redboot_ROM.bin
  bin/grg/redboot_ROM.elf
  bin/grg/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/grg/redboot_RAM.bin
  bin/grg/redboot_RAM.elf
  bin/grg/redboot_RAM.srec


* RedBoot on PrPMC1100

  - Running from the FLASH boot sector:

  bin/prpmc1100/redboot_ROM.bin
  bin/prpmc1100/redboot_ROM.elf
  bin/prpmc1100/redboot_ROM.srec

  - Running from RAM with RedBoot in the FLASH boot sector:

  bin/prpmc1100/redboot_RAM.bin
  bin/prpmc1100/redboot_RAM.elf
  bin/prpmc1100/redboot_RAM.srec


Initial installations deal with the FLASH based RedBoots. Installation and
use of RAM based RedBoots is documented elsewhere.

To install RedBoot to run from the FLASH boot sector, use the manufacturer's
flash utility to install the redboot_ROM.bin image.

After booting the initial installation of RedBoot, this warning may be
printed:

  FLASH configuration checksum error or invalid key

This is normal and indicates that the FLASH must be configured for use by
RedBoot. Even if the above message is not printed, it is a good idea to
reinitialize the FLASH anyway. Do this with the fis command:

  RedBoot> fis init
  About to initialize [format] FLASH image system - are you sure (y/n)? y
  *** Initialize FLASH Image System
      Warning: device contents not erased, some blocks may not be usable
  ... Unlock from 0xf1fc0000-0xf2000000: .
  ... Erase from 0xf1fc0000-0xf2000000: .
  ... Program from 0x03fbf000-0x03fff000 at 0xf1fc0000: .
  ... Lock from 0xf1fc0000-0xf2000000: .


Followed by the fconfig command:

  RedBoot> fconfig -i
  Initialize non-volatile configuration - continue (y/n)? y
  Run script at boot: false
  Use BOOTP for network configuration: true
  Console baud rate: 115200
  DNS server IP address: 
  GDB connection port: 9000
  Force console for special debug messages: false
  Network debug at boot time: false
  Update RedBoot non-volatile configuration - continue (y/n)? y
  ... Unlock from 0xf1f80000-0xf1f81000: .
  ... Erase from 0xf1f80000-0xf1f81000: .
  ... Program from 0x03fb2000-0x03fb3000 at 0xf1f80000: .
  ... Lock from 0xf1f80000-0xf1f81000: .


IQ80310 Diagnostics
-------------------
RedBoot has a 'diag' command which will provide access to a menu of hardware
setup and diagnostic functions:

  RedBoot> diag
  Entering Hardware Diagnostics - Disabling Data Cache!

  Select your Host test system

  Make a selection by typing a number.

  1 - Cyclone SB923
  2 - Personal Computer or other

After selecting 1 or 2, this menu appears:

    IQ80310 Hardware Tests

   1 - Memory Tests
   2 - Repeating Memory Tests
   3 - 16C552 DUART Serial Port Tests
   4 - Rotary Switch S1 Test
   5 - 7 Segment LED Tests
   6 - Backplane Detection Test
   7 - Battery Status Test
   8 - External Timer Test
   9 - i82559 Ethernet Configuration
  10 - i82559 Ethernet Test
  11 - i960Rx/303 PCI Interrupt Test
  12 - Internal Timer Test
  13 - Secondary PCI Bus Test
  14 - Primary PCI Bus Test
  15 - Battery Backup SDRAM Memory Test
  16 - GPIO Test
  17 - Repeat-On-Fail Memory Test
  18 - Coyonosa Cache Loop (No return)
  19 - Show Software and Hardware Revision
   0 - quit

Tests for various hardware subsystems are provided. Some tests require special
hardware in order to execute normally. The Ethernet Configuration item may be
used to set the board ethernet address.

IQ80321 Switch Settings
-----------------------
The 80321 board is highly configurable through a number of switches and
jumpers. RedBoot makes some assumptions about board configuration and attention
must be paid to these assumptions for reliable RedBoot operation:

The onboard ethernet and the secondary slot may be placed in a private space so
that they are not seen by a PC BIOS. If the board is to be used in a PC with
BIOS, then the ethernet should be placed in this private space so that RedBoot
and the BIOS do not conflict.

RedBoot assumes that the board is plugged into a PC with BIOS. This requires
RedBoot to detect when the BIOS has configured the PCI-X secondary bus. If the
board is placed in a backplane, RedBoot will never see the BIOS configure the
secondary bus. To prevent this wait, set switch S7E1-3 to ON when using the
board in a backplane.

      For the remaining switch settings, the following is a known good configuration: 

       S1D1            All OFF
       S7E1            7 is ON, all others OFF
       S8E1            2,3,5,6 are ON, all others OFF
       S8E2            2,3 are ON, all others OFF
       S9E1            3 is ON, all others OFF
       S4D1            1,3 are ON, all others OFF
       J9E1            2,3 jumpered
       J9F1            2,3 jumpered
       J3F1            Nothing jumpered
       J3G1            2,3 jumpered
       J1G2            2,3 jumpered

IQ80321 Diagnostics
-------------------
A special RedBoot command, diag, is used to access a set of hardware
diagnostics. To access the diagnostic menu, enter diag at the RedBoot prompt:
 RedBoot> diag
 Entering Hardware Diagnostics - Disabling Data Cache!

   IQ80321 Hardware Tests

  1 - Memory Tests
  2 - Repeating Memory Tests
  3 - Repeat-On-Fail Memory Tests
  4 - Rotary Switch S1 Test
  5 - 7 Segment LED Tests
  6 - i82544 Ethernet Configuration
  7 - Baterry Status Test
  8 - Battery Backup SDRAM Memory Test
  9 - Timer Test
 10 - PCI Bus test
 11 - CPU Cache Loop (No Return)
  0 - quit
 Enter the menu item number (0 to quit):


Tests for various hardware subsystems are provided, and some tests require
special hardware in order to execute normally. The Ethernet Configuration item
may be used to set the board ethernet address.


IQ80315 Switch Settings
-----------------------
The IQ80315 platform is highly configurable with the switches and jumpers on-board.
The default switch settings are shown below and are a known good configuration
that are used to boot the IQ80315.  

The IQ80315 also has 2 Ethernet ports on board,
of which RedBoot only uses Port0.  Port0 is the lower of the two connectors.  
The EEPROM on-board must contain the MAC address for the ethernet port in order
for the ethernet driver to initialize.  The MAC address for the board is located
on a sticker on the top of the platform.  Programming and erification of the MAC 
address can be done in the diag menu, under "IOC80314 Ethernet Configuration."

Default Jumper and Switch Settings:
Jumpers
J5F1  pins 1 to 2
J5F2  pins 1 to 2
J5F3  pins 1 to 2
J1D2  pins 1 to 2
J1D1  pins 1 to 2
S3E2  SATA DPA Mode
      Switch 1 and 4 On
      Switch 2 and 3 Off
S3E1  PCI A bus speed      sw1      sw2      sw3      sw4
      PCI-X 100            on       off      off      off < Not Supported on A0 >
      PCI-X 50             on       on       off      off < Not Supported on A0 >
      PCI 50               on       on       on       off < Default Setting >
      PCI 25               on       on       on       on

S2F1  PCI B bus speed      sw1      sw2      sw3      sw4
      PCI-X 100            off      on       off      off < Default Setting >
      PCI-X 50             on       on       off      off
      PCI 50               on       on       on       off
      PCI 25               on       on       on       on

S3F1  Jtag       
      Switch 1 on
      Switches 2, 3, and 4 off


IQ80315 Diagnostics
-------------------
A special RedBoot command, diag, is used to access a set of hardware
diagnostics. To access the diagnostic menu, enter diag at the RedBoot prompt:
 RedBoot> diag
 Entering Hardware Diagnostics - Disabling Data Cache!


IQ80315 Hardware Tests

 1 - Memory Test Sub-Menu
 2 - Rotary Switch Test
 3 - 7 Segment LED Tests
 4 - IOC80314 Ethernet Configuration
 5 - Battery Status Test
 6 - Battery Backup SDRAM Memory Test
 7 - Timer Test
 8 - PCI Bus test
 9 - CPU Cache Loop (No return)
10 - Read DDR0 and DDR1 Size on I2C
11 - Read Temp Sensors on I2C
12 - Basic Sanity Tests
13 - Loop Version of Basic Sanity Tests
14 - EEPROM menu
15 - RTC menu
16 - Fan menu
17 - Test Buzzer
18 - CompactFlash Test
19 - LCD Test Menu
 0 - quit
Enter the menu item number (0 to quit):

Tests for various hardware subsystems are provided, and some tests require
special hardware in order to execute normally. 


IQ80331 Switch Settings
-----------------------------
The IQ80331 platform is highly configurable with the switches and jumpers on-board.
The default switch settings are shown below and are a known good configuration
that are used to boot the IQ80331.  

The onboard ethernet and the secondary slot may be placed in a private space so
that they are not seen by a PC BIOS. If the board is to be used in a PC with
BIOS, then the ethernet should be placed in this private space so that RedBoot
and the BIOS do not conflict.

Switch Settings relevant to RedBoot:

(Open == pushed in at bottom; Closed == pushed in at top)

Position 2:  Open: allow core to execute 
             Closed: hold core in reset 
Position 3:  Open: config retry enabled -- use this when booting in a host 
             Closed: config retry disabled -- use this when booting in a backplane 
Position 5:  Open: Enable private devices -- use this to hide the slot and onboard GbE
             from the host and to allow RedBoot to configure and use these devices 
             Closed: Disable private devices.



IQ80332 Switch Settings
-----------------------------
The IQ80332 platform is highly configurable with the switches and jumpers on-board.
The default switch settings are shown below and are a known good configuration
that are used to boot the IQ80332.  

The onboard ethernet and the secondary slot may be placed in a private space so
that they are not seen by a PC BIOS. If the board is to be used in a PC with
BIOS, then the ethernet should be placed in this private space so that RedBoot
and the BIOS do not conflict.  Put the rotary switch in position 1 to enable
device hiding.

Switch Settings relevant to RedBoot:

(Open == pushed in at bottom; Closed == pushed in at top)

Position 2:  Open: allow core to execute 
             Closed: hold core in reset 
Position 3:  Open: config retry enabled -- use this when booting in a host 
             Closed: config retry disabled -- use this when booting in a backplane 
Rotary Switch: 
Position 1:  Enable private devices -- use this to hide the slot and onboard GbE
             from the host and to allow RedBoot to configure and use these devices 
             Others: Disable private devices.



IQ80332/IQ80331 Diagnostics
------------------------------------
A special RedBoot command, diag, is used to access a set of hardware
diagnostics. To access the diagnostic menu, enter diag at the RedBoot prompt:
 RedBoot> diag
 Entering Hardware Diagnostics - Disabling Data Cache!

  IQ80331/IQ80332 CRB Hardware Tests

 1 - Memory Tests
 2 - Random-write Memory Tests
 3 - Repeating Memory Tests
 4 - Repeat-On-Fail Memory Test
 5 - Rotary Switch S1 Test
 6 - 7 Segment LED Tests
 7 - i82545 Ethernet Configuration
 8 - i82545 Ethernet Test
 9 - Battery Status Test
10 - Battery Backup SDRAM Memory Test
11 - Timer Test
12 - PCI Bus test
13 - CPU Cache Loop (No return)
 0 - quit


Tests for various hardware subsystems are provided, and some tests require
special hardware in order to execute normally. The Ethernet Configuration item
may be used to initialize the GbE EEPROM and set the board ethernet address.


Mainstone Switch Settings
-----------------------

Mainstone switches should be setup so that RedBoot boots from CPU card flash
on a 32-bit bus:

   Baseboard: SW1 - dot
              SW2 - dot

   CPU board: SW3 - dot

CPU core speed is set through the rotary hex switch SW9 on the mainstone board:

   Value   Core (MHz)
   -----   ----------
     0        91.0
     1        39.0
     2        65.0
     3        91.0
     4       117.0
     5       143.0
     6       169.0
     7       195.0

NPE Ethernet Support
--------------------
The IXDP425, GRG, and PrPMC1100 boards support the two builtin NPE ethernet
ports. The IXDP425 and GRG also support PCI based i82559 NICs. For the GRG
and IXDP425 board, the default ethernet port is the PCI based NIC. For the
PrPMC1100 board, the default is the EthA port. The GRG board has a group
of four RJ-45 connectors for the first NPE port to use. RedBoot uses only
the leftmost RJ-45 connector from the group of four.

RedBoot allows you to set a preferred default ethernet port using the RedBoot
"fconfig" command. If the default ethernet device is not found (for instance,
the PCI card is not installed), RedBoot will try to use one of the other ports.
The default port is selected with the "fconfig net_device" command. Acceptable
devices for the "fconfig net_device" command are:

   i82559_eth0
   npe_eth0
   npe_eth1

It may be necessary to set the MAC address (or Ethernet Station Address) of
the port before it is usable by RedBoot. The IXDP425 and PrPMC1100 board store
the MAC addresses for the NPE ethernet ports in onboard serial EEPROMs. The GRG
board uses the main flash to hold the NPE ports MAC addresses. To set the MAC
address of an NPE port on the IXDP425 or PrPMC1100 board, use the "set_npe_mac"
command. This command accepts a "-p" switch to specify the port (0 for NPEB,
1 for NPEC) and a MAC address formatted as xx:xx:xx:xx:xx:xx or xxxxxxxxxxxx.
For instance, to set the MAC address for npe_eth0 (NPEB) use:

  RedBoot> set_npe_mac -p 0 00:01:AF:00:20:EC

To set the MAC address of an NPE port on the GRG board, use the RedBoot
fconfig. To set the MAC address of npe_eth0 (NPEB) use:

  RedBoot> fconfig npe_eth0_esa

To set the MAC address of npe_eth1 (NPEC) use:

  RedBoot> fconfig npe_eth1_esa


Rebuilding RedBoot
------------------

The build process is nearly identical all of the six supported configurations.
Assuming that the provided RedBoot source tree is located in the current 
directory and that we want to build a RedBoot that runs from the FLASH boot
sector, the build process for the IQ80310 is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new iq80310 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/iq80310/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For the IQ80321 the build process is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new iq80321 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/iq80321/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For IQ31244, the build process is

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new iq31244 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/iq31244/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For EP80219, the build process is

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new ep80219 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/ep80219/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For Mainstone the build process is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new mainstone redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/mainstone/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For the IXDP425 board, the build process is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new ixdp425 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/ixdp425/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For the GRG board, the build process is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new grg redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/grg/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

For the PrPMC1100 board, the build process is:

  % export TOPDIR=`pwd`
  % export ECOS_REPOSITORY=${TOPDIR}/packages
  % export VERSION=current
  % mkdir ${TOPDIR}/build
  % cd ${TOPDIR}/build
  % ecosconfig new prpmc1100 redboot
  % ecosconfig import ${ECOS_REPOSITORY}/hal/arm/xscale/prpmc1100/${VERSION}/misc/redboot_ROM.ecm
  % ecosconfig tree
  % make

If a different configuration is desired, simply use the above build processes but
substitute an alternate configuration file for the ecosconfig import command.

Rebuilding RedBoot under Cygwin
-------------------------------

All tools required to rebuild RedBoot under Cygwin are included in the latest
Xscale release.  Two minor post-installation steps are required before the
'make' can successfully run.

1.  Create a mount table such that /bin/sh.exe exists like so:

    C:\>mount -f -b c:\redhat\xscale-030422\H-i686-pc-cygwin /

2.  Set the SHELL environment variable to point to sh.exe like so:

    C:\>set SHELL=/bin/sh.exe

These steps must be run once before using the 'make' command.  Once run,
the settings will persist until the window they are issued in is closed.

Building ecosconfig
-------------------

An ecosconfig binary is supplied in the bin directory, but you may wish
to build it from source.

Detailed instructions for building the command-line tool ecosconfig
on UNIX can be found in host/README. For example:

  mkdir $TEMP/redboot-build
  cd $TEMP/redboot-build
  $TOPDIR/host/configure --prefix=$TEMP/redboot-build --with-tcl=/usr
  make 
