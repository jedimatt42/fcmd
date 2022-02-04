#include "banks.h"
#define MYBANK BANK(7)

#include "b7_tipi_log.h"
#include "b7_tipi_msg.h"
#include <string.h>

void tipi_log(char* msg) {
  char logcode = 0x25;
  unsigned int readcount = 0;
  if (tipi_on()) {
    char payload[256];
    payload[0] = logcode;
    bk_strcpy(payload + 1, msg);
    tipi_sendmsg(bk_strlen(payload), payload);
    // contract with TIPI is that this will always read 1 bytes.
    int result;
    tipi_recvmsg(&readcount, (char*)&result);
    tipi_off();
  }
}

