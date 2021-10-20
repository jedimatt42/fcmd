#ifndef _SCREEN_H
#define _SCREEN_H 1

#include <fc_api.h>

#define XADDRESS 58
#define XSTOP (XADDRESS + 10)
#define XQUIT (XSTOP + 7)

#define BL 0xB6
#define BR 0xC7
#define BS 0xC4

void init_screen();
void screen_scroll_to(int lineno);
void screen_redraw();
void screen_status();
void screen_title();
void screen_prompt(char* dst, char* prompt);

extern struct DisplayInformation dinfo;

#endif

