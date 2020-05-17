#!/bin/bash

# makefile supports concurrency.
function build_all() {
  make clean
  make -j 12
  ./makerpk.sh
}

time build_all
