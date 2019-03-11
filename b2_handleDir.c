#include "banks.h"
#define MYBANK BANK_2

#include "commands.h"


#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b1cp_terminal.h"
#include <string.h>

void onLongVolInfo(struct VolInfo* volInfo);
void onLongDirEntry(struct DirEntry* dirEntry);

void onWideVolInfo(struct VolInfo* volInfo);
void onWideDirEntry(struct DirEntry* dirEntry);

static int col = 0;

void handleDir() {
  struct DeviceServiceRoutine* dsr = 0;

  char* peek = strtokpeek(0, " ");
  int wideFormat = 0 == strcmpi("/W", peek);
  if (wideFormat) {
    strtok(0, " "); // consume the option token.
  }

  char path[256];
  bk_parsePathParam(&dsr, path, PR_OPTIONAL);
  if (dsr == 0) {
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }

  unsigned int stat = existsDir(dsr, path);
  if (stat != 0) {
    tputs("error, device/folder not found: ");
    tputs(path);
    tputc('\n');
    return;
  }

  if (wideFormat) {
    col = 0;
    loadDir(dsr, path, onWideVolInfo, onWideDirEntry);
    tputc('\n');
  } else {
    loadDir(dsr, path, onLongVolInfo, onLongDirEntry);
  }
  tputc('\n');
}

void onLongVolInfo(struct VolInfo* volInfo) {
  tputs("Diskname: ");
  tputs(volInfo->volname);
  if (displayWidth == 40) {
    tputc('\n');
  } else {
    tputc(' ');
  }
  tputs("Available: ");
  tputs(uint2str(volInfo->available));
  tputs(" Used: ");
  tputs(uint2str(volInfo->total - volInfo->available));
  tputs("\n\n");
  tputs("Name       Type    Reclen Sectors\n");
  tputs("---------------------------------\n");
}

const char* file_types[] = {
  "DIS/FIX",
  "DIS/VAR",
  "INT/FIX",
  "INT/VAR",
  "PROGRAM",
  "DIR"
};

void onLongDirEntry(struct DirEntry* dirEntry) {
  tputs(dirEntry->name);
  cputpad(11, dirEntry->name);

  char* ftype = (char*) file_types[(dirEntry->type)-1];
  
  tputs(ftype);
  cputpad(8, ftype);

  if (dirEntry->type < 5) {
    char* sizestr = uint2str(dirEntry->reclen);
    tputs(sizestr);
    cputpad(7, sizestr);
  } else {
    cputpad(7, "");
  }

  tputs(uint2str(dirEntry->sectors));

  tputc('\n');
}

void onWideVolInfo(struct VolInfo* volInfo) {
  // do nothing, we don't want volume info in wide listing
}

void onWideDirEntry(struct DirEntry* dirEntry) {
  int collimit = displayWidth / 11;
  tputs(dirEntry->name);
  col++;
  if (col < collimit) {
    cputpad(11, dirEntry->name);
  } else {
    col = 0;
    tputc('\n');
  }
}
