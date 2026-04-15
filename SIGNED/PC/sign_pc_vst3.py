#!/usr/bin/env python3
"""Sign the Pulsar VST3 plugin binary with Azure Code Signing."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

SIGNTOOL = "signtool.exe"
DLIB = Path.home() / "AppData" / "Local" / "Microsoft" / "MicrosoftArtifactSigningClientTools" / "Azure.CodeSigning.Dlib.dll"
METADATA = Path.home() / ".azure" / "metadata.json"


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    bundle_dir = root / "BUILD" / "Pulsar_artefacts" / "Release" / "VST3" / "Pulsar.vst3"
    source = bundle_dir / "Contents" / "x86_64-win" / "Pulsar.vst3"

    if not source.exists():
        print(f"ERROR: VST3 binary not found: {source}", file=sys.stderr)
        return 1

    output_dir = Path(__file__).resolve().parent / "OUTPUT"
    output_dir.mkdir(parents=True, exist_ok=True)

    # Copy the entire VST3 bundle directory so the installer can use it
    target_bundle = output_dir / "Pulsar.vst3"
    if target_bundle.exists():
        if target_bundle.is_dir():
            shutil.rmtree(target_bundle)
        else:
            target_bundle.unlink()
    print(f"Copying bundle: {bundle_dir} -> {target_bundle}")
    shutil.copytree(bundle_dir, target_bundle)

    # The binary to sign is inside the bundle
    target = target_bundle / "Contents" / "x86_64-win" / "Pulsar.vst3"

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
