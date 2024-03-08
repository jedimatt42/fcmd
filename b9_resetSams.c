#include "banks.h"
#define MYBANK BANK(9)

#include "b0_sams.h"

void resetSams() {
    // Reset SAMS if it is in use - no expansion ram can be in use by FC at this point.
    if (sams_total_pages) {
        // preserve lower expansion state so my bank switching cartridge works
        inl_map_page(2, 0xA000);
        inl_map_page(3, 0xB000);
        int* s = (int*)0x2000;
        int* d = (int*)0xA000;
        while (s < (int*)0x4000) {
            *(d++) = *(s++);
        }
        // now swap the copy in and setup 'default' sams page layout
        inl_map_page(2, 0x2000);
        inl_map_page(3, 0x3000);
        inl_map_page(0xA, 0xA000);
        inl_map_page(0xB, 0xB000);
        inl_map_page(0xC, 0xC000);
        inl_map_page(0xD, 0xD000);
        inl_map_page(0xE, 0xE000);
        inl_map_page(0xF, 0xF000);
        // turn off the mapper
        __asm__(
            "LI r12, >1E00\n\t"
            "SBZ 1\n\t"
            : : : "r12"
        );
    }
}
