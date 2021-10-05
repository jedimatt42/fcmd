#include "banks.h"
#define MYBANK BANK(13)

#include "b0_globals.h"
#include "b10_parsing.h"
#include "b0_sams.h"
#include "b2_dsrutil.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b8_getstr.h"
#include "b13_honk.h"
#include "vdp.h"
#include "conio.h"
#include "b1_history.h"
#include "b13_editor.h"
#include "b13_dropdown.h"


void ed_clearBuffer() {
  // zero out the allocated memory buffer space (all of upper memory expansion)
  bk_strset((char*)(EDIT_BUFFER->lines), 0, 80 * EDIT_BUFFER->max_lines);
}

int ed_loadFile(struct DeviceServiceRoutine* dsr, char* path) {
  EDIT_BUFFER->lineCount = 0;
  ed_clearBuffer();

  struct PAB pab;
  int err = bk_dsr_open(dsr, &pab, path, DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL | DSR_TYPE_INPUT, 80);
  if (err) {
    EDIT_BUFFER->lineCount = 0;
    return 1;
  }

  while(!err) {
    err = bk_dsr_read(dsr, &pab, 0);
    if (!err) {
      struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->lineCount]);
      line->length = pab.CharCount;
      vdpmemread(pab.VDPBuffer, line->data, line->length);
      EDIT_BUFFER->lineCount++;
      if (EDIT_BUFFER->lineCount > 250) {
        tputs_rom("Error, file exceeds 250 lines\n");
        err = -2;
      }
    }
  }
  bk_dsr_close(dsr, &pab);
  return err == -2;
}

void ed_renderLines() {
  if (!EDIT_BUFFER->justRendered) {
    EDIT_BUFFER->justRendered = 1;
    if (EDIT_BUFFER->ed_mode == ED_FULL) {
    int y = EDIT_BUFFER->offset_y;
      for (int i = 0; i < displayHeight; i++) {
	// show all the lines available.
	if (y < EDIT_BUFFER->lineCount) {
	  vdpmemcpy(gImage + (displayWidth * i), EDIT_BUFFER->lines[y].data + EDIT_BUFFER->offset_x, displayWidth);
	} else {
	  vdpmemset(gImage + (displayWidth * i), 0, displayWidth);
	}
	y++;
      }
    } else {
      vdpmemcpy(gImage + (displayWidth * EDIT_BUFFER->screen_y) + EDIT_BUFFER->screen_x, EDIT_BUFFER->lines[0].data + EDIT_BUFFER->offset_x, displayWidth - EDIT_BUFFER->screen_x); 
    }
  }
}

void ed_left() {
  if (conio_x - EDIT_BUFFER->screen_x) {
    conio_x--;
  } else {
    if (EDIT_BUFFER->offset_x) {
      EDIT_BUFFER->offset_x--;
      ed_renderLines();
    }
  }
}

