#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"
#include "fcarc_file.h"
#include "fcarc_sel.h"
#include "fcarc_msg.h"

struct FcInfo {
  unsigned long size;
  unsigned long dir_offset;
  unsigned long dir_size;
  unsigned int count;
  unsigned int dir_crc;
  unsigned int version;
};

static int magic_eq(const unsigned char* a, const unsigned char* b, int n) {
  int i;
  for (i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}

static int fc_read_trailer(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                           const char* path, struct FcInfo* info) {
  unsigned long size;
  if (fc_file_size(dsr, path, &size)) {
    term_puts(msg_error);
    term_puts(msg_file_size);
    return 1;
  }
  if (size < 256 || (size & 0xFF) != 0) {
    term_puts(msg_error);
    term_puts(msg_truncated);
    return 1;
  }
  unsigned char t[FCARC_TRAILER_SIZE];
  if (ar_seek(r, size - FCARC_TRAILER_SIZE)) {
    term_puts(msg_error);
    term_puts(msg_truncated);
    return 1;
  }
  if (ar_read(r, t, FCARC_TRAILER_SIZE) != FCARC_TRAILER_SIZE) {
    term_puts(msg_error);
    term_puts(msg_truncated);
    return 1;
  }
  if (!magic_eq(t, fcarc_trailer_magic, 8)) {
    term_puts(msg_error);
    term_puts(msg_not_fcarc);
    return 1;
  }
  info->version = le16(t + 8);
  info->dir_offset = le32(t + 12);
  info->dir_size = le32(t + 16);
  info->count = le16(t + 20);
  unsigned long psize = le32(t + 24);
  info->dir_crc = le16(t + 28);
  info->size = size;
  if (info->version != FCARC_VERSION || psize != size) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  unsigned char hv[12];
  if (ar_seek(r, 0) || ar_read(r, hv, 12) != 12
      || !magic_eq(hv, fcarc_magic, 8)
      || le16(hv + 8) != FCARC_VERSION) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  if (info->count > FCARC_MAX_ENTRIES) {
    term_puts(msg_error);
    term_puts(msg_too_many);
    return 1;
  }
  if (info->dir_offset < FCARC_HEADER_SIZE
      || info->dir_offset + info->dir_size > size - FCARC_TRAILER_SIZE) {
    term_puts(msg_error);
    term_puts(msg_bad_format);
    return 1;
  }
  return 0;
}

static int fc_read_entry(struct ArcReader* r, struct ArcEntry* e) {
  unsigned char h[FCARC_ENTRY_FIXED];
  if (ar_read(r, h, FCARC_ENTRY_FIXED) != FCARC_ENTRY_FIXED) {
    return 1;
  }
  int plen = h[0];
  if (plen < 1 || plen > MAX_PATH) {
    return 1;
  }
  e->kind = h[1];
  e->method = h[2];
  e->flags = h[3];
  e->recs = h[4];
  e->eof = h[5];
  e->reclen = h[6];
  e->sectors = ((unsigned int)h[7] << 8) | h[8];
  e->l3 = (unsigned int)h[9] | ((unsigned int)h[10] << 8);
  e->original_size = le32(h + 12);
  e->stored_size = le32(h + 16);
  e->data_offset = le32(h + 20);
  e->crc = le16(h + 24);
  if (ar_read(r, (unsigned char*)e->path, plen) != plen) {
    return 1;
  }
  e->path[plen] = 0;
  if (!path_validate(e->path)) {
    return 1;
  }
  if (e->kind != 0 && e->kind != 1) {
    return 1;
  }
  if (e->method != 0 && e->method != 1) {
    return 1;
  }
  return 0;
}

static int fc_list(struct ArcReader* r, struct DeviceServiceRoutine* dsr,
                   const char* path) {
  struct FcInfo info;
  if (fc_read_trailer(r, dsr, path, &info)) {
    return 1;
  }
  term_puts(msg_fcarc_v1);
  term_puts(str_from_uint(info.count));
  term_puts(msg_entries);
  term_puts(msg_newline);
  if (ar_seek(r, info.dir_offset)) {
    return 1;
  }
  unsigned int i;
  for (i = 0; i < info.count; i++) {
    struct ArcEntry e;
    if (fc_read_entry(r, &e)) {
      term_puts(msg_error);
      term_puts(msg_bad_format);
      return 1;
    }
    if (e.kind == 1) {
      term_puts(msg_dir);
      term_puts(e.path);
    } else {
      term_puts(e.method == 0 ? msg_store : msg_lzw);
      term_puts(str_from_uint(e.sectors));
      term_puts(msg_sectors);
      term_puts(e.path);
    }
    term_putc('\n');
  }
  return 0;
}

static int parse_archive(char* args, char* arcpath,
                         struct DeviceServiceRoutine** dsr, char** rest) {
  char tok[256];
  if (!args || args[0] == 0) {
    term_puts(msg_error);
    term_puts(msg_missing_archive);
    return 1;
  }
  char* next = str_token_next(tok, args, ' ');
  *rest = next;
  path_parse(tok, dsr, arcpath, PR_REQUIRED);
  if (*dsr == 0) {
    term_puts(msg_error);
    term_puts(msg_not_found);
    term_puts(tok);
    term_putc('\n');
    return 1;
  }
  return 0;
}

static void get_curpath(struct DeviceServiceRoutine** dsr, char* curpath) {
  char empty[2];
  empty[0] = 0;
  path_parse(empty, dsr, curpath, PR_OPTIONAL);
  int clen = str_len(curpath);
  if (clen == 0 || curpath[clen - 1] != '.') {
    str_cat(curpath, msg_dot);
  }
}

int FC_SAMS(1, fc_do_list(char* args)) {
  char arc[256];
  char* rest;
  struct DeviceServiceRoutine* dsr;
  if (parse_archive(args, arc, &dsr, &rest)) {
    return 1;
  }
  struct ArcReader r;
  if (ar_open(&r, dsr, arc)) {
    term_puts(msg_error);
    term_puts(msg_open);
    term_puts(arc);
    term_putc('\n');
    return 1;
  }
  unsigned char h[8];
  ar_seek(&r, 0);
  int n = ar_read(&r, h, 8);
  int rc;
  if (n == 8 && magic_eq(h, fcarc_magic, 8)) {
    rc = fc_list(&r, dsr, arc);
  } else {
    ar_seek(&r, 0);
    rc = fc_legacy_list(&r);
  }
  ar_close(&r);
  return rc;
}

int FC_SAMS(1, fc_do_test(char* args)) {
  char arc[256];
  char* rest;
  struct DeviceServiceRoutine* dsr;
  if (parse_archive(args, arc, &dsr, &rest)) {
    return 1;
  }
  struct ArcReader r;
  if (ar_open(&r, dsr, arc)) {
    term_puts(msg_error);
    term_puts(msg_open);
    term_puts(arc);
    term_putc('\n');
    return 1;
  }
  unsigned char h[8];
  ar_seek(&r, 0);
  int n = ar_read(&r, h, 8);
  int rc;
  if (n == 8 && magic_eq(h, fcarc_magic, 8)) {
    rc = fc_fcarc_test(&r, dsr, arc);
  } else {
    ar_seek(&r, 0);
    rc = fc_legacy_test(&r, dsr, arc);
    if (rc == 0) {
      term_puts(msg_ok);
    }
  }
  ar_close(&r);
  return rc;
}

int FC_SAMS(1, fc_do_extract(char* args)) {
  char arc[256];
  char* rest;
  struct DeviceServiceRoutine* dsr;
  if (parse_archive(args, arc, &dsr, &rest)) {
    return 1;
  }
  struct SelSet sel;
  sel_parse(rest, &sel);

  char curpath[256];
  struct DeviceServiceRoutine* odsr;
  get_curpath(&odsr, curpath);
  if (odsr == 0) {
    term_puts(msg_error);
    term_puts(msg_no_device);
    return 1;
  }
  unsigned int oiocode = path_to_iocode(curpath);

  struct ArcReader r;
  if (ar_open(&r, dsr, arc)) {
    term_puts(msg_error);
    term_puts(msg_open);
    term_puts(arc);
    term_putc('\n');
    return 1;
  }
  unsigned char h[8];
  ar_seek(&r, 0);
  int n = ar_read(&r, h, 8);
  int rc;
  if (n == 8 && magic_eq(h, fcarc_magic, 8)) {
    rc = fc_fcarc_extract(&r, dsr, arc, &sel, odsr, oiocode, curpath);
  } else {
    ar_seek(&r, 0);
    rc = fc_legacy_extract(&r, &sel, odsr, oiocode, curpath, 1);
  }
  ar_close(&r);
  return rc;
}
