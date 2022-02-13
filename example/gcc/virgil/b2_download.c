#include <fc_api.h>
#include <ioports.h>

#include "download.h"
#include "gemini.h"
#include "readline.h"
#include "tifiles.h"

void saveTiFiles(struct TiFiles* tifiles);
void saveDF128(char* data);

unsigned int iocode;
unsigned int crubase;
char filename[11];
unsigned int vdp_io_buf;
int sector_no;

void FC_SAMS(2, gemini_download_begin()) {
  state.cmd = CMD_DOWNLOAD;
  int len;
  // 0xE000
  char* data = readbytes_limit(&len, 128);
  if (len == 0) {
    state.cmd = CMD_STOP;
    return;
  }

  fc_tipi_log(fc_uint2hex(len));

  struct TiFiles* tifiles = (struct TiFiles*) data;
  if (len == 128 && isTiFiles(tifiles)) {
    fc_tipi_log("treating as TIFILES");
    saveTiFiles(tifiles);
  } else {
    fc_tipi_log("treating as DF128");
    saveDF128(data);
  }
}

void FC_SAMS(2, gemini_download_continue()) {
  int len;
  char* data = readbytes_limit(&len, 256);
  if (len == 0) {
    state.cmd = CMD_STOP;
    char msg[80];
    fc_strcpy(msg, "download complete");
    set_error(msg, 0);
    return;
  }
  fc_tipi_log("read block from socket");
  vdp_memcpy(vdp_io_buf, data, 256);
  struct AddInfo* addInfoPtr = (struct AddInfo*)0x8320;
  addInfoPtr->first_sector = sector_no++;
  int ferr = fc_lvl2_output(crubase, iocode, filename, 1, addInfoPtr);
  if (ferr) {
    state.cmd = CMD_STOP;
    char msg[80];
    fc_strcpy(msg, "error saving file");
    set_error(msg, 0x7fff);
    return;
  }
  
  fc_tipi_log("wrote block");
}

void set_download_filename(char* filename) {
  int filepart = 1 + fc_lindexof(state.newurl, '/', fc_strlen(state.newurl));
  fc_strncpy(filename, state.newurl + filepart, 11);
  fc_tipi_log(filename);
}

struct DeviceServiceRoutine* get_downloads_dir(char* pathname) {
  char* dls = fc_vars_get("DOWNLOADS");
  if (dls == 0 || dls[0] == 0) {
    dls = "TIPI.DOWNLOADS.";
  }
  struct DeviceServiceRoutine* dsr;
  fc_parse_path_param(dls, &dsr, pathname, PR_REQUIRED);
  int plen = fc_strlen(pathname);
  if (pathname[plen - 1] != '.') {
    pathname[plen] = '.';
    pathname[plen+1] = 0;
  }
  return dsr;
}

void saveTiFiles(struct TiFiles* tifiles) {
  // create 'sector' 0 - FDR of file from TIFILES data
  char pathname[256];
  struct DeviceServiceRoutine* dsr = get_downloads_dir(pathname);
  crubase = dsr->crubase;
 
  struct SystemInformation sys_info;
  fc_sys_info(&sys_info); 
  vdp_io_buf = sys_info.vdp_io_buf;

  // addinfo must be in scratchpad
  struct AddInfo* addInfoPtr = (struct AddInfo*)0x8320;
  memcpy(&(addInfoPtr->first_sector), &(tifiles->sectors), 8);
  
  iocode = fc_path2iocode(pathname);
  char msg[80];
  int ferr = fc_lvl2_setdir(crubase, iocode, pathname);
  if (ferr) {
    fc_strcpy(msg, "download directory does not exist");
    set_error(msg, 0);
    state.cmd = CMD_STOP;
    return;
  }

  set_download_filename(filename);

  ferr = fc_lvl2_output(crubase, iocode, filename, 0, addInfoPtr);
  if (ferr) {
    fc_strcpy(msg, "error saving file");
    set_error(msg, 0);
    state.cmd = CMD_STOP;
  }
  // main loop will continue to do 256 byte chunks to finish the file out
  // in gemini_download_continue
  sector_no = 0;
}

void saveDF128(char* data) {
  char msg[80];
  fc_strcpy(msg, "not a TIFILES format file");
  set_error(msg, 0x7fff);
  state.cmd = CMD_STOP;
}

