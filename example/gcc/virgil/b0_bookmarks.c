#include <fc_api.h>
#include "bookmarks.h"
#include "gemini.h"
#include "page.h"

#define SAMS_ADDR 0xF000
#define BOOKMARKS ((struct List*) (SAMS_ADDR))

void map_bookmarks() {
  fc_sams_map_page(state.bookmarks_id, SAMS_ADDR);
}

void FC_SAMS(_BOOKMARKS_BANK,init_bookmarks()) {
  state.history_id = fc_sams_alloc_pages(1);
  map_bookmarks();
  fc_strset((char*)SAMS_ADDR, 0, 0x1000); 
  fc_list_init(BOOKMARKS, (char*)(SAMS_ADDR + 8), (char*)(SAMS_ADDR + 0x0FFF));
}

void FC_SAMS(_BOOKMARKS_BANK,bookmarks_add_link(char* link)) {
  // page lines and history are banked into same address space
  // copy it to stack temporarily
  char tmp[256];
  fc_strcpy(tmp, link);
  // now bring in the history page
  map_bookmarks();

  int len = fc_strlen(tmp);
  fc_list_push(BOOKMARKS, tmp, len);
}

// dst must not be in top address bank
void FC_SAMS(_BOOKMARKS_BANK,bookmarks_get_prev(char* dst)) {
  map_bookmarks();
  fc_list_pop(BOOKMARKS, dst, 256);
  struct ListEntry* entry = fc_list_get(BOOKMARKS, 0);
  if (entry) {
    fc_strcpy(dst, entry->data);
  } else {
    dst[0] = 0;
  }
}

// dst must not be in top address bank
int FC_SAMS(_BOOKMARKS_BANK,bookmarks_get(char* dst, int i)) {
  map_bookmarks();
  struct ListEntry* entry = fc_list_get(BOOKMARKS, i);
  if (entry == 0) {
    *dst = 0;
    return 0;
  }
  fc_strncpy(dst, entry->data, entry->length);
  return entry->length;
}

void FC_SAMS(_BOOKMARKS_BANK,show_bookmarks()) {
  char link[4] = "=> ";
  char nl[2] = "\n";
  char line[256];
  fc_strset(line, 0, 256);
  fc_strcpy(line, "# Bookmarks\n\n");
  page_from_buf(line, fc_strlen(line));
  int hist_idx = 0;
  int len = 1;
  while(len > 0) {
    fc_strset(line, 0, 256);
    len = bookmarks_get(line, hist_idx++);
    if (len > 0) {
      page_from_buf(link, 3);
      page_from_buf(line, len);
      page_from_buf(nl, 1);
    }
  }
}

