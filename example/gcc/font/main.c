#include "fc_api.h"

void load_font(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct DisplayInformation dInfo;
  fc_display_info(&dInfo);

  struct PAB pab;
  int ferr = fc_dsr_prg_load(dsr, &pab, fname_buffer, dInfo.patternAddr, 256 * 8);
  if (ferr) {
    fc_tputs("error loading font file\n");
  }
}

void save_font(char* fname_buffer, struct DeviceServiceRoutine* dsr) {
  struct DisplayInformation dInfo;
  fc_display_info(&dInfo);

  struct PAB pab;
  int ferr = fc_dsr_prg_save(dsr, &pab, fname_buffer, dInfo.patternAddr, 256 * 8);
  if (ferr) {
    fc_tputs("error saving font file\n");
  }
}

int fc_main(char* args) {
  struct DeviceServiceRoutine* dsr;

  char tmp[256];
  char* cursor = fc_next_token(tmp, args, ' ');

  int save = 0;
  if (fc_strcmpi(tmp, "/s") == 0) {
    save = 1;
  } else {
    cursor = args;
  }

  fc_parse_path_param(cursor, &dsr, tmp, PR_REQUIRED);
  if (dsr == 0) {
    fc_tputs("error no font file specified\n\n");
    fc_tputs("FONT [/s] <file>\n");
  } else {
    if (save) {
      save_font(tmp, dsr);
    } else {
      load_font(tmp, dsr);
    }
  }
  return 0;
}
