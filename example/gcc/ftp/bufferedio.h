#ifndef _BUFFERED_IO_H
#define _BUFFERED_IO_H 1


struct __attribute__((__packed__)) SocketBuffer {
    unsigned int socket_id;
    char buffer[256];
    int available;
    char lastline[256];
    int loaded;
};


void init_socket_buffer(struct SocketBuffer* socket_buf, unsigned int socketId);

char* readline(struct SocketBuffer* socket_buf);

int readstream(struct SocketBuffer* socket_buf, unsigned char* block, int limit);

#endif
