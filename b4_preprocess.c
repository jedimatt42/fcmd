#include "banks.h"
#define MYBANK BANK(4)

#include "b4_preprocess.h"
#include "b1_strutil.h"
#include "string.h"
#include "b4_variables.h"
#include "b4_aliases.h"

const char VARNAME_CLASS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

char* sub_vars_and_escapes(char* buf);
void replace_alias(char* command, char* resolved);

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

char* preprocess(char* buf) {
  char resolved[256];
  replace_alias(buf, resolved);
  return sub_vars_and_escapes(resolved);
}

void replace_alias(char* command, char* resolved) {
  char* alias_name = bk_strtok(command, ' ');
  char* alias_val = alias_get(alias_name);
  if ((int)alias_val == -1) {
    bk_strncpy(resolved, alias_name, 256);
  } else {
    bk_strncpy(resolved, alias_val, 256);
  }
  int len = bk_strlen(resolved);
  char* cursor = resolved + len;
  *cursor++ = ' ';
  bk_strncpy(cursor, bk_strtok(0, 0), 256 - len - 1);
}

// perform escaping and variable substitutions on command buffer
char* sub_vars_and_escapes(char* buf) {
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