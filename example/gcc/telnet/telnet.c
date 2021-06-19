#include <fc_api.h>
#include <kscan.h>

#define SOCKET 0

// IAC code
#define CMD 0xff

// IAC request codes
#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define SUB 250
#define SE 240

// IAC param codes
#define CMD_ECHO 1
#define CMD_TERMINAL_TYPE 24
#define CMD_WINDOW_SIZE 31
#define SEND 1
#define IS 0

unsigned char tcp_buf[32];
int tcp_buf_idx = 0;
int tcp_buf_len = 0;

unsigned char take_char(int* status);
unsigned char take_char_blocking();

void send_termtype();
void send_window_size();


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

int send_cmd(unsigned char req, unsigned char param) {
  unsigned char cmdbuf[3];
  cmdbuf[0] = CMD;
  cmdbuf[1] = req;
  cmdbuf[2] = param;
  return fc_tcp_send_chars(0, cmdbuf, 3);
}

void handle_do_cmd(unsigned char request) {
  // fc_tputs("received: DO ");
  // fc_tputs(fc_uint2str(request));
  // fc_tputc('\n');
  switch(request) {
    case CMD_TERMINAL_TYPE:
      send_cmd(WILL, CMD_TERMINAL_TYPE);
      break;
    case CMD_WINDOW_SIZE:
      send_cmd(WILL, CMD_WINDOW_SIZE);
      send_window_size();
      break;
    default:
      send_cmd(WONT, request);
  }
}

void handle_dont_cmd(unsigned char request) {
  // fc_tputs("received: DONT ");
  // fc_tputs(fc_uint2str(request));
  // fc_tputc('\n');
  send_cmd(WONT, request);
}

void handle_will_cmd(unsigned char request) {
  // fc_tputs("received: WILL ");
  // fc_tputs(fc_uint2str(request));
  // fc_tputc('\n');
}

void handle_wont_cmd(unsigned char request) {
  // fc_tputs("received: WONT ");
  // fc_tputs(fc_uint2str(request));
  // fc_tputc('\n');
}

void handle_sub_cmd(unsigned char request) {
  // fc_tputs("received: SUB ");
  // fc_tputs(fc_uint2str(request));
  // fc_tputc('\n');
  if (request == CMD_TERMINAL_TYPE) {
    unsigned char send = take_char_blocking();
    unsigned char iac = take_char_blocking(); // consume IAC
    unsigned char se = take_char_blocking(); // consume SE
    if (send == SEND && iac == CMD && se == SE) {
      send_termtype();
    }
  }
}

void send_termtype() {
  char termtype[10];
  termtype[0] = CMD;
  termtype[1] = SUB;
  termtype[2] = CMD_TERMINAL_TYPE;
  termtype[3] = IS;
  termtype[4] = 'A';
  termtype[5] = 'N';
  termtype[6] = 'S';
  termtype[7] = 'I';
  termtype[8] = CMD;
  termtype[9] = SE;
  fc_tcp_send_chars(SOCKET, termtype, 10);
}

void send_window_size() {
  struct DisplayInformation d_info;
  fc_display_info(&d_info);

  char window_size[9];
  window_size[0] = CMD;
  window_size[1] = SUB;
  window_size[2] = CMD_WINDOW_SIZE;
  window_size[3] = 0;
  window_size[4] = d_info.displayWidth;
  window_size[5] = 0;
  window_size[6] = d_info.displayHeight;
  window_size[7] = CMD;
  window_size[8] = SE;
  fc_tcp_send_chars(SOCKET, window_size, 9);
}

unsigned char take_char_blocking() {
  int status = 0;
  unsigned char result = 0;
  while(status == 0) {
    result = take_char(&status);
  }
  return result;
}

void handle_iac() {
  unsigned char cmd = take_char_blocking();
  unsigned char request = take_char_blocking();
  switch(cmd) {
    case DO:
      handle_do_cmd(request);
      break;
    case DONT:
      handle_dont_cmd(request);
      break;
    case WILL:
      handle_will_cmd(request);
      break;
    case WONT:
      handle_wont_cmd(request);
      break;
    case SUB:
      handle_sub_cmd(request);
      break;
  }
}


void process() {
  int status;
  unsigned char current = take_char(&status);
  if (status) {
    if (current == CMD) {
      handle_iac();
    } else {
      fc_tputc(current);
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

unsigned char take_char(int* status) {
  if ((tcp_buf_len - tcp_buf_idx) == 0) {
    tcp_buf_idx = 0;
    tcp_buf_len = fc_tcp_read_socket(SOCKET, tcp_buf, 32);
  }
  if (tcp_buf_len) {
    *status = 1;
    return tcp_buf[tcp_buf_idx++];
  } else {
    *status = 0;
    return 0;
  }
}

int main(char* args) {
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
      process();
    }
  }
}
