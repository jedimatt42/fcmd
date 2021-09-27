#ifndef _ti_socket_h
#define _ti_socket_h 1

#include "banking.h"

unsigned int tcp_connect(unsigned int socketId, char* hostname, char* port);
unsigned int tcp_close(unsigned int socketId);
int tcp_send_chars(unsigned int socketId, char* buf, int size);
int tcp_read_socket(unsigned int socketId, char* buf, int bufsize);

DECLARE_BANKED(tcp_connect, BANK(7), unsigned int, bk_tcp_connect, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))
DECLARE_BANKED(tcp_close, BANK(7), unsigned int, bk_tcp_close, (unsigned int socketId), (socketId))
DECLARE_BANKED(tcp_send_chars, BANK(7), int, bk_tcp_send_chars, (unsigned int socketId, char* buf, int size), (socketId, buf, size))
DECLARE_BANKED(tcp_read_socket, BANK(7), int, bk_tcp_read_socket, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

unsigned int tls_connect(unsigned int socketId, char* hostname, char* port);
unsigned int tls_close(unsigned int socketId);
int tls_send_chars(unsigned int socketId, char* buf, int size);
int tls_read_socket(unsigned int socketId, char* buf, int bufsize);

DECLARE_BANKED(tls_connect, BANK(7), unsigned int, bk_tls_connect, (unsigned int socketId, char* hostname, char* port), (socketId, hostname, port))
DECLARE_BANKED(tls_close, BANK(7), unsigned int, bk_tls_close, (unsigned int socketId), (socketId))
DECLARE_BANKED(tls_send_chars, BANK(7), int, bk_tls_send_chars, (unsigned int socketId, char* buf, int size), (socketId, buf, size))
DECLARE_BANKED(tls_read_socket, BANK(7), int, bk_tls_read_socket, (unsigned int socketId, char* buf, int bufsize), (socketId, buf, bufsize))

#endif
