#!/usr/bin/env python3
"""fcarc.py - Force Command Archive (FCARC) host tool.

Dependency-free Python 3 reference implementation for:

  * FCARC v1 archives (with internal directories)
  * the legacy TI Archiver (ARC/ARK) format
  * TI TIFILES FIAD headers

The 4A FCARC executable implements the same formats.  This tool is intended
for creating/extracting archives on a modern computer and for validating the
on-TI implementation.
"""

from __future__ import annotations

import argparse
import os
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Optional

SECTOR = 256
RECORD = 128
FCARC_MAGIC = b"FCARC1\0\0"
FCARC_TRAILER_MAGIC = b"FCARCEOF"
FCARC_VERSION = 1
FCARC_HEADER_SIZE = 128
FCARC_TRAILER_SIZE = 128
FCARC_ENTRY_FIXED = 26
LEGACY_ENTRY_SIZE = 18
LEGACY_MAX_ENTRIES = 127
FCARC_MAX_ENTRIES = 120
MAX_PATH = 39
MAX_COMPONENT = 10


# ---------------------------------------------------------------------------
# small helpers
# ---------------------------------------------------------------------------

def be16(b: bytes) -> int:
    return (b[0] << 8) | b[1]


def le16(b: bytes) -> int:
    return b[0] | (b[1] << 8)


def le32(b: bytes) -> int:
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24)


def p16le(v: int) -> bytes:
    return struct.pack("<H", v & 0xFFFF)


def p32le(v: int) -> bytes:
    return struct.pack("<I", v & 0xFFFFFFFF)


def p16be(v: int) -> bytes:
    return struct.pack(">H", v & 0xFFFF)


def pad_to(data: bytes, size: int, fill: int = 0) -> bytes:
    if size <= 0:
        return data
    rem = len(data) % size
    if rem == 0:
        return data
    return data + bytes([fill]) * (size - rem)


def _make_crc16_table() -> list[int]:
    table = []
    for i in range(256):
        crc = i << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
        table.append(crc)
    return table


_CRC16_TABLE = _make_crc16_table()


def crc16(data: bytes, crc: int = 0xFFFF) -> int:
    for b in data:
        crc = ((crc << 8) & 0xFFFF) ^ _CRC16_TABLE[((crc >> 8) ^ b) & 0xFF]
    return crc


def ensure_ti_component(name: str, sanitize: bool, warn) -> str:
    """Return a TI-safe 1..10 character path component."""
    if name in ("", ".", ".."):
        raise ValueError("invalid empty path component")
    out = []
    for ch in name:
        if ch.isalnum() or ch in "-_; &!@#$%^()+{}[]',":
            out.append(ch)
        else:
            out.append("_")
    text = "".join(out)
    if len(text) > MAX_COMPONENT:
        text = text[:MAX_COMPONENT]
    if not text:
        text = "_"
    if text != name:
        if not sanitize:
            raise ValueError(
                f"name {name!r} is not a valid TI name (use --sanitize to fix it)"
            )
        warn(f"using TI name {text!r} for {name!r}")
    return text


def host_component(component: str) -> str:
    """Escape TI filename characters that are illegal on host filesystems.

    TI allows printable characters such as '/' in file names.  A host
    filesystem does not, so extraction percent-escapes them.  The archive
    itself always stores the original TI name.
    """
    out = []
    for ch in component:
        if ch == '%' or ch in '/\\:*?"<>|' or ord(ch) < 32:
            out.append(f"%{ord(ch):02X}")
        else:
            out.append(ch)
    return "".join(out)


def host_path(path: str) -> Path:
    return Path(*(host_component(p) for p in path.split(".")))


def validate_ti_path(path: str) -> None:
    if not path:
        raise ValueError("empty archive path")
    if len(path) > MAX_PATH:
        raise ValueError(f"archive path too long ({len(path)} > {MAX_PATH}): {path}")
    if path.startswith(".") or path.endswith("."):
        raise ValueError(f"invalid archive path: {path}")
    parts = path.split(".")
    for part in parts:
        if part in ("", ".", ".."):
            raise ValueError(f"invalid archive path component in {path!r}")
        if len(part) > MAX_COMPONENT:
            raise ValueError(f"path component too long in {path!r}")
        if " " in part:
            raise ValueError(f"space in archive path component: {path!r}")
        if any(ch == ' ' or ord(ch) < 32 for ch in part):
            raise ValueError(f"invalid character in archive path component: {path!r}")


# ---------------------------------------------------------------------------
# LZW bitstream (TI Archiver compatible)
# ---------------------------------------------------------------------------

class BitWriter:
    def __init__(self) -> None:
        self.buf = bytearray()
        self.bitbuf = 0
        self.bitcount = 0

    def write(self, value: int, width: int) -> None:
        self.bitbuf = (self.bitbuf << width) | (value & ((1 << width) - 1))
        self.bitcount += width
        while self.bitcount >= 8:
            self.bitcount -= 8
            self.buf.append((self.bitbuf >> self.bitcount) & 0xFF)
            self.bitbuf &= (1 << self.bitcount) - 1

    def finish(self, pad: int = 1) -> bytes:
        if self.bitcount:
            self.buf.append((self.bitbuf << (8 - self.bitcount)) & 0xFF)
            self.bitcount = 0
        data = bytes(self.buf)
        if pad > 1:
            data = pad_to(data, pad)
        return data


