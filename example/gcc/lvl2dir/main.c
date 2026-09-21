#include "fctest.h"

/*
 * I17 - level-2 directory operations.
 *
 * Covers: lvl2_mkdir, lvl2_rmdir, lvl2_rename, lvl2_rendir, lvl2_protect.
 *
 * Needs a device that supports level-2 directory operations (TIPI, SCSI, IDE,
 * HFDC); on anything else it skips.
 *
 * Names are padded in place to 10 characters by the level-2 layer, so every
 * name buffer is at least 11 bytes and mutable. lvl2_setdir selects the parent
 * directory and the operations take bare names.
 */

#define DIR1  "ZZLVL2A"
#define DIR2  "ZZLVL2B"
#define FILE1 "ZZLVL2F"
#define FILE2 "ZZLVL2G"

static void delete_file(struct DeviceServiceRoutine* dsr, char* fullpath) {
    struct PAB pab;
    str_set((char*)&pab, 0, sizeof(pab));
    pab.pName = fullpath;
    dsr_delete(dsr, &pab);
}

static void test_dirs(unsigned int crubase, unsigned int iocode) {
    char dir1[16];
    char dir2[16];
    unsigned int r;

    str_copy(dir1, DIR1);
    str_copy(dir2, DIR2);

    /* clean up any leftovers first */
    lvl2_rmdir(crubase, iocode, dir1);
    lvl2_rmdir(crubase, iocode, dir2);

    r = lvl2_mkdir(crubase, iocode, dir1);
    if (r != 0) {
        FC_SKIP("device does not support level-2 directory ops");
        return;
    }
    FC_CHECK_EQ(r, 0);

    /* rename DIR1 -> DIR2, then DIR1 must be free to create again */
    FC_CHECK_EQ(lvl2_rendir(crubase, iocode, dir1, dir2), 0);
    FC_CHECK_EQ(lvl2_mkdir(crubase, iocode, dir1), 0);

    FC_CHECK_EQ(lvl2_rmdir(crubase, iocode, dir1), 0);
    FC_CHECK_EQ(lvl2_rmdir(crubase, iocode, dir2), 0);
}

static void test_file_rename(unsigned int crubase, unsigned int iocode,
                             struct DeviceServiceRoutine* dsr, char* path) {
    struct PAB pab;
    char file1[16];
    char file2[16];
    char fpath1[256];
    char fpath2[256];
    char rec[128];
    unsigned int r;

    str_copy(file1, FILE1);
    str_copy(file2, FILE2);

    str_copy(fpath1, path);
    str_cat(fpath1, FILE1);
    str_copy(fpath2, path);
    str_cat(fpath2, FILE2);

    delete_file(dsr, fpath1);
    delete_file(dsr, fpath2);

    r = dsr_open(dsr, &pab, fpath1, DSR_TYPE_OUTPUT, 128);
    if (r != 0) {
        FC_SKIP("could not create scratch file");
        return;
    }
    FC_CHECK_EQ(r, 0);
    str_set(rec, 'R', 128);
    FC_CHECK_EQ(dsr_write(dsr, &pab, rec, 128), 0);
    FC_CHECK_EQ(dsr_close(dsr, &pab), 0);

    /* rename FILE1 -> FILE2 */
    r = lvl2_rename(crubase, iocode, file1, file2);
    if (r != 0) {
        FC_SKIP("device does not support level-2 file rename");
        delete_file(dsr, fpath1);
        return;
    }
    FC_CHECK_EQ(r, 0);

    /* FILE2 now exists and FILE1 is gone */
    r = dsr_open(dsr, &pab, fpath2, DSR_TYPE_INPUT, 128);
    FC_CHECK_EQ(r, 0);
    if (r == 0) {
        dsr_close(dsr, &pab);
    }
    r = dsr_open(dsr, &pab, fpath1, DSR_TYPE_INPUT, 128);
    FC_CHECK(r != 0);
    if (r == 0) {
        dsr_close(dsr, &pab);
    }

    /* protect / unprotect */
    r = lvl2_protect(crubase, iocode, file2, 1);
    if (r != 0) {
        FC_SKIP("device does not support level-2 protect");
    } else {
        FC_CHECK_EQ(r, 0);
        FC_CHECK_EQ(lvl2_protect(crubase, iocode, file2, 0), 0);
    }

    delete_file(dsr, fpath2);
}

static void test_lvl2dir(void) {
    struct SystemInformation si;
    struct DeviceServiceRoutine* dsr;
    char empty[] = "";
    char path[256];
    unsigned int iocode;
    unsigned int crubase;

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

    test_dirs(crubase, iocode);
    test_file_rename(crubase, iocode, dsr, path);
}

int main(char* args) {
    (void)args;

    term_puts("LVL2DIR test\n");

    test_lvl2dir();

    return fc_summary();
}
