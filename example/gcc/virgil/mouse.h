#pragma once

#include <fc_api.h>

#define MP_NORMAL 0
#define MP_WAIT 1
#define MP_SCROLL_DOWN 2
#define MP_SCROLL_UP 3
#define MP_PAGE_DOWN 4
#define MP_PAGE_UP 5
#define MP_BUSY 6

extern int mouse_active;

void init_mouse();

extern struct MouseData md;

int update_mouse();

void handle_mouse_click();

void mouse_set_pointer(int p);

