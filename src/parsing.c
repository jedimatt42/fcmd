#include "banks.h"
#define MYBANK BANK(10)

#include "parsing.h"
#include "main.h"
#include "globals.h"
#include "redirect.h"
#include "pipeline.h"
#include "commands.h"
#include "dsrutil.h"
#include "strutil.h"
#include "terminal.h"
#include "labellist.h"
#include "variables.h"
#include "preprocess.h"
#include "handleExecutable.h"
#include <string.h>

#define MATCH(x,y) (!(bk_strcmpi(x,y)))

static int result;
#define COMMAND(x, y) if (MATCH(tok, str2ram(x))) { result = y(); }

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
  if (isAssignment(buffer)) {
    return 0;
  }

  char* tok = bk_strtokpeek(buffer, ' ');
  if (MATCH(tok, str2ram("alias"))) return 0;
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
  if (MATCH(tok, str2ram("goto"))) return 0;
  if (MATCH(tok, str2ram("if"))) return 0;
  if (MATCH(tok, str2ram("help"))) return 0;
  if (MATCH(tok, str2ram("history"))) return 0;
  if (MATCH(tok, str2ram("lvl2"))) return 0;
  if (MATCH(tok, str2ram("mkdir"))) return 0;
  if (MATCH(tok, str2ram("palette"))) return 0;
  if (MATCH(tok, str2ram("protect"))) return 0;
  if (MATCH(tok, str2ram("pwd"))) return 0;
  if (MATCH(tok, str2ram("readkey"))) return 0;
  if (MATCH(tok, str2ram("rename"))) return 0;
  if (MATCH(tok, str2ram("rdemdk"))) return 0;
  if (MATCH(tok, str2ram("rdpart"))) return 0;
  if (MATCH(tok, str2ram("rdvol"))) return 0;
  if (MATCH(tok, str2ram("rmdir"))) return 0;
  if (MATCH(tok, str2ram("sysinfo"))) return 0;
  if (MATCH(tok, str2ram("tipibeeps"))) return 0;
  if (MATCH(tok, str2ram("type"))) return 0;
  if (MATCH(tok, str2ram("unprotect"))) return 0;
  if (MATCH(tok, str2ram("ver"))) return 0;
  if (MATCH(tok, str2ram("width"))) return 0;

  // for anything else require closing scripts
  return 1;
}

static void set_command_status(int status) {
  char* value = bk_uint2str((unsigned int) status);
  char current[MAX_VAR_VAL + 1];
  if (!bk_vars_get(str2ram("STATUS"), current, sizeof(current))
      || 0 != bk_strcmp(current, value)) {
    bk_vars_set(str2ram("STATUS"), value);
  }
}

static int command_depth;
static int pipeline_depth;

static char* find_pipe(char* s) {
  int escaped = 0;
  while (*s) {
    if (!escaped && *s == '\\') {
      escaped = 1;
      s++;
      continue;
    }
    if (!escaped && *s == '|') {
      return s;
    }
    escaped = 0;
    s++;
  }
  return 0;
}

static void unescape_pipes(char* s) {
  char* read = s;
  char* write = s;
  while (*read) {
    if (*read == '\\' && read[1] == '|') {
      *write++ = '|';
      read += 2;
    } else {
      *write++ = *read++;
    }
  }
  *write = 0;
}

