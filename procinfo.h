#pragma once

struct __attribute__((__packed__)) ProcInfo {
  int base_page;
  struct ProcInfo* prev;
};

extern struct ProcInfo* procInfoPtr;

