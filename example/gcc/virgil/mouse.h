#ifndef _MOUSE_H
#define _MOUSE_H 1

#include <fc_api.h>

void init_mouse();

extern struct MouseData* mouseData;

int update_mouse();

#endif

