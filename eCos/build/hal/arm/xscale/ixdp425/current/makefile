# eCos makefile

# This is a generated file - do not edit

export REPOSITORY := /home/hiroki/xscale/IOP_Redboot_Mar_19_2007/eCos/packages
export PREFIX := /usr/home/hiroki/xscale/IOP_Redboot_Mar_19_2007/eCos/build/install
export COMMAND_PREFIX := xscale-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

PACKAGE := hal/arm/xscale/ixdp425/current
OBJECT_PREFIX := hal_arm_xscale_ixdp425
CFLAGS := -mbig-endian -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority -mapcs-frame
LDFLAGS := -mbig-endian -Wl,--gc-sections -Wl,-static -g -O2 -nostdlib
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := 

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/ixdp425_misc.c src/ixdp425_pci.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)))
	@cat $^ > $(@:.stamp=.deps)
	@touch $@

$(PREFIX)/bin/redboot.bin: $(wildcard $(PREFIX)/bin/redboot.elf)
	$(OBJCOPY) --strip-debug $< $(@:.bin=.img) 
	$(OBJCOPY) -O srec $< $(@:.bin=.srec)
	$(OBJCOPY) -O binary $< $@
	

headers: mlt_headers $(PREFIX)/include/cyg/hal/plf_io.h $(PREFIX)/include/cyg/hal/hal_platform_extras.h $(PREFIX)/include/cyg/hal/hal_plf_ints.h $(PREFIX)/include/cyg/hal/ixdp425.h $(PREFIX)/include/cyg/hal/hal_platform_setup.h

$(PREFIX)/include/cyg/hal/plf_io.h: $(REPOSITORY)/$(PACKAGE)/include/plf_io.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_platform_extras.h: $(REPOSITORY)/$(PACKAGE)/include/hal_platform_extras.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_plf_ints.h: $(REPOSITORY)/$(PACKAGE)/include/hal_plf_ints.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/ixdp425.h: $(REPOSITORY)/$(PACKAGE)/include/ixdp425.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_platform_setup.h: $(REPOSITORY)/$(PACKAGE)/include/hal_platform_setup.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

