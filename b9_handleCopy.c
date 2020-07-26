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

void onIgnoreVolInfo(struct VolInfo *volInfo);
void onCopyDirEntry(struct DirEntry *dirEntry);

struct DeviceServiceRoutine *srcdsr;
char* srcpath;

struct DeviceServiceRoutine *dstdsr;
char* dstpath;

int copycount;
int matched;

void handleCopy() {
  bk_disable_more();

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
  strcpy(tmpsrc, bk_strtok(0, ' '));

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
  if (srcpath[bk_strlen(srcpath)-1] != '.') {
    bk_strcat(srcpath, str2ram("."));
  }
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
    bk_tputs_ram(int2str(copycount));
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
  bk_tputs_ram(srcpath);
  bk_tputs_ram(dirEntry->name);
  tputs_rom(" to ");
  bk_tputs_ram(dstpath);
  bk_tputc('\n');

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
  bk_lvl2_setdir(source_crubase, source_unit, srcpath);
  unsigned int err = bk_lvl2_input(source_crubase, source_unit, dirEntry->name, 0, addInfoPtr);
  if (err) {
    tputs_rom("error reading file: ");
    bk_tputs_ram(bk_uint2hex(err));
    bk_tputc('\n');
    return;
  }

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    tputs_rom("error, source file is empty.\n");
    return;
  }

  // write file meta data
  bk_lvl2_setdir(dest_crubase, dest_unit, dstpath);
  err = bk_lvl2_output(dest_crubase, dest_unit, dirEntry->name, 0, addInfoPtr);
  if (err) {
    tputs_rom("error writing file: ");
    bk_tputs_ram(bk_uint2hex(err));
    bk_tputc('\n');
    return;
  }

  // copy all the data blocks
  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;

    // read a block
    bk_lvl2_setdir(source_crubase, source_unit, srcpath);
    err = bk_lvl2_input(source_crubase, source_unit, dirEntry->name, 1, addInfoPtr);
    if (err) {
      tputs_rom("\nerror reading file: ");
      bk_tputs_ram(bk_uint2hex(err));
      bk_tputc('\n');
      return;
    }

    // write it back out
    bk_lvl2_setdir(dest_crubase, dest_unit, dstpath);
    err = bk_lvl2_output(dest_crubase, dest_unit, dirEntry->name, 1, addInfoPtr);
    if (err) {
      tputs_rom("\nerror writing file: ");
      bk_tputs_ram(bk_uint2hex(err));
      bk_tputc('\n');
      return;
    }

    blockId++;
  }
  copycount++;
}
