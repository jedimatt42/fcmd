#pragma once

#include <fc_api.h>

FC_SAMS_BANKED(1, char*, link_url_scheme, (char* line, int* length), (line, length));
FC_SAMS_BANKED(1, char*, link_label, (char* line, int* length), (line, length));
FC_SAMS_VOIDBANKED(1, update_full_url, (char* dst, char* url), (dst, url));
FC_SAMS_VOIDBANKED(1, normalize_url, (char* url), (url));
FC_SAMS_VOIDBANKED(1, set_hostname_and_port, (char* url, char* hostname, char* port), (url, hostname, port));
FC_SAMS_VOIDBANKED(1, link_set_url, (char* dst, int line_id), (dst, line_id));

