#include "banks.h"
#define MYBANK BANK(13)

#include "b13_editor.h"
#include "b1_history.h"
#include "b0_globals.h"
#include "b8_getstr.h"

int ed_handleKeys(char* devpath, int* insert_mode, int* cursor, int k) {
  switch (k) {
  case KEY_QUIT:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      return 1;
    }
    break;
  case KEY_LEFT:
    if (insert_mode && backspace) {
      ed_erase();
    } else {
      ed_left();
    }
    break;
  case KEY_RIGHT:
    ed_right();
    break;
  case KEY_DOWN:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      ed_down();
    } else {
      ed_historyDown();
    }
    break;
  case KEY_REDO:
    if (EDIT_BUFFER->ed_mode == ED_LINE) {
      ed_historyUp();
    }
    break;
  case KEY_UP:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      ed_up();
    } else {
      ed_historyUp();
    }
    break;
  case KEY_END:
    ed_gotoEndOfLine();
    break;
  case KEY_BEGIN:
    ed_gotoBeginningOfLine();
    break;
  case KEY_ENTER:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      if (insert_mode) {
	ed_breakLine();
      } else {
	ed_enter();
      }
    } else {
      ed_historyPush();
      return 1;
    }
    break;
  case KEY_CTRL_C:
  case KEY_BREAK:
    if (EDIT_BUFFER->ed_mode == ED_LINE) {
      return -1;
    }
  case KEY_ERASE:
    ed_eraseLine();
    break;
  case KEY_DELETE:
    ed_deleteChar();
    break;
  case KEY_SAVE:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      ed_save(devpath);
    }
    break;
  case KEY_AID:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      ed_showHelp();
    }
    break;
  case KEY_INSERT:
    *insert_mode = !*insert_mode;
    *cursor = *insert_mode ? CUR_INSERT : CUR_OVERWRITE;
    break;
  case KEY_CTRL_J:
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
      ed_joinToNext();
    }
    break;
  default:
    // ignore
    break;
  }
  return 0;
}

