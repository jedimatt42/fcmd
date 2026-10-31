#ifndef _LABELLIST_H
#define _LABELLIST_H

#include "banking.h"

#define LABEL_POOL_SIZE 32

struct LabelEntry {
  char name[12]; // room for 11 characters
  unsigned int lineno;
};

struct LabelPool {
  struct LabelEntry entries[LABEL_POOL_SIZE];
  int next;
  struct LabelPool* prev;
};

// Enter/leave a script label pool. Enter saves the previous pool and clears
// the new one; leave restores the previous pool (NULL at top level).
void labels_enter(struct LabelPool* pool);
void labels_leave(void);

void labels_clear();
int labels_get(char* name);
void labels_add(char* name, int lineno);

DECLARE_BANKED_VOID(labels_enter, BANK(4), bk_labels_enter, (struct LabelPool* pool), (pool))
DECLARE_BANKED_VOID(labels_leave, BANK(4), bk_labels_leave, (), ())
DECLARE_BANKED_VOID(labels_clear, BANK(4), bk_labels_clear, (), ())
DECLARE_BANKED(labels_get, BANK(4), int, bk_labels_get, (char* name), (name))
DECLARE_BANKED_VOID(labels_add, BANK(4), bk_labels_add, (char* name, int lineno), (name, lineno))

#endif
