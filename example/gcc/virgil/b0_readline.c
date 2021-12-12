#include <fc_api.h>

#include "readline.h"
#include "gemini.h"

struct SocketBuffer socket_buf;

#define LASTLINE ((char*) 0xE000)


void FC_SAMS(0,init_readline(int socket_id)) {
    fc_init_socket_buffer(&socket_buf, TLS, socket_id);
}

char* FC_SAMS(0,readline()) {
    // clear the line buffer.
    int space = 4096;

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
	    *(onebyte + 1) = 0;
	    return LASTLINE;
	}
	onebyte++;
	if (vdp_read_status()) {
	    process_input();
	}
    }
    if (onebyte != LASTLINE) {
      *(onebyte + 1) = 0;
      return LASTLINE;
    }
    return 0;
}

char* FC_SAMS(0,readbytes(int* len)) {
    *len = fc_readstream(&socket_buf, LASTLINE, 512);
    return LASTLINE;
}

