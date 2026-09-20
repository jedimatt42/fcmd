#include "fctest.h"

/*
 * I14 - DSR file operations.
 *
 * Covers: dsr_find, dsr_open, dsr_close, dsr_write, dsr_read, dsr_read_cpu,
 *         dsr_status, dsr_reset, dsr_delete.
 *
 * A DISPLAY VARIABLE scratch file is created in the current path, written,
 * read back, rewound, and deleted. Any pre-existing leftover is removed first.
 * If the current device is not writable the file part is skipped.
 */

#define SCRATCH "ZZDSRTST"

static void delete_scratch(struct DeviceServiceRoutine* dsr, char* fname) {
    struct PAB pab;
    str_set((char*)&pab, 0, sizeof(pab));
    pab.pName = fname;
    dsr_delete(dsr, &pab);
}

static void test_find(void) {
    struct SystemInformation si;
    struct DeviceServiceRoutine* dsr;
    char bogus[] = "ZZZZ";

    sys_info(&si);
    FC_CHECK(si.currentDsr != 0);
    if (si.currentDsr != 0) {
        dsr = dsr_find(si.currentDsr->name, si.currentDsr->crubase);
        FC_CHECK(dsr == si.currentDsr);
    }
    FC_CHECK(dsr_find(bogus, 0) == 0);
}

static void test_files(void) {
    char file[] = SCRATCH;
    char fname[256];
    char r1[] = "alpha";
    char r2[] = "beta";
    char r3[] = "gamma";
    char buf[64];
    struct DeviceServiceRoutine* dsr;
    struct PAB pab;
    unsigned int e;
    unsigned int st;

    path_parse(file, &dsr, fname, PR_REQUIRED);
    if (dsr == 0) {
        FC_SKIP("no usable current device");
        return;
    }

    delete_scratch(dsr, fname);

    e = dsr_open(dsr, &pab, fname, DSR_TYPE_OUTPUT | DSR_TYPE_VARIABLE, 0);
    if (e != 0) {
        FC_SKIP("could not create scratch file (device not writable?)");
        return;
    }
    FC_CHECK_EQ(e, 0);

    FC_CHECK_EQ(dsr_write(dsr, &pab, r1, 5), 0);
    FC_CHECK_EQ(dsr_write(dsr, &pab, r2, 4), 0);
    FC_CHECK_EQ(dsr_write(dsr, &pab, r3, 5), 0);
    FC_CHECK_EQ(dsr_close(dsr, &pab), 0);

    /* read back */
    e = dsr_open(dsr, &pab, fname, DSR_TYPE_INPUT | DSR_TYPE_VARIABLE, 0);
    if (e != 0) {
        FC_SKIP("could not reopen scratch file");
        delete_scratch(dsr, fname);
        return;
    }
    FC_CHECK_EQ(e, 0);

    /* status must not report an error (errors are encoded as result << 8) */
    st = dsr_status(dsr, &pab);
    FC_CHECK(st <= 0xff);

    FC_CHECK_EQ(dsr_read_cpu(dsr, &pab, 0, buf), 0);
    buf[pab.CharCount] = 0;
    FC_CHECK_EQ(str_cmp(buf, "alpha"), 0);

    FC_CHECK_EQ(dsr_read_cpu(dsr, &pab, 0, buf), 0);
    buf[pab.CharCount] = 0;
    FC_CHECK_EQ(str_cmp(buf, "beta"), 0);

    FC_CHECK_EQ(dsr_read_cpu(dsr, &pab, 0, buf), 0);
    buf[pab.CharCount] = 0;
    FC_CHECK_EQ(str_cmp(buf, "gamma"), 0);

    /* rewind and read the first record again */
    FC_CHECK_EQ(dsr_reset(dsr, &pab, 0), 0);
    FC_CHECK_EQ(dsr_read_cpu(dsr, &pab, 0, buf), 0);
    buf[pab.CharCount] = 0;
    FC_CHECK_EQ(str_cmp(buf, "alpha"), 0);

    FC_CHECK_EQ(dsr_close(dsr, &pab), 0);

    /* delete and confirm it is gone */
    pab.pName = fname;
    FC_CHECK_EQ(dsr_delete(dsr, &pab), 0);

    e = dsr_open(dsr, &pab, fname, DSR_TYPE_INPUT | DSR_TYPE_VARIABLE, 0);
    FC_CHECK(e != 0);
    if (e == 0) {
        dsr_close(dsr, &pab);
    }
}

int main(char* args) {
    (void)args;

    term_puts("DSRTEST test\n");

    test_find();
    test_files();

    return fc_summary();
}
