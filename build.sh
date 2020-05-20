#!/bin/bash

# makefile supports concurrency.
function build_all() {
  make clean
  make -j 12
  ./makerpk.sh
  VER=$(grep "#define APP_VER" b0_main.c | cut -d'"' -f2)
  rm -f forcecmd*.zip
  zip forcecmd_${VER}.zip FCMDG.bin FCMDC.bin README.md
}

time build_all


