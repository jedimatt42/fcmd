#!/bin/sh

rm -f FCMD.ea5 FCMD
rm -f LOAD.xb LOAD

XAS99=`which xas99.py`
XDM99=`which xdm99.py`
XBAS99=`which xbas99.py`

# FCMD - (EA5) causes FinalGROM99 to reload Force Command module
python3 ${XAS99} -i -R -L assemble.lst -o FCMD.ea5 reload_fcmd.asm
python3 ${XDM99} -T FCMD.ea5 -t -f PROGRAM -n FCMD -o FCMD

# FCMDXB - (XB) causes FinalGROM99 to reload Force Command module
python3 ${XAS99} --embed-xb -R -L assemblexb.lst -o FCMD.xb reload_fcmd.asm
python3 ${XDM99} -T FCMD.xb -t -f "INT/VAR 254" -n FCMDXB -o FCMDXB

# LOAD - (XB) auto load entry point for Force Command XB command
python3 ${XBAS99} -c -o LOAD.xb loadxb.bas
python3 ${XDM99} -T LOAD.xb -t -f PROGRAM -n LOAD -o LOAD

