#include <fc_api.h>
#include <kscan.h>
#include "keyboard.h"
#include "gemini.h"
#include "screen.h"

int read_keyboard() {
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

int on_key_down() {
  if (state.line_count - state.line_offset > 28) {
    state.line_offset++;
    return 1;
  }
  return 0;
}

int on_key_up() {
  if (state.line_offset > 0) {
    state.line_offset--;
    return 1;
  }
  return 0;
}

int on_page_down() {
  if (state.line_count - state.line_offset > 28) {
    state.line_offset += 28;
    if (state.line_count - state.line_offset < 28) {
      state.line_offset = state.line_count - 28;
    }
    return 1;
  }
  return 0;
}

int on_page_up() {
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
  if (state.loading) {
    state.stop = 1;
    return 1;
  } else {
    return 0;
  }
}

void handle_keyboard() {
  int redraw = 0;
  int key = read_keyboard();
  switch(key) {
    case 0:
      break;
    case KEY_ESC:
      state.quit = 1;
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
    default:
      break;
  }
  if (redraw) {
    screen_redraw();
  }
}

