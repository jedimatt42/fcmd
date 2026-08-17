#pragma once

struct ProcInfo {
  int base_page;
  struct ProcInfo* prev;

  // Non-local exit context for the currently running executable.
  unsigned int exit_sp;
  unsigned int exit_resume;
  int exit_status;
  int exit_active;
};

extern struct ProcInfo* procInfoPtr;
