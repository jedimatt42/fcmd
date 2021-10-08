
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
    // clear the line buffer.
    bk_strset(socket_buf->lastline, 0, 256);

    // read one char into lastline at a time
    char* onebyte = socket_buf->lastline;
    int space = 256;
    while((space > 0) && readstream(socket_buf, onebyte, 1)) {
        space--;
        if (*onebyte == 13) {
	    // peek to see if next is a linefeed
            int res = readstream(socket_buf, onebyte + 1, 1);
	    if (res) {
	      space--;
	    }
	    if (*(onebyte + 1) == 10) {
	      onebyte++;
	    }
	}
        if (*onebyte == 10) {
	   return socket_buf->lastline;
	}
	onebyte++;
    }
    return 0;
}

/*
    Fills a block with bytes from buffer, upto limit. Returns bytes read.

    socketBuffer->buffer = data taken from socket
    socketBuffer->available = bytes in buffer
    socketBuffer->lastline - not used, see readline
    socketbuffer->loaded = offset into buffer for next byte
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
