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
    source = root / "BUILD" / "Pulsar_artefacts" / "Release" / "VST3" / "Pulsar.vst3" / "Contents" / "x86_64-win" / "Pulsar.vst3"

    if not source.exists():
        print(f"ERROR: VST3 binary not found: {source}", file=sys.stderr)
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
