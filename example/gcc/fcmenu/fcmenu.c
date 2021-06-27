#include "fc_api.h"
#include "ioports.h"

struct __attribute__((__packed__)) MenuEntry {
  char key;
  char title[20];
  char command[80];
};

int entry_idx;
int entry_max;
struct MenuEntry entries[100];

int main(char* args) {
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
          // if not blank, and not starts with # comment
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

  for(int i=0; i<entry_max; i++) {
    fc_tputc(entries[i].key);
    fc_tputc('\n');
    fc_tputs(entries[i].title);
    fc_tputc('\n');
    fc_tputs(entries[i].command);
    fc_tputc('\n');
  }

  return 0;
}
