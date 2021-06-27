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
void layoutMenu(int entry_offset);
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
  layoutMenu(entry_idx);

  struct MenuEntry* entry = 0;
  while(entry == 0) {
    entry = pickEntry();
    if (((int)entry) == 0xFFFF) {
      return 0;
    } else {
      if (((int)entry) != 0) {
        fc_exec(entry->command);
        entry = 0;
        drawBackdrop();
        layoutMenu(entry_idx);
      }
    }
  }
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

void layoutMenu(int entry_offset) {
  int y = 3;
  int limit = 10 > entry_max ? entry_max : 10;
  for(int i = entry_idx; i<limit; i++) {
    fc_ui_gotoxy(1, y);
    if (entries[i].key == '-') {
      // draw a separation
      int vaddr = fc_vdp_get_cursor_addr();
      vdp_memset(vaddr, 0xC4, 18);
    } else {
      fc_tputc(entries[i].key);
      fc_ui_gotoxy(3, y);
      fc_tputs(entries[i].title);
    }
    y += 2;
  }
  fc_ui_gotoxy(0, dinfo.displayHeight - 1);
}

struct MenuEntry* pickEntry() {
  int limit = 10 > entry_max ? entry_max : 10;
  unsigned int key = fc_kscan(5);
  if (key == KEY_BACK) {
    return (struct MenuEntry*) 0xFFFF;
  }
  for(int i = entry_idx; i<limit; i++) {
    if (entries[i].key == key) {
      return &entries[i];
    }
  }
  return 0;
}
