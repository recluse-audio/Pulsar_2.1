#!/usr/bin/env python3
"""
Cross-platform CMake build script (macOS / Windows / Linux).

Equivalent to:
  pushd BUILD
  cmake ..
  cmake --build . --target Pulsar_Standalone [--config Debug|Release]
  popd

Usage:
  python build_standalone.py
  python build_standalone.py --config Release
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

from build_complete import find_cmake

PLUGIN_NAME = "Pulsar"


def run(cmd: list[str], cwd: Path) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def regenerate_cmake_lists() -> None:
    regen_script = Path(__file__).parent / "regenSource.py"
    if regen_script.exists():
        print("Regenerating CMake file lists...")
        subprocess.run([sys.executable, str(regen_script)], check=True)
    else:
        print("Warning: regenSource.py not found, skipping regeneration")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", default="Debug", help="Build config: Debug or Release (default: Debug)")
    args = ap.parse_args()

    regenerate_cmake_lists()

    build_dir = Path("BUILD").resolve()
    if not build_dir.exists():
        raise FileNotFoundError("BUILD directory does not exist")

    cmake = find_cmake()
    run([cmake, ".."], cwd=build_dir)

    build_cmd = [cmake, "--build", ".", "--target", f"{PLUGIN_NAME}_Standalone"]
    if sys.platform.startswith("win"):
        build_cmd += ["--config", args.config]

    run(build_cmd, cwd=build_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
