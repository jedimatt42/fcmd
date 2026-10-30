#include <fc_api.h>
#include "fcarc.h"

extern char __BSS_START[];
extern char __BSS_END[];

static void zero_bss(void) {
  char* p = __BSS_START;
  while (p < __BSS_END) {
    *p++ = 0;
  }
}

static int usage(void) {
#ifdef FCARC_CREATE_BUILD
  term_puts("FCARC - Force Command Archive (create)\n\n");
  term_puts("FCARC c <archive> <file>...       create FCARC\n");
  term_puts("FCARC c /r <archive> <dir>...     create with directories\n");
  term_puts("FCARC c /s <archive> <source>...  store (no compression)\n");
  term_puts("FCARC c /ti <archive> <file>...   create TI Archiver archive\n");
  term_puts("FCARC c /ti /u <archive> <file>.. create uncompressed TI archive\n");
#else
  term_puts("FCUNARC - Force Command Archive (extract)\n\n");
  term_puts("FCUNARC l <archive>               list\n");
  term_puts("FCUNARC x <archive> [<path>...]   extract\n");
  term_puts("FCUNARC t <archive>               test integrity\n");
#endif
  return 1;
}

int main(char* args) {
  zero_bss();

  struct SamsInformation si;
  sys_sams_info(&si);
  if (si.total_pages == 0) {
    term_puts("FCARC requires SAMS memory\n");
    return 1;
  }

  char cmd[64];
  char* rest = str_token_next(cmd, args, ' ');
  if (cmd[0] == 0) {
    return usage();
  }

#ifdef FCARC_CREATE_BUILD
  if (str_cmp_icase(cmd, "c") == 0) {
    char* peek = str_token_peek(rest, ' ');
    if (peek != 0 && str_cmp_icase(peek, "/ti") == 0) {
      char opt[16];
      rest = str_token_next(opt, rest, ' ');
      return fc_do_create_ti(rest);
    }
    return fc_do_create(rest);
  }
#else
  if (str_cmp_icase(cmd, "l") == 0) {
    return fc_do_list(rest);
  }
  if (str_cmp_icase(cmd, "x") == 0) {
    return fc_do_extract(rest);
  }
  if (str_cmp_icase(cmd, "t") == 0) {
    return fc_do_test(rest);
  }
#endif

  return usage();
}
