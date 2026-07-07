GAS=$(shell which tms9900-as)
LD=$(shell which tms9900-ld)
CC=$(shell which tms9900-gcc)
OBJCOPY=$(shell which tms9900-objcopy)
OBJDUMP=$(shell which tms9900-objdump)
XGA99?=$(shell which xga99.py)
XDM99?=$(shell which xdm99.py)

FNAME=FCMD

VER:=$(shell grep "\#define APP_VER" src/main.h | cut -d '"' -f2)

SUPPORT=FC/BOOT FC/LOAD FC/FCMD FC/FCMDXB FC/BIN/DISKIMAGE FC/BIN/FCMENU FC/BIN/FTP FC/BIN/SAMPLE FC/BIN/SAY FC/BIN/TELNET FC/BIN/VIRGIL99 FC/BIN/FONT FC/MDOSANSI FC/MDOSFONT

SUBDIRS=hello samshello charset diskimage fcmenu ftp say telnet virgil font

CFLAGS=\
  -std=gnu99 -nostdlib -ffreestanding -Os -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Werror --save-temps -I$(abspath src) -I$(abspath src/libti99)

ASFLAGS=\
  --defsym BASE_ADDR=$(BASE_ADDR) --defsym CONSOLE_ROM=$(CONSOLE_ROM)

ifeq ($(ONE_TARGET),)

# ======================================================================
# TOP-LEVEL DISPATCH (ONE_TARGET not set)
# ======================================================================

.PHONY: all build_0x6000 build_0x0000 subdirs support clean

all: src/fcbanner.asm linkfile.m4 forcecmd_$(VER).zip

build_0x6000: src/fcbanner.asm linkfile.m4
	$(MAKE) ONE_TARGET=1 BASE_ADDR=0x6000

build_0x0000: src/fcbanner.asm linkfile.m4
	$(MAKE) ONE_TARGET=1 BASE_ADDR=0x0000

src/fcbanner.asm: fcbanner.ans scripts/ans2asm.py
	python3 ./scripts/ans2asm.py

