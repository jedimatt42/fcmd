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
    // The editor can hand us a line that is not NUL terminated (an 80 column
    // line fills the whole Line.data buffer). Copy through a terminated
    // scratch buffer and honor the caller's length limit.
    if (limit > 0) {
      char copy[256];
      int max = limit > (int)sizeof(copy) - 1 ? (int)sizeof(copy) - 1 : limit;
      bk_strncpy(copy, buffer, max);
      int len = bk_strlen(copy);
      if (len > 0) {
        list_push(HISTORY, copy, len + 1 /* include the trailing null */);
      }
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

void history_indexed(char* buffer, int limit, int idx) {
  ensure_sams();
  HIST_IDX = 0;
  if (idx <= 0) {
    bk_strset(buffer, 0, limit);
    return;
  }
  while (idx > 0) {
    hist_handler(buffer, limit, HIST_GET);
    idx--;
  }
}
