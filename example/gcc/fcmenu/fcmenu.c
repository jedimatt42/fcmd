#include <fc_api.h>
#include <ioports.h>
#include <kscan.h>

struct DisplayInformation dinfo;

struct __attribute__((__packed__)) MenuEntry {
  char key;
  char title[20];
  char command[80];
};

int cycles;
volatile int entry_idx;
int entry_max;
struct MenuEntry entries[100];
int disp_limit;
int page_total;
int selection;
int mouseOn;

void drawBackdrop();
void layoutMenu();
struct MenuEntry* pickEntry(int key);
int readKeyboard();
void handleKey(int key);
int updateMouse(struct MouseData* mouseData);
void handleClick(int x, int y);
void previousPage();
void nextPage();
void drawClock();
void selectionUp();
void selectionDown();
void selectionLeft();
void selectionRight();
void selectionRun(struct MenuEntry* entry);
void clearSelection();
void drawSelection();
void skipSeparatorNext();
void skipSeparatorPrev();
void cleanupBeforeExit();

#define VDP_WAIT_VBLANK_CRU	  __asm__( "clr r12\n\ttb 2\n\tjeq -4\n\tmovb @>8802,r12" : : : "r12" );


int fcmain(char* args) {
  fc_display_info(&dinfo);
  disp_limit = dinfo.displayWidth == 40 ? 20 : 40;

  entry_idx = 0;
  entry_max = 0;
  struct DeviceServiceRoutine* dsr;
  char pathname[80];
  selection = 0;

  fc_parse_path_param(args, &dsr, pathname, PR_REQUIRED);
  if (!dsr) {
    fc_tputs("menu data file must be specified\n");
    return 1;
  }

  struct PAB pab;
  int ferr = fc_dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_VARIABLE, 80);
  if (ferr) {
    fc_tputs("could not open ");
    fc_tputs(pathname);
    fc_tputc('\n');
    return 1;
  }

  int recordno = 0;
  while(!ferr) {
    ferr = fc_dsr_read(dsr, &pab, recordno++);
    if (!ferr) {
      // records are a <key>|<title>|<command>
      if (pab.CharCount) {
        char buffer[80];
        for (int c=0; c<80; c++) {buffer[c] = 0;}

        vdp_memread(pab.VDPBuffer, buffer, pab.CharCount);
        if (buffer[0] != '#') {
          // if not blank, and not starts with # comment, build an entry
          // (force uppercase)
          if (buffer[0] >= 'a' && buffer[0] <= 'z') {
            buffer[0] -= 'a' - 'A';
          }
          entries[entry_max].key = buffer[0];

          int first_delim = fc_indexof(buffer, '|');
          int second_delim = first_delim + fc_indexof(buffer+first_delim+1, '|');

          int title_len = second_delim - first_delim;
          if (title_len > 17) {
            title_len = 17;
          }
          fc_strncpy(entries[entry_max].title, buffer + first_delim + 1, title_len);

          fc_strncpy(entries[entry_max].command, buffer + first_delim + second_delim + 1, 80);
          entry_max++;
        }
      }
    }
  }
  fc_dsr_close(dsr, &pab);
  page_total = (entry_max / disp_limit) + 1;

  // slow clear once
  fc_exec("CLS");
  // then color free clearing...
  drawBackdrop();
  layoutMenu();

  mouseOn = 0;
  struct MouseData mouseData;

  fc_tipi_mouse_enable(&mouseData);
  fc_tipi_mouse_disable(); // hide the mouse for now

  cycles = 0;
  while(1) {
    if (dinfo.isPal ? (cycles == 50 * 60) : (cycles == 60 * 60)) {
      cycles = 0;
    }
    if (cycles == 0) {
      drawClock();
    }

    VDP_WAIT_VBLANK_CRU;
    cycles++;
    fc_ui_gotoxy(1, dinfo.displayHeight - 1);

    int buttons = updateMouse(&mouseData);
    if (buttons & MB_LEFT) {
      handleClick(mouseData.pointerx, mouseData.pointery);
    }
    int key = readKeyboard();
    if (key) {
      handleKey(key);
      if (key == KEY_BACK) {
        return 0;
      }
    }
  }
}

