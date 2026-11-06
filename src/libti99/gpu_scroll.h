#ifndef _GPU_SCROLL_H
#define _GPU_SCROLL_H 1

#include "banking.h"

void gpu_scroll_80color();

DECLARE_BANKED_VOID(gpu_scroll_80color, BANK(7), bk_gpu_scroll_80color, (), ())

#endif
