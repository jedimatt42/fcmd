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
#include "b11_handleExecutable.h"
#include <string.h>

#define MATCH(x,y) (!(bk_strcmpi(x,y)))

#define COMMAND(x, y) if (MATCH(tok, str2ram(x))) y();

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

/*
   this has to compare against the list of safe commands so all others,
   scripts, and executables are considered
*/
int must_close_command(char* buffer) {
  char* tok = bk_strtokpeek(buffer, ' ');
  if (MATCH(tok, str2ram("bar"))) return 0;
  if (MATCH(tok, str2ram("cls"))) return 0;
  if (MATCH(tok, str2ram("cd"))) return 0;
  if (MATCH(tok, str2ram("checksum"))) return 0;
  if (MATCH(tok, str2ram("color"))) return 0;
  if (MATCH(tok, str2ram("copy"))) return 0;
  if (MATCH(tok, str2ram("date"))) return 0;
  if (MATCH(tok, str2ram("delete"))) return 0;
  if (MATCH(tok, str2ram("dir"))) return 0;
  if (MATCH(tok, str2ram("drives"))) return 0;
  if (MATCH(tok, str2ram("echo"))) return 0;
  if (MATCH(tok, str2ram("env"))) return 0;
  if (MATCH(tok, str2ram("if"))) return 0;
  if (MATCH(tok, str2ram("help"))) return 0;
  if (MATCH(tok, str2ram("history"))) return 0;
  if (MATCH(tok, str2ram("lvl2"))) return 0;
  if (MATCH(tok, str2ram("mkdir"))) return 0;
  if (MATCH(tok, str2ram("protect"))) return 0;
  if (MATCH(tok, str2ram("readkey"))) return 0;
  if (MATCH(tok, str2ram("rename"))) return 0;
  if (MATCH(tok, str2ram("rmdir"))) return 0;
  if (MATCH(tok, str2ram("sysinfo"))) return 0;
  if (MATCH(tok, str2ram("tipibeeps"))) return 0;
  if (MATCH(tok, str2ram("type"))) return 0;
  if (MATCH(tok, str2ram("unprotect"))) return 0;
  if (MATCH(tok, str2ram("ver"))) return 0;
  if (MATCH(tok, str2ram("width"))) return 0;
  return 1;
}

// NOTE command handle functions in bank 0 do not need bk_ banking stub

void handleCommand(char *buffer) {
  if (buffer[0] == 0) {
    return;
  }
  request_break = 0;

  // perform any escaping, variable substitutions, etc...
  char* procbuf = bk_preprocess(buffer);

  char* tok = bk_strtok(procbuf, ' ');
  COMMAND("bar", bk_handleBar)
  else COMMAND("cls", handleCls)
  else COMMAND("cd", bk_handleCd)
  else COMMAND("cfmount", bk_handleCFMount)
  else COMMAND("checksum", bk_handleChecksum)
  else COMMAND("color", bk_handleColor)
  else COMMAND("copy", bk_handleCopy)
  else COMMAND("date", bk_handleDate)
  else COMMAND("debug", bk_handleDebug)
  else COMMAND("delete", bk_handleDelete)
  else COMMAND("dir", bk_handleDir)
  else COMMAND("drives", bk_handleDrives)
  else COMMAND("echo", bk_handleEcho)
  else COMMAND("env", bk_handleEnv)
  else COMMAND("exit", handleExit)
  else COMMAND("fg99", handleFg99)
  else COMMAND("goto", bk_handleGoto)
  else COMMAND("if", bk_handleIf)
  else COMMAND("help", bk_handleHelp)
  else COMMAND("history", bk_handleHistory)
  else COMMAND("load", handleLoad)
  else COMMAND("lvl2", bk_handleLvl2)
  else COMMAND("mkdir", bk_handleMkdir)
  else COMMAND("protect", bk_handleProtect)
  else COMMAND("readkey", bk_handleReadkey)
  else COMMAND("rename", bk_handleRename)
  else COMMAND("rmdir", bk_handleRmdir)
  else COMMAND("sysinfo", bk_handleSysInfo)
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
  } else if (isAssignment(buffer)) {
    char* name = bk_strtok(buffer, '=');
    char* value = bk_strtokpeek(0, 0); // to end of line
    bk_vars_set(name, value);
  } else {
    if (tok) {
      bk_handleExecutable(tok);
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

void parsePathParam(char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements) {
  filterglob[0] = 0;
  buffer[0] = 0; // null terminate so later we can tell if it is prepared or not.
  char *path = bk_strtok(str_in, ' ');
  *dsr = currentDsr;
  if (path == 0) {
    if (requirements & PR_REQUIRED) {
      *dsr = 0; // set dsr pointer to null to indicate missing required parameter.
      return;
    }
    bk_strcpy(buffer, currentPath); // if not required, use current path
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
        bk_strcpy(buffer, currentPath);
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
        bk_strcpy(buffer, path);
      } else {
        if (buffer[0] == 0) {
          bk_strcpy(buffer, path);
        }
      }
    }
  }
  // separate path and filter if wildcards are supported.
  if (requirements & PR_WILDCARD) {
    int len = bk_strlen(buffer);
    int dotidx = bk_lindexof(buffer, '.', len);

    bk_strcpy(filterglob, buffer+dotidx+1);
    buffer[dotidx] = 0;
  }
}

// if filename matches current glob, or no glob is specified, return true.
int globMatches(char* filename) {
  if (filterglob[0] == 0) {
    return 1;
  }

  int prelen = bk_indexof(filterglob, '*');
  char prefix[12];
  bk_strncpy(prefix, filterglob, prelen);

  char suffix[12];
  bk_strcpy(suffix, filterglob+prelen+1);

  return bk_str_startswith(filename, prefix) && bk_str_endswith(filename, suffix);
}
