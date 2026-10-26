#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"
#include "fcarc_lzw_enc.h"
#include "fcarc_file.h"
#include "fcarc_msg.h"

#ifdef HOST_TEST
unsigned char host_inbuf[1024];
#define inbuf host_inbuf
#endif


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

static int legacy_name_ok(const char* name) {
  int len = str_len(name);
  if (len < 1 || len > 10) {
    return 0;
  }
  int i;
  for (i = 0; i < len; i++) {
    char c = name[i];
    if (c == ' ' || c == '.' || c == '/' || c == '\\' || c < 32) {
      return 0;
    }
  }
  return 1;
}

static int add_legacy_entry(unsigned char* area, int* pos, int* count,
                            const char* name, const struct FileMeta* m,
                            unsigned int sectors) {
  if (*count >= LEGACY_MAX_ENTRIES) {
    term_puts(msg_error);
    term_puts(msg_too_many);
    return 1;
  }
  if (!legacy_name_ok(name)) {
    term_puts(msg_error);
    term_puts(msg_name_invalid);
    term_puts(name);
    term_putc('\n');
    return 1;
  }
  if ((*pos & 0xFF) == 0xFC) {
    if (*pos + 4 > CREATE_AREA_BYTES) {
      return 1;
    }
    area[(*pos)++] = 0;
    area[(*pos)++] = 0;
    area[(*pos)++] = 0;
    area[(*pos)++] = 0;
  }
  if (*pos + LEGACY_ENTRY_SIZE > CREATE_AREA_BYTES) {
    term_puts(msg_error);
    term_puts(msg_no_space);
    return 1;
  }
  int i;
  for (i = 0; i < 10; i++) {
    area[*pos + i] = (unsigned char)(i < str_len((char*)name) ? name[i] : ' ');
  }
  area[*pos + 10] = m->flags;
  area[*pos + 11] = m->recs;
  area[*pos + 12] = (unsigned char)((sectors >> 8) & 0xFF);
  area[*pos + 13] = (unsigned char)(sectors & 0xFF);
  area[*pos + 14] = m->eof;
  area[*pos + 15] = m->reclen;
  area[*pos + 16] = (unsigned char)(m->l3 & 0xFF);
  area[*pos + 17] = (unsigned char)((m->l3 >> 8) & 0xFF);
  *pos += LEGACY_ENTRY_SIZE;
  (*count)++;
  return 0;
}

static int finish_legacy_dir(unsigned char* area, int* pos) {
  while ((*pos & 0xFF) != 0xFC) {
    if (*pos >= CREATE_AREA_BYTES) {
      return 1;
    }
    area[(*pos)++] = 0;
  }
  if (*pos + 4 > CREATE_AREA_BYTES) {
    return 1;
  }
  area[(*pos)++] = 'E';
  area[(*pos)++] = 'N';
  area[(*pos)++] = 'D';
  area[(*pos)++] = '!';
  return 0;
}

