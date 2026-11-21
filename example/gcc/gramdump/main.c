#include <fc_api.h>
#include <ioports.h>

/*
 * GRAMDUMP - inspect and validate ForceCommand's F18A GPU base image.
 *
 * The host cannot read GRAM directly, so a 20-byte GPU routine is uploaded to
 * VRAM >3F20.  It copies GRAM >4000 to VRAM >3000, where the CPU can read it
 * through the normal data port.
 *
 * The first dump is taken before f18a_gpu_info() is called.  Calling the API
 * installs the base if it is missing, so the second dump confirms the upload.
 *
 * Run this in ForceCommand's F18A 80x30 mode.  The report stays inside one
 * screen so it does not exercise the scroll path it is meant to validate.
 *
 * GPU routine at >3F20:
 *
 *      AORG >3F20
 *      MOV  @>3F40,R0      ; source
 *      MOV  @>3F42,R1      ; destination
 *      MOV  @>3F44,R2      ; byte count
 * LOOP MOVB *R0+,*R1+
 *      DEC  R2
 *      JNE  LOOP
 *      IDLE
 */

#define GPU_CODE_ADDR   0x3F20u
#define GPU_PARAM_ADDR  0x3F40u
#define DUMP_VRAM_ADDR  0x3000u
#define GRAM_ADDR       0x4000u
#define DUMP_SIZE       0x0200u
#define PRINT_BYTES     96u
#define BYTES_PER_LINE  24

#define ROW_TITLE       1
#define ROW_STATE       2
#define ROW_DUMP        4
#define ROW_INFO        9
#define ROW_INFO2       10
#define ROW_BASE        11
#define ROW_BLK         12
#define ROW_DONE        14

static const unsigned char gpu_code[] = {
    0xC0, 0x20, 0x3F, 0x40,     /* MOV  @>3F40,R0 */
    0xC0, 0x60, 0x3F, 0x42,     /* MOV  @>3F42,R1 */
    0xC0, 0xA0, 0x3F, 0x44,     /* MOV  @>3F44,R2 */
    0xDC, 0x70,                 /* MOVB *R0+,*R1+ */
    0x06, 0x02,                 /* DEC  R2 */
    0x16, 0xFD,                 /* JNE  loop */
    0x03, 0x40                  /* IDLE */
};

static unsigned char gram[DUMP_SIZE];

static void put_hex_byte(unsigned char b) {
    static const char hex[] = "0123456789ABCDEF";
    term_putc(hex[b >> 4]);
    term_putc(hex[b & 0x0F]);
}

static void put_hex_word(unsigned int w) {
    put_hex_byte((unsigned char)(w >> 8));
    put_hex_byte((unsigned char)w);
}

static void put_addr(unsigned int a) {
    term_putc('>');
    put_hex_word(a);
}

/* Writing the PC LSB triggers the GPU at the address in VR54/VR55. */
static void gpu_trigger_at(unsigned int pc) {
    VDP_SET_REGISTER(0x36, (unsigned char)(pc >> 8));
    VDP_SET_REGISTER(0x37, (unsigned char)pc);
}

/*
 * Wait for the GPU by delay rather than by selecting status register 2
 * through VR15.  A 512-byte GPU copy finishes in well under a frame; this
 * delay is several frames on a 3 MHz 9900.
 */
static int gpu_wait(void) {
    volatile unsigned int i;
    for (i = 0; i < 0xFFFF; i++) {
    }
    return 1;
}

static int upload_gpu_code(void) {
    unsigned char check[sizeof(gpu_code)];
    unsigned int i;

    VDP_SET_ADDRESS_WRITE(GPU_CODE_ADDR);
    for (i = 0; i < sizeof(gpu_code); i++) {
        VDPWD = gpu_code[i];
    }

    /* If the data port or address increment is wrong, the GPU would run
       garbage.  Refuse to trigger it in that case. */
    vdp_memread(GPU_CODE_ADDR, (char*)check, sizeof(check));
    for (i = 0; i < sizeof(gpu_code); i++) {
        if (check[i] != gpu_code[i]) {
            return 0;
        }
    }
    return 1;
}

