#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import sys
from pathlib import Path


ISCC_CANDIDATES = [
    r"C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    r"C:\Program Files\Inno Setup 6\ISCC.exe",
]


def find_iscc() -> str:
    for candidate in ISCC_CANDIDATES:
        if Path(candidate).exists():
            return candidate
    raise FileNotFoundError(
        "Inno Setup compiler (ISCC.exe) not found. "
        "Install it from https://jrsoftware.org/isdownload.php"
    )


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    iss_source = Path(__file__).resolve().parent / "Pulsar.iss"
    signed_output = root / "SIGNED" / "PC" / "OUTPUT"

    if not iss_source.exists():
        print(f"ISS project file not found: {iss_source}", file=sys.stderr)
        return 1

    # Verify signed binaries exist
    signed_vst3 = signed_output / "Pulsar.vst3"
    if not signed_vst3.exists():
        print(f"ERROR: Signed VST3 not found: {signed_vst3}", file=sys.stderr)
        print("Run sign_builds.py first to sign the plugin binaries.", file=sys.stderr)
        return 1

    # Create a modified .iss that sources from signed output
    iss_text = iss_source.read_text()
    iss_text = iss_text.replace(
        r'#define VST3Source SourcePath + "\..\..\BUILD\Pulsar_artefacts\Release\VST3\Pulsar.vst3"',
        f'#define VST3Source "{signed_output}\\Pulsar.vst3"',
    )

    tmp_iss = Path(__file__).resolve().parent / "Pulsar_signed.iss"
    tmp_iss.write_text(iss_text)

    iscc = find_iscc()
    print(f"Using Inno Setup compiler: {iscc}")
    print(f"Building installer from signed binaries...")

    result = subprocess.run([iscc, str(tmp_iss)], check=False)
    tmp_iss.unlink(missing_ok=True)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
