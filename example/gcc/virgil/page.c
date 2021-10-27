#include <fc_api.h>
#include "page.h"
#include "gemini.h"
#include "link.h"

#define LINES_PER_BANK 48

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
void fixup_link(struct Line* line);

void init_page() {
  state.base_id = add_bank();
  state.page_id = state.base_id;
}

void page_clear_lines() {
  state.base_id = fc_sams_free_pages(state.page_count);
  state.page_count = 0;
  state.line_limit = 0;
  state.line_count = 0;
  state.line_offset = 0;
  state.toggle_literal = 0;
  state.page_id = add_bank();
}

int line_type(char* line) {
  if (line[0] == '#') {
    return LINE_TYPE_HEADING;
  } else if (line[0] == '>') {
    return LINE_TYPE_QUOTE;
  } else if (line[0] == '=' && line[1] == '>') {
    return LINE_TYPE_LINK;
  } else if (line[0] == '`' && line[1] == '`' && line[2] == '`') {
    return LINE_TYPE_TOGGLE;
  } else {
    return LINE_TYPE_NORMAL;
  }
}

void page_add_line(char* line) {
  fc_sams_map_page(state.page_id, SAMS_ADDR);
  int type = line_type(line);
  if (type == LINE_TYPE_TOGGLE) {
    state.toggle_literal = !state.toggle_literal;
    return; // don't actually add these "```" as lines
  }

  if (state.toggle_literal) {
    // override the line type if literal is true
    type = LINE_TYPE_LITERAL;
  }

  if (state.line_count == state.line_limit) {
    state.page_id = add_bank();
  }
  int idx = state.line_count - ((state.page_id - state.base_id) * LINES_PER_BANK);

  int c = 0;
  int pc = 0;
  int lastbreak = 0;
  struct Line* pline = &(PAGE->lines[idx]);
  PAGE->lines[idx].type = type;
  while(line[c] != 0) {
    if (line[c] == ' ' || line[c] == '-') {
      lastbreak = c;
    }
    if (pc > 79) {
      pline->length = pc + ((line[lastbreak] == '-') ? 1 : 0) - (c - lastbreak);
      pline->data[pline->length] = 0;
      if (state.line_count == state.line_limit) {
	state.page_id = add_bank();
      }
      state.line_count++;
      idx = state.line_count - ((state.page_id - state.base_id) * LINES_PER_BANK);
      pline = &(PAGE->lines[idx]);
      PAGE->lines[idx].type = type;
      pc = 0;
      if (type == LINE_TYPE_QUOTE) {
	pline->data[0] = '>';
	pline->data[1] = ' ';
	pc += 2;
      }
      c = lastbreak + 1;
    }
    pline->data[pc++] = line[c++];
  }
  pline->length = pc - 1;
  if (pline->data[pline->length - 1] == 0x0D) {
    pline->length--;
    pline->data[pline->length] = 0;
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

