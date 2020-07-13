#include "banks.h"
#define MYBANK BANK(2)

#include "commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b1cp_terminal.h"
#include <string.h>

void onIgnoreVolInfo(struct VolInfo *volInfo);
void onCopyDirEntry(struct DirEntry *dirEntry);

struct DeviceServiceRoutine *srcdsr;
char* srcpath;

struct DeviceServiceRoutine *dstdsr;
char* dstpath;

int copycount;
int matched;

void handleCopy() {
  disable_more();

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
  strcpy(tmpsrc, strtok(0, " "));

  // parse destination first, so glob pattern is preserved on source.
  bk_parsePathParam(&dstdsr, dstpath, PR_REQUIRED);
  if (dstdsr == 0)
  {
    tputs_rom("no path: drive or folder specified\n");
    return;
  }

  // parse source and set glob pattern
  setstrtok(tmpsrc);
  bk_parsePathParam(&srcdsr, srcpath, PR_OPTIONAL | PR_WILDCARD);
  if (srcdsr == 0)
  {
    tputs_rom("error, no device found.\n");
    return;
  }

  // ensure devices are a device path
  if (srcpath[strlen(srcpath)-1] != '.') {
    strcat(srcpath, ".");
  }
  if (dstpath[strlen(dstpath) - 1] != '.') {
    strcat(dstpath, ".");
  }

  unsigned int stat = existsDir(dstdsr, dstpath);
  if (stat != 0) {
    tputs_rom("error, device/folder not found: ");
    tputs_ram(dstpath);
    tputc('\n');
    return;
  }

  loadDir(srcdsr, srcpath, onIgnoreVolInfo, onCopyDirEntry);
  if (matched == 0) {
    tputs_rom("error, no matching file found.\n");
  } else {
    tputs_rom("copied ");
    tputs_ram(int2str(copycount));
    tputs_rom(" files.\n");
  }
}

void onIgnoreVolInfo(struct VolInfo *volInfo) {
  return;
}

void onCopyDirEntry(struct DirEntry *dirEntry) {
  if (!bk_globMatches(dirEntry->name)) {
    return;
  }
  matched = 1;

  tputs_rom("copying ");
  tputs_ram(srcpath);
  tputs_ram(dirEntry->name);
  tputs_rom(" to ");
  tputs_ram(dstpath);
  tputc('\n');

  struct AddInfo* addInfoPtr = (struct AddInfo*) 0x8320;
  addInfoPtr->first_sector = 0;
  addInfoPtr->eof_offset = 0;
  addInfoPtr->flags = 0;
  addInfoPtr->rec_length = 0;
  addInfoPtr->records = 0;
  addInfoPtr->recs_per_sec = 0;

  unsigned int source_crubase = srcdsr->crubase;
  unsigned int source_unit = path2unitmask(srcpath);
  unsigned int dest_crubase = dstdsr->crubase;
  unsigned int dest_unit = path2unitmask(dstpath);

  // get file meta data
  lvl2_setdir(source_crubase, source_unit, srcpath);
  unsigned int err = lvl2_input(source_crubase, source_unit, dirEntry->name, 0, addInfoPtr);
  if (err) {
    tputs_rom("error reading file: ");
    tputs_ram(uint2hex(err));
    tputc('\n');
    return;
  }

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    tputs_rom("error, source file is empty.\n");
    return;
  }

  // write file meta data
  lvl2_setdir(dest_crubase, dest_unit, dstpath);
  err = lvl2_output(dest_crubase, dest_unit, dirEntry->name, 0, addInfoPtr);
  if (err) {
    tputs_rom("error writing file: ");
    tputs_ram(uint2hex(err));
    tputc('\n');
    return;
  }

  // copy all the data blocks
  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;

    // read a block
    lvl2_setdir(source_crubase, source_unit, srcpath);
    err = lvl2_input(source_crubase, source_unit, dirEntry->name, 1, addInfoPtr);
    if (err) {
      tputs_rom("\nerror reading file: ");
      tputs_ram(uint2hex(err));
      tputc('\n');
      return;
    }

    // write it back out
    lvl2_setdir(dest_crubase, dest_unit, dstpath);
    err = lvl2_output(dest_crubase, dest_unit, dirEntry->name, 1, addInfoPtr);
    if (err) {
      tputs_rom("\nerror writing file: ");
      tputs_ram(uint2hex(err));
      tputc('\n');
      return;
    }

    blockId++;
  }
  copycount++;
}
