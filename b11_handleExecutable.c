#include "banking.h"
#define MYBANK BANK(11)

#include "b0_main.h"
#include "b8_terminal.h"
#include "commands.h"
#include "b0_runext.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include "b4_variables.h"
#include "b0_globals.h"
#include "b0_sams.h"
#include "b8_setupScreen.h"
#include "b0_runScript.h"
#include "procinfo.h"
#include <vdp.h>
#include <conio.h>

int loadExecutable(const char* ext, int* cmd_type);
int loadFromPath(const char *ext, const char *entry, int* cmd_type);
int loadFlatFormat(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr);
int prepareMemory();
int checkFormat(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr, int* pages);
int loadPagedFormat(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr, int pages);


#define BIN 0xFCFC
#define SCRIPT 0x0001

#define FMT_ERR -1
#define FMT_EXE 1
#define FMT_SAMS 2

struct __attribute__((__packed__)) FCProgramHeader {
  int fcfc;
  int sams;
  int safe;
  int start;
};

#define SAMS_REG(x) *((volatile int*)(0x4000 + (x << 1)))

inline void snapshotSAMS(int* snapshot) {
  __asm__(
    "li r12,0x1E00\n\t"
    "sbo 0\n\t"
  );

  snapshot[0] = SAMS_REG(0x0A);
  snapshot[1] = SAMS_REG(0x0B);
  snapshot[2] = SAMS_REG(0x0C);
  snapshot[3] = SAMS_REG(0x0D);
  snapshot[4] = SAMS_REG(0x0E);
  snapshot[5] = SAMS_REG(0x0F);

  __asm__(
    "sbz 0\n\t"
  );
}

inline void restoreSAMS(int* snapshot) {
  __asm__(
    "li r12,0x1E00\n\t"
    "sbo 0\n\t"
  );

  SAMS_REG(0x0A) = snapshot[0];
  SAMS_REG(0x0B) = snapshot[1];
  SAMS_REG(0x0C) = snapshot[2];
  SAMS_REG(0x0D) = snapshot[3];
  SAMS_REG(0x0E) = snapshot[4];
  SAMS_REG(0x0F) = snapshot[5];
  
  __asm__(
    "sbz 0\n\t"
  );
}

void popProcInfo() {
    procInfoPtr = procInfoPtr->prev;    
}

void handleExecutable(char *ext)
{
  int cmd_type = 0;

  struct ProcInfo procInfo;
  procInfo.prev = procInfoPtr;
  procInfo.base_page = 0;
  procInfoPtr = &procInfo;

  int samsSnapshot[6];
  snapshotSAMS(samsSnapshot);

  int err = loadExecutable(ext, &cmd_type);

  if (err) {
    tputs_rom("unknown command: ");
    bk_tputs_ram(ext);
    bk_tputc('\n');
    popProcInfo();
    return;
  }

  if (cmd_type == BIN) {
    identify_callback old_hook = bk_get_identify_hook();
    int restoreDisplay = *(volatile int*)0xA004;
    int old_nTitleLine = nTitleLine;
    int old_oldcolors = conio_scrnCol;
    if (restoreDisplay != 0xFCFC) {
      // turn bar off, cause app is using the full screen.
      nTitleLine = 0;
      bk_setupScreen(displayWidth);
    }
    // Go to bank 0 to actually launch so API tables are visible.
    err = bk_runExecutable(ext);

    bk_set_identify_hook(old_hook);
    if (restoreDisplay != 0xFCFC) {
      conio_scrnCol = old_oldcolors;
      nTitleLine = old_nTitleLine;
      bk_setupScreen(displayWidth);
    }

    if (err) {
      tputs_rom("error result: ");
      bk_tputs_ram(bk_uint2str(err));
      bk_tputc('\n');
    }

    if (sams_total_pages) {
      // if we didn't have sams, we wouldn't be able to get here with api_exec true.
      bk_free_pages(sams_next_page - procInfoPtr->base_page);
      // restore previous sams registers for upper memory expansion
      restoreSAMS(samsSnapshot);
    }
  }
  popProcInfo();
}

char* token_cursor(char* dst, char* str, int delim) {
  // return the pointer to the character after the delim following token in original string:str
  // copies token into dst.
  // returns 0 if no following string
  if (!str) {
    dst[0] = 0;
    return 0;
  }

  int i = 0;
  while (str[i] != 0 && str[i] != delim) {
    dst[i] = str[i];
    i++;
  }
  dst[i] = 0;

  char* next = &(str[i]);
  if (*next == delim) {
    return next + 1;
  }
  return 0;
}

