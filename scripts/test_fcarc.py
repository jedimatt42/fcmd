#!/usr/bin/env python3
"""Tests for scripts/fcarc.py.

Run with:  python3 scripts/test_fcarc.py
"""

from __future__ import annotations

import contextlib
import io
import os
import random
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fcarc  # noqa: E402


def ti_program(name: str, data: bytes) -> fcarc.TiFile:
    return fcarc.ti_from_host(data, name, "program")


def ti_content(name: str, data: bytes, flags: int = 0x01,
               recs: int = 0, eof: int = 0, reclen: int = 0,
               l3: int = 0) -> fcarc.TiFile:
    content = fcarc.pad_to(data, fcarc.SECTOR)
    return fcarc.TiFile(name, flags, recs, len(content) // fcarc.SECTOR,
                        eof, reclen, l3, content)


class LzwTests(unittest.TestCase):
    def test_roundtrip(self):
        cases = [
            b"",
            b"A",
            b"AB",
            b"hello hello hello world " * 20,
            bytes(range(256)) * 8,
            b"\0" * 5000,
            bytes(random.randrange(256) for _ in range(5000)),
        ]
        for data in cases:
            with self.subTest(size=len(data)):
                packed = fcarc.lzw_compress(data, pad=1)
                self.assertEqual(fcarc.lzw_decompress(packed), data)

    def test_legacy_padding(self):
        data = b"some text" * 100
        packed = fcarc.lzw_compress(data, pad=fcarc.SECTOR)
        self.assertEqual(len(packed) % fcarc.SECTOR, 0)
        self.assertEqual(fcarc.lzw_decompress(packed), data)


class LegacyTests(unittest.TestCase):
    def test_directory_padding(self):
        for count in (0, 1, 13, 14, 15, 20, 126, 127):
            with self.subTest(count=count):
                files = [
                    ti_content(f"F{i:03d}", b"x")
                    for i in range(count)
                ]
                payload = fcarc.legacy_create(files, compressed=True)
                raw = fcarc.lzw_decompress(payload)
                entries, dirs = fcarc.legacy_parse(raw)
                self.assertEqual(len(entries), count)
                self.assertEqual([e.name for e in entries],
                                 [f.name for f in files])
                for e in entries:
                    self.assertEqual(e.sectors, 1)
                # END! must be at offset 0xFC of the last directory sector
                self.assertEqual(raw[(dirs - 1) * fcarc.SECTOR + 0xFC:
                                     (dirs - 1) * fcarc.SECTOR + 0x100],
                                 b"END!")

    def test_compressed_and_uncompressed(self):
        files = [
            ti_program("ONE", b"hello" * 100),
            ti_program("TWO", bytes(range(256)) * 4),
        ]
        for compressed in (True, False):
            with self.subTest(compressed=compressed):
                payload = fcarc.legacy_create(files, compressed=compressed)
                raw = fcarc.legacy_decompress(payload)
                entries, _ = fcarc.legacy_parse(raw)
                self.assertEqual(len(entries), 2)
                with tempfile.TemporaryDirectory() as td:
                    out = Path(td)
                    written = fcarc.legacy_extract(raw, out, raw_mode=False,
                                                   force=True)
                    self.assertEqual(len(written), 2)
                    for src, name in zip(files, ("ONE", "TWO")):
                        data = (out / name).read_bytes()
                        self.assertEqual(data[0], 0x07)
                        self.assertEqual(data[1:8], b"TIFILES")
                        self.assertEqual(data[128:], src.content)

    def test_selected_extract(self):
        files = [
            ti_program("ONE", b"one" * 10),
            ti_program("TWO", b"two" * 10),
        ]
        payload = fcarc.legacy_create(files, compressed=True)
        raw = fcarc.lzw_decompress(payload)
        with tempfile.TemporaryDirectory() as td:
            out = Path(td)
            written = fcarc.legacy_extract(raw, out, raw_mode=True, force=True,
                                           selected=["TWO"])
            self.assertEqual(len(written), 1)
            self.assertFalse((out / "ONE").exists())
            self.assertTrue((out / "TWO").exists())

    def test_real_samples(self):
        samples = [
            Path("/tmp/opencode/dsr.ark"),
            Path("/tmp/opencode/hit_miss.arc"),
        ]
        if not all(p.exists() for p in samples):
            self.skipTest("real WHTech samples not present")
        for path in samples:
            with self.subTest(path=str(path)):
                payload, _ = fcarc.strip_fiad(path.read_bytes())
                raw = fcarc.legacy_decompress(payload)
                entries, _ = fcarc.legacy_parse(raw)
                self.assertGreater(len(entries), 0)
                for e in entries:
                    self.assertLessEqual(e.offset + e.sectors * fcarc.SECTOR,
                                         len(raw))


