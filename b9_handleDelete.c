#include "banks.h"
#define MYBANK BANK(9)

#include "commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include <string.h>

static void onIgnoreVolInfo(struct VolInfo *volInfo);
static void onDeleteDirEntry(struct DirEntry *dirEntry);

static struct DeviceServiceRoutine *srcdsr;
static char* srcpath;

static int filecount;
static int matched;
static int yes;

void handleDelete() {
  bk_disable_more();

  filecount = 0;
  matched = 0;
  srcdsr = 0;

  // allocate some buffer on the stack, set the global pointers.
  char tmp[256];
  srcpath = tmp;

  for(int i = 0; i<256; i++) {
    srcpath[i] = 0;
  }

  // check for /y option
  char *peek = bk_strtokpeek(0, ' ');
  yes = 0 == bk_strcmpi(str2ram("/y"), peek);
  if (yes) {
    bk_strtok(0, ' '); // consume the optional /y
  }

  bk_parsePathParam(0, &srcdsr, srcpath, PR_OPTIONAL | PR_WILDCARD);
  if (srcdsr == 0)
  {
    tputs_rom("error, no device found.\n");
    return;
  }

  // ensure devices are a device path
  if (srcpath[bk_strlen(srcpath)-1] != '.') {
    bk_strcat(srcpath, str2ram("."));
  }

  loadDir(srcdsr, srcpath, onIgnoreVolInfo, onDeleteDirEntry);
  if (matched == 0) {
    tputs_rom("error, no matching file found.\n");
  } else {
    tputs_rom("deleted ");
    bk_tputs_ram(bk_uint2str(filecount));
    tputs_rom(" files.\n");
  }
}

int keyYesNo() {
  int k = 0;
  while(k != 'y' && k != 'Y' && k != 'N' && k != 'n') {
    k = bk_cgetc();
  }
  tputs_rom("\n");
  return k == 'y' || k == 'Y';
}

static void onIgnoreVolInfo(struct VolInfo *volInfo) {
  return;
}

static void onDeleteDirEntry(struct DirEntry *dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  matched = 1;

  if (yes) {
    tputs_rom("deleting ");
    bk_tputs_ram(srcpath);
    bk_tputs_ram(dirEntry->name);
    bk_tputc('\n');
  } else {
    // prompt to continue
    tputs_rom("delete file ");
    bk_tputs_ram(srcpath);
    bk_tputs_ram(dirEntry->name);
    tputs_rom("? (y/n) ");
    int yn = keyYesNo();
    if (!yn) {
      return;
    }
  }

  char path[256];
  bk_strcpy(path, srcpath);
  bk_strcat(path, dirEntry->name);

  struct PAB pab;
  bk_initPab(&pab);
  pab.pName = path;

  unsigned int err = bk_dsr_delete(srcdsr, &pab);
  if (err)
  {
    tputs_rom("cannot delete file ");
    bk_tputs_ram(path);
    bk_tputc('\n');
  }

  filecount++;
}
