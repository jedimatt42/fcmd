#include "banks.h"
#define MYBANK BANK(6)

#include "commands.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b8_terminal.h"
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
  if (strlen(key) > 3 && (bk_str_startswith(key, str2ram("DSK")) || bk_str_startswith(key, str2ram("URI")))) {
    char* path = strtok(0, " ");
    if (path) {
      if (bk_str_startswith(key, str2ram("URI"))) {
        bk_tputs_ram(key);
        tputs_rom(". => ");
      } else {
        char* drive = strtok(key, "_");
        bk_tputs_ram(drive);
        tputs_rom(". => TIPI.");
      }
      if (path[0] != '.') {
        bk_tputs_ram(path);
      }
      bk_tputc('\n');
    }
  }
}

static void listDrives() {
  visitLines(onLineShowMapping, 0);
}

void onLineIfDriveShowMapping(char* linebuf, char* drivePrefix) {
  char* key = strtok(linebuf, "=");
  if (strlen(key) > 4 && bk_str_startswith(key, drivePrefix)) {
    char* path = strtok(0, " ");
    if (path) {
      tputs_rom("TIPI.");
      if (path[0] != '.') {
        bk_tputs_ram(path);
      }
    } else {
      tputs_rom("not mapped\n");
    }
    bk_tputc('\n');
  }
}

void onLineIfUriShowMapping(char* linebuf, char* uriPrefix) {
  char *key = strtok(linebuf, "=");
  if (bk_str_startswith(key, uriPrefix)) {
    char* path = strtok(0, " ");
    if (path) {
      bk_tputs_ram(path);
    } else {
      tputs_rom("not mapped\n");
    }
  }
}

static void showDriveMapping(const char* drive) {
  char* drivePrefix = strtok((char*) drive, ".");
  if (bk_str_startswith(drivePrefix, str2ram("DSK"))) {
    visitLines(onLineIfDriveShowMapping, drivePrefix);
  } else {
    visitLines(onLineIfUriShowMapping, drivePrefix);
  }
}

static void clearDriveMapping(const char* drive) {
  char keybuf[10];
  if (bk_str_startswith(drive, str2ram("URI"))) {
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
  if (bk_str_startswith(linebuf, str2ram("AUTO="))) {
    char* val = strtok(linebuf, "=");
    val = strtok(0, " ");
    tputs_rom("AUTO ");
    bk_tputs_ram(val);
    bk_tputc('\n');
  }
}

static void setAutoMap() {
  char* onoff = strtok(0, " ");
  if (onoff) {
    if (!bk_strcmpi(str2ram("on"), onoff)) {
      writeConfigItem("AUTO", "on");
    } else if (!bk_strcmpi(str2ram("off"), onoff)) {
      writeConfigItem("AUTO", "off");
    } else {
      tputs_rom("error, value must be one of: on, off\n");
    }
  } else {
    visitLines(onLineAuto, 0);
  }
}

static int __attribute__((noinline)) validDrive(const unsigned char *keybuf, const unsigned char *prefix, char min, char max)
{
  int l = strlen(keybuf);
  int form = (l == 5 && keybuf[4] == '.') || (l == 4);
  return bk_str_startswith(keybuf, prefix) &&
       keybuf[3] >= min &&
       keybuf[3] <= max &&
       form;
}

static int __attribute__((noinline)) isMappable(const char *drive)
{
  return validDrive(drive, "DSK", '1', '4') || validDrive(drive, "URI", '1', '3');
}

static void setDriveMapping(const char* drive, const char* path) {
  unsigned char keybuf[10];
  int dlen = strlen(drive);
  strncpy(keybuf, (char*) drive, dlen < 9 ? dlen : 9);

  if (!isMappable(keybuf))
  {
    tputs_rom("error, bad drive specification\n");
    return;
  }

  int diskmapping = 0;
  if (bk_str_startswith(keybuf, str2ram("DSK"))) {
    strcpy(keybuf+4, "_DIR");
    diskmapping = 1;
  } else if (bk_str_startswith(keybuf, str2ram("URI"))) {
    keybuf[4] = 0;
  }

  char namebuf[256];

  if (diskmapping) {
    strcpy(namebuf, "TIPI");
    struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);
    strcat(namebuf, ".");
    if (bk_str_equals((char*) path, str2ram("TIPI.")) || bk_str_equals((char*) path, str2ram("."))) {
      strcpy(namebuf, ".");
    } else if (bk_str_startswith(path, str2ram("TIPI."))) {
      strcpy(namebuf, path+5);
    } else {
      strcpy(namebuf, path);
    }
  } else {
    if (!bk_str_startswith(path, str2ram("HTTP"))) {
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
  int clear = 0 == bk_strcmpi(str2ram("/c"), peek);

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
      if (!bk_strcmpi(str2ram("auto"), drive)) {
        setAutoMap();
      } else {
        char* peek = strtokpeek(0, " ");
        if (peek) {
          if (bk_str_startswith(drive, str2ram("DSK"))) {
            struct DeviceServiceRoutine *dsr;
            char path[256];
            bk_parsePathParam(&dsr, path, PR_OPTIONAL);
            if (dsr == 0)
            {
              tputs_rom("bad path specified\n");
            } else {
              setDriveMapping(drive, path);
            }
          } else {
            char* tok = strtok(0, " ");
            setDriveMapping(drive, tok);
          }
        }
        else
        {
          showDriveMapping(drive);
        }
      }
    }
  }
  bk_tputc('\n');
}
