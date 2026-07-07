#include "banks.h"
#define MYBANK BANK(9)

#include "commands.h"
#include "globals.h"
#include "parsing.h"
#include "strutil.h"
#include "dsrutil.h"
#include "lvl2.h"
#include "heap.h"
#include "terminal.h"
#include "getstr.h"
#include <string.h>

static void onIgnoreVolInfo(struct VolInfo *volInfo);
static void onDeleteDirEntry(struct DirEntry *dirEntry);
static int deleteQueue(char* filename);

static struct DeviceServiceRoutine *srcdsr;
static char* srcpath;

static int filecount;
static int matched;
static int yes;

struct MatchedName {
  char name[11];
};

static struct MatchedName* names;

int handleDelete() {
  filecount = 0;
  matched = 0;
  srcdsr = 0;
  names = 0;

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
  return 0;
  }

  // ensure devices are a device path
  if (srcpath[bk_strlen(srcpath)-1] != '.') {
    bk_strcat(srcpath, str2ram("."));
  }

  dsr_catalog(srcdsr, srcpath, onIgnoreVolInfo, onDeleteDirEntry);
  if (matched == 0) {
    tputs_rom("error, no matching file found.\n");
  } else {
    for(int idx = 0; idx < matched; idx++) {
      deleteQueue((names + idx)->name);
    }

    if (names) {
      bk_free((char*)names);
    }

    tputs_rom("deleted ");
    bk_tputs_ram(bk_uint2str(filecount));
    tputs_rom(" files.\n");
  }
  return 0;
}

int keyYesNo() {
  int k = 0;
  while(k != 'y' && k != 'Y' && k != 'N' && k != 'n') {
    k = bk_cgetc(CUR_INSERT);
  }
  tputs_rom("\n");
  return k == 'y' || k == 'Y';
}

static void onIgnoreVolInfo(struct VolInfo *volInfo) {
}

static void onDeleteDirEntry(struct DirEntry *dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
  return;
  }

  struct MatchedName* nameMatch = (struct MatchedName*) bk_alloc(sizeof(struct MatchedName));
  if (nameMatch == 0) {
    bk_tputs_ram("out of heap, skipping ");
    bk_tputs_ram(dirEntry->name);
    bk_tputc('\n');
  return;
  }
  if (names == 0) {
    names = nameMatch;
  }

  bk_strcpy(nameMatch->name, dirEntry->name);
  matched++;
}

static int deleteQueue(char* filename) {
  char path[256];
  bk_strcpy(path, srcpath);
  bk_strcat(path, filename);

  if (yes) {
    tputs_rom("deleting ");
    bk_tputs_ram(path);
    bk_tputc('\n');
  } else {
    // prompt to continue
    tputs_rom("delete file ");
    bk_tputs_ram(path);
    tputs_rom("? (y/n) ");
    int yn = keyYesNo();
    if (!yn) {
  return 0;
    }
  }

  struct PAB pab;
  bk_initPab(&pab);
  pab.pName = path;

  unsigned int err = bk_dsr_delete(srcdsr, &pab);
  if (err)
  {
    tputs_rom("cannot delete file ");
    bk_tputs_ram(path);
    bk_tputc('\n');
  } else {
    filecount++;
  }
  return 0;
}
