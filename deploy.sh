#!/bin/bash

if [ -z ${CART_DEPLOY_DIR:-} ]; then
  echo "export CART_DEPLOY_DIR to directory you want to copy .bin and .rpk files to"
else
  # Copy cartridge binaries to a place Classic99.ini can reference them
  # that is convenient for also fetching for deployment to my FinalGROM99 cartridge
  cp FCMD*.bin $CART_DEPLOY_DIR
  # Copy the RPK cartridge binary somewhere convenient to upload to JS99ER
  cp *.RPK $CART_DEPLOY_DIR
fi

if [ -z ${CLASSIC99_DSK1_DIR:-} ]; then
  echo "export CLASSIC99_DSK1_DIR to copy utlities for emulator use"
else
  # Copy stuff to DSK1 for use with CLASSIC99 FIAD storage
  cp example/gcc/fcmenu/FCMENU $CLASSIC99_DSK1_DIR
  cp example/gcc/ftp/FTP $CLASSIC99_DSK1_DIR
  cp example/gcc/telnet/TELNET $CLASSIC99_DSK1_DIR
  cp example/gcc/hello/HELLO $CLASSIC99_DSK1_DIR
  cp example/gcc/charset/CHARSET $CLASSIC99_DSK1_DIR
  cp example/gcc/ntscpal/NTSCPAL $CLASSIC99_DSK1_DIR
  cp example/gcc/say/SAY $CLASSIC99_DSK1_DIR
  cp example/gcc/say/SAMPLE $CLASSIC99_DSK1_DIR
fi

if [ -z ${TIPI_HOST_NAME:-} ]; then
  echo "export TIPI_HOST_NAME to tipi.local or your appropriate host to copy support files to tipi disk"
else
  # Copy the support files onto my 4A's TIPI where they are expected to be.
  # some functionality depends on them being in the FC folder.
  scp FC/LOAD tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/LOAD
  scp FC/FCMD tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/FCMD
  scp FC/FCMDXB tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/FCMDXB
  scp example/gcc/fcmenu/FCMENU tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/BIN/
  scp example/gcc/ftp/FTP tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/BIN/
  scp example/gcc/telnet/TELNET tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/BIN/
  scp example/gcc/say/SAY tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/BIN/
  scp example/gcc/say/SAMPLE tipi@${TIPI_HOST_NAME}:/home/tipi/tipi_disk/FC/BIN/
fi

