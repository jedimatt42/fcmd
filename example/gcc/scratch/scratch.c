#include <fc_api.h>


void createFile() {
  term_puts("create fixed file SCRATCH/FD with 10 records\n");
  char file[11] = "SCRATCH/FD";
  char destination[256];
  struct PAB pab;
  struct DeviceServiceRoutine* dest_dsr;
  path_parse(file, &dest_dsr, destination, PR_REQUIRED);
  int ferr = dsr_open(dest_dsr, &pab, destination, DSR_TYPE_OUTPUT | DSR_TYPE_RELATIVE, 42);
  if (ferr) {
    term_puts("error\n");
    return;
  }

  char data[42];
  str_set(data, ' ', 42);
  for(int i=0; i<10; i++) {
    pab.RecordNumber = i;
    str_copy(data, str_from_uint(i));
    dsr_write(dest_dsr, &pab, data, 42);
  }

  dsr_close(dest_dsr, &pab);
}

void scratchRecord(int recno) {
  term_puts("scratching record ");
  term_puts(str_from_uint(recno));
  term_puts("\n");

  char file[11] = "SCRATCH/FD";
  char destination[256];
  struct PAB pab;
  struct DeviceServiceRoutine* dest_dsr;
  path_parse(file, &dest_dsr, destination, PR_REQUIRED);
  int ferr = dsr_open(dest_dsr, &pab, destination, DSR_TYPE_RELATIVE, 42);
  if (ferr) {
    term_puts("error\n");
    return;
  }

  ferr = dsr_scratch(dest_dsr, &pab, recno);
  if (ferr) {
    term_puts("error ");
    term_puts(str_from_uint(ferr));
    term_puts("\n");
  }

  dsr_close(dest_dsr, &pab);
}

int main(char* args) {
  char tmp[256];
  char* cursor = str_token_next(tmp, args, ' ');

  if (str_cmp_icase(tmp, "/c") == 0) {
    createFile();
  } else if (str_cmp_icase(tmp, "/s") == 0) {
    int recno = str_to_int(cursor);
    scratchRecord(recno);
  } else {
    term_puts("/c to create test file\n");
    term_puts("/s <recno> to scratch given record\n");
  }

  return 0;
}
