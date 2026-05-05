#!/usr/bin/env python3
"""Submit the signed macOS installer to Apple's notary service, then staple.

Reads the signed pkg from SIGNED/MAC/OUTPUT/, submits it via `xcrun notarytool
submit --wait`, and on success runs `xcrun stapler staple` to attach the
notarization ticket to the pkg in-place.

Notarization typically completes in 1-10 minutes for a small plugin pkg, but
can take longer under load. `--wait` blocks until the submission resolves.

On notarization failure, prints the submission ID and how to fetch the
detailed log.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

sys.stdout.reconfigure(line_buffering=True)

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from plugin_info import get_plugin_info

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mac_identity import notary_profile

ROOT = Path(__file__).resolve().parents[2]
INFO = get_plugin_info(ROOT)


def find_signed_pkg() -> Path | None:
    name = INFO["product_name"]
    output_dir = ROOT / "SIGNED" / "MAC" / "OUTPUT"
    matches = sorted(output_dir.glob(f"{name}_v*_macOS_Installer.pkg"))
    return matches[-1] if matches else None


def parse_submission_id(output: str) -> str | None:
    m = re.search(r"id:\s*([0-9a-fA-F-]{36})", output)
    return m.group(1) if m else None


def submit_and_wait(pkg: Path, profile: str) -> tuple[int, str]:
    cmd = [
        "xcrun", "notarytool", "submit",
        str(pkg),
        "--keychain-profile", profile,
        "--wait",
    ]
    print("+", " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)
    output = (result.stdout or "") + (result.stderr or "")
    print(output.rstrip())
    return result.returncode, output


def staple(pkg: Path) -> int:
    cmd = ["xcrun", "stapler", "staple", str(pkg)]
    print("+", " ".join(cmd))
    return subprocess.run(cmd).returncode


def validate_staple(pkg: Path) -> int:
    cmd = ["xcrun", "stapler", "validate", str(pkg)]
    print("+", " ".join(cmd))
    return subprocess.run(cmd).returncode


def main() -> int:
    pkg = find_signed_pkg()
    if pkg is None:
        print("ERROR: no signed pkg found in SIGNED/MAC/OUTPUT/", file=sys.stderr)
        print("Run: python SIGNED/MAC/sign_mac_installer.py", file=sys.stderr)
        return 1

    profile = notary_profile()
    print(f"Notarizing: {pkg}")
    print(f"Notary profile: {profile}")

    rc, output = submit_and_wait(pkg, profile)
    submission_id = parse_submission_id(output)

    accepted = "status: Accepted" in output or "\"status\": \"Accepted\"" in output
    if rc != 0 or not accepted:
        print(f"\nFAIL: notarization did not return Accepted (exit {rc}).", file=sys.stderr)
        if submission_id:
            print(f"\nFetch the detailed log with:", file=sys.stderr)
            print(f"  xcrun notarytool log {submission_id} --keychain-profile \"{profile}\"",
                  file=sys.stderr)
        return rc if rc != 0 else 1

    print("\nNotarization Accepted. Stapling ticket...")
    rc = staple(pkg)
    if rc != 0:
        print(f"\nFAIL: stapler staple exited {rc}", file=sys.stderr)
        return rc

    rc = validate_staple(pkg)
    if rc != 0:
        print(f"\nFAIL: stapler validate exited {rc}", file=sys.stderr)
        return rc

    print(f"\nOK: {pkg.name} is notarized and stapled.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
