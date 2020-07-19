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

char hostname[30];

static char commandresponse[100];
static int linebufload;
static int tcpbufavail = 0;
static int crlfstate;

static unsigned char block[256];
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
  char commandbuf[120];
  while(1) {
    tputs("ftp> ");
    strset(commandbuf, 0, 120);
    bk_getstr(5, conio_y, commandbuf, displayWidth - 3, backspace);
    bk_tputc('\n');
    bk_enable_more();
    char* tok = strtok(commandbuf, " ");
    if (!bk_strcmpi(str2ram("open"), tok)) {
      ftpOpen();
    } else if (!bk_strcmpi(str2ram("bye"), tok) || !bk_strcmpi(str2ram("quit"), tok) || !bk_strcmpi(str2ram("exit"), tok)) {
      if (connected) {
        ftpQuit();
      }
      return;
    } else if (!bk_strcmpi(str2ram("help"), tok)) {
      tputs("open <hostname> [port] - connect to an ftp server, defaults to port 21\n");
      tputs("dir [/w] [pathname] - list directory\n");
      tputs("  alias: ls\n");
      tputs("pwd - show current server directory\n");
      tputs("cd <pathname> - change server directory location\n");
      tputs("get <filename> [tiname] - retrieve a file\n");
      tputs("lcd <pathname> - change directory on local machine\n");
      tputs("ldir [pathname] - list local machine directory\n");
      tputs("bye - close connection\n");
      tputs("  aliases: exit, quit\n");
    } else if (connected) {
      if (!bk_strcmpi(str2ram("pwd"), tok)) {
        ftpPwd();
      } else if (!bk_strcmpi(str2ram("cd"), tok)) {
        ftpCd();
      } else if (!bk_strcmpi(str2ram("dir"), tok) || !strcmpi("ls", tok)) {
        ftpDir();
      } else if (!bk_strcmpi(str2ram("get"), tok)) {
        ftpGet();
      } else if (!bk_strcmpi(str2ram("lcd"), tok)) {
        ftpLcd();
      } else if (!bk_strcmpi(str2ram("ldir"), tok)) {
        bk_handleDir();
      } else {
        tputs("Error, unknown command.\n");
      }
    } else {
      tputs("Error, not connected.\n");
    }
  }
}

