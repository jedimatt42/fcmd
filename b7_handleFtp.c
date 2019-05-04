#include "banks.h"
#define MYBANK BANK_7

#include "commands.h"
#include "b7_ti_socket.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b0_globals.h"
#include "b0_getstr.h"
#include "b2_lvl2.h"
#include <conio.h>
#include <string.h>

int connected = 0;

const char EOL[3] = {13, 10, 0};

char hostname[30];

void ftpOpen();
void ftpQuit();
void ftpPwd();
void ftpCd();
void ftpDir();
void ftpGet();

int getFtpCode();
int sendFtpCommand(char* command, char* argstring);
unsigned int sendFtpPasv();
void drainCommandChannel();
void drainChannel(unsigned char socketId);
void saveTiFile(struct AddInfo* addInfo, char* sector, int bufoffset);
void saveForeignFile(char* sector, int bufoffset);
char* readline(unsigned char socketId);

struct __attribute__((__packed__)) TiFiles {
  unsigned char seven;
  unsigned char tifiles[7];
  unsigned int sectors;
  union {
    struct {
      unsigned int variable : 1;
      unsigned int unused : 3;
      unsigned int protected : 1;
      unsigned int unused2 : 1;
      unsigned int internal : 1;
      unsigned int program : 1;
    } bits;
    unsigned char flags;
  } filetype;
  unsigned char recpersec;
  unsigned char lastseclen;
  unsigned char reclen;
  unsigned int sw_records;
}; 

int isTiFiles(struct TiFiles* buffer);


void handleFtp() {
  char commandbuf[120];
  while(1) {
    tputs("ftp> ");
    strset(commandbuf, 0, 120);
    bk_getstr(5, conio_y, commandbuf, displayWidth - 3, backspace);
    tputc('\n');
    char* tok = strtok(commandbuf, " ");
    if (!strcmpi("open", tok)) {
      ftpOpen();
    } else if (!strcmpi("bye", tok) || !strcmpi("quit", tok) || !strcmpi("exit", tok)) {
      ftpQuit();
      return;
    } else if (connected) {
      if (!strcmpi("pwd", tok)) {
        ftpPwd();
      } else if (!strcmpi("cd", tok)) {
        ftpCd();
      } else if (!strcmpi("dir", tok) || !strcmpi("ls", tok)) {
        ftpDir();
      } else if (!strcmpi("get", tok)) {
        ftpGet();
      } else if (!strcmpi("help", tok)) {
        tputs("open <hostname> [port] - connect to an ftp server, defaults to port 21\n");
        tputs("dir [/w] [pathname] - list directory\n");
        tputs("  alias: ls\n");
        tputs("pwd - show current server directory\n");
        tputs("cd <pathname> - change server directory location\n");
        tputs("get <filename> - retrieve a file\n");
        tputs("bye - close connection\n");
        tputs("  aliases: exit, quit\n");
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
  drainCommandChannel();

  while(code != 230) {
    while(code != 331) {
      char login[20];
      strset(login, 0, 20);
      tputs("login: ");
      bk_getstr(7, conio_y, login, 20, backspace);
      tputc('\n');
      code = sendFtpCommand("USER", login);
    }

    while(!(code == 230 || code == 530)) {
      char passwd[20];
      strset(passwd, 0, 20);
      tputs("password: ");
      int y = conio_y;
      bk_getstr(9, y, passwd, 20, backspace);
      int plen = strlen(passwd);

      for(int i=0; i<plen; i++) {
        tputc(8);
      }
      for(int i=0; i<plen; i++) {
        tputc('*');
      }
      tputc('\n');
      code = sendFtpCommand("PASS", passwd);
    }
  }
  drainCommandChannel();
}

void ftpQuit() {
  int code = 0;
  sendFtpCommand("QUIT", 0);
  for(volatile int i = 0; i<7000; i++) { /* spin */ }
  tcp_close(0);
  tcp_close(1);
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
  drainCommandChannel();
}

#define macro_min(x,y) ((y>x)?x:y)

void ftpGet() {
  char* tok = strtok(0, "");
  if (!tok) {
    tputs("Error, no file specified.\n");
    return;
  }
  char* tiname = strtok(0, "");
  if (!tiname) {
    tiname = tok;
  }
  sendFtpCommand("TYPE", "I");
  unsigned int port = sendFtpPasv();
  for(volatile int delay=0; delay<7000; delay++) { /* a moment for server to listen */ }
  int res = tcp_connect(1, hostname, uint2str(port));
  if (res) {
    int code = sendFtpCommand("RETR", tok);
    char sector[256];
    int bufload = 0;
    int datalen = tcp_read_socket(1);
    tputs("read "); tputs(uint2str(datalen)); tputs(" bytes\n");
    int bufoffset = 0;
    int cplen = macro_min(128, datalen);
  
    memcpy(sector, tcpbuf, cplen);
    bufload += cplen;
    bufoffset += cplen;
    
    while(bufload < 128) {
      datalen = tcp_read_socket(1);
      tputs("read "); tputs(uint2str(datalen)); tputs(" bytes\n");
      bufoffset = 0;
      cplen = macro_min(128-bufload, datalen);
      memcpy(sector + bufload, tcpbuf + bufoffset, cplen);
      bufload += cplen;
      bufoffset += cplen;
    }

    unsigned char unit = bk_path2unitmask(currentPath);

    // should have sector loaded with first 128 bytes
    //   either a foreign file record D/F128, or a TIFILES header
    struct TiFiles* tifiles = (struct TiFiles*) sector;
    struct AddInfo addInfo;

    if (isTiFiles(tifiles)) {
      tputs("found TIFILES header\n");
      addInfo.first_sector = tifiles->sectors;
      addInfo.flags = tifiles->filetype.flags;
      addInfo.recs_per_sec = tifiles->recpersec;
      addInfo.eof_offset = tifiles->lastseclen;
      addInfo.rec_length = tifiles->reclen;

      bk_lvl2_output(currentDsr->crubase, unit, tiname, 0, &addInfo);
      saveTiFile(&addInfo, sector, bufoffset);
    } else {
      tputs("foreign file, will use D/F 128");
      saveForeignFile(sector, bufoffset);
    }

  }
  drainCommandChannel();
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

static char commandresponse[100];
static int linebufload;
static int tcpbufavail = 0;
static int crlfstate;

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

int getFtpCode() {
  tcpbufavail = 0;
  // read until we get some response.
  char* line = readline(0);
  tputs(line);
  // get code from first response...
  int code = atoi(line);
  return code;
}

void drainCommandChannel() {
  drainChannel(0);
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
  tputs("seven: "); tputs(uint2str(tifiles->seven)); tputs("\n");
  tputs("sectors: "); tputs(uint2str(tifiles->sectors)); tputs("\n");
  tputs("filetype: "); tputs(uint2hex(tifiles->filetype.flags)); tputs("\n");
  return !basic_strcmp((char*) tifiles, "TIFILES");
}

void saveTiFile(struct AddInfo* addInfo, char* sector, int bufoffset) {
  // use direct output... caller has written meta-data code 0... now
  // finish gathering sectors and writing sectors...
  // sector buffer starts effectively empty but there might be data in tcpbuf still.
  
}

void saveForeignFile(char* sector, int bufoffset) {
  // use record IO... open a D/F 128 file, and write 128 byte chunks.
}