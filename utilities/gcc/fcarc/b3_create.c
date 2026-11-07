#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"
#include "fcarc_lzw_enc.h"
#include "fcarc_file.h"
#include "fcarc_msg.h"

static int add_dir_recursive(struct ArcWriter* w, struct DeviceServiceRoutine* dsr,
                             const char* full_dir, const char* arch_prefix,
                             int* area_pos, int* count, int store, int depth) {
  if (depth > 3 || str_len(full_dir) > 63) {
    term_puts(msg_error);
    term_puts(msg_path_too_long);
    term_puts(full_dir);
    term_putc('\n');
    return 1;
  }
  int skip = 0;
  for (;;) {
    if (catalog_entries(dsr, full_dir, skip)) {
      term_puts(msg_error);
      term_puts(msg_read);
      term_puts(full_dir);
      term_putc('\n');
      return 1;
    }
    struct CatalogEntry entries[CATALOG_MAX];
    int n = g_catalog_count;
    int more = g_catalog_overflow;
    int i;
    for (i = 0; i < n; i++) {
      entries[i] = g_catalog[i];
    }
    for (i = 0; i < n; i++) {
      char full[64];
      char arch[64];
      if (path_append(full, full_dir, entries[i].name, 64)) {
        term_puts(msg_error);
        term_puts(msg_path_too_long);
        return 1;
      }
      if (path_append(arch, arch_prefix, entries[i].name, 64)) {
        term_puts(msg_error);
        term_puts(msg_path_too_long);
        return 1;
      }
      if (entries[i].type == 6) {
        if (create_add_dir(w, arch, area_pos, count)) {
          return 1;
        }
        char subfull[64];
        char subarch[64];
        str_copy(subfull, full);
        str_cat(subfull, msg_dot);
        str_copy(subarch, arch);
        str_cat(subarch, msg_dot);
        if (add_dir_recursive(w, dsr, subfull, subarch, area_pos, count, store, depth + 1)) {
          return 1;
        }
      } else {
        if (create_add_file(w, dsr, full, arch, area_pos, count, store)) {
          return 1;
        }
      }
    }
    if (!more || n == 0) {
      break;
    }
    skip += n;
  }
  return 0;
}

static void delete_partial(struct DeviceServiceRoutine* dsr, const char* path) {
  struct PAB pab;
  str_set((char*)&pab, 0, (int)sizeof(struct PAB));
  pab.pName = (char*)path;
  dsr_delete(dsr, &pab);
}

static void fail_partial(struct ArcWriter* w, struct DeviceServiceRoutine* dsr,
                         const char* path) {
  aw_close(w);
  delete_partial(dsr, path);
}

