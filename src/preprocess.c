#include "banks.h"
#define MYBANK BANK(4)

#include "preprocess.h"
#include "strutil.h"
#include "string.h"
#include "variables.h"
#include "aliases.h"
#include "terminal.h"

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
  char* alias_name = bk_strtok(command, ' ');
  if (alias_name == 0) {
    resolved[0] = 0;
    return;
  }

  // The tokenizer leaves lasts at the first argument (or at the terminator).
  char* args = lasts;

  char alias_buf[MAX_ALIAS_VAL + 1];
  char* alias_value = alias_buf;
  // if the alias is not defined, use the name to rebuild the command
  if (!alias_get(alias_name, alias_buf, sizeof(alias_buf))) {
    alias_value = alias_name;
  }
  int value_len = bk_strlen(alias_value);
  
  bk_strncpy(resolved, alias_value, 256);

  if (args != 0 && *args != 0) {
    // Put the space back in between the alias value and the additional args.
    resolved[value_len] = ' ';
    bk_strncpy(resolved + value_len + 1, args, 256);
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
      char val[MAX_VAR_VAL + 1];
      if (var_get(varname, val, sizeof(val))) {
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