#include <fc_api.h>
#include <kscan.h>
#include "keyboard.h"
#include "gemini.h"
#include "screen.h"

int read_keyboard() {
  unsigned int key = fc_kscan(5);
  if (KSCAN_STATUS & KSCAN_MASK) {
    return key;
  } else {
    return 0;
  }
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

int handle_keyboard() {
  int redraw = 0;
  int key = read_keyboard();
  switch(key) {
    case 0:
      break;
    case KEY_ESC:
      return 1;
    case KEY_DOWN:
      redraw = on_key_down();
      break;
    case KEY_UP:
      redraw = on_key_up();
      break;
    default:
      break;
  }
  if (redraw) {
    screen_redraw();
  }
  return 0;
}

