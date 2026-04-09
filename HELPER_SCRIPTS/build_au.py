#!/usr/bin/env python3

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

PLUGIN_NAME = "Pulsar"


def find_cmake() -> str:
    candidates: list[str] = []

    if sys.platform == "darwin":
        candidates += [
            "/opt/cmake-3.24.1/CMake.app/Contents/bin/cmake",
            "/opt/cmake/CMake.app/Contents/bin/cmake",
            "/Applications/CMake.app/Contents/bin/cmake",
        ]
    elif sys.platform.startswith("win"):
        candidates += [
            r"C:\Program Files\CMake\bin\cmake.exe",
            r"C:\Program Files (x86)\CMake\bin\cmake.exe",
        ]
    else:
        candidates += [
            "/usr/local/bin/cmake",
            "/usr/bin/cmake",
        ]

    for candidate in candidates:
        if Path(candidate).exists():
            return candidate

    cmake = shutil.which("cmake")
    if cmake:
        return cmake

    raise FileNotFoundError("Could not find cmake on this machine.")


def get_generator() -> str:
    if sys.platform.startswith("win"):
        return "Ninja"
    return "Unix Makefiles"


def run(cmd: list[str], cwd: Path) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def regenerate() -> None:
    regen = Path(__file__).parent / "regenSource.py"
    if regen.exists():
        print("Regenerating CMake file lists...")
        subprocess.run([sys.executable, str(regen)], check=True)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    build = root / "BUILD"
    cmake = find_cmake()
    generator = get_generator()

    print(f"Using cmake: {cmake}")
    print(f"Using generator: {generator}")

    regenerate()

    if build.exists():
        shutil.rmtree(build)

    build.mkdir(parents=True, exist_ok=True)

    run(
        [
            cmake,
            "-S", str(root),
            "-B", str(build),
            "-G", generator,
        ],
        cwd=root,
    )

    build_cmd = [
        cmake,
        "--build", str(build),
        "--target", f"{PLUGIN_NAME}_AU",
    ]

    if sys.platform.startswith("win"):
        build_cmd += ["--config", "Debug"]

    run(build_cmd, cwd=root)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())