class BitReader:
    """MSB-first bit reader matching TIImageTool's LZW reader."""

    def __init__(self, data: bytes, width: int = 9) -> None:
        self.data = data
        self.width = width
        self.initial = width
        self.pos = 0

    def reset_width(self) -> None:
        self.width = self.initial

    def read(self) -> int:
        value = 0
        for _ in range(self.width):
            byte_pos = self.pos >> 3
            bit = self.pos & 7
            if byte_pos >= len(self.data):
                value <<= 1
            else:
                value = (value << 1) | ((self.data[byte_pos] >> (7 - bit)) & 1)
            self.pos += 1
        return value

    def push_back(self) -> None:
        self.pos -= self.width

    def available(self) -> bool:
        return self.pos < len(self.data) * 8

    def increase(self) -> None:
        self.width += 1


def lzw_compress(data: bytes, pad: int = 1) -> bytes:
    """Compress with the TI Archiver LZW variant.

    The width-increase/clear ordering follows TIImageTool's LZW.compress().
    The result is a valid stream even if an implementation uses a smaller
    dictionary; this host implementation uses the full 12-bit dictionary.
    """
    table: dict[bytes, int] = {bytes([i]): i for i in range(256)}
    next_code = 258
    width = 9
    out = BitWriter()
    out.write(256, width)
    pos = 0
    n = len(data)

    while pos < n:
        # longest match in the current dictionary
        w = data[pos:pos + 1]
        length = 1
        while pos + length < n:
            candidate = w + data[pos + length:pos + length + 1]
            if candidate not in table:
                break
            w = candidate
            length += 1
        out.write(table[w], width)
        pos += length
        if pos >= n:
            break
        if (next_code >> width) != 0:
            if width == 12:
                out.write(256, width)
                table = {bytes([i]): i for i in range(256)}
                next_code = 258
                width = 9
                continue
            width += 1
        table[w + data[pos:pos + 1]] = next_code
        next_code += 1

    out.write(257, width)
    return out.finish(pad)


def lzw_decompress(data: bytes) -> bytes:
    """Decompress a TI Archiver LZW stream (ported from TIImageTool)."""
    prefix = [-1] * 4096
    suffix = [0] * 4096
    for i in range(258):
        suffix[i] = i
    next_code = 0x102
    saved_prefix = -1
    out = bytearray()

    def get_sequence(key: int) -> bytes:
        nonlocal next_code, saved_prefix
        if key > next_code:
            raise ValueError(f"invalid LZW code {key:#x}")
        if key == next_code:
            if saved_prefix < 0:
                raise ValueError("invalid LZW first code")
            stack = []
            te = saved_prefix
            while prefix[te] != -1:
                stack.append(suffix[te])
                te = prefix[te]
            stack.append(suffix[te])
            stack.reverse()
            seq = bytes(stack + [stack[0]])
        else:
            stack = []
            te = key
            while prefix[te] != -1:
                stack.append(suffix[te])
                te = prefix[te]
            stack.append(suffix[te])
            stack.reverse()
            seq = bytes(stack)
        if saved_prefix != -1:
            if next_code < 4096:
                prefix[next_code] = saved_prefix
                suffix[next_code] = seq[0]
                next_code += 1
        saved_prefix = key
        return seq

    reader = BitReader(data, 9)
    terminated = False
    while reader.available() and not terminated:
        value = reader.read()
        if value == 256:
            next_code = 0x102
            saved_prefix = -1
            reader.reset_width()
        elif value == 257:
            terminated = True
        else:
            out.extend(get_sequence(value))
            if (next_code >> reader.width) != 0 and reader.width != 12:
                save = reader.pos
                peek = reader.read()
                if peek == 257:
                    terminated = True
                else:
                    reader.pos = save
                    reader.increase()
    if not terminated:
        raise ValueError("LZW stream has no end code")
    return bytes(out)


# ---------------------------------------------------------------------------
# TIFILES files
# ---------------------------------------------------------------------------

@dataclass
class TiFile:
    name: str
    flags: int
    recs_per_sec: int
    sectors: int
    eof_offset: int
    rec_length: int
    l3: int
    content: bytes

    @property
    def is_program(self) -> bool:
        return bool(self.flags & 0x01)

    @property
    def is_internal(self) -> bool:
        return bool(self.flags & 0x02)

    @property
    def is_variable(self) -> bool:
        return bool(self.flags & 0x80)

    def type_name(self) -> str:
        if self.is_program:
            return "PROGRAM"
        prefix = "INT" if self.is_internal else "DIS"
        return prefix + "/" + ("VAR" if self.is_variable else "FIX")

    def meta_bytes(self) -> bytes:
        return bytes([
            self.flags & 0xFF,
            self.recs_per_sec & 0xFF,
            self.eof_offset & 0xFF,
            self.rec_length & 0xFF,
            self.l3 & 0xFF,
            (self.l3 >> 8) & 0xFF,
            (self.sectors >> 8) & 0xFF,
            self.sectors & 0xFF,
        ])

    def to_tifiles(self, name: Optional[str] = None) -> bytes:
        nm = (name if name is not None else self.name)
        nm = nm[:10].ljust(10)
        header = bytearray(128)
        header[0] = 0x07
        header[1:8] = b"TIFILES"
        header[8:10] = p16be(self.sectors)
        header[10] = self.flags & 0xFF
        header[11] = self.recs_per_sec & 0xFF
        header[12] = self.eof_offset & 0xFF
        header[13] = self.rec_length & 0xFF
        header[14:16] = p16le(self.l3)
        header[16:26] = nm.encode("latin-1", "replace")
        content = self.content
        want = self.sectors * SECTOR
        if len(content) < want:
            content = content + b"\0" * (want - len(content))
        elif len(content) > want:
            content = content[:want]
        return bytes(header) + content


