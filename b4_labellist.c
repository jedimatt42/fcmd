#include "banks.h"
#define MYBANK BANK_4

#include "b4_labellist.h"
#include "b1cp_terminal.h"
#include "b1cp_strutil.h"
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
    if (str_equals(name, labels[i].name)) {
      return labels[i].lineno;
    }
  }
  return -1;
}

void labels_add(char* name, int gotoline) {
  if (strlen(name) > 10) {
    tputs_rom("Error, label name too long, limit is 10\n");
    return;
  }
  int existing = labels_get(name);
  if (existing != -1) {
    tputs_rom("Error, label already defined at line: ");
    tputs_ram(uint2str(existing));
    tputc('\n');
    return;
  }

  if (label_next >= 50) {
    tputs_rom("Error, no more room in label dictionary\n");
    return;
  }
  strcpy(labels[label_next].name, name);
  labels[label_next].lineno = gotoline;
  label_next++;
}

