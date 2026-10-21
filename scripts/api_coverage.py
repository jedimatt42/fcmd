#!/usr/bin/env python3
"""Audit ForceCommand API coverage by the example/gcc test programs.

Each name in fc_api.lst is searched for as a whole word in the example/gcc
sources (*.c/*.h). The fcsdk directory (generated header) and build objects
are skipped.

Usage:
  scripts/api_coverage.py                 # per-API coverage list
  scripts/api_coverage.py --missing       # only uncovered names
  scripts/api_coverage.py --verbose       # show referencing files
  scripts/api_coverage.py --groups        # per-prefix summary
  scripts/api_coverage.py --require-all   # exit 1 if anything is uncovered
  scripts/api_coverage.py gfx_circle tui_win_open   # audit specific names
"""

import argparse
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
APILIST = os.path.join(ROOT, "fc_api.lst")
EXAMPLES = os.path.join(ROOT, "example", "gcc")
SKIP_DIRS = {"fcsdk", "objects"}


def load_api():
    names = []
    with open(APILIST, encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if s and not s.startswith("#"):
                names.append(s)
    return names


def collect_sources():
    sources = []
    for dirpath, dirnames, filenames in os.walk(EXAMPLES):
        dirnames[:] = sorted(d for d in dirnames if d not in SKIP_DIRS)
        for fn in sorted(filenames):
            if fn.endswith((".c", ".h")):
                sources.append(os.path.join(dirpath, fn))
    return sources


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("names", nargs="*", help="limit the audit to these API names")
    ap.add_argument("--missing", action="store_true", help="print only uncovered names")
    ap.add_argument("--verbose", "-v", action="store_true", help="show referencing files")
    ap.add_argument("--groups", action="store_true", help="print a per-prefix summary")
    ap.add_argument("--require-all", action="store_true",
                    help="exit 1 if any audited API is uncovered")
    args = ap.parse_args()

    api = load_api()
    if args.names:
        api = args.names

    texts = {}
    for path in collect_sources():
        with open(path, encoding="utf-8", errors="replace") as f:
            texts[path] = f.read()

    coverage = {}
    for name in api:
        pattern = re.compile(r"\b" + re.escape(name) + r"\b")
        coverage[name] = sorted(
            os.path.relpath(path, ROOT)
            for path, text in texts.items()
            if pattern.search(text)
        )

    covered = [n for n, refs in coverage.items() if refs]
    missing = [n for n, refs in coverage.items() if not refs]

    print(f"API entries: {len(coverage)}")
    print(f"covered:     {len(covered)}")
    print(f"uncovered:   {len(missing)}")

    if args.groups:
        groups = {}
        for name in coverage:
            group = name.split("_", 1)[0]
            total, hit = groups.get(group, (0, 0))
            groups[group] = (total + 1, hit + (1 if coverage[name] else 0))
        print("\nby prefix:")
        for group in sorted(groups):
            total, hit = groups[group]
            print(f"  {group:10s} {hit:3d}/{total:3d}")

    if args.missing:
        for name in missing:
            print(name)
    else:
        print("\ncoverage:")
        for name in coverage:
            refs = coverage[name]
            if refs:
                if args.verbose:
                    print(f"  {name}: {', '.join(refs)}")
                else:
                    print(f"  {name}")
            else:
                print(f"  {name}: MISSING")

    if args.require_all and missing:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
