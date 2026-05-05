#!/usr/bin/env python3
"""Sign the unsigned macOS .pkg installer with the Developer ID Installer cert.

Reads the unsigned pkg from INSTALLERS/MAC/BUILD/, writes the signed copy
to SIGNED/MAC/OUTPUT/. Uses `productsign` (not `codesign`) since pkg signing
uses a different mechanism than bundle signing.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

sys.stdout.reconfigure(line_buffering=True)

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from plugin_info import get_plugin_info

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mac_identity import developer_id_installer

ROOT = Path(__file__).resolve().parents[2]
INFO = get_plugin_info(ROOT)


def find_unsigned_pkg() -> Path | None:
    name = INFO["product_name"]
    build_dir = ROOT / "INSTALLERS" / "MAC" / "BUILD"
    matches = sorted(build_dir.glob(f"{name}_v*_macOS_Installer.pkg"))
    return matches[-1] if matches else None


def main() -> int:
    identity = developer_id_installer()
    unsigned_pkg = find_unsigned_pkg()
    if unsigned_pkg is None:
        print(f"ERROR: no unsigned pkg found in INSTALLERS/MAC/BUILD/", file=sys.stderr)
        print("Run: python INSTALLERS/MAC/build_mac_installer.py", file=sys.stderr)
        return 1

    output_dir = Path(__file__).resolve().parent / "OUTPUT"
    output_dir.mkdir(parents=True, exist_ok=True)
    signed_pkg = output_dir / unsigned_pkg.name

    if signed_pkg.exists():
        signed_pkg.unlink()

    cmd = [
        "productsign",
        "--sign", identity,
        "--timestamp",
        str(unsigned_pkg),
        str(signed_pkg),
    ]
    print(f"Signing: {unsigned_pkg} -> {signed_pkg}")
    print("+", " ".join(cmd))
    return subprocess.run(cmd).returncode


if __name__ == "__main__":
    raise SystemExit(main())
