# TLSTEST - API test I22

Test for TLS sockets.

Covers: `tls_connect`, `tls_send_chars`, `tls_read_socket`, `tls_close`.

## Type

`AUTO` for the failure path and `sockbuf_init`; `COND` for the success path,
which needs a reachable TLS host.

## Usage

```
TLSTEST                 ; failure path only
TLSTEST <host> [port]   ; also exercise the success path (port defaults to 1965)
```

A companion Gemini server is included:

```
python3 example/gcc/tlstest/tls_server.py --port 1965
```

It generates a throwaway self-signed certificate with `openssl` (pass
`--cert`/`--key` to use your own). The TIPI performs TLS with an unverified
context, so a self-signed certificate is accepted.

**Address note:** the TIPI performs the TLS connect itself, so `localhost` means
the TIPI/Raspberry Pi. If the server runs elsewhere, pass its LAN address.

## Semantics exercised

- `sockbuf_init` with `tls = TLS` sets the transport flag and clears the buffers.
- The failure path connects to `127.0.0.1:1` (closed) and expects `tls_connect`
  to return 0.
- The success path connects, sends `gemini://<host>/\r\n`, reads the response
  header and body, prints them, and closes. Connect, send, and close must be
  non-zero.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`; the success path adds
one skip if no host was supplied or the connect failed.

## Build and run

```
make -C example/gcc/tlstest
```

Copy `TLSTEST` to a ForceCommand-visible device and run it.
