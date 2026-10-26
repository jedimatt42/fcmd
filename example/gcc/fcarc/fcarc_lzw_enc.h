#ifndef _FCARC_LZW_ENC_H
#define _FCARC_LZW_ENC_H 1

#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_io.h"

struct LzwEnc {
  int width;
  int next_code;
  int count;
  unsigned int bitbuf;
  int bitcount;
};

struct LzwStream {
  int w;
  int have_w;
};

static inline void enc_putbits(struct LzwEnc* e, struct ArcWriter* w,
                               int value, int width) {
  int i;
  for (i = width - 1; i >= 0; i--) {
    e->bitbuf = (unsigned int)((e->bitbuf << 1) | ((value >> i) & 1));
    e->bitcount++;
    if (e->bitcount == 8) {
      aw_putc(w, (int)e->bitbuf);
      e->bitbuf = 0;
      e->bitcount = 0;
    }
  }
}

static inline void enc_flushbits(struct LzwEnc* e, struct ArcWriter* w) {
  if (e->bitcount > 0) {
    e->bitbuf = (unsigned int)(e->bitbuf << (8 - e->bitcount));
    aw_putc(w, (int)e->bitbuf);
    e->bitcount = 0;
  }
  e->bitbuf = 0;
}

static inline void enc_reset_state(struct LzwEnc* e) {
  memset(g_ws.create.hash, 0, sizeof(g_ws.create.hash));
  e->width = 9;
  e->next_code = 258;
  e->count = 0;
}

static inline void enc_init(struct LzwEnc* e, struct ArcWriter* w,
                            struct LzwStream* s) {
  e->bitbuf = 0;
  e->bitcount = 0;
  enc_reset_state(e);
  s->w = 0;
  s->have_w = 0;
  enc_putbits(e, w, 256, 9);
}

static inline void enc_clear(struct LzwEnc* e, struct ArcWriter* w) {
  enc_putbits(e, w, 256, e->width);
  enc_reset_state(e);
}

static inline int enc_find(struct LzwEnc* e, int prefix, int ch) {
  unsigned int idx = (((unsigned int)prefix * 17u) + ((unsigned int)ch * 31u)) & (ENC_HASH_SLOTS - 1);
  unsigned int start = idx;
  unsigned long key = ((unsigned long)prefix << 8) | (unsigned int)ch;
  do {
    unsigned long v = g_ws.create.hash[idx];
    if (v == 0) {
      return -1;
    }
    if ((v >> 12) == key) {
      return (int)(v & 0x0FFF);
    }
    idx = (idx + 1) & (ENC_HASH_SLOTS - 1);
  } while (idx != start);
  return -1;
}

static inline int enc_insert(struct LzwEnc* e, int prefix, int ch, int code) {
  if (e->count >= (ENC_HASH_SLOTS - 128)) {
    return 0;
  }
  unsigned int idx = (((unsigned int)prefix * 17u) + ((unsigned int)ch * 31u)) & (ENC_HASH_SLOTS - 1);
  unsigned int start = idx;
  unsigned long key = ((unsigned long)prefix << 8) | (unsigned int)ch;
  do {
    if (g_ws.create.hash[idx] == 0) {
      g_ws.create.hash[idx] = (key << 12) | (unsigned long)(code & 0x0FFF);
      e->count++;
      return 1;
    }
    idx = (idx + 1) & (ENC_HASH_SLOTS - 1);
  } while (idx != start);
  return 0;
}

/* Feed bytes to the encoder. The stream keeps the current partial match
 * across calls. */
static inline void enc_bytes(struct LzwEnc* e, struct ArcWriter* w,
                             struct LzwStream* s,
                             const unsigned char* data, int len) {
  int i;
  for (i = 0; i < len; i++) {
    int c = data[i];
    if (!s->have_w) {
      s->w = c;
      s->have_w = 1;
      continue;
    }
    int code = enc_find(e, s->w, c);
    if (code >= 0) {
      s->w = code;
      continue;
    }
    enc_putbits(e, w, s->w, e->width);
    if ((e->next_code >> e->width) != 0) {
      if (e->width == 12) {
        enc_clear(e, w);
        s->w = c;
        continue;
      }
      e->width++;
    }
    if (!enc_insert(e, s->w, c, e->next_code)) {
      enc_clear(e, w);
      s->w = c;
      continue;
    }
    e->next_code++;
    s->w = c;
  }
}

static inline void enc_finish(struct LzwEnc* e, struct ArcWriter* w,
                              struct LzwStream* s) {
  if (s->have_w) {
    enc_putbits(e, w, s->w, e->width);
  }
  enc_putbits(e, w, 257, e->width);
  enc_flushbits(e, w);
}

#endif
