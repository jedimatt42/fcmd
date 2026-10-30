#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_util.h"
#include "fcarc_path.h"
#include "fcarc_io.h"
#include "fcarc_lzw_enc.h"
#include "fcarc_file.h"
#include "fcarc_msg.h"


static int add_entry(struct ArcWriter* w, const char* path, int kind, int method,
                     const struct FileMeta* m, unsigned long data_offset,
                     unsigned long stored_size, unsigned int crc,
                     int* area_pos, int* count) {
  int plen = str_len(path);
  int scan = 0;
  while (scan < *area_pos) {
    int elen = g_ws.create.area[scan];
    const unsigned char* ep = g_ws.create.area + scan + FCARC_ENTRY_FIXED;
    int minlen = plen < elen ? plen : elen;
    int same = 1;
    int i;
    for (i = 0; i < minlen; i++) {
      if (ci_char(ep[i]) != ci_char((unsigned char)path[i])) {
        same = 0;
        break;
      }
    }
    if (same) {
      int conflict = 0;
      if (plen == elen) {
        conflict = 1;
      } else if (plen > elen && path[elen] == '.') {
        if (g_ws.create.area[scan + 1] == 0) {
          conflict = 1;
        }
      } else if (elen > plen && ep[plen] == '.') {
        if (kind == 0) {
          conflict = 1;
        }
      }
      if (conflict) {
        term_puts(msg_error);
        term_puts(msg_exists);
        term_puts(path);
        term_putc('\n');
        return 1;
      }
    }
    scan += FCARC_ENTRY_FIXED + elen;
  }
  if (plen < 1 || plen > MAX_PATH || !path_validate(path)) {
    term_puts(msg_error);
    term_puts(msg_name_invalid);
    term_puts(path);
    term_putc('\n');
    return 1;
  }
  if (*count >= FCARC_MAX_ENTRIES) {
    term_puts(msg_error);
    term_puts(msg_too_many);
    return 1;
  }
  if (*area_pos + FCARC_ENTRY_FIXED + plen > CREATE_AREA_BYTES) {
    term_puts(msg_error);
    term_puts(msg_no_space);
    return 1;
  }
  unsigned char* p = g_ws.create.area + *area_pos;
  p[0] = (unsigned char)plen;
  p[1] = (unsigned char)kind;
  p[2] = (unsigned char)method;
  p[3] = m->flags;
  p[4] = m->recs;
  p[5] = m->eof;
  p[6] = m->reclen;
  /* sectors: original_size / 256, 16-bit */
  unsigned int sectors = (unsigned int)(m->original_size >> 8);
  p[7] = (unsigned char)((sectors >> 8) & 0xFF);
  p[8] = (unsigned char)(sectors & 0xFF);
  p[9] = (unsigned char)(m->l3 & 0xFF);
  p[10] = (unsigned char)((m->l3 >> 8) & 0xFF);
  p[11] = 0;
  put32le(p + 12, m->original_size);
  put32le(p + 16, stored_size);
  put32le(p + 20, data_offset);
  put16le(p + 24, crc);
  int i;
  for (i = 0; i < plen; i++) {
    p[FCARC_ENTRY_FIXED + i] = (unsigned char)path[i];
  }
  *area_pos += FCARC_ENTRY_FIXED + plen;
  (*count)++;
  return 0;
}

static int add_file(struct ArcWriter* w, struct DeviceServiceRoutine* dsr,
                    const char* fullpath, const char* archpath,
                    int* area_pos, int* count, int store) {
  char* parent = g_ws.scratch.parent;
  char* name = g_ws.scratch.name;
  path_split(fullpath, parent, name);

  unsigned int iocode = path_to_iocode(fullpath);
  unsigned int err = lvl2_setdir(dsr->crubase, iocode, parent);
  if (err) {
    term_puts(msg_error);
    term_puts(msg_read);
    term_puts(fullpath);
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
  err = lvl2_input(dsr->crubase, iocode, bare, 0, ADDINFO);
  if (err) {
    term_puts(msg_error);
    term_puts(msg_read);
    term_puts(fullpath);
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
  unsigned int blocks = ADDINFO->first_sector;
  m.original_size = (unsigned long)blocks * SECTOR_SIZE;
  m.crc = 0xFFFF;

  if (aw_pad128(w)) {
    return 1;
  }
  unsigned long data_offset = w->total;

  int method = 0;
  unsigned int crc = 0xFFFF;

  if (!store && blocks > 0) {
    method = 1;
    struct LzwEnc enc;
    struct LzwStream st;
    enc_init(&enc, w, &st);
    unsigned int block = 0;
    while (block < blocks) {
      unsigned int n = 1;
      ADDINFO->first_sector = block;
      SET_ADDINFO_BUFFER(g_filebuf);
      unsigned int e = lvl2_input_cpu(dsr->crubase, iocode, bare, n, ADDINFO);
      if (e) {
        term_puts(msg_error);
        term_puts(msg_read);
        term_puts(fullpath);
        term_putc('\n');
        return 1;
      }
      int bytes = (int)n * SECTOR_SIZE;
      int i;
      for (i = 0; i < bytes; i++) {
        crc = crc16_byte(crc, g_filebuf[i]);
      }
      enc_bytes(&enc, w, &st, g_filebuf, bytes);
      block += n;
    }
    enc_finish(&enc, w, &st);
  } else {
    method = 0;
    unsigned int block = 0;
    while (block < blocks) {
      unsigned int n = 1;
      ADDINFO->first_sector = block;
      SET_ADDINFO_BUFFER(g_filebuf);
      unsigned int e = lvl2_input_cpu(dsr->crubase, iocode, bare, n, ADDINFO);
      if (e) {
        term_puts(msg_error);
        term_puts(msg_read);
        term_puts(fullpath);
        term_putc('\n');
        return 1;
      }
      int bytes = (int)n * SECTOR_SIZE;
      int i;
      for (i = 0; i < bytes; i++) {
        crc = crc16_byte(crc, g_filebuf[i]);
      }
      if (aw_write(w, g_filebuf, bytes)) {
        return 1;
      }
      block += n;
    }
  }

  unsigned long stored_size = w->total - data_offset;
  m.crc = crc;
  if (aw_pad128(w)) {
    return 1;
  }
  if (!g_quiet) {
    term_puts(msg_creating);
    term_puts(archpath);
    term_putc('\n');
  }
  return add_entry(w, archpath, 0, method, &m, data_offset, stored_size, crc,
                   area_pos, count);
}

static int add_dir(struct ArcWriter* w, const char* archpath,
                   int* area_pos, int* count) {
  struct FileMeta m;
  m.flags = 0;
  m.recs = 0;
  m.eof = 0;
  m.reclen = 0;
  m.l3 = 0;
  m.original_size = 0;
  m.crc = 0;
  return add_entry(w, archpath, 1, 0, &m, 0, 0, 0, area_pos, count);
}


int FCARC_SAMS(B_WRITER, create_add_dir(struct ArcWriter* w, const char* archpath,
                              int* area_pos, int* count)) {
  return add_dir(w, archpath, area_pos, count);
}

int FCARC_SAMS(B_WRITER, create_add_file(struct ArcWriter* w, struct DeviceServiceRoutine* dsr,
                               const char* fullpath, const char* archpath,
                               int* area_pos, int* count, int store)) {
  return add_file(w, dsr, fullpath, archpath, area_pos, count, store);
}