def ti_from_tifiles(data: bytes) -> TiFile:
    if len(data) < 128 or data[0] not in (0x07, 0x08) or data[1:8] != b"TIFILES":
        raise ValueError("not a TIFILES file")
    sectors = be16(data[8:10])
    flags = data[10]
    recs = data[11]
    eof = data[12]
    reclen = data[13]
    l3 = le16(data[14:16])
    name = data[16:26].decode("latin-1").rstrip(" \0")
    content = data[128:]
    want = sectors * SECTOR
    if len(content) < want:
        content += b"\0" * (want - len(content))
    else:
        content = content[:want]
    return TiFile(name, flags, recs, sectors, eof, reclen, l3, content)


def _encode_variable(data: bytes, reclen: int) -> bytes:
    """Encode host text as a TI variable-record file body."""
    if not (2 <= reclen <= 254):
        raise ValueError("variable record length must be 2..254")
    out = bytearray()

    def flush_sector() -> None:
        out.append(0xFF)
        while len(out) % SECTOR != 0:
            out.append(0x00)

    for raw_line in data.splitlines(keepends=False):
        line = raw_line.replace(b"\r", b"")
        while len(line) > reclen - 1:
            piece = line[:reclen - 1]
            line = line[reclen - 1:]
            if len(out) % SECTOR + len(piece) + 1 >= SECTOR:
                flush_sector()
            out.append(len(piece))
            out.extend(piece)
        if len(out) % SECTOR + len(line) + 1 >= SECTOR:
            flush_sector()
        out.append(len(line))
        out.extend(line)
    out.append(0xFF)
    while len(out) % SECTOR != 0:
        out.append(0x00)
    return bytes(out)


def ti_from_host(data: bytes, name: str, kind: str = "program",
                 reclen: int = 0) -> TiFile:
    """Create TI metadata for a headerless host file."""
    kind = kind.lower()
    if kind == "program":
        sectors = (len(data) + SECTOR - 1) // SECTOR
        eof = len(data) - (sectors - 1) * SECTOR if sectors else 0
        if eof == SECTOR:
            eof = 0
        return TiFile(name, 0x01, 0, sectors, eof, 0, 0,
                      pad_to(data, SECTOR))
    if kind in ("disvar", "intvar"):
        if reclen <= 0:
            reclen = 80
        body = _encode_variable(data, reclen)
        sectors = len(body) // SECTOR
        eof = 0
        for i in range(sectors * SECTOR - 1, (sectors - 1) * SECTOR - 1, -1):
            if body[i] == 0xFF:
                eof = i - (sectors - 1) * SECTOR
                break
        flags = 0x80 | (0x02 if kind == "intvar" else 0x00)
        return TiFile(name, flags, SECTOR // reclen, sectors, eof, reclen,
                      sectors, body)
    if kind in ("disfix", "intfix"):
        if not (1 <= reclen <= 254):
            raise ValueError("fixed record length must be 1..254")
        sectors = (len(data) + SECTOR - 1) // SECTOR
        recs = SECTOR // reclen
        eof = len(data) - (sectors - 1) * SECTOR if sectors else 0
        if eof == SECTOR:
            eof = 0
        flags = 0x02 if kind == "intfix" else 0x00
        return TiFile(name, flags, recs, sectors, eof, reclen,
                      sectors * recs, pad_to(data, SECTOR))
    raise ValueError(f"unsupported file type {kind!r}")


def read_ti_file(path: Path, kind: str = "program", reclen: int = 0,
                 name: Optional[str] = None) -> TiFile:
    data = path.read_bytes()
    if len(data) >= 128 and data[0] in (0x07, 0x08) and data[1:8] == b"TIFILES":
        ti = ti_from_tifiles(data)
        if name:
            ti.name = name
        return ti
    nm = name or path.name
    return ti_from_host(data, nm, kind, reclen)


# ---------------------------------------------------------------------------
# Legacy TI Archiver format
# ---------------------------------------------------------------------------

@dataclass
class LegacyEntry:
    name: str
    flags: int
    recs_per_sec: int
    sectors: int
    eof_offset: int
    rec_length: int
    l3: int
    offset: int = 0


def legacy_is_compressed(payload: bytes) -> bool:
    return bool(payload) and payload[0] == 0x80


def legacy_decompress(payload: bytes) -> bytes:
    if legacy_is_compressed(payload):
        return lzw_decompress(payload)
    return payload


def legacy_looks_like(payload: bytes) -> bool:
    if not payload:
        return False
    if legacy_is_compressed(payload):
        return True
    for i in range(10):
        off = i * SECTOR + 0xFC
        if off + 4 <= len(payload) and payload[off:off + 4] == b"END!":
            return True
        if off + 4 <= len(payload) and payload[off:off + 4] != b"\0\0\0\0":
            return False
    return False


