#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include <conio.h>
#include <string.h>

static void listDrives() {
  char namebuf[14];
  strcpy(namebuf, "DSK1");

  struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);

  struct PAB pab;
  bk_initPab(&pab);

  strcpy(namebuf, "DSK1.CONFIG");

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
  
  if (err) {
    cputs("could no open PI.CONFIG\n");
    return;
  }
  while(!err) {
    err = bk_dsr_read(dsr, &pab, 0);
    if (!err) {
      // print the line...
      char linebuf[256];
      vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);
      linebuf[pab.CharCount] = 0;

      char* key = strtok(linebuf, "=");
      if (strlen(key) > 3 && str_startswith(key, "DSK")) {
        char* path = strtok(0, " ");
        if (path) {
          char* drive = strtok(key, "_");
          cputs(drive);
          cputs(". => TIPI.");
          cputs(path);
        }
        cputc('\n');
      }
    }
  }
  bk_dsr_close(dsr, &pab);
}

void handleTipimap() {
  //  [/c] [drive] [path]
  char* peek = strtokpeek(0, " ");
  int clear = 0 == strcmpi("/C", peek);
 
  if (clear) {
    strtok(0, " "); // consume the optional /c
  }

  char* drive = strtok(0, " ");
  if (drive == 0) {
    if (clear) {
      cputs("error, must specify drive to with /c\n");
    } else {
      listDrives();
    }
  }
  cputc('\n');
}