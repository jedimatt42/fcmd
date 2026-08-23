#ifndef VDP_INTERNAL_H
#define VDP_INTERNAL_H 1

#include "vdp.h"

/* Called after a mode initializer has completed successfully. */
void vdp_screen_mode_changed(int mode);
void vdp_screen_mode_changed_ex(int mode, unsigned int pab,
                                unsigned int buffer,
                                unsigned int file_buffer,
                                unsigned int file_buffer_size);

#ifdef MYBANK
#include "banking.h"
DECLARE_BANKED_VOID(vdp_screen_mode_changed_ex, BANK(8),
                    bk_vdp_screen_mode_changed_ex,
                    (int mode, unsigned int pab, unsigned int buffer,
                     unsigned int file_buffer,
                     unsigned int file_buffer_size),
                    (mode, pab, buffer, file_buffer, file_buffer_size))
DECLARE_BANKED_VOID(lock_f18a, BANK(8), bk_lock_f18a, (), ())
DECLARE_BANKED_VOID(unlock_f18a, BANK(8), bk_unlock_f18a, (), ())
#endif

#endif
