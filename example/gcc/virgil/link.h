#ifndef _LINK_H
#define _LINK_H 1

char* link_url(char* line, int* length);
char* link_label(char* line, int* length);
void update_full_url(char* dst, char* url);
void set_hostname_and_port(char* url, char* hostname, char* port);

#endif

