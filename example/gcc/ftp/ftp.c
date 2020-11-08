#include <fc_api.h>

#include "string.h"
#include "bufferedio.h"

int connected = 0;

struct SocketBuffer control;
struct SocketBuffer data;

const char EOL[3] = {13, 10, 0};

static char* hostname;

void ftpOpen();
void ftpQuit();
void ftpPwd();
void ftpCd();
void ftpDir();
void ftpGet();
void ftpLcd();

int getFtpCode(struct SocketBuffer* socket_buf);
int sendFtpCommand(char* command, char* argstring);
unsigned int sendFtpPasv();
void drainChannel(struct SocketBuffer* socket_buf);

struct __attribute__((__packed__)) TiFiles {
  unsigned char seven;
  unsigned char tifiles[7];
  unsigned int sectors;
  unsigned char flags;
  unsigned char recs_per_sec;
  unsigned char eof_offset;
  unsigned char rec_length;
  unsigned int records; // swizzled
};

int displayWidth;
struct DeviceServiceRoutine* currentDsr;
const char* currentPath;
unsigned int vdp_io_buf;

int isTiFiles(struct TiFiles* buffer);

//*********************
// VDP access ports
//*********************

// Read Data
#define VDPRD	*((volatile unsigned char*)0x8800)
// Read Status
#define VDPST	*((volatile unsigned char*)0x8802)
// Write Address/Register
#define VDPWA	*((volatile unsigned char*)0x8C02)
// Write Data
#define VDPWD	*((volatile unsigned char*)0x8C00)

//*********************
// Inline VDP helpers
//*********************

// Set VDP address for read (no bit added)
inline void VDP_SET_ADDRESS(unsigned int x) { VDPWA = ((x) & 0xff); VDPWA = ((x) >> 8); }

// Set VDP address for write (adds 0x4000 bit)
inline void VDP_SET_ADDRESS_WRITE(unsigned int x) { VDPWA = ((x) & 0xff); VDPWA = (((x) >> 8) | 0x40); }
// vdpmemcpy - copies a block of data from CPU to VDP memory
// Inputs: VDP address to write to, CPU address to copy from, number of bytes to copy
// void vdpmemcpy(int pAddr, const unsigned char *pSrc, int cnt);
//   inlining this will be about the same expense as a bankswitch call.
inline void vdpmemcpy(int pAddr, const unsigned char* pSrc, int cnt)
{
  VDP_SET_ADDRESS_WRITE(pAddr);
  while (cnt--)
  {
    VDPWD = *(pSrc++);
  }
}

void fetchInfo() {
  struct DisplayInformation info;
  fc_display_info(&info);
  displayWidth = info.displayWidth;
  struct SystemInformation sys_info;
  fc_sys_info(&sys_info);
  currentDsr = sys_info.currentDsr;
  currentPath = sys_info.currentPath;
  vdp_io_buf = sys_info.vdp_io_buf;
}

int main(char* args) {
  fetchInfo();

  init_socket_buffer(&control, 0);
  init_socket_buffer(&data, 1);

  // allocate a common buffers on the stack
  unsigned char host[30];
  hostname = host;

  char commandbuf[120];
  while(1) {
    fc_tputs("ftp> ");
    strset(commandbuf, 0, 120);
    fc_getstr(commandbuf, displayWidth - 3, 1);
    fc_tputc('\n');

    char* tok = strtok(commandbuf, ' ');
    if (!strcmpi("open", tok)) {
      ftpOpen();
    } else if (!strcmpi("bye", tok) || !strcmpi("quit", tok) || !strcmpi("exit", tok)) {
      if (connected) {
        ftpQuit();
      }
      return 0;
    } else if (!strcmpi("help", tok)) {
      fc_tputs("open <hostname> [port] - connect to an ftp server, defaults to port 21\n");
      fc_tputs("dir [/w] [pathname] - list directory\n");
      fc_tputs("  alias: ls\n");
      fc_tputs("pwd - show current server directory\n");
      fc_tputs("cd <pathname> - change server directory location\n");
      fc_tputs("get <filename> [tiname] - retrieve a file\n");
      fc_tputs("lcd <pathname> - change directory on local machine\n");
      fc_tputs("ldir [pathname] - list local machine directory\n");
      fc_tputs("bye - close connection\n");
      fc_tputs("  aliases: exit, quit\n");
    } else if (connected) {
      if (!strcmpi("pwd", tok)) {
        ftpPwd();
      } else if (!strcmpi("cd", tok)) {
        ftpCd();
      } else if (!strcmpi("dir", tok) || !strcmpi("ls", tok)) {
        ftpDir();
      } else if (!strcmpi("get", tok)) {
        ftpGet();
      } else if (!strcmpi("lcd", tok)) {
        ftpLcd();
      } else if (!strcmpi("ldir", tok)) {
        // Todo: build the command from the arguments
        fc_exec("DIR");
      } else {
        fc_tputs("Error, unknown command.\n");
      }
    } else {
      fc_tputs("Error, not connected.\n");
    }
  }

  return 0;
}

