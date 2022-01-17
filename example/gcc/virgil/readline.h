#pragma once

#include <fc_api.h>

#define _READLINE_PAGE 1

FC_SAMS_VOIDBANKED(_READLINE_PAGE, init_readline, (int socket_id), (socket_id));

FC_SAMS_BANKED(_READLINE_PAGE, char*, readline, (), ());

FC_SAMS_BANKED(_READLINE_PAGE, char*, readbytes, (int* len), (len));

