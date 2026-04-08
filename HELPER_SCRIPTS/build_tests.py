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

    build_dir = Path("BUILD")
    build_dir.mkdir(exist_ok=True)

    try:
        run(["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTS=ON", ".."], build_dir)

        build_cmd = ["cmake", "--build", ".", "--target", "Tests"]
        if sys.platform.startswith("win"):
            build_cmd += ["--config", "Debug"]
        run(build_cmd, build_dir)
    except Exception:
        beep(success=False)
        raise

    beep(success=True)


if __name__ == "__main__":
    main()