void ftpOpen() {
  char* host = strtok(0, ' ');
  if (!host) {
    fc_tputs("Error, no host provided.\n");
    return;
  }
  strcpy(hostname, host); // store for pasv connections later.
  char* port = strtok(0, ' ');
  if (!port) {
    port = "21";
  } else {
    int pint = atoi(port);
    if (!pint) {
      fc_tputs("Error, bad port specified.\n");
      return;
    }
  }

  int res = fc_tcp_connect(control.socket_id, host, port);
  if(!res) {
    fc_tputs("Error, connecting to host\n");
    connected = 0;
    return;
  }
  connected = 1;
  fc_tputs("connected\n");

  int code = getFtpCode(&control);
  fc_tputs(uint2str(code));
  drainChannel(&control);

  while(code != 230) {
    while(code != 331) {
      char login[20];
      strset(login, 0, 20);
      fc_tputs("login: ");
      fc_getstr(login, 20, 1);
      fc_tputc('\n');
      code = sendFtpCommand("USER", login);
    }

    while(!(code == 230 || code == 530)) {
      char passwd[20];
      strset(passwd, 0, 20);
      fc_tputs("password: ");
      fc_getstr(passwd, 20, 1);
      int plen = strlen(passwd);

      for(int i=0; i<plen; i++) {
        fc_tputc(8); // backspace
      }
      for(int i=0; i<plen; i++) {
        fc_tputc('*');
      }
      fc_tputc('\n');
      code = sendFtpCommand("PASS", passwd);
    }
  }
  drainChannel(&control);
}

void ftpQuit() {
  int code = 0;
  sendFtpCommand("QUIT", 0);
  for(volatile int i = 0; i<7000; i++) { /* spin */ }
  fc_tcp_close(control.socket_id);
}

void ftpPwd() {
  int code = 0;
  sendFtpCommand("PWD", 0);
}

void ftpCd() {
  char* tok = strtokpeek(0, 0);
  int code = 0;
  sendFtpCommand("CWD", tok);
}

typedef void (*data_handler_func)(char* params[0]);

void onDir(char* params[]) {
  sendFtpCommand("LIST", params[0]);
  drainChannel(&data);
}

void onDirW(char* params[]) {
  sendFtpCommand("NLST", params[0]);
  drainChannel(&data);
}

void handleTransfer(char* type, data_handler_func dataHandler, char* params[]) {
  int expect_200 = sendFtpCommand("TYPE", type);
  if (expect_200 != 200) {
    return;
  }
  unsigned int port = sendFtpPasv();
  // connect second socket to provided port number.
  for (volatile int delay = 0; delay < 7000; delay++) { /* a moment for server to listen */ }
  int res = fc_tcp_connect(data.socket_id, hostname, uint2str(port));
  if (res) {
    dataHandler(params);
  }
  fc_tcp_close(data.socket_id);
  drainChannel(&control);
}

void ftpDir() {
  char* tok = strtokpeek(0, 0);
  int nlist = 0;
  if (str_startswith("/w ", tok)) {
    nlist = 1;
    tok = strtok(0, ' '); // consume the "/w"
    tok = strtokpeek(0, 0);
  }

  char* params[1];
  params[0] = tok;

  handleTransfer("A", nlist ? onDirW : onDir, params);
}

