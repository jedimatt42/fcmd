#include "banks.h"
#define MYBANK BANK(4)

#include "b4_preprocess.h"
#include "b1_strutil.h"
#include "string.h"
#include "b4_variables.h"
#include "b4_aliases.h"
#include "b8_terminal.h"

const char VARNAME_CLASS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

void replace_alias(char* command, char* resolved);
char* phase2_preprocess(char* buf);
char* alias_preprocess(char* buf);

char* preprocess(char* buf) {
  return alias_preprocess(buf);
}

// return 1 if c is in set, otherwise return 0
static int charin(char c, char* set) {
  int i = 0;
  while(set[i] != 0) {
    if (set[i] == c) {
      return 1;
    }
    i++;
  }
  return 0;
}

static char procbuf[256];

char* alias_preprocess(char* buf) {
  char resolved[256];
  replace_alias(buf, resolved);
  return phase2_preprocess(resolved);
}

void replace_alias(char* command, char* resolved) {
  int cmdlen = bk_strlen(command);

  char* alias_name = bk_strtok(command, ' ');

  int name_len = bk_strlen(alias_name);
  char* alias_value = alias_get(alias_name);
  // if the alias is not defined, use the name to rebuild the command
  if ((int)alias_value == -1) {
    alias_value = alias_name;
  }
  int value_len = bk_strlen(alias_value);
  
  bk_strncpy(resolved, alias_value, 256);

  if (cmdlen > name_len) {
    // append the rest of the command (the tokenizer nulled out the leading space, so there is a
    // +1 to get to the args.
    // we also have to put the space back in between the alias value and the additional args
    resolved[value_len] = ' ';
    bk_strncpy(resolved + value_len + 1, command + name_len + 1, 256);
  }
}

// perform escaping and variable substitutions on command buffer
char* phase2_preprocess(char* buf) {
  bk_strset(procbuf, 0, 256);
  int i = 0;
  int pi = 0;
  while(buf[i] != 0) {
    if (buf[i] == '\\') {
      switch(buf[i+1]) {
        case 'e': // replace with ESCAPE
          procbuf[pi] = 27;
          i++;
          break;
        case '\\': // replace double escape with single slash
          // slash is already in place
          i++;
          break;
        default:
          break;
      }
    } else if (buf[i] == '$') {
      // attempt variable substitution
      int ve = i+1;
      char varname[MAX_VAR_NAME+1];
      bk_strset(varname, 0, MAX_VAR_NAME+1);
      if (buf[ve] == '(') {
        ve++;
        int vn = 0;
        while(charin(buf[ve], (char*) VARNAME_CLASS)) {
          if (vn < MAX_VAR_NAME) {
            varname[vn] = buf[ve];
          }
          ve++;
          vn++;
        }
        if (buf[ve] != ')') {
          // short out the substitution...
          varname[0] = 0; // should really just be an early error...
        } else {
          ve++;
        }
      } else {
        int vn = 0;
        while(charin(buf[ve], (char*) VARNAME_CLASS)) {
          if (vn < MAX_VAR_NAME) {
            varname[vn] = buf[ve];
          }
          ve++;
          vn++;
        }
      }
      char* val = vars_get(varname);
      if ((int) val != -1) {
        bk_strcpy(&procbuf[pi], val);
        i = ve - 1;
        pi += bk_strlen(val) - 1;
      } else {
        // if variable is not set, then leave it unsubstituted.
        procbuf[pi] = buf[i];
      }
    } else {
      procbuf[pi] = buf[i];
    }
    i++;
    pi++;
  }
  procbuf[pi] = 0;
  return procbuf;
}