#include <fc_api.h>
#include <ioports.h>
#include "fileload.h"
#include "gemini.h"
#include "page.h"


int FC_SAMS(2, file_exists(char* args)) {
  char filename[256];
  struct DeviceServiceRoutine* dsr = 0;
  fc_parse_path_param(args, &dsr, filename, PR_REQUIRED);

  return dsr != 0;
}

void FC_SAMS(2,file_load(char* args)) {
  char filename[256];
  struct DeviceServiceRoutine* dsr;

  fc_parse_path_param(args, &dsr, filename, PR_REQUIRED);
 
  struct PAB pab;
  int ferr = fc_dsr_open(dsr, &pab, filename, DSR_TYPE_VARIABLE | DSR_TYPE_INPUT, 80);
  if (ferr) {
    return; 
  }

  char nl[2] = "\n";

  while(!ferr) {
    ferr = fc_dsr_read(dsr, &pab, 0);
    // copy from vdp to cpu ram
    char line[80];
    fc_strset(line, 0, 80);
    vdp_memread(pab.VDPBuffer, line, pab.CharCount);
    if (pab.CharCount > 0) {
      page_from_buf(line, pab.CharCount);
    }
    page_from_buf(nl, 1);
  }

  fc_dsr_close(dsr, &pab);
}