/* Copy one block of GRAM to VRAM >3000 through the uploaded GPU code. */
static int copy_gram_chunk(unsigned int src, unsigned int len) {
    unsigned char check[6];

    VDP_SET_ADDRESS_WRITE(GPU_PARAM_ADDR);
    VDPWD = (unsigned char)(src >> 8);
    VDPWD = (unsigned char)src;
    VDPWD = (unsigned char)(DUMP_VRAM_ADDR >> 8);
    VDPWD = (unsigned char)DUMP_VRAM_ADDR;
    VDPWD = (unsigned char)(len >> 8);
    VDPWD = (unsigned char)len;

    vdp_memread(GPU_PARAM_ADDR, (char*)check, 6);
    if (check[0] != (unsigned char)(src >> 8) ||
        check[1] != (unsigned char)src ||
        check[2] != (unsigned char)(DUMP_VRAM_ADDR >> 8) ||
        check[3] != (unsigned char)DUMP_VRAM_ADDR ||
        check[4] != (unsigned char)(len >> 8) ||
        check[5] != (unsigned char)len) {
        return 0;
    }

    gpu_trigger_at(GPU_CODE_ADDR);
    return gpu_wait();
}

/* Read the first DUMP_SIZE bytes of GRAM into gram[]. */
static int read_gram(void) {
    unsigned int chunk;
    unsigned int i;

    /* Marker in the transfer buffer.  If the GPU never runs, this stays
       0xAA and the dump is stale VRAM, not GRAM. */
    vdp_memset(DUMP_VRAM_ADDR, 0xAA, DUMP_SIZE);

    for (chunk = 0; chunk < DUMP_SIZE; chunk += 0x100) {
        if (!copy_gram_chunk(GRAM_ADDR + chunk, 0x100)) {
            return 0;
        }
        vdp_memread(DUMP_VRAM_ADDR, (char*)&gram[chunk], 0x100);
        if (chunk == 0) {
            int ran = 0;
            for (i = 0; i < 0x100; i++) {
                if (gram[i] != 0xAA) {
                    ran = 1;
                    break;
                }
            }
            if (!ran) {
                return 0;
            }
        }
    }
    return 1;
}

static void print_dump(void) {
    unsigned int i, j;
    for (i = 0; i < PRINT_BYTES; i += BYTES_PER_LINE) {
        put_addr(GRAM_ADDR + i);
        term_puts(": ");
        for (j = 0; j < BYTES_PER_LINE; j++) {
            put_hex_byte(gram[i + j]);
            term_putc(' ');
        }
        term_putc('\n');
    }
}

static int base_signature(void) {
    return gram[0] == 0x03 && gram[1] == 0x40 &&
           gram[2] == 0xFC && gram[3] == 0x01;
}

static void print_base_state(void) {
    term_gotoxy(1, ROW_BASE);
    if (base_signature()) {
        term_puts("base: IDLE + >FC01 at >4000");
    } else if (gram[0] == 0x03 && gram[1] == 0x40) {
        term_puts("base: IDLE stub, no ForceCommand >FC01");
    } else if (gram[0] == 0 && gram[1] == 0 && gram[2] == 0 && gram[3] == 0) {
        term_puts("base: absent (all zero)");
    } else {
        term_puts("base: unknown bytes at >4000");
    }
}

static void print_info(int result, struct F18aGpuInfo* info) {
    term_gotoxy(1, ROW_INFO);
    term_puts("info: rc=");
    term_putc(result < 0 ? '-' : '+');
    term_puts(str_from_uint(result < 0 ? (unsigned int)-result : (unsigned int)result));

    if (result != GFX_OK) {
        term_gotoxy(1, ROW_INFO2);
        term_puts("info: no GPU base (is this 80x30?)");
        return;
    }

    term_gotoxy(1, ROW_INFO2);
    term_puts("v=");
    put_hex_word(info->version);
    term_puts(" free=");
    put_addr(info->free_addr);
    term_puts(" vt=");
    put_addr(info->vector_table);
    term_puts(" uv=");
    put_addr(info->user_vector_table);
}

