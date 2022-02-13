#pragma once

#include <fc_api.h>

FC_SAMS_VOIDBANKED(1, init_readline, (int socket_id), (socket_id));

FC_SAMS_BANKED(1, char*, readline, (), ());

FC_SAMS_BANKED(1, char*, readbytes, (int* len), (len));

FC_SAMS_BANKED(1, char*, readbytes_limit, (int* len, int limit), (len, limit));

