#ifndef _PAGE_H
#define _PAGE_H 1

struct __attribute__((__packed__)) Line {
  int length;
  char data[80];
};

void init_page();
void page_clear_lines();
void page_add_line(char* line);

struct Line* page_get_line(int idx);

#endif

