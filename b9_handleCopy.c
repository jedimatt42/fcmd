#include "banks.h"
#define MYBANK BANK(9)

#include "commands.h"
#include "b0_globals.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include "b5_copySingleFile.h"
#include <string.h>

static void onIgnoreVolInfo(struct VolInfo *volInfo);
static void onCopyDirEntry(struct DirEntry *dirEntry);

static struct DeviceServiceRoutine *srcdsr;
static char* srcpath;

static struct DeviceServiceRoutine *dstdsr;
static char* dstpath;

static int copycount;
static int matched;

static void copyMultipleFiles();
static void copyOneFile();

void handleCopy() {
  copycount = 0;
  matched = 0;
  srcdsr = 0;
  dstdsr = 0;

  // allocate some buffer on the stack, set the global pointers.
  char srcn[256];
  char dstn[256];
  srcpath = srcn;
  dstpath = dstn;

  for(int i = 0; i<256; i++) {
    srcpath[i] = 0;
    dstpath[i] = 0;
  }

  char tmpsrc[256];
  bk_strcpy(tmpsrc, bk_strtok(0, ' '));

  // parse destination first, so glob pattern is preserved on source.
  bk_parsePathParam(0, &dstdsr, dstpath, PR_OPTIONAL);
  if (dstdsr == 0)
  {
    tputs_rom("no path: drive or folder specified\n");
    return;
  }

  // parse source and set glob pattern
  setstrtok(tmpsrc);
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

  // maybe take a turn off and handle single file copy 
  // special...
  if (-1 == bk_indexof(filterglob, '*')) {
    copyOneFile();
  } else {
    copyMultipleFiles();
  }
}

static void copyOneFile() {
  if (dstpath[bk_strlen(dstpath) - 1] != '.') {
    bk_strcat(dstpath, str2ram("."));
  }

  unsigned int stat = bk_existsDir(dstdsr, dstpath);
  if (stat != 0) {
    tputs_rom("error, device/folder not found: ");
    bk_tputs_ram(dstpath);
    bk_tputc('\n');
    return;
  }

  bk_copySingleFile(srcdsr, srcpath, filterglob, dstdsr, dstpath);
}

static void copyMultipleFiles() {
  if (dstpath[bk_strlen(dstpath) - 1] != '.') {
    bk_strcat(dstpath, str2ram("."));
  }

  unsigned int stat = bk_existsDir(dstdsr, dstpath);
  if (stat != 0) {
    tputs_rom("error, device/folder not found: ");
    bk_tputs_ram(dstpath);
    bk_tputc('\n');
    return;
  }

  loadDir(srcdsr, srcpath, onIgnoreVolInfo, onCopyDirEntry);
  if (matched == 0) {
    tputs_rom("error, no matching file found.\n");
  } else {
    tputs_rom("copied ");
    bk_tputs_ram(bk_uint2str(copycount));
    tputs_rom(" files.\n");
  }
}

static void onIgnoreVolInfo(struct VolInfo *volInfo) {
  return;
}

static void onCopyDirEntry(struct DirEntry *dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  matched = 1;

  bk_copySingleFile(srcdsr, srcpath, dirEntry->name, dstdsr, dstpath);

  copycount++;
}
