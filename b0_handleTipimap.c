#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include <conio.h>
#include <string.h>

typedef void (*line_cb)(char*, char*);

void handleTipimap();
static void listDrives();
static void visitLines(line_cb onLine, char* filterArg1);
static void showDriveMapping(const char* drive);
static void clearDriveMapping(const char* drive);
static void setAutoMap();
static void setDriveMapping(const char* drive, const char* path);

static void visitLines(line_cb onLine, char* filterArg1) {
  char namebuf[14];
  strcpy(namebuf, "DSK1");

  struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);

  struct PAB pab;
  bk_initPab(&pab);

  strcpy(namebuf, "DSK1.CONFIG");

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
  
  if (err) {
    cputs("could no open PI.CONFIG\n");
    return;
  }
  while(!err) {
    err = bk_dsr_read(dsr, &pab, 0);
    if (!err) {
      // print the line...
      char linebuf[256];
      vdpmemread(pab.VDPBuffer, linebuf, pab.CharCount);
      linebuf[pab.CharCount] = 0;

      onLine(linebuf, filterArg1);
    }
  }
  bk_dsr_close(dsr, &pab);
}

void onLineShowMapping(char* linebuf, char* extra) {
  char* key = strtok(linebuf, "=");
  if (strlen(key) > 3 && str_startswith(key, "DSK")) {
    char* path = strtok(0, " ");
    if (path) {
      char* drive = strtok(key, "_");
      cputs(drive);
      cputs(". => TIPI.");
      cputs(path);
    }
    cputc('\n');
  }
}

static void listDrives() {
  visitLines(onLineShowMapping, 0);
}

void onLineIfDriveShowMapping(char* linebuf, char* drivePrefix) {
  char* key = strtok(linebuf, "=");
  if (strlen(key) > 4 && str_startswith(key, drivePrefix)) {
    char* path = strtok(0, " ");
    if (path) {
      cputs("TIPI.");
      cputs(path);
    } else {
      cputs("not mapped\n");
    }
    cputc('\n');
  }
}

static void showDriveMapping(const char* drive) {
  char* drivePrefix = strtok((char*) drive, ".");

  visitLines(onLineIfDriveShowMapping, drivePrefix);
}

static void clearDriveMapping(const char* drive) {

}

static void setAutoMap() {

}

static void setDriveMapping(const char* drive, const char* path) {

}

void handleTipimap() {
  //  [/c] [drive] [path]
  char* peek = strtokpeek(0, " ");
  int clear = 0 == strcmpi("/C", peek);
 
  if (clear) {
    strtok(0, " "); // consume the optional /c
  }

  char* drive = strtok(0, " ");
  if (!drive) {
    if (clear) {
      cputs("error, must specify drive to with /c\n");
    } else {
      listDrives();
    }
  } else {
    if (clear) {
      clearDriveMapping(drive);
    } else {
      if (!strcmpi(drive, "AUTO")) {
        setAutoMap();
      } else {
        char* path = strtok(0, " ");
        if (path) {
          setDriveMapping(drive, path);
        } else {
          showDriveMapping(drive);
        }
      }
    }
  }
  cputc('\n');
}