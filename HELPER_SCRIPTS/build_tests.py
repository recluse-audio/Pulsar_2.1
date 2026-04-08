#!/usr/bin/env python3

from build_complete import beep
from pathlib import Path
import subprocess
import sys


def run(cmd, cwd):
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


def main():
    # Regenerate CMake file lists before building
    regenerate_cmake_lists()

    try:
        run(["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTS=ON", ".."], Path("BUILD"))
        run(["cmake", "--build", ".", "--target", "Tests"], Path("BUILD"))
    except Exception:
        beep(success=False)
        raise

    beep(success=True)


if __name__ == "__main__":
    main()
