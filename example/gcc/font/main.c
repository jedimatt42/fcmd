#include "fc_api.h"

int load_offset = 0;

void load_font(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct DisplayInformation dInfo;
  fc_sys_display_info(&dInfo);

  struct PAB pab;
  int ferr = fc_dsr_prg_load(dsr, &pab, fname_buffer, dInfo.patternAddr + load_offset, 256 * 8);
  if (ferr) {
    fc_term_puts("error loading font file\n");
  }
}

void save_font(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct DisplayInformation dInfo;
  fc_sys_display_info(&dInfo);

  struct PAB pab;
  int ferr = fc_dsr_prg_save(dsr, &pab, fname_buffer, dInfo.patternAddr, 256 * 8);
  if (ferr) {
    fc_term_puts("error saving font file\n");
  }
}

int fc_main(char* args) {
  struct DeviceServiceRoutine* dsr;

  char tmp[256];
  char* cursor = fc_str_token_next(tmp, args, ' ');

  int save = 0;
  if (fc_str_cmp_icase(tmp, "/s") == 0) {
    save = 1;
  } else if (fc_str_cmp_icase(tmp, "/w") == 0) {
    load_offset += 32 * 8;
  } else {
    cursor = args;
  }

  fc_path_parse(cursor, &dsr, tmp, PR_REQUIRED);
  if (dsr == 0) {
    fc_term_puts("error no font file specified\n\n");
    fc_term_puts("FONT [/s] [/w] <file>\n");
  } else {
    if (save) {
      save_font(tmp, dsr);
    } else {
      load_font(tmp, dsr);
    }
  }
  return 0;
}
