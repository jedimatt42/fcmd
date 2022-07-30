#include "fc_api.h"

int usage() {
  fc_tputs("DISKIMAGE /r <device> <filename>\n\n");
  fc_tputs("DISKIMAGE /w <filename> <device>\n\n");
  return 1;
}

int create_image(struct DeviceServiceRoutine* source_dsr, struct DeviceServiceRoutine* dest_dsr, char* destination) {
  return 1;
}

int create_disk(struct DeviceServiceRoutine* source_dsr, char* source, struct DeviceServiceRoutine* dest_dsr) {
  return 1;
}

int fc_main(char* args) {

  char tmp[256];
  char* cursor = fc_next_token(tmp, args, ' ');

  int read = 0;
  int write = 0;
  if (fc_strcmpi(tmp, "/r") == 0) {
    read = 1;
  } else if (fc_strcmpi(tmp, "/w") == 0) {
    write = 1;
  } else {
    fc_tputs("Incorrect arguments\n\n");
    return usage();
  }

  char source[256];
  struct DeviceServiceRoutine* source_dsr;
  fc_parse_path_param(cursor, &source_dsr, source, PR_REQUIRED);
  if (source_dsr == 0) {
    fc_tputs("error bad source specified\n\n");
    return usage();
  }

  char destination[256];
  struct DeviceServiceRoutine* dest_dsr;
  fc_parse_path_param(cursor, &dest_dsr, destination, PR_REQUIRED);
  if (dest_dsr == 0) {
    fc_tputs("error bad destination specified\n\n");
    return usage();
  }

  if (read) {
    return create_image(source_dsr, dest_dsr, destination);
  } else {
    return create_disk(source_dsr, source, dest_dsr);
  }
}
