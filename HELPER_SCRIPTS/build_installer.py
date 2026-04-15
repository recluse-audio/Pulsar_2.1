#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def main() -> int:
    installers_dir = Path(__file__).resolve().parents[1] / "INSTALLERS"

    if sys.platform == "darwin":
        script = installers_dir / "MAC" / "build_mac_installer.py"
    elif sys.platform.startswith("win"):
        script = installers_dir / "PC" / "build_pc_installer.py"
    else:
        print(f"Unsupported platform: {sys.platform}", file=sys.stderr)
        return 1

    if not script.exists():
        print(f"Installer script not found: {script}", file=sys.stderr)
        return 1

    print(f"Platform: {sys.platform} -> Running {script.name}")
    return subprocess.run([sys.executable, str(script), *sys.argv[1:]], check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
