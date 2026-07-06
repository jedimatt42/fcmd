#ifndef _DEBUG_WAIT
#define _DEBUG_WAIT 1

inline void debug_wait() {
    volatile int flag = 0;
    while( ! flag ) {
        /* wait */
    }
}

#endif