#include "banking.h"
#define MYBANK BANK(11)

#include "b0_main.h"
#include "b8_terminal.h"
#include "commands.h"
#include "b0_runext.h"
#include "b10_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include "b4_variables.h"
#include "b0_globals.h"
#include "b0_sams.h"
#include "b8_setupScreen.h"
#include <vdp.h>

int loadExecutable(const char* ext, int* cmd_type);
int loadFromPath(const char *ext, const char *entry, int* cmd_type);
int allocAndLoad(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr);
int binLoad(struct DeviceServiceRoutine *dsr, int iocode, char *filename, struct AddInfo *addInfoPtr);

#define BIN 0xFCFC
#define SCRIPT 0x0001

void handleExecutable(char *ext)
{
    int cmd_type = 0;
    int err = loadExecutable(ext, &cmd_type);

    if (err) {
        tputs_rom("unknown command: ");
        bk_tputs_ram(ext);
        bk_tputc('\n');
        return;
    }

    if (cmd_type == BIN) {
        identify_callback old_hook = bk_get_identify_hook();
        int restoreDisplay = *(volatile int*)0xA004;
        int old_nTitleLine = nTitleLine;
        if (restoreDisplay != 0xFCFC) {
            // turn bar off, cause app is using the full screen.
            nTitleLine = 0;
            bk_setupScreen(displayWidth);
        }
        // Go to bank 0 to actually launch so API tables are visible.
        err = bk_runExecutable(ext);

        bk_set_identify_hook(old_hook);
        if (restoreDisplay != 0xFCFC) {
            nTitleLine = old_nTitleLine;
            bk_setupScreen(displayWidth);
        }

        if (err)
        {
            tputs_rom("error result: ");
            bk_tputs_ram(bk_uint2str(err));
            bk_tputc('\n');
        }

        if (sams_total_pages) {
            // if we didn't have sams, we wouldn't be able to get here with api_exec true.
            int newbase = bk_free_pages(6) - 6;
            for (int i = 0; i < 6; i++) {
                bk_map_page(newbase + i, 0xA000 + (i * 0x1000));
            }
        }
    }
}

char* token_cursor(char* dst, char* str, int delim) {
    // return the pointer to the character after the delim following token in original string:str
    // copies token into dst.
    // returns 0 if no following string
    if (!str)
    {
        dst[0] = 0;
        return 0;
    }

    int i = 0;
    while (str[i] != 0 && str[i] != delim)
    {
        dst[i] = str[i];
        i++;
    }
    dst[i] = 0;

    char* next = &(str[i]);
    if (*next == delim) {
        return next + 1;
    }
    return 0;
}

int loadExecutable(const char* ext, int* cmd_type) {
    // allow ext to be fully qualified path to command
    if (!loadFromPath(ext, 0, cmd_type)) {
        return 0;
    }

    // look in current directory second, always... regardless of PATH
    if (!loadFromPath(ext, currentPath, cmd_type)) {
        return 0;
    }

    char* path = bk_vars_get(str2ram("PATH"));
    if (path != (char*)-1) {
        char entry[64];
        char *cursor = token_cursor(entry, path, ';');
        // ensure '.' at end of path entry
        if (!bk_str_endswith(entry, str2ram("."))) {
          bk_strcat(entry, str2ram("."));
        }

        while(entry[0]) {
            if (!loadFromPath(ext, entry, cmd_type)) {
                return 0;
            }
            cursor = token_cursor(entry, cursor, ';');
        }
        return 1;
    }
}

/* return true if there is a problem preparing memory */
int prepareMemory() {
    if (sams_next_page + 6 > sams_total_pages) {
        return 1;  // don't allow nested execution if we don't have space
    }
    int pageStart = bk_alloc_pages(6);
    if (pageStart == -1) {
        return 1; // no more pages, fail the same way
    }
    // should be good to map upper memory expansion to the new pages.
    for (int i = 0; i < 6; i++) {
        bk_map_page(pageStart + i, 0xA000 + (i * 0x1000));
    }
    return 0;
}

