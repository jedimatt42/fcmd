#!/bin/bash

rm -f *.RPK
# good for MAME 225 and up
zip FCMD.RPK layout.xml FCMD8.bin FCMDG.bin
## For MAME 224
# create an full 512k ROM
python3 ./512.py
mkdir tmp
cd tmp
mv ../FCMD512.bin .
cp ../layout-224.xml ./layout.xml
zip ../FCMD-MAME-224.RPK layout.xml FCMD512.bin

