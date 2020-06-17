#include "banks.h"
#define MYBANK BANK_4

#include "commands.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <vdp.h>

void handleChecksum() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no file path name specified\n");
    return;
  }

  unsigned int unit = bk_path2unitmask(path);

  int parent_idx = lindexof(path, '.', strlen(path) - 1);
  char filename[11];
  strncpy(filename, path + parent_idx, 11);
  path[parent_idx + 1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, unit, path);

  struct AddInfo* addInfoPtr = (struct AddInfo*) 0x8320;
  addInfoPtr->first_sector = 0;
  addInfoPtr->eof_offset = 0;
  addInfoPtr->flags = 0;
  addInfoPtr->rec_length = 0;
  addInfoPtr->records = 0;
  addInfoPtr->recs_per_sec = 0;

  unsigned int err = bk_lvl2_input(dsr->crubase, unit, filename, 0, addInfoPtr);
  if (err) {
    tputs_rom("error reading file: ");
    tputs_ram(uint2hex(err));
    tputc('\n');
    return;
  }

  unsigned char eof_offset = addInfoPtr->eof_offset;

  int sum1 = 0;
  int sum2 = 0;

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    tputs_rom("error, source file is empty.\n");
    return;
  }
  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;
    err = bk_lvl2_input(dsr->crubase, unit, filename, 1, addInfoPtr);
    if (err) {
      tputs_rom("error reading file: ");
      tputs_ram(uint2hex(err));
      tputc('\n');
      return;
    }

    unsigned char maxByte = 255;
    if (blockId+1 == totalBlocks) {
      maxByte = eof_offset == 0 ? 255 : eof_offset;
    }

    for(int i=0; i<= maxByte; i++) {
      sum1 = (sum1 + vdpreadchar(addInfoPtr->buffer + i)) & 0xFF;
      sum2 = (sum2 + sum1) & 0xFF;
    }

    blockId++;
  }

  int result = sum2;
  result <<= 8;
  result += sum1;
  tputs_rom("checksum: ");
  tputs_ram(uint2hex(result));
  tputc('\n');
}

