#include <fc_api.h>
#include "page.h"
#include "gemini.h"

#define LINES_PER_BANK 40

struct __attribute__((__packed__)) Bank {
  int id;
  struct Line lines[LINES_PER_BANK];
};

#define SAMS_ADDR 0xF000
#define PAGE ((struct Bank*) SAMS_ADDR)

struct State state;


int add_bank();
void next_bank();
void prev_bank();

void init_page() {
  state.base_id = add_bank();
  state.page_id = state.base_id;
}

void page_clear_lines() {
  state.base_id = fc_sams_free_pages(state.page_count);
  state.page_count = 0;
  state.line_limit = 0;
  state.line_count = 0;
  state.page_id = add_bank();
}

void page_add_line(char* line) {
  if (state.line_count == state.line_limit) {
    state.page_id = add_bank();
  }
  int idx = state.line_count - ((state.page_id - state.base_id) * LINES_PER_BANK);

  int c = 0;
  int pc = 0;
  int lastbreak = 0;
  struct Line* pline = &(PAGE->lines[idx]);
  while(line[c] != 0) {
    if (line[c] == ' ' || line[c] == '-') {
      lastbreak = c;
    }
    if (pc > 79) {
      pline->data[pc] = 0;
      if (state.line_count == state.line_limit) {
	state.page_id = add_bank();
      }
      state.line_count++;
      idx = state.line_count - ((state.page_id - state.base_id) * LINES_PER_BANK);
      pline = &(PAGE->lines[idx]);
      pc = 0;
    }
    pline->data[pc++] = line[c++];
  }
  state.line_count++;
}

int add_bank() {
  int bank_id = fc_sams_alloc_pages(1);
  fc_sams_map_page(bank_id, SAMS_ADDR);
  PAGE->id = bank_id;
  fc_strset((char*)PAGE->lines, 0, LINES_PER_BANK * 80);
  state.line_limit += LINES_PER_BANK;
  state.page_count++;
  return bank_id;
}

void next_bank() {
  state.page_id++;
  fc_sams_map_page(state.page_id, SAMS_ADDR);
}

void prev_bank() {
  state.page_id--;
  fc_sams_map_page(state.page_id, SAMS_ADDR);
}

struct Line* page_get_line(int idx) {
  int page_offset = idx / LINES_PER_BANK;
  state.page_id = page_offset + state.base_id;
  fc_sams_map_page(state.page_id, SAMS_ADDR);
  int line_offset = idx - (page_offset * LINES_PER_BANK);
  return &(PAGE->lines[line_offset]);
}

