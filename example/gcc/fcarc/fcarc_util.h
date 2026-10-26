#ifndef _FCARC_UTIL_H
#define _FCARC_UTIL_H 1

#include <fc_api.h>

static inline unsigned int le16(const unsigned char* p) {
  return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static inline unsigned long le32(const unsigned char* p) {
  return (unsigned long)p[0]
       | ((unsigned long)p[1] << 8)
       | ((unsigned long)p[2] << 16)
       | ((unsigned long)p[3] << 24);
}

static inline void put16le(unsigned char* p, unsigned int v) {
  p[0] = (unsigned char)(v & 0xFF);
  p[1] = (unsigned char)((v >> 8) & 0xFF);
}

static inline void put32le(unsigned char* p, unsigned long v) {
  p[0] = (unsigned char)(v & 0xFF);
  p[1] = (unsigned char)((v >> 8) & 0xFF);
  p[2] = (unsigned char)((v >> 16) & 0xFF);
  p[3] = (unsigned char)((v >> 24) & 0xFF);
}

/* CRC-16/CCITT, init 0xFFFF, poly 0x1021, nibble table. */
static const unsigned int crc16_nibble[16] = {
  0x0000, 0x1021, 0x2042, 0x3063,
  0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B,
  0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
};

static inline unsigned int crc16_byte(unsigned int crc, unsigned char b) {
  crc = (unsigned int)((crc << 4) & 0xFFFF) ^ crc16_nibble[((crc >> 12) ^ (b >> 4)) & 0x0F];
  crc = (unsigned int)((crc << 4) & 0xFFFF) ^ crc16_nibble[((crc >> 12) ^ (b & 0x0F)) & 0x0F];
  return crc & 0xFFFF;
}

static inline int ci_char(int c) {
  if (c >= 'a' && c <= 'z') {
    return c - 32;
  }
  return c;
}

static inline int ci_equal(const char* a, const char* b) {
  while (*a && *b) {
    if (ci_char((unsigned char)*a) != ci_char((unsigned char)*b)) {
      return 0;
    }
    a++;
    b++;
  }
  return *a == 0 && *b == 0;
}

/* True if path is sel or a descendant of sel. */
static inline int ci_path_match(const char* path, const char* sel) {
  while (*path && *sel) {
    if (ci_char((unsigned char)*path) != ci_char((unsigned char)*sel)) {
      return 0;
    }
    path++;
    sel++;
  }
  if (*sel == 0) {
    return *path == 0 || *path == '.';
  }
  return 0;
}

#endif
