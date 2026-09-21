#!/usr/bin/env python3
"""Tiny TLS (Gemini) service for example/gcc/tlstest.

The TIPI performs TLS with an unverified context, so a self-signed certificate
is accepted. By default this generates a throwaway self-signed certificate with
openssl; pass --cert/--key to use your own.

    python3 example/gcc/tlstest/tls_server.py --port 1965
    TLSTEST <host> 1965
"""

import argparse
import os
import socket
import ssl
import subprocess
import tempfile


def make_self_signed():
    tmp = tempfile.mkdtemp(prefix="apitest-tls-")
    cert = os.path.join(tmp, "cert.pem")
    key = os.path.join(tmp, "key.pem")
    subprocess.run(
        ["openssl", "req", "-x509", "-newkey", "rsa:2048", "-nodes",
         "-keyout", key, "-out", cert, "-days", "1", "-subj", "/CN=apitest"],
        check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return cert, key


def handle(conn):
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
    print("request: %r" % data.decode("latin-1").rstrip("\r\n"), flush=True)
    conn.sendall(b"20 text/gemini\r\n")
    conn.sendall(b"APITEST-TLS-SERVER\n")


def main():
    parser = argparse.ArgumentParser(
        description="Test TLS/Gemini server for example/gcc/tlstest"
    )
    parser.add_argument("--host", default="0.0.0.0",
                        help="address to bind (default: all interfaces)")
    parser.add_argument("--port", type=int, default=1965,
                        help="port to listen on (default: 1965)")
    parser.add_argument("--cert", help="PEM certificate (default: self-signed)")
    parser.add_argument("--key", help="PEM private key (default: self-signed)")
    args = parser.parse_args()

    cert, key = args.cert, args.key
    if not cert or not key:
        cert, key = make_self_signed()

    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(cert, key)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((args.host, args.port))
        srv.listen(1)
        print("listening on %s:%d" % (args.host, args.port), flush=True)
        while True:
            conn, addr = srv.accept()
            print("connection from %s:%d" % addr, flush=True)
            try:
                tls = context.wrap_socket(conn, server_side=True)
                handle(tls)
                tls.close()
            except Exception as exc:  # noqa: BLE001
                print("error: %s" % exc, flush=True)


if __name__ == "__main__":
    main()
