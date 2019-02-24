#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"


#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"

#include <conio.h>
#include <string.h>

static void onLongVolInfo(struct VolInfo* volInfo);
static void onLongDirEntry(struct DirEntry* dirEntry);

void handleDir() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_OPTIONAL);
  if (dsr == 0) {
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }

  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cputs("error, device/folder not found: ");
    cputs(path);
    cputc('\n');
    return;
  }

  loadDir(dsr, path, onLongVolInfo, onLongDirEntry);
  cputc('\n');
}

static void onLongVolInfo(struct VolInfo* volInfo) {
  cputs("Diskname: ");
  cputs(volInfo->volname);
  cputs(" Available: ");
  cputs(uint2str(volInfo->available));
  cputs(" Used: ");
  cputs(uint2str(volInfo->total - volInfo->available));
  cputs("\n\n");
  cputs("Name       Size Type\n");
  cputs("--------------------\n");
}

const char* file_types[] = {
  "D/F",
  "D/V",
  "I/F",
  "I/V",
  "PRG",
  "DIR"
};

static void onLongDirEntry(struct DirEntry* dirEntry) {
  cputs(dirEntry->name);
  for(int i=strlen(dirEntry->name); i<11; i++) {
    cputc(' ');
  }

  char* sizestr = 0;
  if (dirEntry->type < 5) {
    sizestr = uint2str(dirEntry->reclen);
  } else if (dirEntry->type == 5) {
    sizestr = uint2str(dirEntry->sectors);
  }
  if (sizestr != 0) {
    for(int i=strlen(sizestr); i<4; i++) {
      cputc(' ');
    }
    cputs(sizestr);
  } else {
    cputs("    ");
  }
  cputc(' ');
  cputs(file_types[(dirEntry->type)-1]);
  cputc('\n');
}

