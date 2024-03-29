#pragma once

#include <fc_api.h>

#define XMENU 60
#define XSTOP 68
#define XQUIT 75

#define BL 0xB6
#define BR 0xC7
#define BS 0xC4

FC_SAMS_VOIDBANKED(1, init_screen, (), ());
FC_SAMS_VOIDBANKED(1, screen_scroll_to, (int lineno), (lineno));
FC_SAMS_VOIDBANKED(1, screen_redraw, (), ());
FC_SAMS_VOIDBANKED(1, screen_status, (), ());
FC_SAMS_VOIDBANKED(1, screen_title, (), ());
FC_SAMS_VOIDBANKED(1, screen_prompt, (char* dst, char* prompt), (dst, prompt));
FC_SAMS_VOIDBANKED(1, screen_menu, (), ());

