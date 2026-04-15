#!/usr/bin/env python3
"""Verify that signed Pulsar binaries carry the expected digital signature (macOS)."""

from __future__ import annotations

import sys
from pathlib import Path


def verify(file: Path) -> bool:
    # TODO: implement using codesign --verify --deep --strict
    print(f"\n--- Verifying: {file.name} ---")
    print(f"macOS signature verification not yet implemented for: {file}")
    return False


def main() -> int:
    # If file paths are passed as arguments, verify those specific files
    if len(sys.argv) > 1:
        all_passed = True
        for arg in sys.argv[1:]:
            f = Path(arg)
            if not f.exists():
                print(f"ERROR: File not found: {f}", file=sys.stderr)
                all_passed = False
                continue
            if not verify(f):
                all_passed = False

        print("\n===========================")
        if all_passed:
            print("Result: ALL files passed signature verification.")
        else:
            print("Result: One or more files FAILED signature verification.")
        print("===========================")
        return 0 if all_passed else 1

    # Default: scan OUTPUT/ directory
    output_dir = Path(__file__).resolve().parent / "OUTPUT"

    if not output_dir.exists():
        print(f"ERROR: OUTPUT directory not found: {output_dir}", file=sys.stderr)
        return 1

    files = sorted(output_dir.iterdir())
    if not files:
        print("ERROR: No files found in OUTPUT directory.", file=sys.stderr)
        return 1

    all_passed = True
    for f in files:
        if f.is_file():
            if not verify(f):
                all_passed = False

    print("\n===========================")
    if all_passed:
        print("Result: ALL files passed signature verification.")
    else:
        print("Result: One or more files FAILED signature verification.")
    print("===========================")

    return 0 if all_passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
