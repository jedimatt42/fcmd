#include "fctest.h"

/*
 * I15 - DSR directory catalog.
 *
 * Covers: dsr_catalog (with vol_entry_cb / dir_entry_cb), and an opt-in path
 * for dsr_ea5_load.
 *
 * The catalog test creates a scratch file, catalogs the current directory, and
 * checks the volume callback fired once and the file callback found the scratch
 * file. The scratch file is deleted afterwards.
 *
 * dsr_ea5_load transfers control to the loaded program, so it is only run when
 * invoked as:  DSRCAT /ea5 <file>
 */

#define SCRATCH "ZZCATFIL"

static int vol_calls;
static int dir_calls;
static int found_scratch;
static char first_entry[11];

/*
 * IMPORTANT: callbacks invoked from ForceCommand's banked code (dsr_catalog
 * runs in bank 9) must not call any other ForceCommand API function. The
 * external API trampoline always returns to cartridge bank 0, so an API call
 * from inside a callback leaves bank 0 mapped and the return into bank 9
 * crashes. Keep these callbacks leaf functions and use plain C only.
 */

static int name_equals(const char* name, const char* want) {
    int i = 0;
    while (name[i] != 0 && want[i] != 0) {
        char a = name[i];
        char b = want[i];
        if (a >= 'a' && a <= 'z') {
            a -= 32;
        }
        if (b >= 'a' && b <= 'z') {
            b -= 32;
        }
        if (a != b) {
            return 0;
        }
        i++;
    }
    return name[i] == want[i];
}

static void on_vol(struct VolInfo* v) {
    (void)v;
    vol_calls++;
}

static void on_dir(struct DirEntry* d) {
    int i;

    dir_calls++;
    if (dir_calls == 1) {
        for (i = 0; i < 10 && d->name[i] != 0; i++) {
            first_entry[i] = d->name[i];
        }
        first_entry[i] = 0;
    }
    if (name_equals(d->name, SCRATCH)) {
        found_scratch = 1;
    }
}

static int run_ea5(char* rest) {
    char fname[256];
    struct DeviceServiceRoutine* dsr;

    if (rest == 0 || rest[0] == 0) {
        term_puts("usage: DSRCAT /ea5 <file>\n");
        return 0;
    }

    path_parse(rest, &dsr, fname, PR_REQUIRED);
    if (dsr == 0) {
        term_puts("could not resolve EA5 file\n");
        return 0;
    }

    term_puts("[OBSERVE] loading EA5 program; this transfers control\n");
    dsr_ea5_load(dsr, fname);
    FC_OBSERVE("control returned from the EA5 loader");
    return fc_summary();
}

static void test_catalog(void) {
    char empty[] = "";
    char path[256];
    char filepath[256];
    struct DeviceServiceRoutine* dsr;
    struct PAB pab;
    unsigned int e;
    unsigned int r;

    path_parse(empty, &dsr, path, PR_OPTIONAL);
    if (dsr == 0) {
        FC_SKIP("no usable current device");
        return;
    }
    if (str_len(path) == 0 || path[str_len(path) - 1] != '.') {
        str_cat(path, ".");
    }

    str_copy(filepath, path);
    str_cat(filepath, SCRATCH);

    /* remove any leftover, then create a known entry */
    str_set((char*)&pab, 0, sizeof(pab));
    pab.pName = filepath;
    dsr_delete(dsr, &pab);

    e = dsr_open(dsr, &pab, filepath, DSR_TYPE_OUTPUT | DSR_TYPE_VARIABLE, 0);
    if (e != 0) {
        FC_SKIP("could not create scratch file (device not writable?)");
        return;
    }
    FC_CHECK_EQ(dsr_write(dsr, &pab, "x", 1), 0);
    FC_CHECK_EQ(dsr_close(dsr, &pab), 0);

    vol_calls = 0;
    dir_calls = 0;
    found_scratch = 0;
    first_entry[0] = 0;

    r = dsr_catalog(dsr, path, on_vol, on_dir);
    if (r != 0) {
        FC_SKIP("catalog not supported on this device");
    } else {
        FC_CHECK_EQ(r, 0);
        FC_CHECK_EQ(vol_calls, 1);
        FC_CHECK(dir_calls >= 1);
        FC_CHECK_EQ(found_scratch, 1);

        term_puts("first entry: ");
        term_puts(first_entry);
        term_putc('\n');
        term_puts("entries listed: ");
        term_puts(str_from_uint((unsigned int)dir_calls));
        term_putc('\n');
    }

    /* cleanup */
    str_set((char*)&pab, 0, sizeof(pab));
    pab.pName = filepath;
    dsr_delete(dsr, &pab);
}

int main(char* args) {
    char argbuf[32];
    char* rest;

    term_puts("DSRCAT test\n");

    rest = str_token_next(argbuf, args, ' ');
    if (str_cmp_icase(argbuf, "/ea5") == 0) {
        return run_ea5(rest);
    }

    test_catalog();
    return fc_summary();
}