void ed_right() {
  int lineLimit = EDIT_BUFFER->lines[(conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y].length;
  if (conio_x < (displayWidth-1) && (conio_x - EDIT_BUFFER->screen_x) < lineLimit) {
    conio_x++;
  } else {
    int line_x = EDIT_BUFFER->offset_x + (conio_x - EDIT_BUFFER->screen_x);
    if (line_x < 79 && line_x < lineLimit) {
      EDIT_BUFFER->offset_x++;
      ed_renderLines();
    }
  }
}

// BUG::: things go wrong when joining the last line in the editor
void ed_joinLines(int lineone, int linetwo) {
  if (linetwo == 0 || EDIT_BUFFER->lineCount < linetwo) {
    honk();
    return;
  }
  if (EDIT_BUFFER->lines[linetwo].length + EDIT_BUFFER->lines[lineone].length < 80) {
    beep();
    // capture previous end of line.
    int prevlen = EDIT_BUFFER->lines[lineone].length;
    // copy lineno to end of lineno-1
    bk_strncpy(EDIT_BUFFER->lines[lineone].data + prevlen, EDIT_BUFFER->lines[linetwo].data, 80 - prevlen);
    EDIT_BUFFER->lines[lineone].length += EDIT_BUFFER->lines[linetwo].length;
    // delete lineno
    conio_y = linetwo;
    ed_eraseLine(); // may have moved up if was previous last line
    // move cursor to previous end of line
    EDIT_BUFFER->offset_x = prevlen > displayWidth ? displayWidth : 0;
    conio_x = prevlen - EDIT_BUFFER->offset_x;
    conio_y = EDIT_BUFFER->screen_y + lineone;
    // renderlines
    EDIT_BUFFER->justRendered = 0;
    ed_renderLines();
  } else {
    honk();
  }
}

void ed_joinToNext() {
  int lineone = (conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y;
  ed_joinLines(lineone, lineone + 1);
}

void ed_joinToPrevious() {
  int linetwo = (conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y;
  ed_joinLines(linetwo - 1, linetwo);
}

void ed_overwrite(int k) {
  // place the character on screen
  bk_raw_cputc(k);

  // store in the record
  int line_idx = conio_x + EDIT_BUFFER->offset_x - EDIT_BUFFER->screen_x;
  struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->offset_y + (conio_y - EDIT_BUFFER->screen_y)]);
  line->data[line_idx] = k;
  if ((conio_x - EDIT_BUFFER->screen_x) + EDIT_BUFFER->offset_x == 79) {
    honk();
  } else {
    if (line->length < 80) {
      if (line_idx == line->length) {
        line->length++;
      }
    }
  }
  ed_right();
  ed_renderLines();
}

void ed_insert(int k) {
  // insert into current line
  struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->offset_y + (conio_y - EDIT_BUFFER->screen_y)]);
  if (line->length < 80) {
    // place the character on screen
    bk_raw_cputc(k);

    int imin = conio_x + EDIT_BUFFER->offset_x - EDIT_BUFFER->screen_x;
    for(int i = line->length-1; i >= imin; i--) {
      line->data[i+1] = line->data[i];
    }
    line->length++;
    line->data[imin] = k;
    ed_right();
    ed_renderLines();
  } else {
    honk();
  }
}

void ed_erase() {
  struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->offset_y + (conio_y - EDIT_BUFFER->screen_y)]);
  // shift all the characters in the line one left
  int x = (conio_x - EDIT_BUFFER->screen_x) + EDIT_BUFFER->offset_x;
  if (x > 0) {
    x--;
    while (x < line->length) {
      line->data[x] = line->data[x + 1];
      x++;
    }
    line->length = x - 1;
    while (x < 80) {
      line->data[x++] = 0;
    }
    // move current location one left
    ed_left();
    // force screen update
    ed_renderLines();
  } else {
    ed_joinToPrevious();
  }
}

void ed_deleteChar() {
  struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->offset_y + (conio_y - EDIT_BUFFER->screen_y)]);
  int x = (conio_x - EDIT_BUFFER->screen_x) + EDIT_BUFFER->offset_x;
  if (x == line->length) {
    honk();
    return;
  }

  for(int i=x; i<line->length; i++) {
    line->data[i] = line->data[i+1];
  }
  line->length--;
  ed_renderLines();
}