void handleKey(int key) {
  switch(key) {
    case KEY_BACK:
      cleanupBeforeExit();
      break;
    case ',':
      previousPage();
      break;
    case '.':
      nextPage();
      break;
    case KEY_DOWN:
      selectionDown();
      break;
    case KEY_UP:
      selectionUp();
      break;
    case KEY_RIGHT:
      selectionRight();
      break;
    case KEY_LEFT:
      selectionLeft();
      break;
    case KEY_ENTER:
      selectionRun(&(entries[selection]));
      break;
    default:
      {
        struct MenuEntry* entry = pickEntry(key);
        if (entry != 0) {
          selectionRun(entry);
        }
      }
      break;
  }
}

void handleClick(int x, int y) {
  // find 'widget' that was clicked on...
  int i = entry_idx;
  while(i < (disp_limit + entry_idx) && i < entry_max) {
    if (entries[i].key != '-') {
      int col = i / 10;
      int ex = 8 + 3 + (col * 18 * 3);
      int exl = ex + (18*3);

      int row = i % 10;
      int ey = (row * 16) + 15;
      int eyl = ey + 8;

      if (x >= ex && x <= exl) {
        if (y >= ey && y <= eyl) {
          selectionRun(&(entries[i]));
          return;
        }
      }
    }
    i++;
  }
}

void cleanupBeforeExit() {
  fc_tipi_mouse_disable();
  fc_exec("CLS");
}

void selectionUp() {
  clearSelection();
  if (selection > 0) {
    int old_page_offset = selection % disp_limit;
    selection--;
    int new_page_offset = selection % disp_limit;
    if (old_page_offset < new_page_offset) {
      previousPage();
    }
  }
  skipSeparatorPrev();
  drawSelection();
}

void selectionDown() {
  clearSelection();
  if (selection < (entry_max - 1)) {
    int old_page_offset = selection % disp_limit;
    selection++;
    int new_page_offset = selection % disp_limit;
    if (old_page_offset > new_page_offset) {
      nextPage();
    }
  }
  skipSeparatorNext();
  drawSelection();
}

void selectionRight() {
  clearSelection();
  if ((selection + 10) <= (entry_max - 1)) {
    int old_page_offset = selection % disp_limit;
    selection += 10;
    int new_page_offset = selection % disp_limit;
    if (old_page_offset > new_page_offset) {
      nextPage();
    }
  }
  skipSeparatorNext();
  drawSelection();
}

void selectionLeft() {
  clearSelection();
  if ((selection - 10) >= 0) {
    int old_page_offset = selection % disp_limit;
    selection -= 10;
    int new_page_offset = selection % disp_limit;
    if (old_page_offset < new_page_offset) {
      previousPage();
    }
  }
  skipSeparatorPrev();
  drawSelection();
}

void skipSeparatorNext() {
  if (entries[selection].key == '-') {
    int old_page_offset = selection % disp_limit;
    int new_page_offset = (selection + 1) % disp_limit;
    if (selection < entry_max && new_page_offset > old_page_offset) {
      selection++;
    } else {
      selection--;
    }
  }
}

void skipSeparatorPrev() {
  if (entries[selection].key == '-') {
    int old_page_offset = selection % disp_limit;
    int new_page_offset = (selection - 1) % disp_limit;
    if (selection > 0 && new_page_offset < old_page_offset) {
      selection--;
    } else {
      selection++;
    }
  }
}

void selectionRun(struct MenuEntry* entry) {
  mouseOn = 0;
  fc_tipi_mouse_disable();
  fc_exec(entry->command);
  drawBackdrop();
  layoutMenu();
}

void drawBackdrop() {
  vdp_memset(0, ' ', dinfo.displayWidth * dinfo.displayHeight);
  fc_ui_gotoxy(0,0);
  vdp_memset(0, 0xB0, dinfo.displayWidth);
  vdp_memset((dinfo.displayHeight - 1) * dinfo.displayWidth, 0xB0, dinfo.displayWidth);
  fc_ui_gotoxy((dinfo.displayWidth / 2) - 7,0);
  fc_tputs(" FCMenu v1.1 ");
  cycles = 0; // since we erased the clock, allow it to redraw on next attempt
}

