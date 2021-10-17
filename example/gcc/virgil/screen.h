#ifndef _SCREEN_H
#define _SCREEN_H 1

#include <fc_api.h>

void init_screen();
void screen_scroll_to(int lineno);
void screen_redraw();
void screen_status();
void screen_title();

extern struct DisplayInformation dinfo;

#endif

