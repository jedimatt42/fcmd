#include "fctest.h"

/*
 * I22 - TLS sockets.
 *
 * Covers: tls_connect, tls_send_chars, tls_read_socket, tls_close.
 *
 * The failure path (connect to a closed local port) always runs. The success
 * path only runs when a host is supplied:
 *
 *   TLSTEST <host> [port]     (port defaults to 1965, Gemini)
 *
 * It sends a Gemini request and reads the response. A companion Gemini server
 * with a self-signed certificate is at example/gcc/tlstest/tls_server.py; the
 * TIPI does not verify certificates, so a self-signed cert is fine.
 */

#define SOCK_BAD 2
#define SOCK_GOOD 3

static void test_sockbuf_init(void) {
    struct SocketBuffer sb;

    str_set((char*)&sb, 0xAA, sizeof(sb));
    sockbuf_init(&sb, TLS, 5);

    FC_CHECK_EQ(sb.socket_id, 5);
    FC_CHECK_EQ(sb.tls, TLS);
    FC_CHECK_EQ(sb.available, 0);
    FC_CHECK_EQ(sb.loaded, 0);
    FC_CHECK_EQ(sb.buffer[0], 0);
    FC_CHECK_EQ(sb.lastline[0], 0);
}

static void test_failure_path(void) {
    char host[] = "127.0.0.1";
    char port[] = "1";
    unsigned int res;

    res = tls_connect(SOCK_BAD, host, port);
    FC_CHECK_EQ(res, 0);
}

static void test_success_path(char* host, char* port) {
    struct SocketBuffer sb;
    char request[300];
    char block[256];
    char* line;
    int n;
    unsigned int res;

    sockbuf_init(&sb, TLS, SOCK_GOOD);

    res = tls_connect(SOCK_GOOD, host, port);
    if (!res) {
        term_puts("connect failed for ");
        term_puts(host);
        term_putc(':');
        term_puts(port);
        term_putc('\n');
        term_puts("note: from the TIPI, localhost is the TIPI itself; use the server host's address\n");
        FC_SKIP("could not connect to supplied host:port");
        return;
    }
    FC_CHECK(res != 0);

    str_copy(request, "gemini://");
    str_cat(request, host);
    str_cat(request, "/\r\n");

    res = tls_send_chars(SOCK_GOOD, request, str_len(request));
    FC_CHECK(res != 0);

    line = sockbuf_readline(&sb);
    if (line) {
        term_puts("[OBSERVE] response: ");
        term_puts(line);
        term_putc('\n');
    }
    FC_CHECK(line != 0);

    n = sockbuf_readstream(&sb, block, 128);
    block[n] = 0;
    term_puts("[OBSERVE] body (");
    term_puts(str_from_uint((unsigned int)n));
    term_puts(" bytes): ");
    term_puts(block);
    term_putc('\n');
    FC_CHECK(n >= 0);

    res = tls_close(SOCK_GOOD);
    FC_CHECK(res != 0);
}

int main(char* args) {
    char host[64];
    char port[16];
    char* rest;

    term_puts("TLSTEST test\n");

    test_sockbuf_init();
    test_failure_path();

    rest = str_token_next(host, args, ' ');
    port[0] = 0;
    if (host[0] != 0) {
        str_token_next(port, rest, ' ');
    }

    if (host[0] != 0) {
        if (port[0] == 0) {
            str_copy(port, "1965");
        }
        test_success_path(host, port);
    } else {
        FC_SKIP("no host supplied; run TLSTEST <host> [port] for the success path");
    }

    return fc_summary();
}