linkfile.m4: scripts/gen-linkfile-m4.py linkfile.m4.in src/fcbanner.asm $(wildcard src/*.c) $(wildcard src/*.asm) $(wildcard src/libti99/*.c)
	python3 ./scripts/gen-linkfile-m4.py $@

subdirs: build_0x6000
	cp build_0x6000/fcsdk.linkfile example/gcc/fcsdk/fc.ld
	for d in $(SUBDIRS); do $(MAKE) -C example/gcc/$$d; done

support:
	$(MAKE) -C FC

$(FNAME).DSK: subdirs support
	python3 $(XDM99) $(FNAME).DSK -X 360 -t -a FC/BIN/DISKIMAGE FC/BIN/FCMENU FC/BIN/FTP FC/BIN/TELNET FC/BIN/SAY FC/BIN/SAMPLE FC/BIN/VIRGIL99 FC/BIN/FONT FC/MDOSANSI FC/MDOSFONT

fcsdk: subdirs
	rm -fr ./fcsdk
	cp -a example/gcc/fcsdk ./fcsdk
	cp -a example/gcc/hello ./fcsdk/hello
	cp -a example/gcc/samshello ./fcsdk/samshello
	find fcsdk -name "objects" -o -name "mapfile" -o -name "*HELLO*" | xargs rm -r

forcecmd_$(VER).zip: build_0x6000 build_0x0000 subdirs support $(FNAME).DSK fcsdk
	rm -f $@
	zip -j $@ README.md build_0x0000/FCMD_0000.bin build_0x6000/FCMD.RPK build_0x6000/FCMDC.bin build_0x6000/FCMDG.bin $(FNAME).DSK
	zip -r $@ $(SUPPORT) fcsdk

clean:
	rm -f forcecmd_*.zip
	rm -f api.asm api.banks
	rm -f src/fcbanner.asm
	rm -f *.RPK
	rm -f *.DSK
	rm -f linkfile.m4 linkfile fcsdk.linkfile
	rm -fr build_0x6000 build_0x0000
	$(MAKE) -C FC clean
	for d in $(SUBDIRS); do $(MAKE) -C example/gcc/$$d clean; done
	rm -fr fcsdk

else

# ======================================================================
# SINGLE-TARGET BUILD (ONE_TARGET=1)
# ======================================================================

BUILD_DIR = build_$(BASE_ADDR)
OBJ_DIR   = $(BUILD_DIR)/objects

BASE_HEX_STR := $(shell printf "%04X" $$(( $(BASE_ADDR) )) )
BASE_80_HEX_STR := $(shell printf "%X" $$(( $(BASE_ADDR) + 0x80 )) )

ifneq ($(filter 0x0000,$(BASE_ADDR)),)
override CONSOLE_ROM=1
endif
CONSOLE_ROM ?= 0

ifeq ($(CONSOLE_ROM),1)
HEADER_SRC = src/header_console.asm
FINAL_BIN  = $(BUILD_DIR)/$(FNAME)_$(BASE_HEX_STR).bin
else
HEADER_SRC = src/header_cart.asm
FINAL_BIN  = $(BUILD_DIR)/$(FNAME)C.bin
endif

# Default target when recursing: build the final binary, fcsdk.linkfile, and RPK
.PHONY: _one_target_all
_one_target_all: $(FINAL_BIN) $(BUILD_DIR)/fcsdk.linkfile

# Cartridge-only: also build the RPK (which pulls in FCMDG.bin)
ifneq ($(CONSOLE_ROM),1)
_one_target_all: $(BUILD_DIR)/$(FNAME).RPK
endif

CFLAGS += -DCONSOLE_ROM=$(CONSOLE_ROM) -DBASE_ADDR=$(BASE_ADDR) -DBASE_HEX_STR=$(BASE_HEX_STR) -DBASE_80_HEX_STR=$(BASE_80_HEX_STR)

BANKBINS:=$(shell seq -s ' ' -f "$(BUILD_DIR)/bank%1g.page" 0 15)

LDFLAGS=\
  --script=$(BUILD_DIR)/linkfile

HEADBIN:=$(OBJ_DIR)/header.bin

SRCS:=$(sort $(wildcard src/*.c) $(wildcard src/*.asm))
LIBTI99_SRCS=$(sort $(wildcard src/libti99/*.c))

LIBTI99_OBJS:=$(notdir $(LIBTI99_SRCS:.c=.o))
OBJECT_LIST:=$(notdir $(SRCS:.c=.o)) $(LIBTI99_OBJS)
OBJECT_LIST:=$(filter-out api.o fcbanner.o header_cart.o header_console.o trampoline.o trampoline_alt.o,$(OBJECT_LIST:.asm=.o)) api.o fcbanner.o
OBJECT_LIST+=header.o trampoline.o

LINK_OBJECTS:=$(addprefix $(OBJ_DIR)/,$(OBJECT_LIST))

COMMON_SIZE = 128

# Explicit header rule
$(OBJ_DIR)/header.o: $(HEADER_SRC) | $(OBJ_DIR)
	cd $(OBJ_DIR); $(GAS) $(ASFLAGS) $(abspath $(HEADER_SRC)) -o header.o

# Select trampoline source based on bank switching hardware
ifeq ($(CONSOLE_ROM),1)
TRAMPOLINE_SRC = src/trampoline_alt.asm
else
TRAMPOLINE_SRC = src/trampoline.asm
endif

$(OBJ_DIR)/trampoline.o: $(TRAMPOLINE_SRC) | $(OBJ_DIR)
	cd $(OBJ_DIR); $(GAS) $(ASFLAGS) $(abspath $<) -o trampoline.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BUILD_DIR)

$(HEADBIN): $(BUILD_DIR)/$(FNAME).elf
	$(OBJCOPY) -O binary -j .text $< $(HEADBIN)
	@dd if=/dev/null of=$(HEADBIN) bs=$(COMMON_SIZE) seek=1

$(BUILD_DIR)/bank0.page: $(BUILD_DIR)/$(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .data $< $(OBJ_DIR)/data.bin_tmp
	$(OBJCOPY) -O binary -j .bank0 $< $(OBJ_DIR)/$(@F)_tmp
	cat $(HEADBIN) $(OBJ_DIR)/$(@F)_tmp $(OBJ_DIR)/data.bin_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(BUILD_DIR)/bank%.page: $(BUILD_DIR)/$(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .$(basename $(@F)) $< $(OBJ_DIR)/$(@F)_tmp
	cat $(HEADBIN) $(OBJ_DIR)/$(@F)_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

# Console ROM: 512KB image with original console ROM patched in
ifeq ($(CONSOLE_ROM),1)
$(FINAL_BIN): $(BANKBINS) 994a_rom/994a_rom_8k.bin
	cat $(BANKBINS) > $@
	dd if=/dev/zero bs=1 count=$$((512*1024 - 128*1024 - 8192)) >> $@ 2>/dev/null
	cat 994a_rom/994a_rom_8k.bin >> $@
else
$(FINAL_BIN): $(BANKBINS)
	cat $^ >$@
endif

# GROM boot binary — cartridge only
ifneq ($(CONSOLE_ROM),1)
$(BUILD_DIR)/$(FNAME)G.bin: src/gpl-boot.g99 $(BUILD_DIR)/$(FNAME).elf
	python3 $(XGA99) -D "CART=$(shell echo -n '>' ; grep _cart $(BUILD_DIR)/mapfile | sed 's/^\s*0x0*\([0-9a-f]*\) *_cart/\1/')" -o $@ $<
endif

$(BUILD_DIR)/$(FNAME).elf: $(LINK_OBJECTS) $(BUILD_DIR)/linkfile
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(BUILD_DIR)/$(FNAME).elf -Map=$(BUILD_DIR)/mapfile

$(BUILD_DIR)/linkfile: linkfile.m4 | $(BUILD_DIR)
	m4 -DBASE_ADDR=$(BASE_ADDR) -DOBJDIR=$(OBJ_DIR) $< > $@

# Compilation rules
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	cd $(OBJ_DIR); $(CC) -c $(abspath $<) $(CFLAGS) -o $(notdir $@)

$(OBJ_DIR)/%.o: src/%.asm | $(OBJ_DIR)
	cd $(OBJ_DIR); $(GAS) $(ASFLAGS) $(abspath $<) -o $(notdir $@)

$(OBJ_DIR)/%.o: src/libti99/%.c | $(OBJ_DIR)
	cd $(OBJ_DIR); $(CC) -c $(abspath $<) $(CFLAGS) -o $(notdir $@)

# API table generation (per-variant — bank addresses depend on BASE_ADDR)
$(OBJ_DIR)/api.asm: fc_api.lst scripts/makeapi.py fc_api_template | $(OBJ_DIR)
	grep DECLARE_BANKED src/libti99/*.h src/*.h >$(OBJ_DIR)/api.banks
	python3 ./scripts/makeapi.py fc_api.lst $(OBJ_DIR)/api.asm $(OBJ_DIR)/api.banks example/gcc/fcsdk/fc_api.h $(BASE_ADDR)

# api.asm is per-variant (in OBJ_DIR), so explicit rule needed — pattern rule looks for it at project root
$(OBJ_DIR)/api.o: $(OBJ_DIR)/api.asm | $(OBJ_DIR)
	cd $(OBJ_DIR); $(GAS) $(ASFLAGS) $(abspath $<) -o api.o

# fcsdk linkfile — symbols for client programs
$(BUILD_DIR)/fcsdk.linkfile: $(BUILD_DIR)/$(FNAME).elf
	( echo -n "memcpy = 0x"; grep gcc_memcpy $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_memcpy/\1;/' ) > $@
	( echo -n "memset = 0x"; grep gcc_memset $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_memset/\1;/' ) >> $@
	( echo -n "__adddf3 = 0x"; grep gcc_adddf3 $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_adddf3/\1;/' ) >> $@
	( echo -n "__subdf3 = 0x"; grep gcc_subdf3 $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_subdf3/\1;/' ) >> $@
	( echo -n "__muldf3 = 0x"; grep gcc_muldf3 $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_muldf3/\1;/' ) >> $@
	( echo -n "__divdf3 = 0x"; grep gcc_divdf3 $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_divdf3/\1;/' ) >> $@
	( echo -n "__ltdf2 = 0x"; grep gcc_d_compare $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_d_compare/\1;/' ) >> $@
	( echo -n "__ledf2 = 0x"; grep gcc_d_compare $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_d_compare/\1;/' ) >> $@
	( echo -n "__gtdf2 = 0x"; grep gcc_d_compare $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_d_compare/\1;/' ) >> $@
	( echo -n "__gedf2 = 0x"; grep gcc_d_compare $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_d_compare/\1;/' ) >> $@
	( echo -n "__eqdf2 = 0x"; grep gcc_eqdf2 $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_eqdf2/\1;/' ) >> $@
	( echo -n "__nedf2 = 0x"; grep gcc_eqdf2 $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_eqdf2/\1;/' ) >> $@
	( echo -n "__floatsidf = 0x"; grep gcc_floatsidf $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_floatsidf/\1;/' ) >> $@
	( echo -n "__fixdfsi = 0x"; grep gcc_fixdfsi $(BUILD_DIR)/mapfile | egrep -v '\.' | sed 's/^\s*0x0*\([0-9a-f]*\) *gcc_fixdfsi/\1;/' ) >> $@

# Cartridge-only: RPK packaging
ifneq ($(CONSOLE_ROM),1)
$(BUILD_DIR)/$(FNAME).RPK: $(FINAL_BIN) $(BUILD_DIR)/$(FNAME)G.bin layout.xml
	cp layout.xml $(BUILD_DIR)/
	cd $(BUILD_DIR) && zip $(FNAME).RPK $(FNAME)C.bin $(FNAME)G.bin layout.xml

$(BUILD_DIR)/$(FNAME)_ALT.RPK: $(FINAL_BIN) layout_alt.xml
	cp $(FINAL_BIN) layout_alt.xml /tmp
	mv /tmp/layout_alt.xml /tmp/layout.xml
	cd /tmp && zip $@ layout.xml $(FNAME)C.bin
	mv /tmp/$@ $(BUILD_DIR)/
endif

endif
