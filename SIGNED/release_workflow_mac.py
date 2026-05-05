#!/usr/bin/env python3
"""macOS release workflow: verify, sign, build installer, notarize, package.

Steps (mirrors release_workflow_pc.py with notarization inserted at step 8.5):
  1. Confirm release builds exist (VST3, AU, Standalone)
  2. Confirm installer project exists
  3. Confirm builds are NOT already signed
  4. Sign release builds (sign_builds.py -> sign_mac_{vst3,AU,standalone}.py)
  5. Verify builds are signed
  6. Build the installer (build_mac_installer.py — version flows in via pkgbuild)
  7. Sign installer (sign_installers.py -> sign_mac_installer.py)
  8. Verify installer is signed
  8.5. Notarize and staple (notarize_mac.py)
  9. Copy all signed+notarized assets to RELEASE/MAC/{version}/
"""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

sys.stdout.reconfigure(line_buffering=True)

from plugin_info import get_plugin_info

ROOT = Path(__file__).resolve().parents[1]
INFO = get_plugin_info(ROOT)


def read_version(version_file: Path) -> str:
    return version_file.read_text().strip()


def read_bundle_version(bundle: Path) -> str:
    """Return CFBundleShortVersionString from a bundle's Info.plist."""
    plist = bundle / "Contents" / "Info.plist"
    result = subprocess.run(
        ["plutil", "-extract", "CFBundleShortVersionString", "raw", str(plist)],
        capture_output=True, text=True, check=True,
    )
    return result.stdout.strip()


def run_step(description: str, cmd: list[str]) -> int:
    print(f"\n{'='*60}")
    print(f"STEP: {description}")
    print(f"{'='*60}")
    print("+", " ".join(cmd))
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print(f"FAILED: {description}")
    return result.returncode


def find_signed_pkg(signed_output: Path, name: str) -> Path | None:
    matches = sorted(signed_output.glob(f"{name}_v*_macOS_Installer.pkg"))
    return matches[-1] if matches else None


