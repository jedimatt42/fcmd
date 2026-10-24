#include "banks.h"
#define MYBANK BANK(8)

#include "redirect.h"
#include "globals.h"
#include "terminal.h"
#include <conio.h>
#include <vdp.h>

int redirect_flush(int allow_empty);
static void redirect_check_break(void);
static void redirect_advance(unsigned char ch);

static void redirect_check_break(void) {
  if (request_break) {
    return;
  }
  unsigned int k = term_kscan(5);
  if (k == 2 || k == 131) {
    request_break = 1;
  }
}

/*
 * Keep conio_x/conio_y in sync with what the screen would have done, but do
 * not draw or scroll. Commands such as help use wherex() to decide wrapping.
 */
static void redirect_advance(unsigned char ch) {
  int height = (int) displayHeight;
  int last = nTextEnd - nTextRow;

  switch (ch) {
    case '\n':
      conio_x = 0;
      if (conio_y < height - 1) {
        conio_y++;
      }
      break;
    case '\r':
      conio_x = 0;
      break;
    case '\b':
      conio_x--;
      if (conio_x < 0) {
        conio_x = last + 1;
        if (conio_y > 0) {
          conio_y--;
        }
      }
      break;
    case '\t':
      conio_x++;
      while (conio_x & 0x03) {
        conio_x++;
      }
      if (conio_x > last) {
        conio_x = 0;
        if (conio_y < height - 1) {
          conio_y++;
        }
      }
      break;
    case '\v':
      if (conio_y < height - 1) {
        conio_y++;
      }
      break;
    case '\f':
      conio_x = 0;
      conio_y = nTitleLine;
      break;
    default:
      if (conio_x > last) {
        conio_x = 0;
        if (conio_y < height - 1) {
          conio_y++;
        }
      }
      conio_x++;
      if (conio_x > last) {
        conio_x = 0;
        if (conio_y < height - 1) {
          conio_y++;
        }
      }
      break;
  }
}

/*
 * Write the current line buffer. A zero-length record is used for blank lines.
 * Some devices reject empty records; if that happens once, stop trying so the
 * rest of the redirection can still succeed.
 */
int redirect_flush(int allow_empty) {
  int len = redirect_state.linelen;

  if (len == 0 && !allow_empty) {
    return 0;
  }
  if (len == 0 && redirect_state.empty_unsupported) {
    return 0;
  }

  unsigned int err = bk_dsr_write(redirect_state.dsr, &redirect_state.pab,
                                  redirect_state.line, len);
  if (err) {
    if (len == 0) {
      redirect_state.empty_unsupported = 1;
      return 0;
    }
    if (redirect_state.error == 0) {
      redirect_state.error = err;
    }
    redirect_state.active = 0;
    redirect_state.linelen = 0;
    conio_x = redirect_state.saved_x;
    conio_y = redirect_state.saved_y;
    return (int) err;
  }

  redirect_state.linelen = 0;
  return 0;
}

static void redirect_char(unsigned char ch) {
  if (ch == '\n' || ch == '\r') {
    if (ch == '\n' && redirect_state.last_cr) {
      redirect_state.last_cr = 0;
      redirect_advance(ch);
      redirect_check_break();
      return;
    }

    if (redirect_state.linelen > 0) {
      if (redirect_flush(0) != 0) {
        return;
      }
    } else {
      if (redirect_flush(1) != 0) {
        return;
      }
    }

    redirect_state.last_cr = (ch == '\r') ? 1 : 0;
    redirect_advance(ch);
    redirect_check_break();
    return;
  }

  redirect_state.last_cr = 0;

  if (redirect_state.linelen >= REDIR_LINE_MAX) {
    if (redirect_flush(0) != 0) {
      return;
    }
    redirect_check_break();
  }

  redirect_state.line[redirect_state.linelen++] = ch;
  redirect_advance(ch);
}

/*
 * Strip ANSI and legacy terminal escape sequences. The file gets clean text;
 * the screen is not touched while redirection is active.
 */
void redirect_putc(int c) {
  unsigned char ch = (unsigned char) c;

  switch (redirect_state.esc_state) {
    case REDIR_ESC_NONE:
      if (ch == 27) {
        redirect_state.esc_state = REDIR_ESC_START;
        return;
      }
      redirect_char(ch);
      return;

    case REDIR_ESC_START:
      if (ch == '[') {
        redirect_state.esc_state = REDIR_ESC_CSI;
        return;
      }
      if (ch == 'Y') {
        redirect_state.esc_state = REDIR_ESC_Y1;
        return;
      }
      if (ch == '5' || ch == '6') {
        redirect_state.esc_state = REDIR_ESC_N;
        return;
      }
      redirect_state.esc_state = REDIR_ESC_NONE;
      return;

    case REDIR_ESC_CSI:
      /* Parameter and intermediate bytes keep us in the sequence. */
      if (ch < 0x20 || ch > 0x3f) {
        redirect_state.esc_state = REDIR_ESC_NONE;
      }
      return;

    case REDIR_ESC_Y1:
      redirect_state.esc_state = REDIR_ESC_Y2;
      return;

    case REDIR_ESC_Y2:
      redirect_state.esc_state = REDIR_ESC_NONE;
      return;

    case REDIR_ESC_N:
      redirect_state.esc_state = REDIR_ESC_NONE;
      return;
  }
}
