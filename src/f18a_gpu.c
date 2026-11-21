#include "banks.h"
#define MYBANK BANK(7)

#include "f18a_gpu.h"
#include "detect_vdp.h"
#include "graphics.h"
#include <vdp.h>

extern const unsigned char gpu_base[];
extern const unsigned char gpu_boot[];

static unsigned int gpu_free;
static unsigned int gpu_entry;
static int gpu_base_ready;

#define GPU_BASE_CHECK_ADDR 0x3000

static void gpu_unlock(void) {
    /*
     * Clearing VR57 first makes the sequence deterministic. Writing >1C
     * twice only unlocks from a known-locked state; if the F18A is already
     * unlocked, the first >1C write re-locks it.
     */
    VDP_SET_REGISTER(0x39, 0x00);
    VDP_SET_REGISTER(0x39, 0x1C);
    VDP_SET_REGISTER(0x39, 0x1C);
}

static void gpu_trigger(unsigned int pc) {
    VDP_SET_REGISTER(0x36, (unsigned char)(pc >> 8));
    VDP_SET_REGISTER(0x37, (unsigned char)pc);
}

static void gpu_write_byte(unsigned int addr, unsigned char value) {
    VDP_SET_ADDRESS_WRITE(addr);
    VDPWD = value;
}

static unsigned char gpu_read_byte(unsigned int addr) {
    VDP_SET_ADDRESS(addr);
    return VDPRD;
}

static int gpu_wait_done(void) {
    unsigned int timeout = 0xFFFF;
    while (timeout--) {
        if (gpu_read_byte(GPU_DONE) == 0xFF) {
            return 1;
        }
    }
    return 0;
}

int f18a_gpu_blkcpy_raw(unsigned int src, unsigned int dst,
                        unsigned int len, int flags) {
    VDP_SET_ADDRESS_WRITE(GPU_PARAMS);
    VDPWD = 0;                          /* unused */
    VDPWD = (unsigned char)flags;
    VDPWD = (unsigned char)(src >> 8);
    VDPWD = (unsigned char)src;
    VDPWD = (unsigned char)(dst >> 8);
    VDPWD = (unsigned char)dst;
    VDPWD = (unsigned char)(len >> 8);
    VDPWD = (unsigned char)len;

    gpu_write_byte(GPU_DONE, 0);
    gpu_unlock();
    gpu_trigger(GPU_HOST_BLKCPY);
    return gpu_wait_done() ? GFX_OK : GFX_ERR_BUSY;
}

static int gpu_check_base(void) {
    unsigned char check[4];
    if (f18a_gpu_blkcpy_raw(GPU_BASE_ADDR, GPU_BASE_CHECK_ADDR, 4, 0) != GFX_OK) {
        return 0;
    }
    vdpmemread(GPU_BASE_CHECK_ADDR, (char*)check, 4);
    return check[0] == 0x03 && check[1] == 0x40 &&
           check[2] == 0xFC && check[3] == 0x01;
}

static int gpu_upload_base(void) {
    vdpmemcpy(GPU_BASE_CHECK_ADDR, (const char*)gpu_base, GPU_BASE_LEN);
    vdpmemcpy(GPU_BOOT_ADDR, (const char*)gpu_boot, GPU_BOOT_LEN);

    gpu_write_byte(GPU_DONE, 0);
    gpu_unlock();
    gpu_trigger(GPU_BOOT_ADDR);
    if (!gpu_wait_done()) {
        return 0;
    }
    return gpu_check_base();
}

int f18a_gpu_init(void) {
    if (vdp_type != VDP_F18A) {
        return GFX_ERR_UNSUPPORTED;
    }

    gpu_unlock();

    if (gpu_base_ready) {
        if (gpu_check_base()) {
            if (gpu_free == 0) {
                gpu_free = GPU_FREE_INIT;
            }
            return GFX_OK;
        }
        /* The base is gone; any active executable GPU code is invalid too. */
        gpu_base_ready = 0;
        gpu_free = 0;
        gpu_entry = 0;
    }

    if (!gpu_upload_base()) {
        gpu_base_ready = 0;
        gpu_free = 0;
        gpu_entry = 0;
        return GFX_ERR_BUSY;
    }

    if (gpu_free == 0) {
        gpu_free = GPU_FREE_INIT;
        gpu_entry = 0;
    }
    gpu_base_ready = 1;
    return GFX_OK;
}

int f18a_gpu_ready(void) {
    int result;

    if (vdp_type != VDP_F18A) {
        return GFX_ERR_UNSUPPORTED;
    }
    if ((nTextFlags & (TEXT_FLAG_IS_F18A | TEXT_HEIGHT_30)) !=
        (TEXT_FLAG_IS_F18A | TEXT_HEIGHT_30)) {
        return GFX_ERR_WRONG_MODE;
    }
    if (!gpu_base_ready) {
        result = f18a_gpu_init();
        if (result != GFX_OK) {
            return result;
        }
    }
    return GFX_OK;
}

unsigned int f18a_gpu_get_free(void) {
    return gpu_free;
}

unsigned int f18a_gpu_get_entry(void) {
    return gpu_entry;
}

int gpu_push(unsigned int vram_src, unsigned int len, struct GpuFrame* frame) {
    int result;

    if (!frame) return GFX_ERR_INVALID;

    frame->free_addr = gpu_free;
    frame->entry = gpu_entry;

    /*
     * The loader may run before the executable selects an 80x30 mode. Verify
     * or install the base now; the blob survives a later mode switch.
     */
    result = f18a_gpu_init();
    if (result != GFX_OK) {
        gpu_entry = 0;
        return result;
    }
    if (len == 0) {
        gpu_entry = 0;
        return GFX_ERR_INVALID;
    }
    if (vram_src >= 0x4000 || len > (unsigned int)(0x4000 - vram_src)) {
        gpu_entry = 0;
        return GFX_ERR_RANGE;
    }
    if (gpu_free < GPU_FREE_INIT || gpu_free > GPU_STACK_LIMIT ||
        len > (unsigned int)(GPU_STACK_LIMIT - gpu_free)) {
        gpu_entry = 0;
        return GFX_ERR_NO_VRAM;
    }

    result = f18a_gpu_blkcpy_raw(vram_src, gpu_free, len, 0);
    if (result != GFX_OK) {
        gpu_entry = 0;
        return result;
    }

    gpu_entry = gpu_free;
    gpu_free += (len + 1) & ~1;

    /*
     * Only the managed 80x30 mode expects the F18A to stay unlocked. In any
     * other mode restore the lock state the mode setup selected.
     */
    if ((nTextFlags & (TEXT_FLAG_IS_F18A | TEXT_HEIGHT_30)) !=
        (TEXT_FLAG_IS_F18A | TEXT_HEIGHT_30)) {
        VDP_SET_REGISTER(0x39, 0x00);
    }
    return GFX_OK;
}

void gpu_pop(struct GpuFrame* frame) {
    if (!frame) return;
    gpu_free = frame->free_addr;
    gpu_entry = frame->entry;
}

void f18a_gpu_blkcpy(unsigned int src, unsigned int dst, unsigned int len) {
    if (gpu_base_ready) {
        f18a_gpu_blkcpy_raw(src, dst, len, 0);
    }
}

void f18a_gpu_blkfill(unsigned int dst, int value, unsigned int len) {
    if (gpu_base_ready) {
        gpu_write_byte(GPU_FILLBYTE, (unsigned char)value);
        f18a_gpu_blkcpy_raw(GPU_FILLBYTE, dst, len, 1);
    }
}
