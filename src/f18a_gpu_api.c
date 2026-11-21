#include "banks.h"
#define MYBANK BANK(0)

#include "f18a_gpu.h"
#include "graphics.h"
#include <vdp.h>

static void api_unlock(void) {
    /* Clear VR57 first; >1C twice only unlocks from a known-locked state. */
    VDP_SET_REGISTER(0x39, 0x00);
    VDP_SET_REGISTER(0x39, 0x1C);
    VDP_SET_REGISTER(0x39, 0x1C);
}

static void api_trigger(unsigned int pc) {
    VDP_SET_REGISTER(0x36, (unsigned char)(pc >> 8));
    VDP_SET_REGISTER(0x37, (unsigned char)pc);
}

static void api_write_byte(unsigned int addr, unsigned char value) {
    VDP_SET_ADDRESS_WRITE(addr);
    VDPWD = value;
}

static unsigned char api_read_byte(unsigned int addr) {
    VDP_SET_ADDRESS(addr);
    return VDPRD;
}

static int api_wait_done(void) {
    unsigned int timeout = 0xFFFF;
    while (timeout--) {
        if (api_read_byte(GPU_DONE) == 0xFF) {
            return 1;
        }
    }
    return 0;
}

static int f18a_gpu_valid_range(unsigned int addr, unsigned int len) {
    if (len == 0 || (len & 0x8000)) return GFX_ERR_RANGE;
    if (addr & 0x8000) return GFX_ERR_RANGE;
    if (addr >= 0x4000) return GFX_ERR_RANGE;
    if (len > (unsigned int)(0x4000 - addr)) return GFX_ERR_RANGE;
    return GFX_OK;
}

static int f18a_gpu_valid_copy(unsigned int src, unsigned int dst,
                               unsigned int len) {
    int result = f18a_gpu_valid_range(src, len);
    if (result != GFX_OK) return result;
    return f18a_gpu_valid_range(dst, len);
}

int f18a_gpu_info(struct F18aGpuInfo* info) {
    int result;
    if (!info) return GFX_ERR_INVALID;
    result = bk_f18a_gpu_ready();
    if (result != GFX_OK) return result;

    info->version = GPU_BASE_MAGIC;
    info->free_addr = bk_f18a_gpu_get_free();
    info->vector_table = GPU_HOST_BLKCPY;
    info->user_vector_table = bk_f18a_gpu_get_entry();
    return GFX_OK;
}

int f18a_gpu_copy(unsigned int src, unsigned int dst, unsigned int len) {
    int result = bk_f18a_gpu_ready();
    if (result != GFX_OK) return result;
    result = f18a_gpu_valid_copy(src, dst, len);
    if (result != GFX_OK) return result;
    return bk_f18a_gpu_blkcpy_raw(src, dst, len, 0);
}

int f18a_gpu_fill(unsigned int dst, int value, unsigned int len) {
    int result = bk_f18a_gpu_ready();
    if (result != GFX_OK) return result;
    if (value < 0 || value > 255) return GFX_ERR_RANGE;
    result = f18a_gpu_valid_range(dst, len);
    if (result != GFX_OK) return result;

    api_write_byte(GPU_FILLBYTE, (unsigned char)value);
    return bk_f18a_gpu_blkcpy_raw(GPU_FILLBYTE, dst, len, 1);
}

int f18a_gpu_exec(int vector, int flags, unsigned int src, unsigned int dst,
                  unsigned int len) {
    int result = bk_f18a_gpu_ready();
    if (result != GFX_OK) return result;
    if (vector != F18A_GPU_BLKCPY) return GFX_ERR_UNSUPPORTED;
    result = f18a_gpu_valid_copy(src, dst, len);
    if (result != GFX_OK) return result;
    return bk_f18a_gpu_blkcpy_raw(src, dst, len, flags);
}

int f18a_gpu_call(struct GpuCall* call) {
    int result;
    int i;
    unsigned int entry;

    if (!call) return GFX_ERR_INVALID;
    result = bk_f18a_gpu_ready();
    if (result != GFX_OK) return result;

    entry = bk_f18a_gpu_get_entry();
    if (entry == 0) return GFX_ERR_UNSUPPORTED;

    VDP_SET_ADDRESS_WRITE(GPU_FLAGS);
    VDPWD = call->func;
    for (i = 0; i < 6; i++) {
        VDPWD = (unsigned char)(call->arg[i] >> 8);
        VDPWD = (unsigned char)call->arg[i];
    }

    api_write_byte(GPU_APP_BASE, (unsigned char)(entry >> 8));
    api_write_byte(GPU_APP_BASE + 1, (unsigned char)entry);
    api_write_byte(GPU_DONE, 0);
    api_write_byte(GPU_STATUS, 0);

    api_unlock();
    api_trigger(entry);
    if (!api_wait_done()) {
        return GFX_ERR_BUSY;
    }

    call->status = api_read_byte(GPU_STATUS);
    VDP_SET_ADDRESS(GPU_RETURN);
    for (i = 0; i < 6; i++) {
        unsigned int hi = VDPRD;
        unsigned int lo = VDPRD;
        call->result[i] = (hi << 8) | lo;
    }

    return call->status ? call->status : GFX_OK;
}