int loadExecutable(const char* ext, int* cmd_type) {
  // allow ext to be fully qualified path to command
  if (!loadFromPath(ext, 0, cmd_type)) {
    return 0;
  }

  // look in current directory second, always... regardless of PATH
  if (!loadFromPath(ext, currentPath, cmd_type)) {
    return 0;
  }

  char* path = bk_vars_get(str2ram("PATH"));
  if (path != (char*)-1) {
    char entry[64];
    char *cursor = token_cursor(entry, path, ';');
    // ensure '.' at end of path entry
    if (!bk_str_endswith(entry, str2ram("."))) {
      bk_strcat(entry, str2ram("."));
    }

    while(entry[0]) {
      if (!loadFromPath(ext, entry, cmd_type)) {
	  return 0;
      }
      cursor = token_cursor(entry, cursor, ';');
    }
    return 1;
  }
  return 1;
}

void setDsrAndPath(const char* ext, const char* entry, struct DeviceServiceRoutine** dsr, char* path) {
  char fullname[256];
  fullname[0] = 0;
  if (entry != 0) {
    bk_strcpy(fullname, entry);
  }
  bk_strcat(fullname, ext);
  bk_parsePathParam(fullname, dsr, path, PR_REQUIRED);
}

void clearAddInfo(struct AddInfo* addInfoPtr) {
  addInfoPtr->first_sector = 0;
  addInfoPtr->eof_offset = 0;
  addInfoPtr->flags = 0;
  addInfoPtr->rec_length = 0;
  addInfoPtr->records = 0;
  addInfoPtr->recs_per_sec = 0;
}

int loadFromPath(const char *ext, const char *entry, int* cmd_type) {
  struct DeviceServiceRoutine *dsr = 0;
  char path[256];

  setDsrAndPath(ext, entry, &dsr, path);
  if (!dsr) {
    return 1;
  }

  if (bk_runScript(dsr, path)) {
    *cmd_type = SCRIPT;
    return 0;
  }

  unsigned int iocode = bk_path2iocode(path);

  int parent_idx = bk_lindexof(path, '.', bk_strlen(path) - 1);
  char filename[11];
  bk_strncpy(filename, path + parent_idx + 1, 10);
  path[parent_idx + 1] = 0x00;

  bk_lvl2_setdir(dsr->crubase, iocode, path);

  // AddInfo must be in scratchpad
  struct AddInfo *addInfoPtr = (struct AddInfo *)0x8320;
  clearAddInfo(addInfoPtr);

  unsigned int err = bk_lvl2_input(dsr->crubase, iocode, filename, 0, addInfoPtr);
  if (err) {
    return 1;
  }

  // Check the type is PROGRAM
  int type = (addInfoPtr->flags & 0x03);
  if (type) {
    *cmd_type = BIN;
    int pages = 0;
    int format = checkFormat(dsr, iocode, filename, addInfoPtr, &pages);
    if (format == FMT_EXE) {
      return loadFlatFormat(dsr, iocode, filename, addInfoPtr);
    } else if (format == FMT_SAMS) {
      return loadPagedFormat(dsr, iocode, filename, addInfoPtr, pages);
    }
  }
  return 1;
}

int checkFormat(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr, int* pages) {
  if (addInfoPtr->first_sector == 0) {
    tputs_rom("error, file is empty.\n");
    return FMT_ERR;
  }
  // peek at first block FC header
  addInfoPtr->first_sector = 0;
  int err = bk_lvl2_input(dsr->crubase, iocode, filename, 1, addInfoPtr);
  if (err) {
    tputs_rom("error reading file: ");
    bk_tputs_ram(bk_uint2hex(err));
    bk_tputc('\n');
    return FMT_ERR;
  }
  // just after the AddInfo in scratchpad
  struct FCProgramHeader header;
  vdpmemread(addInfoPtr->buffer, (char*)&header, sizeof(struct FCProgramHeader));

  if (header.fcfc != 0xFCFC) {
    // not an FC executable
    return FMT_ERR;
  }

  if (header.sams == 0) {
  *pages = 6;
    return FMT_EXE;
  } else {
    *pages = header.sams;
    return FMT_SAMS;
  }
}

