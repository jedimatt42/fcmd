#include "banks.h"
#define MYBANK BANK(8)

#include "b8_terminal.h"
#include "b8_getstr.h"
#include "b0_globals.h"
#include "b10_detect_vdp.h"
#include "b13_honk.h"
#include <conio.h>
#include <string.h>

extern unsigned char colors[];
extern unsigned char isBold;

void resetState();
int getParamA(int def);
int getParamB(int def);
void cursorUp(int lines);
void cursorDown(int lines);
void cursorRight(int cols);
void cursorLeft(int cols);
void cursorGoto(int x, int y);
void eraseDisplay(int opt);
void eraseLine(int opt);
void scrollUp(int lc);
void setColors();
void doSGRCommand();
void doCsiCommand(unsigned char c);
int doEscCommand(unsigned char c);
void charout(unsigned char ch);

// Cheating, and reaching into conio_cputc.c
void inc_row();

#define COLOR_DEFAULT 7

int stage;
char bytestr[128];
int bs_idx;

unsigned char cursor_store_x;
unsigned char cursor_store_y;

identify_callback identify_cb = 0;

void set_identify_hook(identify_callback cb) {
  identify_cb = cb;
}

identify_callback get_identify_hook() {
  return identify_cb;
}

#define STAGE_OPEN 0
#define STAGE_ESC 1
#define STAGE_CSI 2

inline void tgotoxy(int x, int y) {
  gotoxy(x, y+nTitleLine);
}

inline int twherey() {
  return wherey() - nTitleLine;
}

void resetState() {
  stage = STAGE_OPEN;
  bs_idx = 0;
}

void initTerminal() {
  cursor_store_x = 0;
  cursor_store_y = 0;

  resetState();
}

int getParamA(int def) {
  int i = 0;
  while(i<bs_idx && bytestr[i] != ';') {
    i++;
  }
  if (i == 0) {
    return def;
  }
  return bk_atoi(bytestr);
}

int getParamB(int def) {
  int i = 0;
  while(i<bs_idx && bytestr[i] != ';') {
    i++;
  }
  // we should be pointing to beginning of param B in buffer.
  if (i == 0 || i == bs_idx) {
    return def;
  }
  char* paramb = bytestr + i + 1;
  return bk_atoi(paramb);
}

void cursorUp(int lines) {
  int y = twherey() - lines;
  if (y < 0) {
    y = 0;
  }
  gotoy(y);
}

void cursorDown(int lines) {
  int y = twherey() + lines;
  unsigned char scx;
  unsigned char scy;
  screensize(&scx, &scy);
  scy--;
  if (y > scy) {
    y = scy;
  }
  gotoy(y);
}

void cursorRight(int cols) {
  int x = wherex() + cols;
  unsigned char scx;
  unsigned char scy;
  screensize(&scx, &scy);
  scx--;
  if (x > scx) {
    x = scx;
  }
  gotox(x);
}

void cursorLeft(int cols) {
  int x = wherex() + cols;
  if (x < 0) {
    x = 0;
  }
  gotox(x);
}

void cursorGoto(int x, int y) {
  unsigned char scx;
  unsigned char scy;
  screensize(&scx, &scy);
  if (x > scx) {
    x = scx;
  } else if (x < 1) {
    x = 1;
  }
  if (y > scy) {
    y = scy;
  } else if (y < 1) {
    y = 1;
  }
  // zero based screen
  gotox(x - 1);
  gotoy(y - 1);
}

void eraseDisplay(int opt) {
  unsigned char scx;
  unsigned char scy;
  screensize(&scx, &scy);
  int oldx = wherex();
  int oldy = twherey();
  int cursorAddr = gImage + (oldx + (oldy + scy));

  switch (opt) {
    case 0: // clear from cursor to end of screen, remain at location
      cclear(scx - oldx + scx*(scy - oldy - 1) - 1); // clear except last character - avoid scrolling
      tgotoxy(oldx, oldy);
      break;
    case 1: // clear from cursor to beginning of screen, remain at location
      cclearxy(0,0, scx*oldy + oldx); // should end up at oldx,oldy
      break;
    case 3: // TODO: if we add scroll back buffer, 3 should clear that too.
    case 2: // clear full screen, return to top
      clrscr();
      tgotoxy(0,0);
      break;
  }
}

