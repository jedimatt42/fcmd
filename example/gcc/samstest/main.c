#include "fctest.h"

/*
 * I13 - SAMS page allocation and mapping.
 *
 * Covers: sys_sams_info, sams_alloc_pages, sams_free_pages, sams_map_page,
 *         sams_read_page.
 *
 * On a machine without SAMS the whole test is skipped.
 *
 * Mapping uses the top 4K window (0xF000). A small program does not reach that
 * far, so the window is safe scratch space. The original page mapping is read
 * first and restored, and ForceCommand also restores its own snapshot when the
 * executable returns.
 */

#define SLOT 0xF000

static void test_info_alloc_free(void) {
    struct SamsInformation before, mid, after;
    int p, top;

    sys_sams_info(&before);
    FC_CHECK(before.total_pages >= 0);
    FC_CHECK(before.next_page >= 0);

    p = sams_alloc_pages(1);
    FC_CHECK_EQ(p, before.next_page);

    sys_sams_info(&mid);
    FC_CHECK_EQ(mid.next_page, before.next_page + 1);

    top = sams_free_pages(1);
    FC_CHECK_EQ(top, before.next_page);

    sys_sams_info(&after);
    FC_CHECK_EQ(after.next_page, before.next_page);
}

static void test_read_page_invalid(void) {
    FC_CHECK_EQ(sams_read_page(0x8000), -1);
    FC_CHECK_EQ(sams_read_page(0x0000), -1);
    FC_CHECK_EQ(sams_read_page(0xF001), -1);
}

static void test_map_pages(void) {
    struct SamsInformation info;
    volatile unsigned char* mem = (volatile unsigned char*)SLOT;
    int original, p1, p2;

    sys_sams_info(&info);
    if (info.total_pages - info.next_page < 2) {
        FC_SKIP("fewer than two free SAMS pages");
        return;
    }

    original = sams_read_page(SLOT);
    p1 = sams_alloc_pages(1);
    p2 = sams_alloc_pages(1);

    sams_map_page(p1, SLOT);
    FC_CHECK_EQ(sams_read_page(SLOT), p1);
    mem[0] = 0x11;
    mem[1] = 0x22;

    sams_map_page(p2, SLOT);
    FC_CHECK_EQ(sams_read_page(SLOT), p2);
    mem[0] = 0x33;
    mem[1] = 0x44;

    /* the two pages must hold independent data */
    sams_map_page(p1, SLOT);
    FC_CHECK_EQ(mem[0], 0x11);
    FC_CHECK_EQ(mem[1], 0x22);

    sams_map_page(p2, SLOT);
    FC_CHECK_EQ(mem[0], 0x33);
    FC_CHECK_EQ(mem[1], 0x44);

    /* restore the caller's mapping and release our pages */
    sams_map_page(original, SLOT);
    FC_CHECK_EQ(sams_read_page(SLOT), original);
    sams_free_pages(2);
}

int main(char* args) {
    struct SamsInformation info;
    (void)args;

    term_puts("SAMSTEST test\n");

    sys_sams_info(&info);
    if (info.total_pages == 0) {
        FC_SKIP("no SAMS detected");
        return fc_summary();
    }

    term_puts("SAMS total=");
    term_puts(str_from_uint((unsigned int)info.total_pages));
    term_puts(" next=");
    term_puts(str_from_uint((unsigned int)info.next_page));
    term_putc('\n');

    test_info_alloc_free();
    test_read_page_invalid();
    test_map_pages();

    return fc_summary();
}