static void print_blkcpy(void) {
    static const unsigned int lens[] = {1, 80, 320, 1280};
    unsigned int i;

    term_gotoxy(1, ROW_BLK);
    term_puts("blk:");
    for (i = 0; i < sizeof(lens) / sizeof(lens[0]); i++) {
        term_putc(' ');
        term_puts(str_from_uint(lens[i]));
        if (f18a_gpu_copy(DUMP_VRAM_ADDR, DUMP_VRAM_ADDR + 0x100, lens[i]) != GFX_OK) {
            term_puts(" ERR");
            return;
        }
    }

    if (f18a_gpu_fill(DUMP_VRAM_ADDR, 0xAA, 16) != GFX_OK) {
        term_puts(" fill ERR");
        return;
    }
    term_puts(" fill");
}

int main(char* args) {
    struct GfxInformation gfx;
    struct F18aGpuInfo info;
    int safe_mode = 0;
    int run_blk = 0;
    int gpu_ran = 0;
    int info_result;
    char* a = args;

    /* "safe" or "nopgu" skips every GPU access. */
    while (a && (*a == ' ' || *a == '\t')) {
        a++;
    }
    if (a && (*a == 's' || *a == 'S' || *a == 'n' || *a == 'N')) {
        safe_mode = 1;
    }
    if (a && (*a == 'b' || *a == 'B')) {
        run_blk = 1;
    }

    term_cls();

    if (gfx_get_info(&gfx) != GFX_OK) {
        term_puts("gfx_get_info failed\n");
        return 1;
    }
    if (gfx.vdp_type != VDP_F18A) {
        term_puts("not an F18A\n");
        return 1;
    }
    if (gfx.mode != GFX_MODE_F18A_TEXT80X30) {
        term_puts("run this in the F18A 80x30 mode\n");
        return 1;
    }

    term_gotoxy(1, ROW_TITLE);
    term_puts("F18A GRAM >4000 / ForceCommand GPU base\n");

    if (safe_mode) {
        term_gotoxy(1, ROW_STATE);
        term_puts("safe mode: GPU not run\n");
        term_gotoxy(1, ROW_DONE);
        term_puts("done\n");
        return 0;
    }

    if (!upload_gpu_code()) {
        term_gotoxy(1, ROW_STATE);
        term_puts("GPU code upload failed - VRAM port problem\n");
        return 3;
    }

    term_gotoxy(1, ROW_STATE);
    term_puts("reading GRAM before init ...\n");
    gpu_ran = read_gram();
    if (!gpu_ran) {
        /* 1.C can leave the F18A locked while the display is still 80x30. */
        term_gotoxy(1, ROW_STATE);
        term_puts("GPU did not run - retrying after unlock ...\n");
        VDP_SET_REGISTER(0x39, 0x00);
        VDP_SET_REGISTER(0x39, 0x1C);
        VDP_SET_REGISTER(0x39, 0x1C);
        gpu_ran = read_gram();
    }
    if (!gpu_ran) {
        term_gotoxy(1, ROW_STATE);
        term_puts("GPU did not run - dump is stale VRAM\n");
        return 4;
    }

    term_gotoxy(1, ROW_DUMP);
    print_dump();
    print_base_state();

    /* Calling the public API installs the base if it is missing. */
    info_result = f18a_gpu_info(&info);
    print_info(info_result, &info);

    term_gotoxy(1, ROW_STATE);
    term_puts("reading GRAM after init ...\n");
    if (!read_gram()) {
        term_gotoxy(1, ROW_STATE);
        term_puts("GPU did not run after init\n");
        return 4;
    }

    term_gotoxy(1, ROW_DUMP);
    print_dump();
    print_base_state();

    if (info_result == GFX_OK && run_blk) {
        print_blkcpy();
    } else if (info_result == GFX_OK) {
        term_gotoxy(1, ROW_BLK);
        term_puts("blk: skipped (use 'blk' to probe)");
    }

    term_gotoxy(1, ROW_DONE);
    term_puts("done\n");
    return 0;
}