void ftpOpen() {
  char* host = strtok(0, " ");
  if (!host) {
    tputs("Error, no host provided.\n");
    return;
  }
  strcpy(hostname, host); // store for pasv connections later.
  char* port = strtok(0, " ");
  if (!port) {
    port = "21";
  } else {
    int pint = atoi(port);
    if (!pint) {
      tputs("Error, bad port specified.\n");
      return;
    }
  }

  int res = tcp_connect(0, host, port);
  if(!res) {
    tputs("Error, connecting to host");
    connected = 0;
    return;
  }
  connected = 1;
  tputs("connected\n");

  int code = getFtpCode();
  drainChannel(0);

  while(code != 230) {
    while(code != 331) {
      char login[20];
      strset(login, 0, 20);
      tputs("login: ");
      bk_getstr(7, conio_y, login, 20, backspace);
      bk_tputc('\n');
      code = sendFtpCommand("USER", login);
    }

    while(!(code == 230 || code == 530)) {
      char passwd[20];
      strset(passwd, 0, 20);
      tputs("password: ");
      int y = conio_y;
      bk_getstr(10, y, passwd, 20, backspace);
      int plen = strlen(passwd);

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
  char* tok = strtokpeek(0, "");
  int code = 0;
  sendFtpCommand("CWD", tok);
}

void ftpDir() {
  char* tok = strtokpeek(0, "");
  int nlist = 0;
  if (str_startswith("/w ", tok)) {
    nlist = 1;
    tok = strtok(0, " "); // consume the "/w"
    tok = strtokpeek(0, "");
  }
  sendFtpCommand("TYPE", "A");
  unsigned int port = sendFtpPasv();
  // connect second socket to provided port number.
  for(volatile int delay=0; delay<7000; delay++) { /* a moment for server to listen */ }
  int res = tcp_connect(1, hostname, uint2str(port));
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
    code = atoi(line);
  }
  tputs(line);
}

void ftpGet() {
  char* tok = strtok(0, " ");
  if (!tok) {
    tputs("Error, no file specified.\n");
    return;
  }
  char safetiname[12];
  strset(safetiname, 0, 12);
  char* tiname = strtok(0, "");
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
  tputs("tiname: ");
  tputs(tiname);
  tputs("\n");

  unsigned int unit = bk_path2unitmask(currentPath);

  sendFtpCommand("TYPE", "I");
  unsigned int port = sendFtpPasv();
  for(volatile int delay=0; delay<7000; delay++) { /* a moment for server to listen */ }
  int res = tcp_connect(1, hostname, uint2str(port));
  if (res) {
    int code = sendFtpCommand("RETR", tok);

    int len = readstream(1, 128);
    tputs("read "); tputs(int2str(len)); tputs(" bytes of data\n");

    // should have sector loaded with first 128 bytes
    //   either a foreign file record D/F128, or a TIFILES header
    struct TiFiles* tifiles = (struct TiFiles*) block;

    if (len == 128 && isTiFiles(tifiles)) {
      struct AddInfo* addInfoPtr = (struct AddInfo*) 0x8320;
      tputs("found TIFILES header\n");
      memcpy(&(addInfoPtr->first_sector), &(tifiles->sectors), 8);

      tputs("setdir: "); tputs(currentPath); tputs("\n");
      bk_lvl2_setdir(currentDsr->crubase, unit, currentPath);

      int ferr = bk_lvl2_output(currentDsr->crubase, unit, tiname, 0, addInfoPtr);
      if (ferr) {
        tputs("Error, could not output file\n");
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
            tputs("Error, failed to write block\n");
          } else {
            tputs(".");
          }
        }
        tputs("\n");
      }
    } else {
      tputs("foreign file, will use D/F 128");
      if (len == 0) {
        tputs("Error, no file data received\n");
        tcp_close(1);
        return;
      }
      char fullfilename[256];
      strcpy(fullfilename, currentPath);
      strcat(fullfilename, tiname);

      struct PAB pab;
      int ferr = bk_dsr_open(currentDsr, &pab, fullfilename, DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_OUTPUT | DSR_TYPE_SEQUENTIAL, 128);
      while (len > 0 && !ferr) {
        ferr = bk_dsr_write(currentDsr, &pab, block, 128);
        if (ferr) {
          tputs("Error, writing file\n");
          // should probably send an abort command.
          drainChannel(1);
          drainChannel(0);
          return;
        }
        len = readstream(1, 128);
        tputs("\rread "); tputs(int2str(len)); tputs(" bytes of data\n");
      }
      if (bk_dsr_close(currentDsr, &pab)) {
        tputs("Error, closing file\n");
        return;
      }
    }

  }
  int code = 0;
  char* line;
  while(code != 226) {
    line = readline(0);
    code = atoi(line);
  }
  tputs(line);
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
  int res = tcp_send_chars(0, ftpcmd, len);
  if (!res) {
    tputs("Error, server disconnected\n");
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
  strcpy(ftpcmd, "PASV");
  strcat(ftpcmd, EOL);
  int len = strlen(ftpcmd);
  int res = tcp_send_chars(0, ftpcmd, len);
  if (!res) {
    tputs("Error, server disconnected\n");
    return -1;
  }

  char* line = readline(0);
  tputs(line);
  if (!str_startswith(line, "227")) {
    return 0;
  }
  char* tok = strtok(line, "(");
  for(int i=0; i<4; i++) {
    tok = strtok(0, ",");
  }
  tok = strtok(0, ",");
  unsigned int port = ((unsigned int)atoi(tok)) << 8;
  tok = strtok(0, ")");
  port += (unsigned int)atoi(tok);

  return port;
}

char* readline(unsigned char socketId) {
  strset(commandresponse, 0, 100);
  linebufload = 0;
  crlfstate = 0;
  while (tcpbufavail == 0) {
    tcpbufavail = tcp_read_socket(socketId);
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
  strset(block, 0, 256);
  blockload = 0;
  int retries = 0;

  while(retries < 10 && blockload < limit) {
    while(retries < 10 && !tcpbufavail) {
      tcpbufavail = tcp_read_socket(socketId);
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
  tputs(line);
  // get code from first response...
  int code = atoi(line);
  return code;
}

void drainChannel(unsigned char socketId) {
  int retries = 0;
  while(retries < 10) {
    int datalen = tcp_read_socket(socketId);
    tcpbuf[datalen] = 0;
    tputs(tcpbuf);
    if (datalen) {
      retries = 0;
    } else {
      for (volatile int i=0;i<3000;i++) {/*delay*/}
      retries++;
    }
  }
}

int isTiFiles(struct TiFiles* tifiles) {
  return !basic_strcmp((char*) tifiles, "TIFILES");
}

void ftpLcd() {
  bk_handleCd();
  tputs("local dir: ");
  tputs(uint2hex(currentDsr->crubase));
  bk_tputc('.');
  tputs(currentPath);
  bk_tputc('\n');
}