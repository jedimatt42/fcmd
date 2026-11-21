#ifndef _F18A_GPU_H
#define _F18A_GPU_H 1

#include "banking.h"

/*
 * ForceCommand owns the F18A GPU GRAM. The base image is copied to >4000
 * after F18A detection and 80x30 mode setup.
 *
 *   >4000  IDLE
 *   >4002  >FC01 magic
 *   >4004  reserved
 *   >4006  host BLKCPY entry (sets done, idles)
 *   >4014  GPU-callable BLKCPY entry (returns via R11)
 *   >4018  BLKCPY core
 *   >403C  first free address
 *   >4700-47FF  GPU stack reserve, R15 = >47FE
 */
#define GPU_BASE_ADDR     0x4000
#define GPU_BASE_MAGIC    0xFC01
#define GPU_HOST_BLKCPY   0x4006
#define GPU_CALL_BLKCPY   0x4014
#define GPU_BLKCPY_CORE   0x4018
#define GPU_FREE_INIT     0x403C
#define GPU_BASE_LEN      0x003C
#define GPU_BOOT_ADDR     0x3E00
#define GPU_BOOT_LEN      0x0020
#define GPU_STACK_TOP     0x47FE
#define GPU_STACK_LIMIT   0x4700
#define GPU_MAX_BLOB      (GPU_STACK_LIMIT - GPU_FREE_INIT)

/*
 * CPU-accessible GPU mailbox in VRAM.
 *
 *   >3F00        unused
 *   >3F01        BLKCPY flags / app function ID
 *   >3F02-0D     args / BLKCPY source, destination, length
 *   >3F0E        done: 0 = busy, >FF = done
 *   >3F0F        status: 0 = OK
 *   >3F10        BLKCPY fill byte / scratch
 *   >3F1E-1F     app blob base (R14)
 *   >3F20-2F     return block
 */
#define GPU_PARAMS        0x3F00
#define GPU_FLAGS         0x3F01
#define GPU_ARG0          0x3F02
#define GPU_ARG1          0x3F04
#define GPU_ARG2          0x3F06
#define GPU_ARG3          0x3F08
#define GPU_ARG4          0x3F0A
#define GPU_ARG5          0x3F0C
#define GPU_DONE          0x3F0E
#define GPU_STATUS        0x3F0F
#define GPU_FILLBYTE      0x3F10
#define GPU_APP_BASE      0x3F1E
#define GPU_RETURN        0x3F20

/* The only built-in base vector is BLKCPY. */
#define F18A_GPU_BLKCPY   0

/* Public API results use the graphics error codes from graphics.h. */
struct F18aGpuInfo {
    int version;
    unsigned int free_addr;
    unsigned int vector_table;
    unsigned int user_vector_table;
};

/*
 * Argument block for a compiled GPU routine. func is copied to >3F01;
 * arg[0..5] to >3F02-0D; result[0..5] is read from >3F20-2F.
 */
struct GpuCall {
    unsigned char func;
    unsigned char status;
    unsigned int arg[6];
    unsigned int result[6];
};

/* A saved GPU allocation frame: free pointer and current app entry. */
struct GpuFrame {
    unsigned int free_addr;
    unsigned int entry;
};

/* Public API: callable from external programs through the ForceCommand API. */
int f18a_gpu_info(struct F18aGpuInfo* info);
int f18a_gpu_copy(unsigned int src, unsigned int dst, unsigned int len);
int f18a_gpu_fill(unsigned int dst, int value, unsigned int len);
int f18a_gpu_exec(int vector, int flags, unsigned int src, unsigned int dst,
                  unsigned int len);
int f18a_gpu_call(struct GpuCall* call);

/* Internal support in bank 7. */
int f18a_gpu_init(void);
int f18a_gpu_ready(void);
int f18a_gpu_blkcpy_raw(unsigned int src, unsigned int dst, unsigned int len,
                        int flags);
unsigned int f18a_gpu_get_free(void);
unsigned int f18a_gpu_get_entry(void);
int gpu_push(unsigned int vram_src, unsigned int len, struct GpuFrame* frame);
void gpu_pop(struct GpuFrame* frame);
void f18a_gpu_blkcpy(unsigned int src, unsigned int dst, unsigned int len);
void f18a_gpu_blkfill(unsigned int dst, int value, unsigned int len);

#ifdef MYBANK
/* Public API lives in bank 0, which is the API bank. */
DECLARE_BANKED(f18a_gpu_info, BANK(0), int, bk_f18a_gpu_info, (struct F18aGpuInfo* info), (info))
DECLARE_BANKED(f18a_gpu_copy, BANK(0), int, bk_f18a_gpu_copy, (unsigned int src, unsigned int dst, unsigned int len), (src, dst, len))
DECLARE_BANKED(f18a_gpu_fill, BANK(0), int, bk_f18a_gpu_fill, (unsigned int dst, int value, unsigned int len), (dst, value, len))
DECLARE_BANKED(f18a_gpu_exec, BANK(0), int, bk_f18a_gpu_exec, (int vector, int flags, unsigned int src, unsigned int dst, unsigned int len), (vector, flags, src, dst, len))
DECLARE_BANKED(f18a_gpu_call, BANK(0), int, bk_f18a_gpu_call, (struct GpuCall* call), (call))

/* Internal helpers live in bank 7. */
DECLARE_BANKED(f18a_gpu_init, BANK(7), int, bk_f18a_gpu_init, (), ())
DECLARE_BANKED(f18a_gpu_ready, BANK(7), int, bk_f18a_gpu_ready, (), ())
DECLARE_BANKED(f18a_gpu_blkcpy_raw, BANK(7), int, bk_f18a_gpu_blkcpy_raw, (unsigned int src, unsigned int dst, unsigned int len, int flags), (src, dst, len, flags))
DECLARE_BANKED(f18a_gpu_get_free, BANK(7), unsigned int, bk_f18a_gpu_get_free, (), ())
DECLARE_BANKED(f18a_gpu_get_entry, BANK(7), unsigned int, bk_f18a_gpu_get_entry, (), ())
DECLARE_BANKED(gpu_push, BANK(7), int, bk_gpu_push, (unsigned int vram_src, unsigned int len, struct GpuFrame* frame), (vram_src, len, frame))
DECLARE_BANKED_VOID(gpu_pop, BANK(7), bk_gpu_pop, (struct GpuFrame* frame), (frame))
DECLARE_BANKED_VOID(f18a_gpu_blkcpy, BANK(7), bk_f18a_gpu_blkcpy, (unsigned int src, unsigned int dst, unsigned int len), (src, dst, len))
DECLARE_BANKED_VOID(f18a_gpu_blkfill, BANK(7), bk_f18a_gpu_blkfill, (unsigned int dst, int value, unsigned int len), (dst, value, len))
#endif

#endif
