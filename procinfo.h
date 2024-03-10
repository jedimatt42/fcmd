#pragma once

struct ProcInfo {
  int base_page;
  struct ProcInfo* prev;
};

extern struct ProcInfo* procInfoPtr;

