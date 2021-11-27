#pragma once

#include <fc_api.h>

void init_readline(int socket_id);

char* readline();

char* readbytes(int* len);
