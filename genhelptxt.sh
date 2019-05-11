#!/bin/bash

echo "[code]"
egrep "(wraptext|if)" b5_handleHelp.c | egrep -v "(void|define|HELP)" \
  | sed 's/^.*if (.*$/[\/code]\n\n[code]/' | sed 's/^.*wraptext("//' | sed 's/\\n");$//' \
  | sed 's/^\\n/\n/'

echo "[/code]"
