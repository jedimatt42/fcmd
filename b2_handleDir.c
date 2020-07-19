#include "banks.h"
#define MYBANK BANK(2)

#include "commands.h"


#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b8_terminal.h"
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
  int wideFormat = bk_strcmpi(str2ram("/w"), peek) == 0 ? 1 : 0;
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
    bk_tputs_ram(path);
    bk_tputc('\n');
    return;
  }

  if (wideFormat) {
    col = 0;
    loadDir(dsr, path, onWideVolInfo, onWideDirEntry);
    bk_tputc('\n');
  } else {
    loadDir(dsr, path, onLongVolInfo, onLongDirEntry);
  }
  bk_tputc('\n');
}

void onLongVolInfo(struct VolInfo* volInfo) {
  tputs_rom("Diskname: ");
  bk_tputs_ram(volInfo->volname);
  if (displayWidth == 40) {
    bk_tputc('\n');
  } else {
    bk_tputc(' ');
  }
  tputs_rom("Used: ");
  bk_tputs_ram(uint2str(volInfo->total - volInfo->available));
  tputs_rom(" Available: ");
  bk_tputs_ram(uint2str(volInfo->available));
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

void cputpad(int padding, char *str)
{
  int c = 0;
  for (int i = 0; i < padding; i++)
  {
    if (c == 0 && str[i] == 0)
    {
      c = 1;
    }
    if (c == 1)
    {
      bk_tputc(' ');
    }
  }
}

void onLongDirEntry(struct DirEntry* dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  bk_tputs_ram(dirEntry->name);
  cputpad(11, dirEntry->name);

  int de_type = dirEntry->type < 0 ? -1 * dirEntry->type : dirEntry->type;
  de_type--;

  char* ftype = (char*) file_types[de_type];

  tputs_rom(ftype);
  cputpad(8, ftype);

  if (dirEntry->type < 0) {
    bk_tputc('P');
  } else {
    bk_tputc(' ');
  }
  bk_tputc(' ');

  if (de_type >= 5) { // is program or dir? skip record details.
    cputpad(7, "");
  } else {
    char* sizestr = uint2str(dirEntry->reclen);
    bk_tputs_ram(sizestr);
    cputpad(7, sizestr);
  }

  bk_tputs_ram(uint2str(dirEntry->sectors));

  bk_tputc('\n');
}

void onWideVolInfo(struct VolInfo* volInfo) {
  // do nothing, we don't want volume info in wide listing
}

void onWideDirEntry(struct DirEntry* dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  int collimit = displayWidth / 11;
  bk_tputs_ram(dirEntry->name);
  col++;
  if (col < collimit) {
    cputpad(11, dirEntry->name);
  } else {
    col = 0;
    bk_tputc('\n');
  }
}
