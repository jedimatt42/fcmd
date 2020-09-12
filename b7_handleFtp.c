#include "banks.h"
#define MYBANK BANK(7)

#include <conio.h>
#include <string.h>
#include "commands.h"
#include "b0_globals.h"
#include "b1_strutil.h"
#include "b2_lvl2.h"
#include "b7_ti_socket.h"
#include "b8_terminal.h"
#include "b8_getstr.h"

int connected = 0;

const char EOL[3] = {13, 10, 0};

static char* hostname;

unsigned char* tcpbuf;

static char* commandresponse;
static int linebufload;
static int tcpbufavail = 0;
static int crlfstate;

static unsigned char* block;
static int blockload;
static int tcpbufoffset;

void ftpOpen();
void ftpQuit();
void ftpPwd();
void ftpCd();
void ftpDir();
void ftpGet();
void ftpLcd();

int getFtpCode();
int sendFtpCommand(char* command, char* argstring);
unsigned int sendFtpPasv();
void drainChannel(unsigned char socketId);
char* readline(unsigned char socketId);
void readline_paged(unsigned char socketId);
int readstream(unsigned char socketId, int limit);

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

int isTiFiles(struct TiFiles* buffer);

// this file got too big with all the inlined tputs_rom calls...
void tputs(const char* str) {
  while(*str) {
    bk_tputc(*str++);
  }
}

void handleFtp() {
  // allocate a common buffers on the stack
  unsigned char stackbuf[256];
  tcpbuf = stackbuf;
  unsigned char fileblock[256];
  block = fileblock;
  unsigned char ftpresponse[100];
  commandresponse = ftpresponse;
  unsigned char host[30];
  hostname = host;

  char commandbuf[120];
  while(1) {
    tputs_rom("ftp> ");
    bk_strset(commandbuf, 0, 120);
    bk_getstr(commandbuf, displayWidth - 3, backspace);
    bk_tputc('\n');
    bk_enable_more();
    char* tok = bk_strtok(commandbuf, ' ');
    if (!bk_strcmpi(str2ram("open"), tok)) {
      ftpOpen();
    } else if (!bk_strcmpi(str2ram("bye"), tok) || !bk_strcmpi(str2ram("quit"), tok) || !bk_strcmpi(str2ram("exit"), tok)) {
      if (connected) {
        ftpQuit();
      }
      return;
    } else if (!bk_strcmpi(str2ram("help"), tok)) {
      tputs_rom("open <hostname> [port] - connect to an ftp server, defaults to port 21\n");
      tputs_rom("dir [/w] [pathname] - list directory\n");
      tputs_rom("  alias: ls\n");
      tputs_rom("pwd - show current server directory\n");
      tputs_rom("cd <pathname> - change server directory location\n");
      tputs_rom("get <filename> [tiname] - retrieve a file\n");
      tputs_rom("lcd <pathname> - change directory on local machine\n");
      tputs_rom("ldir [pathname] - list local machine directory\n");
      tputs_rom("bye - close connection\n");
      tputs_rom("  aliases: exit, quit\n");
    } else if (connected) {
      if (!bk_strcmpi(str2ram("pwd"), tok)) {
        ftpPwd();
      } else if (!bk_strcmpi(str2ram("cd"), tok)) {
        ftpCd();
      } else if (!bk_strcmpi(str2ram("dir"), tok) || !bk_strcmpi(str2ram("ls"), tok)) {
        ftpDir();
      } else if (!bk_strcmpi(str2ram("get"), tok)) {
        ftpGet();
      } else if (!bk_strcmpi(str2ram("lcd"), tok)) {
        ftpLcd();
      } else if (!bk_strcmpi(str2ram("ldir"), tok)) {
        bk_handleDir();
      } else {
        tputs_rom("Error, unknown command.\n");
      }
    } else {
      tputs_rom("Error, not connected.\n");
    }
  }
}

void ftpOpen() {
  char* host = bk_strtok(0, ' ');
  if (!host) {
    tputs_rom("Error, no host provided.\n");
    return;
  }
  bk_strcpy(hostname, host); // store for pasv connections later.
  char* port = bk_strtok(0, ' ');
  if (!port) {
    port = "21";
  } else {
    int pint = bk_atoi(str2ram(port));
    if (!pint) {
      tputs_rom("Error, bad port specified.\n");
      return;
    }
  }

  int res = tcp_connect(0, host, port);
  if(!res) {
    tputs_rom("Error, connecting to host\n");
    connected = 0;
    return;
  }
  connected = 1;
  tputs_rom("connected\n");

  int code = getFtpCode();
  bk_tputs_ram(bk_uint2str(code));
  drainChannel(0);

  while(code != 230) {
    while(code != 331) {
      char login[20];
      bk_strset(login, 0, 20);
      tputs_rom("login: ");
      bk_getstr(login, 20, backspace);
      bk_tputc('\n');
      code = sendFtpCommand("USER", login);
    }

    while(!(code == 230 || code == 530)) {
      char passwd[20];
      bk_strset(passwd, 0, 20);
      tputs_rom("password: ");
      bk_getstr(passwd, 20, backspace);
      int plen = bk_strlen(passwd);

      for(int i=0; i<plen; i++) {
        bk_tputc(8); // backspace
      }
      for(int i=0; i<plen; i++) {
        bk_tputc('*');
      }
      bk_tputc('\n');
      code = sendFtpCommand("PASS", passwd);
    }
  }
  drainChannel(0);
}

