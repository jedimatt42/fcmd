#include "commands.h"
#include "globals.h"
#include "main.h"
#include "parsing.h"
#include "strutil.h"
#include "lvl2.h"

#include <system.h>
#include <conio.h>
#include <string.h>

void handleCd() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    cprintf("no path: drive or folder specified\n");
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }
  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cprintf("error, device/folder not found: %s\n", path);
    return;
  }
  
  currentDsr = dsr;
  strcpy(currentPath, path);
}

void handleChecksum() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }
  char path[256];
  // check that file exists
  strcpy(path, currentPath);
  strcat(path, filename);
  if (0 != existsFile(currentDsr, path)) {
    cprintf("error, file not found: %s%s\n", currentPath, filename);
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
    cprintf("error reading file: %x\n", err);
    return;
  }

  unsigned char eof_offset = addInfoPtr->eof_offset;

  int sum1 = 0;
  int sum2 = 0;

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    cprintf("error, source file is empty.\n");
    return;
  }
  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;
    lvl2_setdir(source_crubase, source_unit, currentPath);
    err = lvl2_input(source_crubase, source_unit, filename, 1, addInfoPtr);
    if (err) {
      cprintf("error reading file: %x\n", err);
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
  cprintf("checksum: %x\n", result);
}

void handleCopy() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }
  char path[256];
  // check that file exists
  strcpy(path, currentPath);
  strcat(path, filename);
  if (0 != existsFile(currentDsr, path)) {
    cprintf("error, file not found: %s%s\n", currentPath, filename);
    return;
  }

  struct DeviceServiceRoutine* dsr = 0;
  parsePathParam(&dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    cprintf("no path: drive or folder specified\n");
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }
  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cprintf("error, device/folder not found: %s\n", path);
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
  unsigned char err = lvl2_input(source_crubase, source_unit, filename, 0, addInfoPtr);
  if (err) {
    cprintf("error reading file: %x\n", err);
    return;
  }

  int totalBlocks = addInfoPtr->first_sector;
  if (totalBlocks == 0) {
    cprintf("error, source file is empty.\n");
    return;
  }

  cprintf("\n"); // prepare status line

  // write file meta data
  lvl2_setdir(dest_crubase, dest_unit, path);
  err = lvl2_output(dest_crubase, dest_unit, filename, 0, addInfoPtr);
  if (err) {
    cprintf("error writing file: %x\n", err);
    return;
  }

  int blockId = 0;
  while(blockId < totalBlocks) {
    addInfoPtr->first_sector = blockId;
    lvl2_setdir(source_crubase, source_unit, currentPath);
    err = lvl2_input(source_crubase, source_unit, filename, 1, addInfoPtr);
    if (err) {
      cprintf("\nerror reading file: %x\n", err);
      return;
    }

    lvl2_setdir(dest_crubase, dest_unit, path);
    err = lvl2_output(dest_crubase, dest_unit, filename, 1, addInfoPtr);
    if (err) {
      cprintf("\nerror reading file: %x\n", err);
      return;
    }

    blockId++;
    cprintf("\rcopied block %d of %d", blockId, totalBlocks);
  }
  cprintf("\n"); // end status line.
}

void handleVer() {
  titleScreen();
}

void handleDrives() {
  int i = 0;
  int cb = 0;
  
  while(dsrList[i].name[0] != 0) {
    cb = dsrList[i].crubase;
    cprintf("%x -", cb);
    while (cb == dsrList[i].crubase) {
      cprintf(" %s", dsrList[i].name);
      i++;
    }
    cprintf("\n");
  }
}

void handleLvl2() {
  char* tok = strtok(0, " ");
  int crubase = htoi(tok);

  if (crubase == 0) {
    cprintf("no crubase specified\n");
    return;
  }

  enableROM(crubase);
  struct DeviceRomHeader* rom = (struct DeviceRomHeader*)0x4000;

  struct NameLink* link = rom->basiclnk;
  while(link != 0) {
    if (link->name[0] == 1) {
      cprintf(" >%x", link->name[1]);
    }
    link = link->next;
  }
  cprintf("\n");

  disableROM(crubase);
}

void handleDir() {
  struct DeviceServiceRoutine* dsr = 0;
  char path[256];
  parsePathParam(&dsr, path, PR_OPTIONAL);
  if (dsr == 0) {
    return;
  }
  if (path[strlen(path)-1] != '.') {
    strcat(path, ".");
  }

  unsigned char stat = existsDir(dsr, path);
  if (stat != 0) {
    cprintf("error, device/folder not found: %s\n", path);
    return;
  }

  loadDir(dsr, path, onVolInfo, onDirEntry);
  cprintf("\n");
}

void handleMkdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cprintf("error, must specify a directory name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_mkdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cprintf("cannot create directory %s%s\n", currentPath, dirname);
  }
}

void handleProtect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 1);
  if (err) {
    cprintf("cannot protect file %s%s\n", currentPath, filename);
  }
}

void handleUnprotect() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_protect(currentDsr->crubase, unit, filename, 0);
  if (err) {
    cprintf("cannot unprotect file %s%s\n", currentPath, filename);
  }
}

void handleDelete() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify a file name\n");
    return;
  }

  char buffer[256];
  strcpy(buffer, currentPath);
  strcat(buffer, ".");
  strcat(buffer, filename);
  struct PAB pab;
  initPab(&pab);
  pab.pName = buffer;
  
  unsigned char err = dsr_status(currentDsr, &pab);
  if (err) {
    cprintf("file not found %s%s\n", currentPath, filename);
  }

  err = dsr_delete(currentDsr, &pab);
  if (err) {
    cprintf("cannot delete file %s%s\n", currentPath, filename);
  }
}

void handleRename() {
  char* filename = strtok(0, " ");
  if (filename == 0) {
    cprintf("error, must specify source file name\n");
    return;
  }
  char* newname = strtok(0, " ");
  if (newname == 0) {
    cprintf("error, must specify new file name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  char path[256];
  strcpy(path, currentPath);
  strcat(path, ".");
  strcat(path, filename);

  unsigned char stat = existsDir(currentDsr, path);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);
  unsigned char err = 0xff;
  if (stat == 0) {
    err = lvl2_rendir(currentDsr->crubase, unit, filename, newname);
  } else {
    err = lvl2_rename(currentDsr->crubase, unit, filename, newname);
  }

  if (err) {
    cprintf("cannot rename file %s%s\n", currentPath, filename);
  }
}

void handleRmdir() {
  char* dirname = strtok(0, " ");
  if (dirname == 0) {
    cprintf("error, must specify a directory name\n");
    return;
  }

  unsigned char unit = path2unitmask(currentPath);

  lvl2_setdir(currentDsr->crubase, unit, currentPath);

  unsigned char err = lvl2_rmdir(currentDsr->crubase, unit, dirname);
  if (err) {
    cprintf("cannot remove directory %s%s\n", currentPath, dirname);
  }
}

void handleWidth() {
  char* tok = strtok(0, " ");
  int width = atoi(tok);

  if (width == 40 || width == 80) {
    setupScreen(width);
  } else {
    cprintf("no width specified\n");
  }
}

void handleExit() {
  resetF18A();
  exit();
}