#!/usr/bin/env python3
"""Test digital signature verification scripts across platforms.

Runs the platform-appropriate check script against a known signed binary
(expected: PASS) and a known unsigned binary (expected: FAIL).
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

from plugin_info import get_plugin_info

ROOT = Path(__file__).resolve().parents[1]
INFO = get_plugin_info(ROOT)


def run_check(script: Path, binary: Path) -> int:
    """Run the check script on a binary and return its exit code."""
    cmd = [sys.executable, str(script), str(binary)]
    print("+", " ".join(cmd))
    result = subprocess.run(cmd)
    return result.returncode


def main() -> int:
    name = INFO['product_name']
    target = INFO['target']
    passed = 0
    failed = 0
    skipped = 0

    if sys.platform.startswith("win"):
        check_script = Path(__file__).resolve().parent / "PC" / "check_if_signed_pc.py"
        signed_binary = (
            Path(__file__).resolve().parent / "PC" / "OUTPUT"
            / f"{name}.vst3" / "Contents" / "x86_64-win" / f"{name}.vst3"
        )
        unsigned_binary = (
            ROOT / "BUILD" / f"{target}_artefacts" / "Release" / "VST3"
            / f"{name}.vst3" / "Contents" / "x86_64-win" / f"{name}.vst3"
        )
    elif sys.platform == "darwin":
        check_script = Path(__file__).resolve().parent / "MAC" / "check_if_signed_mac.py"
        signed_binary = Path(__file__).resolve().parent / "MAC" / "OUTPUT" / f"{name}.vst3"
        unsigned_binary = (
            ROOT / "BUILD" / f"{target}_artefacts" / "Release" / "VST3"
            / f"{name}.vst3" / "Contents" / "MacOS" / name
        )
    else:
        print(f"Unsupported platform: {sys.platform}")
        return 1

    if not check_script.exists():
        print(f"ERROR: Check script not found: {check_script}")
        return 1

    # Test 1: signed binary should PASS (exit code 0)
    print(f"\n{'='*60}")
    print("TEST 1: Signed binary should PASS verification")
    print(f"{'='*60}")
    if not signed_binary.exists():
        print(f"SKIP: Signed binary not found: {signed_binary}")
        skipped += 1
    else:
        rc = run_check(check_script, signed_binary)
        if rc == 0:
            print("TEST 1: PASSED (signed binary verified)")
            passed += 1
        else:
            print("TEST 1: FAILED (signed binary was NOT verified)")
            failed += 1

    # Test 2: unsigned binary should FAIL (exit code != 0)
    print(f"\n{'='*60}")
    print("TEST 2: Unsigned binary should FAIL verification")
    print(f"{'='*60}")
    if not unsigned_binary.exists():
        print(f"SKIP: Unsigned binary not found: {unsigned_binary}")
        skipped += 1
    else:
        rc = run_check(check_script, unsigned_binary)
        if rc != 0:
            print("TEST 2: PASSED (unsigned binary correctly rejected)")
            passed += 1
        else:
            print("TEST 2: FAILED (unsigned binary was incorrectly verified as signed)")
            failed += 1

    # Summary
    print(f"\n{'='*60}")
    print(f"RESULTS: {passed} passed, {failed} failed, {skipped} skipped")
    print(f"{'='*60}")

    return 1 if failed > 0 else 0


if __name__ == "__main__":
    raise SystemExit(main())
