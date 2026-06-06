#include <fc_api.h>


void createFile() {
  fc_term_puts("create fixed file SCRATCH/FD with 10 records\n");
  char file[11] = "SCRATCH/FD";
  char destination[256];
  struct PAB pab;
  struct DeviceServiceRoutine* dest_dsr;
  fc_path_parse(file, &dest_dsr, destination, PR_REQUIRED);
  int ferr = fc_dsr_open(dest_dsr, &pab, destination, DSR_TYPE_OUTPUT | DSR_TYPE_RELATIVE, 42);
  if (ferr) {
    fc_term_puts("error\n");
    return;
  }

  char data[42];
  fc_str_set(data, ' ', 42);
  for(int i=0; i<10; i++) {
    pab.RecordNumber = i;
    fc_str_copy(data, fc_str_from_uint(i));
    fc_dsr_write(dest_dsr, &pab, data, 42);
  }

  fc_dsr_close(dest_dsr, &pab);
}

void scratchRecord(int recno) {
  fc_term_puts("scratching record ");
  fc_term_puts(fc_str_from_uint(recno));
  fc_term_puts("\n");

  char file[11] = "SCRATCH/FD";
  char destination[256];
  struct PAB pab;
  struct DeviceServiceRoutine* dest_dsr;
  fc_path_parse(file, &dest_dsr, destination, PR_REQUIRED);
  int ferr = fc_dsr_open(dest_dsr, &pab, destination, DSR_TYPE_RELATIVE, 42);
  if (ferr) {
    fc_term_puts("error\n");
    return;
  }

  ferr = fc_dsr_scratch(dest_dsr, &pab, recno);
  if (ferr) {
    fc_term_puts("error ");
    fc_term_puts(fc_str_from_uint(ferr));
    fc_term_puts("\n");
  }

  fc_dsr_close(dest_dsr, &pab);
}

int fc_main(char* args) {
  char tmp[256];
  char* cursor = fc_str_token_next(tmp, args, ' ');

  if (fc_str_cmp_icase(tmp, "/c") == 0) {
    createFile();
  } else if (fc_str_cmp_icase(tmp, "/s") == 0) {
    int recno = fc_str_to_int(cursor);
    scratchRecord(recno);
  } else {
    fc_term_puts("/c to create test file\n");
    fc_term_puts("/s <recno> to scratch given record\n");
  }

  return 0;
}
