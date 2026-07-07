#include "banks.h"
#define MYBANK BANK(0)

#include "commands.h"
#include "honk.h"

int handleTipiBeeps() {
    bk_playtipi();
  return 0;
}