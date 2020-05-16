#!/bin/bash

rm FCMD.RPK 
python ./2meg.py
zip FCMD.RPK layout.xml FCMD2MEG.bin

