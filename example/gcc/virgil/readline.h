#ifndef _READLINE_H 
#define _READLINE_H 1

#include <fc_api.h>

void init_readline(int socket_id);

char* readline();

#endif
