#include "banks.h"
#define MYBANK BANK(0)

#include "b0_parsing.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "commands.h"
#include "b2_dsrutil.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b4_labellist.h"
#include "b4_variables.h"
#include "b4_preprocess.h"
#include <string.h>

#define MATCH(x,y) (!(bk_strcmpi(str2ram(x),y)))

#define COMMAND(x, y) if (MATCH(tok, x)) y();

static int isAssignment(char* str) {
  int i = 0;
  while(str[i] != 0) {
    if (str[i] == '=') {
      return 1;
    }
    i++;
  }
  return 0;
}

// NOTE command handle functions in bank 0 do not need bk_ banking stub

void handleCommand(char *buffer) {
  if (buffer[0] == 0) {
    return;
  }

  // perform any escaping, variable substitutions, etc...
  char* procbuf = bk_preprocess(buffer);

  char* tok = bk_strtok(procbuf, ' ');
  COMMAND("call", handleCall)
  else COMMAND("cls", handleCls)
  else COMMAND("cd", bk_handleCd)
  else COMMAND("cfmount", bk_handleCFMount)
  else COMMAND("checksum", bk_handleChecksum)
  else COMMAND("color", bk_handleColor)
  else COMMAND("copy", bk_handleCopy)
  else COMMAND("delete", bk_handleDelete)
  else COMMAND("dir", bk_handleDir)
  else COMMAND("drives", bk_handleDrives)
  else COMMAND("echo", bk_handleEcho)
  else COMMAND("env", bk_handleEnv)
  else COMMAND("exit", handleExit)
  else COMMAND("fg99", handleFg99)
  else COMMAND("ftp", bk_handleFtp)
  else COMMAND("goto", bk_handleGoto)
  else COMMAND("if", bk_handleIf)
  else COMMAND("help", bk_handleHelp)
  else COMMAND("load", handleLoad)
  else COMMAND("lvl2", bk_handleLvl2)
  else COMMAND("mkdir", bk_handleMkdir)
  else COMMAND("protect", bk_handleProtect)
  else COMMAND("readkey", bk_handleReadkey)
  else COMMAND("rename", bk_handleRename)
  else COMMAND("rmdir", bk_handleRmdir)
  else COMMAND("tipibeeps", playtipi)
  else COMMAND("tipihalt", bk_handleTipiHalt)
  else COMMAND("tipimap", bk_handleTipimap)
  else COMMAND("tipireboot", bk_handleTipiReboot)
  else COMMAND("type", bk_handleType)
  else COMMAND("unprotect", bk_handleUnprotect)
  else COMMAND("ver", titleScreen)
  else COMMAND("width", handleWidth)
  else COMMAND("xb", bk_handleXb)
  else if (tok[bk_strlen(tok)-1] == ':') {
    if (scripton) {
      tok[bk_strlen(tok)-1] = 0; // shorten to just the name
      bk_labels_add(tok, lineno);
    } else {
      tputs_rom("error, label only supported in script\n");
    }
  } else if (isAssignment(tok)) {
    char* name = bk_strtok(procbuf, '=');
    char* value = bk_strtokpeek(0, 0); // to end of line
    bk_vars_set(name, value);
  } else {
    if (tok) {
      tputs_rom("unknown command: ");
      bk_tputs_ram(tok);
      bk_tputc('\n');
    }
  }
}

int parsePath(char* path, char* devicename) {
  char workbuf[14];
  int crubase = 0;
  bk_strncpy(workbuf, path, 13);
  char* tok = bk_strtok(workbuf, '.');
  if (tok != 0 && tok[0] == '1' && bk_strlen(tok) == 4) {
    crubase = bk_htoi(tok);
    tok = bk_strtok(0, '.');
    bk_strncpy(devicename, tok, 8);
  } else {
    bk_strncpy(devicename, tok, 8);
  }
  return crubase;
}

void parsePathParam(struct DeviceServiceRoutine** dsr, char* buffer, int requirements) {
  filterglob[0] = 0;
  buffer[0] = 0; // null terminate so later we can tell if it is prepared or not.
  char* path = bk_strtok(0, ' ');
  *dsr = currentDsr;
  if (path == 0) {
    if (requirements & PR_REQUIRED) {
      *dsr = 0; // set dsr pointer to null to indicate missing required parameter.
      return;
    }
    strcpy(buffer, currentPath); // if not required, use current path
    return;
  } else {
    char devicename[8];
    if (0 == bk_strcmp(str2ram(".."), path)) {
      int ldot = bk_lindexof(currentPath, '.', bk_strlen(currentPath) - 2);
      if (ldot == -1) {
        *dsr = 0;
        tputs_rom("No parent folder\n");
        return;
      }
      bk_strncpy(buffer, currentPath, ldot);
      return;
    } else {
      int crubase = parsePath(path, devicename);
      *dsr = bk_findDsr(devicename, crubase);
      if (*dsr == 0) {
        // not a base device, so try subdir
        strcpy(buffer, currentPath);
        bk_strcat(buffer, path);
        parsePath(buffer, devicename);
        *dsr = bk_findDsr(devicename, currentDsr->crubase);
        // if still not found, then give up.
        if (*dsr == 0) {
          tputs_rom("device not found.\n");
          return;
        }
        // at this stage, buffer is set with result device name.
      }
      if (crubase != 0) {
        path = bk_strtok(path, '.');
        path = bk_strtok(0, ' ');
        strcpy(buffer, path);
      } else {
        if (buffer[0] == 0) {
          strcpy(buffer, path);
        }
      }
    }
  }
  // separate path and filter if wildcards are supported.
  if (requirements & PR_WILDCARD) {
    int len = bk_strlen(buffer);
    int dotidx = bk_lindexof(buffer, '.', len);

    strcpy(filterglob, buffer+dotidx+1);
    buffer[dotidx] = 0;
  }
}

// if filename matches current glob, or no glob is specified, return true.
int globMatches(char* filename) {
  if (filterglob[0] == 0) {
    return 1;
  }

  int prelen = indexof(filterglob, '*');
  char prefix[12];
  bk_strncpy(prefix, filterglob, prelen);

  char suffix[12];
  strcpy(suffix, filterglob+prelen+1);

  return bk_str_startswith(filename, prefix) && bk_str_endswith(filename, suffix);
}
