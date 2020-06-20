#include "banks.h"
#define MYBANK BANK_6

#include "commands.h"
#include "b0_parsing.h"
#include "b1cp_strutil.h"
#include "b2_dsrutil.h"
#include "b2_tifloat.h"
#include "b1cp_terminal.h"
#include <string.h>
#include <vdp.h>

/*
 *  VDP Address | Purpose
 *  ----------- | ----------------------------
 *  0x3FF8-9    | if 0xAA03 then CF exists
 *  0x3FFA-B    | volume# of DSK1
 *  0x3FFC-D    | volume# of DSK2
 *  0x3FFE-F    | volume# of DSK3
 */

static void tmp_mount(int drive, int volume);


static int getVolume(int drive, int volume, char* volumeName) {
    // it seems a directory listing of an unformatted volume
    // will just show trash...
    // but the used + available should be 1598
    volumeName[0] = 0;

    unsigned char path[6]; // paranoid about the parse command.
    strcpy(path, "DSKx");
    path[3] = drive + '0';
    struct DeviceServiceRoutine *dsr = bk_findDsr(path, 0x1100);
    strcat(path, ".");

    struct VolInfo volInfo;
    // basically just read the first record from the catalog.
    struct PAB pab;
    unsigned char cbuf[150];

    int result = 0;
    unsigned int ferr = bk_dsr_open(dsr, &pab, path, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 0);
    if (ferr == DSR_ERR_NONE) {
        ferr = bk_dsr_read(dsr, &pab, 0);
        if (ferr == DSR_ERR_NONE) {
            vdpmemread(FBUF, cbuf, pab.CharCount);
            int namlen = basicToCstr(cbuf, volInfo.volname);
            int a = bk_ti_floatToInt(cbuf + 1 + namlen);
            int j = bk_ti_floatToInt(cbuf + 10 + namlen);
            int k = bk_ti_floatToInt(cbuf + 19 + namlen);
            volInfo.total = j;
            volInfo.available = k;
            if (j == 1598) {
                strncpy(volumeName, volInfo.volname, 10);
                result = 1;
            }
        }
    }
    bk_dsr_close(dsr, &pab);
    return result;
}

static void listAllVolumes(int begin, int end) {
    // backup mappings
    int mapping[3];
    vdpmemread(0x3FFA, (unsigned char *)mapping, 6);

    char volumeName[11];

    int blanks = 0;
    for (int i = begin; i <= end; i++)
    {
        // mount the volume into DSK1.
        tmp_mount(1, i);

        if (1 == getVolume(1, i, volumeName)) {
            tputs_ram(int2str(i));
            tputs_rom(" : ");
            tputs_ram(volumeName);
            tputc('\n');
            blanks = 0;
        } else {
            blanks++;
            if (blanks > 10) {
                break;
            }
        }
    }

    // restore original mappings...
    vdpmemcpy(0x3FFA, (const unsigned char *)mapping, 6);
}

static void listCurrentVolumes() {
    int mapping[3];
    vdpmemread(0x3FFA, (unsigned char*)mapping, 6);
    for (int i=0; i<3; i++) {
        tputs_rom("DSK");
        tputs_ram(int2str(i + 1));
        tputs_rom(". -> ");
        tputs_ram(int2str(mapping[i]));
        char volumeName[11];
        getVolume(i + 1, mapping[i], volumeName);
        tputs_rom(" : ");
        tputs_ram(volumeName);
        tputc('\n');
    }
}

static void call_mount(int drive, int volume) {

}

static void tmp_mount(int drive, int volume) {
    int base_addr = 0x3FF8;
    base_addr += (2*drive);
    VDP_SET_ADDRESS_WRITE(base_addr);
    VDPWD = volume >> 8;
    VDPWD = volume & 0x00FF;
}


void handleCFMount() {
    //  CFMOUNT
    //  CFMOUNT [/v] [begin] [end]
    //  CFMOUNT [/p] DSKn. vol
    char *peek = strtokpeek(0, " ");
    int list = peek == 0;
    int volumes = 0 == strcmpi("/v", peek);
    int persist = 0 == strcmpi("/p", peek);

    // test for CF7/nanopeb
    VDP_SET_ADDRESS(0x3FF8);
    int cf_nano_flag = 0;
    cf_nano_flag = VDPRD;
    cf_nano_flag = (cf_nano_flag << 8) + VDPRD;

    if (cf_nano_flag != 0xAA03) {
        tputs_rom("no CF7 or Nanopeb found\n");
        return;
    }

    if (list) {
        listCurrentVolumes();
        return;
    }

    if (volumes) {
        listAllVolumes(1, 312);
        return;
    }

    if (persist)
    {
        strtok(0, " "); // consume the optional /c
    }

    int drive = atoi(strtok(0, " "));
    if (drive < 1 || drive > 3) {
        tputs_rom("drive number must be 1, 2 or 3\n");
        return;
    }

    int volume = atoi(strtok(0, " "));
    if (volume == 0) {
        tputs_rom("volume number must be > 0\n");
        return;
    }

    if (persist) {
        call_mount(drive, volume);
    } else {
        tmp_mount(drive, volume);
    }
}
