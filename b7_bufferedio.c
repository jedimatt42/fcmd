
#include "banks.h"
#define MYBANK BANK(7)

#include "b7_bufferedio.h"
#include "b7_ti_socket.h"
#include "b1_strutil.h"


void init_socket_buffer(struct SocketBuffer* socket_buf, int tls, unsigned int socketId) {
    bk_strset(socket_buf->lastline, 0, 256);
    bk_strset(socket_buf->buffer, 0, 256);
    socket_buf->available = 0;
    socket_buf->loaded = 0;
    socket_buf->socket_id = socketId;
    socket_buf->tls = tls;
}

static int socket_read(struct SocketBuffer* socket_buf) {
    if (socket_buf->tls) {
        return tls_read_socket(socket_buf->socket_id, socket_buf->buffer, 256);
    }
    else {
        return tcp_read_socket(socket_buf->socket_id, socket_buf->buffer, 256);
    }
}

/*
    Reads a line from an open socket, upto 256 characters long
*/
char* readline(struct SocketBuffer* socket_buf) {
    bk_strset(socket_buf->lastline, 0, 256);
    socket_buf->loaded = 0;
    int crlfstate = 0;
    while (socket_buf->available == 0) {
        socket_buf->available = socket_read(socket_buf);

        int i = 0;
        while (socket_buf->available) {
            if (crlfstate == 0 && socket_buf->buffer[i] == 13) {
                socket_buf->lastline[socket_buf->loaded++] = socket_buf->buffer[i++];
                socket_buf->available--;
                crlfstate = 1;
            }
            else if (socket_buf->buffer[i] == 10) {
                socket_buf->lastline[socket_buf->loaded] = 10;
                if (crlfstate != 1) {
                    socket_buf->loaded++;
                }
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
int readstream(struct SocketBuffer* socket_buf, char* block, int limit) {
    bk_strset(block, 0, limit);
    int blockload = 0;
    int retries = 0;
    int maxtries = 5;

    while (retries < maxtries && blockload < limit) {
        while (retries < maxtries && !socket_buf->available) {
            // todo: this entire routine can be rewritten to read directly into block and leaving
            // buffered data in the socket by reducing the read limit.
            socket_buf->available = socket_read(socket_buf);
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
