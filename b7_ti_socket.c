#include "banks.h"
#define MYBANK BANK(7)

#include "b7_ti_socket.h"
#include "b7_tipi_msg.h"
#include "b1_strutil.h"
#include <string.h>

#define TI_SOCKET_REQUEST 0x22
#define TLS_SOCKET_REQUEST 0x24
#define TI_SOCKET_OPEN 0x01
#define TI_SOCKET_CLOSE 0x02
#define TI_SOCKET_WRITE 0x03
#define TI_SOCKET_READ 0x04

static unsigned int connect(unsigned char type, unsigned int socketId, char* hostname, char* port);
static unsigned int close(unsigned char type, unsigned int socketId);
static int send_chars(unsigned char type, unsigned int socketId, char* buf, int size);
static int read_socket(unsigned char type, unsigned int socketId, char* buf, int bufsize);

unsigned int tcp_connect(unsigned int socketId, char* hostname, char* port) {
  return connect(TI_SOCKET_REQUEST, socketId, hostname, port);
}

unsigned int tls_connect(unsigned int socketId, char* hostname, char* port) {
  return connect(TLS_SOCKET_REQUEST, socketId, hostname, port);
}

int tcp_send_chars(unsigned int socketId, char* buf, int size) {
  return send_chars(TI_SOCKET_REQUEST, socketId, buf, size);
}

int tls_send_chars(unsigned int socketId, char* buf, int size) {
  return send_chars(TLS_SOCKET_REQUEST, socketId, buf, size);
}

int tcp_read_socket(unsigned int socketId, char* buf, int bufsize) {
  return read_socket(TI_SOCKET_REQUEST, socketId, buf, bufsize);
}

int tls_read_socket(unsigned int socketId, char* buf, int bufsize) {
  return read_socket(TLS_SOCKET_REQUEST, socketId, buf, bufsize);
}

unsigned int tcp_close(unsigned int socketId) {
  return close(TI_SOCKET_REQUEST, socketId);
}

unsigned int tls_close(unsigned int socketId) {
  return close(TLS_SOCKET_REQUEST, socketId);
}

static unsigned int connect(unsigned char type, unsigned int socketId, char* hostname, char* port) {
  char tcpbuf[256];
  tcpbuf[0] = type;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_OPEN;
  char* cursor = tcpbuf + 3;
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

static int send_chars(unsigned char type, unsigned int socketId, char* buf, int size) {
  unsigned char tcpbuf[260];
  tcpbuf[0] = type;
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

static int read_socket(unsigned char type, unsigned int socketId, char* buf, int bufsize) {
  unsigned char request[5];
  request[0] = type;
  request[1] = socketId;
  request[2] = TI_SOCKET_READ;
  request[3] = bufsize >> 8;
  request[4] = bufsize & 0xff;
  tipi_on();
  tipi_sendmsg(5, request);
  int recvsize = 0;
  tipi_recvmsg(&recvsize, buf);
  tipi_off();
  return recvsize;
}

static unsigned int close(unsigned char type, unsigned int socketId) {
  unsigned char tcpbuf[3];
  tcpbuf[0] = type;
  tcpbuf[1] = socketId;
  tcpbuf[2] = TI_SOCKET_CLOSE;
  tipi_on();
  tipi_sendmsg(3, tcpbuf);
  int bufsize = 0;
  tipi_recvmsg(&bufsize, tcpbuf);
  tipi_off();
  return tcpbuf[0];
}
