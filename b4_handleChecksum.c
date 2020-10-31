#include "banks.h"
#define MYBANK BANK(4)

#include "commands.h"
#include "b0_parsing.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include <string.h>
#include <vdp.h>

void handleChecksum() {
  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no file path name specified\n");
    return;
  }

  unsigned int iocode = bk_path2iocode(path);

  int parent_idx = bk_lindexof(path, '.', bk_strlen(path) - 1);
  char filename[11];
  bk_strncpy(filename, path + parent_idx + 1, 10);
  path[parent_idx + 1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, iocode, path);

  // AddInfo must be in scratchpad
  struct AddInfo* addInfoPtr = (struct AddInfo*) 0x8320;
  addInfoPtr->first_sector = 0;
  addInfoPtr->eof_offset = 0;
  addInfoPtr->flags = 0;
  addInfoPtr->rec_length = 0;
  addInfoPtr->records = 0;
  addInfoPtr->recs_per_sec = 0;

  unsigned int err = bk_lvl2_input(dsr->crubase, iocode, filename, 0, addInfoPtr);
  if (err) {
    tputs_rom("error reading file: ");
    bk_tputs_ram(bk_uint2hex(err));
    bk_tputc('\n');
    return;
  }

  unsigned char eof_offset = addInfoPtr->eof_offset;

  int sum1 = 0;
  int sum2 = 0;

  int totalBlocks = addInfoPtr->first_sector;

  // now read and process all the blocks
  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;

    int blk_cnt = totalBlocks - blockId;
    if (blk_cnt > 17) {
      blk_cnt = 17;
    }

    err = bk_lvl2_input(dsr->crubase, iocode, filename, blk_cnt, addInfoPtr);
    if (err) {
      tputs_rom("error reading file: ");
      bk_tputs_ram(bk_uint2hex(err));
      bk_tputc('\n');
      return;
    }

    unsigned int maxByte = 0;
    if (blockId+blk_cnt == totalBlocks) {
      maxByte = eof_offset == 0 ? 256 : eof_offset;
      maxByte += 256 * (blk_cnt - 1);
    } else {
      maxByte = blk_cnt * 256;
    }

    for(int i=0; i < maxByte; i++) {
      sum1 = (sum1 + vdpreadchar(addInfoPtr->buffer + i)) & 0xFF;
      sum2 = (sum2 + sum1) & 0xFF;
    }

    blockId += blk_cnt;
  }

  int result = sum2;
  result <<= 8;
  result += sum1;
  tputs_rom("checksum: ");
  bk_tputs_ram(bk_uint2hex(result));
  bk_tputc('\n');
}

