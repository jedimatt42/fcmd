#include "banks.h"
#define MYBANK BANK(1)

#include "b1_history.h"
#include "b0_sams.h"
#include "b1_strutil.h"
#include "b1_list.h"
#include <vdp.h>

#define VDP_REDO_BUFFER 0x3500

void sams_hist_handler(char* buffer, int limit, int op);
void vdp_hist_handler(char* buffer, int limit, int op);

static void (*hist_handler)(char* buffer, int limit, int op);

#define HISTORY ((struct List*) 0xA002)
#define HIST_IDX *((volatile int*) 0xA000)

void history_init() {
  if (sams_total_pages) {
    bk_map_page(1, 0xA000);
    HIST_IDX = 0;
    list_init(HISTORY, (char*)0xA008, (char*)0xBFFF);
    hist_handler = sams_hist_handler;
  } else {
    vdpmemset(VDP_REDO_BUFFER, 0, 256);
    hist_handler = vdp_hist_handler;
  }
}

void history_redo(char* buffer, int limit, int op) {
  hist_handler(buffer, limit, op);
}

void vdp_hist_handler(char* buffer, int limit, int op) {
  if (op == HIST_STORE && bk_strlen(buffer) > 0) {
    vdpmemcpy(VDP_REDO_BUFFER, buffer, 256);
  } else if (op == HIST_GET) {
    strset(buffer, 0, limit);
    vdpmemread(VDP_REDO_BUFFER, buffer, limit);
  } else if (op == HIST_GET_DEC) {
    strset(buffer, 0, limit);
  }
}

void sams_hist_handler(char* buffer, int limit, int op) {
  if (op == HIST_STORE) {
    int len = bk_strlen(buffer);
    if (len > 0) {
      list_push(HISTORY, buffer, len);
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