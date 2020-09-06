#ifndef _tipi_msg_h
#define _tipi_msg_h 1

#include "banking.h"


void tipi_on();

void tipi_off();

void tipi_recvmsg(unsigned int* len, unsigned char* buf);

void tipi_sendmsg(unsigned int len, const unsigned char* buf);

DECLARE_BANKED_VOID(tipi_on, BANK(7), bk_tipi_on, (), ())
DECLARE_BANKED_VOID(tipi_off, BANK(7), bk_tipi_off, (), ())
DECLARE_BANKED_VOID(tipi_recvmsg, BANK(7), bk_tipi_recvmsg, (unsigned int *len, unsigned char *buf), (len, buf))
DECLARE_BANKED_VOID(tipi_sendmsg, BANK(7), bk_tipi_sendmsg, (unsigned int len, const unsigned char *buf), (len, buf))

#endif
