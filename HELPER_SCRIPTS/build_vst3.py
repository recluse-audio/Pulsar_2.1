#!/usr/bin/env python3
"""
Cross-platform CMake build script (macOS / Windows / Linux).

Equivalent to:
  pushd BUILD
  cmake ..
  cmake --build . --target Pulsar_VST3
  popd
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

# Plugin name must match the CMake target defined in CMakeLists.txt
PLUGIN_NAME = "Pulsar"


def run(cmd: list[str], cwd: Path) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def regenerate_cmake_lists() -> None:
    """Regenerate CMAKE/SOURCES.cmake and CMAKE/TESTS.cmake from directory scan."""
    regen_script = Path(__file__).parent / "regenSource.py"
    if regen_script.exists():
        print("Regenerating CMake file lists...")
        subprocess.run([sys.executable, str(regen_script)], check=True)
    else:
        print("Warning: regenSource.py not found, skipping regeneration")


def main() -> int:
    # Regenerate CMake file lists before building
    regenerate_cmake_lists()

    build_dir = Path("BUILD").resolve()
    if not build_dir.exists():
        raise FileNotFoundError("BUILD directory does not exist")

    # Configure
    run(["cmake", ".."], cwd=build_dir)

    # Build target
    build_cmd = ["cmake", "--build", ".", "--target", f"{PLUGIN_NAME}_VST3"]

    # Visual Studio / multi-config handling
    if sys.platform.startswith("win"):
        build_cmd += ["--config", "Debug"]

    run(build_cmd, cwd=build_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
