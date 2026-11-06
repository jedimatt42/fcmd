#include "banks.h"
#define MYBANK BANK(7)

#include "f18a_gpu.h"
#include "detect_vdp.h"
#include "graphics.h"
#include <vdp.h>

/* Select the F18A GPU status register while running a built-in routine. */
#define F18A_GPU_STATUS_REG 0x0f
#define F18A_GPU_STATUS_SR2 2
#define F18A_GPU_RUNNING 0x80

/* VR54/VR55 hold the GPU program counter. */
#define F18A_GPU_PC_MSB 0x36
#define F18A_GPU_PC_LSB 0x37

/* Writing >01 to VR56 triggers the GPU at its current program counter. */
#define F18A_GPU_TRIGGER 0x38

/* The built-in routines only address the normal 16 KB of VRAM. */
#define F18A_GPU_VRAM_END 0x4000

static int f18a_gpu_ready(void) {
    if (vdp_type != VDP_F18A) return GFX_ERR_UNSUPPORTED;
    if ((nTextFlags & (TEXT_FLAG_IS_F18A | TEXT_HEIGHT_30)) !=
        (TEXT_FLAG_IS_F18A | TEXT_HEIGHT_30)) {
        return GFX_ERR_WRONG_MODE;
    }
    return GFX_OK;
}

static int f18a_gpu_valid_range(unsigned int addr, unsigned int len) {
    /* Keep all comparisons in the positive signed range. BLKCPY would otherwise
       wrap a 16-bit count or run into GPU GRAM above >3FFF. */
    if (len == 0 || (len & 0x8000)) return GFX_ERR_RANGE;
    if (addr & 0x8000) return GFX_ERR_RANGE;
    if (addr >= F18A_GPU_VRAM_END) return GFX_ERR_RANGE;
    if (len > (unsigned int)(F18A_GPU_VRAM_END - addr)) return GFX_ERR_RANGE;
    return GFX_OK;
}

static int f18a_gpu_valid_copy(unsigned int src, unsigned int dst,
                               unsigned int len) {
    int result = f18a_gpu_valid_range(src, len);
    if (result != GFX_OK) return result;
    return f18a_gpu_valid_range(dst, len);
}

static void f18a_gpu_wait(void) {
    VDP_SET_REGISTER(F18A_GPU_STATUS_REG, F18A_GPU_STATUS_SR2);
    while (VDPST & F18A_GPU_RUNNING) {
    }
    VDP_SET_REGISTER(F18A_GPU_STATUS_REG, 0);
}

static void f18a_gpu_set_params(int vector, int flags, unsigned int src,
                                unsigned int dst, unsigned int len) {
    VDP_SET_ADDRESS_WRITE(F18A_GPU_PARAMS);
    VDPWD = (unsigned char)vector;
    VDPWD = (unsigned char)flags;
    VDPWD = (unsigned char)(src >> 8);
    VDPWD = (unsigned char)src;
    VDPWD = (unsigned char)(dst >> 8);
    VDPWD = (unsigned char)dst;
    VDPWD = (unsigned char)(len >> 8);
    VDPWD = (unsigned char)len;
}

static void f18a_gpu_trigger(void) {
    VDP_SET_REGISTER(F18A_GPU_TRIGGER, 0x01);
    f18a_gpu_wait();
}

static void f18a_gpu_exec_raw(int vector, int flags, unsigned int src,
                              unsigned int dst, unsigned int len) {
    f18a_gpu_set_params(vector, flags, src, dst, len);
    f18a_gpu_trigger();
}

void f18a_gpu_init(void) {
    /* Start with a harmless vector in case the GPU executes the dispatcher
       before the caller has prepared a request. */
    f18a_gpu_exec_raw(F18A_GPU_GETINF, 0, 0, 0, 0);
    VDP_SET_REGISTER(F18A_GPU_PC_MSB, (unsigned char)(F18A_GPU_DISPATCH >> 8));
    VDP_SET_REGISTER(F18A_GPU_PC_LSB, (unsigned char)F18A_GPU_DISPATCH);
    f18a_gpu_wait();
}

int f18a_gpu_info(struct F18aGpuInfo* info) {
    char buf[8];
    int result;

    if (!info) return GFX_ERR_INVALID;
    result = f18a_gpu_ready();
    if (result != GFX_OK) return result;

    f18a_gpu_exec_raw(F18A_GPU_GETINF, 0, 0, 0, 0);
    vdpmemread(F18A_GPU_PARAMS, buf, 8);
    info->version = (unsigned char)buf[0];
    info->free_addr = ((unsigned char)buf[2] << 8) | (unsigned char)buf[3];
    info->vector_table = ((unsigned char)buf[4] << 8) | (unsigned char)buf[5];
    info->user_vector_table = ((unsigned char)buf[6] << 8) | (unsigned char)buf[7];
    return GFX_OK;
}

int f18a_gpu_copy(unsigned int src, unsigned int dst, unsigned int len) {
    int result = f18a_gpu_ready();
    if (result != GFX_OK) return result;
    result = f18a_gpu_valid_copy(src, dst, len);
    if (result != GFX_OK) return result;
    f18a_gpu_exec_raw(F18A_GPU_BLKCPY, 0, src, dst, len);
    return GFX_OK;
}

int f18a_gpu_fill(unsigned int dst, int value, unsigned int len) {
    int result = f18a_gpu_ready();
    if (result != GFX_OK) return result;
    if (value < 0 || value > 255) return GFX_ERR_RANGE;
    result = f18a_gpu_valid_range(dst, len);
    if (result != GFX_OK) return result;

    VDP_SET_ADDRESS_WRITE(F18A_GPU_FILLBYTE);
    VDPWD = (unsigned char)value;
    f18a_gpu_exec_raw(F18A_GPU_BLKCPY, 1, F18A_GPU_FILLBYTE, dst, len);
    return GFX_OK;
}

int f18a_gpu_exec(int vector, int flags, unsigned int src, unsigned int dst,
                  unsigned int len) {
    int result = f18a_gpu_ready();
    if (result != GFX_OK) return result;
    if (vector < 0 || vector > F18A_GPU_BLOBLD) return GFX_ERR_INVALID;
    if (vector == F18A_GPU_BLKCPY) {
        result = f18a_gpu_valid_copy(src, dst, len);
        if (result != GFX_OK) return result;
    }
    f18a_gpu_exec_raw(vector, flags, src, dst, len);
    return GFX_OK;
}

void f18a_gpu_blkcpy(unsigned int src, unsigned int dst, unsigned int len) {
    /* BLKCPY flag 0: increment the source address (copy). */
    f18a_gpu_exec_raw(F18A_GPU_BLKCPY, 0, src, dst, len);
}

void f18a_gpu_blkfill(unsigned int dst, int value, unsigned int len) {
    /* BLKCPY flag != 0: read once from the fixed source and repeat it. */
    VDP_SET_ADDRESS_WRITE(F18A_GPU_FILLBYTE);
    VDPWD = (unsigned char)value;
    f18a_gpu_exec_raw(F18A_GPU_BLKCPY, 1, F18A_GPU_FILLBYTE, dst, len);
}
