#!/usr/bin/env python3

from build_complete import beep, find_cmake
from pathlib import Path
import argparse
import subprocess
import sys


def run(cmd, cwd):
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def regenerate_cmake_lists() -> None:
    regen_script = Path(__file__).parent / "regenSource.py"
    if regen_script.exists():
        print("Regenerating CMake file lists...")
        subprocess.run([sys.executable, str(regen_script)], check=True)
    else:
        print("Warning: regenSource.py not found, skipping regeneration")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", default="Debug", help="Build config: Debug or Release (default: Debug)")
    args = ap.parse_args()

    regenerate_cmake_lists()

    build_dir = Path("BUILD")
    build_dir.mkdir(exist_ok=True)

    cmake = find_cmake()
    try:
        run([cmake, "-DCMAKE_BUILD_TYPE=" + args.config, "-DBUILD_TESTS=ON", ".."], build_dir)

        build_cmd = [cmake, "--build", ".", "--target", "Tests"]
        if sys.platform.startswith("win"):
            build_cmd += ["--config", args.config]
        run(build_cmd, build_dir)
    except Exception:
        beep(success=False)
        raise

    beep(success=True)


if __name__ == "__main__":
    main()