def legacy_parse(raw: bytes) -> tuple[list[LegacyEntry], int]:
    dir_sectors = 0
    found = False
    for i in range(10):
        off = i * SECTOR + 0xFC
        if off + 4 <= len(raw) and raw[off:off + 4] == b"END!":
            found = True
            break
        if off + 4 > len(raw):
            break
        if raw[off:off + 4] != b"\0\0\0\0":
            break
        dir_sectors += 1
    if not found:
        raise ValueError("not a legacy archive (no END! marker)")
    dir_sectors += 1
    data_start = dir_sectors * SECTOR
    entries: list[LegacyEntry] = []
    pos = 0
    sector = 0
    running = data_start
    while True:
        reserved = sector < dir_sectors - 1 or pos != (dir_sectors - 1) * SECTOR + 0xFC
        if pos >= len(raw):
            raise ValueError("truncated legacy directory")
        if raw[pos] != 0 and reserved:
            if pos + LEGACY_ENTRY_SIZE > len(raw):
                raise ValueError("truncated legacy directory entry")
            name = raw[pos:pos + 10].decode("latin-1").rstrip(" \0")
            flags = raw[pos + 10]
            recs = raw[pos + 11]
            sectors = be16(raw[pos + 12:pos + 14])
            eof = raw[pos + 14]
            reclen = raw[pos + 15]
            l3 = le16(raw[pos + 16:pos + 18])
            entries.append(LegacyEntry(name, flags, recs, sectors, eof,
                                       reclen, l3, running))
            running += sectors * SECTOR
            pos += LEGACY_ENTRY_SIZE
        else:
            sector += 1
            if sector >= dir_sectors:
                break
            pos = sector * SECTOR
    if running > len(raw):
        # Some archives are not padded to the last file's full sector count.
        # Allow it if the data is truncated only in unused tail bytes.
        if len(raw) < data_start:
            raise ValueError("legacy archive is truncated")
    return entries, dir_sectors


def legacy_build_directory(entries: list[LegacyEntry]) -> bytes:
    if len(entries) > LEGACY_MAX_ENTRIES:
        raise ValueError(f"legacy archives support at most {LEGACY_MAX_ENTRIES} files")
    out = bytearray()
    for e in entries:
        name = e.name
        if not name or len(name) > 10:
            raise ValueError(f"invalid legacy file name {name!r}")
        if any(ch in " ." or ord(ch) < 32 or ord(ch) > 126 for ch in name):
            raise ValueError(f"invalid legacy file name {name!r}")
        name = name.encode("latin-1", "replace").ljust(10)
        if len(out) % SECTOR == 252:
            out += b"\0\0\0\0"
        out += name
        out += bytes([e.flags & 0xFF, e.recs_per_sec & 0xFF])
        out += p16be(e.sectors)
        out += bytes([e.eof_offset & 0xFF, e.rec_length & 0xFF])
        out += p16le(e.l3)
    while len(out) % SECTOR != 252:
        out.append(0)
    out += b"END!"
    return bytes(out)


def legacy_create(files: list[TiFile], compressed: bool = True) -> bytes:
    if len(files) > LEGACY_MAX_ENTRIES:
        raise ValueError(f"legacy archives support at most {LEGACY_MAX_ENTRIES} files")
    entries: list[LegacyEntry] = []
    for f in files:
        sectors = len(f.content) // SECTOR
        entries.append(LegacyEntry(f.name, f.flags, f.recs_per_sec, sectors,
                                   f.eof_offset, f.rec_length, f.l3))
    directory = legacy_build_directory(entries)
    raw = bytearray(directory)
    for f in files:
        raw += pad_to(f.content, SECTOR)
    raw = pad_to(bytes(raw), SECTOR)
    if compressed:
        return lzw_compress(raw, pad=SECTOR)
    return raw


def legacy_extract(raw: bytes, outdir: Path, raw_mode: bool,
                   force: bool, selected: Optional[list[str]] = None) -> list[str]:
    entries, _ = legacy_parse(raw)
    selected_up = {s.upper() for s in (selected or [])}
    written = []
    for e in entries:
        if selected_up and e.name.upper() not in selected_up:
            continue
        start = e.offset
        end = start + e.sectors * SECTOR
        content = raw[start:end]
        if len(content) < e.sectors * SECTOR:
            content += b"\0" * (e.sectors * SECTOR - len(content))
        ti = TiFile(e.name, e.flags, e.recs_per_sec, e.sectors, e.eof_offset,
                    e.rec_length, e.l3, content)
        name = e.name
        if name in ("", ".", ".."):
            raise ValueError("invalid legacy file name")
        if any(ord(ch) < 32 for ch in name):
            raise ValueError(f"invalid legacy file name {name!r}")
        if len(name) > 10:
            raise ValueError(f"legacy file name too long: {name!r}")
        out = outdir / host_component(name)
        if out.exists() and not force:
            raise FileExistsError(str(out))
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(content if raw_mode else ti.to_tifiles())
        written.append(str(out))
    return written


# ---------------------------------------------------------------------------
# FCARC v1 format
# ---------------------------------------------------------------------------

