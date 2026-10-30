#include <fc_api.h>
#include "fcarc.h"

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

/* Cataloging must run from bank 0.  dsr_catalog() invokes its callbacks
 * with the executable's bank 0 mapped, so the callback addresses below are
 * only valid while bank 0 is the current SAMS code bank. */
int FCARC_SAMS(B_CATALOG, catalog_entries(struct DeviceServiceRoutine* dsr, const char* path,
                               int skip)) {
  g_catalog_count = 0;
  g_catalog_overflow = 0;
  g_catalog_skip = skip;
  g_catalog_seen = 0;
  unsigned int err = dsr_catalog(dsr, path, catalog_vol, catalog_dir);
  if (err) {
    return 1;
  }
  return 0;
}

