#include <fc_api.h>
#include "history.h"
#include "gemini.h"
#include "page.h"

#define SAMS_ADDR 0xF000
#define HISTORY ((struct List*) (SAMS_ADDR))

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
}

// dst must not be in top address bank
void history_get_prev(char* dst) {
  map_history();
  fc_list_pop(HISTORY, dst, 256);
  struct ListEntry* entry = fc_list_get(HISTORY, 0);
  if (entry) {
    fc_strcpy(dst, entry->data);
  } else {
    dst[0] = 0;
  }
}

