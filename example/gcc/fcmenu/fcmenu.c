#include "fc_api.h"
#include "ioports.h"
#include "kscan.h"

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

void drawBackdrop();
void layoutMenu();
struct MenuEntry* pickEntry(int key);
int readKeyboard();
int handleKey(int key);
void previousPage();
void nextPage();
void drawClock();

#define VDP_WAIT_VBLANK_CRU	  __asm__( "clr r12\n\ttb 2\n\tjeq -4\n\tmovb @>8802,r12" : : : "r12" );


int main(char* args) {
  fc_display_info(&dinfo);
  disp_limit = dinfo.displayWidth == 40 ? 20 : 40;

  entry_idx = 0;
  entry_max = 0;
  struct DeviceServiceRoutine* dsr;
  char pathname[80];

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
          entries[entry_max].key = buffer[0];

          int first_delim = fc_indexof(buffer, '|');
          int second_delim = first_delim + fc_indexof(buffer+first_delim+1, '|');

          int title_len = second_delim - first_delim;
          if (title_len > 20) {
            title_len = 20;
          }
          fc_strncpy(entries[entry_max].title, buffer + first_delim + 1, second_delim - first_delim);

          fc_strncpy(entries[entry_max].command, buffer + first_delim + second_delim + 1, 80);
          entry_max++;
        }
      }
    }
  }
  fc_dsr_close(dsr, &pab);
  page_total = (entry_max / disp_limit) + 1;

  drawBackdrop();
  layoutMenu();

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

    int key = readKeyboard();
    switch(key) {
      case KEY_BACK:
        fc_exec("CLS");
        return 0;
      case ',':
        previousPage();
        break;
      case '.':
        nextPage();
        break;
      default:
        {
          struct MenuEntry* entry = pickEntry(key);
          if (entry != 0) {
            fc_exec(entry->command);
            drawBackdrop();
            layoutMenu();
          }
        }
        break;
    }
  }
}

void drawBackdrop() {
  vdp_memset(0, ' ', dinfo.displayWidth * dinfo.displayHeight);
  fc_ui_gotoxy(0,0);
  vdp_memset(0, 0xB0, dinfo.displayWidth);
  vdp_memset((dinfo.displayHeight - 1) * dinfo.displayWidth, 0xB0, dinfo.displayWidth);
  fc_ui_gotoxy((dinfo.displayWidth / 2) - 7,0);
  fc_tputs(" FCMenu v1.0 ");
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

void layoutMenu() {
  int page_total = (entry_max / disp_limit) + 1;
  int page = (entry_idx / disp_limit) + 1;

  int y = 3;
  int limit = entry_idx + disp_limit;
  if (limit > entry_max) {
    limit = entry_max;
  }
  int column = 1;

  int itemcount = 0;
  for(int i = entry_idx; i<limit; i++) {
    if (itemcount >= 30) {
      column = 61;
    } else if (itemcount >= 20) {
      column = 41;
    } else if (itemcount >= 10) {
      column = 21;
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

struct MenuEntry* pickEntry(int key) {
  int limit = entry_idx + disp_limit;
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
