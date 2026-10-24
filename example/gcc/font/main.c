#include "fc_api.h"

int load_offset = 0;

void print_help() {
  term_puts("\nFONT - load/save VDP character font\n\n");
  term_puts("FONT <file>\n");
  term_puts("  Load a PROGRAM image font into the\n");
  term_puts("  pattern table.\n");
  term_puts("FONT /w <file>\n");
  term_puts("  Load a Wilhelm XB256 font, which\n");
  term_puts("  starts at the space character.\n");
  term_puts("FONT /s <file>\n");
  term_puts("  Save the current 256 character\n");
  term_puts("  patterns as a PROGRAM image.\n\n");
}

void load_font(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct GfxInformation dInfo;
  gfx_get_info(&dInfo);

  struct PAB pab;
  int ferr = dsr_prg_load(dsr, &pab, fname_buffer, dInfo.pattern_addr + load_offset, 256 * 8);
  if (ferr) {
    term_puts("error loading font file\n");
  }
}

void save_font(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct GfxInformation dInfo;
  gfx_get_info(&dInfo);

  struct PAB pab;
  int ferr = dsr_prg_save(dsr, &pab, fname_buffer, dInfo.pattern_addr, 256 * 8);
  if (ferr) {
    term_puts("error saving font file\n");
  }
}

int main(char* args) {
  struct DeviceServiceRoutine* dsr;

  if (args == 0 || args[0] == 0) {
    print_help();
    return 0;
  }

  char tmp[256];
  char* cursor = str_token_next(tmp, args, ' ');

  int save = 0;
  if (str_cmp_icase(tmp, "/s") == 0) {
    save = 1;
  } else if (str_cmp_icase(tmp, "/w") == 0) {
    load_offset += 32 * 8;
  } else {
    cursor = args;
  }

  if (cursor == 0 || cursor[0] == 0) {
    term_puts("error no font file specified\n");
    print_help();
    return 0;
  }

  path_parse(cursor, &dsr, tmp, PR_REQUIRED);
  if (dsr == 0) {
    term_puts("error no font file specified\n");
    print_help();
  } else {
    if (save) {
      save_font(tmp, dsr);
    } else {
      load_font(tmp, dsr);
    }
  }
  return 0;
}
