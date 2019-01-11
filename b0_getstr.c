#include "getstr.h"
#include "strutil.h"
#include <conio.h>
#include <string.h>
#include <kscan.h>

static unsigned char mycgetc();
#define CUR_OVERWRITE 219
#define CUR_INSERT '_'
unsigned char cursor = CUR_OVERWRITE;

int insertMode = 0;


// Requirements:
//  Act like TI BASIC keyboard input as much as logical
//
//  FCTN-1 DEL - Delete a character from input line
//  FCTN-2 INS - Characters are inserted until next FCTN key
//  FCTN-3 ERASE - Erases the entire line
//  FCTN-4 CLEAR - return empty string.
//  FCTN-D RIGHT - move forward, inserting spaces if necessary.
//  FCTN-S LEFT  - if backspace is set, then delete a character to the left.
//                 else, move cursor to the left.
void getstr(int x, int y, char* var, int limit, int backspace) {
  gotoxy(x,y);
  cputs(var);

  unsigned char key = 0;
  int idx = strlen(var);
  while(key != 13) {
    gotoxy(x+idx,y);
    key = mycgetc();

    switch(key) {
      case 3: // F1 - delete
          strcpy(var+idx, var+idx+1);
          gotoxy(x+idx,y);
          cputs(var+idx);
          cputc(' ');
        break;
      case 4: // F2 - insert
          if (insertMode) {
            insertMode = 0;
            cursor = CUR_OVERWRITE;
          } else {
            insertMode = 1;
            cursor = CUR_INSERT;
          }
        break;
      case 7: // F3 - erase line
        idx = strlen(var);
        strset(var, 0, limit);
        cclearxy(x, y, idx);
        gotoxy(x,y);
        idx = 0;
        break;
      case 2: // F4 - clear
        var[0] = 0;
        key = 13; // get out.
        break;
      case 8: // left arrow
        if (idx != 0) {
          idx--;
          if (backspace) {
            cputc(' ');
            strcpy(var+idx, var+idx+1);
            gotoxy(x+idx,y);
            cputs(var+idx);
          }
        }
        break;
      case 9: // right arrow
        if (idx < limit) {
          if (var[idx] == 0) {
            cputc(' ');
            var[idx] = ' ';
          }
          idx++;
        }
        break;
      case 13: // return
        break;
      default: // alpha numeric
        if (key >= 32 && key <= 122) {
          if (insertMode) {
            int end = strlen(var);
            if (end != limit) {
              for(int i=end; i>idx; i--) {
                var[i] = var[i-1];
              }
              cputs(var+idx);
            }
          }
          gotoxy(x+idx,y);
          cputc(key);
          var[idx++] = key;
        }
        break;
    }
    if (idx >= limit) {
      idx = limit - 1;
    }
  }
}

extern unsigned char last_conio_key;
#define BLINK_DELAY 230

static unsigned char mycgetc() {
    unsigned char k = -1;

    unsigned int blinkCounter = BLINK_DELAY;
    unsigned int vdpaddr = conio_getvram();
    unsigned char screenChar = vdpreadchar(vdpaddr);

    if (last_conio_key != 255) {
        k = last_conio_key;
        last_conio_key = 255;
        return k;
    }

    do {
        k = kscan(5);
        blinkCounter--;
        if (blinkCounter == 0) {
          blinkCounter = BLINK_DELAY;
          if (vdpreadchar(vdpaddr) == cursor) {
            vdpchar(vdpaddr, screenChar);
          } else {
            vdpchar(vdpaddr, cursor);
          }
        }
    } while ((k == 255) || ((KSCAN_STATUS&KSCAN_MASK) == 0));
    // restore display incase we put a cursor on it.
    vdpchar(vdpaddr, screenChar);
    
    return k;
}
