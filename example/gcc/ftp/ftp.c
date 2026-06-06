#include <fc_api.h>

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

int getFtpCode(struct SocketBuffer* socket_buf);
int sendFtpCommand(char* command, char* argstring);
unsigned int sendFtpPasv();
void drainChannel(struct SocketBuffer* socket_buf);

struct TiFiles {
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
void localCmd(char* cmd);

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
inline void vdpmemcpy(int pAddr, const char* pSrc, int cnt)
{
  VDP_SET_ADDRESS_WRITE(pAddr);
  while (cnt--)
  {
    VDPWD = *(pSrc++);
  }
}

void fetchInfo() {
  struct DisplayInformation info;
  fc_sys_display_info(&info);
  displayWidth = info.displayWidth;
  struct SystemInformation sys_info;
  fc_sys_info(&sys_info);
  currentDsr = sys_info.currentDsr;
  currentPath = sys_info.currentPath;
  vdp_io_buf = sys_info.vdp_io_buf;
}

int fc_main(char* args) {
  fetchInfo();

  fc_sockbuf_init(&control, TCP, 0);
  fc_sockbuf_init(&data, TCP, 1);

  // allocate a common buffers on the stack
  char host[30];
  hostname = host;

  char commandbuf[120];

  if (args && args[0]) {
    fc_str_copy(commandbuf, "open ");
    fc_str_cat(commandbuf, args);
    fc_str_token(commandbuf, ' ');
    // now parsing is caught up to where open expects it.
    ftpOpen();
  }

  while(1) {
    fc_term_puts("ftp> ");
    fc_str_set(commandbuf, 0, 120);
    fc_term_gets(commandbuf, displayWidth - 3, 1);
    fc_term_putc('\n');

    char* tok = fc_str_token(commandbuf, ' ');
    if (!fc_str_cmp_icase("open", tok)) {
      ftpOpen();
      continue;
    } else if (!fc_str_cmp_icase("bye", tok) || !fc_str_cmp_icase("quit", tok) || !fc_str_cmp_icase("exit", tok)) {
      if (connected) {
        ftpQuit();
      }
      return 0;
    } else if (!fc_str_cmp_icase("help", tok)) {
      fc_term_puts(" -- ftp version 1.1 -- \n");
      fc_term_puts("open <hostname> [port] - connect to an ftp server, defaults to port 21\n");
      fc_term_puts("dir [/w] [pathname] - list directory\n");
      fc_term_puts("  alias: ls\n");
      fc_term_puts("pwd - show current server directory\n");
      fc_term_puts("cd <pathname> - change server directory location\n");
      fc_term_puts("get <filename> [tiname] - retrieve a file\n");
      fc_term_puts("lcd <pathname> - change directory on local machine\n");
      fc_term_puts("ldir [pathname] - list local machine directory\n");
      fc_term_puts("bye - close connection\n");
      fc_term_puts("  aliases: exit, quit\n");
      continue;
    } else if (!fc_str_cmp_icase("lcd", tok)) {
      localCmd("CD");
      continue;
    } else if (!fc_str_cmp_icase("ldir", tok)) {
      localCmd("DIR");
      continue;
   } else if (connected) {
      if (!fc_str_cmp_icase("pwd", tok)) {
        ftpPwd();
        continue;
      } else if (!fc_str_cmp_icase("cd", tok)) {
        ftpCd();
        continue;
      } else if (!fc_str_cmp_icase("dir", tok) || !fc_str_cmp_icase("ls", tok)) {
        ftpDir();
        continue;
     } else if (!fc_str_cmp_icase("get", tok)) {
        ftpGet();
        continue;
      }
    }
    fc_term_puts("try 'help' to see list of commands\n");
  }

  return 0;
}

void ftpOpen() {
  char* host = fc_str_token(0, ' ');
  if (!host) {
    fc_term_puts("Error, no host provided.\n");
    return;
  }
  fc_str_copy(hostname, host); // store for pasv connections later.
  char* port = fc_str_token(0, ' ');
  if (!port) {
    port = "21";
  } else {
    int pint = fc_str_to_int(port);
    if (!pint) {
      fc_term_puts("Error, bad port specified.\n");
      return;
    }
  }

  int res = fc_tcp_connect(control.socket_id, host, port);
  if(!res) {
    fc_term_puts("Error, connecting to host\n");
    connected = 0;
    return;
  }
  connected = 1;
  fc_term_puts("connected\n");

  int code = getFtpCode(&control);
  fc_term_puts(fc_str_from_uint(code));
  drainChannel(&control);

  while(code != 230) {
    while(code != 331) {
      char login[20];
      fc_str_set(login, 0, 20);
      fc_term_puts("login: ");
      fc_term_gets(login, 20, 1);
      fc_term_putc('\n');
      code = sendFtpCommand("USER", login);
    }

    while(!(code == 230 || code == 530)) {
      char passwd[20];
      fc_str_set(passwd, 0, 20);
      fc_term_puts("password: ");
      fc_term_gets(passwd, 20, 1);
      int plen = fc_str_len(passwd);

      for(int i=0; i<plen; i++) {
        fc_term_putc(8); // backspace
      }
      for(int i=0; i<plen; i++) {
        fc_term_putc('*');
      }
      fc_term_putc('\n');
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
  char* tok = fc_str_token_peek(0, 0);
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
  int res = fc_tcp_connect(data.socket_id, hostname, fc_str_from_uint(port));
  if (res) {
    dataHandler(params);
  }
  fc_tcp_close(data.socket_id);
  drainChannel(&control);
}

void ftpDir() {
  char* tok = fc_str_token_peek(0, 0);
  int nlist = 0;
  if (fc_str_startswith("/w ", tok)) {
    nlist = 1;
    tok = fc_str_token(0, ' '); // consume the "/w"
    tok = fc_str_token_peek(0, 0);
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

  unsigned int iocode = fc_path_to_iocode(currentPath);
  char block[256];

  int len = fc_sockbuf_readstream(&data, block, 128);
  fc_term_puts("read ");
  fc_term_puts(fc_str_from_uint(len));
  fc_term_puts(" bytes of data\n");

  // should have sector loaded with first 128 bytes
  //   either a foreign file record D/F128, or a TIFILES header
  struct TiFiles* tifiles = (struct TiFiles*)block;

  if (len == 128 && isTiFiles(tifiles)) {
    fc_term_puts("found TIFILES header\n");

    // AddInfo must be in scratchpad
    struct AddInfo* addInfoPtr = (struct AddInfo*)0x8320;
    memcpy(&(addInfoPtr->first_sector), &(tifiles->sectors), 8);

    fc_term_puts("setdir: ");
    fc_term_puts(currentPath);
    fc_term_putc('\n');
    fc_lvl2_setdir(currentDsr->crubase, iocode, (char*)currentPath);

    int ferr = fc_lvl2_output(currentDsr->crubase, iocode, params[1], 0, addInfoPtr);
    if (ferr) {
      fc_term_puts("Error, could not output file\n");
    }
    else {
      int totalsectors = tifiles->sectors;
      int secno = 0;
      while (secno < totalsectors) {
        len = fc_sockbuf_readstream(&data, block, 256); // now work in single block chunks.
        vdpmemcpy(vdp_io_buf, block, 256);
        addInfoPtr->first_sector = secno++;
        ferr = fc_lvl2_output(currentDsr->crubase, iocode, params[1], 1, addInfoPtr);
        if (ferr) {
          fc_term_puts("Error, failed to write block\n");
        }
        else {
          fc_term_putc('.');
        }
      }
      fc_term_putc('\n');
    }
  } else {
    fc_term_puts("foreign file, will use D/F 128\n");
    if (len == 0) {
      fc_term_puts("Error, no file data received\n");
      fc_tcp_close(data.socket_id);
      return;
    }
    char fullfilename[256];
    fc_str_copy(fullfilename, currentPath);
    fc_str_cat(fullfilename, params[1]);

    struct PAB pab;
    int ferr = fc_dsr_open(currentDsr, &pab, fullfilename, DSR_TYPE_OUTPUT, 128);
    while (len > 0 && !ferr) {
      ferr = fc_dsr_write(currentDsr, &pab, block, 128);
      if (ferr) {
        fc_term_puts("Error, writing file\n");
        return;
      }
      len = fc_sockbuf_readstream(&data, block, 128);
      fc_term_putc('.');
    }
    if (fc_dsr_close(currentDsr, &pab)) {
      fc_term_puts("Error, closing file\n");
      return;
    }
  }
}

void ftpGet() {
  char* tok = fc_str_token(0, ' ');
  if (!tok) {
    fc_term_puts("Error, no file specified.\n");
    return;
  }
  char block[256];
  char safetiname[12];
  fc_str_set(safetiname, 0, 12);
  char* tiname = fc_str_token(0, ' ');
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

  fc_term_puts("tiname: ");
  fc_term_puts(tiname);
  fc_term_putc('\n');

  handleTransfer("I", onReceiveFile, params);
}

int sendFtpCommand(char* command, char* argstring) {
  char ftpcmd[80];
  fc_str_copy(ftpcmd, command);
  if (argstring != 0) {
    fc_str_cat(ftpcmd, " ");
    fc_str_cat(ftpcmd, argstring);
  }
  fc_str_cat(ftpcmd, EOL);
  int len = fc_str_len(ftpcmd);
  int res = fc_tcp_send_chars(control.socket_id, ftpcmd, len);
  if (!res) {
    fc_term_puts("Error, server disconnected\n");
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
  fc_str_copy(ftpcmd, "PASV");
  fc_str_cat(ftpcmd, EOL);
  int len = fc_str_len(ftpcmd);
  int res = fc_tcp_send_chars(control.socket_id, ftpcmd, len);
  if (!res) {
    fc_term_puts("Error, server disconnected\n");
    return -1;
  }

  char* line = fc_sockbuf_readline(&control);
  fc_term_puts(line);
  if (!fc_str_startswith(line, "227")) {
    return 0;
  }
  char* tok = fc_str_token(line, '(');
  for(int i=0; i<4; i++) {
    tok = fc_str_token(0, ',');
  }
  tok = fc_str_token(0, ',');
  unsigned int port = ((unsigned int)fc_str_to_int(tok)) << 8;
  tok = fc_str_token(0, ')');
  port += (unsigned int)fc_str_to_int(tok);

  return port;
}

int getFtpCode(struct SocketBuffer* socket_buf) {
  // read until we get some response.
  char* line = fc_sockbuf_readline(socket_buf);
  fc_term_puts(line);
  // get code from first response...
  int code = fc_str_to_int(line);
  return code;
}

void drainChannel(struct SocketBuffer* socket_buf) {
  char buf[256];
  fc_str_set(buf, 0, 256);
  int retries = 0;
  while(retries < 2) {
    int datalen = fc_sockbuf_readstream(socket_buf, buf, 255); // stop short to ensure last character remains nul
    buf[datalen] = 0;
    fc_term_puts(buf);
    if (datalen) {
      retries = 0;
    } else {
      retries++;
    }
  }
}

int isTiFiles(struct TiFiles* tifiles) {
  char buf[11];
  char* raw = (char*) tifiles;
  if (raw[0] != 7) {
    return 0; 
  }
  fc_str_from_basic(raw, buf);
  return !fc_str_cmp(buf, "TIFILES");
}

void localCmd(char* cmd) {
  char buf[100];
  fc_str_copy(buf, cmd);
  char* tok = fc_str_token(0, '\n');
  if (tok) {
    fc_str_cat(buf, " ");
    fc_str_cat(buf, tok);
  }
  fc_exec_cmd(buf);
}
