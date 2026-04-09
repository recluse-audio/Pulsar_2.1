#!/usr/bin/env python3

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


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--config",
        choices=["Debug", "Release"],
        default="Release",
        help="Build config (default: Release)",
    )
    return ap.parse_args()


def main() -> int:
    args = parse_args()

    regenerate_cmake_lists()

    root = Path(__file__).resolve().parents[1]
    build_dir = root / "BUILD"
    build_dir.mkdir(parents=True, exist_ok=True)

    cmake = find_cmake()
    print(f"Using cmake: {cmake}")
    print(f"Using config: {args.config}")

    configure_cmd = [
        cmake,
        "-S", str(root),
        "-B", str(build_dir),
    ]

    if not sys.platform.startswith("win"):
        configure_cmd += [f"-DCMAKE_BUILD_TYPE={args.config}"]

    run(configure_cmd, cwd=root)

    build_cmd = [
        cmake,
        "--build", str(build_dir),
        "--target", f"{PLUGIN_NAME}_Standalone",
    ]

    if sys.platform.startswith("win"):
        build_cmd += ["--config", args.config]

    run(build_cmd, cwd=root)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())