int FC_SAMS(4, fc_do_create_ti(char* args)) {
  char tok[256];
  char* rest = args;
  int uncompressed = 0;

  while (rest && rest[0]) {
    char* peek = str_token_peek(rest, ' ');
    if (!peek || peek[0] != '/') {
      break;
    }
    rest = str_token_next(tok, rest, ' ');
    if (str_cmp_icase(tok, opt_u) == 0) {
      uncompressed = 1;
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

  if (!rest || rest[0] == 0) {
    term_puts(msg_error);
    term_puts(msg_missing_archive);
    return 1;
  }
  rest = str_token_next(tok, rest, ' ');
  char arc[256];
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
  char* sources = rest;

  /* First pass: build the legacy directory. */
  unsigned char* area = g_ws.create.area;
  int pos = 0;
  int count = 0;
  char* scan = sources;
  while (scan && scan[0]) {
    char* next = str_token_next(tok, scan, ' ');
    char src[256];
    struct DeviceServiceRoutine* sdsr = 0;
    path_parse(tok, &sdsr, src, PR_REQUIRED);
    if (sdsr == 0) {
      term_puts(msg_error);
      term_puts(msg_not_found);
      term_puts(tok);
      term_putc('\n');
      return 1;
    }
    if (dir_exists(sdsr, src)) {
      term_puts(msg_error);
      term_puts(msg_ti_no_dir);
      return 1;
    }
    char parent[256];
    char name[16];
    path_split(src, parent, name);
    unsigned int iocode = path_to_iocode(src);
    unsigned int err = lvl2_setdir(sdsr->crubase, iocode, parent);
    if (err) {
      term_puts(msg_error);
      term_puts(msg_read);
      term_puts(src);
      term_putc('\n');
      return 1;
    }
    char bare[16];
    str_ncopy(bare, name, 10);
    bare[10] = 0;
    ADDINFO->first_sector = 0;
    ADDINFO->buffer = 0;
    ADDINFO->flags = 0;
    ADDINFO->recs_per_sec = 0;
    ADDINFO->eof_offset = 0;
    ADDINFO->rec_length = 0;
    ADDINFO->records = 0;
    err = lvl2_input(sdsr->crubase, iocode, bare, 0, ADDINFO);
    if (err) {
      term_puts(msg_error);
      term_puts(msg_read);
      term_puts(src);
      term_putc('\n');
      return 1;
    }
    struct FileMeta m;
    m.flags = ADDINFO->flags;
    m.recs = ADDINFO->recs_per_sec;
    m.eof = ADDINFO->eof_offset;
    m.reclen = ADDINFO->rec_length;
    unsigned char* lp = (unsigned char*)&ADDINFO->records;
    m.l3 = (unsigned int)lp[0] | ((unsigned int)lp[1] << 8);
    m.original_size = 0;
    m.crc = 0;
    if (add_legacy_entry(area, &pos, &count, name, &m,
                         (unsigned int)ADDINFO->first_sector)) {
      return 1;
    }
    scan = next;
  }

  if (finish_legacy_dir(area, &pos)) {
    term_puts(msg_error);
    term_puts(msg_no_space);
    return 1;
  }
  int dir_size = pos;

  struct ArcWriter w;
  int flags = DSR_TYPE_OUTPUT;
  if (!uncompressed) {
    flags |= DSR_TYPE_INTERNAL;
  }
  if (aw_open(&w, dsr, arc, flags)) {
    term_puts(msg_error);
    term_puts(msg_create);
    term_puts(arc);
    term_putc('\n');
    return 1;
  }

  struct LzwEnc enc;
  struct LzwStream st;
  if (uncompressed) {
    if (aw_write(&w, area, dir_size)) {
      fail_partial(&w, dsr, arc);
      return 1;
    }
  } else {
    enc_init(&enc, &w, &st);
    enc_bytes(&enc, &w, &st, area, dir_size);
  }

  /* Second pass: stream the file data in the same order. */
  scan = sources;
#ifndef HOST_TEST
  unsigned char inbuf[1024];
#endif
  while (scan && scan[0]) {
    char* next = str_token_next(tok, scan, ' ');
    char src[256];
    struct DeviceServiceRoutine* sdsr = 0;
    path_parse(tok, &sdsr, src, PR_REQUIRED);
    char parent[256];
    char name[16];
    path_split(src, parent, name);
    unsigned int iocode = path_to_iocode(src);
    unsigned int err = lvl2_setdir(sdsr->crubase, iocode, parent);
    if (err) {
      fail_partial(&w, dsr, arc);
      return 1;
    }
    char bare[16];
    str_ncopy(bare, name, 10);
    bare[10] = 0;
    ADDINFO->first_sector = 0;
    ADDINFO->buffer = 0;
    ADDINFO->flags = 0;
    ADDINFO->recs_per_sec = 0;
    ADDINFO->eof_offset = 0;
    ADDINFO->rec_length = 0;
    ADDINFO->records = 0;
    err = lvl2_input(sdsr->crubase, iocode, bare, 0, ADDINFO);
    if (err) {
      fail_partial(&w, dsr, arc);
      return 1;
    }
    unsigned int blocks = ADDINFO->first_sector;
    unsigned int block = 0;
    while (block < blocks) {
      unsigned int n = blocks - block;
      if (n > 4) {
        n = 4;
      }
      ADDINFO->first_sector = block;
      SET_ADDINFO_BUFFER(inbuf);
      err = lvl2_input_cpu(sdsr->crubase, iocode, bare, n, ADDINFO);
      if (err) {
        fail_partial(&w, dsr, arc);
        return 1;
      }
      int bytes = (int)n * SECTOR_SIZE;
      if (uncompressed) {
        if (aw_write(&w, inbuf, bytes)) {
          fail_partial(&w, dsr, arc);
          return 1;
        }
      } else {
        enc_bytes(&enc, &w, &st, inbuf, bytes);
      }
      block += n;
    }
    if (!g_quiet) {
      term_puts(msg_creating);
      term_puts(name);
      term_putc('\n');
    }
    scan = next;
  }

  if (!uncompressed) {
    enc_finish(&enc, &w, &st);
  }
  while ((w.total & 0xFF) != 0) {
    if (aw_putc(&w, 0)) {
      fail_partial(&w, dsr, arc);
      return 1;
    }
  }
  if (aw_close(&w)) {
    delete_partial(dsr, arc);
    return 1;
  }

  if (uncompressed) {
    /* Archiver writes uncompressed archives as protected DIS/FIX 128 files. */
    char parent[256];
    char name[16];
    path_split(arc, parent, name);
    unsigned int iocode = path_to_iocode(arc);
    if (lvl2_setdir(dsr->crubase, iocode, parent) == 0) {
      char bare[16];
      str_ncopy(bare, name, 10);
      bare[10] = 0;
      lvl2_protect(dsr->crubase, iocode, bare, 1);
    }
  }

  term_puts(msg_created);
  term_puts(arc);
  term_puts(msg_newline);
  return 0;
}
