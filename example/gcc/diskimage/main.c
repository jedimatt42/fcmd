#include "fc_api.h"
#include "ioports.h"

int usage() {
  fc_tputs("DISKIMAGE /r <device> <filename>\n\n");
  fc_tputs("DISKIMAGE /w <filename> <device>\n\n");
  return 1;
}

int create_image(struct DeviceServiceRoutine* source_dsr, char* source, struct DeviceServiceRoutine* dest_dsr, char* dest) {
  char sector_buf[256];
  
  unsigned int iocode = fc_path2iocode(source);
  int ferr = fc_lvl2_sector_read(source_dsr->crubase, iocode, 0, sector_buf);
  if (ferr) {
    fc_tputs("error reading source disk\n");
    return 1;
  }

  char volume[11];
  fc_strncpy(volume, sector_buf, 10);
  fc_tputs("volume: ");
  fc_tputs(volume);

  int total_sectors = *((int*)&(sector_buf[10]));
  fc_tputs("\ntotal sectors: ");
  fc_tputs(fc_uint2str(total_sectors));
  fc_tputs("\n");

  if (total_sectors != 360 /*90k*/ && total_sectors != 720 /*180k*/ && total_sectors != 1440 /*360k*/ && total_sectors != 1600 /*400k*/) {
    fc_tputs("\nunrecognized sector count, aborting.\n");
    return 1;
  }

  struct PAB pab;
  ferr = fc_dsr_open(dest_dsr, &pab, dest, DSR_TYPE_OUTPUT, 128); // DIS/FIX 128
  if (ferr) {
    fc_tputs("error opening destination file: ");
    fc_tputs(dest);
    fc_tputs("\n");
    return 1;
  }


  fc_tputs("\n");
  for(int sno = 0; sno < total_sectors; sno++) {
    ferr = fc_lvl2_sector_read(source_dsr->crubase, iocode, sno, sector_buf);
    if (ferr) {
      fc_tputs("error reading sector\n");
      return 1;
    }
    fc_tputs("\rsector: ");
    fc_tputs(fc_uint2str(sno));

    fc_dsr_write(dest_dsr, &pab, sector_buf, 128);
    fc_dsr_write(dest_dsr, &pab, sector_buf + 128, 128);
  }
  fc_tputs("\n");

  fc_dsr_close(dest_dsr, &pab);

  return 0;
}

int create_disk(struct DeviceServiceRoutine* source_dsr, char* source, struct DeviceServiceRoutine* dest_dsr, char* dest) {
  char sector_buf[256];
  
  unsigned int iocode = fc_path2iocode(dest);
  int ferr = fc_lvl2_sector_read(dest_dsr->crubase, iocode, 0, sector_buf);
  if (ferr) {
    fc_tputs("error reading destination disk\n");
    return 1;
  }

  struct PAB pab;
  ferr = fc_dsr_open(source_dsr, &pab, source, DSR_TYPE_INPUT, 128); // DIS/FIX 128
  if (ferr) {
    fc_tputs("error opening source file: ");
    fc_tputs(source);
    fc_tputs("\n");
    return 1;
  }

  ferr = fc_dsr_read(source_dsr, &pab, 0);

  char volume[11];
  volume[10] = 0; // volume name in image will be space padded, 
		  // need a null terminator to complete c string
  vdp_memread(pab.VDPBuffer, volume, 10);
  fc_tputs("volume: ");
  fc_tputs(volume);

  int total_sectors = 0;
  vdp_memread(pab.VDPBuffer + 10, (char*) &total_sectors, 2);
  fc_tputs("\ntotal sectors: ");
  fc_tputs(fc_uint2str(total_sectors));
  fc_tputs("\n");

  if (total_sectors != 360 /*90k*/ 
      && total_sectors != 720 /*180k*/ 
      && total_sectors != 1440 /*360k*/ 
      && total_sectors != 1600 /*400k*/) {
    fc_tputs("\nunrecognized sector count, aborting.\n");
    return 1;
  }

  fc_dsr_close(source_dsr, &pab);
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
  fc_dsr_open(source_dsr, &pab, source, DSR_TYPE_INPUT, 128);

  fc_tputs("\n");
  int record = 0;
  for(int sno = 0; sno < total_sectors; sno++) {
    // read 2 records to get a sector... 
    fc_dsr_read_cpu(source_dsr, &pab, record++, sector_buf);
    if (ferr) {
      fc_tputs("error reading record\n");
      return 1;
    }
    // the second record into second half of sector buffer
    fc_dsr_read_cpu(source_dsr, &pab, record++, sector_buf + 128);
    if (ferr) {
      fc_tputs("error reading record\n");
      return 1;
    }

    ferr = fc_lvl2_sector_write(dest_dsr->crubase, iocode, sno, sector_buf);
    if (ferr) {
      fc_tputs("error writing sector\n");
      return 1;
    }
    fc_tputs("\rsector: ");
    fc_tputs(fc_uint2str(sno));

  }
  fc_tputs("\n");

  fc_dsr_close(source_dsr, &pab);

  return 0;
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

  // copy source arg into tmp
  cursor = fc_next_token(tmp, cursor, ' ');
  char source[256];
  struct DeviceServiceRoutine* source_dsr;
  fc_parse_path_param(tmp, &source_dsr, source, PR_REQUIRED);
  if (source_dsr == 0) {
    fc_tputs("error bad source specified\n\n");
    return usage();
  }

  // copy destination arg into tmp
  cursor = fc_next_token(tmp, cursor, ' ');
  char destination[256];
  struct DeviceServiceRoutine* dest_dsr;
  fc_parse_path_param(tmp, &dest_dsr, destination, PR_REQUIRED);
  if (dest_dsr == 0) {
    fc_tputs("error bad destination specified\n\n");
    return usage();
  }

  if (read) {
    return create_image(source_dsr, source, dest_dsr, destination);
  } else {
    return create_disk(source_dsr, source, dest_dsr, destination);
  }
}
