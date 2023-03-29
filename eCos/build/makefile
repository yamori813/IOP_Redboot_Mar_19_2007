# eCos makefile

# This is a generated file - do not edit

export REPOSITORY := /home/hiroki/xscale/IOP_Redboot_Mar_19_2007/eCos/packages
export HOST := UNIX
export PREFIX := /usr/home/hiroki/xscale/IOP_Redboot_Mar_19_2007/eCos/build/install
export COMMAND_PREFIX := xscale-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export AR := $(COMMAND_PREFIX)ar

.PHONY: default build clean tests headers

build: headers $(PREFIX)/include/pkgconf/ecos.mak
	$(MAKE) -r -C hal/arm/arch/current arm.inc
	$(MAKE) -r -C services/memalloc/common/current heapgeninc.tcl
	$(MAKE) -r -C services/memalloc/common/current heaps.cxx
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/xscale/cores/current $@
	$(MAKE) -r -C hal/arm/xscale/ixp425/current $@
	$(MAKE) -r -C hal/arm/xscale/ixdp425/current $@
	$(MAKE) -r -C io/pci/current $@
	$(MAKE) -r -C devs/eth/intel/i82559/current $@
	$(MAKE) -r -C devs/eth/arm/ixdp425/i82559/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	$(MAKE) -r -C hal/common/current $(PREFIX)/lib/extras.o
	$(MAKE) -r -C hal/arm/arch/current $(PREFIX)/lib/vectors.o
	$(MAKE) -r -C hal/arm/arch/current $(PREFIX)/lib/target.ld
	$(MAKE) -r -C redboot/current $(PREFIX)/bin/redboot.elf
	$(MAKE) -r -C hal/arm/xscale/ixdp425/current $(PREFIX)/bin/redboot.bin
	@echo $@ finished

clean:
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/xscale/cores/current $@
	$(MAKE) -r -C hal/arm/xscale/ixp425/current $@
	$(MAKE) -r -C hal/arm/xscale/ixdp425/current $@
	$(MAKE) -r -C io/pci/current $@
	$(MAKE) -r -C devs/eth/intel/i82559/current $@
	$(MAKE) -r -C devs/eth/arm/ixdp425/i82559/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	@echo $@ finished

tests: build
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/xscale/cores/current $@
	$(MAKE) -r -C hal/arm/xscale/ixp425/current $@
	$(MAKE) -r -C hal/arm/xscale/ixdp425/current $@
	$(MAKE) -r -C io/pci/current $@
	$(MAKE) -r -C devs/eth/intel/i82559/current $@
	$(MAKE) -r -C devs/eth/arm/ixdp425/i82559/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	@echo $@ finished

headers:
	$(MAKE) -r -C hal/arm/arch/current $@
	$(MAKE) -r -C hal/arm/xscale/cores/current $@
	$(MAKE) -r -C hal/arm/xscale/ixp425/current $@
	$(MAKE) -r -C hal/arm/xscale/ixdp425/current $@
	$(MAKE) -r -C io/pci/current $@
	$(MAKE) -r -C devs/eth/intel/i82559/current $@
	$(MAKE) -r -C devs/eth/arm/ixdp425/i82559/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C redboot/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	@echo $@ finished

$(PREFIX)/include/pkgconf/ecos.mak: makefile
	@echo 'ECOS_GLOBAL_CFLAGS = -mbig-endian -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority -mapcs-frame' > $@
	@echo 'ECOS_GLOBAL_LDFLAGS = -mbig-endian -Wl,--gc-sections -Wl,-static -g -O2 -nostdlib' >> $@
	@echo 'ECOS_COMMAND_PREFIX = $(COMMAND_PREFIX)' >> $@

