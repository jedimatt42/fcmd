#include <fc_api.h>

#include "readline.h"

struct SocketBuffer socket_buf;

#define LASTLINE ((char*) 0xE000)

void init_readline(int socket_id) {
    fc_init_socket_buffer(&socket_buf, TLS, socket_id);
}

char* readline() {
    // clear the line buffer.
    int space = 4096;
    fc_strset(LASTLINE, 0, space);

    // read one char into lastline at a time
    char* onebyte = LASTLINE;
    while((space > 0) && fc_readstream(&socket_buf, onebyte, 1)) {
        space--;
        if (*onebyte == 13) {
	    // peek to see if next is a linefeed
            int res = fc_readstream(&socket_buf, onebyte + 1, 1);
	    if (res) {
	      space--;
	    }
	    if (*(onebyte + 1) == 10) {
	      onebyte++;
	    }
	}
        if (*onebyte == 10) {
	   return LASTLINE;
	}
	onebyte++;
    }
    return 0;
}