void onReceiveFile(char* params[]) {
  int code = sendFtpCommand("RETR", params[0]);
  if (code != 150) {
    fc_tcp_close(data.socket_id);
    return;
  }

  unsigned int iocode = fc_path2iocode(currentPath);
  char block[256];

  int len = readstream(&data, block, 128);
  fc_tputs("read ");
  fc_tputs(uint2str(len));
  fc_tputs(" bytes of data\n");

  // should have sector loaded with first 128 bytes
  //   either a foreign file record D/F128, or a TIFILES header
  struct TiFiles* tifiles = (struct TiFiles*)block;

  if (len == 128 && isTiFiles(tifiles)) {
    fc_tputs("found TIFILES header\n");

    // AddInfo must be in scratchpad
    struct AddInfo* addInfoPtr = (struct AddInfo*)0x8320;
    memcpy(&(addInfoPtr->first_sector), &(tifiles->sectors), 8);

    fc_tputs("setdir: ");
    fc_tputs(currentPath);
    fc_tputc('\n');
    fc_lvl2_setdir(currentDsr->crubase, iocode, (char*)currentPath);

    int ferr = fc_lvl2_output(currentDsr->crubase, iocode, params[1], 0, addInfoPtr);
    if (ferr) {
      fc_tputs("Error, could not output file\n");
    }
    else {
      int totalsectors = tifiles->sectors;
      int secno = 0;
      while (secno < totalsectors) {
        len = readstream(&data, block, 256); // now work in single block chunks.
        vdpmemcpy(vdp_io_buf, block, 256);
        addInfoPtr->first_sector = secno++;
        ferr = fc_lvl2_output(currentDsr->crubase, iocode, params[1], 1, addInfoPtr);
        if (ferr) {
          fc_tputs("Error, failed to write block\n");
        }
        else {
          fc_tputc('.');
        }
      }
      fc_tputc('\n');
    }
  } else {
    fc_tputs("foreign file, will use D/F 128\n");
    if (len == 0) {
      fc_tputs("Error, no file data received\n");
      fc_tcp_close(data.socket_id);
      return;
    }
    char fullfilename[256];
    strcpy(fullfilename, currentPath);
    strcat(fullfilename, params[1]);

    struct PAB pab;
    int ferr = fc_dsr_open(currentDsr, &pab, fullfilename, DSR_TYPE_OUTPUT, 128);
    while (len > 0 && !ferr) {
      ferr = fc_dsr_write(currentDsr, &pab, block, 128);
      if (ferr) {
        fc_tputs("Error, writing file\n");
        return;
      }
      len = readstream(&data, block, 128);
      fc_tputc('.');
    }
    if (fc_dsr_close(currentDsr, &pab)) {
      fc_tputs("Error, closing file\n");
      return;
    }
  }
}

void ftpGet() {
  char* tok = strtok(0, ' ');
  if (!tok) {
    fc_tputs("Error, no file specified.\n");
    return;
  }
  char block[256];
  char safetiname[12];
  strset(safetiname, 0, 12);
  char* tiname = strtok(0, ' ');
  if (!tiname) {
    for (int i = 0; i < 10;i++) {
      if (tok[i] == '.') {
        safetiname[i] = '/';
      }
      else if (tok[i] == ' ') {
        safetiname[i] = '_';
      }
      else {
        safetiname[i] = tok[i];
      }
    }
    safetiname[10] = 0;
    tiname = safetiname;
  }
  char* params[2];
  params[0] = tok;
  params[1] = tiname;

  fc_tputs("tiname: ");
  fc_tputs(tiname);
  fc_tputc('\n');

  handleTransfer("I", onReceiveFile, params);
}

int sendFtpCommand(char* command, char* argstring) {
  char ftpcmd[80];
  strcpy(ftpcmd, command);
  if (argstring != 0) {
    strcat(ftpcmd, " ");
    strcat(ftpcmd, argstring);
  }
  strcat(ftpcmd, EOL);
  int len = strlen(ftpcmd);
  int res = fc_tcp_send_chars(control.socket_id, ftpcmd, len);
  if (!res) {
    fc_tputs("Error, server disconnected\n");
    return -1;
  }

  return getFtpCode(&control);
}

unsigned int sendFtpPasv() {
  /*  int bufsize = 0;

  PASV
  227 Entering Passive Mode (127,0,0,1,156,117).
  */
  char ftpcmd[8];
  strcpy(ftpcmd, "PASV");
  strcat(ftpcmd, EOL);
  int len = strlen(ftpcmd);
  int res = fc_tcp_send_chars(control.socket_id, ftpcmd, len);
  if (!res) {
    fc_tputs("Error, server disconnected\n");
    return -1;
  }

  char* line = readline(&control);
  fc_tputs(line);
  if (!str_startswith(line, "227")) {
    return 0;
  }
  char* tok = strtok(line, '(');
  for(int i=0; i<4; i++) {
    tok = strtok(0, ',');
  }
  tok = strtok(0, ',');
  unsigned int port = ((unsigned int)atoi(tok)) << 8;
  tok = strtok(0, ')');
  port += (unsigned int)atoi(tok);

  return port;
}

int getFtpCode(struct SocketBuffer* socket_buf) {
  // read until we get some response.
  char* line = readline(socket_buf);
  fc_tputs(line);
  // get code from first response...
  int code = atoi(line);
  return code;
}

void drainChannel(struct SocketBuffer* socket_buf) {
  char buf[256];
  strset(buf, 0, 256);
  int retries = 0;
  while(retries < 2) {
    int datalen = readstream(socket_buf, buf, 255); // stop short to ensure last character remains nul
    buf[datalen] = 0;
    fc_tputs(buf);
    if (datalen) {
      retries = 0;
    } else {
      retries++;
    }
  }
}

int isTiFiles(struct TiFiles* tifiles) {
  return !basic_strcmp((char*) tifiles, "TIFILES");
}

void ftpLcd() {
  // Todo: build the command from the arguments
  fc_exec("CD");
}