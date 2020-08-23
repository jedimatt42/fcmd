#include "banks.h"
#define MYBANK BANK(7)

#include "b7_ti_socket.h"
#include "b7_tipi_msg.h"
#include "b1_strutil.h"
#include <string.h>

#define TI_SOCKET_REQUEST 0x22
#define TI_SOCKET_OPEN 0x01
#define TI_SOCKET_CLOSE 0x02
#define TI_SOCKET_WRITE 0x03
#define TI_SOCKET_READ 0x04


unsigned int tcp_connect(unsigned int socketId, unsigned char* hostname, unsigned char* port) {
  unsigned char tcpbuf[256];
  tcpbuf[0] = TI_SOCKET_REQUEST;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_OPEN;
  unsigned char* cursor = tcpbuf + 3;
  bk_strcpy(cursor, hostname);
  cursor += bk_strlen(hostname);
  *cursor = ':';
  cursor++;
  bk_strcpy(cursor, str2ram(port));
  cursor += bk_strlen(str2ram(port));
  int bufsize = cursor - tcpbuf;

  tipi_on();
  tipi_sendmsg(bufsize, tcpbuf);
  bufsize = 0;
  tipi_recvmsg(&bufsize, tcpbuf);
  tipi_off();

  return (unsigned int)tcpbuf[0];
}

int tcp_send_chars(unsigned int socketId, unsigned char* buf, int size) {
  unsigned char tcpbuf[260];
  tcpbuf[0] = TI_SOCKET_REQUEST;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_WRITE;

  if (size > 256) {
    size = 256;
  }
  // append socket output to message
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

int tcp_read_socket(unsigned int socketId, unsigned char* buf, int bufsize) {
  buf[0] = TI_SOCKET_REQUEST;
  buf[1] = socketId;
  buf[2] = TI_SOCKET_READ;
  buf[3] = bufsize >> 8;
  buf[4] = bufsize & 0xff;
  tipi_on();
  tipi_sendmsg(5, buf);
  int recvsize = 0;
  tipi_recvmsg(&recvsize, buf);
  tipi_off();
  return recvsize;
}

unsigned int tcp_close(unsigned int socketId) {
  unsigned char tcpbuf[3];
  tcpbuf[0] = TI_SOCKET_REQUEST;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_CLOSE;
  tipi_on();
  tipi_sendmsg(3, tcpbuf);
  int bufsize = 0;
  tipi_recvmsg(&bufsize, tcpbuf);
  tipi_off();
  return tcpbuf[0];
}