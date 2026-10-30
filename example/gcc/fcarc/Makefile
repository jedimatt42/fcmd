GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
OBJCOPY=tms9900-objcopy
FCSDK=$(abspath ../fcsdk)
XDM99?=$(shell which xdm99.py)

FNAME_CREATE=FCARC
FNAME_READ=FCUNARC

CFLAGS=-std=gnu99 -O2 -Werror --save-temps -I$(FCSDK)
LDFLAGS_CREATE=--script=$(FCSDK)/fc.ld --script=linkfile.create
LDFLAGS_READ=--script=$(FCSDK)/fc.ld --script=linkfile.read

CREATE_SRCS=b0_main.c b0_catalog.c b0_data.c b0_file.c b3_create.c b4_legacy.c b7_create_file.c
READ_SRCS=b0_main.c b0_data.c b0_file.c b1_read.c b2_legacy.c b5_fcarc.c b6_test.c

CREATE_OBJS=$(addprefix objects/create/,$(CREATE_SRCS:.c=.o))
READ_OBJS=$(addprefix objects/read/,$(READ_SRCS:.c=.o))

CREATE_PAGES=objects/create/bank0.page objects/create/bank1.page objects/create/bank2.page objects/create/bank3.page
READ_PAGES=objects/read/bank0.page objects/read/bank1.page objects/read/bank2.page objects/read/bank3.page objects/read/bank4.page

all: $(FNAME_CREATE) $(FNAME_READ)

$(FNAME_CREATE).elf: $(CREATE_OBJS) objects/create/_header_create.o
	$(LD) $(CREATE_OBJS) objects/create/_header_create.o $(LDFLAGS_CREATE) -o $@ -Map=objects/create/mapfile

$(FNAME_READ).elf: $(READ_OBJS) objects/read/_header_read.o
	$(LD) $(READ_OBJS) objects/read/_header_read.o $(LDFLAGS_READ) -o $@ -Map=objects/read/mapfile

$(FNAME_CREATE).bin: $(CREATE_PAGES) objects/create/data.bin
	cat $^ > $@

$(FNAME_READ).bin: $(READ_PAGES) objects/read/data.bin
	cat $^ > $@

objects/create/data.bin: $(FNAME_CREATE).elf
	$(OBJCOPY) -O binary -j .data $< $@
	DATASIZE=`stat --printf="%s" $@`; \
	DATASIZE=`expr $$DATASIZE / 4096`; \
	DATASIZE=`expr 1 + $$DATASIZE`; \
	DATASIZE=`expr 4096 \* $$DATASIZE`; \
	dd if=/dev/null of=$@ bs=$$DATASIZE seek=1

objects/read/data.bin: $(FNAME_READ).elf
	$(OBJCOPY) -O binary -j .data $< $@
	DATASIZE=`stat --printf="%s" $@`; \
	DATASIZE=`expr $$DATASIZE / 4096`; \
	DATASIZE=`expr 1 + $$DATASIZE`; \
	DATASIZE=`expr 4096 \* $$DATASIZE`; \
	dd if=/dev/null of=$@ bs=$$DATASIZE seek=1

objects/create/%.page: $(FNAME_CREATE).elf
	$(OBJCOPY) -O binary -j .$(shell basename $@ .page) $< $@
	@dd if=/dev/null of=$@ bs=8192 seek=1

objects/read/%.page: $(FNAME_READ).elf
	$(OBJCOPY) -O binary -j .$(shell basename $@ .page) $< $@
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(FNAME_CREATE): $(FNAME_CREATE).bin
	python3 $(XDM99) -T $< -t -f PROGRAM -n $(FNAME_CREATE) -o $(FNAME_CREATE)
	mkdir -p ../../../FC/BIN && cp $(FNAME_CREATE) ../../../FC/BIN/

$(FNAME_READ): $(FNAME_READ).bin
	python3 $(XDM99) -T $< -t -f PROGRAM -n $(FNAME_READ) -o $(FNAME_READ)
	mkdir -p ../../../FC/BIN && cp $(FNAME_READ) ../../../FC/BIN/

objects/create/_header_create.o: _header_create.asm
	mkdir -p objects/create
	cd objects/create; $(GAS) $(abspath $<) -o $(notdir $@)

objects/read/_header_read.o: _header_read.asm
	mkdir -p objects/read
	cd objects/read; $(GAS) $(abspath $<) -o $(notdir $@)

# Bank 0 is the default for each build.
objects/create/%.o: CURRBANK=0
objects/read/%.o: CURRBANK=0

# Create build bank map: main/catalog/data/file = 0, create = 1,
# legacy create = 2, writer/encoder = 3.
objects/create/b3_create.o: CURRBANK=1
objects/create/b4_legacy.o: CURRBANK=2
objects/create/b7_create_file.o: CURRBANK=3

# Read build bank map: main/data/file = 0, read = 1, legacy read = 2,
# FCARC extract = 3, FCARC test = 4.
objects/read/b1_read.o: CURRBANK=1
objects/read/b2_legacy.o: CURRBANK=2
objects/read/b5_fcarc.o: CURRBANK=3
objects/read/b6_test.o: CURRBANK=4

objects/create/%.o: %.c $(wildcard *.h)
	mkdir -p objects/create
	cd objects/create; $(CC) -c $(abspath $<) $(CFLAGS) -DFCARC_CREATE_BUILD -DSAMS_CURRENT_BANK=$(CURRBANK) -o $(notdir $@)

objects/read/%.o: %.c $(wildcard *.h)
	mkdir -p objects/read
	cd objects/read; $(CC) -c $(abspath $<) $(CFLAGS) -DFCUNARC_READ_BUILD -DSAMS_CURRENT_BANK=$(CURRBANK) -o $(notdir $@)

clean:
	rm -rf objects
	rm -f *.elf *.bin mapfile $(FNAME_CREATE) $(FNAME_READ)
