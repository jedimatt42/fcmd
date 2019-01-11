#include "main.h"

#include "conio.h"
#include "strutil.h"

const char* HELP_COMMANDS =
"Show help\n"
"\n"
"syntax: 'help' (cmd)\n"
"\n"
"cmd = 'cd' 'checksum' 'copy' 'delete' 'dir' 'drives' 'exit' 'lvl2' 'mkdir' 'protect' 'unprotect' 'rename' 'rmdir' 'ver' 'width'\n";

const char* HELP_CD =
"Change directory\n"
"\n"
"syntax: 'cd' (path|..)\n";

const char* HELP_CHECKSUM =
"Checksum a file\n"
"\n"
"syntax: 'checksum' (name)\n";

const char* HELP_COPY =
"Copy file from current location to specified path\n"
"\n"
"syntax: 'copy' (name) (path)\n";

const char* HELP_DELETE =
"Delete a file\n"
"\n"
"syntax: 'delete' (name)\n"; 

const char* HELP_DIR = 
"List files in current directory or given directory\n"
"\n"
"syntax: 'dir' [path]\n";

const char* HELP_DRIVES =
"List drives along with disambiguation\n"
"\n"
"syntax: 'drives'\n";

const char* HELP_EXIT =
"Exit TIPIFM\n"
"\n"
"syntax: 'exit'\n";

const char* HELP_LVL2 =
"Display Level 2 IO routines for a card\n"
"\n"
"syntax: 'lvl2' (crubase)\n";

const char* HELP_MKDIR =
"Create a sub-directory in the current location\n"
"\n"
"syntax: 'mkdir' (name)\n";

const char* HELP_PROTECT =
"Set protect bit on a file\n"
"\n"
"syntax: 'protect' (name)\n";

const char* HELP_RENAME =
"Rename a file or directory\n"
"\n"
"syntax: 'rename' (oldname) (newname)\n";

const char* HELP_RMDIR =
"Remove a directory\n"
"\n"
"syntax: 'rmdir' (name)\n";

const char* HELP_UNPROTECT =
"Clear protect bit on a file\n"
"\n"
"syntax: 'unprotect' (name)\n";

const char* HELP_VER =
"Print information about TIPIFM\n"
"\n"
"syntax: 'ver'\n";

const char* HELP_WIDTH =
"Change display width. TIPIFM starts in 40 column mode\n"
"\n"
"syntax: 'width' (40|80)\n";

#define CMD_HELP(x,y) else if (0==strcmpi(x,tok)) { cprintf(y); }

void handleHelp() {
  char* tok = strtok(0, " ");
  if (tok == 0) {
    cprintf(HELP_COMMANDS);
  } 
  CMD_HELP("cd", HELP_CD)
  CMD_HELP("checksum",HELP_CHECKSUM)
  CMD_HELP("copy",HELP_COPY)
  CMD_HELP("delete",HELP_DELETE)
  CMD_HELP("dir",HELP_DIR)
  CMD_HELP("drives",HELP_DRIVES)
  CMD_HELP("exit",HELP_EXIT)
  CMD_HELP("lvl2",HELP_LVL2)
  CMD_HELP("mkdir",HELP_MKDIR)
  CMD_HELP("protect",HELP_PROTECT)
  CMD_HELP("rename",HELP_RENAME)
  CMD_HELP("rmdir",HELP_RMDIR)
  CMD_HELP("unprotect",HELP_UNPROTECT)
  CMD_HELP("ver",HELP_VER)
  CMD_HELP("width",HELP_WIDTH)
  else cprintf("unknown command: %s\n", tok);
}
