#!/usr/bin/env python3
"""Build the macOS distribution .pkg installer (unsigned).

Sources signed bundles from SIGNED/MAC/OUTPUT/, builds three component pkgs
in a temp dir, then composes them into a single distribution pkg via
productbuild + a generated distribution.xml.

Output: INSTALLERS/MAC/BUILD/{product}_v{version}_macOS_Installer.pkg

The output is UNSIGNED — sign_mac_installer.py is the next step (Phase 10).
This mirrors the PC pattern (build_pc_installer.py builds unsigned, then
sign_pc_installer.py signs).

Install layout (system-wide, requires admin auth):
  /Library/Audio/Plug-Ins/VST3/Pulsar.vst3
  /Library/Audio/Plug-Ins/Components/Pulsar.component
  /Applications/Pulsar.app
"""

from __future__ import annotations

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.stdout.reconfigure(line_buffering=True)

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "SIGNED"))
from plugin_info import get_plugin_info

ROOT = Path(__file__).resolve().parents[2]
INFO = get_plugin_info(ROOT)

COMPONENTS = [
    {
        "id": "vst3",
        "bundle_suffix": ".vst3",
        "install_dir": "Library/Audio/Plug-Ins/VST3",
        "title": "VST3 Plugin",
        "description": "Install Pulsar.vst3 to /Library/Audio/Plug-Ins/VST3/",
    },
    {
        "id": "au",
        "bundle_suffix": ".component",
        "install_dir": "Library/Audio/Plug-Ins/Components",
        "title": "Audio Unit (AU) Plugin",
        "description": "Install Pulsar.component to /Library/Audio/Plug-Ins/Components/",
    },
    {
        "id": "standalone",
        "bundle_suffix": ".app",
        "install_dir": "Applications",
        "title": "Standalone App",
        "description": "Install Pulsar.app to /Applications/",
    },
]


def read_version() -> str:
    return (ROOT / "VERSION.txt").read_text().strip()


def component_identifier(name: str, component_id: str) -> str:
    return f"com.recluseaudio.{name.lower()}.{component_id}"


def build_component_pkg(component: dict, name: str, version: str,
                        signed_output: Path, work_dir: Path) -> Path:
    bundle_name = f"{name}{component['bundle_suffix']}"
    src_bundle = signed_output / bundle_name
    if not src_bundle.exists():
        raise FileNotFoundError(f"Signed bundle not found: {src_bundle}")

    staging = work_dir / f"staging-{component['id']}"
    install_root = staging / component["install_dir"]
    install_root.mkdir(parents=True, exist_ok=True)

    dest_bundle = install_root / bundle_name
    shutil.copytree(src_bundle, dest_bundle)

    component_pkg = work_dir / f"{name}-{component['id']}.pkg"
    cmd = [
        "pkgbuild",
        "--root", str(staging),
        "--identifier", component_identifier(name, component["id"]),
        "--version", version,
        "--install-location", "/",
        str(component_pkg),
    ]
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True)
    return component_pkg


def write_distribution_xml(work_dir: Path, name: str, version: str) -> Path:
    xml_path = work_dir / "distribution.xml"
    lines = [
        '<?xml version="1.0" encoding="utf-8"?>',
        '<installer-gui-script minSpecVersion="2">',
        f'    <title>{name}</title>',
        '    <organization>com.recluseaudio</organization>',
        '    <options customize="always" require-scripts="false" hostArchitectures="arm64,x86_64"/>',
        '    <choices-outline>',
    ]
    for c in COMPONENTS:
        lines.append(f'        <line choice="{c["id"]}"/>')
    lines.append('    </choices-outline>')
    for c in COMPONENTS:
        cid = component_identifier(name, c["id"])
        lines.append(
            f'    <choice id="{c["id"]}" title="{c["title"]}" description="{c["description"]}">'
        )
        lines.append(f'        <pkg-ref id="{cid}"/>')
        lines.append('    </choice>')
    for c in COMPONENTS:
        cid = component_identifier(name, c["id"])
        lines.append(
            f'    <pkg-ref id="{cid}" version="{version}" auth="root">{name}-{c["id"]}.pkg</pkg-ref>'
        )
    lines.append('</installer-gui-script>')
    xml_path.write_text("\n".join(lines) + "\n")
    return xml_path


def main() -> int:
    name = INFO["product_name"]
    version = read_version()

    signed_output = ROOT / "SIGNED" / "MAC" / "OUTPUT"

    missing = [
        f"{name}{c['bundle_suffix']}"
        for c in COMPONENTS
        if not (signed_output / f"{name}{c['bundle_suffix']}").exists()
    ]
    if missing:
        print(f"ERROR: signed bundles missing in {signed_output}: {missing}", file=sys.stderr)
        print("Run: python HELPER_SCRIPTS/sign_builds.py", file=sys.stderr)
        return 1

    build_dir = ROOT / "INSTALLERS" / "MAC" / "BUILD"
    build_dir.mkdir(parents=True, exist_ok=True)

    out_pkg = build_dir / f"{name}_v{version}_macOS_Installer.pkg"
    if out_pkg.exists():
        out_pkg.unlink()

    with tempfile.TemporaryDirectory(prefix="pulsar_pkg_") as tmp:
        work_dir = Path(tmp)

        for c in COMPONENTS:
            build_component_pkg(c, name, version, signed_output, work_dir)

        dist_xml = write_distribution_xml(work_dir, name, version)

        cmd = [
            "productbuild",
            "--distribution", str(dist_xml),
            "--package-path", str(work_dir),
            str(out_pkg),
        ]
        print("+", " ".join(cmd))
        subprocess.run(cmd, check=True)

    print(f"\nOK: built unsigned distribution pkg: {out_pkg}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
