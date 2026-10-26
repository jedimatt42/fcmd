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

void catalog_vol(struct VolInfo* v) {
  (void)v;
}

void catalog_dir(struct DirEntry* d) {
  if (g_catalog_seen++ < g_catalog_skip) {
    return;
  }
  if (g_catalog_count >= CATALOG_MAX) {
    g_catalog_overflow = 1;
    return;
  }
  str_ncopy(g_catalog[g_catalog_count].name, d->name, 10);
  g_catalog[g_catalog_count].name[10] = 0;
  int last = 9;
  while (last >= 0 && g_catalog[g_catalog_count].name[last] == ' ') {
    g_catalog[g_catalog_count].name[last] = 0;
    last--;
  }
  int t = d->type;
  if (t < 0) {
    t = -t;
  }
  g_catalog[g_catalog_count].type = (unsigned char)t;
  g_catalog[g_catalog_count].reclen = (unsigned char)d->reclen;
  g_catalog[g_catalog_count].sectors = (unsigned int)d->sectors;
  g_catalog_count++;
}

static int usage(void) {
  term_puts("FCARC - Force Command Archive\n\n");
  term_puts("FCARC c <archive> <file>...       create FCARC\n");
  term_puts("FCARC c /r <archive> <dir>...     create with directories\n");
  term_puts("FCARC c /s <archive> <source>...  store (no compression)\n");
  term_puts("FCARC c /ti <archive> <file>...   create TI Archiver archive\n");
  term_puts("FCARC c /ti /u <archive> <file>.. create uncompressed TI archive\n");
  term_puts("FCARC l <archive>                 list\n");
  term_puts("FCARC x <archive> [<path>...]     extract\n");
  term_puts("FCARC t <archive>                 test integrity\n");
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

  if (str_cmp_icase(cmd, "l") == 0) {
    return fc_do_list(rest);
  }
  if (str_cmp_icase(cmd, "x") == 0) {
    return fc_do_extract(rest);
  }
  if (str_cmp_icase(cmd, "t") == 0) {
    return fc_do_test(rest);
  }
  if (str_cmp_icase(cmd, "c") == 0) {
    char* peek = str_token_peek(rest, ' ');
    if (peek != 0 && str_cmp_icase(peek, "/ti") == 0) {
      char opt[16];
      rest = str_token_next(opt, rest, ' ');
      return fc_do_create_ti(rest);
    }
    return fc_do_create(rest);
  }

  return usage();
}
