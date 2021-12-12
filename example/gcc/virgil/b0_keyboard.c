#include <fc_api.h>
#include <kscan.h>
#include "keyboard.h"
#include "gemini.h"
#include "screen.h"
#include "history.h"
#include "link.h"

int FC_SAMS(0,read_keyboard()) {
  unsigned int key = fc_kscan(5);
  return key;
  /*
  if (KSCAN_STATUS & KSCAN_MASK) {
    return key;
  } else {
    return 0;
  }
  */
}

int FC_SAMS(0,on_key_down()) {
  if (state.line_count - state.line_offset > 28) {
    state.line_offset++;
    return 1;
  }
  return 0;
}

int FC_SAMS(0,on_key_up()) {
  if (state.line_offset > 0) {
    state.line_offset--;
    return 1;
  }
  return 0;
}

int FC_SAMS(0,on_page_down()) {
  if (state.line_count - state.line_offset > 28) {
    state.line_offset += 28;
    if (state.line_count - state.line_offset < 28) {
      state.line_offset = state.line_count - 28;
    }
    return 1;
  }
  return 0;
}

int FC_SAMS(0,on_page_up()) {
  if (state.line_offset > 0) {
    state.line_offset -= 28;
    if (state.line_offset < 0) {
      state.line_offset = 0;
    }
    return 1;
  }
  return 0;
}

int on_stop() {
  if (state.cmd == CMD_READPAGE) {
    state.cmd = CMD_STOP;
    return 1;
  } else {
    return 0;
  }
}

void FC_SAMS(0,on_back()) {
  char tmp[80];
  history_get_prev(tmp);
  fc_strcpy(state.newurl, tmp);
  state.cmd = CMD_RELOAD_NOHIST;
}

void FC_SAMS(0,on_address()) {
  char tmp[80];
  fc_strset(tmp, 0, 80);
  fc_strcpy(tmp, state.url);
  screen_prompt(tmp, "Address:");
  if (tmp[0] != 0) {
    if (0 != fc_strcmp(tmp, state.url)) {
      fc_strcpy(state.newurl, tmp);
      state.cmd = CMD_RELOAD;
    }
  }
}

void FC_SAMS(0,handle_keyboard()) {
  int redraw = 0;
  int key = read_keyboard();
  switch(key) {
    case 0:
      break;
    case KEY_ESC:
      state.cmd = CMD_QUIT;
      break;
    case KEY_DOWN:
      redraw = on_key_down();
      break;
    case KEY_UP:
      redraw = on_key_up();
      break;
    case KEY_CTRL_X:
      redraw = on_page_down();
      break;
    case KEY_CTRL_E:
      redraw = on_page_up();
      break;
    case KEY_CTRL_S:
      redraw = on_stop();
      break;
    case KEY_LEFT:
      on_back();
      break;
    case KEY_CTRL_A:
      on_address();
      break;
    default:
      break;
  }
  if (redraw) {
    screen_redraw();
  }
}

