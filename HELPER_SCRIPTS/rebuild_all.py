#!/usr/bin/env python3
"""
Cross-platform CMake build script (macOS / Windows / Linux).

Builds ALL available plugin formats for the current platform:
  - Standalone (all platforms)
  - VST3       (all platforms)
  - AU         (macOS only)

Usage examples:
  python rebuild_all.py
  python rebuild_all.py --config Release
  python rebuild_all.py --build-dir BUILD --generator "Ninja" --clean
"""

from __future__ import annotations

import argparse
import os
import shutil
import stat
import subprocess
import sys
from pathlib import Path

from build_complete import find_cmake

# Plugin name must match the CMake target defined in CMakeLists.txt
PLUGIN_NAME = "Pulsar"


def run(cmd: list[str], cwd: Path | None = None) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd) if cwd else None, check=True)


def rmtree(path: Path) -> None:
    """Delete a directory tree, clearing read-only bits first (required on Windows for git objects)."""
    def _clear_readonly(func, fpath, _exc):
        os.chmod(fpath, stat.S_IWRITE)
        func(fpath)

    if sys.version_info >= (3, 12):
        shutil.rmtree(path, onexc=_clear_readonly)
    else:
        shutil.rmtree(path, onerror=_clear_readonly)


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


def get_targets() -> list[str]:
    """Return the list of plugin format targets to build for the current platform.

    Pulsar_SharedCode must be first — all format targets link against it.
    """
    targets = [
        f"{PLUGIN_NAME}",          # shared code lib — must build first
        f"{PLUGIN_NAME}_Standalone",
        f"{PLUGIN_NAME}_VST3",
    ]
    if sys.platform == "darwin":
        targets.append(f"{PLUGIN_NAME}_AU")
    return targets


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source-dir", default=".", help="CMake source dir (default: .)")
    ap.add_argument("--build-dir", default="BUILD", help="Build dir (default: BUILD)")
    ap.add_argument("--config", default=None, help="Build config (Debug/Release/etc). Needed for VS/Xcode.")
    ap.add_argument("--clean", action="store_true", help="Delete build dir before configuring.")
    ap.add_argument("--generator", default=None, help='CMake generator, e.g. "Ninja", "Unix Makefiles", "Visual Studio 17 2022".')
    ap.add_argument("--parallel", type=int, default=0, help="Parallel jobs (0 = let CMake decide).")
    ap.add_argument("--verbose", action="store_true", help="Enable verbose build output where supported.")
    args, unknown = ap.parse_known_args()

    src_dir = Path(args.source_dir).resolve()
    bld_dir = Path(args.build_dir).resolve()
    cmake = find_cmake()

    print(f"Using cmake: {cmake}")

    # Regenerate CMake file lists before building
    regenerate_cmake_lists()

    if args.clean and bld_dir.exists():
        rmtree(bld_dir)

    bld_dir.mkdir(parents=True, exist_ok=True)

    # Configure (once)
    cfg_cmd = [cmake, "-S", str(src_dir), "-B", str(bld_dir)]
    if args.generator:
        cfg_cmd += ["-G", args.generator]

    gen = args.generator
    multi = is_multi_config_generator(gen)

    # For single-config generators, set CMAKE_BUILD_TYPE at configure time
    if not multi and not sys.platform.startswith("win"):
        config = args.config or "Release"
        cfg_cmd += [f"-DCMAKE_BUILD_TYPE={config}"]

    cfg_cmd += unknown  # allow passing extra -DVAR=VALUE etc.
    run(cfg_cmd)

    # Build each target
    targets = get_targets()
    print(f"\nBuilding targets: {', '.join(targets)}\n")

    for target in targets:
        build_cmd = [cmake, "--build", str(bld_dir), "--target", target]

        if sys.platform.startswith("win"):
            build_cmd += ["--config", args.config or "Debug"]
        elif multi:
            build_cmd += ["--config", args.config or "Release"]

        if args.parallel and args.parallel > 0:
            build_cmd += ["-j", str(args.parallel)]

        if args.verbose:
            build_cmd += ["--verbose"]

        run(build_cmd)

    print(f"\nAll targets built successfully.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as e:
        print(f"\nBuild failed with exit code {e.returncode}", file=sys.stderr)
        raise