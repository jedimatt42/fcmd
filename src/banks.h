#ifndef _BANKS_H
#define _BANKS_H 1

#define BUILD_CONSOLE 1
#define BUILD_CART 0

#if CONSOLE_ROM == BUILD_CART
#define BANK(x) (BASE_ADDR|(x<<1))
#else
#define BANK(x) (BASE_ADDR|x)
#endif

#endif

