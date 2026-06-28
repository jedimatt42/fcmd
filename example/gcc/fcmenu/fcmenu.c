#include <fc_api.h>
#include <ioports.h>
#include <kscan.h>

struct DisplayInformation dinfo;

struct MenuEntry {
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
int childWantsQuit();

#define VDP_WAIT_VBLANK_CRU	  __asm__( "clr r12\n\ttb 2\n\tjeq -4\n\tmovb @>8802,r12" : : : "r12" );

#define QUITVAR "FCM"

int fcmain(char* args) {
  sys_display_info(&dinfo);
  disp_limit = dinfo.displayWidth == 40 ? 20 : 40;

  entry_idx = 0;
  entry_max = 0;
  struct DeviceServiceRoutine* dsr;
  char pathname[80];
  selection = 0;

  path_parse(args, &dsr, pathname, PR_REQUIRED);
  if (!dsr) {
    term_puts("menu data file must be specified\n");
    return 1;
  }

  struct PAB pab;
  int ferr = dsr_open(dsr, &pab, pathname, DSR_TYPE_INPUT | DSR_TYPE_VARIABLE, 80);
  if (ferr) {
    term_puts("could not open ");
    term_puts(pathname);
    term_putc('\n');
    return 1;
  }

  int recordno = 0;
  while(!ferr) {
    char buffer[80];
    for (int c=0; c<80; c++) {buffer[c] = 0;}

    ferr = dsr_read_cpu(dsr, &pab, recordno++, buffer);
    if (!ferr) {
      // records are a <key>|<title>|<command>
      if (pab.CharCount) {
        if (buffer[0] != '#') {
          // if not blank, and not starts with # comment, build an entry
          // (force uppercase)
          if (buffer[0] >= 'a' && buffer[0] <= 'z') {
            buffer[0] -= 'a' - 'A';
          }
          entries[entry_max].key = buffer[0];

          int first_delim = str_index_of(buffer, '|');
          int second_delim = first_delim + str_index_of(buffer+first_delim+1, '|');

          int title_len = second_delim - first_delim;
          if (title_len > 17) {
            title_len = 17;
          }
          str_ncopy(entries[entry_max].title, buffer + first_delim + 1, title_len);

          str_ncopy(entries[entry_max].command, buffer + first_delim + second_delim + 1, 80);
          entry_max++;
        }
      }
    }
  }
  dsr_close(dsr, &pab);
  page_total = (entry_max / disp_limit) + 1;

  // slow clear once
  exec_cmd("CLS");
  // then color free clearing...
  drawBackdrop();
  layoutMenu();

  mouseOn = 0;
  struct MouseData mouseData;

  mouse_show(&mouseData);
  mouse_hide(); // hide the mouse for now

  updateMouse(&mouseData); // clear any click queued by tipi

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
    ui_gotoxy(1, dinfo.displayHeight - 1);

    int buttons = updateMouse(&mouseData);
    if (buttons & MB_LEFT) {
      handleClick(mouseData.pointerx, mouseData.pointery);
    }
    int key = readKeyboard();
    if (key) {
      handleKey(key);
      if (key == KEY_BACK) {
        cleanupBeforeExit();
        return 0;
      }
    }
    if (childWantsQuit()) {
      cleanupBeforeExit();
      return 0;
    }
  }
}

int childWantsQuit() {
  char* value = var_get(QUITVAR);
  return !str_cmp("QUIT", value);
}

void handleKey(int key) {
  switch(key) {
    case KEY_BACK:
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
  mouse_hide();
  exec_cmd("CLS");
  var_set(QUITVAR, "");
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
  mouse_hide();
  exec_cmd(entry->command);
  drawBackdrop();
  layoutMenu();
}

void drawBackdrop() {
  vdp_memset(0, ' ', dinfo.displayWidth * dinfo.displayHeight);
  ui_gotoxy(0,0);
  vdp_memset(0, 0xB0, dinfo.displayWidth);
  vdp_memset((dinfo.displayHeight - 1) * dinfo.displayWidth, 0xB0, dinfo.displayWidth);
  ui_gotoxy((dinfo.displayWidth / 2) - 7,0);
  term_puts(" FCMenu v1.2 ");
  cycles = 0; // since we erased the clock, allow it to redraw on next attempt
}

void drawClock() {
  struct DateTime dt;
  time_get(&dt);
  if (dt.hours != 0 && dt.minutes != 0) {
    ui_gotoxy(dinfo.displayWidth - 9, 0);
    term_putc(' ');
    term_puts(str_from_uint(dt.hours));
    term_putc(':');
    if (dt.minutes < 10) {
      term_putc('0');
    }
    term_puts(str_from_uint(dt.minutes));
    if (dt.pm) {
      term_putc('p');
    }
    else {
      term_putc('a');
    }
    term_puts("m ");
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
  ui_gotoxy(x, y);
  term_putc(' ');
}

void drawSelection() {
  int x;
  int y;
  selection_x_y(&x, &y);
  ui_gotoxy(x, y);
  term_putc(0x1A);
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
    ui_gotoxy(column, y);
    if (entries[i].key == '-') {
      // draw a separation
      int vaddr = vdp_cursor_addr();
      vdp_memset(vaddr, 0xC4, 18);
    } else {
      term_putc(entries[i].key);
      ui_gotoxy(column + 2, y);
      term_puts(entries[i].title);
      if (selection == i) {
        ui_gotoxy(column - 1, y);
        term_putc(0x1A);
      }
    }
    y += 2;
    if (y > 22) {
      y = 3;
    }
    itemcount++;
  }

  // draw a page indicator
  ui_gotoxy(dinfo.displayWidth - 8, dinfo.displayHeight - 1);
  term_puts("Page ");
  term_puts(str_from_uint(page));
  term_putc('/');
  term_puts(str_from_uint(page_total));

  ui_gotoxy(0, dinfo.displayHeight - 1);
}

int readKeyboard() {
  unsigned int key = term_kscan(5);
  if (KSCAN_STATUS & KSCAN_MASK) {
    return key;
  } else {
    return 0;
  }
}

int updateMouse(struct MouseData* mouseData) {
  mouse_move(mouseData);
  if (!mouseOn && (mouseData->mx != 0 || mouseData->my != 0)) {
    mouseOn = 1;
    mouse_show(mouseData);
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
