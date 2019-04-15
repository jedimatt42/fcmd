#ifndef _LABELLIST_H
#define _LABELLIST_H

#include "banking.h"

void labels_clear();
int labels_get(char* name);
void labels_add(char* name, int lineno);

DECLARE_BANKED_VOID(labels_clear, BANK_4, bk_labels_clear, (), ())
DECLARE_BANKED(labels_get, BANK_4, int, bk_labels_get, (char* name), (name))
DECLARE_BANKED_VOID(labels_add, BANK_4, bk_labels_add, (char* name, int lineno), (name, lineno))

#endif