#!/usr/bin/env python3
"""
Cross-platform CMake build script (macOS / Windows / Linux).

Equivalent to:
  rm -rf BUILD
  mkdir BUILD
  cmake ..
  cmake --build . --target <PluginName>

Usage examples:
  python build.py
  python build.py --target Pulsar --config Release
  python build.py --build-dir BUILD --generator "Ninja" --clean
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

# Get plugin name from current working directory
PLUGIN_NAME = Path.cwd().name


def run(cmd: list[str], cwd: Path | None = None) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd) if cwd else None, check=True)


def regenerate_cmake_lists() -> None:
    """Regenerate CMAKE/SOURCES.cmake and CMAKE/TESTS.cmake from directory scan."""
    regen_script = Path(__file__).parent / "regenSource.py"
    if regen_script.exists():
        print("Regenerating CMake file lists...")
        subprocess.run([sys.executable, str(regen_script)], check=True)
    else:
        print("Warning: regenSource.py not found, skipping regeneration")


def is_multi_config_generator(gen: str | None) -> bool:
    """Heuristic: VS and Xcode are multi-config; Ninja/Makefiles are typically single-config."""
    if not gen:
        return False
    g = gen.lower()
    return ("visual studio" in g) or ("xcode" in g) or ("multi-config" in g)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-dir", default=".", help="CMake source dir (default: .)")
    ap.add_argument("--build-dir", default="BUILD", help="Build dir (default: BUILD)")
    ap.add_argument("--target", default=PLUGIN_NAME, help=f"Build target (default: {PLUGIN_NAME})")
    ap.add_argument("--config", default=None, help="Build config (Debug/Release/etc). Needed for VS/Xcode.")
    ap.add_argument("--clean", action="store_true", help="Delete build dir before configuring.")
    ap.add_argument("--generator", default=None, help='CMake generator, e.g. "Ninja", "Unix Makefiles", "Visual Studio 17 2022".')
    ap.add_argument("--parallel", type=int, default=0, help="Parallel jobs (0 = let CMake decide).")
    ap.add_argument("--verbose", action="store_true", help="Enable verbose build output where supported.")
    args, unknown = ap.parse_known_args()

    src_dir = Path(args.source_dir).resolve()
    bld_dir = Path(args.build_dir).resolve()

    # Regenerate CMake file lists before building
    regenerate_cmake_lists()

    if args.clean and bld_dir.exists():
        shutil.rmtree(bld_dir)

    bld_dir.mkdir(parents=True, exist_ok=True)

    # Configure
    cfg_cmd = ["cmake", "-S", str(src_dir), "-B", str(bld_dir)]
    if args.generator:
        cfg_cmd += ["-G", args.generator]
    cfg_cmd += unknown  # allow passing extra -DVAR=VALUE etc.
    run(cfg_cmd)

    # Build
    build_cmd = ["cmake", "--build", str(bld_dir), "--target", args.target]

    # On Windows with Visual Studio (multi-config), you usually must pass --config.
    gen = args.generator
    multi = is_multi_config_generator(gen)

    if sys.platform.startswith("win"):
        if args.config is None:
            args.config = "Debug"
        build_cmd += ["--config", args.config]
    else:
        if args.config is not None or multi:
            build_cmd += ["--config", args.config or "Debug"]

    if args.parallel and args.parallel > 0:
        build_cmd += ["-j", str(args.parallel)]

    if args.verbose:
        build_cmd += ["--verbose"]

    run(build_cmd)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as e:
        print(f"\nBuild failed with exit code {e.returncode}", file=sys.stderr)
        raise
