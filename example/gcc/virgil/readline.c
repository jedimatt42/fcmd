#include <fc_api.h>

#include "readline.h"
#include "gemini.h"

struct SocketBuffer socket_buf;

#define LASTLINE ((char*) 0xE000)

int vdp_read_status() {
  int status;
  __asm__( "movb @>8802,%0" : "=rm" (status) : : "r12" );
  return status;
}


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
	if (vdp_read_status()) {
	    process_input();
	}
    }
    return 0;
}
