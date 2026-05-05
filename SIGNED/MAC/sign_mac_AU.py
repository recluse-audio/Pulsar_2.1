#!/usr/bin/env python3
"""Sign the macOS Audio Unit (.component) bundle with the Developer ID Application cert."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

sys.stdout.reconfigure(line_buffering=True)

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from plugin_info import get_plugin_info

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mac_identity import developer_id_application

ROOT = Path(__file__).resolve().parents[2]
INFO = get_plugin_info(ROOT)


def main() -> int:
    identity = developer_id_application()
    name = INFO["product_name"]
    target = INFO["target"]

    bundle_dir = ROOT / "BUILD" / f"{target}_artefacts" / "Release" / "AU" / f"{name}.component"
    if not bundle_dir.exists():
        print(f"ERROR: AU bundle not found: {bundle_dir}", file=sys.stderr)
        print("Run: python HELPER_SCRIPTS/build_au.py", file=sys.stderr)
        return 1

    output_dir = Path(__file__).resolve().parent / "OUTPUT"
    output_dir.mkdir(parents=True, exist_ok=True)

    target_bundle = output_dir / f"{name}.component"
    if target_bundle.exists():
        shutil.rmtree(target_bundle)
    print(f"Copying bundle: {bundle_dir} -> {target_bundle}")
    shutil.copytree(bundle_dir, target_bundle)

    cmd = [
        "codesign",
        "--force",
        "--timestamp",
        "--options", "runtime",
        "--sign", identity,
        str(target_bundle),
    ]
    print(f"Signing: {target_bundle}")
    print("+", " ".join(cmd))
    return subprocess.run(cmd).returncode


if __name__ == "__main__":
    raise SystemExit(main())
