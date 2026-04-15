#!/usr/bin/env python3
"""Sign the Windows installer with Azure Code Signing."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from plugin_info import get_plugin_info

ROOT = Path(__file__).resolve().parents[2]
INFO = get_plugin_info(ROOT)
SIGNTOOL = "signtool.exe"
DLIB = Path.home() / "AppData" / "Local" / "Microsoft" / "MicrosoftArtifactSigningClientTools" / "Azure.CodeSigning.Dlib.dll"
METADATA = Path.home() / ".azure" / "metadata.json"


def find_installer(build_dir: Path) -> Path | None:
    """Find the installer exe in the build output directory."""
    candidates = list(build_dir.glob(f"{INFO['product_name']}_v*_Windows_Installer.exe"))
    if not candidates:
        return None
    return max(candidates, key=lambda p: p.stat().st_mtime)


def main() -> int:
    build_dir = ROOT / "INSTALLERS" / "PC" / "BUILD"

    if not build_dir.exists():
        print(f"ERROR: Installer build directory not found: {build_dir}", file=sys.stderr)
        return 1

    source = find_installer(build_dir)
    if source is None:
        print(f"ERROR: No installer exe found in: {build_dir}", file=sys.stderr)
        return 1

    output_dir = Path(__file__).resolve().parent / "OUTPUT"
    output_dir.mkdir(parents=True, exist_ok=True)
    target = output_dir / source.name

    print(f"Copying: {source} -> {target}")
    shutil.copy2(source, target)

    cmd = [
        SIGNTOOL, "sign",
        "/v", "/debug",
        "/fd", "SHA256",
        "/tr", "http://timestamp.acs.microsoft.com",
        "/td", "SHA256",
        "/dlib", str(DLIB),
        "/dmdf", str(METADATA),
        str(target),
    ]

    print(f"Signing: {target}")
    print("+", " ".join(cmd))
    result = subprocess.run(cmd)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
