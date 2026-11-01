#include "banks.h"
#define MYBANK BANK(1)

#include "history.h"
#include "sams.h"
#include "strutil.h"
#include "list.h"

int history_on = 0;

static int hist_page;

static void ensure_sams() {
  bk_map_page(hist_page, SAMS_HIST_ADDR);
  bk_map_page(hist_page + 1, SAMS_HIST_ADDR + 0x1000);
}

static void hist_handler(char* buffer, int limit, int op) {
  ensure_sams();
  if (op == HIST_STORE) {
    int len = bk_strlen(buffer);
    if (len > 0) {
      list_push(HISTORY, buffer, len + 1 /* include the trailing null */);
    }
    HIST_IDX = 0;
  } else if (op == HIST_GET) {
    bk_strset(buffer, 0, limit);
    HIST_IDX = HIST_IDX + 1;
    struct ListEntry* entry = list_get(HISTORY, HIST_IDX - 1);
    if (entry) {
      bk_strncpy(buffer, entry->data, limit);
    } else {
      HIST_IDX = HIST_IDX - 1;
    }
  } else if (op == HIST_GET_DEC) {
    bk_strset(buffer, 0, limit);
    HIST_IDX = HIST_IDX - 1;
    if (HIST_IDX > 0) {
      struct ListEntry* entry = list_get(HISTORY, HIST_IDX - 1);
      if (entry) {
        bk_strncpy(buffer, entry->data, limit);
      } else {
        HIST_IDX = 0;
      }
    } else {
      HIST_IDX = 0;
    }
  }
}

void history_init() {
  hist_page = bk_alloc_pages(2);
  bk_map_page(hist_page, SAMS_HIST_ADDR);
  bk_map_page(hist_page + 1, SAMS_HIST_ADDR + 0x1000);
  HIST_IDX = 0;
  list_init(HISTORY, (char*)(SAMS_HIST_ADDR + 8), (char*)(SAMS_HIST_ADDR + 0x1FFF));
}

void history_redo(char* buffer, int limit, int op) {
  if (op == HIST_STORE) {
    // never store the history command itself.
    if (!bk_strcmpi(str2ram("history"), buffer)) {
      return;
    }
    // never store history references
    if (buffer[0] == '!') {
      return;
    }
  }
  hist_handler(buffer, limit, op);
}