@dataclass
class FcarcEntry:
    path: str
    kind: int = 0              # 0 file, 1 directory
    method: int = 0            # 0 stored, 1 LZW
    flags: int = 0
    recs_per_sec: int = 0
    eof_offset: int = 0
    rec_length: int = 0
    l3: int = 0
    sectors: int = 0
    original_size: int = 0
    stored_size: int = 0
    data_offset: int = 0
    crc: int = 0

    def to_bytes(self) -> bytes:
        path = self.path.encode("ascii", "replace")
        if not (1 <= len(path) <= 255):
            raise ValueError("archive path length out of range")
        fixed = struct.pack(
            "<12BIIIH",
            len(path), self.kind, self.method, self.flags & 0xFF,
            self.recs_per_sec & 0xFF, self.eof_offset & 0xFF,
            self.rec_length & 0xFF, (self.sectors >> 8) & 0xFF,
            self.sectors & 0xFF, self.l3 & 0xFF, (self.l3 >> 8) & 0xFF, 0,
            self.original_size, self.stored_size, self.data_offset,
            self.crc & 0xFFFF,
        )
        return fixed + path

    @classmethod
    def from_bytes(cls, data: bytes, pos: int) -> tuple["FcarcEntry", int]:
        if pos + FCARC_ENTRY_FIXED > len(data):
            raise ValueError("truncated FCARC directory entry")
        vals = struct.unpack_from("<12BIIIH", data, pos)
        (plen, kind, method, flags, recs, eof, reclen, shi, slo, l3lo,
         l3hi, _res, orig, stored, offset, crc) = vals
        if plen == 0:
            raise ValueError("empty FCARC path")
        end = pos + FCARC_ENTRY_FIXED + plen
        if end > len(data):
            raise ValueError("truncated FCARC path")
        path = data[pos + FCARC_ENTRY_FIXED:end].decode("ascii", "replace")
        entry = cls(path, kind, method, flags, recs, eof, reclen,
                    l3lo | (l3hi << 8), (shi << 8) | slo, orig, stored,
                    offset, crc)
        return entry, end

    def ti_file(self, content: bytes) -> TiFile:
        return TiFile(self.path.split(".")[-1], self.flags,
                      self.recs_per_sec, self.sectors, self.eof_offset,
                      self.rec_length, self.l3, content)


def fcarc_header(comment: str = "") -> bytes:
    h = bytearray(FCARC_HEADER_SIZE)
    h[0:8] = FCARC_MAGIC
    h[8:10] = p16le(FCARC_VERSION)
    h[10:12] = p16le(0x0003)
    text = comment.encode("latin-1", "replace")[:31]
    h[0x0C:0x0C + len(text)] = text
    return bytes(h)


def fcarc_trailer(dir_offset: int, dir_size: int, count: int,
                  payload_size: int, dir_crc: int) -> bytes:
    t = bytearray(FCARC_TRAILER_SIZE)
    t[0:8] = FCARC_TRAILER_MAGIC
    t[8:10] = p16le(FCARC_VERSION)
    t[10:12] = p16le(0x0003)
    t[12:16] = p32le(dir_offset)
    t[16:20] = p32le(dir_size)
    t[20:22] = p16le(count)
    t[24:28] = p32le(payload_size)
    t[28:30] = p16le(dir_crc)
    return bytes(t)


def fcarc_create(files: list[tuple[str, TiFile]], dirs: Iterable[str],
                 store: bool = False, comment: str = "") -> bytes:
    entries: list[FcarcEntry] = []
    out = bytearray(fcarc_header(comment))

    for path, ti in files:
        validate_ti_path(path)
        out = bytearray(pad_to(bytes(out), RECORD))
        offset = len(out)
        content = pad_to(ti.content, SECTOR)
        if store or not content:
            method = 0
            stored = content
        else:
            method = 1
            stored = lzw_compress(content, pad=1)
        out += stored
        out = bytearray(pad_to(bytes(out), RECORD))
        entries.append(FcarcEntry(
            path=path, kind=0, method=method, flags=ti.flags,
            recs_per_sec=ti.recs_per_sec, eof_offset=ti.eof_offset,
            rec_length=ti.rec_length, l3=ti.l3,
            sectors=len(content) // SECTOR, original_size=len(content),
            stored_size=len(stored), data_offset=offset,
            crc=crc16(content),
        ))

    seen = {e.path.upper() for e in entries}
    for d in dirs:
        validate_ti_path(d)
        key = d.upper()
        if key in seen:
            raise ValueError(f"archive path is both a file and a directory: {d}")
        entries.append(FcarcEntry(path=d, kind=1))
        seen.add(key)

    if len(entries) > FCARC_MAX_ENTRIES:
        raise ValueError(f"FCARC archives support at most {FCARC_MAX_ENTRIES} entries")

    # Reject duplicate paths and file/directory prefix conflicts.  A file
    # path must not contain children; a directory path may.
    for i, a in enumerate(entries):
        ua = a.path.upper()
        for b in entries[:i]:
            ub = b.path.upper()
            if ua == ub:
                raise ValueError(f"duplicate archive path: {a.path}")
            if ua.startswith(ub + "."):
                if b.kind == 0:
                    raise ValueError(
                        f"file {b.path} cannot contain {a.path}")
            elif ub.startswith(ua + "."):
                if a.kind == 0:
                    raise ValueError(
                        f"file {a.path} cannot contain {b.path}")

    dir_bytes = b"".join(e.to_bytes() for e in entries)
    dir_offset = len(out)
    out += dir_bytes
    out = bytearray(pad_to(bytes(out), RECORD))
    if (len(out) + FCARC_TRAILER_SIZE) % SECTOR != 0:
        out += b"\0" * RECORD
    trailer = fcarc_trailer(dir_offset, len(dir_bytes), len(entries),
                            len(out) + FCARC_TRAILER_SIZE, crc16(dir_bytes))
    out += trailer
    if len(out) % SECTOR != 0:
        raise AssertionError("internal error: FCARC payload not sector aligned")
    return bytes(out)


