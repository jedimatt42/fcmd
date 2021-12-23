
volatile int debug_loop = 1;

void b1_waitfordebugger() {
  debug_loop = 1;
  while(debug_loop) {
  }
}

