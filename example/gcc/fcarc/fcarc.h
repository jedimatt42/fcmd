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

/* Decoder tables. */
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

/* Path scratch that is only needed before a file is decoded.  It shares
 * storage with the decoder/create workspaces so it costs no additional
 * program data space.  Do not use these fields while lzw_decode() is
 * running. */
struct ArcScratch {
  char arc[128];
  char full[128];
  char parent[128];
  char work[128];
  char dir[128];
  char name[16];
};

union Workspace {
  struct DecoderWork dec;
  struct CreateWork create;
  struct ArcScratch scratch;
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

#ifdef FCARC_CREATE_BUILD
extern struct CatalogEntry g_catalog[CATALOG_MAX];
extern int g_catalog_count;
extern int g_catalog_overflow;
extern int g_catalog_skip;
extern int g_catalog_seen;
extern unsigned char g_filebuf[256];
#endif
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

/* ------------------------------------------------------------------ */
/* Bank map and banked-call helpers                                   */
/* ------------------------------------------------------------------ */

#ifdef FCARC_CREATE_BUILD
#define B_MAIN 0
#define B_FILE 0
#define B_CATALOG 0
#define B_CREATE 1
#define B_LEGACY_CREATE 2
#define B_WRITER 3
#else
#define B_MAIN 0
#define B_FILE 0
#define B_READ 1
#define B_LEGACY_READ 2
#define B_EXTRACT 3
#define B_TEST 4
#endif

/* The extra CAT indirection lets B_* expand to a numeric literal before
 * token pasting, unlike the fixed-number FC_SAMS_BANKED macro. */
#define FCARC_CAT2(a,b) a##b
#define FCARC_CAT(a,b) FCARC_CAT2(a,b)

#ifdef HOST_TEST
#define FCARC_BANKED(bank_id, return_type, function_name, param_signature, param_list) \
  return_type FCARC_CAT(FCARC_CAT(b, bank_id), FCARC_CAT(_, function_name)) param_signature; \
  static inline return_type function_name param_signature { \
    return FCARC_CAT(FCARC_CAT(b, bank_id), FCARC_CAT(_, function_name)) param_list; \
  }
#else
#define FCARC_BANKED(bank_id, return_type, function_name, param_signature, param_list) \
  return_type FCARC_CAT(FCARC_CAT(b, bank_id), FCARC_CAT(_, function_name)) param_signature; \
  static inline return_type function_name param_signature { \
    if ((bank_id) == SAMS_CURRENT_BANK) { \
      return FCARC_CAT(FCARC_CAT(b, bank_id), FCARC_CAT(_, function_name)) param_list; \
    } \
    static const int fcsams_data_##function_name[] = { \
      (int)(bank_id), (int)SAMS_CURRENT_BANK, \
      (int)FCARC_CAT(FCARC_CAT(b, bank_id), FCARC_CAT(_, function_name)) \
    }; \
    FC_SAMS_TRAMP_DATA = (int)fcsams_data_##function_name; \
    return_type (*fcstramp) param_signature = (return_type (*) param_signature) FC_SAMS_TRAMP; \
    return fcstramp param_list; \
  }
#endif

#define FCARC_SAMS(bank_id, name) FCARC_CAT(FCARC_CAT(b, bank_id), FCARC_CAT(_, name))

#ifdef FCARC_CREATE_BUILD
FCARC_BANKED(B_CATALOG, int, catalog_entries,
  (struct DeviceServiceRoutine* dsr, const char* path, int skip),
  (dsr, path, skip))
FCARC_BANKED(B_CREATE, int, fc_do_create, (char* args), (args))
FCARC_BANKED(B_LEGACY_CREATE, int, fc_do_create_ti, (char* args), (args))
FCARC_BANKED(B_WRITER, int, create_add_dir,
  (struct ArcWriter* w, const char* archpath, int* area_pos, int* count),
  (w, archpath, area_pos, count))
FCARC_BANKED(B_WRITER, int, create_add_file,
  (struct ArcWriter* w, struct DeviceServiceRoutine* dsr, const char* fullpath,
   const char* archpath, int* area_pos, int* count, int store),
  (w, dsr, fullpath, archpath, area_pos, count, store))
#else
FCARC_BANKED(B_READ, int, fc_do_list, (char* args), (args))
FCARC_BANKED(B_READ, int, fc_do_extract, (char* args), (args))
FCARC_BANKED(B_READ, int, fc_do_test, (char* args), (args))
FCARC_BANKED(B_LEGACY_READ, int, fc_legacy_list, (struct ArcReader* r), (r))
FCARC_BANKED(B_LEGACY_READ, int, fc_legacy_extract,
  (struct ArcReader* r, struct SelSet* sel, struct DeviceServiceRoutine* dsr,
   unsigned int iocode, const char* curpath, int extract),
  (r, sel, dsr, iocode, curpath, extract))
FCARC_BANKED(B_LEGACY_READ, int, fc_legacy_test,
  (struct ArcReader* r, struct DeviceServiceRoutine* dsr, const char* path),
  (r, dsr, path))
FCARC_BANKED(B_EXTRACT, int, fc_fcarc_extract,
  (struct ArcReader* r, struct DeviceServiceRoutine* dsr, const char* path,
   struct SelSet* sel, struct DeviceServiceRoutine* odsr, unsigned int oiocode,
   const char* curpath),
  (r, dsr, path, sel, odsr, oiocode, curpath))
FCARC_BANKED(B_TEST, int, fc_fcarc_test,
  (struct ArcReader* r, struct DeviceServiceRoutine* dsr, const char* path),
  (r, dsr, path))
#endif

/* Callbacks must live in bank 0. */
void catalog_vol(struct VolInfo* v);
void catalog_dir(struct DirEntry* d);

#endif
