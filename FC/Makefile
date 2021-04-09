XAS99?=$(shell which xas99.py)
XDM99?=$(shell which xdm99.py)
XBAS99?=$(shell which xbas99.py)

all: FCMD FCMDXB LOAD

.phony clean:
	rm -fr LOAD.xb LOAD
	rm -fr FCMD.xb FCMDXB
	rm -fr FCMD.ea5 FCMD

FCMD: reload_fcmd.asm
	python3 $(XAS99) -i -R -L assemble.lst -o FCMD.ea5 reload_fcmd.asm
	python3 $(XDM99) -T FCMD.ea5 -t -f PROGRAM -n FCMD -o FCMD

FCMDXB: reload_fcmd.asm
	python3 $(XAS99) --embed-xb -R -L assemblexb.lst -o FCMD.xb reload_fcmd.asm
	python3 $(XDM99) -T FCMD.xb -t -f "INT/VAR 254" -n FCMDXB -o FCMDXB

LOAD: loadxb.bas
	python3 $(XBAS99) -c -o LOAD.xb loadxb.bas
	python3 $(XDM99) -T LOAD.xb -t -f PROGRAM -n LOAD -o LOAD

