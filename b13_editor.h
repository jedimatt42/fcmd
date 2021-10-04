#ifndef _EDITOR_H
#define _EDITOR_H 1

#include "b2_dsrutil.h"

#define KEY_QUIT 145
#define KEY_SAVE 151
#define KEY_CTRL_R 146
#define KEY_CTRL_C 131
#define KEY_LEFT 8
#define KEY_RIGHT 9
#define KEY_DOWN 10
#define KEY_UP 11
#define KEY_AID 1
#define KEY_BREAK 2
#define KEY_DELETE 3
#define KEY_INSERT 4
#define KEY_ERASE 7
#define KEY_BACK 15
#define KEY_SPACE 0x20
#define KEY_TILDE 0x7E
#define KEY_ENTER 13
#define KEY_BEGIN 147 
#define KEY_END 132

struct __attribute__((__packed__)) Line {
  int length;
  char data[80];
};

struct __attribute__((__packed__)) EditBuffer {
  int justRendered;
  int screen_x;
  int screen_y;
  int max_lines;
  int ed_mode;
  int offset_x;
  int offset_y;
  int lineCount;
  struct Line lines[];
};

#define ED_FULL 1
#define ED_LINE 2

#define MAX_LINES 250

#define EDIT_BUFFER ((struct EditBuffer*)0xA000)

#define ED_VDPCHAR(x) vdpmemset(gImage + (displayWidth*conio_y) + conio_x++, x, 1)

void ed_clearBuffer(); 
int ed_loadFile(struct DeviceServiceRoutine* dsr, char* path); 
void ed_renderLines(); 
void ed_left();
void ed_right(); 
void ed_overwrite(int k);
void ed_insert(int k);
void ed_erase(); 
void ed_deleteChar();
void ed_jumpEOLonYchange();
void ed_gotoEndOfLine();
void ed_gotoBeginningOfLine();
void ed_clearLine(struct Line* line);
void ed_down();
void ed_up();
void ed_historyDown();
void ed_historyUp();
void ed_historyPush();
void ed_enter();
void ed_eraseLine();
void ed_breakLine();
void ed_removeTrailingSpaces();
void ed_save(char* devpath);
void ed_showHelp();
int ed_edit_loop(char* devpath);
int ed_handleKeys(char* devpath, int* insert_mode, int* cursor, int k);

#endif
