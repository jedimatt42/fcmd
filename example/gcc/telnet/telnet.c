#include <fc_api.h>
#include <kscan.h>
#include <ioports.h>
#include "menu.h"

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

#define READ_BUF_SIZE 128
char tcp_buf[READ_BUF_SIZE];
int tcp_buf_idx = 0;
int tcp_buf_len = 0;

unsigned char take_char(int* status);
unsigned char take_char_blocking();

void send_termtype();
void send_window_size();

char ANSI_UP[3] = { 0x1b, 0x5b, 0x41 };
char ANSI_DOWN[3] = { 0x1b, 0x5b, 0x42 };
char ANSI_RIGHT[3] = { 0x1b, 0x5b, 0x43 };
char ANSI_LEFT[3] = { 0x1b, 0x5b, 0x44 };

int terminalKey(unsigned char key) {
  // translate output keys into correct terminal keyboard commands
  // send single key byte
  if (key == KEY_UP) {
    return !fc_tcp_send_chars(SOCKET, ANSI_UP, 3);
  } else if (key == KEY_DOWN) {
    return !fc_tcp_send_chars(SOCKET, ANSI_DOWN, 3);
  } else if (key == KEY_LEFT) {
    return !fc_tcp_send_chars(SOCKET, ANSI_LEFT, 3);
  } else if (key == KEY_RIGHT) {
    return !fc_tcp_send_chars(SOCKET, ANSI_RIGHT, 3);
  } else if (key == KEY_BACK) {
    key = 27;
  } else if (key >= 129 && key <= 154) {
    // translate all the control+alpha keys
    key -= 128;
  }

  return !fc_tcp_send_chars(SOCKET, (char*) &key, 1);
}

int send_cmd(unsigned char req, unsigned char param) {
  char cmdbuf[3];
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
  unsigned char termtype[10];
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
  fc_tcp_send_chars(SOCKET, (char*) termtype, 10);
}

void send_window_size() {
  struct DisplayInformation d_info;
  fc_display_info(&d_info);

  unsigned char window_size[9];
  window_size[0] = CMD;
  window_size[1] = SUB;
  window_size[2] = CMD_WINDOW_SIZE;
  window_size[3] = 0;
  window_size[4] = d_info.displayWidth;
  window_size[5] = 0;
  window_size[6] = d_info.displayHeight;
  window_size[7] = CMD;
  window_size[8] = SE;
  fc_tcp_send_chars(SOCKET, (char*) window_size, 9);
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
  char hostname[80];
  char port[80];

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
    tcp_buf_len = fc_tcp_read_socket(SOCKET, tcp_buf, READ_BUF_SIZE);
  }
  if (tcp_buf_len) {
    *status = 1;
    return tcp_buf[tcp_buf_idx++];
  } else {
    *status = 0;
    return 0;
  }
}

void term_identify(int flag) {
  if (flag == 1) {
    char buf[8];
    buf[0] = 27;
    fc_strcpy(buf + 1, "[?1;0c");
    fc_tcp_send_chars(SOCKET, buf, 7);
  } else if (flag == 52) {
    char buf[3];
    buf[0] = 27;
    buf[1] = '/';
    buf[2] = 'Z';
    fc_tcp_send_chars(SOCKET, buf, 3);
  } else if (flag & 0x8000) {
    unsigned int x = (((unsigned int)flag) & 0x7F00) >> 8;
    unsigned int y = (((unsigned int)flag) & 0x00FF);
    char buf[30];
    buf[0] = 27;
    buf[1] = '[';
    char* cursor = buf + 2;
    fc_strcpy(cursor, fc_uint2str(y));
    cursor = buf + fc_strlen(buf);
    *cursor = ';';
    cursor++;
    fc_strcpy(cursor, fc_uint2str(x));
    cursor = buf + fc_strlen(buf);
    *cursor = 'R';
    cursor++;
    *cursor = 0;
    fc_tcp_send_chars(SOCKET, buf, fc_strlen(buf));
  }
}

// iteration counter used by blink() routine.
unsigned blinkenLights = 0;
// storage for character under cursor.
unsigned char cursor_char = 0;

void unblink() {
  if (cursor_char != 0) {
    fc_vdp_setchar(fc_vdp_get_cursor_addr(), cursor_char);
    cursor_char = 0;
  }
}

void blink() {
  if ((blinkenLights % 100) < 50) {
    if (cursor_char == 0) {
      int here = fc_vdp_get_cursor_addr();
      VDP_SET_ADDRESS(here);
      __asm__("NOP");
      cursor_char = VDPRD;
      fc_vdp_setchar(here, 219);
    }
  }
  else {
    unblink();
  }
}

void disconnect() {
  fc_tcp_close(SOCKET);
  fc_tputs("Disconnected.\n");
}

int fc_main(char* args) {
  unsigned char result = connect(args);
  if (result != 255) {
    fc_tputs("Error connecting\n");
    return 1;
  } else {
    fc_tputs("Connected.\n");
  }
  fc_set_identify_hook(term_identify);

  while( 1 ) {
    blinkenLights++;
    blink();
    int key = fc_kscan(5);

    unblink();
    if (KSCAN_STATUS & KSCAN_MASK) {
      if (key == KEY_AID) {
        int choice = show_menu();
        if (choice == KEY_QUIT) {
          disconnect();
          return 0;
        }
      }

      if (terminalKey(key) != 0) {
        return 0;
      }
    } else {
      process();
    }
  }
}
