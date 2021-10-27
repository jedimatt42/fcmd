#ifndef _HISTORY_H
#define _HISTORY_H 1

#include "page.h"

struct __attribute__((__packed__)) Link {
  char data[80];
};

void init_history();
void history_add_link(char* link);
void history_get_prev(char* dst);

#endif