def fcarc_parse(payload: bytes) -> tuple[dict, list[FcarcEntry]]:
    if len(payload) < FCARC_HEADER_SIZE + FCARC_TRAILER_SIZE:
        raise ValueError("FCARC file too short")
    if payload[0:8] != FCARC_MAGIC:
        raise ValueError("not an FCARC archive")
    version = le16(payload[8:10])
    if version != FCARC_VERSION:
        raise ValueError(f"unsupported FCARC version {version}")
    trailer = payload[-FCARC_TRAILER_SIZE:]
    if trailer[0:8] != FCARC_TRAILER_MAGIC:
        raise ValueError("FCARC trailer missing")
    tver = le16(trailer[8:10])
    if tver != FCARC_VERSION:
        raise ValueError(f"unsupported FCARC trailer version {tver}")
    dir_offset = le32(trailer[12:16])
    dir_size = le32(trailer[16:20])
    count = le16(trailer[20:22])
    payload_size = le32(trailer[24:28])
    dir_crc = le16(trailer[28:30])
    if payload_size != len(payload):
        raise ValueError("FCARC payload size mismatch")
    if count > FCARC_MAX_ENTRIES:
        raise ValueError("FCARC entry count too large")
    if dir_offset < FCARC_HEADER_SIZE or dir_size > len(payload):
        raise ValueError("invalid FCARC directory bounds")
    if dir_offset + dir_size > len(payload) - FCARC_TRAILER_SIZE:
        raise ValueError("FCARC directory overlaps trailer")
    dir_bytes = payload[dir_offset:dir_offset + dir_size]
    if crc16(dir_bytes) != dir_crc:
        raise ValueError("FCARC directory checksum mismatch")
    entries = []
    pos = 0
    seen: set[str] = set()
    for _ in range(count):
        entry, pos = FcarcEntry.from_bytes(dir_bytes, pos)
        validate_ti_path(entry.path)
        key = entry.path.upper()
        if key in seen:
            raise ValueError(f"duplicate FCARC path {entry.path}")
        seen.add(key)
        if entry.kind not in (0, 1):
            raise ValueError("invalid FCARC entry kind")
        if entry.method not in (0, 1):
            raise ValueError("unsupported FCARC method")
        if entry.kind == 1:
            if entry.original_size or entry.stored_size or entry.data_offset:
                raise ValueError("invalid FCARC directory entry")
        else:
            if entry.data_offset < FCARC_HEADER_SIZE:
                raise ValueError("invalid FCARC data offset")
            if entry.data_offset % RECORD != 0:
                raise ValueError("FCARC data offset is not record aligned")
            if entry.data_offset + entry.stored_size > dir_offset:
                raise ValueError("FCARC data overlaps directory")
            if entry.original_size % SECTOR != 0:
                raise ValueError("FCARC original size is not sector aligned")
            if entry.sectors * SECTOR != entry.original_size:
                raise ValueError("FCARC sector count does not match original size")
            if entry.method == 0 and entry.stored_size != entry.original_size:
                raise ValueError("stored FCARC entry has wrong stored size")
            if entry.stored_size > entry.original_size + entry.original_size // 2 + 1024:
                raise ValueError("implausible FCARC stored size")
        entries.append(entry)
    if pos > dir_size:
        raise ValueError("FCARC directory overrun")
    return {
        "version": version,
        "flags": le16(payload[10:12]),
        "comment": payload[0x0C:0x2C].decode("latin-1").rstrip("\0 "),
        "dir_offset": dir_offset,
        "dir_size": dir_size,
        "count": count,
        "payload_size": payload_size,
    }, entries


def _selected_match(path: str, selected: list[str]) -> bool:
    if not selected:
        return True
    up = path.upper()
    for sel in selected:
        s = sel.upper()
        if up == s or up.startswith(s + "."):
            return True
    return False


def fcarc_extract(payload: bytes, entries: list[FcarcEntry], outdir: Path,
                  selected: list[str], raw_mode: bool, force: bool) -> list[str]:
    written = []
    for e in entries:
        target = outdir / host_path(e.path)
        if e.kind == 1:
            if _selected_match(e.path, selected) or any(
                    s.upper().startswith(e.path.upper() + ".") for s in selected):
                target.mkdir(parents=True, exist_ok=True)
            continue
        if not _selected_match(e.path, selected):
            continue
        stored = payload[e.data_offset:e.data_offset + e.stored_size]
        if len(stored) != e.stored_size:
            raise ValueError(f"truncated FCARC data for {e.path}")
        if e.method == 0:
            content = stored
        else:
            content = lzw_decompress(stored)
        if len(content) != e.original_size:
            raise ValueError(
                f"size mismatch for {e.path}: got {len(content)}, "
                f"expected {e.original_size}")
        if crc16(content) != e.crc:
            raise ValueError(f"checksum mismatch for {e.path}")
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists() and not force:
            raise FileExistsError(str(target))
        ti = e.ti_file(content)
        target.write_bytes(content if raw_mode else ti.to_tifiles())
        written.append(str(target))
    return written


