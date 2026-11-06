#ifndef _F18A_GPU_H
#define _F18A_GPU_H 1

#include "banking.h"

/*
 * The F18A firmware contains a small set of GPU routines in its private
 * GRAM at >4000. The routines use a parameter block in normal VRAM and a
 * vector table in the GPU's GRAM:
 *
 *   >3F00      vector index (high byte)
 *   >3F01      routine-specific flags
 *   >3F02-03   source address
 *   >3F04-05   destination address
 *   >3F06-07   byte count
 *
 * The dispatcher itself lives at >4026. Writing the GPU PC to that address
 * starts it, and the GPU returns to idle at the same address after the vector
 * returns. A further call can then be made by writing >01 to VR56.
 *
 * This interface is documented by the gpu_preload.asm source distributed with
 * the F18A firmware. It is not part of the F18A's host register interface, so
 * callers must leave the F18A unlocked.
 */

#define F18A_GPU_BLKCPY 0
#define F18A_GPU_FONTLD 1
#define F18A_GPU_GETINF 2
#define F18A_GPU_GETIDX 3
#define F18A_GPU_BLOBLD 4

#define F18A_GPU_PARAMS 0x3F00
#define F18A_GPU_FILLBYTE 0x3F10
#define F18A_GPU_DISPATCH 0x4026

/* Initialize the built-in dispatcher and leave the GPU idle at >4026. */
void f18a_gpu_init(void);

/* Run the routine selected by vector, using parameters already at >3F00. */
void f18a_gpu_call(int vector);

/* Stage a parameter block and run the routine selected by vector. */
void f18a_gpu_exec(int vector, int flags, unsigned int src, unsigned int dst,
                   unsigned int len);

/* Block copy: len bytes from src to dst. */
void f18a_gpu_blkcpy(unsigned int src, unsigned int dst, unsigned int len);

/* Block fill: repeat value into len bytes at dst. */
void f18a_gpu_blkfill(unsigned int dst, int value, unsigned int len);

#ifdef MYBANK
DECLARE_BANKED_VOID(f18a_gpu_init, BANK(7), bk_f18a_gpu_init, (), ())
DECLARE_BANKED_VOID(f18a_gpu_call, BANK(7), bk_f18a_gpu_call,
                    (int vector), (vector))
DECLARE_BANKED_VOID(f18a_gpu_exec, BANK(7), bk_f18a_gpu_exec,
                    (int vector, int flags, unsigned int src,
                     unsigned int dst, unsigned int len),
                    (vector, flags, src, dst, len))
DECLARE_BANKED_VOID(f18a_gpu_blkcpy, BANK(7), bk_f18a_gpu_blkcpy,
                    (unsigned int src, unsigned int dst, unsigned int len),
                    (src, dst, len))
DECLARE_BANKED_VOID(f18a_gpu_blkfill, BANK(7), bk_f18a_gpu_blkfill,
                    (unsigned int dst, int value, unsigned int len),
                    (dst, value, len))
#endif

#endif
