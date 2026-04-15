#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "SIGNED"))
from plugin_info import get_plugin_info

ROOT = Path(__file__).resolve().parents[2]
INFO = get_plugin_info(ROOT)

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
    name = INFO['product_name']
    target = INFO['target']
    signed_output = ROOT / "SIGNED" / "PC" / "OUTPUT"
    iss_source = Path(__file__).resolve().parent / f"{name}.iss"

    if not iss_source.exists():
        print(f"ISS project file not found: {iss_source}", file=sys.stderr)
        return 1

    signed_vst3 = signed_output / f"{name}.vst3"
    if not signed_vst3.exists():
        print(f"ERROR: Signed VST3 not found: {signed_vst3}", file=sys.stderr)
        print("Run sign_builds.py first to sign the plugin binaries.", file=sys.stderr)
        return 1

    # Create a modified .iss that sources from signed output
    iss_text = iss_source.read_text()
    # Replace the VST3Source define to point at signed output
    original_vst3_source = f'#define VST3Source SourcePath + "\\..\\..\\BUILD\\{target}_artefacts\\Release\\VST3\\{name}.vst3"'
    new_vst3_source = f'#define VST3Source "{signed_output}\\{name}.vst3"'
    iss_text = iss_text.replace(original_vst3_source, new_vst3_source)

    tmp_iss = Path(__file__).resolve().parent / f"{name}_signed.iss"
    tmp_iss.write_text(iss_text)

    iscc = find_iscc()
    print(f"Using Inno Setup compiler: {iscc}")
    print(f"Building installer from signed binaries...")

    result = subprocess.run([iscc, str(tmp_iss)], check=False)
    tmp_iss.unlink(missing_ok=True)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
