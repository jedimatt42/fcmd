#include "banking.h"
#define MYBANK BANK(9)

#include "b0_main.h"
#include "b8_terminal.h"
#include "commands.h"
#include "b0_runext.h"
#include "b0_parsing.h"
#include "b1_strutil.h"
#include "b2_dsrutil.h"
#include "b2_lvl2.h"
#include "b8_terminal.h"
#include "b4_variables.h"
#include "b0_globals.h"
#include <vdp.h>

int loadExtension(const char* ext, int* cmd_type);
int loadFromPath(const char *ext, const char *entry, int* cmd_type);
int binLoad(struct DeviceServiceRoutine *dsr, int unit, char *filename, struct AddInfo *addInfoPtr);

#define BIN 0xFCFC
#define SCRIPT 0x0000

void handleExtension(const char *ext)
{
    int cmd_type = 0;
    int err = loadExtension(ext, &cmd_type);

    if (err) {
        tputs_rom("unknown command: ");
        bk_tputs_ram(ext);
        bk_tputc('\n');
        return;
    }

    if (cmd_type == BIN) {
        // Go to bank 0 to actually launch so API tables are visible.
        err = bk_runExtension(ext);

        if (err)
        {
            tputs_rom("error result: ");
            bk_tputs_ram(bk_uint2str(err));
            bk_tputc('\n');
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

int loadExtension(const char* ext, int* cmd_type) {
    // allow ext to be fully qualified path to command
    if (!loadFromPath(ext, "", cmd_type)) {
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

        while(entry[0]) {
            if (!loadFromPath(ext, entry, cmd_type)) {
                return 0;
            }
            cursor = token_cursor(entry, cursor, ';');
        }
        return 1;
    }
}

int loadFromPath(const char *ext, const char *entry, int* cmd_type)
{
    struct DeviceServiceRoutine *dsr = 0;
    char path[256];

    {
        char fullname[256];
        bk_strcpy(fullname, str2ram(entry));
        bk_strcat(fullname, ext);
        bk_parsePathParam(fullname, &dsr, path, PR_REQUIRED);
        if (!dsr) {
            return 1;
        }
    }

    unsigned int unit = bk_path2unitmask(path);

    int parent_idx = bk_lindexof(path, '.', bk_strlen(path) - 1);
    char filename[11];
    bk_strncpy(filename, path + parent_idx + 1, 10);
    path[parent_idx + 1] = 0x00;

    bk_lvl2_setdir(dsr->crubase, unit, path);

    // AddInfo must be in scratchpad
    struct AddInfo *addInfoPtr = (struct AddInfo *)0x8320;
    addInfoPtr->first_sector = 0;
    addInfoPtr->eof_offset = 0;
    addInfoPtr->flags = 0;
    addInfoPtr->rec_length = 0;
    addInfoPtr->records = 0;
    addInfoPtr->recs_per_sec = 0;

    unsigned int err = bk_lvl2_input(dsr->crubase, unit, filename, 0, addInfoPtr);
    if (err)
    {
        return 1;
    }

    // Check the type is PROGRAM
    int type = (addInfoPtr->flags & 0x03);
    if (type) {
        *cmd_type = BIN;
        return binLoad(dsr, unit, filename, addInfoPtr);
    }
    // else it is DISPLAY type record file
    // PATH has had the filename broken off, runScript needs it back on.
    bk_strcpy(path + parent_idx + 1, filename);
    *cmd_type = SCRIPT;
    return !bk_runScript(dsr, path);
}

int binLoad(struct DeviceServiceRoutine *dsr, int unit, char *filename, struct AddInfo *addInfoPtr)
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
        addInfoPtr->first_sector = blockId;
        int err = bk_lvl2_input(dsr->crubase, unit, filename, 1, addInfoPtr);
        if (err)
        {
            tputs_rom("error reading file: ");
            bk_tputs_ram(bk_uint2hex(err));
            bk_tputc('\n');
            return 1;
        }

        vdpmemread(addInfoPtr->buffer, cpuAddr, 256);

        // If first block, check that it meets header requirements
        // must start with 0xFCFC
        if (blockId == 0) {
            if (*(int *)cpuAddr != 0xFCFC) {
                return 1;
            }
        }

        cpuAddr += 256;
        blockId++;
    }
    return 0;
}