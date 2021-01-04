#include "banks.h"
#define MYBANK BANK(13)

#include "b0_globals.h"
#include "b0_parsing.h"
#include "b0_sams.h"
#include "b2_dsrutil.h"
#include "b1_strutil.h"
#include "b8_terminal.h"
#include "b8_getstr.h"
#include "vdp.h"
#include "conio.h"

struct __attribute__((__packed__)) Line {
  int length;
  unsigned char data[80];
};

struct __attribute__((__packed__)) EditBuffer {
  int offset_x;
  int offset_y;
  int lineCount;
  struct Line lines[];
};

#define MAX_LINE 250

#define EDIT_BUFFER ((struct EditBuffer*)0xA000)

static int loadFile(struct DeviceServiceRoutine* dsr, char* path);
static void renderLines();
static void edit_loop();

void handleEd() {
  // A tiny DV80 editor

  struct DeviceServiceRoutine* dsr = 0;

  char path[256];
  bk_parsePathParam(0, &dsr, path, PR_REQUIRED);
  if (dsr == 0) {
    tputs_rom("no file specified\n");
    return;
  }

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
  }

  if (!err) {
    int backup_nTitleline = nTitleLine;
    nTitleLine = 0;
    renderLines();

    conio_x = 0;
    conio_y = 0;

    edit_loop();
    nTitleLine = backup_nTitleline;
  }

  if (sams_total_pages) {
    bk_free_pages(6);
  }
}

static int loadFile(struct DeviceServiceRoutine* dsr, char* path) {
  // zero out the allocated memory buffer space (all of upper memory expansion)
  bk_strset((char*)0xA000, 0, 24 * 1024);

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
}

static void left() {
  if (conio_x) {
    conio_x--;
  } else {
    if (EDIT_BUFFER->offset_x) {
      EDIT_BUFFER->offset_x--;
      renderLines();
    }
  }
}

static void right() {
  int lineLimit = EDIT_BUFFER->lines[conio_y + EDIT_BUFFER->offset_y].length;
  if (conio_x < (displayWidth-1) && conio_x < lineLimit) {
    conio_x++;
  } else {
    int line_x = EDIT_BUFFER->offset_x + conio_x;
    if (line_x < 79 && line_x < lineLimit) {
      EDIT_BUFFER->offset_x++;
      renderLines();
    }
  }
}

static void rightGrow(int k) {
  // place the character on screen
  int o_x = conio_x;
  int o_y = conio_y;
  bk_tputc(k);
  conio_x = o_x;
  conio_y = o_y;
  // store in the record
  int line_idx = conio_x + EDIT_BUFFER->offset_x;
  struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->offset_y + conio_y]);
  line->data[line_idx] = k;
  if (line_idx == line->length && line->length < 80) {
    line->length++;
  }
  right();
}

static void jumpEOLonYchange() {
  int lineLimit = EDIT_BUFFER->lines[conio_y + EDIT_BUFFER->offset_y].length;
  while(EDIT_BUFFER->offset_x + conio_x > lineLimit) {
    left();
  }
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

// quit: CTRL-Q
#define KEY_QUIT 145
#define KEY_SAVE 147
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

static void edit_loop() {
  int quit = 0;

  while(!quit) {
    unsigned int k = bk_cgetc(CUR_OVERWRITE);

    if (k >= KEY_SPACE && k <= KEY_TILDE) {
      rightGrow(k);
    } else {
      switch (k) {
      case KEY_QUIT:
        quit = 1;
        break;
      case KEY_LEFT:
        left();
        break;
      case KEY_RIGHT:
        right();
        break;
      case KEY_DOWN:
        down();
        break;
      case KEY_UP:
        up();
        break;
      default:
        // ignore
        break;
      }
    }
  }
}