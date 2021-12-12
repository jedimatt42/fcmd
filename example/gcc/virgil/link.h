#pragma once

#include <fc_api.h>

FC_SAMS_BANKED(1, char*, link_url, (char* line, int* length), (line, length));
FC_SAMS_BANKED(1, char*, link_label, (char* line, int* length), (line, length));
FC_SAMS_VOIDBANKED(1, update_full_url, (char* dst, char* url), (dst, url));
FC_SAMS_VOIDBANKED(1, normalize_url, (char* url), (url));
FC_SAMS_VOIDBANKED(1, set_hostname_and_port, (char* url, char* hostname, char* port), (url, hostname, port));

