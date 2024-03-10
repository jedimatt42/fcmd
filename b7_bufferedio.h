#ifndef _BUFFERED_IO_H
#define _BUFFERED_IO_H 1

#include "banking.h"

struct SocketBuffer {
    unsigned int socket_id;
    char buffer[256];
    int available;
    char lastline[256];
    int loaded;
    int tls;
};

#define TLS 1
#define TCP 0


void init_socket_buffer(struct SocketBuffer* socket_buf, int tls, unsigned int socketId);
char* readline(struct SocketBuffer* socket_buf);
int readstream(struct SocketBuffer* socket_buf, char* block, int limit);

DECLARE_BANKED_VOID(init_socket_buffer, BANK(7), bk_init_socket_buffer, (struct SocketBuffer* socket_buf, int tls, unsigned int socketId), (socket_buf, tls, socketId));
DECLARE_BANKED(readline, BANK(7), char*, bk_readline, (struct SocketBuffer* socket_buf), (socket_buf));
DECLARE_BANKED(readstream, BANK(7), int, bk_readstream, (struct SocketBuffer* socket_buf, char* block, int limit), (socket_buf, block, limit));

#endif
