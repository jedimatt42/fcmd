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

/*
 want to be able to handle:
   dir 1100.TIPI.GAMES
   dir
   dir F*
   dir TIPI.GAMES.P*
*/
void handleDir() {
  struct DeviceServiceRoutine* dsr = 0;

  char* peek = strtokpeek(0, " ");
  int wideFormat = 0 == strcmpi("/W", peek);
  if (wideFormat) {
    strtok(0, " "); // consume the option token.
  }

  char path[256];
  bk_parsePathParam(&dsr, path, PR_OPTIONAL | PR_WILDCARD);
  if (dsr == 0) {
    tputs_rom("error, no device found.\n");
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }

  unsigned int stat = existsDir(dsr, path);
  if (stat != 0) {
    tputs_rom("error, device/folder not found: ");
    tputs_ram(path);
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
  tputs_rom("Diskname: ");
  tputs_ram(volInfo->volname);
  if (displayWidth == 40) {
    tputc('\n');
  } else {
    tputc(' ');
  }
  tputs_rom("Used: ");
  tputs_ram(uint2str(volInfo->total - volInfo->available));
  tputs_rom(" Available: ");
  tputs_ram(uint2str(volInfo->available));
  tputs_rom("\n\n");
  tputs_rom("Name       Type    P Reclen Sectors\n");
  tputs_rom("-----------------------------------\n");
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
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  tputs_ram(dirEntry->name);
  cputpad(11, dirEntry->name);

  int de_type = dirEntry->type < 0 ? -1 * dirEntry->type : dirEntry->type;
  de_type--;

  char* ftype = (char*) file_types[de_type];

  tputs_rom(ftype);
  cputpad(8, ftype);

  if (dirEntry->type < 0) {
    tputc('P');
  } else {
    tputc(' ');
  }
  tputc(' ');

  if (de_type >= 5) { // is program or dir? skip record details.
    cputpad(7, "");
  } else {
    char* sizestr = uint2str(dirEntry->reclen);
    tputs_ram(sizestr);
    cputpad(7, sizestr);
  }

  tputs_ram(uint2str(dirEntry->sectors));

  tputc('\n');
}

void onWideVolInfo(struct VolInfo* volInfo) {
  // do nothing, we don't want volume info in wide listing
}

void onWideDirEntry(struct DirEntry* dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  int collimit = displayWidth / 11;
  tputs_ram(dirEntry->name);
  col++;
  if (col < collimit) {
    cputpad(11, dirEntry->name);
  } else {
    col = 0;
    tputc('\n');
  }
}
