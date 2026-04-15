#!/usr/bin/env python3
"""
Build and release workflow.

1. Builds all available platform targets in Release config via rebuild_all.py.
2. On success, runs the platform-specific release workflow via release_workflow.py.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent


def run_script(script: Path, extra_args: list[str] | None = None) -> int:
    cmd = [sys.executable, str(script)] + (extra_args or [])
    print(f"\n{'=' * 60}")
    print(f"Running: {script.name} {' '.join(extra_args or [])}")
    print(f"{'=' * 60}\n")
    result = subprocess.run(cmd)
    return result.returncode


def main() -> int:
    rebuild_script = SCRIPTS_DIR / "rebuild_all.py"
    release_script = SCRIPTS_DIR / "release_workflow.py"

    for script in (rebuild_script, release_script):
        if not script.exists():
            print(f"ERROR: required script not found: {script}")
            return 1

    # Step 1: Clean build all targets in Release config
    rc = run_script(rebuild_script, ["--clean", "--config", "Release"])
    if rc != 0:
        print(f"\nERROR: rebuild_all.py failed with exit code {rc}. Aborting release.",
              file=sys.stderr)
        return rc

    print("\nAll targets built successfully. Proceeding to release workflow...")

    # Step 2: Run the release workflow
    rc = run_script(release_script)
    if rc != 0:
        print(f"\nERROR: release_workflow.py failed with exit code {rc}.",
              file=sys.stderr)
        return rc

    print("\nBuild and release workflow completed successfully.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
