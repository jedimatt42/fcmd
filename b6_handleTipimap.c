#include "banks.h"
#define MYBANK BANK(6)

#include "commands.h"
#include "b10_parsing.h"
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
  bk_strcpy(namebuf, str2ram("PI"));

  struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);

  struct PAB pab;

  bk_strcpy(namebuf, str2ram("PI.CONFIG"));

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL, 0);

  if (err) {
    tputs_rom("could no open PI.CONFIG\n");
    return;
  }
  while(!err) {
    char linebuf[256];
    err = bk_dsr_read_cpu(dsr, &pab, 0, linebuf);
    if (!err) {
      // print the line...
      linebuf[pab.CharCount] = 0;

      onLine(linebuf, filterArg1);
    }
  }
  bk_dsr_close(dsr, &pab);
}

static void writeConfigItem(const char* key, const char* value) {
  char namebuf[14];
  bk_strcpy(namebuf, str2ram("PI"));

  struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);

  struct PAB pab;

  bk_strcpy(namebuf, str2ram("PI.CONFIG"));

  int err = bk_dsr_open(dsr, &pab, namebuf, DSR_TYPE_APPEND | DSR_TYPE_VARIABLE, 0);

  if (err) {
    tputs_rom("could no open PI.CONFIG\n");
    return;
  }
  bk_initPab(&pab);

  char linebuf[250];
  bk_strcpy(linebuf, str2ram(key));
  bk_strcat(linebuf, str2ram("="));
  bk_strcat(linebuf, str2ram(value));
  err = bk_dsr_write(dsr, &pab, linebuf, bk_strlen(linebuf));

  bk_dsr_close(dsr, &pab);
}

void onLineShowMapping(char* linebuf, char* extra) {
  char* key = bk_strtok(linebuf, '=');
  if (bk_str_startswith(key, str2ram("DSK")) || bk_str_startswith(key, str2ram("URI")) || bk_str_startswith(key, str2ram("CS1"))) {
    char* path = bk_strtok(0, ' ');
    if (path) {
      if (bk_str_startswith(key, str2ram("URI"))) {
        bk_tputs_ram(key);
        tputs_rom(". => ");
      } else {
        char* drive = bk_strtok(key, '_');
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
  char* key = bk_strtok(linebuf, '=');
  if (bk_strlen(key) > 4 && bk_str_startswith(key, drivePrefix)) {
    char* path = bk_strtok(0, ' ');
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
  char *key = bk_strtok(linebuf, '=');
  if (bk_str_startswith(key, uriPrefix)) {
    char* path = bk_strtok(0, ' ');
    if (path) {
      bk_tputs_ram(path);
    } else {
      tputs_rom("not mapped\n");
    }
  }
}

static void showDriveMapping(const char* drive) {
  char* drivePrefix = bk_strtok((char*) drive, '.');
  if (bk_str_startswith(drivePrefix, str2ram("URI"))) {
    visitLines(onLineIfUriShowMapping, drivePrefix);
  } else {
    visitLines(onLineIfDriveShowMapping, drivePrefix);
  }
}

static void clearDriveMapping(const char* drive) {
  char keybuf[10];
  if (bk_str_startswith(drive, str2ram("URI"))) {
    bk_strcpy(keybuf, drive);
    keybuf[4] = 0;
  } else if (bk_str_startswith(drive, str2ram("CS1"))) {
    bk_strcpy(keybuf, drive);
    bk_strcpy(keybuf+3, str2ram("_FILE"));
  } else {
    bk_strcpy(keybuf, drive);
    bk_strcpy(keybuf+4, str2ram("_DIR"));
  }

  char empty[1];
  empty[0] = 0;

  writeConfigItem(keybuf, empty);
}

void onLineAuto(char* linebuf, char* extra) {
  if (bk_str_startswith(linebuf, str2ram("AUTO="))) {
    char* val = bk_strtok(linebuf, '=');
    val = bk_strtok(0, ' ');
    tputs_rom("AUTO ");
    bk_tputs_ram(val);
    bk_tputc('\n');
  }
}

static void setAutoMap() {
  char* onoff = bk_strtok(0, ' ');
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

static int __attribute__((noinline)) validDrive(const unsigned char *keybuf, const unsigned char *prefix, char min, char max, int pl)
{
  int dl = bk_strlen(keybuf);
  int form = (dl == (pl+2) && keybuf[dl-1] == '.') || (dl == (pl+1));
  return bk_str_startswith(keybuf, str2ram(prefix)) &&
       keybuf[pl] >= min &&
       keybuf[pl] <= max &&
       form;
}

static int __attribute__((noinline)) isMappable(const char *drive)
{
  return validDrive(drive, "DSK", '1', '9', 3) || 
         validDrive(drive, "URI", '1', '3', 3) ||
         validDrive(drive, "CS", '1', '1', 2);
}

static void setDriveMapping(const char* drive, const char* path) {
  unsigned char keybuf[10];
  int dlen = bk_strlen(drive);
  bk_strncpy(keybuf, (char*) drive, dlen < 9 ? dlen : 9);

  if (!isMappable(keybuf))
  {
    tputs_rom("error, bad drive specification\n");
    return;
  }

  int diskmapping = 0; // cassette or disk
  if (bk_str_startswith(keybuf, str2ram("DSK"))) {
    bk_strcpy(keybuf+4, str2ram("_DIR"));
    diskmapping = 1;
  } else if (bk_str_startswith(keybuf, str2ram("CS1"))) {
    bk_strcpy(keybuf+3, str2ram("_FILE"));
    diskmapping = 1;
  } else if (bk_str_startswith(keybuf, str2ram("URI"))) {
    keybuf[4] = 0;
  }

  char namebuf[256];

  if (diskmapping) {
    bk_strcpy(namebuf, str2ram("TIPI"));
    struct DeviceServiceRoutine* dsr = bk_findDsr(namebuf, 0);
    bk_strcat(namebuf, str2ram("."));
    if (bk_str_equals((char*) path, str2ram("TIPI.")) || bk_str_equals((char*) path, str2ram("."))) {
      bk_strcpy(namebuf, ".");
    } else if (bk_str_startswith(path, str2ram("TIPI."))) {
      bk_strcpy(namebuf, path+5);
    } else {
      bk_strcpy(namebuf, path);
    }
  } else {
    if (!bk_str_startswith(path, str2ram("HTTP"))) {
      tputs_rom("error path must be a URL prefix\n");
      return;
    }
    bk_strcpy(namebuf, path);
  }
  writeConfigItem(keybuf, namebuf);
}

void handleTipimap() {
  //  [/c] [drive] [path]
  char* peek = bk_strtokpeek(0, ' ');
  int clear = 0 == bk_strcmpi(str2ram("/c"), peek);

  if (clear) {
    bk_strtok(0, ' '); // consume the optional /c
  }

  char* drive = bk_strtok(0, ' ');
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
        char* peek = bk_strtokpeek(0, ' ');
        if (peek) {
          if (bk_str_startswith(drive, str2ram("DSK")) || bk_str_startswith(drive, str2ram("CS1"))) {
            struct DeviceServiceRoutine *dsr;
            char path[256];
            bk_parsePathParam(0, &dsr, path, PR_OPTIONAL);
            if (dsr == 0)
            {
              tputs_rom("bad path specified\n");
            } else {
              setDriveMapping(drive, path);
            }
          } else {
            char* tok = bk_strtok(0, ' ');
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
