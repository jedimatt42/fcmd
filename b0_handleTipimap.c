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
    tputs("could no open PI.CONFIG\n");
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
    tputs("could no open PI.CONFIG\n");
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
        tputs(key);
        tputs(". => ");
      } else {
        char* drive = strtok(key, "_");
        tputs(drive);
        tputs(". => TIPI.");
      }
      tputs(path);
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
      tputs("TIPI.");
      tputs(path);
    } else {
      tputs("not mapped\n");
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
    tputs("AUTO ");
    tputs(val);
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
      tputs("error, value must be one of: on, off\n");
    }
  } else {
    visitLines(onLineAuto, 0);
  }
}

static void setDriveMapping(const char* drive, const char* path) {
  char keybuf[10];
  int dlen = strlen(drive);
  memcpy(keybuf, drive, dlen < 9 ? dlen : 9);
  keybuf[9] = 0;
  
  if (!((dlen == 4 || (dlen == 5 && keybuf[4] == '.')) && 
       (0==str_startswith(keybuf, "DSK") || 0==str_startswith(keybuf, "URI")) && 
       keybuf[3] >= '1' && keybuf[3] <= '3'))
  {
    tputs("error, bad drive specification\n");
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
    if (str_startswith(path, "TIPI.")) {
      strcpy(namebuf, path+5);
    } else {
      strcpy(namebuf, path);
    }
    
    if(0!=bk_existsDir(dsr, namebuf)) {
      tputs("error target invalid path\n");
      // return;
    }
    tputs(keybuf);
    tputs(" => TIPI.");
  } else {
    if (!str_startswith(path, "HTTP")) {
      tputs("error path must be a URL prefix\n");
      return;
    }
    strcpy(namebuf, path);
    tputs(keybuf);
    tputs(" => ");
  }
  
  tputs(namebuf); tputs("\n");
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
      tputs("error, /c requies drive to clear\n");
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