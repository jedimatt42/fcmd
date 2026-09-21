#include "fctest.h"

/*
 * I18 - level-2 raw sector I/O.
 *
 * Covers: lvl2_sector_read, lvl2_sector_write.
 *
 * lvl2_format is intentionally not exercised: it needs a real floppy controller
 * and does not work under the available emulators.
 *
 * Sector write is destructive in general, so the test reads sector 0, writes
 * the exact same bytes back, reads it again, and compares. Content is
 * preserved. Needs a device that supports level-2 sector I/O; otherwise skips.
 */

static int bytes_equal(const char* a, const char* b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void test_sector_io(void) {
    struct SystemInformation si;
    struct DeviceServiceRoutine* dsr;
    char empty[] = "";
    char path[256];
    char buf[256];
    char buf2[256];
    unsigned int iocode;
    unsigned int crubase;
    unsigned int r;

    sys_info(&si);
    dsr = si.currentDsr;
    if (dsr == 0) {
        FC_SKIP("no current device");
        return;
    }

    path_parse(empty, &dsr, path, PR_OPTIONAL);
    if (dsr == 0) {
        FC_SKIP("no usable current device");
        return;
    }

    iocode = path_to_iocode(path);
    crubase = dsr->crubase;

    r = lvl2_sector_read(crubase, iocode, 0, buf);
    if (r != 0) {
        FC_SKIP("device does not support level-2 sector I/O");
        return;
    }
    FC_CHECK_EQ(r, 0);

    term_puts("sector 0 starts: ");
    term_puts(hex_from_uint((unsigned int)(unsigned char)buf[0]));
    term_putc(' ');
    term_puts(hex_from_uint((unsigned int)(unsigned char)buf[1]));
    term_putc('\n');

    /* write the same sector back unchanged, then read and compare */
    r = lvl2_sector_write(crubase, iocode, 0, buf);
    FC_CHECK_EQ(r, 0);

    r = lvl2_sector_read(crubase, iocode, 0, buf2);
    FC_CHECK_EQ(r, 0);
    if (r == 0) {
        FC_CHECK(bytes_equal(buf, buf2, 256));
    }
}

int main(char* args) {
    (void)args;

    term_puts("LVL2SECTOR test\n");

    test_sector_io();

    return fc_summary();
}
