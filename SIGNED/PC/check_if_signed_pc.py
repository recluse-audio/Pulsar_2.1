#!/usr/bin/env python3
"""Verify that signed plugin binaries carry the expected digital signature."""

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

    entries = sorted(output_dir.iterdir())
    if not entries:
        print("ERROR: No files found in OUTPUT directory.", file=sys.stderr)
        return 1

    all_passed = True
    found_any = False
    for entry in entries:
        if entry.is_file():
            found_any = True
            if not verify(entry):
                all_passed = False
        elif entry.is_dir() and entry.suffix == ".vst3":
            # VST3 bundle — find the signed binary inside
            inner = entry / "Contents" / "x86_64-win" / entry.name
            if inner.exists():
                found_any = True
                if not verify(inner):
                    all_passed = False
            else:
                print(f"WARNING: VST3 bundle found but no inner binary at {inner}")

    if not found_any:
        print("ERROR: No verifiable files found in OUTPUT directory.", file=sys.stderr)
        return 1

    print("\n===========================")
    if all_passed:
        print("Result: ALL files passed signature verification.")
    else:
        print("Result: One or more files FAILED signature verification.")
    print("===========================")

    return 0 if all_passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
