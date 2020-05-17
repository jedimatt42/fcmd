#!/bin/bash

rm -f FCMD.RPK 
python ./2meg.py
zip FCMD.RPK layout.xml FCMD2MEG.bin

