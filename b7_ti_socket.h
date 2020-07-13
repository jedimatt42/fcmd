#ifndef _ti_socket_h
#define _ti_socket_h 1

#include "banking.h"

// data received lands in this buffer.
// it is also overwritten to form outgoing messages.
extern unsigned char tcpbuf[];

unsigned int tcp_connect(unsigned int socketId, unsigned char* hostname, unsigned char* port);

unsigned int tcp_close(unsigned int socketId);

// will send at most 122 byte character sequences (cause size of output buffer)
int tcp_send_chars(unsigned int socketId, unsigned char* buf, int size);

int tcp_read_socket(unsigned int socketId);

DECLARE_BANKED(tcp_connect, BANK(7), unsigned int, bk_tcp_connect, (unsigned int socketId, unsigned char* hostname, unsigned char* port), (socketId, hostname, port))
DECLARE_BANKED(tcp_close, BANK(7), unsigned int, bk_tcp_close, (unsigned int socketId), (socketId))
DECLARE_BANKED(tcp_send_chars, BANK(7), int, bk_tcp_send_chars, (unsigned int socketId, unsigned char* buf, int size), (socketId, buf, size))
DECLARE_BANKED(tcp_read_socket, BANK(7), int, bk_tcp_read_socket, (unsigned int socketId), (socketId))

#endif
