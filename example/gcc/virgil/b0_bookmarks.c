#include <fc_api.h>
#include <ioports.h>
#include "bookmarks.h"
#include "gemini.h"
#include "page.h"

#include "debug.h"

#define SAMS_ADDR 0xF000
#define BOOKMARKS ((struct List*) (SAMS_ADDR))

void map_bookmarks() {
  fc_sams_map_page(state.bookmarks_id, SAMS_ADDR);
}

static void load_bookmarks() {
  char* favs = fc_vars_get("VIRGIL_FAVS");
  if (favs == 0 || favs[0] == 0) {
    favs = "TIPI.FC.VIRGIL.BOOKMARKS";
  }
  struct PAB pab;
  struct DeviceServiceRoutine* dsr;
  char filename[30];
  fc_parse_path_param(favs, &dsr, filename, PR_REQUIRED);
  int ferr = fc_dsr_open(dsr, &pab, filename, DSR_TYPE_VARIABLE | DSR_TYPE_INPUT, 80);
  if (ferr) {
    return; // no book marks file to load
  }
  int y = 5;
  map_bookmarks();
  while(!ferr) {
    ferr = fc_dsr_read(dsr, &pab, 0);
    // copy from vdp to cpu ram
    char link[80];
    fc_strset(link, 0, 80);
    vdp_memread(pab.VDPBuffer, link, pab.CharCount);
    // then add to bookmarks
    if (pab.CharCount > 0) { // skip empty lines
      fc_list_push(BOOKMARKS, link, pab.CharCount);
    }
  }

  fc_dsr_close(dsr, &pab);
}

void FC_SAMS(0,init_bookmarks()) {
  state.bookmarks_id = fc_sams_alloc_pages(1);
  map_bookmarks();
  fc_strset((char*)SAMS_ADDR, 0, 0x1000); 
  fc_list_init(BOOKMARKS, (char*)(SAMS_ADDR + 6), (char*)(SAMS_ADDR + 0x0FFF));
  load_bookmarks();
}

void FC_SAMS(0,bookmarks_add_link(char* link)) {
  // page lines and bookmarks are banked into same address space
  // copy it to stack temporarily
  char tmp[256];
  fc_strcpy(tmp, link);
  // now bring in the bookmarks page
  map_bookmarks();

  int len = fc_strlen(tmp);
  fc_list_push(BOOKMARKS, tmp, len);
}

// dst must not be in top address bank
void FC_SAMS(0,bookmarks_get_prev(char* dst)) {
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
int FC_SAMS(0,bookmarks_get(char* dst, int i)) {
  map_bookmarks();
  struct ListEntry* entry = fc_list_get(BOOKMARKS, i);
  if (entry == 0) {
    *dst = 0;
    return 0;
  }
  fc_strncpy(dst, entry->data, entry->length);
  return entry->length;
}

void FC_SAMS(0,show_bookmarks()) {
  char link[4] = "=> ";
  char nl[2] = "\n";
  char line[256];
  fc_strset(line, 0, 256);
  fc_strcpy(line, "# Bookmarks\n\n");
  page_from_buf(line, 13);
  int list_idx = 0;
  int len = 1;
  while(len > 0) {
    fc_strset(line, 0, 256);
    len = bookmarks_get(line, list_idx++);

    if (len > 0) {
      page_from_buf(link, 3);
      page_from_buf(line, len);
      page_from_buf(nl, 1);
    }
  }
}

