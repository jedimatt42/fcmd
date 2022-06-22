#include "banks.h"
#define MYBANK BANK(1)

#include "b1_history.h"
#include "b0_sams.h"
#include "b1_strutil.h"
#include "b1_list.h"
#include <vdp.h>

#define VDP_REDO_BUFFER 0x3500
#define VDP_BUFFER_LEN 256

void sams_hist_handler(char* buffer, int limit, int op);
void sams_hist_indexed(char* buffer, int limit, int idx);
void vdp_hist_handler(char* buffer, int limit, int op);
void vdp_hist_indexed(char* buffer, int limit, int idx);

int history_on = 0;

static void (*hist_handler)(char* buffer, int limit, int op);

static int hist_page;


void history_init() {
  if (sams_total_pages) {
    hist_page = bk_alloc_pages(2);
    bk_map_page(hist_page, SAMS_HIST_ADDR);
    bk_map_page(hist_page+1, SAMS_HIST_ADDR + 0x1000);
    HIST_IDX = 0;
    list_init(HISTORY, (char*)(SAMS_HIST_ADDR + 8), (char*)(SAMS_HIST_ADDR + 0x1FFF));
    hist_handler = sams_hist_handler;
  } else {
    vdpmemset(VDP_REDO_BUFFER, 0, VDP_BUFFER_LEN);
    hist_handler = vdp_hist_handler;
  }
}

inline static void ensure_sams() {
  bk_map_page(hist_page, SAMS_HIST_ADDR);
  bk_map_page(hist_page + 1, SAMS_HIST_ADDR + 0x1000);
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

void vdp_hist_indexed(char* buffer, int limit, int idx) {
  vdp_hist_handler(buffer, limit, HIST_GET);
}

void sams_hist_indexed(char* buffer, int limit, int idx) {
  ensure_sams();
  HIST_IDX = 0;
  while(idx > 0) {
    sams_hist_handler(buffer, limit, HIST_GET);
    idx--;
  }
}

void history_indexed(char* buffer, int limit, int idx) {
  if (sams_total_pages) {
    sams_hist_indexed(buffer, limit, idx);
  } else {
    vdp_hist_indexed(buffer, limit, idx);
  }
}

void vdp_hist_handler(char* buffer, int limit, int op) {
  if (op == HIST_STORE) {
    int len = bk_strlen(buffer);
    if (len > 0) {
      vdpmemcpy(VDP_REDO_BUFFER, buffer, VDP_BUFFER_LEN);
    }
  } else if (op == HIST_GET) {
    strset(buffer, 0, limit);
    vdpmemread(VDP_REDO_BUFFER, buffer, limit);
  } else if (op == HIST_GET_DEC) {
    strset(buffer, 0, limit);
  }
}

void sams_hist_handler(char* buffer, int limit, int op) {
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