int FCARC_SAMS(B_CREATE, fc_do_create(char* args)) {
  char tok[128];
  char* rest = args;
  int recursive = 0;
  int store = 0;

  while (rest && rest[0]) {
    char* peek = str_token_peek(rest, ' ');
    if (!peek || peek[0] != '/') {
      break;
    }
    rest = str_token_next(tok, rest, ' ');
    if (str_cmp_icase(tok, opt_r) == 0) {
      recursive = 1;
    } else if (str_cmp_icase(tok, opt_s) == 0) {
      store = 1;
    } else if (str_cmp_icase(tok, opt_q) == 0) {
      g_quiet = 1;
    } else {
      term_puts(msg_error);
      term_puts(msg_unknown_option);
      term_puts(tok);
      term_putc('\n');
      return 1;
    }
  }

  char arc[128];
  if (!rest || rest[0] == 0) {
    term_puts(msg_error);
    term_puts(msg_missing_archive);
    return 1;
  }
  rest = str_token_next(tok, rest, ' ');
  struct DeviceServiceRoutine* dsr = 0;
  path_parse(tok, &dsr, arc, PR_REQUIRED);
  if (dsr == 0) {
    term_puts(msg_error);
    term_puts(msg_not_found);
    term_puts(tok);
    term_putc('\n');
    return 1;
  }
  if (!rest || rest[0] == 0) {
    term_puts(msg_error);
    term_puts(msg_no_sources);
    return 1;
  }

  struct ArcWriter w;
  if (aw_open(&w, dsr, arc, DSR_TYPE_OUTPUT | DSR_TYPE_INTERNAL)) {
    term_puts(msg_error);
    term_puts(msg_create);
    term_puts(arc);
    term_putc('\n');
    return 1;
  }

  unsigned char hdr[FCARC_HEADER_SIZE];
  str_set((char*)hdr, 0, FCARC_HEADER_SIZE);
  int i;
  for (i = 0; i < 8; i++) {
    hdr[i] = fcarc_magic[i];
  }
  put16le(hdr + 8, FCARC_VERSION);
  put16le(hdr + 10, 0x0003);
  if (aw_write(&w, hdr, FCARC_HEADER_SIZE)) {
    fail_partial(&w, dsr, arc);
    return 1;
  }

  int area_pos = 0;
  int count = 0;

  while (rest && rest[0]) {
    char src[128];
    char* next = str_token_next(tok, rest, ' ');
    struct DeviceServiceRoutine* sdsr = 0;
    path_parse(tok, &sdsr, src, PR_REQUIRED);
    if (sdsr == 0) {
      term_puts(msg_error);
      term_puts(msg_not_found);
      term_puts(tok);
      term_putc('\n');
      fail_partial(&w, dsr, arc);
      return 1;
    }
    char catpath[128];
    str_copy(catpath, src);
    int clen = str_len(catpath);
    if (clen == 0 || catpath[clen - 1] != '.') {
      str_cat(catpath, msg_dot);
    }
    int isdir = (catalog_entries(sdsr, catpath, 0) == 0);
    if (isdir) {
      if (!recursive) {
        term_puts(msg_error);
        term_puts(msg_dir_needs_r);
        fail_partial(&w, dsr, arc);
        return 1;
      }
      char comp[16];
      if (path_component(src, comp, 16)) {
        fail_partial(&w, dsr, arc);
        return 1;
      }
      if (create_add_dir(&w, comp, &area_pos, &count)) {
        fail_partial(&w, dsr, arc);
        return 1;
      }
      char full[128];
      str_copy(full, src);
      int len = str_len(full);
      if (len == 0 || full[len - 1] != '.') {
        str_cat(full, msg_dot);
      }
      char prefix[64];
      str_copy(prefix, comp);
      str_cat(prefix, msg_dot);
      if (add_dir_recursive(&w, sdsr, full, prefix, &area_pos, &count, store, 1)) {
        fail_partial(&w, dsr, arc);
        return 1;
      }
    } else {
      char comp[16];
      if (path_component(src, comp, 16)) {
        fail_partial(&w, dsr, arc);
        return 1;
      }
      if (create_add_file(&w, sdsr, src, comp, &area_pos, &count, store)) {
        fail_partial(&w, dsr, arc);
        return 1;
      }
    }
    rest = next;
  }

  unsigned long dir_offset = w.total;
  unsigned int dir_crc = 0xFFFF;
  for (i = 0; i < area_pos; i++) {
    dir_crc = crc16_byte(dir_crc, g_ws.create.area[i]);
  }
  if (aw_write(&w, g_ws.create.area, area_pos) || aw_pad128(&w)) {
    fail_partial(&w, dsr, arc);
    return 1;
  }
  if (((w.total + FCARC_TRAILER_SIZE) & 0xFF) != 0) {
    unsigned char zero[128];
    str_set((char*)zero, 0, 128);
    if (aw_write(&w, zero, 128) || aw_pad128(&w)) {
      fail_partial(&w, dsr, arc);
      return 1;
    }
  }
  unsigned long payload_size = w.total + FCARC_TRAILER_SIZE;
  unsigned char tr[FCARC_TRAILER_SIZE];
  str_set((char*)tr, 0, FCARC_TRAILER_SIZE);
  for (i = 0; i < 8; i++) {
    tr[i] = fcarc_trailer_magic[i];
  }
  put16le(tr + 8, FCARC_VERSION);
  put16le(tr + 10, 0x0003);
  put32le(tr + 12, dir_offset);
  put32le(tr + 16, (unsigned long)area_pos);
  put16le(tr + 20, (unsigned int)count);
  put32le(tr + 24, payload_size);
  put16le(tr + 28, dir_crc);
  if (aw_write(&w, tr, FCARC_TRAILER_SIZE)) {
    fail_partial(&w, dsr, arc);
    return 1;
  }
  if (aw_close(&w)) {
    fail_partial(&w, dsr, arc);
    return 1;
  }

  term_puts(msg_created);
  term_puts(arc);
  term_puts(msg_newline);
  return 0;
}