void ftpQuit() {
  int code = 0;
  sendFtpCommand("QUIT", 0);
  for(volatile int i = 0; i<7000; i++) { /* spin */ }
  tcp_close(0);
}

void ftpPwd() {
  int code = 0;
  sendFtpCommand("PWD", 0);
}

void ftpCd() {
  char* tok = bk_strtokpeek(0, 0);
  int code = 0;
  sendFtpCommand("CWD", tok);
}

void ftpDir() {
  char* tok = bk_strtokpeek(0, 0);
  int nlist = 0;
  if (bk_str_startswith(str2ram("/w "), tok)) {
    nlist = 1;
    tok = bk_strtok(0, ' '); // consume the "/w"
    tok = bk_strtokpeek(0, 0);
  }
  sendFtpCommand("TYPE", "A");
  unsigned int port = sendFtpPasv();
  // connect second socket to provided port number.
  for(volatile int delay=0; delay<7000; delay++) { /* a moment for server to listen */ }
  int res = tcp_connect(1, hostname, bk_uint2str(port));
  if (res) {
    if (nlist) {
      sendFtpCommand("NLST", tok);
    } else {
      sendFtpCommand("LIST", tok);
    }
    drainChannel(1);
  }
  int code = 0;
  char* line;
  while(!code) {
    line = readline(0);
    code = bk_atoi(line);
  }
  tputs(line);
}

void ftpGet() {
  char* tok = bk_strtok(0, ' ');
  if (!tok) {
    tputs_rom("Error, no file specified.\n");
    return;
  }
  char safetiname[12];
  bk_strset(safetiname, 0, 12);
  char* tiname = bk_strtok(0, ' ');
  if (!tiname) {
    for(int i=0; i<10;i++) {
      if (tok[i] == '.') {
        safetiname[i] = '/';
      } else if (tok[i] == ' ') {
        safetiname[i] = '_';
      } else {
        safetiname[i] = tok[i];
      }
    }
    safetiname[10] = 0;
    tiname = safetiname;
  }
  tputs_rom("tiname: ");
  bk_tputs_ram(tiname);
  bk_tputc('\n');

  unsigned int unit = bk_path2unitmask(currentPath);

  sendFtpCommand("TYPE", "I");
  unsigned int port = sendFtpPasv();
  for(volatile int delay=0; delay<7000; delay++) { /* a moment for server to listen */ }
  int res = tcp_connect(1, hostname, bk_uint2str(port));
  if (res) {
    int code = sendFtpCommand("RETR", tok);

    int len = readstream(1, 128);
    tputs_rom("read ");
    bk_tputs_ram(bk_uint2str(len));
    tputs_rom(" bytes of data\n");

    // should have sector loaded with first 128 bytes
    //   either a foreign file record D/F128, or a TIFILES header
    struct TiFiles* tifiles = (struct TiFiles*) block;

    if (len == 128 && isTiFiles(tifiles)) {
      // AddInfo must be in scratchpad
      struct AddInfo* addInfoPtr = (struct AddInfo*) 0x8320;
      tputs_rom("found TIFILES header\n");
      memcpy(&(addInfoPtr->first_sector), &(tifiles->sectors), 8);

      tputs_rom("setdir: ");
      bk_tputs_ram(currentPath);
      bk_tputc('\n');
      bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

      int ferr = bk_lvl2_output(currentDsr->crubase, unit, tiname, 0, addInfoPtr);
      if (ferr) {
        tputs_rom("Error, could not output file\n");
      } else {
        int totalsectors = tifiles->sectors;
        int secno = 0;
        while(secno < totalsectors) {
          len = readstream(1, 256); // now work in single block chunks.
          // if (len != 256) {
          //   tputs("Error, receiving full block\n");
          //   break;
          // }
          vdpmemcpy(VDPFBUF, block, 256);
          addInfoPtr->first_sector = secno++;
          ferr = bk_lvl2_output(currentDsr->crubase, unit, tiname, 1, addInfoPtr);
          if (ferr) {
            tputs_rom("Error, failed to write block\n");
          } else {
            bk_tputc('.');
          }
        }
        bk_tputc('\n');
      }
    } else {
      tputs_rom("foreign file, will use D/F 128");
      if (len == 0) {
        tputs_rom("Error, no file data received\n");
        tcp_close(1);
        return;
      }
      char fullfilename[256];
      bk_strcpy(fullfilename, currentPath);
      bk_strcat(fullfilename, tiname);

      struct PAB pab;
      int ferr = bk_dsr_open(currentDsr, &pab, fullfilename, DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_OUTPUT | DSR_TYPE_SEQUENTIAL, 128);
      while (len > 0 && !ferr) {
        ferr = bk_dsr_write(currentDsr, &pab, block, 128);
        if (ferr) {
          tputs_rom("Error, writing file\n");
          // should probably send an abort command.
          drainChannel(1);
          drainChannel(0);
          return;
        }
        len = readstream(1, 128);
        tputs_rom("\rread ");
        bk_tputs_ram(bk_uint2str(len));
        tputs_rom(" bytes of data\n");
      }
      if (bk_dsr_close(currentDsr, &pab)) {
        tputs_rom("Error, closing file\n");
        return;
      }
    }

  }
  int code = 0;
  char* line;
  while(code != 226) {
    line = readline(0);
    code = bk_atoi(line);
  }
  tputs(line);
}

