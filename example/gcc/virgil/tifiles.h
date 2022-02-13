#pragma once

#include <fc_api.h>

struct __attribute__((__packed__)) TiFiles {
  unsigned char seven;
  unsigned char tifiles[7];
  unsigned int sectors;
  unsigned char flags;
  unsigned char recs_per_sec;
  unsigned char eof_offset;
  unsigned char rec_length;
  unsigned int records; // swizzled
};

int isTiFiles(struct TiFiles* tifiles);

