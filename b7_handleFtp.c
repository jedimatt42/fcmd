#include "banks.h"
#define MYBANK BANK_7

#include "commands.h"
#include "b7_ti_socket.h"
#include "b1cp_strutil.h"
#include "b1cp_terminal.h"
#include "b0_globals.h"
#include "b0_getstr.h"
#include <conio.h>
#include <string.h>

int connected = 0;

const char EOL[3] = {13, 10, 0};

void ftpOpen();
void ftpQuit();
void ftpPwd();
void ftpCd();

int getFtpCode();
int sendFtpCommand(char* command, char* argstring);

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
    } else if (connected) {
      if (!strcmpi("bye", tok)) {
        ftpQuit();
        return;
      } else if (!strcmpi("pwd", tok)) {
        ftpPwd();
      } else if (!strcmpi("cd", tok)) {
        ftpCd();
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

  int code = 0;
  while(code != 220) {
    code = getFtpCode();
  }

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
      if (plen == 0) {
        return;
      }
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
}

void ftpQuit() {
  int code = 0;
  while(code != 221) {
    code = sendFtpCommand("QUIT", 0);
  }
}

void ftpPwd() {
  int code = 0;
  while(code == 0) {
    code = sendFtpCommand("PWD", 0);
  }
}

void ftpCd() {
  char* tok = strtokpeek(0, "");
  int code = 0;
  while(code == 0) {
    code = sendFtpCommand("CWD", tok);
  }
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

int getFtpCode() {
  int bufsize = 0;
  // read until we get some response.
  while(1) {
    bufsize = tcp_read_socket(0);
    if (bufsize > 0) {
      break;
    }
  }
  tcpbuf[bufsize] = 0;
  tputs(tcpbuf);
  // get code from first response...
  int code = atoi(tcpbuf);
  // make sure we print any of the remaining response text.
  while(1) {
    bufsize = tcp_read_socket(0);
    if (bufsize > 0) {
      tcpbuf[bufsize] = 0;
      tputs(tcpbuf);
    } else {
      break;
    }
  }
  return code;
}