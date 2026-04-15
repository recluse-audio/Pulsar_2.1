#!/usr/bin/env python3
"""Verify that signed Pulsar binaries carry the expected digital signature."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

SIGNTOOL = "signtool.exe"


def verify(file: Path) -> bool:
    cmd = [SIGNTOOL, "verify", "/pa", "/v", str(file)]
    print(f"\n--- Verifying: {file.name} ---")
    print("+", " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    if result.returncode == 0:
        print(f"PASS: {file.name} is signed.")
    else:
        print(f"FAIL: {file.name} is NOT signed or signature is invalid.")
    return result.returncode == 0


def main() -> int:
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
