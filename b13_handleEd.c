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
  int top_line;
  int left_column;
  int lineCount;
  struct Line lines[];
};

#define MAX_LINE 250

#define EDIT_BUFFER ((struct EditBuffer*)0xA000)

static void loadFile(struct DeviceServiceRoutine* dsr, char* path);
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

  loadFile(dsr, path);

  renderLines();

  conio_x = 0;
  conio_y = 0;

  edit_loop();

  if (sams_total_pages) {
    bk_free_pages(6);
  }
}

static void loadFile(struct DeviceServiceRoutine* dsr, char* path) {
  // zero out the allocated memory buffer space (all of upper memory expansion)
  bk_strset((char*)0xA000, 0, 24 * 1024);

  struct PAB pab;
  int err = bk_dsr_open(dsr, &pab, path, DSR_TYPE_DISPLAY | DSR_TYPE_VARIABLE | DSR_TYPE_SEQUENTIAL | DSR_TYPE_INPUT, 80);
  if (err) {
    EDIT_BUFFER->lineCount = 0;
    return;
  }

  while(!err) {
    err = bk_dsr_read(dsr, &pab, 0);
    if (!err) {
      struct Line* line = &(EDIT_BUFFER->lines[EDIT_BUFFER->lineCount]);
      line->length = pab.CharCount;
      vdpmemread(pab.VDPBuffer, line->data, line->length);
      EDIT_BUFFER->lineCount++;
    }
  }
  bk_dsr_close(dsr, &pab);
}

static void renderLines() {
  // NO ansi, so bypass cputc.

  int displayAddr = gImage;
  for(int l=EDIT_BUFFER->top_line; l < EDIT_BUFFER->top_line + displayHeight; l++) {
    if (l<EDIT_BUFFER->lineCount) {
      vdpmemcpy(displayAddr, EDIT_BUFFER->lines[l].data + EDIT_BUFFER->left_column, displayWidth);
    } else {
      vdpmemset(displayAddr, 0, displayWidth);
    }
    displayAddr += displayWidth;
  }
}

static void movex(int x) {
  int requiresRender = 0;
  conio_x += x;
  if (conio_x == -1) {
    EDIT_BUFFER->left_column -= 1;
    conio_x = 0;
    requiresRender = 1;
    if (EDIT_BUFFER->left_column == -1) {
      EDIT_BUFFER->left_column = 0;
      requiresRender = 0;
    }
  }

  if (conio_x == displayWidth) {
    EDIT_BUFFER->left_column += 1;
    conio_x = displayWidth - 1;
    requiresRender = 1;
    if (EDIT_BUFFER->left_column + displayWidth > EDIT_BUFFER->lines[conio_y+EDIT_BUFFER->top_line].length) {
      EDIT_BUFFER->left_column -= 1;
      requiresRender = 0;
    }
  }

  if (requiresRender) {
    renderLines();
  }
}

static void movey(int y) {
  int requiresRender = 0;
  conio_y += y;
  if (conio_y == -1) {
    // adjust current file line
    EDIT_BUFFER->top_line -= 1;
    conio_y = 0;
    requiresRender = 1;
    if (EDIT_BUFFER->top_line == -1) {
      EDIT_BUFFER->top_line = 0;
      requiresRender = 0;
    }
  }

  if (conio_y == displayHeight) {
    EDIT_BUFFER->top_line += 1;
    conio_y = displayHeight - 1;
    requiresRender = 1;
    if (EDIT_BUFFER->top_line + displayHeight > EDIT_BUFFER->lineCount) {
      EDIT_BUFFER->top_line -= 1;
      requiresRender = 0;
    }
  }

  if (requiresRender) {
    renderLines();
  }
}

// quit: CTRL-Q
#define KEY_QUIT 145
#define KEY_LEFT 8
#define KEY_RIGHT 9
#define KEY_DOWN 10
#define KEY_UP 11

static void edit_loop() {
  int quit = 0;

  while(!quit) {
    unsigned int k = bk_cgetc(CUR_OVERWRITE);
    switch(k) {
      case KEY_QUIT:
        quit = 1;
        break;
      case KEY_LEFT:
        movex(-1);
        break;
      case KEY_RIGHT:
        movex(1);
        break;
      case KEY_DOWN:
        movey(1);
        break;
      case KEY_UP:
        movey(-1);
        break;
      default:
        // handle actual typing...
        break;
    }
  }
}