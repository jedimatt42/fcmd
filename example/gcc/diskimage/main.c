#include "fc_api.h"
#include "ioports.h"

int usage() {
  term_puts("DISKIMAGE /r <device> <filename>\n\n");
  term_puts("DISKIMAGE /w <filename> <device>\n\n");
  return 1;
}

int create_image(struct DeviceServiceRoutine* source_dsr, char* source, struct DeviceServiceRoutine* dest_dsr, char* dest) {
  char sector_buf[256];
  
  unsigned int iocode = path_to_iocode(source);
  int ferr = lvl2_sector_read(source_dsr->crubase, iocode, 0, sector_buf);
  if (ferr) {
    term_puts("error reading source disk\n");
    return 1;
  }

  char volume[11];
  str_ncopy(volume, sector_buf, 10);
  term_puts("volume: ");
  term_puts(volume);

  int total_sectors = *((int*)&(sector_buf[10]));
  term_puts("\ntotal sectors: ");
  term_puts(str_from_uint(total_sectors));
  term_puts("\n");

  if (total_sectors != 360 /*90k*/ && total_sectors != 720 /*180k*/ && total_sectors != 1440 /*360k*/ && total_sectors != 1600 /*400k*/) {
    term_puts("\nunrecognized sector count, aborting.\n");
    return 1;
  }

  struct PAB pab;
  ferr = dsr_open(dest_dsr, &pab, dest, DSR_TYPE_OUTPUT, 128); // DIS/FIX 128
  if (ferr) {
    term_puts("error opening destination file: ");
    term_puts(dest);
    term_puts("\n");
    return 1;
  }


  term_puts("\n");
  for(int sno = 0; sno < total_sectors; sno++) {
    ferr = lvl2_sector_read(source_dsr->crubase, iocode, sno, sector_buf);
    if (ferr) {
      term_puts("error reading sector\n");
      return 1;
    }
    term_puts("\rsector: ");
    term_puts(str_from_uint(sno));

    dsr_write(dest_dsr, &pab, sector_buf, 128);
    dsr_write(dest_dsr, &pab, sector_buf + 128, 128);
  }
  term_puts("\n");

  dsr_close(dest_dsr, &pab);

  return 0;
}

int create_disk(struct DeviceServiceRoutine* source_dsr, char* source, struct DeviceServiceRoutine* dest_dsr, char* dest) {
  char sector_buf[256];
  
  unsigned int iocode = path_to_iocode(dest);
  int ferr = lvl2_sector_read(dest_dsr->crubase, iocode, 0, sector_buf);
  if (ferr) {
    term_puts("error reading destination disk\n");
    return 1;
  }

  struct PAB pab;
  ferr = dsr_open(source_dsr, &pab, source, DSR_TYPE_INPUT, 128); // DIS/FIX 128
  if (ferr) {
    term_puts("error opening source file: ");
    term_puts(source);
    term_puts("\n");
    return 1;
  }

  ferr = dsr_read(source_dsr, &pab, 0);

  char volume[11];
  volume[10] = 0; // volume name in image will be space padded, 
		  // need a null terminator to complete c string
  vdp_memread(pab.VDPBuffer, volume, 10);
  term_puts("volume: ");
  term_puts(volume);

  int total_sectors = 0;
  vdp_memread(pab.VDPBuffer + 10, (char*) &total_sectors, 2);
  term_puts("\ntotal sectors: ");
  term_puts(str_from_uint(total_sectors));
  term_puts("\n");

  if (total_sectors != 360 /*90k*/ 
      && total_sectors != 720 /*180k*/ 
      && total_sectors != 1440 /*360k*/ 
      && total_sectors != 1600 /*400k*/) {
    term_puts("\nunrecognized sector count, aborting.\n");
    return 1;
  }

  dsr_close(source_dsr, &pab);
  // reset pab
  pab.pName = 0;
  pab.CharCount = 0;
  pab.NameLength = 0;
  pab.OpCode = 0;
  pab.RecordLength = 0;
  pab.RecordNumber = 0;
  pab.ScreenOffset = 0;
  pab.Status = 0;
  pab.VDPBuffer = 0;
  // reopen
  dsr_open(source_dsr, &pab, source, DSR_TYPE_INPUT, 128);

  term_puts("\n");
  int record = 0;
  for(int sno = 0; sno < total_sectors; sno++) {
    // read 2 records to get a sector... 
    dsr_read_cpu(source_dsr, &pab, record++, sector_buf);
    if (ferr) {
      term_puts("error reading record\n");
      return 1;
    }
    // the second record into second half of sector buffer
    dsr_read_cpu(source_dsr, &pab, record++, sector_buf + 128);
    if (ferr) {
      term_puts("error reading record\n");
      return 1;
    }

    ferr = lvl2_sector_write(dest_dsr->crubase, iocode, sno, sector_buf);
    if (ferr) {
      term_puts("error writing sector\n");
      return 1;
    }
    term_puts("\rsector: ");
    term_puts(str_from_uint(sno));

  }
  term_puts("\n");

  dsr_close(source_dsr, &pab);

  return 0;
}

int main(char* args) {

  char tmp[256];
  char* cursor = str_token_next(tmp, args, ' ');

  int read = 0;
  int write = 0;
  if (str_cmp_icase(tmp, "/r") == 0) {
    read = 1;
  } else if (str_cmp_icase(tmp, "/w") == 0) {
    write = 1;
  } else {
    term_puts("Incorrect arguments\n\n");
    return usage();
  }

  // copy source arg into tmp
  cursor = str_token_next(tmp, cursor, ' ');
  char source[256];
  struct DeviceServiceRoutine* source_dsr;
  path_parse(tmp, &source_dsr, source, PR_REQUIRED);
  if (source_dsr == 0) {
    term_puts("error bad source specified\n\n");
    return usage();
  }

  // copy destination arg into tmp
  cursor = str_token_next(tmp, cursor, ' ');
  char destination[256];
  struct DeviceServiceRoutine* dest_dsr;
  path_parse(tmp, &dest_dsr, destination, PR_REQUIRED);
  if (dest_dsr == 0) {
    term_puts("error bad destination specified\n\n");
    return usage();
  }

  if (read) {
    return create_image(source_dsr, source, dest_dsr, destination);
  } else {
    return create_disk(source_dsr, source, dest_dsr, destination);
  }
}
