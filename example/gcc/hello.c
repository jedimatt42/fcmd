
#define FC_TRAMPOLINE *(int*)0x6080
#define FC_TPUTC 0x6082

#define RR0 *((volatile int*)0x8300)

void tputc(int value);

static inline void fc_tputc(int value) {
    RR0 = FC_TPUTC;
    void (*tramp)(int value) = (void*) FC_TRAMPOLINE;
    tramp(value);
}

int main(char* args) {
   fc_tputc('!');
   return 0;
}

