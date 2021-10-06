#include "banks.h"
#define MYBANK BANK(5)

#include "b0_globals.h"
#include "b5_copySingleFile.h"
#include "b2_dsrutil.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b2_lvl2.h"

void copySingleFile(struct CopySpec* src, struct CopySpec* dst) {

  if (dst->filename == 0) {
    dst->filename = src->filename;
  }

  tputs_rom("copying ");
  bk_tputs_ram(src->path);
  bk_tputs_ram(src->filename);
  tputs_rom(" to ");
  bk_tputs_ram(dst->path);
  bk_tputs_ram(dst->filename);
  bk_tputc('\n');

  // AddInfo must be in scratchpad
  struct AddInfo *addInfoPtr = (struct AddInfo *)0x8320;
  addInfoPtr->first_sector = 0;
  addInfoPtr->eof_offset = 0;
  addInfoPtr->flags = 0;
  addInfoPtr->rec_length = 0;
  addInfoPtr->records = 0;
  addInfoPtr->recs_per_sec = 0;

  unsigned int source_crubase = src->dsr->crubase;
  unsigned int source_iocode = bk_path2iocode(src->path);
  unsigned int dest_crubase = dst->dsr->crubase;
  unsigned int dest_iocode = bk_path2iocode(dst->path);

  // get file meta data
  bk_lvl2_setdir(source_crubase, source_iocode, src->path);
  unsigned int err = bk_lvl2_input(source_crubase, source_iocode, src->filename, 0, addInfoPtr);
  if (err) {
    tputs_rom("error reading file: ");
    bk_tputs_ram(bk_uint2hex(err));
    bk_tputc('\n');
    return;
  }

  int totalBlocks = addInfoPtr->first_sector;

  // write file meta data
  bk_lvl2_setdir(dest_crubase, dest_iocode, dst->path);
  err = bk_lvl2_output(dest_crubase, dest_iocode, dst->filename, 0, addInfoPtr);
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

    int blk_cnt = totalBlocks - blockId;
    if (blk_cnt > 17) {
      blk_cnt = 17;
    }

    // read a batch of blocks
    bk_lvl2_setdir(source_crubase, source_iocode, src->path);
    err = bk_lvl2_input(source_crubase, source_iocode, src->filename, blk_cnt, addInfoPtr);
    if (err) {
      tputs_rom("\nerror reading file: ");
      bk_tputs_ram(bk_uint2hex(err));
      bk_tputc('\n');
      return;
    }

    // write them back out
    bk_lvl2_setdir(dest_crubase, dest_iocode, dst->path);
    err = bk_lvl2_output(dest_crubase, dest_iocode, dst->filename, blk_cnt, addInfoPtr);
    if (err) {
      tputs_rom("\nerror writing file: ");
      bk_tputs_ram(bk_uint2hex(err));
      bk_tputc('\n');
      return;
    }

    blockId += blk_cnt;
  }
}