void drawClock() {
  struct DateTime dt;
  fc_datetime(&dt);
  if (dt.hours != 0 && dt.minutes != 0) {
    fc_ui_gotoxy(dinfo.displayWidth - 9, 0);
    fc_tputc(' ');
    fc_tputs(fc_uint2str(dt.hours));
    fc_tputc(':');
    if (dt.minutes < 10) {
      fc_tputc('0');
    }
    fc_tputs(fc_uint2str(dt.minutes));
    if (dt.pm) {
      fc_tputc('p');
    }
    else {
      fc_tputc('a');
    }
    fc_tputs("m ");
  }
}

void selection_x_y(int* rx, int* ry) {
  int psel = (selection % disp_limit);
  int tc = (psel / 10);
  *rx = 1 + (20 * tc);
  *ry = 3 + (2 * psel % 20);
}

void clearSelection() {
  int x;
  int y;
  selection_x_y(&x, &y);
  fc_ui_gotoxy(x, y);
  fc_tputc(' ');
}

void drawSelection() {
  int x;
  int y;
  selection_x_y(&x, &y);
  fc_ui_gotoxy(x, y);
  fc_tputc(0x1A);
}

void layoutMenu() {
  int page_total = (entry_max / disp_limit) + 1;
  int page = (entry_idx / disp_limit) + 1;

  int y = 3;
  int limit = entry_idx + disp_limit;
  if (limit > entry_max) {
    limit = entry_max;
  }
  int column = 2;

  int itemcount = 0;
  for(int i = entry_idx; i<limit; i++) {
    if (itemcount >= 30) {
      column = 62;
    } else if (itemcount >= 20) {
      column = 42;
    } else if (itemcount >= 10) {
      column = 22;
    }
    fc_ui_gotoxy(column, y);
    if (entries[i].key == '-') {
      // draw a separation
      int vaddr = fc_vdp_get_cursor_addr();
      vdp_memset(vaddr, 0xC4, 18);
    } else {
      fc_tputc(entries[i].key);
      fc_ui_gotoxy(column + 2, y);
      fc_tputs(entries[i].title);
      if (selection == i) {
        fc_ui_gotoxy(column - 1, y);
        fc_tputc(0x1A);
      }
    }
    y += 2;
    if (y > 22) {
      y = 3;
    }
    itemcount++;
  }

  // draw a page indicator
  fc_ui_gotoxy(dinfo.displayWidth - 8, dinfo.displayHeight - 1);
  fc_tputs("Page ");
  fc_tputs(fc_uint2str(page));
  fc_tputc('/');
  fc_tputs(fc_uint2str(page_total));

  fc_ui_gotoxy(0, dinfo.displayHeight - 1);
}

int readKeyboard() {
  unsigned int key = fc_kscan(5);
  if (KSCAN_STATUS & KSCAN_MASK) {
    return key;
  } else {
    return 0;
  }
}

int updateMouse(struct MouseData* mouseData) {
  fc_tipi_mouse_move(mouseData);
  if (!mouseOn && (mouseData->mx != 0 || mouseData->my != 0)) {
    mouseOn = 1;
    fc_tipi_mouse_enable(mouseData);
  }
  return mouseData->buttons;
}

struct MenuEntry* pickEntry(int key) {
  int limit = entry_idx + disp_limit;
  if (key >= 'a' && key <= 'z') {
    key -= 'a' - 'A';
  }
  if (limit > entry_max) {
    limit = entry_max;
  }
  for(int i = entry_idx; i<limit; i++) {
    if (entries[i].key == key) {
      return &entries[i];
    }
  }
  return 0;
}

void previousPage() {
  if (entry_idx - disp_limit >= 0) {
    entry_idx -= disp_limit;
    drawBackdrop();
    layoutMenu();
  }
}

void nextPage() {
  if (entry_idx + disp_limit < entry_max) {
    entry_idx += disp_limit;
    drawBackdrop();
    layoutMenu();
  }
}