static int executeSingle(char* buffer, const char* forced_path, int* redirect_failed) {
  result = 0;
  *redirect_failed = 0;
  if (buffer[0] == 0) {
    return 0;
  }

  int owns_redirect;
  if (forced_path != 0) {
    bk_redirect_unescape(buffer);
    owns_redirect = bk_redirect_force(forced_path, 0);
  } else {
    owns_redirect = bk_redirect_prepare(buffer);
  }
  if (owns_redirect < 0) {
    result = 1;
    *redirect_failed = 1;
    set_command_status(result);
    return result;
  }

  // perform any escaping, variable substitutions, etc...
  char* procbuf = bk_preprocess(buffer);

  char* tok = bk_strtok(procbuf, ' ');
  if (tok) {
    COMMAND("alias", bk_handleAlias)
    else COMMAND("bar", bk_handleBar)
    else COMMAND("cls", bk_handleCls)
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
    else COMMAND("ed", bk_handleEd)
    else COMMAND("env", bk_handleEnv)
    else COMMAND("exit", bk_handleExit)
    else COMMAND("f18a", bk_handleF18A)
    else COMMAND("fg99", bk_handleFg99)
    else COMMAND("goto", bk_handleGoto)
    else COMMAND("if", bk_handleIf)
    else COMMAND("help", bk_handleHelp)
    else COMMAND("history", bk_handleHistory)
    else COMMAND("load", bk_handleLoad)
    else COMMAND("lvl2", bk_handleLvl2)
    else COMMAND("mkdir", bk_handleMkdir)
    else COMMAND("palette", bk_handlePalette)
    else COMMAND("protect", bk_handleProtect)
    else COMMAND("pwd", bk_handlePwd)
    else COMMAND("readkey", bk_handleReadkey)
    else COMMAND("rename", bk_handleRename)
    else COMMAND("rdemdk", bk_handleRDEmdk)
    else COMMAND("rdpart", bk_handleRDPart)
    else COMMAND("rdvol", bk_handleRDVol)
    else COMMAND("rmdir", bk_handleRmdir)
    else COMMAND("sysinfo", bk_handleSysInfo)
    else COMMAND("tipibeeps", bk_handleTipiBeeps)
    else COMMAND("tipihalt", bk_handleTipiHalt)
    else COMMAND("tipimap", bk_handleTipimap)
    else COMMAND("tipireboot", bk_handleTipiReboot)
    else COMMAND("type", bk_handleType)
    else COMMAND("unprotect", bk_handleUnprotect)
    else COMMAND("ver", bk_handleVer)
    else COMMAND("width", bk_handleWidth)
    else COMMAND("xb", bk_handleXb)
    else if (tok[bk_strlen(tok)-1] == ':') {
      if (scripton) {
        tok[bk_strlen(tok)-1] = 0; // shorten to just the name
        bk_labels_add(tok, *goto_line_ref);
      } else {
        tputs_rom("error, label only supported in script\n");
      }
    } else if (isAssignment(procbuf)) {
      char* name = bk_strtok(procbuf, '=');
      char* value = bk_strtokpeek(0, 0); // to end of line
      bk_vars_set(name, value);
    } else {
      result = bk_handleExecutable(tok);
    }
  }

  if (owns_redirect) {
    int finish_status = bk_redirect_finish();
    if (finish_status != 0) {
      *redirect_failed = 1;
    }
    if (result == 0) {
      result = finish_status;
    }
  }

  set_command_status(result);
  return result;
}

int executeStage(char* buffer, const char* forced_path, int* redirect_failed) {
  return executeSingle(buffer, forced_path, redirect_failed);
}

void setCommandStatus(int status) {
  set_command_status(status);
}

static int runCommandLine(char* buffer) {
  char* pipe = find_pipe(buffer);

  if (pipe == 0) {
    unescape_pipes(buffer);
    int fatal = 0;
    return executeSingle(buffer, 0, &fatal);
  }

  if (pipeline_depth > 0) {
    tputs_rom("error, nested pipelines are not supported\n");
    set_command_status(1);
    return 1;
  }

  pipeline_depth++;
  int status = bk_runPipeline(buffer);
  pipeline_depth--;
  return status;
}

// NOTE command handle functions in bank 0 do not need bk_ banking stub

int handleCommand(char *buffer) {
  if (buffer[0] == 0) {
    return 0;
  }

  if (command_depth == 0) {
    request_break = 0;
  }
  command_depth++;

  int status = runCommandLine(buffer);

  command_depth--;
  return status;
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

void path_parse(char* str_in, struct DeviceServiceRoutine** dsr, char* buffer, int requirements) {
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

    if (dotidx >= 0) {
      bk_strcpy(filterglob, buffer+dotidx+1);
      buffer[dotidx] = 0;
    } else {
      /* Device-only path; there is no filename component. */
      filterglob[0] = 0;
    }
  }
}

// if filename matches current glob, or no glob is specified, return true.
int globMatches(char* filename) {
  if (filterglob[0] == 0) {
    return 1;
  }

  int prelen = bk_indexof(filterglob, '*');
  if (prelen == -1) {
    return 0 == bk_strcmp(filename, filterglob);
  } else {
    char prefix[12];
    bk_strncpy(prefix, filterglob, prelen);

    char suffix[12];
    bk_strcpy(suffix, filterglob+prelen+1);

    return bk_str_startswith(filename, prefix) && bk_str_endswith(filename, suffix);
  }
}
