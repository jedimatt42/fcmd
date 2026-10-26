#ifndef _FCARC_LZW_H
#define _FCARC_LZW_H 1

#include <fc_api.h>
#include "fcarc.h"
#include "fcarc_io.h"

/* 12-bit packed prefix table.  Stored value is prefix+1; 0 means none.
 * Since 12-bit fields always start at bit offsets 0 or 4 in a 16-bit word,
 * every field fits in two bytes and no third-byte access is needed. */
static inline int dec_prefix_get(int code) {
  unsigned int off = (unsigned int)code * 12u;
  unsigned int by = off >> 3;
  unsigned int sh = off & 7u;
  unsigned int v = (unsigned int)g_ws.dec.prefix[by]
                 | ((unsigned int)g_ws.dec.prefix[by + 1] << 8);
  v = (v >> sh) & 0x0FFF;
  if (v == 0) {
    return -1;
  }
  return (int)(v - 1);
}

static inline void dec_prefix_set(int code, int value) {
  unsigned int off = (unsigned int)code * 12u;
  unsigned int by = off >> 3;
  unsigned int sh = off & 7u;
  unsigned int v = (unsigned int)g_ws.dec.prefix[by]
                 | ((unsigned int)g_ws.dec.prefix[by + 1] << 8);
  unsigned int mask = 0x0FFF << sh;
  v = (v & ~mask) | (((unsigned int)(value + 1) << sh) & mask);
  g_ws.dec.prefix[by] = (unsigned char)(v & 0xFF);
  g_ws.dec.prefix[by + 1] = (unsigned char)((v >> 8) & 0xFF);
}

struct LzwInput {
  struct ArcReader* r;
  int bitbuf;
  int bitcount;
  int error;
};

static inline void lzw_input_init(struct LzwInput* in, struct ArcReader* r) {
  in->r = r;
  in->bitbuf = 0;
  in->bitcount = 0;
  in->error = 0;
}

static inline int lzw_input_bit(struct LzwInput* in) {
  if (in->bitcount == 0) {
    int c = ar_getc(in->r);
    if (c < 0) {
      in->error = 1;
      return -1;
    }
    in->bitbuf = c & 0xFF;
    in->bitcount = 8;
  }
  in->bitcount--;
  return (in->bitbuf >> in->bitcount) & 1;
}

static inline int lzw_input_code(struct LzwInput* in, int width) {
  int v = 0;
  int i;
  for (i = 0; i < width; i++) {
    int b = lzw_input_bit(in);
    if (b < 0) {
      return -1;
    }
    v = (v << 1) | b;
  }
  return v;
}

typedef int (*LzwSink)(void* ctx, const unsigned char* data, int len);

/* Build the byte string for code into stack and return its length.
 * Returns -1 for a corrupt or too-long sequence. */
static int lzw_decode_one(int code, int next_code, int saved_prefix,
                          unsigned char* out) {
  int n = 0;
  int c = code;
  int kwkwk = 0;

  if (code == next_code) {
    if (saved_prefix < 0) {
      return -1;
    }
    kwkwk = 1;
    c = saved_prefix;
  } else if (code > next_code) {
    return -1;
  }

  while (c >= 0) {
    if (n >= DEC_STACK_BYTES) {
      return -1;
    }
    out[n++] = g_ws.dec.suffix[c];
    c = dec_prefix_get(c);
  }

  /* Reverse in place into forward order. */
  int i;
  for (i = 0; i < (n >> 1); i++) {
    unsigned char t = out[i];
    out[i] = out[n - 1 - i];
    out[n - 1 - i] = t;
  }

  if (kwkwk) {
    if (n >= DEC_STACK_BYTES) {
      return -1;
    }
    out[n] = out[0];
    n++;
  }
  return n;
}

/* Decode a stream. Returns 0 on an end code, 1 if the sink requested a stop,
 * and -1 on a format/read error. */
static int lzw_decode(struct ArcReader* r, LzwSink sink, void* ctx) {
  struct LzwInput in;
  int width = 9;
  int next_code = 258;
  int saved_prefix = -1;

  lzw_input_init(&in, r);
  memset(g_ws.dec.prefix, 0, DEC_PREFIX_BYTES);
  int i;
  for (i = 0; i < 256; i++) {
    g_ws.dec.suffix[i] = (unsigned char)i;
  }

  while (1) {
    int code = lzw_input_code(&in, width);
    if (code < 0) {
      return -1;
    }
    if (code == 256) {
      next_code = 258;
      saved_prefix = -1;
      width = 9;
      continue;
    }
    if (code == 257) {
      return 0;
    }
    if (code > next_code) {
      return -1;
    }

    int len = lzw_decode_one(code, next_code, saved_prefix, g_ws.dec.stack);
    if (len < 0) {
      return -1;
    }
    if (sink(ctx, g_ws.dec.stack, len)) {
      return 1;
    }

    if (saved_prefix != -1 && next_code < 4096) {
      dec_prefix_set(next_code, saved_prefix);
      g_ws.dec.suffix[next_code] = g_ws.dec.stack[0];
      next_code++;
    }
    saved_prefix = code;

    if (width != 12 && (next_code >> width) != 0) {
      /* Match the TI Archiver decoder: peek for an end code before the
       * width increases. Restore both the source position and the bit
       * reader state if the peeked code is not an end code. */
      unsigned long pos = ar_tell(r);
      int bb = in.bitbuf;
      int bc = in.bitcount;
      in.error = 0;
      int peek = lzw_input_code(&in, width);
      if (peek == 257) {
        return 0;
      }
      if (peek < 0) {
        return -1;
      }
      if (ar_seek(r, pos)) {
        return -1;
      }
      in.bitbuf = bb;
      in.bitcount = bc;
      in.error = 0;
      width++;
    }
  }
}

#endif