void eraseLine(int opt) {
  unsigned char scx;
  unsigned char scy;
  screensize(&scx, &scy);
  int oldx = wherex();
  int oldy = twherey();
  switch (opt) {
    case 0: // to end of line
      cclear(scx - oldx);
      break;
    case 1: // to beginning of line
      gotox(0);
      cclear(oldx);
      break;
    case 2: // entire line
      gotox(0);
      cclear(scx);
      break;
  }
  tgotoxy(oldx, oldy);
}

void scrollUp(int lc) {
  tgotoxy(0,23);
  for (int i = 0; i < lc; i++) {
    inc_row();
  }
}

unsigned char isBold = 0;

unsigned char colors[16] = {
  COLOR_BLACK,
  COLOR_MEDRED,
  COLOR_MEDGREEN,
  COLOR_DKYELLOW,
  COLOR_DKBLUE,
  COLOR_MAGENTA,
  COLOR_CYAN,
  COLOR_GRAY,
  // now the BOLD variety
  COLOR_GRAY,
  COLOR_LTRED,
  COLOR_LTGREEN,
  COLOR_LTYELLOW,
  COLOR_LTBLUE,
  COLOR_DKRED, // bold magenta?
  COLOR_CYAN, // ???
  COLOR_WHITE
};

void setColors(int fore, int back) {
  bgcolor(back);
  textcolor(fore);
}

void doSGRCommand() {
  // only support color in 80 column mode.
  if (displayWidth != 80 || vdp_type != VDP_F18A) {
    return;
  }

  // each param ( there can be n ) is processed to set attributes
  // on all subsequent text. We'll only support foreground and
  // background color.
  // bs_idx
  // bytestr
  if (bs_idx == 0) {
    // set defaults and return
    isBold = 0;
    setColors(colors[COLOR_DEFAULT], COLOR_BLACK);
    return;
  }
  int i = 0;
  char* params = bytestr;
  while(i < bs_idx) {
    int sgr = bk_atoi(params + i);
    int foreground = FOREGROUND;
    int background = BACKGROUND;
    switch(sgr) {
      case 0: // clear attrs
        isBold = 0;
        foreground = colors[COLOR_DEFAULT];
        background = colors[0];
        break;
      case 1: // bold
        isBold = 8;
        break;
      case 30: // standard foregrounds
      case 31:
      case 32:
      case 33:
      case 34:
      case 35:
      case 36:
      case 37:
        foreground = colors[(sgr - 30) | isBold];
        break;
      case 40: // standard backgrounds
      case 41:
      case 42:
      case 43:
      case 44:
      case 45:
      case 46:
      case 47:
        background = colors[sgr - 40];
        break;
      case 90: // high intensity fore
      case 91:
      case 92:
      case 93:
      case 94:
      case 95:
      case 96:
      case 97:
        foreground = colors[(sgr - 90) | 8];
        break;
      case 100: // high intensity background
      case 101:
      case 102:
      case 103:
      case 104:
      case 105:
      case 106:
      case 107:
        background = colors[(sgr - 100) | 8];
        break;
    }
    setColors(foreground, background);
    while(i < bs_idx && params[i] >= '0' && params[i] <= '9') {
      i++;
    }
    if (i < bs_idx && params[i] == ';') {
      i++;
    }
  }
  bytestr[0] = 0;
  bs_idx = 0;
}

