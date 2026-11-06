#include "banks.h"
#define MYBANK BANK(7)

#include "f18a_gpu.h"
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

void f18a_gpu_init(void) {
    /* Start with a harmless vector in case the GPU executes the dispatcher
       before the caller has prepared a request. */
    f18a_gpu_set_params(F18A_GPU_GETINF, 0, 0, 0, 0);
    VDP_SET_REGISTER(F18A_GPU_PC_MSB, (unsigned char)(F18A_GPU_DISPATCH >> 8));
    VDP_SET_REGISTER(F18A_GPU_PC_LSB, (unsigned char)F18A_GPU_DISPATCH);
    f18a_gpu_wait();
}

void f18a_gpu_call(int vector) {
    VDP_SET_ADDRESS_WRITE(F18A_GPU_PARAMS);
    VDPWD = (unsigned char)vector;
    f18a_gpu_trigger();
}

void f18a_gpu_exec(int vector, int flags, unsigned int src,
                   unsigned int dst, unsigned int len) {
    f18a_gpu_set_params(vector, flags, src, dst, len);
    f18a_gpu_trigger();
}

void f18a_gpu_blkcpy(unsigned int src, unsigned int dst, unsigned int len) {
    /* BLKCPY flag 0: increment the source address (copy). */
    f18a_gpu_exec(F18A_GPU_BLKCPY, 0, src, dst, len);
}

void f18a_gpu_blkfill(unsigned int dst, int value, unsigned int len) {
    /* BLKCPY flag != 0: read once from the fixed source and repeat it. */
    VDP_SET_ADDRESS_WRITE(F18A_GPU_FILLBYTE);
    VDPWD = (unsigned char)value;
    f18a_gpu_exec(F18A_GPU_BLKCPY, 1, F18A_GPU_FILLBYTE, dst, len);
}