def main() -> int:
    python = sys.executable
    name = INFO["product_name"]
    target = INFO["target"]

    version_file = ROOT / "VERSION.txt"
    artefacts = ROOT / "BUILD" / f"{target}_artefacts" / "Release"
    vst3_build = artefacts / "VST3" / f"{name}.vst3"
    au_build = artefacts / "AU" / f"{name}.component"
    standalone_build = artefacts / "Standalone" / f"{name}.app"

    installer_script = ROOT / "INSTALLERS" / "MAC" / "build_mac_installer.py"
    check_script = ROOT / "SIGNED" / "MAC" / "check_if_signed_mac.py"
    sign_builds_script = ROOT / "HELPER_SCRIPTS" / "sign_builds.py"
    sign_installers_script = ROOT / "HELPER_SCRIPTS" / "sign_installers.py"
    notarize_script = ROOT / "SIGNED" / "MAC" / "notarize_mac.py"
    signed_output = ROOT / "SIGNED" / "MAC" / "OUTPUT"

    version = read_version(version_file)
    release_dir = ROOT / "RELEASE" / "MAC" / version

    print(f"Plugin: {name} (target: {target})")
    print(f"Release version: {version}")
    print(f"Release directory: {release_dir}")

    # ── Step 1: Confirm release builds exist & versions match VERSION.txt ──
    print(f"\n[Step 1] Checking release builds exist and match VERSION.txt={version}...")
    for path, label in [(vst3_build, "VST3"), (au_build, "AU"), (standalone_build, "Standalone")]:
        if not path.exists():
            print(f"ERROR: {label} build not found: {path}")
            print("Run: python HELPER_SCRIPTS/rebuild_all.py --config Release")
            return 1
        bundle_version = read_bundle_version(path)
        if bundle_version != version:
            print(f"ERROR: {label} bundle reports CFBundleShortVersionString={bundle_version}, "
                  f"but VERSION.txt is {version}.")
            print(f"  Path: {path}")
            print(f"  The build is stale (linked before the most recent VERSION.txt bump).")
            print(f"  Run: python HELPER_SCRIPTS/rebuild_all.py --config Release")
            return 1
    print("OK: All release builds found, versions match VERSION.txt.")

    # ── Step 2: Confirm installer project exists ──────────────────
    print(f"\n[Step 2] Checking installer project...")
    if not installer_script.exists():
        print(f"ERROR: installer build script not found: {installer_script}")
        return 1
    print("OK: Installer project found.")

    # ── Step 3: Confirm builds are NOT already signed ─────────────
    print(f"\n[Step 3] Verifying builds are NOT already signed...")
    result = subprocess.run(
        [python, str(check_script), str(vst3_build)],
        capture_output=True,
    )
    if result.returncode == 0:
        print("ERROR: Build is already signed. Aborting to prevent double-signing.")
        print("(re-run rebuild_all.py to produce fresh adhoc-signed builds)")
        return 1
    print("OK: Builds are unsigned (expected).")

    # ── Step 4: Sign the release builds ───────────────────────────
    rc = run_step("Sign release builds", [python, str(sign_builds_script)])
    if rc != 0:
        return rc

    # ── Step 5: Verify builds ARE now signed ──────────────────────
    print(f"\n[Step 5] Verifying signed builds...")
    signed_vst3 = signed_output / f"{name}.vst3"
    signed_au = signed_output / f"{name}.component"
    signed_app = signed_output / f"{name}.app"
    for f in [signed_vst3, signed_au, signed_app]:
        if not f.exists():
            print(f"ERROR: Signed bundle not found: {f}")
            return 1
        rc = subprocess.run([python, str(check_script), str(f)]).returncode
        if rc != 0:
            print(f"ERROR: Signature verification failed for {f}")
            return 1
    print("OK: All bundles verified as signed.")

    # ── Step 6: Build installer ───────────────────────────────────
    rc = run_step("Build installer", [python, str(installer_script)])
    if rc != 0:
        return rc

    # ── Step 7: Sign the installer ────────────────────────────────
    rc = run_step("Sign installer", [python, str(sign_installers_script)])
    if rc != 0:
        return rc

    # ── Step 8: Verify installer is signed ────────────────────────
    print(f"\n[Step 8] Verifying installer signature...")
    signed_pkg = find_signed_pkg(signed_output, name)
    if signed_pkg is None:
        print("ERROR: Signed installer not found in OUTPUT directory.")
        return 1
    rc = subprocess.run([python, str(check_script), str(signed_pkg)]).returncode
    if rc != 0:
        print("ERROR: Installer signature verification failed.")
        return rc
    print("OK: Installer verified as signed.")

    # ── Step 8.5: Notarize and staple (mac-only step) ─────────────
    rc = run_step("Notarize and staple installer", [python, str(notarize_script)])
    if rc != 0:
        return rc

    # ── Step 9: Copy signed+notarized assets to RELEASE/MAC/{version}/ ──
    if release_dir.exists() and any(release_dir.iterdir()):
        print(f"ERROR: Release directory already exists and is not empty: {release_dir}")
        print("Remove it manually if you want to re-release this version.")
        return 1

    print(f"\n[Step 9] Copying signed+notarized assets to {release_dir}...")
    release_dir.mkdir(parents=True, exist_ok=True)

    # Bundles (directories)
    for src in [signed_vst3, signed_au, signed_app]:
        dst = release_dir / src.name
        print(f"  {src.name}/ -> {dst}")
        shutil.copytree(src, dst)

    # Installer pkg (file)
    dst_pkg = release_dir / signed_pkg.name
    print(f"  {signed_pkg.name} -> {dst_pkg}")
    shutil.copy2(signed_pkg, dst_pkg)

    print(f"\n{'='*60}")
    print(f"RELEASE COMPLETE: {release_dir}")
    print(f"{'='*60}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