int sendFtpCommand(char* command, char* argstring) {
  char ftpcmd[80];
  bk_strcpy(ftpcmd, str2ram(command));
  if (argstring != 0) {
    bk_strcat(ftpcmd, str2ram(" "));
    bk_strcat(ftpcmd, str2ram(argstring));
  }
  bk_strcat(ftpcmd, str2ram(EOL));
  int len = bk_strlen(ftpcmd);
  int res = tcp_send_chars(0, ftpcmd, len);
  if (!res) {
    tputs_rom("Error, server disconnected\n");
    return -1;
  }

  return getFtpCode();
}

unsigned int sendFtpPasv() {
  /*  int bufsize = 0;

  PASV
  227 Entering Passive Mode (127,0,0,1,156,117).
  */
  char ftpcmd[8];
  bk_strcpy(ftpcmd, str2ram("PASV"));
  bk_strcat(ftpcmd, str2ram(EOL));
  int len = bk_strlen(ftpcmd);
  int res = tcp_send_chars(0, ftpcmd, len);
  if (!res) {
    tputs_rom("Error, server disconnected\n");
    return -1;
  }

  char* line = readline(0);
  tputs_rom(line);
  if (!bk_str_startswith(line, str2ram("227"))) {
    return 0;
  }
  char* tok = bk_strtok(line, '(');
  for(int i=0; i<4; i++) {
    tok = bk_strtok(0, ',');
  }
  tok = bk_strtok(0, ',');
  unsigned int port = ((unsigned int)bk_atoi(tok)) << 8;
  tok = bk_strtok(0, ')');
  port += (unsigned int)bk_atoi(tok);

  return port;
}

char* readline(unsigned char socketId) {
  bk_strset(commandresponse, 0, 100);
  linebufload = 0;
  crlfstate = 0;
  while (tcpbufavail == 0) {
    tcpbufavail = tcp_read_socket(socketId, tcpbuf, 256);
    int i = 0;
    while(tcpbufavail) {
      if (crlfstate == 0 && tcpbuf[i] == 13) {
        commandresponse[linebufload++] = tcpbuf[i++];
        tcpbufavail--;
        crlfstate = 1;
      } else if (crlfstate == 1 && tcpbuf[i] == 10) {
        commandresponse[linebufload++] = tcpbuf[i++];
        tcpbufavail--;
        crlfstate = 0;
        return commandresponse;
      } else {
        commandresponse[linebufload++] = tcpbuf[i++];
        tcpbufavail--;
      }
    }
  }
}

int readstream(unsigned char socketId, int limit) {
  bk_strset(block, 0, 256);
  blockload = 0;
  int retries = 0;

  while(retries < 10 && blockload < limit) {
    while(retries < 10 && !tcpbufavail) {
      tcpbufavail = tcp_read_socket(socketId, tcpbuf, 256);
      tcpbufoffset = 0;
      if (tcpbufavail) {
        retries = 0;
      } else {
        for(volatile int d=0; d<7000; d++) { /* delay */ }
        retries++;
      }
    }
    while (blockload < limit && tcpbufavail > 0) {
      block[blockload++] = tcpbuf[tcpbufoffset++];
      tcpbufavail--;
      if (blockload == limit) {
        break;
      }
    }
  }

  return blockload;
}

int getFtpCode() {
  tcpbufavail = 0;
  // read until we get some response.
  char* line = readline(0);
  bk_tputs_ram(line);
  // get code from first response...
  int code = bk_atoi(line);
  return code;
}

void drainChannel(unsigned char socketId) {
  int retries = 0;
  while(retries < 10) {
    int datalen = tcp_read_socket(socketId, tcpbuf, 256);
    tcpbuf[datalen] = 0;
    bk_tputs_ram(tcpbuf);
    if (datalen) {
      retries = 0;
    } else {
      for (volatile int i=0;i<3000;i++) {/*delay*/}
      retries++;
    }
  }
}

int isTiFiles(struct TiFiles* tifiles) {
  return !bk_basic_strcmp((char*) tifiles, str2ram("TIFILES"));
}

void ftpLcd() {
  bk_handleCd();
  tputs_rom("local dir: ");
  bk_tputs_ram(bk_uint2hex(currentDsr->crubase));
  bk_tputc('.');
  bk_tputs_ram(currentPath);
  bk_tputc('\n');
}