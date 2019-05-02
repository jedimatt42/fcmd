#include "banks.h"
#define MYBANK BANK_7

#include "b7_ti_socket.h"
#include "b7_tipi_msg.h"
#include <string.h>

#define TI_SOCKET_REQUEST 0x22
#define TI_SOCKET_OPEN 0x01
#define TI_SOCKET_CLOSE 0x02
#define TI_SOCKET_WRITE 0x03
#define TI_SOCKET_READ 0x04

unsigned char tcpbuf[512];

unsigned int tcp_connect(unsigned char socketId, unsigned char* hostname, unsigned char* port) {
  tcpbuf[0] = TI_SOCKET_REQUEST;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_OPEN;
  unsigned char* cursor = tcpbuf + 3;
  strcpy(cursor, hostname);
  cursor += strlen(hostname);
  *cursor = ':';
  cursor++;
  strcpy(cursor, port);
  cursor += strlen(port);
  int bufsize = cursor - tcpbuf;

  tipi_on();
  tipi_sendmsg(bufsize, tcpbuf);
  bufsize = 0;
  tipi_recvmsg(&bufsize, tcpbuf);
  tipi_off();

  return (unsigned int)tcpbuf[0];
}

// will send at most 122 byte character sequences (cause size of output buffer)
int tcp_send_chars(unsigned char socketId, unsigned char* buf, int size) {
  tcpbuf[0] = TI_SOCKET_REQUEST;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_WRITE;

  if (size > 122) {
    size = 122;
  }
  for(int i=3; i<(3+size); i++) {
    tcpbuf[i] = buf[i-3];
  }
  tipi_on();
  tipi_sendmsg(3 + size, tcpbuf);
  int bufsize = 0;
  tipi_recvmsg(&bufsize, tcpbuf);
  tipi_off();
  return tcpbuf[0];
}

int tcp_read_socket(unsigned char socketId) {
  tcpbuf[0] = TI_SOCKET_REQUEST;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_READ;
  tcpbuf[3] = 2; // buffer size is 512 bytes
  tcpbuf[4] = 0;
  tipi_on();
  tipi_sendmsg(5, tcpbuf);
  int bufsize = 0;
  tipi_recvmsg(&bufsize, tcpbuf);
  tipi_off();
  return bufsize;
}

