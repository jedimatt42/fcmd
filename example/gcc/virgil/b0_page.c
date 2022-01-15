#include <fc_api.h>
#include "page.h"
#include "gemini.h"
#include "link.h"
#include "readline.h"
#include "screen.h"

#define LINES_PER_BANK 48

struct __attribute__((__packed__)) Bank {
  struct Line lines[LINES_PER_BANK];
};

#define SAMS_ADDR 0xF000
#define PAGE ((struct Bank*) SAMS_ADDR)

struct State state;

static volatile struct Line* last_line;

static int add_bank();

void FC_SAMS(0,init_page()) {
  struct SamsInformation samsInfo;
  fc_sams_info(&samsInfo);
  state.page_count = 0;
  state.max_page = samsInfo.total_pages;
  state.base_id = add_bank();
  state.line_count = 1;
  last_line = page_get_line(1);
}

void FC_SAMS(0,page_clear_lines()) {
  state.base_id = fc_sams_free_pages(state.page_count);
  state.page_count = 0;
  state.line_limit = 0;
  state.line_count = 1;
  state.line_offset = 0;
  state.toggle_literal = 0;
  state.utfstate = 0;
  add_bank();
  last_line = page_get_line(1);
}

static int add_bank() {
  int bank_id = fc_sams_alloc_pages(1);
  state.page_count++;
  fc_sams_map_page(bank_id, SAMS_ADDR);
  state.line_limit += LINES_PER_BANK;
  fc_strset((char*)SAMS_ADDR, 0, 4096);
  return bank_id;
}

static struct Line* page_add_line() {
  state.line_count++;
  if (state.line_count > state.line_limit) {
    add_bank();
  }
  return page_get_line(state.line_count);
}

// 1 based counting system
struct Line* FC_SAMS(0,page_get_line(int idx)) {
  idx--;
  int page_offset = idx / LINES_PER_BANK;
  int line_offset = idx - (page_offset * LINES_PER_BANK);
  int page_id = page_offset + state.base_id;
  fc_sams_map_page(page_id, SAMS_ADDR);
  return &(PAGE->lines[line_offset]);
}

static void update_line_type(volatile struct Line* line) {
  if (line->length >= 3 && line->data[0] == '`' && line->data[1] == '`' && line->data[2] == '`') {
    line->type = LINE_TYPE_TOGGLE;
    return;
  } 
  if (state.toggle_literal) {
    line->type = LINE_TYPE_LITERAL;
    return;
  }
  if (line->length >= 2 && line->data[0] == '=' && line->data[1] == '>') {
    line->type = LINE_TYPE_LINK;
    return;
  } 
  if (line->length >= 1) {
    if (line->data[0] == '#') {
      line->type = LINE_TYPE_HEADING;
    } else if (line->data[0] == '>') {
      line->type = LINE_TYPE_QUOTE;
    } else if (line->data[0] == '*') {
      line->type = LINE_TYPE_BULLET;
    } else if (line->data[0] == '`') {
      line->type = LINE_TYPE_UNKNOWN;
    } else if (line->data[0] == '=') {
      line->type = LINE_TYPE_UNKNOWN;
    } else {
      line->type = LINE_TYPE_NORMAL;
    }
  } else {
    line->type = LINE_TYPE_UNKNOWN;
  }
}

// page_load reads one segment from the socket, and adds it
// to the current line.. or wraps if necessary.

void FC_SAMS(0,page_load()) {
  int len = 0;
  char* buf = readbytes(&len);
  if (len == 0) {
    state.cmd = CMD_IDLE;
    return;
  }
  page_from_buf(buf, len);
}

void FC_SAMS(0,page_from_buf(char* buf, int len)) {
  // ensure the correct page is mapped into ram:
  last_line = page_get_line(state.line_count);

  int i = 0;
  while(i < len) {
    // strip UTF8
    if (state.utfstate) {
      state.utfstate--;
      i++;
      continue;
    }
    if (0x80 & buf[i]) {
      state.utfstate = 1;
      if (0xC0 & buf[i]) {
	state.utfstate++;
      }
      if (0xE0 & buf[i]) {
	state.utfstate++;
      }
      i++;
      continue;
    }

    // if newline, then fill current line with blank, and add a line.
    if (buf[i] == 10 || last_line->length == 80) {
      for(int x = last_line->length; x < 80; x++) {
	last_line->data[x] = 0;
      }
      // erase CR if it was (likely) CRLF
      char* final_char = (char*) (last_line->data + (last_line->length - 1));
      if (*final_char == 13) {
	*final_char = 0;
      }
      // set the line type based on line content
      if (last_line->type == LINE_TYPE_UNKNOWN) {
        update_line_type(last_line);
      }
      int new_line_type = LINE_TYPE_UNKNOWN;

      // if we wrapped, use same linetype as previous
      if (buf[i] != 10) {
	new_line_type = last_line->type;
	if (new_line_type == LINE_TYPE_LINK) {
          new_line_type = LINE_TYPE_LINK_CONT;
	}
      }
      
      // don't add a new line if the line type is toggle...
      if (last_line->type == LINE_TYPE_TOGGLE) {
	last_line->length = 0;
	state.toggle_literal = !state.toggle_literal;
	if (state.toggle_literal) {
	  last_line->type = LINE_TYPE_LITERAL;
	} else {
	  last_line->type = LINE_TYPE_UNKNOWN;
	}
      } else {
        last_line = page_add_line();
      }

      // also if we wrapped, indent based on previous line type
      if (buf[i] != 10 && !state.toggle_literal) {
	last_line->type = new_line_type;
	if (new_line_type == LINE_TYPE_QUOTE) {
	  last_line->data[last_line->length++] = '>';
	  last_line->data[last_line->length++] = ' ';
	} else if (new_line_type == LINE_TYPE_BULLET || 
		   last_line->type == LINE_TYPE_HEADING) {
	  last_line->data[last_line->length++] = ' ';
	  last_line->data[last_line->length++] = ' ';
	} else if (new_line_type == LINE_TYPE_LINK_CONT) {
	  last_line->data[last_line->length++] = '-';
	  last_line->data[last_line->length++] = '>';
	  last_line->data[last_line->length++] = ' ';
	}
      }
    }
    // add the byte to the current line..
    if (buf[i] != 10) {
      last_line->data[last_line->length++] = buf[i];
    }
    i++;
  }
  if (last_line->length > 0) {
    update_line_type(last_line);
  }
  // check for out of memory
  if (state.page_count + state.base_id >= (state.max_page - 1)) {
    state.cmd = CMD_IDLE;
    fc_strcpy(state.error, "Out of memory");
  }
}

