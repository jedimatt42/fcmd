#include <fc_api.h>
#include "history.h"
#include "gemini.h"
#include "page.h"

#define SAMS_ADDR 0xF000
#define HISTORY ((struct List*) (SAMS_ADDR))

void map_history() {
  fc_sams_map_page(state.history_id, SAMS_ADDR);
}

void FC_SAMS(0,init_history()) {
  state.history_id = fc_sams_alloc_pages(1);
  map_history();
  fc_strset((char*)SAMS_ADDR, 0, 0x1000); 
  fc_list_init(HISTORY, (char*)(SAMS_ADDR + 6), (char*)(SAMS_ADDR + 0x0FFF));
}

void FC_SAMS(0,history_push(char* link)) {
  // page lines and history are banked into same address space
  // copy it to stack temporarily
  char tmp[256];
  fc_strcpy(tmp, link);
  // now bring in the history page
  map_history();

  int len = fc_strlen(tmp);
  fc_list_push(HISTORY, tmp, len + 1);
  state.history_count++;
}

// dst must not be in top address bank
void FC_SAMS(0,history_pop(char* dst)) {
  if (state.history_count > 0) {
    map_history();
    fc_strset(dst, 0, 256);
    fc_list_pop(HISTORY, dst, 256);
    state.history_count--;
  } else {
    fc_strcpy(dst, "history:");
  }
}

// dst must not be in top address bank
int FC_SAMS(0,history_get(char* dst, int i)) {
  map_history();
  struct ListEntry* entry = fc_list_get(HISTORY, i);
  if (entry == 0) {
    *dst = 0;
    return 0;
  }
  fc_strncpy(dst, entry->data, entry->length);
  return entry->length;
}

void FC_SAMS(0,show_history()) {
  char link[4] = "=> ";
  char nl[2] = "\n";
  char line[256];
  fc_strset(line, 0, 256);
  fc_strcpy(line, "# Virgil99 Browser History\n\n");
  page_from_buf(line, fc_strlen(line));
  int link_idx = 0;
  int len = 1;
  while(len > 0) {
    fc_strset(line, 0, 256);
    len = history_get(line, link_idx++);
    if (len > 0) {
      page_from_buf(link, 3);
      page_from_buf(line, len);
      page_from_buf(nl, 1);
    }
  }
}