class FcarcTests(unittest.TestCase):
    def _build(self, store: bool = False):
        files = [
            ("README", ti_program("README", b"hello fcarc")),
            ("GAMES.ONE", ti_program("ONE", b"game one " * 100)),
            ("GAMES.SUB.TWO", ti_program("TWO", bytes(range(256)) * 3)),
            ("EMPTY", ti_program("EMPTY", b"")),
        ]
        dirs = ["GAMES.SUB", "EMPTYDIR"]
        payload = fcarc.fcarc_create(files, dirs, store=store,
                                     comment="test archive")
        return payload, files, dirs

    def test_roundtrip(self):
        for store in (False, True):
            with self.subTest(store=store):
                payload, files, dirs = self._build(store)
                self.assertEqual(len(payload) % fcarc.SECTOR, 0)
                info, entries = fcarc.fcarc_parse(payload)
                self.assertEqual(info["count"], len(files) + len(dirs))
                self.assertEqual(len(entries), len(files) + len(dirs))
                fcarc.fcarc_test(payload, entries)
                with tempfile.TemporaryDirectory() as td:
                    out = Path(td)
                    fcarc.fcarc_extract(payload, entries, out, [], False, True)
                    self.assertEqual((out / "README").read_bytes()[128:],
                                     files[0][1].content)
                    self.assertEqual((out / "GAMES" / "ONE").read_bytes()[128:],
                                     files[1][1].content)
                    self.assertEqual((out / "GAMES" / "SUB" / "TWO").read_bytes()[128:],
                                     files[2][1].content)
                    self.assertTrue((out / "EMPTYDIR").is_dir())

    def test_selected(self):
        payload, _, _ = self._build()
        _, entries = fcarc.fcarc_parse(payload)
        with tempfile.TemporaryDirectory() as td:
            out = Path(td)
            fcarc.fcarc_extract(payload, entries, out, ["GAMES.SUB"], True, True)
            self.assertTrue((out / "GAMES" / "SUB" / "TWO").exists())
            self.assertFalse((out / "GAMES" / "ONE").exists())
            self.assertFalse((out / "README").exists())

    def test_tifiles_roundtrip(self):
        files = [
            fcarc.ti_from_host(b"program data\n", "PROG", "program"),
            fcarc.ti_from_host(b"A" * 300, "FIX", "intfix", 128),
            fcarc.ti_from_host(b"line one\nline two\n", "VAR", "disvar", 80),
        ]
        for f in files:
            with self.subTest(name=f.name):
                data = f.to_tifiles()
                back = fcarc.ti_from_tifiles(data)
                self.assertEqual(back.flags, f.flags)
                self.assertEqual(back.sectors, f.sectors)
                self.assertEqual(back.eof_offset, f.eof_offset)
                self.assertEqual(back.rec_length, f.rec_length)
                self.assertEqual(back.l3, f.l3)
                self.assertEqual(back.content, f.content)

    def test_malformed(self):
        payload, _, _ = self._build()
        with self.assertRaises(ValueError):
            fcarc.fcarc_parse(payload[:-1])
        with self.assertRaises(ValueError):
            fcarc.fcarc_parse(b"junk" + payload[8:])
        bad = bytearray(payload)
        bad[-128:-120] = b"BADMAGIC"
        with self.assertRaises(ValueError):
            fcarc.fcarc_parse(bytes(bad))
        bad = bytearray(payload)
        info, _ = fcarc.fcarc_parse(payload)
        bad[info["dir_offset"]] ^= 0xFF
        with self.assertRaises(ValueError):
            fcarc.fcarc_parse(bytes(bad))

    def test_path_validation(self):
        for path in ("..", "A..B", "A.", ".A", "A.B/C", "A" * 11,
                     "A" * 10 + "." + "B" * 10 + "." + "C" * 10 + "." + "D" * 10,
                     "A B"):
            with self.subTest(path=path):
                with self.assertRaises(ValueError):
                    fcarc.validate_ti_path(path)
        fcarc.validate_ti_path("GAMES.LEVEL1.FILE")

    def test_create_ti_rejects_directory(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "d").mkdir()
            (root / "d" / "f").write_bytes(b"x")

            def make_ti(host, name):
                return fcarc.read_ti_file(host, name=name)

            with self.assertRaises(ValueError):
                fcarc.collect_sources([root / "d"], recursive=False,
                                      sanitize=True, make_ti=make_ti,
                                      create_ti=True)


class CliTests(unittest.TestCase):
    def test_create_list_extract(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            src = root / "src"
            (src / "sub").mkdir(parents=True)
            (src / "one.txt").write_bytes(b"one\n")
            (src / "sub" / "two.txt").write_bytes(b"two\n")
            archive = root / "test.fca"
            with contextlib.redirect_stdout(io.StringIO()):
                rc = fcarc.main(["create", str(archive), str(src),
                                 "--recursive", "--tifiles"])
            self.assertEqual(rc, 0)
            self.assertTrue(archive.exists())

            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                rc = fcarc.main(["list", str(archive)])
            self.assertEqual(rc, 0)
            self.assertIn("src.one_txt", buf.getvalue())
            self.assertIn("src.sub.two_txt", buf.getvalue())

            out = root / "out"
            with contextlib.redirect_stdout(io.StringIO()):
                rc = fcarc.main(["extract", str(archive), "--out", str(out),
                                 "--raw", "--force"])
            self.assertEqual(rc, 0)
            self.assertEqual((out / "src" / "one_txt").read_bytes(),
                             b"one\n" + b"\0" * 252)

    def test_create_ti_cli_rejects_dir(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "d").mkdir()
            err = io.StringIO()
            with contextlib.redirect_stderr(err):
                rc = fcarc.main(["create-ti", str(root / "x.ark"),
                                 str(root / "d")])
            self.assertNotEqual(rc, 0)
            self.assertIn("directory", err.getvalue())


if __name__ == "__main__":
    unittest.main(verbosity=2)
