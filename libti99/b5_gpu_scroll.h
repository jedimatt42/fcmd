#ifndef _GPU_SCROLL_H
#define _GPU_SCROLL_H 1

#include "banking.h"

void install_gpu_scroll();

DECLARE_BANKED_VOID(install_gpu_scroll, BANK(5), bk_install_gpu_scroll, (), ())


#endif