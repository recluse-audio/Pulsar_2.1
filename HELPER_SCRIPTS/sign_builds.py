#!/usr/bin/env python3
"""Sign all plugin builds (VST3, Standalone, AU) for the current platform."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def main() -> int:
    signed_dir = Path(__file__).resolve().parents[1] / "SIGNED"

    if sys.platform == "darwin":
        scripts = [
            signed_dir / "MAC" / "sign_mac_vst3.py",
            signed_dir / "MAC" / "sign_mac_standalone.py",
            signed_dir / "MAC" / "sign_mac_AU.py",
        ]
    elif sys.platform.startswith("win"):
        scripts = [
            signed_dir / "PC" / "sign_pc_vst3.py",
            signed_dir / "PC" / "sign_pc_standalone.py",
        ]
    else:
        print(f"Unsupported platform: {sys.platform}")
        return 1

    for script in scripts:
        if not script.exists():
            print(f"ERROR: signing script not found: {script}")
            return 1
        print(f"\n{'=' * 60}")
        print(f"Running: {script.name}")
        print(f"{'=' * 60}")
        result = subprocess.run([sys.executable, str(script)])
        if result.returncode != 0:
            print(f"FAILED: {script.name} (exit code {result.returncode})")
            return result.returncode

    print(f"\nAll builds signed successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
