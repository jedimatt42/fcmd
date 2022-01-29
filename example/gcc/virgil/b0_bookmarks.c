#include <fc_api.h>
#include <ioports.h>
#include "bookmarks.h"
#include "gemini.h"
#include "page.h"

#include "debug.h"


void FC_SAMS(0,bookmarks_add_link(char* link)) {
}

void FC_SAMS(0,show_bookmarks()) {
  char line[14];
  fc_strset(line, 0, 14);
  fc_strcpy(line, "# Bookmarks\n\n");
  page_from_buf(line, 13);

  char* favs = fc_vars_get("VIRGIL_FAVS");
  if (favs == 0 || favs[0] == 0) {
    favs = "TIPI.FC.VIRGIL.BOOKMARKS";
  }
  struct PAB pab;
  struct DeviceServiceRoutine* dsr;
  char filename[30];
  fc_parse_path_param(favs, &dsr, filename, PR_REQUIRED);
  int ferr = fc_dsr_open(dsr, &pab, filename, DSR_TYPE_VARIABLE | DSR_TYPE_INPUT, 80);
  if (ferr) {
    return; // no book marks file to load
  }

  char link[4] = "=> ";
  char nl[2] = "\n";

  while(!ferr) {
    ferr = fc_dsr_read(dsr, &pab, 0);
    // copy from vdp to cpu ram
    char uri[80];
    fc_strset(uri, 0, 80);
    vdp_memread(pab.VDPBuffer, uri, pab.CharCount);
    // then add to bookmarks
    if (pab.CharCount > 0) { // skip empty lines
      page_from_buf(link, 3);
      page_from_buf(uri, pab.CharCount);
      page_from_buf(nl, 1);
    }
  }

  fc_dsr_close(dsr, &pab);
}

