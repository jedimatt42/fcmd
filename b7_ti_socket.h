#ifndef _ti_socket_h
#define _ti_socket_h 1

#include "banking.h"

// data received lands in this buffer.
// it is also overwritten to form outgoing messages.
extern unsigned char tcpbuf[];

unsigned int tcp_connect(unsigned char socketId, unsigned char* hostname, unsigned char* port);

// will send at most 122 byte character sequences (cause size of output buffer)
int tcp_send_chars(unsigned char socketId, unsigned char* buf, int size);

int tcp_read_socket(unsigned char socketId);

DECLARE_BANKED(tcp_connect, BANK_7, unsigned int, bk_tcp_connect, (unsigned char socketId, unsigned char* hostname, unsigned char* port), (socketId, hostname, port))
DECLARE_BANKED(tcp_send_chars, BANK_7, int, bk_tcp_send_chars, (unsigned char socketId, unsigned char* buf, int size), (socketId, buf, size))
DECLARE_BANKED(tcp_read_socket, BANK_7, int, bk_tcp_read_socket, (unsigned char socketId), (socketId))

#endif
