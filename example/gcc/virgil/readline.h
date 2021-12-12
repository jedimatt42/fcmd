#pragma once

#include <fc_api.h>

FC_SAMS_VOIDBANKED(0, init_readline, (int socket_id), (socket_id));

FC_SAMS_BANKED(0, char*, readline, (), ());

FC_SAMS_BANKED(0, char*, readbytes, (int* len), (len));