void ed_jumpEOLonYchange() {
  int lineLimit = EDIT_BUFFER->lines[(conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y].length;
  while(EDIT_BUFFER->offset_x + (conio_x - EDIT_BUFFER->screen_x) > lineLimit) {
    EDIT_BUFFER->justRendered = 1;
    ed_left();
  }
  EDIT_BUFFER->justRendered = 0;
  ed_renderLines();
}

void ed_gotoEndOfLine() {
  int lineLimit = EDIT_BUFFER->lines[(conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y].length;
  conio_x = displayWidth - 1;
  if (displayWidth == 40 && lineLimit > 40) {
    EDIT_BUFFER->offset_x = (lineLimit - 40) + EDIT_BUFFER->screen_x;
  }
  ed_jumpEOLonYchange();
}

void ed_gotoBeginningOfLine() {
  conio_x = EDIT_BUFFER->screen_x;
  EDIT_BUFFER->offset_x = 0;
  EDIT_BUFFER->justRendered = 0;
  ed_renderLines();
}

void ed_clearLine(struct Line* line) {
  line->length = 0;
  for(int i=0; i<80; i++) {
    line->data[i] = 0;
  }
}

void ed_down() {
  if (conio_y < (displayHeight-1) && conio_y < (EDIT_BUFFER->lineCount-1)) {
    conio_y++;
  } else {
    if (conio_y + EDIT_BUFFER->offset_y < (EDIT_BUFFER->lineCount-1)) {
      EDIT_BUFFER->offset_y++;
      ed_renderLines();
    }
  }
  ed_jumpEOLonYchange();
}

void ed_up() {
  if (conio_y) {
    conio_y--;
  } else {
    if (EDIT_BUFFER->offset_y) {
      EDIT_BUFFER->offset_y--;
      ed_renderLines();
    }
  }
  ed_jumpEOLonYchange();
}

void ed_historyDown() {
  if (history_on) {
    struct Line* line = EDIT_BUFFER->lines; // always line 0 when history at play
    ed_clearLine(line);
    bk_history_redo(line->data, 80, HIST_GET_DEC);
    line->length = bk_strlen(line->data);
    ed_gotoBeginningOfLine();
  }
}

void ed_historyUp() {
  if (history_on) {
    struct Line* line = EDIT_BUFFER->lines; // always line 0 when history at play
    ed_clearLine(line);
    bk_history_redo(line->data, 80, HIST_GET);
    line->length = bk_strlen(line->data);
    ed_gotoBeginningOfLine();
  }
}

void ed_historyPush() {
  if (history_on) {
    struct Line* line = EDIT_BUFFER->lines;
    bk_history_redo(line->data, line->length, HIST_STORE);
  }
}

void ed_enter() {
  if (conio_y + EDIT_BUFFER->offset_y == EDIT_BUFFER->lineCount - 1) {
    EDIT_BUFFER->lineCount++;
    ed_clearLine(&(EDIT_BUFFER->lines[EDIT_BUFFER->lineCount-1]));
  }
  ed_down();
}

void ed_eraseLine() {
  // remove current line. - requires compacting buffer
  // if no more lines, erase to end of line. keep 1 empty line.
  // else if last line in buffer, move up.

  if (EDIT_BUFFER->lineCount == 1) {
    // delete contents of line, but leave single line.
    struct Line* line = &(EDIT_BUFFER->lines[0]);
    ed_clearLine(line);
  } else {
    // this gets messy if later we have live buffer in pages
    int szline = sizeof(struct Line);
    int startLine = conio_y + EDIT_BUFFER->offset_y;
    char* dst = (char*) &(EDIT_BUFFER->lines[startLine]);
    char* src = dst + szline;
    char* end = (char*) &(EDIT_BUFFER->lines[EDIT_BUFFER->lineCount]);
    while(src < end) {
      *dst++ = *src++;
    }
    EDIT_BUFFER->lineCount--;
    if ((conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y == EDIT_BUFFER->lineCount) {
      ed_up();
    }
  }
  ed_jumpEOLonYchange();
  ed_renderLines();
}

void ed_breakLine() {
  // text to the right of the cursor is moved to a new line inserted after the current line.

  // if there is no more room, honk...
  if (EDIT_BUFFER->lineCount == EDIT_BUFFER->max_lines) {
    honk();
    return;
  }
  // start by inserting a line...
  int szline = sizeof(struct Line);
  struct Line* line = &(EDIT_BUFFER->lines[conio_y + EDIT_BUFFER->offset_y]);
  char* dst = ((char*) &(EDIT_BUFFER->lines[EDIT_BUFFER->lineCount])) + szline;
  char* src = dst - szline;
  char* end = ((char*) line) + szline;
  while(src > end) {
    *--dst = *--src;
  }
  EDIT_BUFFER->lineCount++;
  struct Line* newline = line + 1;
  ed_clearLine(newline);

  // now copy from cursor to end of current line to beginning of new line.
  for(int x = conio_x + EDIT_BUFFER->offset_x; x < line->length; x++) {
    newline->data[newline->length++] = line->data[x];
  }

  // then truncate the old line.
  for(int x = conio_x + EDIT_BUFFER->offset_x; x < line->length; x++) {
    line->data[x] = 0;
  }
  line->length = conio_x + EDIT_BUFFER->offset_x;

  // move down and to the left most column.
  conio_x = 0;
  EDIT_BUFFER->offset_x = 0;
  ed_down();
  ed_renderLines();
}

void ed_removeTrailingSpaces() {
  for(int i=0; i<EDIT_BUFFER->lineCount; i++) {
    // all records can be upto 80 characters... they are padded with zeros to the right.
    struct Line* line = &(EDIT_BUFFER->lines[i]);
    int c = 79;
    while((line->data[c] == ' ' || line->data[c] == 0) && c >= 0) {
      line->data[c] = 0;
      c--;
    }
    line->length = c + 1;
  }
}

void ed_save(char* devpath) {
  int o_x = conio_x;
  int o_y = conio_y;

  dropDown(4);
  conio_x = 2;
  conio_y = 1;
  tputs_rom("Write File: ");
  conio_x = 2;
  conio_y = 2;
  char filename[80];
  bk_strncpy(filename, devpath, displayWidth-4);
  bk_getstr(filename, displayWidth-4, backspace);

  if (filename[0]) {
    struct DeviceServiceRoutine* dsr;
    char path[80];
    bk_parsePathParam(filename, &dsr, path, PR_REQUIRED);
    if (dsr) {
      struct PAB pab;
      int err = bk_dsr_open(dsr, &pab, path, DSR_TYPE_OUTPUT | DSR_TYPE_VARIABLE | DSR_TYPE_DISPLAY | DSR_TYPE_SEQUENTIAL, 80);

      // save will remove trailing spaces.
      ed_removeTrailingSpaces();

      int lno = 0;
      while (!err && lno < EDIT_BUFFER->lineCount) {
        struct Line* line = &(EDIT_BUFFER->lines[lno]);
        err = bk_dsr_write(dsr, &pab, line->data, line->length);
        lno++;
      }
      bk_dsr_close(dsr, &pab);
    }
  }

  conio_x = o_x;
  conio_y = o_y;
  ed_renderLines();
}

void ed_showHelp() {
  int o_x = conio_x;
  int o_y = conio_y;

  dropDown(11);

  conio_x = 2;
  conio_y = 1;
  tputs_rom("^-W : Write File");
  conio_x = 2;
  conio_y++;
  tputs_rom("^-Q : Quit");
  conio_x = 2;
  conio_y++;
  tputs_rom("F-1 : Delete Char");
  conio_x = 2;
  conio_y++;
  tputs_rom("F-2 : Toggle Insert");
  conio_x = 2;
  conio_y++;
  tputs_rom("F-3 : Delete Line");
  conio_x = 2;
  conio_y++;
  tputs_rom("^-S : Jump to Beginning of Line");
  conio_x = 2;
  conio_y++;
  tputs_rom("^-D : Jump to End of Line");
  conio_x = 2;
  conio_y++;
  tputs_rom("F-7 : Show Help");
  conio_x = 2;
  conio_y++;
  tputs_rom("Any Key : Exit Help");
  bk_cgetc(0);

  conio_x = o_x;
  conio_y = o_y;
  ed_renderLines();
}

int ed_edit_loop(char* devpath) {
  int insert_mode = 1;
  int cursor = CUR_INSERT;

  while(1) {
    EDIT_BUFFER->justRendered = 0;
    unsigned int k = bk_cgetc(cursor);

    if (k >= KEY_SPACE && k <= KEY_TILDE) {
      if (insert_mode) {
        ed_insert(k);
      } else {
        ed_overwrite(k);
      }
    } else {
      int quit = ed_handleKeys(devpath, &insert_mode, &cursor, k);
      if (quit != 0) {
	return quit;
      }
    }
  }
}

