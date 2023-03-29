# eCos makefile

# This is a generated file - do not edit

export REPOSITORY := /home/hiroki/xscale/IOP_Redboot_Mar_19_2007/eCos/packages
export PREFIX := /usr/home/hiroki/xscale/IOP_Redboot_Mar_19_2007/eCos/build/install
export COMMAND_PREFIX := xscale-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

PACKAGE := io/pci/current
OBJECT_PREFIX := io_pci
CFLAGS := -mbig-endian -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority -mapcs-frame
LDFLAGS := -mbig-endian -Wl,--gc-sections -Wl,-static -g -O2 -nostdlib
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/pci1 tests/pci2

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/pci.c src/pci_hw.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))
	@cat $^ > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/cyg/io/pci.h $(PREFIX)/include/cyg/io/pci_hw.h $(PREFIX)/include/cyg/io/pci_cfg.h

$(PREFIX)/include/cyg/io/pci.h: $(REPOSITORY)/$(PACKAGE)/include/pci.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/io/pci_hw.h: $(REPOSITORY)/$(PACKAGE)/include/pci_hw.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/io/pci_cfg.h: $(REPOSITORY)/$(PACKAGE)/include/pci_cfg.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

