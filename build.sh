#!/bin/bash

# makefile supports concurrency.
function build_all() {
  make clean
  make -j 12
  ./makerpk.sh
  echo "---- Creating distribution ----"
  ( cd FC; ./build.sh )
  VER=$(grep "#define APP_VER" b0_main.h | cut -d'"' -f2)
  rm -f forcecmd*.zip
  zip forcecmd_${VER}.zip FCMDG.bin FCMDC.bin README.md FC/LOAD FC/FCMD FC/FCMDXB
}

time build_all

cp FCMD*.bin ~/Documents/data/retro/TI99/
