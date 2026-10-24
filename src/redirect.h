#ifndef _REDIRECT_H
#define _REDIRECT_H 1

#include "dsrutil.h"

#define REDIR_LINE_MAX 80
#define REDIR_PATH_MAX 256

enum {
  REDIR_ESC_NONE = 0,
  REDIR_ESC_START,
  REDIR_ESC_CSI,
  REDIR_ESC_Y1,
  REDIR_ESC_Y2,
  REDIR_ESC_N
};

struct RedirectState {
  int active;              /* terminal sink enabled */
  int open;                /* PAB currently open */
  int append;              /* opened with APPEND */
  int empty_unsupported;   /* target rejected zero-length records */
  unsigned int error;      /* first open/write/close error */
  struct DeviceServiceRoutine* dsr;
  struct PAB pab;
  char path[REDIR_PATH_MAX];
  char line[REDIR_LINE_MAX];
  unsigned char linelen;
  unsigned char last_cr;
  unsigned char esc_state;
  int saved_x;
  int saved_y;
};

extern struct RedirectState redirect_state;

/* bank 5 - called through the trampoline from parsing.c */
int redirect_prepare(char* command);
DECLARE_BANKED(redirect_prepare, BANK(5), int, bk_redirect_prepare,
               (char* command), (command))

int redirect_force(const char* path, int append);
DECLARE_BANKED(redirect_force, BANK(5), int, bk_redirect_force,
               (const char* path, int append), (path, append))

void redirect_unescape(char* command);
DECLARE_BANKED_VOID(redirect_unescape, BANK(5), bk_redirect_unescape,
                    (char* command), (command))

/* bank 8 - used by the bank 9 finish routine */
int redirect_flush(int allow_empty);
DECLARE_BANKED(redirect_flush, BANK(8), int, bk_redirect_flush, (int allow_empty), (allow_empty))

/* bank 9 - called through the trampoline from parsing.c */
int redirect_finish(void);
DECLARE_BANKED(redirect_finish, BANK(9), int, bk_redirect_finish, (), ())

/* bank 8 - called directly from terminal.c */
void redirect_putc(int c);

#endif
