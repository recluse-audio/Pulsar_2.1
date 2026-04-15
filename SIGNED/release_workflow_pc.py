#!/usr/bin/env python3
"""Windows release workflow: verify, sign, build installer, package.

Steps:
  1. Confirm release builds exist
  2. Confirm installer project (.iss) exists
  3. Confirm builds are NOT already signed
  4. Sign release builds
  5. Verify builds are signed
  6. Update .iss version, build installer
  7. Sign installer
  8. Verify installer is signed
  9. Copy all signed assets to RELEASE/PC/{version}/
"""

from __future__ import annotations

import re
import shutil
import subprocess
import sys
from pathlib import Path


def read_version(version_file: Path) -> str:
    """Read the release version string from VERSION.txt."""
    return version_file.read_text().strip()


def update_iss_version(iss_path: Path, version_string: str) -> None:
    """Replace the hardcoded MyAppVersion in the .iss file."""
    text = iss_path.read_text()
    text = re.sub(
        r'#define MyAppVersion ".*?"',
        f'#define MyAppVersion "{version_string}"',
        text,
    )
    iss_path.write_text(text)


def run_step(description: str, cmd: list[str]) -> int:
    """Run a subprocess, print status, return exit code."""
    print(f"\n{'='*60}")
    print(f"STEP: {description}")
    print(f"{'='*60}")
    print("+", " ".join(cmd))
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"FAILED: {description}")
    return result.returncode


def find_signed_installer(signed_output: Path) -> Path | None:
    """Find the signed installer executable in the signed output directory."""
    matches = sorted(signed_output.glob("Pulsar_v*_Windows_Installer.exe"))
    return matches[-1] if matches else None


def main() -> int:
    python = sys.executable
    root = Path(__file__).resolve().parents[1]

    # Paths
    version_file = root / "VERSION.txt"
    vst3_build = root / "BUILD" / "Pulsar_artefacts" / "Release" / "VST3" / "Pulsar.vst3"
    vst3_inner = vst3_build / "Contents" / "x86_64-win" / "Pulsar.vst3"
    standalone_build = root / "BUILD" / "Pulsar_artefacts" / "Release" / "Standalone" / "Pulsar.exe"
    iss_file = root / "INSTALLERS" / "PC" / "Pulsar.iss"
    check_script = root / "SIGNED" / "PC" / "check_if_signed_pc.py"
    sign_builds_script = root / "HELPER_SCRIPTS" / "sign_builds.py"
    build_installer_script = root / "INSTALLERS" / "PC" / "build_pc_installer.py"
    sign_installers_script = root / "HELPER_SCRIPTS" / "sign_installers.py"
    signed_output = root / "SIGNED" / "PC" / "OUTPUT"

    # Read version
    version = read_version(version_file)
    release_dir = root / "RELEASE" / "PC" / version

    print(f"Release version: {version}")
    print(f"Release directory: {release_dir}")

    # ── Step 1: Confirm release builds exist ──────────────────────
    print(f"\n[Step 1] Checking release builds exist...")
    for path, label in [(vst3_build, "VST3"), (standalone_build, "Standalone")]:
        if not path.exists():
            print(f"ERROR: {label} build not found: {path}")
            print("Run: python HELPER_SCRIPTS/rebuild_all.py --config Release")
            return 1
    print("OK: Release builds found.")

    # ── Step 2: Confirm installer project exists ──────────────────
    print(f"\n[Step 2] Checking installer project...")
    if not iss_file.exists():
        print(f"ERROR: Inno Setup project not found: {iss_file}")
        return 1
    print("OK: Installer project found.")

    # ── Step 3: Confirm builds are NOT already signed ─────────────
    print(f"\n[Step 3] Verifying builds are NOT already signed...")
    result = subprocess.run(
        [python, str(check_script), str(vst3_inner)],
        capture_output=True,
    )
    if result.returncode == 0:
        print("ERROR: Build is already signed. Aborting to prevent double-signing.")
        return 1
    print("OK: Builds are unsigned (expected).")

    # ── Step 4: Sign the release builds ───────────────────────────
    rc = run_step("Sign release builds", [python, str(sign_builds_script)])
    if rc != 0:
        return rc

    # ── Step 5: Verify builds ARE now signed ──────────────────────
    print(f"\n[Step 5] Verifying signed builds...")
    signed_vst3_bundle = signed_output / "Pulsar.vst3"
    signed_vst3 = signed_vst3_bundle / "Contents" / "x86_64-win" / "Pulsar.vst3"
    signed_exe = signed_output / "Pulsar.exe"
    for f in [signed_vst3, signed_exe]:
        if not f.exists():
            print(f"ERROR: Signed file not found: {f}")
            return 1
        rc = subprocess.run([python, str(check_script), str(f)]).returncode
        if rc != 0:
            print(f"ERROR: Signature verification failed for {f}")
            return 1
    print("OK: All builds verified as signed.")

    # ── Step 6: Update .iss version and build installer ───────────
    print(f"\n[Step 6] Updating Pulsar.iss version to {version}...")
    update_iss_version(iss_file, version)

    rc = run_step("Build installer", [python, str(build_installer_script)])
    if rc != 0:
        return rc

    # ── Step 7: Sign the installer ────────────────────────────────
    rc = run_step("Sign installer", [python, str(sign_installers_script)])
    if rc != 0:
        return rc

    # ── Step 8: Verify installer is signed ────────────────────────
    print(f"\n[Step 8] Verifying installer signature...")
    signed_installer = find_signed_installer(signed_output)
    if signed_installer is None:
        print("ERROR: Signed installer not found in OUTPUT directory.")
        return 1
    rc = subprocess.run([python, str(check_script), str(signed_installer)]).returncode
    if rc != 0:
        print("ERROR: Installer signature verification failed.")
        return rc
    print("OK: Installer verified as signed.")

    # ── Step 9: Copy signed assets to RELEASE/PC/{version}/ ──────
    if release_dir.exists() and any(release_dir.iterdir()):
        print(f"ERROR: Release directory already exists and is not empty: {release_dir}")
        print("Remove it manually if you want to re-release this version.")
        return 1

    print(f"\n[Step 9] Copying signed assets to {release_dir}...")
    release_dir.mkdir(parents=True, exist_ok=True)

    # Copy VST3 bundle (directory)
    dst_vst3 = release_dir / "Pulsar.vst3"
    print(f"  Pulsar.vst3/ -> {dst_vst3}")
    shutil.copytree(signed_vst3_bundle, dst_vst3)

    # Copy standalone and installer (files)
    for src in [signed_exe, signed_installer]:
        dst = release_dir / src.name
        print(f"  {src.name} -> {dst}")
        shutil.copy2(src, dst)

    print(f"\n{'='*60}")
    print(f"RELEASE COMPLETE: {release_dir}")
    print(f"{'='*60}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
