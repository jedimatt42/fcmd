#include <fc_api.h>


void createFile() {
  fc_tputs("create fixed file SCRATCH/FD with 10 records\n");
  char file[11] = "SCRATCH/FD";
  char destination[256];
  struct PAB pab;
  struct DeviceServiceRoutine* dest_dsr;
  fc_parse_path_param(file, &dest_dsr, destination, PR_REQUIRED);
  int ferr = fc_dsr_open(dest_dsr, &pab, destination, DSR_TYPE_OUTPUT | DSR_TYPE_RELATIVE, 42);
  if (ferr) {
    fc_tputs("error\n");
    return;
  }

  char data[42];
  fc_strset(data, ' ', 42);
  for(int i=0; i<10; i++) {
    pab.RecordNumber = i;
    fc_strcpy(data, fc_uint2str(i));
    fc_dsr_write(dest_dsr, &pab, data, 42);
  }

  fc_dsr_close(dest_dsr, &pab);
}

void scratchRecord(int recno) {
  fc_tputs("scratching record ");
  fc_tputs(fc_uint2str(recno));
  fc_tputs("\n");

  char file[11] = "SCRATCH/FD";
  char destination[256];
  struct PAB pab;
  struct DeviceServiceRoutine* dest_dsr;
  fc_parse_path_param(file, &dest_dsr, destination, PR_REQUIRED);
  int ferr = fc_dsr_open(dest_dsr, &pab, destination, DSR_TYPE_RELATIVE, 42);
  if (ferr) {
    fc_tputs("error\n");
    return;
  }

  ferr = fc_dsr_scratch(dest_dsr, &pab, recno);
  if (ferr) {
    fc_tputs("error ");
    fc_tputs(fc_uint2str(ferr));
    fc_tputs("\n");
  }

  fc_dsr_close(dest_dsr, &pab);
}

int fc_main(char* args) {
  char tmp[256];
  char* cursor = fc_next_token(tmp, args, ' ');

  if (fc_strcmpi(tmp, "/c") == 0) {
    createFile();
  } else if (fc_strcmpi(tmp, "/s") == 0) {
    int recno = fc_atoi(cursor);
    scratchRecord(recno);
  } else {
    fc_tputs("/c to create test file\n");
    fc_tputs("/s <recno> to scratch given record\n");
  }

  return 0;
}
