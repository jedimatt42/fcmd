#include <fc_api.h>
#include "history.h"
#include "gemini.h"
#include "page.h"

#define SAMS_ADDR 0xF000
#define HIST_IDX *((volatile int*) SAMS_ADDR)
#define HISTORY ((struct List*) (SAMS_ADDR + 2))

void map_history() {
  fc_sams_map_page(state.history_id, SAMS_ADDR);
}

void init_history() {
  state.history_id = fc_sams_alloc_pages(1);
  map_history();
  fc_strset((char*)SAMS_ADDR, 0, 0x1000); 
  fc_list_init(HISTORY, (char*)(SAMS_ADDR + 8), (char*)(SAMS_ADDR + 0x0FFF));
}

void history_add_link(char* link) {
  // page lines and history are banked into same address space
  // copy it to stack temporarily
  char tmp[80];
  fc_strcpy(tmp, link);
  // now bring in the history page
  map_history();

  int len = fc_strlen(tmp);
  fc_list_push(HISTORY, tmp, len);
  HIST_IDX = 0;
}

// dst must not be in top address bank
void history_get_prev(char* dst) {
  map_history();
  HIST_IDX = HIST_IDX + 1;
  struct ListEntry* entry = fc_list_get(HISTORY, HIST_IDX);
  if (entry) {
    fc_strcpy(dst, entry->data);
  }
}

void history_show() {
}


