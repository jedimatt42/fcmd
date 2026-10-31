#include "banks.h"
#define MYBANK BANK(4)

#include "labellist.h"
#include "terminal.h"
#include "strutil.h"
#include "string.h"

static struct LabelPool* label_pool = 0;

void labels_enter(struct LabelPool* pool) {
  pool->next = 0;
  pool->prev = label_pool;
  label_pool = pool;
}

void labels_leave(void) {
  if (label_pool) {
    label_pool = label_pool->prev;
  }
}

void labels_clear() {
  if (label_pool) {
    label_pool->next = 0;
  }
}

int labels_get(char* name) {
  if (label_pool == 0) {
    return -1;
  }
  for(int i = 0; i < label_pool->next; i++) {
    if (bk_str_equals(name, label_pool->entries[i].name)) {
      return label_pool->entries[i].lineno;
    }
  }
  return -1;
}

void labels_add(char* name, int gotoline) {
  if (label_pool == 0) {
    return;
  }

  if (bk_strlen(name) > 10) {
    tputs_rom("Error, label name too long, limit is 10\n");
    return;
  }
  int existing = labels_get(name);
  if (existing == gotoline) {
    return; // all done... already defined.
  }
  if (existing != -1) {
    tputs_rom("Error, label already defined at line: ");
    bk_tputs_ram(bk_uint2str(existing));
    bk_tputc('\n');
    return;
  }

  if (label_pool->next >= LABEL_POOL_SIZE) {
    tputs_rom("Error, no more room in label dictionary\n");
    return;
  }
  bk_strcpy(label_pool->entries[label_pool->next].name, name);
  label_pool->entries[label_pool->next].lineno = gotoline;
  label_pool->next++;
}
