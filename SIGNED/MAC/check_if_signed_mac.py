#!/usr/bin/env python3
"""Verify that signed Pulsar binaries carry a Developer ID signature (macOS).

Returns True only when:
  - bundles (.vst3, .component, .app): pass `codesign --verify --deep --strict`
    AND have an Authority of "Developer ID Application:" or "Developer ID Installer:"
    (rejects ad-hoc / linker-signed JUCE defaults)
  - packages (.pkg): pass `pkgutil --check-signature` AND show
    "Developer ID Installer:" in the output

This is used by release_workflow_mac.py:
  - Step 3 (assert builds are NOT signed): expects verify() to return False
    for raw BUILD/ artifacts that are still ad-hoc signed by JUCE.
  - Step 5 (assert builds ARE signed): expects verify() to return True.
  - Step 8 (assert installer signed): expects verify() to return True for the .pkg.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

BUNDLE_SUFFIXES = {".vst3", ".component", ".app"}


def _run(cmd: list[str]) -> tuple[int, str]:
    """Run a command, return (returncode, combined stdout+stderr)."""
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode, (result.stdout or "") + (result.stderr or "")


def _verify_bundle(file: Path) -> bool:
    rc, output = _run(["codesign", "--verify", "--deep", "--strict", "--verbose=2", str(file)])
    print(output.rstrip())
    if rc != 0:
        print(f"FAIL: codesign --verify exited {rc}")
        return False

    rc, output = _run(["codesign", "-dv", "--verbose=2", str(file)])
    if rc != 0:
        print(f"FAIL: codesign -dv exited {rc}")
        return False

    if "Signature=adhoc" in output:
        print("FAIL: bundle is ad-hoc signed (not Developer ID)")
        return False

    has_devid_authority = (
        "Authority=Developer ID Application:" in output
        or "Authority=Developer ID Installer:" in output
    )
    if not has_devid_authority:
        print("FAIL: signature is not from Developer ID Application or Installer")
        print(output.rstrip())
        return False

    for line in output.splitlines():
        if line.startswith(("Authority=", "TeamIdentifier=", "Timestamp=")):
            print(line)
    print("OK: signed with Developer ID")
    return True


def _verify_pkg(file: Path) -> bool:
    rc, output = _run(["pkgutil", "--check-signature", str(file)])
    print(output.rstrip())
    if rc != 0:
        print(f"FAIL: pkgutil --check-signature exited {rc}")
        return False
    if "Developer ID Installer:" not in output:
        print("FAIL: pkg is not signed with Developer ID Installer")
        return False
    print("OK: pkg signed with Developer ID Installer")
    return True


def verify(file: Path) -> bool:
    print(f"\n--- Verifying: {file.name} ---")
    if file.suffix == ".pkg":
        return _verify_pkg(file)
    if file.suffix in BUNDLE_SUFFIXES:
        return _verify_bundle(file)
    print(f"FAIL: unsupported file type for verification: {file.suffix}")
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
        if f.suffix in BUNDLE_SUFFIXES or f.suffix == ".pkg":
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
