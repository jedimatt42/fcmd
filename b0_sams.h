#ifndef SAMS_H
#define SAMS_H 1

#include "banking.h"

extern int sams_next_page;
extern int sams_total_pages;

// determine how many SAMS pages are available, or 0 if no SAMS detected.
int init_sams();

// map a logical page number into the target address
void map_page(int page, int addr);

// return first logical page number of allocated block of pages.
// or -1 if not enough pages are available.
int alloc_pages(int count);

// return highest pages back to system. If you allocated 3, and free 2, you still have your first page.
void free_pages(int count);

DECLARE_BANKED_VOID(map_page, BANK(0), bk_map_page, (int page, int addr), (page, addr))
DECLARE_BANKED(alloc_pages, BANK(0), int, bk_alloc_pages, (int count), (count))
DECLARE_BANKED_VOID(free_pages, BANK(0), bk_free_pages, (int count), (count))

#endif