#!/usr/bin/env bash

# Find the location of this script
SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
SCRIPT_DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
# Set the current directory to that of the script
cd $SCRIPT_DIR


if [ -z ${TIPI_DIR:-} ]; then
  echo "export TIPI_DIR to the path of your TIPI disk (for example /media/tipi/tipi_disk) to copy the FC support files to \$TIPI_DIR/FC"
else
  # Copy the support files onto the TIPI where they are expected to be.
  # some functionality depends on them being in the FC folder.
  mkdir -p "$TIPI_DIR/FC/BIN"
  cp -a FC/BOOT FC/LOAD FC/FCMD FC/FCMDXB "$TIPI_DIR/FC/"
  cp -a FC/BIN/. "$TIPI_DIR/FC/BIN/"
  echo "Copied FC support files to $TIPI_DIR/FC"
fi

