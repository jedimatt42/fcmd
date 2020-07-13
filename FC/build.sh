#!/bin/sh

rm -f FCMD.ea5 FCMD
rm -f LOAD.xb LOAD

# FCMD - (EA5) causes FinalGROM99 to reload Force Command module
xas99.py -i -R -L assemble.lst -o FCMD.ea5 reload_fcmd.asm
xdm99.py -T FCMD.ea5 -t -f PROGRAM -n FCMD -o FCMD

# FCMDXB - (XB) causes FinalGROM99 to reload Force Command module
xas99.py --embed-xb -R -L assemblexb.lst -o FCMD.xb reload_fcmd.asm
xdm99.py -T FCMD.xb -t -f "INT/VAR 254" -n FCMDXB -o FCMDXB

# LOAD - (XB) auto load entry point for Force Command XB command
xbas99.py -c -o LOAD.xb loadxb.bas
xdm99.py -T LOAD.xb -t -f PROGRAM -n LOAD -o LOAD

