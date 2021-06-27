#include "fc_api.h"
#include "ioports.h"
#include "kscan.h"

struct DisplayInformation dinfo;

struct __attribute__((__packed__)) MenuEntry {
  char key;
  char title[20];
  char command[80];
};

int entry_idx;
int entry_max;
struct MenuEntry entries[100];

void drawBackdrop();
void layoutMenu();
struct MenuEntry* pickEntry();

int main(char* args) {
  fc_display_info(&dinfo);

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

  drawBackdrop();
  layoutMenu();

  struct MenuEntry* entry = 0;
  while(entry == 0) {
    entry = pickEntry();
    if (((int)entry) == 0xFFFF) {
      break;
    } else {
      if (((int)entry) != 0) {
        fc_exec(entry->command);
        entry = 0;
        drawBackdrop();
        layoutMenu();
      }
    }
  }
  fc_exec("CLS");
  return 0;
}

void drawBackdrop() {
  vdp_memset(0, ' ', dinfo.displayWidth * dinfo.displayHeight);
  fc_ui_gotoxy(0,0);
  vdp_memset(0, 0xB0, dinfo.displayWidth);
  vdp_memset((dinfo.displayHeight - 1) * dinfo.displayWidth, 0xB0, dinfo.displayWidth);
  fc_ui_gotoxy((dinfo.displayWidth / 2) - 7,0);
  fc_tputs(" FCMenu v1.0 ");
}

void layoutMenu() {
  int y = 3;
  int disp_limit = dinfo.displayWidth == 40 ? 20 : 40;
  int limit = disp_limit > entry_max ? entry_max : disp_limit;
  int column = 1;

  for(int i = entry_idx; i<limit; i++) {
    int itemcount = i - entry_idx;
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
  }
  fc_ui_gotoxy(0, dinfo.displayHeight - 1);
}

struct MenuEntry* pickEntry() {
  int limit = 10 > entry_max ? entry_max : 10;
  unsigned int key = fc_kscan(5);
  if (KSCAN_STATUS & KSCAN_MASK) {
    if (key == KEY_BACK) {
      return (struct MenuEntry*) 0xFFFF;
    }
    for(int i = entry_idx; i<limit; i++) {
      if (entries[i].key == key) {
        return &entries[i];
      }
    }
  }
  return 0;
}