void setDsrAndPath(const char* ext, const char* entry, struct DeviceServiceRoutine** dsr, char* path) {
    char fullname[256];
    fullname[0] = 0;
    if (entry != 0) {
        bk_strcpy(fullname, entry);
    }
    bk_strcat(fullname, ext);
    bk_parsePathParam(fullname, dsr, path, PR_REQUIRED);
}

int loadFromPath(const char *ext, const char *entry, int* cmd_type)
{
    struct DeviceServiceRoutine *dsr = 0;
    char path[256];

    setDsrAndPath(ext, entry, &dsr, path);
    if (!dsr) {
        return 1;
    }

    if (bk_runScript(dsr, path)) {
        *cmd_type = SCRIPT;
        return 0;
    }

    unsigned int iocode = bk_path2iocode(path);

    int parent_idx = bk_lindexof(path, '.', bk_strlen(path) - 1);
    char filename[11];
    bk_strncpy(filename, path + parent_idx + 1, 10);
    path[parent_idx + 1] = 0x00;

    bk_lvl2_setdir(dsr->crubase, iocode, path);

    // AddInfo must be in scratchpad
    struct AddInfo *addInfoPtr = (struct AddInfo *)0x8320;
    addInfoPtr->first_sector = 0;
    addInfoPtr->eof_offset = 0;
    addInfoPtr->flags = 0;
    addInfoPtr->rec_length = 0;
    addInfoPtr->records = 0;
    addInfoPtr->recs_per_sec = 0;

    unsigned int err = bk_lvl2_input(dsr->crubase, iocode, filename, 0, addInfoPtr);
    if (err)
    {
        return 1;
    }

    // Check the type is PROGRAM
    int type = (addInfoPtr->flags & 0x03);
    if (type) {
        *cmd_type = BIN;
        return allocAndLoad(dsr, iocode, filename, addInfoPtr);
    } else {
        return 1;
    }
}

int allocAndLoad(struct DeviceServiceRoutine* dsr, int iocode, char* filename, struct AddInfo* addInfoPtr) {
    if (sams_total_pages) { // if sams exists
        if (prepareMemory()) {
            return 1;
        }
    }
    return binLoad(dsr, iocode, filename, addInfoPtr);
}

int binLoad(struct DeviceServiceRoutine *dsr, int iocode, char *filename, struct AddInfo *addInfoPtr)
{
    char *cpuAddr = (char *)0xA000;
    unsigned char eof_offset = addInfoPtr->eof_offset;

    int totalBlocks = addInfoPtr->first_sector;
    if (totalBlocks == 0)
    {
        tputs_rom("error, file is empty.\n");
        return 1;
    }
    // Allow the file to be larger, but only load the first 24K.
    if (totalBlocks > 96) {
        totalBlocks = 96;
    }

    int blockId = 0;
    while (blockId < totalBlocks)
    {
        int blk_cnt = totalBlocks - blockId;
        if (blk_cnt > 17) {
            blk_cnt = 17;
        }
        int load_size = blk_cnt << 8;

        addInfoPtr->first_sector = blockId;
        int err = bk_lvl2_input(dsr->crubase, iocode, filename, blk_cnt, addInfoPtr);
        if (err)
        {
            tputs_rom("error reading file: ");
            bk_tputs_ram(bk_uint2hex(err));
            bk_tputc('\n');
            return 1;
        }

        vdpmemread(addInfoPtr->buffer, cpuAddr, load_size);

        // If first block, check that it meets header requirements
        // must start with 0xFCFC
        if (blockId == 0) {
            if (*(int *)cpuAddr != 0xFCFC) {
                return 1;
            }
        }

        cpuAddr += load_size;
        blockId += blk_cnt;
    }
    return 0;
}