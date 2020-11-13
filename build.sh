#!/bin/bash

# makefile supports concurrency.
function build_all() {
  make clean
  make -j 12
  cp FCMD8.bin FCMDC.bin
  ./makerpk.sh
  echo "---- Creating distribution ----"
  ( cd FC; ./build.sh )
  VER=$(grep "#define APP_VER" b0_main.h | cut -d'"' -f2)
  rm -f forcecmd*.zip
  zip forcecmd_${VER}.zip FCMDG.bin FCMDC.bin *.RPK README.md FC/LOAD FC/FCMD FC/FCMDXB FC/BIN/*
}

time build_all

cp FCMD*.bin ~/Documents/data/retro/TI99/
cp *.RPK ~/Documents/data/retro/TI99/
cp example/gcc/ftp/FTP ~/Documents/data/retro/TI99/classic99/DSK1/
scp example/gcc/ftp/FTP tipi@tipi4a:/home/tipi/tipi_disk/FC/BIN/
cp example/gcc/hello/HELLO ~/Documents/data/retro/TI99/classic99/DSK1/
cp example/gcc/charset/CHARSET ~/Documents/data/retro/TI99/classic99/DSK1/
cp example/gcc/ntscpal/NTSCPAL ~/Documents/data/retro/TI99/classic99/DSK1/

