#include "banks.h"
#define MYBANK BANK_2

#include "commands.h"
#include "b0_globals.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b1cp_terminal.h"
#include <string.h>

void handleCopy() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    tputs("error, must specify a file name\n");
    return;
  }
  char path[256];
  // check that file exists
  strcpy(path, currentPath);
  strcat(path, filename);
  if (0 != existsFile(currentDsr, path)) {
    tputs("error, file not found: ");
    tputs(currentPath);
    tputs(filename);
    tputc('\n');
    return;
  }

  struct DeviceServiceRoutine* dsr = 0;
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs("no path: drive or folder specified\n");
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

  struct AddInfo* addInfoPtr = (struct AddInfo*) 0x8320;
  addInfoPtr->first_sector = 0;
  addInfoPtr->eof_offset = 0;
  addInfoPtr->flags = 0;
  addInfoPtr->rec_length = 0;
  addInfoPtr->records = 0;
  addInfoPtr->recs_per_sec = 0;
  
  unsigned int source_crubase = currentDsr->crubase;
  unsigned char source_unit = path2unitmask(currentPath);
  unsigned int dest_crubase = dsr->crubase;
  unsigned char dest_unit = path2unitmask(path);

  lvl2_setdir(source_crubase, source_unit, currentPath);
  unsigned int err = lvl2_input(source_crubase, source_unit, filename, 0, addInfoPtr);
  if (err) {
    tputs("error reading file: ");
    tputs(uint2hex(err));
    tputc('\n');
    return;
  }

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    tputs("error, source file is empty.\n");
    return;
  }

  tputc('\n'); // prepare status line

  // write file meta data
  lvl2_setdir(dest_crubase, dest_unit, path);
  err = lvl2_output(dest_crubase, dest_unit, filename, 0, addInfoPtr);
  if (err) {
    tputs("error writing file: ");
    tputs(uint2hex(err));
    tputc('\n');
    return;
  }

  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;
    lvl2_setdir(source_crubase, source_unit, currentPath);
    err = lvl2_input(source_crubase, source_unit, filename, 1, addInfoPtr);
    if (err) {
      tputs("\nerror reading file: ");
      tputs(uint2hex(err));
      tputc('\n');
      return;
    }

    lvl2_setdir(dest_crubase, dest_unit, path);
    err = lvl2_output(dest_crubase, dest_unit, filename, 1, addInfoPtr);
    if (err) {
      tputs("\nerror reading file: ");
      tputs(uint2hex(err));
      tputc('\n');
      return;
    }

    blockId++;
    tputs("\rcopied block ");
    tputs(uint2str(blockId));
    tputs(" of ");
    tputs(uint2str(totalBlocks));
  }
  tputc('\n'); // end status line.
}
