#include <fc_api.h>
#include <kscan.h>

#define SOCKET 0

#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define CMD 0xff
#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31

// state machine variables
int mode;
unsigned char command;
unsigned char param;


void terminalKey(unsigned char* buf, int* len) {
  // Length must be set to 1 before calling

  // translate output keys into correct terminal keyboard commands
  // TI control keys ctrl-a = 129 ---> ctrl-z = 154
  if (buf[0] >= 129 && buf[0] <= 154) {
    buf[0] = buf[0] - 128;
    return;
  }

  switch (buf[0]) {
  case 1: // tab
    buf[0] = 9;
    break;
  case 8: // left-arrrow
    /*
    buf[0] = 27; // esc
    buf[1] = 'D';
    *len = 2;
    */
    buf[0] = '\b';
    break;
  case 9: // right-arrow
    buf[0] = 27;
    buf[1] = 'C';
    *len = 2;
    break;
  case 10: // down-arrow
    buf[0] = 27;
    buf[1] = 'B';
    *len = 2;
    break;
  case 11: // up-arrow
    buf[0] = 27;
    buf[1] = 'A';
    *len = 2;
    break;
  case 15: // F-9
    buf[0] = 27;
    break;
  default:
    break;
  }
}


void clear_cmd_state() {
  mode = 0;
  command = 0;
  param = 0;
}

int send_cmd(unsigned char req, unsigned char param) {
  unsigned char cmdbuf[3];
  cmdbuf[0] = CMD;
  cmdbuf[1] = req;
  cmdbuf[2] = param;

  return fc_tcp_send_chars(0, cmdbuf, 3);
}

void process(int bufsize, unsigned char* buffer) {
  for(int i=0; i<bufsize; i++) {
    unsigned char current = buffer[i];
    if (mode == CMD) {
      if (command == 0) {
        command = current;
      } else {
        switch (command) {
          case DO:
            send_cmd(WONT, current);
            break;
          case DONT:
            send_cmd(WONT, current);
            break;
          case WILL:
            break;
          case WONT:
            break;
          default:
            break;
        }
        clear_cmd_state();
      }
    } else {
      if (current == CMD) {
        mode = CMD;
      } else {
        fc_tputc(current);
      }
    }
  }
}

unsigned char connect(char* args) {
  unsigned char hostname[80];
  unsigned char port[80];

  fc_strset(hostname, 0, 80);
  fc_strset(port, 0, 80);

  char* cursor = fc_next_token(hostname, args, ' ');
  if (!fc_strlen(hostname)) {
    fc_tputs("No hostname specified\n");
    return 1;
  }

  cursor = fc_next_token(port, cursor, ' ');
  if (!fc_strcmpi("/P", port)) {
    cursor = fc_next_token(port, cursor, ' ');
    int nport = fc_atoi(port);
    if (!nport) {
      fc_tputs("Bad port value\n");
      return 1;
    }
    // copy the number form back to the string to normalize.
    fc_strcpy(port, fc_uint2str(nport));
  } else {
    fc_strcpy(port, "23");
  }

  return fc_tcp_connect(SOCKET, hostname, port);
}


int main(char* args) {
  clear_cmd_state();

  unsigned char result = connect(args);
  if (result != 255) {
    fc_tputs("Error connecting\n");
    return 1;
  } else {
    fc_tputs("Connected.\n");
  }

  while( 1 ) {
    int key = fc_kscan(5);

    if (KSCAN_STATUS & KSCAN_MASK) {
      if (key == 0x01) {
        // the F7 key
        fc_tputs("Disconnecting...");
        fc_tcp_close(SOCKET);
        fc_tputs("\n");
        return 0;
      }

      // terminal may need to transform this to
      // multiple characters.
      unsigned char keybuf[4];
      keybuf[0] = key;
      int keylen = 1;

      terminalKey(keybuf, &keylen);

      if (!fc_tcp_send_chars(SOCKET, keybuf, keylen)) {
        fc_tcp_close(SOCKET);
        fc_tputs("Disconnected.\n");
        return 0;
      }
    } else {
      char buffer[512];
      int bufsize = fc_tcp_read_socket(SOCKET, buffer, 512);
      if (bufsize) {
        process(bufsize, buffer);
      }
    }
  }
}
