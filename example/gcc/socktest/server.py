#!/usr/bin/env python3
"""Tiny TCP service for example/gcc/socktest.

Run this on any host the TIPI can reach, then run the TI test as:

    SOCKTEST <host> <port>

The server sends a short two-line banner on connect (so sockbuf_readline gets a
complete line), reads the client's request line, echoes it back, and closes.
"""

import argparse
import socket


def handle(conn, addr):
    with conn:
        conn.sendall(b"APITEST-SERVER-READY\n")
        conn.sendall(b"APITEST-SERVER-BANNER-2\n")

        conn.settimeout(5.0)
        data = b""
        try:
            while not data.endswith(b"\n"):
                chunk = conn.recv(256)
                if not chunk:
                    break
                data += chunk
        except socket.timeout:
            pass

        line = data.decode("latin-1").rstrip("\r\n")
        conn.sendall(("APITEST-SERVER-ECHO:" + line + "\n").encode("latin-1"))


def main():
    parser = argparse.ArgumentParser(
        description="Test server for example/gcc/socktest"
    )
    parser.add_argument("--host", default="0.0.0.0",
                        help="address to bind (default: all interfaces)")
    parser.add_argument("--port", type=int, default=9641,
                        help="port to listen on (default: 9641)")
    args = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((args.host, args.port))
        srv.listen(1)
        print("listening on %s:%d" % (args.host, args.port), flush=True)
        while True:
            conn, addr = srv.accept()
            print("connection from %s:%d" % addr, flush=True)
            handle(conn, addr)


if __name__ == "__main__":
    main()
