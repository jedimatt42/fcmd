#include "banks.h"
#define MYBANK BANK(4)

#include <string.h>
#include "commands.h"
#include "b0_globals.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"

struct VIB {
  char volumename[10];
  unsigned int aus;
  unsigned char sectors_per_track;
  unsigned char d;
  unsigned char s;
  unsigned char k;
  unsigned char protection;
  unsigned char tracks;
  unsigned char sides;
  unsigned char density;
  char reserved[36];
  unsigned char bitmap[200];
};

void handleFormat() {
  unsigned char tracks = 40;
  unsigned char sides = 1;
  unsigned char density = 1;
  unsigned char interleave = 0;
  char volumename[11];
  volumename[0] = 0;

  // usage: format [/ss] [/ds] [/sd] [/dd] [/40] [/80] [/i <interleave>] [/v <volumename>] <drive>

  int options = 1;
  while(options) {
    options = 0;
    char *peek = bk_strtokpeek(0, ' ');
    if (0 == bk_strcmpi(str2ram("/ss"), peek)) {
      sides = 1;
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/ds"), peek)) {
      sides = 2;
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/sd"), peek)) {
      density = 1;
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/dd"), peek)) {
      density = 2;
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/40"), peek)) {
      tracks = 40;
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/80"), peek)) {
      tracks = 80;
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/i"), peek)) {
      bk_strtok(0, ' '); // consume the /i 
      interleave = bk_atoi(bk_strtokpeek(0, ' '));
      options = 1;
    } else if (0 == bk_strcmpi(str2ram("/v"), peek)) {
      bk_strtok(0, ' '); // consume the /v
      char* volarg = bk_strtokpeek(0, ' ');
      if (bk_strlen(volarg) > 10) {
        tputs_rom("volume name must be 10 characters or less\n");
        return;
      }
      bk_strncpy(volumename, volarg, 10);
      options = 1;
    }
    if (options) {
      bk_strtok(0, ' '); // consume the optional argument
    }
  }

  // make sure we are talking about 1100.DSK?
  char devicename[128];
  char* patharg = bk_strtok(0, ' ');

  if (bk_str_startswith(patharg, str2ram("1100."))) {
    bk_strncpy(devicename, patharg, 128);
  } else {
    // insert 1100. the floppy controller cru on the front before device lookup
    bk_strcpy(devicename, str2ram("1100."));
    bk_strncpy(devicename + 5, patharg, (128 - 5));
  }

  struct DeviceServiceRoutine *dsr;
  char path[256];
  bk_parsePathParam(devicename, &dsr, path, PR_REQUIRED);
  if (dsr == 0)
  {
    tputs_rom("no supported disk device name specified\n");
    return;
  }

  unsigned int unitno = 0;
  if (path[0] == 'D' && path[1] == 'S' && path[2] == 'K') {
    if (path[3] >= '1' && path[3] <= '4') {
      unitno = path[3] - '0';
    }
  }
  if (unitno == 0) {
    tputs_rom("unsupported device specified\n");
    return;
  }

  // Report configured options
  tputs_rom("Formatting\n  sides:      ");
  bk_tputs_ram(bk_uint2str(sides));
  tputs_rom("\n  density:    ");
  bk_tputs_ram(bk_uint2str(density));
  tputs_rom("\n  tracks:     ");
  bk_tputs_ram(bk_uint2str(tracks));
  if (interleave) {
    tputs_rom("\n  interleave: ");
    bk_tputs_ram(bk_uint2str(interleave));
  }
  if (volumename[0]) {
    tputs_rom("\n  volumename: ");
    bk_tputs_ram(volumename);
  }
  tputs_rom("\n");

  // we should be good to go.
  unsigned int iocode = bk_path2iocode(path);

  unsigned int sectors = bk_lvl2_format(dsr->crubase, iocode, tracks, density, sides, interleave);

  // all sectors are written full of 0xe5 (on a 4A controller). So this could be validated

  tputs_rom("initializing ");
  bk_tputs_ram(bk_uint2str(sectors));
  tputs_rom(" sectors...\n");

  unsigned int buffer[128];
  for(int i = 0; i < 128; i++) { buffer[i] = 0; }

  // Load up the Volume information block
  struct VIB* vib = (struct VIB*) buffer;
  bk_strncpy(vib->volumename, str2ram(volumename), 10);
  // pad the volume name with spaces. 
  int vlen = bk_strlen(volumename);
  for(int v = vlen; v < 10; v++) {
    vib->volumename[v] = ' ';
  }
  vib->aus = sectors;
  vib->sectors_per_track = sectors / tracks;
  vib->d = 'D';
  vib->s = 'S';
  vib->k = 'K';
  vib->protection = ' ';
  vib->tracks = tracks;
  vib->sides = sides;
  vib->density = density;
  // setup the allocation bit map, first 2 sectors are used, then the free sectors, and then mark the remaing bit maps as used.
  // - first mark all bits used
  for(int b = 0; b < 200; b++) {
    vib->bitmap[b] = 0xff;
  }
  // next, free all the sectors not used.
  vib->bitmap[0] = 0x03;
  for(int c = 1; c < (sectors / 8); c++) {
    vib->bitmap[c] = 0;
  }

  bk_lvl2_sector_write(dsr->crubase, iocode, 0, (char*) buffer);

  // now clear all the sectors. 
  for(int i = 0; i < 128; i++) { buffer[i] = 0; }
  for(int i=1; i < sectors; i++) {
    bk_lvl2_sector_write(dsr->crubase, iocode, i, (char*) buffer);
    tputs_rom("\r");
    bk_tputs_ram(bk_uint2str(i));
  }
  tputs_rom("\rcompleted\n");
  tputs_rom("\n");
}
