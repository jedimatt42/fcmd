#include "banks.h"
#define MYBANK BANK_0

#include "b0_parsing.h"
#include "b0_main.h"
#include "b0_globals.h"
#include "commands.h"
#include "b2_dsrutil.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include <string.h>

#define MATCH(x,y) (!(strcmpi(x,y)))

#define COMMAND(x, y) if (MATCH(tok, x)) y();

// NOTE command handle functions in bank 0 do not need bk_ banking stub

void handleCommand(char *buffer) {
  char* tok = strtok(buffer, " ");
  COMMAND("cls", handleCls)
  else COMMAND("cd", bk_handleCd)
  else COMMAND("checksum", bk_handleChecksum)
  else COMMAND("copy", bk_handleCopy)
  else COMMAND("delete", handleDelete)
  else COMMAND("dir", bk_handleDir)
  else COMMAND("drives", handleDrives)
  else COMMAND("echo", handleEcho)
  else COMMAND("exit", handleExit)
  else COMMAND("fg99", handleFg99)
  else COMMAND("help", bk_handleHelp)
  else COMMAND("load", handleLoad)
  else COMMAND("lvl2", bk_handleLvl2)
  else COMMAND("mkdir", bk_handleMkdir)
  else COMMAND("protect", bk_handleProtect)
  else COMMAND("rename", handleRename)
  else COMMAND("rmdir", handleRmdir)
  else COMMAND("tipibeeps", playtipi)
  else COMMAND("tipimap", handleTipimap)
  else COMMAND("type", handleType)
  else COMMAND("unprotect", handleUnprotect)
  else COMMAND("ver", titleScreen)
  else COMMAND("width", handleWidth)
  else {
    tputs("unknown command: ");
    tputs(tok);
    tputc('\n');
  }
}

int parsePath(char* path, char* devicename) {
  char workbuf[14];
  int crubase = 0;
  strncpy(workbuf, path, 14);
  char* tok = strtok(workbuf, ". ");
  if (tok != 0 && tok[0] == '1' && strlen(tok) == 4) {
    crubase = htoi(tok);
    tok = strtok(0, ". ");
    strcpy(devicename, tok);
  } else {
    strcpy(devicename, tok);
  }
  return crubase;
}

void parsePathParam(struct DeviceServiceRoutine** dsr, char* buffer, int requirements) {
  buffer[0] = 0; // null terminate so later we can tell if it is prepared or not.
  char* path = strtok(0, " ");
  *dsr = currentDsr;
  if (path == 0) {
    if (requirements & PR_REQUIRED) {
      *dsr = 0; // set dsr pointer to null to indicate missing required parameter.
      return;
    }
    path = currentPath; // if not required, use current path
  } else {
    char devicename[8];
    if (0 == strcmp("..", path)) {
      int ldot = lindexof(currentPath, '.', strlen(currentPath) - 2);
      if (ldot == -1) {
        *dsr = 0;
        tputs("No parent folder\n");
        return;
      }
      strncpy(buffer, currentPath, ldot + 1);
      return;
    } else {
      int crubase = parsePath(path, devicename);
      *dsr = bk_findDsr(devicename, crubase);
      if (*dsr == 0) {
        // not a base device, so try subdir
        strcpy(buffer, currentPath);
        strcat(buffer, path);
        crubase = parsePath(buffer, devicename);
        *dsr = bk_findDsr(devicename, crubase);
        // if still not found, then give up.
        if (*dsr == 0) {  
          tputs("device not found.\n");
          return;
        }
      }
      if (crubase != 0) {
        path = strtok(path, ".");
        path = strtok(0, " ");
      }
    }
  }
  // Todo: test for existance and matching requirements
  if (buffer[0] == 0) {
    strcpy(buffer, path);
  }
}
