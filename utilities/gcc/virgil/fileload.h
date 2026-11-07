#pragma once

#include <fc_api.h>

FC_SAMS_BANKED(2, int, file_exists, (char* args), (args));
FC_SAMS_VOIDBANKED(2, file_load, (char* args), (args));

