#include "banks.h"
#define MYBANK BANK_4

#include "b4_labellist.h"
#include "b1cp_terminal.h"
#include "b1cp_strutil.h"
#include "string.h"

struct __attribute__((__packed__)) LabelEntry {
  unsigned int lineno;
  char name[12]; // room for 11 characters
};

struct LabelEntry labels[50];
int label_next = 0;

void labels_clear() {
  label_next = 0;
}

int labels_get(char* name) {
  for(int i = 0; i < label_next; i++) {
    if (0 == strcmpi(name, labels[i].name)) {
      return labels[i].lineno;
    }
  }
  return -1;
}

void labels_add(char* name, int lineno) {
  if (label_next >= 50) {
    tputs("Error, no more room in label dictionary\n");
    return;
  }
  strcpy(labels[label_next].name, name);
  labels[label_next].lineno = lineno;
  label_next++;
}

