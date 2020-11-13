#!/bin/bash

rm -f JS99ER-FCMD.RPK 
cp js99er-layout.xml layout.xml
zip JS99ER-FCMD.RPK layout.xml FCMD8.bin FCMDG.bin
rm -f MAME-FCMD.RPK
python ./512.py
cp mame-layout.xml layout.xml
zip MAME-FCMD.RPK layout.xml FCMD512.bin

