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

#define KEY_QUIT 145
#define KEY_SAVE 151
#define KEY_CTRL_R 146
#define KEY_LEFT 8
#define KEY_RIGHT 9
#define KEY_DOWN 10
#define KEY_UP 11
#define KEY_AID 1
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

static int loadFile(struct DeviceServiceRoutine* dsr, char* path);
static void renderLines();
static void edit_loop(char* devpath);
static void clearBuffer();

int commandLineEd(char* cmdline, int limit) {
  // command line editor using same code as full screen editor - mostly
  if (sams_total_pages) {
    int pagebase = bk_alloc_pages(1);
    bk_map_page(pagebase, 0xA000);
  }

  EDIT_BUFFER->justRendered = 0;
  EDIT_BUFFER->ed_mode = ED_LINE;
  EDIT_BUFFER->max_lines = 1;
  EDIT_BUFFER->lineCount = 1;
  EDIT_BUFFER->screen_x = conio_x;
  EDIT_BUFFER->screen_y = conio_y;
  EDIT_BUFFER->offset_x = 0;
  EDIT_BUFFER->offset_y = 0;

  clearBuffer();

  renderLines();

  conio_x = EDIT_BUFFER->screen_x;
  conio_y = EDIT_BUFFER->screen_y;

  edit_loop(0); // null devpath as save is disabled.

  bk_strncpy(cmdline, EDIT_BUFFER->lines[0].data, limit);

  if (sams_total_pages) {
    bk_free_pages(1);
  }

  return 0; // TODO return indicator of ctrl-c
}

void handleEd() {
  // A tiny DV80 editor
  EDIT_BUFFER->justRendered = 0;
  EDIT_BUFFER->ed_mode = ED_FULL;
  EDIT_BUFFER->max_lines = MAX_LINES;
  EDIT_BUFFER->lineCount = 1;
  EDIT_BUFFER->screen_x = 0;
  EDIT_BUFFER->screen_y = 0;
  EDIT_BUFFER->offset_x = 0;
  EDIT_BUFFER->offset_y = 0;

  struct DeviceServiceRoutine* dsr = 0;

  char devpath[80];
  char* path = devpath + 5;
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
    return;
  }

  char* dsr_cruhex = bk_uint2hex(dsr->crubase);
  bk_strncpy(devpath, dsr_cruhex, 4);
  devpath[4] = '.';

  if (sams_total_pages) {
    int pagebase = bk_alloc_pages(6);
    for(int i=pagebase; i<pagebase+6; i++) {
      bk_map_page(i, 0xA000 + ((i-pagebase) * 0x1000));
    }
  }
  int err = 0;

  int existing = bk_existsFile(dsr, path);

  if (existing) {
    err = loadFile(dsr, path);
  } else {
    clearBuffer();
    EDIT_BUFFER->lineCount = 1;
  }

  if (!err) {
    int backup_nTitleline = nTitleLine;
    nTitleLine = 0;
    char cls[] = { 27, '[', '2', 'J', 0 };
    tputs_rom(cls);
    renderLines(0, 0);

    conio_x = 0;
    conio_y = 0;

    edit_loop(devpath);
    nTitleLine = backup_nTitleline;

    conio_x = 0;
    conio_y = displayHeight - 1;
  }

  if (sams_total_pages) {
    bk_free_pages(6);
  }
}

static void clearBuffer() {
  // zero out the allocated memory buffer space (all of upper memory expansion)
  bk_strset((char*)(EDIT_BUFFER->lines), 0, 80 * EDIT_BUFFER->max_lines);
}

