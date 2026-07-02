#ifndef _BANKS_H
#define _BANKS_H 1

#ifndef BASE_ADDR
#define BASE_ADDR 0x6000
#define BANK(x) (BASE_ADDR|(x<<1))
#else
#define BANK(x) (BASE_ADDR|x)
#endif

#endif

