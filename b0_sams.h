#ifndef SAMS_H
#define SAMS_H 1

extern int sams_next_page;
extern int sams_total_pages;

// determine how many SAMS pages are available, or 0 if no SAMS detected.
int init_sams();

// map a logical page number into the target address
void map_page(int page, int addr);

// return first logical page number of allocated block of pages.
// or -1 if not enough pages are available.
int alloc_pages(int count);

#endif