#!/bin/sh

# FCMD - (EA5) causes FinalGROM99 to reload Force Command module
rm -f FCMD*
xas99.py -i -R -L assemble.lst -o FCMD.ea5 reload_fcmd.asm
xdm99.py -T FCMD.ea5 -t -f PROGRAM -n FCMD -o FCMD

# LOAD - (XB) auto load entry point for Force Command XB command
xbas99.py -c -o LOAD.xb loadxb.bas
xdm99.py -T LOAD.xb -t -f PROGRAM -n LOAD -o LOAD

