# SOCKTEST - API test I21

Test for TCP sockets and the buffered socket reader.

Covers: `tcp_connect`, `tcp_send_chars`, `tcp_read_socket`, `tcp_close`,
`sockbuf_init`, `sockbuf_readline`, `sockbuf_readstream`.

## Type

`AUTO` for the failure path and `sockbuf_init`; `COND` for the success path,
which needs a reachable host/port.

## Usage

```
SOCKTEST                 ; failure path only
SOCKTEST <host> [port]   ; also exercise the success path (port defaults to 9641)
```

A companion service is included for the success path. Run it on any host the
TIPI can reach (for example the same machine running the TIPI, or your PC):

```
python3 example/gcc/socktest/server.py --port 9641
```

Then run `SOCKTEST <host> 9641` on the TI. The server sends a two-line banner,
reads the client's `APITEST` line, echoes it, and closes.

**Address note:** the TIPI performs the TCP connect itself, so `localhost` means
the TIPI/Raspberry Pi, not the machine you typed the command from. If the server
runs on your PC, pass the PC's LAN address (for example `SOCKTEST 192.168.1.50
9641`); if it runs on the TIPI, `localhost` works.

## Semantics exercised

- `sockbuf_init` resets the socket id, transport flag, offsets, and both
  buffers.
- The failure path connects to `127.0.0.1:1` (closed) and expects
  `tcp_connect` to return 0.
- The success path connects, sends `APITEST\r\n`, reads a line with
  `sockbuf_readline` and the rest with `sockbuf_readstream`, prints the
  response, and closes the socket. Connect, send, and close must be non-zero.

## Expected result

The final line is `pass=N fail=N skip=N` with `fail=0`; the success path adds
one skip if no host/port was supplied.

## Build and run

```
make -C example/gcc/socktest
```

Copy `SOCKTEST` to a ForceCommand-visible device and run it.
