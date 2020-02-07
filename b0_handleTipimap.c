#include "banks.h"
#define MYBANK BANK_0

#include "commands.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <vdp.h>

typedef void (*line_cb)(char*, char*);

void handleTipimap();
static void listDrives();
static void visitLines(line_cb onLine, char* filterArg1);
static void showDriveMapping(const char* drive);
static void clearDriveMapping(const char* drive);
static void setAutoMap();
static void setDriveMapping(const char* drive, const char* path);
static void writeConfigItem(const char* key, const char* value);

static void visitLines(line_cb onLine, char* filterArg1) {
  char namebuf[14];
  strcpy(namebuf, "PI");

  struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);

  struct PAB pab;

  strcpy(namebuf, "PI.CONFIG");

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);
  
  if (err) {
    tputs_rom("could no open PI.CONFIG\n");
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

static void writeConfigItem(const char* key, const char* value) {
  char namebuf[14];
  strcpy(namebuf, "PI");

  struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);

  struct PAB pab;

  strcpy(namebuf, "PI.CONFIG");

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_APPEND | DSR_TYPE_VARIABLE, 0);
  
  if (err) {
    tputs_rom("could no open PI.CONFIG\n");
    return;
  }
  bk_initPab(&pab);

  char linebuf[250];
  strcpy(linebuf, key);
  strcat(linebuf, "=");
  strcat(linebuf, (char*) value);
  err = bk_dsr_write(dsr, &pab, linebuf, strlen(linebuf));

  bk_dsr_close(dsr, &pab);
}

void onLineShowMapping(char* linebuf, char* extra) {
  char* key = strtok(linebuf, "=");
  if (strlen(key) > 3 && (str_startswith(key, "DSK") || str_startswith(key, "URI"))) {
    char* path = strtok(0, " ");
    if (path) {
      if (str_startswith(key, "URI")) {
        tputs_ram(key);
        tputs_rom(". => ");
      } else {
        char* drive = strtok(key, "_");
        tputs_ram(drive);
        tputs_rom(". => TIPI.");
      }
      tputs_ram(path);
      tputc('\n');
    }
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
      tputs_rom("TIPI.");
      tputs_ram(path);
    } else {
      tputs_rom("not mapped\n");
    }
    tputc('\n');
  }
}

static void showDriveMapping(const char* drive) {
  char* drivePrefix = strtok((char*) drive, ".");

  visitLines(onLineIfDriveShowMapping, drivePrefix);
}

static void clearDriveMapping(const char* drive) {
  char keybuf[10];
  if (str_startswith(drive, "URI")) {
    strcpy(keybuf, drive);
    keybuf[4] = 0;
  } else {
    strcpy(keybuf, drive);
    strcpy(keybuf+4, "_DIR");
  }

  char empty[1];
  empty[0] = 0;

  writeConfigItem(keybuf, empty);
}

void onLineAuto(char* linebuf, char* extra) {
  if (str_startswith(linebuf, "AUTO=")) {
    char* val = strtok(linebuf, "=");
    val = strtok(0, " ");
    tputs_rom("AUTO ");
    tputs_ram(val);
    tputc('\n');
  }
}

static void setAutoMap() {
  char* onoff = strtok(0, " ");
  if (onoff) {
    if (!strcmpi(onoff, "ON")) {
      writeConfigItem("AUTO", "on");
    } else if (!strcmpi(onoff, "OFF")) {
      writeConfigItem("AUTO", "off");
    } else {
      tputs_rom("error, value must be one of: on, off\n");
    }
  } else {
    visitLines(onLineAuto, 0);
  }
}

static int validDrive(const char* keybuf, const char* prefix, char min, char max) {
  return (0==str_startswith(keybuf, prefix) && 
       keybuf[3] >= min && keybuf[3] <= max);
}

static void setDriveMapping(const char* drive, const char* path) {
  char keybuf[10];
  int dlen = strlen(drive);
  memcpy(keybuf, drive, dlen < 9 ? dlen : 9);
  keybuf[9] = 0;
  
  if (!((dlen == 4 || (dlen == 5 && keybuf[4] == '.')) && 
       (validDrive(keybuf, "DSK", '0', '4') || validDrive(keybuf, "URI", '1', '3'))))
  {
    tputs_rom("error, bad drive specification\n");
    return;
  }

  if (str_startswith(keybuf, "DSK")) {
    strcpy(keybuf+4, "_DIR");
  } else if (str_startswith(keybuf, "URI")) {
    keybuf[4] = 0;
  }
  
  char namebuf[256];
  
  if (str_startswith(keybuf, "DSK")) {
    strcpy(namebuf, "TIPI");
    struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);
    strcat(namebuf, ".");
    if (str_equals((char*) path, "TIPI.") || str_equals((char*) path, ".")) {
      strcpy(namebuf, ".");
    } else if (str_startswith(path, "TIPI.")) {
      strcpy(namebuf, path+5);
    } else {
      strcpy(namebuf, path);
    }
  } else {
    if (!str_startswith(path, "HTTP")) {
      tputs_rom("error path must be a URL prefix\n");
      return;
    }
    strcpy(namebuf, path);
  }
  writeConfigItem(keybuf, namebuf);
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
      tputs_rom("error, no mapping drive specified\n");
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
  tputc('\n');
}