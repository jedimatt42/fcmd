#include "banks.h"
#define MYBANK BANK(6)

#include "commands.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_tifloat.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
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

// https://atariage.com/forums/topic/290966-force-command-ver-0k-kinda-like-commandcom-from-1985/?do=findComment&comment=4557648
//  ^--- Lee provides details on calling the basic MOUNT subroutine

static void tmp_mount(int drive, int volume);

// Warning: hardcoded crubase for physical... classic99 uses >1000
#define CFNANO_CRUBASE 0x1100


static int getVolume(int drive, char* volumeName) {
    // it seems a directory listing of an unformatted volume
    // will just show trash...
    // but the used + available should be 1598
    volumeName[0] = 0;

    unsigned char path[6]; // paranoid about the parse command.
    strcpy(path, "DSKx");
    path[3] = drive + '0';
    struct DeviceServiceRoutine *dsr = bk_findDsr(path, CFNANO_CRUBASE);
    strcat(path, ".");

    struct VolInfo volInfo;
    // basically just read the first record from the catalog.
    struct PAB pab;

    int result = 0;
    unsigned int ferr = bk_dsr_open(dsr, &pab, path, DSR_TYPE_INPUT | DSR_TYPE_DISPLAY | DSR_TYPE_FIXED | DSR_TYPE_INTERNAL | DSR_TYPE_RELATIVE, 0);
    if (ferr == DSR_ERR_NONE) {
        ferr = bk_dsr_read(dsr, &pab, 0);
        if (ferr == DSR_ERR_NONE) {
            // this is excessively large to avoid trouble when reading from unitialized volumes.
            unsigned char cbuf[256];

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

    for (int i = begin; i <= end; i++)
    {
        // mount the volume into DSK1.
        tmp_mount(1, i);

        if (1 == getVolume(1, volumeName)) {
            bk_tputs_ram(int2str(i));
            tputs_rom(" : ");
            bk_tputs_ram(volumeName);
            bk_tputc('\n');
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
        bk_tputs_ram(int2str(i + 1));
        tputs_rom(". -> ");
        bk_tputs_ram(int2str(mapping[i]));
        char volumeName[11];
        getVolume(i + 1, volumeName);
        tputs_rom(" : ");
        bk_tputs_ram(volumeName);
        bk_tputc('\n');
    }
}

#define PADDR *((volatile int*)0x832C)
#define FACADDR *((volatile int*)0x834A)
#define PARAMS *((volatile int*)0x8356)

static void call_mount(int drive, int volume) {
    // setup parameters -
    // PADDR <- FBUF: (VDP) 05 "MOUNT" B7 C8 01 <drive> B3 C8 <vlen> <volstr> B6
    // 0x834A <- "MOUNT"
    // 0x8356 <- FBUF + 6
    PADDR = FBUF;
    PARAMS = FBUF+6;
    strncpy((char*)FACADDR, "MOUNT", 5);

    // build FBUF up
    unsigned char command[30];
    int i = 0;
    command[i++] = 5;
    strcpy(command+i, "MOUNT");
    i += 5;
    command[i++] = 0xB7;
    command[i++] = 0xC8;
    command[i++] = 1;
    command[i++] = '0' + drive;
    command[i++] = 0xB3;
    command[i++] = 0xC8;
    char* vol = int2str(volume);
    command[i++] = strlen(vol);
    strcpy(command+(i++), vol);
    vdpmemcpy(FBUF, command, 30);

    // call persistent mount routine
    char subname[6] = "MOUNT";
    bk_call_basic_sub(CFNANO_CRUBASE, subname);
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
    //  CFMOUNT [/p] 1-3 vol
    char *peek = strtokpeek(0, " ");
    int list = peek == 0;
    int volumes = 0 == strcmpi("/v", peek);
    int persist = 0 == strcmpi("/p", peek);
    if (persist || volumes)
    {
        strtok(0, " "); // consume the optional /c
    }

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
        int begin = 1;
        int end = 312;
        peek = strtokpeek(0, " ");
        if (peek) {
            peek = strtok(0, " ");
            begin = atoi(peek);
            peek = strtok(0, " ");
            end = atoi(peek);
            if (end < begin || begin == 0 || end == 0) {
                tputs_rom("illegal range specified\n");
                return;
            }
        }

        listAllVolumes(begin, end);
        return;
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