void doCsiCommand(unsigned char c) {
  // Note, ANSI cursor locations (1,1) upper left corner.
  switch (c) {
    case 'A': // cursor UP, 1 param, default 1
      cursorUp(getParamA(1));
      break;
    case 'B': // cursor DOWN, 1 param, default 1
      cursorDown(getParamA(1));
      break;
    case 'C': // cursor RIGHT, 1 param, default 1
      cursorRight(getParamA(1));
      break;
    case 'D': // cursor LEFT, 1 param, default 1
      cursorLeft(getParamA(1));
      break;
    case 'E': // cursor next line, 1 param, default 1
      cursorDown(getParamA(1));
      gotox(0);
      break;
    case 'F': // cursor prev line, 1 param, default 1
      cursorUp(getParamA(1));
      gotox(0);
      break;
    case 'G': // set cursor column, 1 param, default 1
      cursorGoto(getParamA(1),twherey() + 1);
      break;
    case 'H': // set position, 2 param, defaults 1, 1
      cursorGoto(getParamB(1), getParamA(1));
      break;
    case 'f': // synonym
      cursorGoto(getParamB(1), getParamA(1));
      break;
    case 'J': // erase in display, 1 param, default 0
      eraseDisplay(getParamA(0));
      break;
    case 'K': // erase in line, 1 param, default 0
      eraseLine(getParamA(0));
      break;
    case 'S': // scroll up lines, 1 param, default 1
      scrollUp(getParamA(1));
      break;
    case 'T': // scroll down lines, 1 param, default 1
      charout('^'); // just note that I didn't handle this.
      break;
    case 'm': // color (SGR), n params
      doSGRCommand();
      break;
    case 'n': // some introspection
      if (getParamA(0) == 6) {
        if (identify_cb) {
          identify_cb(0x8000 | wherex() << 8 | wherey());
        }
      }
      break;
    case 's': // store cursor, no params
      cursor_store_x = wherex();
      cursor_store_y = twherey();
      break;
    case 'u': // restore cursor, no params.
      tgotoxy(cursor_store_x, cursor_store_y);
      break;
    case 'c': // identify term type.
      if (identify_cb) {
        identify_cb(0);
      }
      break;
  }
}

#define ESC_OPEN 1
#define ESC_CLOSED 0
#define ESC_FIVE 5
#define ESC_SIX 6
#define ESC_Y 'Y'
#define ESC_X 'X'

char esc_state = ESC_OPEN;

int doEscCommand(unsigned char c) {
  if (esc_state == ESC_OPEN) {
    switch (c) {
      case '7': // save cursor
        cursor_store_x = wherex();
        cursor_store_y = twherey();
        break;
      case '8': // restore cursor
        tgotoxy(cursor_store_x, cursor_store_y);
        break;
      case '5': // possible status
        esc_state = ESC_FIVE;
        break;
      case '6': // possible status
        esc_state = ESC_SIX;
        break;
      case 'Z': // vt52 ident
        if (identify_cb) {
          identify_cb(52);
        }
        break;
      case 'Y': // vt52 goto
        esc_state = ESC_Y;
        break;
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
      case '\v':
        vdpchar(conio_getvram(), c);
        ++conio_x;
        if (conio_x > nTextEnd - nTextRow) {
          conio_x = 0;
          inc_row();
        }
        return 1;
      default:
        charout(c);
        return 1; // done.
    }
  } else {
    if (esc_state == ESC_Y) {
      gotoy(c - 32);
      return 0;
    } else if (esc_state == ESC_X) {
      gotox(c - 32);
      return 1;
    } else if (c == 'n') {
      if (esc_state == ESC_FIVE) {
        return 1;
      } else if (esc_state == ESC_SIX) {
        return 1;
      }
    }
  }
  return 0;
}

void charout(unsigned char ch) {
  switch (ch) {
    case '\n': // line feed
      conio_x=0;
      inc_row();
      break;
    default: // it is important to handle control codes before choosing to wrap to next line.
      if (conio_x > nTextEnd-nTextRow) {
        conio_x=0;
        inc_row();
      }
      cputc(ch);
    break;
  }
}

void tputc(int c) {
  if (stage == STAGE_OPEN) {
    if (c == 27) {
      stage = STAGE_ESC;
      esc_state = ESC_OPEN;
    } else {
      charout(c);
    }
  } else if (stage == STAGE_ESC) {
    if (c == '[') {
      // command begins
      stage = STAGE_CSI;
      bs_idx = 0;
      bytestr[0] = 0;
    } else {
      // pass on to legacy state machine.
      int escDone = doEscCommand(c);
      if (escDone) {
        stage = STAGE_OPEN;
        esc_state = ESC_CLOSED;
      }
    }
  } else if (stage == STAGE_CSI) {
    if (c >= 0x40 && c <= 0x7E) {
      // command complete
      doCsiCommand(c);
      stage = STAGE_OPEN;
    } else if (c >= 0x30 && c <= 0x3F) {
      // capture params. If we still have room in our buffer
      if (bs_idx < 128) {
        bytestr[bs_idx] = c;
        bs_idx++;
        bytestr[bs_idx] = 0;
      }
    } else {
      // this is basically an error state... ignoring the command.
      stage = STAGE_OPEN;
    }
  }
}

void tputs_ram(const char* str) {
  while(*str) {
    tputc(*str++);
  }
}

void vdp_setchar(int pAddr, int ch) {
  vdpchar(pAddr, ch);
}
