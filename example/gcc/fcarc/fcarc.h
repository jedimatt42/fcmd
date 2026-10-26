#ifndef _FCARC_H
#define _FCARC_H 1

#include <fc_api.h>

/* Force Command provides memset in the cartridge ROM. */
extern void* memset(void* dest, int c, int count);

#define SECTOR_SIZE 256
#define RECORD_SIZE 128
#define FCARC_HEADER_SIZE 128
#define FCARC_TRAILER_SIZE 128
#define FCARC_ENTRY_FIXED 26
#define LEGACY_ENTRY_SIZE 18
#define LEGACY_MAX_ENTRIES 127
#define FCARC_MAX_ENTRIES 120
#define MAX_PATH 39
#define MAX_COMPONENT 10

static const unsigned char fcarc_magic[8] = {'F','C','A','R','C','1',0,0};
static const unsigned char fcarc_trailer_magic[8] = {'F','C','A','R','C','E','O','F'};
#define FCARC_VERSION 1

/* Decoder tables.
 *
 * prefix[] holds 12-bit prefix+1 values packed into 1.5 bytes. 0 means
 * "no prefix". suffix[] holds one byte per dictionary code. stack[] is the
 * largest LZW string we are willing to materialize; real TI archives are far
 * below this, but a corrupt stream is rejected rather than overrunning.
 */
#define DEC_PREFIX_BYTES 6144
#define DEC_SUFFIX_BYTES 4096
#define DEC_STACK_BYTES  4096
#define LEGACY_DIR_BYTES 2560

#define ENC_HASH_SLOTS 2048
#define CREATE_AREA_BYTES 6144

struct DecoderWork {
  unsigned char prefix[DEC_PREFIX_BYTES];
  unsigned char suffix[DEC_SUFFIX_BYTES];
  unsigned char stack[DEC_STACK_BYTES];
};

struct CreateWork {
  unsigned long hash[ENC_HASH_SLOTS];
  unsigned char area[CREATE_AREA_BYTES];
};

union Workspace {
  struct DecoderWork dec;
  struct CreateWork create;
};

extern union Workspace g_ws;

struct ArcReader;
struct ArcWriter;

struct CatalogEntry {
  char name[11];
  unsigned char type;
  unsigned char reclen;
  unsigned int sectors;
};

#define CATALOG_MAX 32

extern struct CatalogEntry g_catalog[CATALOG_MAX];
extern int g_catalog_count;
extern int g_catalog_overflow;
extern int g_catalog_skip;
extern int g_catalog_seen;
extern int g_quiet;

struct ArcEntry {
  char path[MAX_PATH + 1];
  unsigned char kind;
  unsigned char method;
  unsigned char flags;
  unsigned char recs;
  unsigned char eof;
  unsigned char reclen;
  unsigned int sectors;
  unsigned int l3;
  unsigned long original_size;
  unsigned long stored_size;
  unsigned long data_offset;
  unsigned int crc;
};

#define MAX_SELECT 8
#define SELECT_LEN 48

struct SelSet {
  int count;
  char item[MAX_SELECT][SELECT_LEN];
};

/* Banked entry points. The actual definitions live in b1_/b2_/b3_/b4_ files. */
FC_SAMS_BANKED(1, int, fc_do_list, (char* args), (args))
FC_SAMS_BANKED(1, int, fc_do_extract, (char* args), (args))
FC_SAMS_BANKED(1, int, fc_do_test, (char* args), (args))
FC_SAMS_BANKED(2, int, fc_legacy_list, (struct ArcReader* r), (r))
FC_SAMS_BANKED(2, int, fc_legacy_extract,
  (struct ArcReader* r, struct SelSet* sel, struct DeviceServiceRoutine* dsr,
   unsigned int iocode, const char* curpath, int extract),
  (r, sel, dsr, iocode, curpath, extract))
FC_SAMS_BANKED(2, int, fc_legacy_test,
  (struct ArcReader* r, struct DeviceServiceRoutine* dsr, const char* path),
  (r, dsr, path))
FC_SAMS_BANKED(3, int, fc_do_create, (char* args), (args))
FC_SAMS_BANKED(4, int, fc_do_create_ti, (char* args), (args))
FC_SAMS_BANKED(5, int, fc_fcarc_extract,
  (struct ArcReader* r, struct DeviceServiceRoutine* dsr, const char* path,
   struct SelSet* sel, struct DeviceServiceRoutine* odsr, unsigned int oiocode,
   const char* curpath),
  (r, dsr, path, sel, odsr, oiocode, curpath))
FC_SAMS_BANKED(6, int, fc_fcarc_test,
  (struct ArcReader* r, struct DeviceServiceRoutine* dsr, const char* path),
  (r, dsr, path))
FC_SAMS_BANKED(7, int, create_add_dir,
  (struct ArcWriter* w, const char* archpath, int* area_pos, int* count),
  (w, archpath, area_pos, count))
FC_SAMS_BANKED(7, int, create_add_file,
  (struct ArcWriter* w, struct DeviceServiceRoutine* dsr, const char* fullpath,
   const char* archpath, int* area_pos, int* count, int store),
  (w, dsr, fullpath, archpath, area_pos, count, store))

/* Callbacks must live in bank 0: Force Command invokes external callbacks with
 * bank 0 mapped. */
void catalog_vol(struct VolInfo* v);
void catalog_dir(struct DirEntry* d);

#endif