int allocatePages(int pages) {
  if (sams_next_page + pages > sams_total_pages) {
    tputs_rom("Not enough SAMS pages remaining\n");
    return 1;  // don't allow nested execution if we don't have space
  }
  int pageStart = bk_alloc_pages(pages);
  procInfoPtr->base_page = pageStart;
  if (pageStart == -1) {
    tputs_rom("Failed to allocate SAMS pages\n");
    return 1; // no more pages, fail the same way
  }
  return 0;
}

/* return true if there is a problem preparing memory */
int prepareMemory() {
  if (allocatePages(6)) {
    return 1;
  }
  // should be good to map upper memory expansion to the new pages.
  for (int i = 0; i < 6; i++) {
    bk_map_page(procInfoPtr->base_page + i, 0xA000 + (i * 0x1000));
  }
  return 0;
}

int loadPagedFormat(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr, int pages) {
  clearAddInfo(addInfoPtr);
  bk_lvl2_input(dsr->crubase, iocode, filename, 0, addInfoPtr);

  int totalBlocks = addInfoPtr->first_sector;
  if (sams_total_pages) {
    int extraPages = (totalBlocks >> 4) - pages;
    if (extraPages < 4) {
      // ensure we have at least enough pages to fill upper memory expansion
      extraPages = 4;
    }
    if (allocatePages(pages + extraPages)) {
      return 1;
    }
  } else {
    tputs_rom("SAMS Memory Required\n");
    return 1;
  }

  int blockId = 0;
  int page = 0;
  while(page < pages) {
    int loadAddr = 0xA000 + ((page & 1) << 12);
    bk_map_page(procInfoPtr->base_page + page, loadAddr);
    addInfoPtr->first_sector = blockId;
    int err = bk_lvl2_input(dsr->crubase, iocode, filename, 16, addInfoPtr);
    blockId += 16;
	
    // every other page goes into A000 or B000
    vdpmemread(addInfoPtr->buffer, (char*)loadAddr, 4096);

    page++;
  }
  // expect the first 2 pages mapped in
  bk_map_page(procInfoPtr->base_page, 0xA000);
  if (pages > 1) {
    bk_map_page(procInfoPtr->base_page + 1, 0xB000);
  }

  // then load the remaining data/bss blocks from disk
  //   totalBlocks - blockId -> 0xC000
  int cpuAddr = 0xC000;
  while(blockId < totalBlocks) {
    bk_map_page(procInfoPtr->base_page + page, cpuAddr);
    addInfoPtr->first_sector = blockId;
    int err = bk_lvl2_input(dsr->crubase, iocode, filename, 16, addInfoPtr);
    blockId += 16;
    
    vdpmemread(addInfoPtr->buffer, (char*)cpuAddr, 4096);
    if (cpuAddr != 0xF000) {
      cpuAddr += 0x1000;
    }
    page++;
  }
  // pad the upper 16K out with sams pages if the exe is doesn't
  // fill the space.
  while(page < (pages + 4)) {
    bk_map_page(procInfoPtr->base_page + page, cpuAddr);
    cpuAddr += 0x1000;
    page++;
  }

  return 0;
}

int loadFlatFormat(struct DeviceServiceRoutine *dsr, int iocode, char *filename, struct AddInfo *addInfoPtr) {
  if (sams_total_pages) { // if sams exists
    if (prepareMemory()) {
        return 1;
    }
  }
  char *cpuAddr = (char *)0xA000;

  // need to repair ?addInfo? to state before reading the first block
  clearAddInfo(addInfoPtr);
  bk_lvl2_input(dsr->crubase, iocode, filename, 0, addInfoPtr);

  int totalBlocks = addInfoPtr->first_sector;
  // Allow the file to be larger, but only load the first 24K.
  if (totalBlocks > 96) {
    totalBlocks = 96;
  }

  int blockId = 0;
  while (blockId < totalBlocks) {
    int blk_cnt = totalBlocks - blockId;
    if (blk_cnt > 17) {
      blk_cnt = 17;
    }
    int load_size = blk_cnt << 8;

    addInfoPtr->first_sector = blockId;
    int err = bk_lvl2_input(dsr->crubase, iocode, filename, blk_cnt, addInfoPtr);
    if (err) {
      tputs_rom("error reading file: ");
      bk_tputs_ram(bk_uint2hex(err));
      bk_tputc('\n');
      return 1;
    }

    vdpmemread(addInfoPtr->buffer, cpuAddr, load_size);

    cpuAddr += load_size;
    blockId += blk_cnt;
  }
  return 0;
}