def fcarc_test(payload: bytes, entries: list[FcarcEntry]) -> None:
    for e in entries:
        if e.kind == 1:
            continue
        stored = payload[e.data_offset:e.data_offset + e.stored_size]
        if len(stored) != e.stored_size:
            raise ValueError(f"truncated FCARC data for {e.path}")
        content = stored if e.method == 0 else lzw_decompress(stored)
        if len(content) != e.original_size:
            raise ValueError(f"size mismatch for {e.path}")
        if crc16(content) != e.crc:
            raise ValueError(f"checksum mismatch for {e.path}")


def fcarc_list(payload: bytes, entries: list[FcarcEntry]) -> None:
    info, _ = fcarc_parse(payload)
    print(f"FCARC v{info['version']}  entries={info['count']}  "
          f"size={info['payload_size']}  comment={info['comment']!r}")
    for e in entries:
        if e.kind == 1:
            print(f"DIR   {e.path}")
        else:
            method = "store" if e.method == 0 else "lzw"
            print(f"{e.original_size:8d}  {method:5s}  {e.path}")


# ---------------------------------------------------------------------------
# Archive wrapper helpers
# ---------------------------------------------------------------------------

def strip_fiad(data: bytes) -> tuple[bytes, Optional[TiFile]]:
    if len(data) >= 128 and data[0] in (0x07, 0x08) and data[1:8] == b"TIFILES":
        ti = ti_from_tifiles(data)
        return data[128:], ti
    return data, None


def wrap_fiad(payload: bytes, name: str, flags: int) -> bytes:
    if len(payload) % SECTOR != 0:
        raise ValueError("archive payload must be a multiple of 256 bytes")
    sectors = len(payload) // SECTOR
    header = bytearray(128)
    header[0] = 0x07
    header[1:8] = b"TIFILES"
    header[8:10] = p16be(sectors)
    header[10] = flags
    header[11] = 2          # I/F 128 -> 2 records per sector
    header[12] = 0
    header[13] = 128
    header[14:16] = p16le(sectors * 2)
    header[16:26] = name[:10].ljust(10).encode("latin-1", "replace")
    return bytes(header) + payload


def detect_format(payload: bytes) -> str:
    if payload[0:8] == FCARC_MAGIC:
        return "fcarc"
    if legacy_looks_like(payload):
        return "legacy"
    raise ValueError("unrecognized archive format (neither FCARC nor TI Archiver)")


# ---------------------------------------------------------------------------
# host-side source collection
# ---------------------------------------------------------------------------

def _warn(msg: str) -> None:
    print(f"warning: {msg}", file=sys.stderr)


def collect_sources(sources: list[Path], recursive: bool, sanitize: bool,
                    make_ti, create_ti: bool) -> tuple[list[tuple[str, TiFile]], list[str]]:
    """Collect files and directory entries for FCARC create.

    Archive paths are relative to each source's parent.  A directory source
    contributes its own name as the top-level archive directory.
    """
    files: list[tuple[str, TiFile]] = []
    dirs: list[str] = []
    seen: set[str] = set()

    def add_file(archive_path: str, host: Path) -> None:
        parts = [ensure_ti_component(p, sanitize, _warn)
                 for p in archive_path.split("/") if p != ""]
        path = ".".join(parts)
        validate_ti_path(path)
        if path.upper() in seen:
            raise ValueError(f"duplicate archive path: {path}")
        seen.add(path.upper())
        files.append((path, make_ti(host, parts[-1])))

    def add_dir(archive_path: str) -> None:
        parts = [ensure_ti_component(p, sanitize, _warn)
                 for p in archive_path.split("/") if p != ""]
        path = ".".join(parts)
        validate_ti_path(path)
        if path.upper() not in seen:
            seen.add(path.upper())
            dirs.append(path)

    for src in sources:
        if not src.exists():
            raise FileNotFoundError(str(src))
        if src.is_dir():
            if create_ti:
                raise ValueError("create-ti does not accept a directory source")
            if not recursive:
                raise ValueError(f"source is a directory: {src} (use --recursive)")
            root_name = ensure_ti_component(src.name, sanitize, _warn)
            add_dir(root_name)
            for dirpath, dirnames, filenames in os.walk(src):
                rel = Path(dirpath).relative_to(src)
                archive_dir = root_name
                if str(rel) != ".":
                    archive_dir += "/" + "/".join(rel.parts)
                    add_dir(archive_dir)
                for fn in filenames:
                    add_file(archive_dir + "/" + fn, Path(dirpath) / fn)
        else:
            add_file(src.name, src)

    return files, dirs


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _load_archive(path: Path) -> tuple[bytes, Optional[TiFile]]:
    return strip_fiad(path.read_bytes())


def cmd_list(args) -> int:
    payload, _ = _load_archive(Path(args.archive))
    fmt = detect_format(payload)
    if fmt == "fcarc":
        _, entries = fcarc_parse(payload)
        fcarc_list(payload, entries)
    else:
        raw = legacy_decompress(payload)
        entries, _ = legacy_parse(raw)
        print(f"TI Archiver  entries={len(entries)}")
        for e in entries:
            print(f"{e.name:10s}  {e.sectors * SECTOR:8d}  "
                  f"flags={e.flags:#04x}  {e.rec_length}")
    return 0