static int loadFile(struct DeviceServiceRoutine* dsr, char* path) {
  EDIT_BUFFER->lineCount = 0;
  clearBuffer();

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

static void renderLines() {
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

static void left() {
  if (conio_x - EDIT_BUFFER->screen_x) {
    conio_x--;
  } else {
    if (EDIT_BUFFER->offset_x) {
      EDIT_BUFFER->offset_x--;
      renderLines();
    }
  }
}

static void right() {
  int lineLimit = EDIT_BUFFER->lines[(conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y].length;
  if (conio_x < (displayWidth-1) && (conio_x - EDIT_BUFFER->screen_x) < lineLimit) {
    conio_x++;
  } else {
    int line_x = EDIT_BUFFER->offset_x + (conio_x - EDIT_BUFFER->screen_x);
    if (line_x < 79 && line_x < lineLimit) {
      EDIT_BUFFER->offset_x++;
      renderLines();
    }
  }
}

static void overwrite(int k) {
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
  right();
}

static void insert(int k) {
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
    right();
    renderLines();
  } else {
    honk();
  }
}

static void erase() {
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
    left();
    // force screen update
    renderLines();
  } else {
    honk();
  }
}

static void deleteChar() {
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
  renderLines();
}

static void jumpEOLonYchange() {
  int lineLimit = EDIT_BUFFER->lines[(conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y].length;
  while(EDIT_BUFFER->offset_x + (conio_x - EDIT_BUFFER->screen_x) > lineLimit) {
    EDIT_BUFFER->justRendered = 1;
    left();
  }
  EDIT_BUFFER->justRendered = 0;
  renderLines();
}

static void gotoEndOfLine() {
  int lineLimit = EDIT_BUFFER->lines[(conio_y - EDIT_BUFFER->screen_y) + EDIT_BUFFER->offset_y].length;
  conio_x = displayWidth - 1;
  if (displayWidth == 40 && lineLimit > 40) {
    EDIT_BUFFER->offset_x = (lineLimit - 40) + EDIT_BUFFER->screen_x;
  }
  jumpEOLonYchange();
}

static void gotoBeginningOfLine() {
  conio_x = EDIT_BUFFER->screen_x;
  EDIT_BUFFER->offset_x = 0;
  renderLines();
}

static void down() {
  if (conio_y < (displayHeight-1) && conio_y < (EDIT_BUFFER->lineCount-1)) {
    conio_y++;
  } else {
    if (conio_y + EDIT_BUFFER->offset_y < (EDIT_BUFFER->lineCount-1)) {
      EDIT_BUFFER->offset_y++;
      renderLines();
    }
  }
  jumpEOLonYchange();
}

static void up() {
  if (conio_y) {
    conio_y--;
  } else {
    if (EDIT_BUFFER->offset_y) {
      EDIT_BUFFER->offset_y--;
      renderLines();
    }
  }
  jumpEOLonYchange();
}

static void clearLine(struct Line* line) {
  line->length = 0;
  for(int i=0; i<80; i++) {
    line->data[i] = 0;
  }
}

static void enter() {
  if (conio_y + EDIT_BUFFER->offset_y == EDIT_BUFFER->lineCount - 1) {
    EDIT_BUFFER->lineCount++;
    clearLine(&(EDIT_BUFFER->lines[EDIT_BUFFER->lineCount-1]));
  }
  down();
}

static void eraseLine() {
  // remove current line. - requires compacting buffer
  // if no more lines, erase to end of line. keep 1 empty line.
  // else if last line in buffer, move up.

  if (EDIT_BUFFER->lineCount == 1) {
    // delete contents of line, but leave single line.
    struct Line* line = &(EDIT_BUFFER->lines[0]);
    clearLine(line);
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
      up();
    }
  }
  jumpEOLonYchange();
  renderLines();
}

static void breakLine() {
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
  clearLine(newline);

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
  down();
  renderLines();
}

#define BAD_VDPCHAR(x) vdpmemset(gImage + (displayWidth*conio_y) + conio_x++, x, 1)

static void dropDownBar(int y) {
  conio_x = 0;
  conio_y = y;
  BAD_VDPCHAR(0xd4);
  for (int x = 1; x < displayWidth - 1; x++) {
    BAD_VDPCHAR(0xcd);
  }
  BAD_VDPCHAR(0xbe);
}

static void dropDownSpace(int y) {
  conio_x = 0;
  conio_y = y;
  BAD_VDPCHAR(0xb3);
  for (int x = 1; x < displayWidth - 1; x++) {
    BAD_VDPCHAR(' ');
  }
  BAD_VDPCHAR(0xb3);
}

void dropDown(int linecount) {
  for(int i=0; i<linecount; i++) {
    dropDownSpace(i);
  }
  dropDownBar(linecount);
}

static void removeTrailingSpaces() {
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

static void save(char* devpath) {
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
      removeTrailingSpaces();

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
  renderLines();
}

static void showHelp() {
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
  renderLines();
}

static void edit_loop(char* devpath) {
  int quit = 0;
  int insert_mode = 1;
  int cursor = CUR_INSERT;

  while(!quit) {
    EDIT_BUFFER->justRendered = 0;
    unsigned int k = bk_cgetc(cursor);

    if (k >= KEY_SPACE && k <= KEY_TILDE) {
      if (insert_mode) {
        insert(k);
      } else {
        overwrite(k);
      }
    } else {
      switch (k) {
      case KEY_QUIT:
	if (EDIT_BUFFER->ed_mode == ED_FULL) {
	  quit = 1;
	}
        break;
      case KEY_LEFT:
	if (insert_mode && backspace) {
	  erase();
	} else {
	  left();
	}
        break;
      case KEY_RIGHT:
        right();
        break;
      case KEY_DOWN:
	if (EDIT_BUFFER->ed_mode == ED_FULL) {
          down();
	}
        break;
      case KEY_UP:
	if (EDIT_BUFFER->ed_mode == ED_FULL) {
          up();
	}
        break;
      case KEY_END:
	gotoEndOfLine();
	break;
      case KEY_BEGIN:
	gotoBeginningOfLine();
	break;
      case KEY_ENTER:
	if (EDIT_BUFFER->ed_mode == ED_FULL) {
	  if (insert_mode) {
	    breakLine();
	  } else {
	    enter();
	  }
	} else {
	  quit = 1;
	}
        break;
      case KEY_ERASE:
        eraseLine();
        break;
      case KEY_DELETE:
        deleteChar();
        break;
      case KEY_SAVE:
	if (EDIT_BUFFER->ed_mode == ED_FULL) {
          save(devpath);
	}
        break;
      case KEY_AID:
	if (EDIT_BUFFER->ed_mode == ED_FULL) {
	  showHelp();
	}
        break;
      case KEY_INSERT:
        insert_mode = !insert_mode;
        cursor = insert_mode ? CUR_INSERT : CUR_OVERWRITE;
        break;
      default:
        // ignore
        break;
      }
    }
  }
}
