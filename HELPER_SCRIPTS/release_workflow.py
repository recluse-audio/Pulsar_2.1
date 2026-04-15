#!/usr/bin/env python3
"""Release workflow dispatcher -- delegates to platform-specific release script."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


def main() -> int:
    signed_dir = Path(__file__).resolve().parents[1] / "SIGNED"

    if sys.platform == "darwin":
        script = signed_dir / "release_workflow_mac.py"
    elif sys.platform.startswith("win"):
        script = signed_dir / "release_workflow_pc.py"
    else:
        print(f"Unsupported platform: {sys.platform}")
        return 1

    if not script.exists():
        print(f"ERROR: release workflow script not found: {script}")
        return 1

    print(f"Platform: {sys.platform} -> Running {script.name}")
    return subprocess.run([sys.executable, str(script)]).returncode


if __name__ == "__main__":
    raise SystemExit(main())
