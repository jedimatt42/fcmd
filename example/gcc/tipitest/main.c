#include "fctest.h"

/*
 * I20 - TIPI messaging.
 *
 * Covers: tipi_on, tipi_off, tipi_sendmsg, tipi_recvmsg, tipi_log.
 *
 * Skips when no TIPI is detected. Uses the benign TIPI log message (0x25) for
 * the send/receive round trip; the contract is that it always returns one byte.
 */

static void test_tipi(void) {
    char payload[64];
    char resp[4];
    unsigned int len;
    int on;

    on = tipi_on();
    if (!on) {
        FC_SKIP("no TIPI detected");
        return;
    }
    FC_CHECK(on != 0);

    /* disable and re-enable */
    tipi_off();
    FC_CHECK(tipi_on() != 0);

    /* benign log message round trip */
    payload[0] = 0x25;
    str_copy(payload + 1, "APITEST log");
    tipi_sendmsg((unsigned int)str_len(payload), payload);

    len = 0;
    resp[0] = 0;
    tipi_recvmsg(&len, resp);
    tipi_off();

    FC_CHECK_EQ(len, 1);
    term_puts("[OBSERVE] log response byte: ");
    term_puts(hex_from_uint((unsigned int)(unsigned char)resp[0]));
    term_putc('\n');

    /* convenience wrapper */
    tipi_log("APITEST tipi_log");
    FC_OBSERVE("an APITEST log line should appear in the TIPI log");
}

int main(char* args) {
    (void)args;

    term_puts("TIPITEST test\n");

    test_tipi();

    return fc_summary();
}
