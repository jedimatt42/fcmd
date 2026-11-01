#pragma once

struct ProcInfo {
  int base_page;              // >00
  struct ProcInfo* prev;      // >02

  // Non-local exit context for the currently running executable.
  unsigned int exit_sp;       // >04  parent r10 to restore
  unsigned int exit_resume;   // >06  address of the resume stub (bank 0)
  int exit_status;            // >08
  int exit_active;            // >0A
  unsigned int exit_page;     // >0C  SAMS page to restore at >3000
  unsigned int ret_addr;      // >0E  r11 of the runExecutable() caller
  int stack_page;             // >10  private stack page for this executable
  unsigned int saved_r12;     // >12  callee-saved register for the C caller
};

extern struct ProcInfo* procInfoPtr;

/*
 * Resident copy of the active process context. While an executable runs, its
 * stack page is mapped at >3000, so ProcInfo must not live on that stack.
 * fc_exit and stramp read this through procInfoPtr.
 */
extern struct ProcInfo activeProcInfo;
