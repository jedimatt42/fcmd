#!/bin/bash

rm -f FCMD.RPK 
cp js99er-layout.xml layout.xml
zip FCMD.RPK layout.xml FCMD8.bin FCMDG.bin
rm -f MAME-FCMD.RPK
cp mame-layout.xml layout.xml
zip MAME-FCMD.RPK layout.xml FCMD8.bin

