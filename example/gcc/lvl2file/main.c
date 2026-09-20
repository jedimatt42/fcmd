#include "fctest.h"
#include <ioports.h>

/*
 * I16 - level-2 direct file I/O.
 *
 * Covers: path_to_iocode, lvl2_setdir, lvl2_input, lvl2_input_cpu,
 *         lvl2_output, lvl2_output_cpu, and the AddInfo fields.
 *
 * path_to_iocode and lvl2_setdir are always checked. The direct I/O round-trip
 * needs a device that supports level-2 direct input/output (TIPI, SCSI, IDE,
 * HFDC); on anything else it skips.
 *
 * A FIXED 128 source file with four records is created with DSR. lvl2_input
 * with blockcount 0 fills AddInfo, lvl2_output with blockcount 0 creates a
 * destination of the same shape, then sector 0 is copied with the VDP variants
 * and sector 1 with the CPU variants. The destination is verified by DSR read.
 *
 * NOTE: direct I/O pads filenames in place to 10 characters, so filename
 * buffers must be at least 11 bytes and mutable. AddInfo must be in scratchpad.
 */

#define SCRATCH_SRC "ZZLVL2S"
#define SCRATCH_DST "ZZLVL2D"

static struct AddInfo* addInfo = (struct AddInfo*)0x8320;

static void test_iocode(void) {
    FC_CHECK_EQ(path_to_iocode("TIPI."), 0x0010);
    FC_CHECK_EQ(path_to_iocode("DSK."), 0x0010);
    FC_CHECK_EQ(path_to_iocode("DSK1."), 0x0110);
    FC_CHECK_EQ(path_to_iocode("DSK2."), 0x0210);
    FC_CHECK_EQ(path_to_iocode("RD."), 0x0710);
    FC_CHECK_EQ(path_to_iocode("WDS1."), 0x0120);
    FC_CHECK_EQ(path_to_iocode("SCS1."), 0x0120);
    FC_CHECK_EQ(path_to_iocode("IDE1."), 0x0180);
    FC_CHECK_EQ(path_to_iocode("HDX1."), 0x0190);
}

static void delete_file(struct DeviceServiceRoutine* dsr, char* fullpath) {
    struct PAB pab;
    str_set((char*)&pab, 0, sizeof(pab));
    pab.pName = fullpath;
    dsr_delete(dsr, &pab);
}

static void fill_record(char* rec, char c) {
    str_set(rec, c, 128);
}

static int bytes_equal(const char* a, const char* b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int check_sector(const char* buf, char first, char second) {
    int i;
    for (i = 0; i < 128; i++) {
        if (buf[i] != first) {
            return 0;
        }
    }
    for (i = 128; i < 256; i++) {
        if (buf[i] != second) {
            return 0;
        }
    }
    return 1;
}

static void test_direct_io(void) {
    struct SystemInformation si;
    struct DeviceServiceRoutine* dsr;
    struct PAB pab;
    char empty[] = "";
    char path[256];
    char fsrc[256];
    char fdst[256];
    char srcname[16];
    char dstname[16];
    char rec[128];
    char expect[128];
    char cpu_buf[256];
    char vbuf[256];
    unsigned int iocode;
    unsigned int crubase;
    unsigned int r;
    int i;
    int opened;

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
    if (str_len(path) == 0 || path[str_len(path) - 1] != '.') {
        str_cat(path, ".");
    }

    iocode = path_to_iocode(path);
    crubase = dsr->crubase;

    FC_CHECK_EQ(lvl2_setdir(crubase, iocode, path), 0);

    str_copy(fsrc, path);
    str_cat(fsrc, SCRATCH_SRC);
    str_copy(fdst, path);
    str_cat(fdst, SCRATCH_DST);
    str_copy(srcname, SCRATCH_SRC);
    str_copy(dstname, SCRATCH_DST);

    delete_file(dsr, fsrc);
    delete_file(dsr, fdst);

    /* create the source file with DSR */
    r = dsr_open(dsr, &pab, fsrc, DSR_TYPE_OUTPUT, 128);
    if (r != 0) {
        FC_SKIP("could not create scratch file (device not writable?)");
        return;
    }
    FC_CHECK_EQ(r, 0);
    for (i = 0; i < 4; i++) {
        fill_record(rec, (char)('A' + i));
        FC_CHECK_EQ(dsr_write(dsr, &pab, rec, 128), 0);
    }
    FC_CHECK_EQ(dsr_close(dsr, &pab), 0);

    /* open the source for direct input; fills AddInfo */
    r = lvl2_input(crubase, iocode, srcname, 0, addInfo);
    if (r != 0) {
        FC_SKIP("device does not support level-2 direct I/O");
        delete_file(dsr, fsrc);
        return;
    }
    FC_CHECK_EQ(r, 0);
    FC_CHECK(addInfo->rec_length > 0);
    FC_CHECK(addInfo->records > 0);

    term_puts("AddInfo rec_length=");
    term_puts(str_from_uint((unsigned int)addInfo->rec_length));
    term_puts(" records=");
    term_puts(str_from_uint((unsigned int)addInfo->records));
    term_putc('\n');

    /* create a destination of the same shape */
    r = lvl2_output(crubase, iocode, dstname, 0, addInfo);
    FC_CHECK_EQ(r, 0);
    if (r != 0) {
        delete_file(dsr, fsrc);
        return;
    }

    /* sector 0 via the VDP-buffer variants */
    addInfo->first_sector = 0;
    r = lvl2_input(crubase, iocode, srcname, 1, addInfo);
    FC_CHECK_EQ(r, 0);
    if (r == 0) {
        vdp_memread(si.vdp_io_buf, vbuf, 256);
        FC_CHECK(check_sector(vbuf, 'A', 'B'));
        addInfo->first_sector = 0;
        FC_CHECK_EQ(lvl2_output(crubase, iocode, dstname, 1, addInfo), 0);
    }

    /* sector 1 via the CPU-buffer variants */
    addInfo->first_sector = 1;
    addInfo->buffer = (unsigned int)cpu_buf;
    r = lvl2_input_cpu(crubase, iocode, srcname, 1, addInfo);
    FC_CHECK_EQ(r, 0);
    if (r == 0) {
        FC_CHECK(check_sector(cpu_buf, 'C', 'D'));
        addInfo->first_sector = 1;
        addInfo->buffer = (unsigned int)cpu_buf;
        FC_CHECK_EQ(lvl2_output_cpu(crubase, iocode, dstname, 1, addInfo), 0);
    }

    /* verify the destination by reading it back with DSR */
    r = dsr_open(dsr, &pab, fdst, DSR_TYPE_INPUT, 128);
    FC_CHECK_EQ(r, 0);
    opened = (r == 0);
    if (opened) {
        for (i = 0; i < 4; i++) {
            fill_record(expect, (char)('A' + i));
            r = dsr_read_cpu(dsr, &pab, 0, rec);
            FC_CHECK_EQ(r, 0);
            if (r == 0) {
                FC_CHECK(bytes_equal(rec, expect, 128));
            }
        }
        dsr_close(dsr, &pab);
    }

    delete_file(dsr, fsrc);
    delete_file(dsr, fdst);
}

int main(char* args) {
    (void)args;

    term_puts("LVL2FILE test\n");

    test_iocode();
    test_direct_io();

    return fc_summary();
}