def cmd_test(args) -> int:
    payload, _ = _load_archive(Path(args.archive))
    fmt = detect_format(payload)
    if fmt == "fcarc":
        _, entries = fcarc_parse(payload)
        fcarc_test(payload, entries)
        print(f"ok: FCARC, {len(entries)} entries")
    else:
        raw = legacy_decompress(payload)
        entries, _ = legacy_parse(raw)
        for e in entries:
            end = e.offset + e.sectors * SECTOR
            if end > len(raw):
                raise ValueError(f"truncated legacy data for {e.name}")
        print(f"ok: legacy archive, {len(entries)} entries")
    return 0


def cmd_extract(args) -> int:
    payload, _ = _load_archive(Path(args.archive))
    fmt = detect_format(payload)
    outdir = Path(args.out or ".").resolve()
    outdir.mkdir(parents=True, exist_ok=True)
    selected = [s.replace("/", ".") for s in args.paths]
    if fmt == "fcarc":
        _, entries = fcarc_parse(payload)
        written = fcarc_extract(payload, entries, outdir, selected,
                                args.raw, args.force)
    else:
        raw = legacy_decompress(payload)
        written = legacy_extract(raw, outdir, args.raw, args.force, selected)
    for name in written:
        print(name)
    return 0


def cmd_create(args) -> int:
    def make_ti(host: Path, name: str) -> TiFile:
        return read_ti_file(host, args.type, args.reclen, name)

    files, dirs = collect_sources([Path(s) for s in args.sources],
                                  args.recursive, args.sanitize, make_ti,
                                  create_ti=False)
    payload = fcarc_create(files, dirs, store=args.store, comment=args.comment)
    data = wrap_fiad(payload, Path(args.archive).name, 0x02) if args.tifiles else payload
    Path(args.archive).write_bytes(data)
    print(f"created {args.archive}: {len(files)} files, {len(dirs)} dirs, "
          f"{len(payload)} bytes")
    return 0


def cmd_create_ti(args) -> int:
    def make_ti(host: Path, name: str) -> TiFile:
        return read_ti_file(host, args.type, args.reclen, name)

    files, _dirs = collect_sources([Path(s) for s in args.sources],
                                   recursive=False, sanitize=args.sanitize,
                                   make_ti=make_ti, create_ti=True)
    if len(files) > LEGACY_MAX_ENTRIES:
        raise ValueError(f"legacy archives support at most {LEGACY_MAX_ENTRIES} files")
    payload = legacy_create([f for _, f in files], compressed=not args.uncompressed)
    flags = 0x02 if not args.uncompressed else 0x00
    data = wrap_fiad(payload, Path(args.archive).name, flags) if args.tifiles else payload
    Path(args.archive).write_bytes(data)
    print(f"created {args.archive}: {len(files)} files, {len(payload)} bytes"
          f" ({'uncompressed' if args.uncompressed else 'LZW'})")
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="fcarc.py",
        description="Force Command Archive (FCARC) host tool")
    sub = p.add_subparsers(dest="command", required=True)

    sp = sub.add_parser("list", help="list archive contents")
    sp.add_argument("archive")
    sp.set_defaults(func=cmd_list)

    sp = sub.add_parser("test", help="verify archive integrity")
    sp.add_argument("archive")
    sp.set_defaults(func=cmd_test)

    sp = sub.add_parser("extract", help="extract archive contents")
    sp.add_argument("archive")
    sp.add_argument("paths", nargs="*",
                    help="archive paths to extract (default: all)")
    sp.add_argument("--out", "-o", default=None, help="output directory")
    sp.add_argument("--raw", action="store_true",
                    help="write headerless files instead of TIFILES FIADs")
    sp.add_argument("--force", "-f", action="store_true",
                    help="overwrite existing files")
    sp.set_defaults(func=cmd_extract)

    sp = sub.add_parser("create", help="create an FCARC archive")
    sp.add_argument("archive")
    sp.add_argument("sources", nargs="+")
    sp.add_argument("--recursive", "-r", action="store_true",
                    help="recurse into directory sources")
    sp.add_argument("--store", action="store_true",
                    help="do not compress file data")
    sp.add_argument("--tifiles", action="store_true",
                    help="wrap the archive in a TIFILES FIAD header")
    sp.add_argument("--comment", default="", help="archive comment")
    sp.add_argument("--sanitize", action="store_true", default=True,
                    help="fix non-TI source names (default)")
    sp.add_argument("--no-sanitize", action="store_false", dest="sanitize",
                    help="reject non-TI source names")
    sp.add_argument("--type", default="program",
                    choices=["program", "intfix", "disfix", "intvar", "disvar"],
                    help="type for headerless source files")
    sp.add_argument("--reclen", type=int, default=0,
                    help="record length for headerless fixed/variable sources")
    sp.set_defaults(func=cmd_create)

    sp = sub.add_parser("create-ti", help="create a legacy TI Archiver archive")
    sp.add_argument("archive")
    sp.add_argument("sources", nargs="+")
    sp.add_argument("--uncompressed", action="store_true",
                    help="write an uncompressed DIS/FIX 128 archive")
    sp.add_argument("--tifiles", action="store_true",
                    help="wrap the archive in a TIFILES FIAD header")
    sp.add_argument("--sanitize", action="store_true", default=True)
    sp.add_argument("--no-sanitize", action="store_false", dest="sanitize")
    sp.add_argument("--type", default="program",
                    choices=["program", "intfix", "disfix", "intvar", "disvar"])
    sp.add_argument("--reclen", type=int, default=0)
    sp.set_defaults(func=cmd_create_ti)
    return p


def main(argv: Optional[list[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except (OSError, ValueError, FileExistsError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
