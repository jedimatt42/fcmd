#include "banks.h"
#define MYBANK BANK(4)

#include "b4_labellist.h"
#include "b8_terminal.h"
#include "b1_strutil.h"
#include "string.h"

struct __attribute__((__packed__)) LabelEntry {
  char name[12]; // room for 11 characters
  unsigned int lineno;
};

struct LabelEntry labels[50];
int label_next = 0;

void labels_clear() {
  label_next = 0;
}

int labels_get(char* name) {
  for(int i = 0; i < label_next; i++) {
    if (bk_str_equals(name, labels[i].name)) {
      return labels[i].lineno;
    }
  }
  return -1;
}

void labels_add(char* name, int gotoline) {
  if (bk_strlen(name) > 10) {
    tputs_rom("Error, label name too long, limit is 10\n");
    return;
  }
  int existing = labels_get(name);
  if (existing != -1) {
    tputs_rom("Error, label already defined at line: ");
    bk_tputs_ram(bk_uint2str(existing));
    bk_tputc('\n');
    return;
  }

  if (label_next >= 50) {
    tputs_rom("Error, no more room in label dictionary\n");
    return;
  }
  bk_strcpy(labels[label_next].name, name);
  labels[label_next].lineno = gotoline;
  label_next++;
}

