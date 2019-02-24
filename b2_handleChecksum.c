#include "banks.h"
#define MYBANK BANK_2

#include "b2_commands.h"
#include "b0_globals.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include <conio.h>
#include <string.h>

void handleChecksum() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cputs("error, must specify a file name\n");
    return;
  }
  char path[256];
  // check that file exists
  strcpy(path, currentPath);
  strcat(path, filename);
  if (0 != existsFile(currentDsr, path)) {
    cputs("error, file not found: ");
    cputs(currentPath);
    cputs(filename);
    cputc('\n');
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

  lvl2_setdir(source_crubase, source_unit, currentPath);
  unsigned char err = lvl2_input(source_crubase, source_unit, filename, 0, addInfoPtr);
  if (err) {
    cputs("error reading file: ");
    cputs(uint2hex(err));
    cputc('\n');
    return;
  }

  unsigned char eof_offset = addInfoPtr->eof_offset;

  int sum1 = 0;
  int sum2 = 0;

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    cputs("error, source file is empty.\n");
    return;
  }
  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;
    lvl2_setdir(source_crubase, source_unit, currentPath);
    err = lvl2_input(source_crubase, source_unit, filename, 1, addInfoPtr);
    if (err) {
      cputs("error reading file: ");
      cputs(uint2hex(err));
      cputc('\n');
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
  cputs("checksum: ");
  cputs(uint2hex(result));
  cputc('\n');
}

