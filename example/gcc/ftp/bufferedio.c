
#include <fc_api.h>

#include "bufferedio.h"
#include "string.h"


void init_socket_buffer(struct SocketBuffer* socket_buf, unsigned int socketId) {
    strset(socket_buf->lastline, 0, 256);
    strset(socket_buf->buffer, 0, 256);
    socket_buf->available = 0;
    socket_buf->loaded = 0;
    socket_buf->socket_id = socketId;
}

/*
    Reads a line from an open socket, upto 256 characters long
*/
char* readline(struct SocketBuffer* socket_buf) {
    strset(socket_buf->lastline, 0, 256);
    socket_buf->loaded = 0;
    int crlfstate = 0;
    while (socket_buf->available == 0) {
        socket_buf->available = fc_tcp_read_socket(socket_buf->socket_id, socket_buf->buffer, 256);
        int i = 0;
        while (socket_buf->available) {
            if (crlfstate == 0 && socket_buf->buffer[i] == 13) {
                socket_buf->lastline[socket_buf->loaded++] = socket_buf->buffer[i++];
                socket_buf->available--;
                crlfstate = 1;
            }
            else if (crlfstate == 1 && socket_buf->buffer[i] == 10) {
                socket_buf->lastline[socket_buf->loaded++] = socket_buf->buffer[i++];
                socket_buf->available--;
                return socket_buf->lastline;
            }
            else {
                socket_buf->lastline[socket_buf->loaded++] = socket_buf->buffer[i++];
                socket_buf->available--;
            }
        }
    }
}

/*
    Fills a block with bytes from buffer, upto limit. Returns bytes read.
*/
int readstream(struct SocketBuffer* socket_buf, unsigned char* block, int limit) {
    strset(block, 0, limit);
    int blockload = 0;
    int retries = 0;
    int maxtries = 5;

    while (retries < maxtries && blockload < limit) {
        while (retries < maxtries && !socket_buf->available) {
            // todo: this entire routine can be rewritten to read directly into block and leaving
            // buffered data in the socket by reducing the read limit.
            socket_buf->available = fc_tcp_read_socket(socket_buf->socket_id, socket_buf->buffer, 256);
            socket_buf->loaded = 0;
            if (socket_buf->available) {
                retries = 0;
            }
            else {
                for (volatile int d = 0; d < 3000; d++) { /* delay */ }
                retries++;
            }
        }
        while (blockload < limit && socket_buf->available > 0) {
            block[blockload++] = socket_buf->buffer[socket_buf->loaded++];
            socket_buf->available--;
            if (blockload == limit) {
                break;
            }
        }
    }

    return blockload;
}