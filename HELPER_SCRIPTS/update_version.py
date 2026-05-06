#!/usr/bin/env python3
"""Generate SOURCE/Util/Version.h from VERSION.txt.

Pure read-then-write: VERSION.txt is the source of truth and is never modified
by this script. Bumping the version is a separate, deliberate action — edit
VERSION.txt by hand or via a dedicated bump script.

This is invoked by the `update_version_header` custom target in CMakeLists.txt
as a build dependency of the Pulsar target.
"""
import sys
import pathlib

version_file = pathlib.Path(sys.argv[1])
header_file = pathlib.Path(sys.argv[2])

if not version_file.exists():
    sys.exit(f"ERROR: VERSION.txt not found: {version_file}")

raw = version_file.read_text().strip()
parts = raw.split(".")
if len(parts) != 3 or not all(p.isdigit() for p in parts):
    sys.exit(f"ERROR: VERSION.txt must be MAJOR.MINOR.PATCH (got {raw!r})")

major, minor, patch = map(int, parts)
version_string = f"{major}.{minor}.{patch}"

header_file.parent.mkdir(parents=True, exist_ok=True)
header_file.write_text(f"""#pragma once
#define BUILD_VERSION_MAJOR {major}
#define BUILD_VERSION_MINOR {minor}
#define BUILD_VERSION_PATCH {patch}
#define BUILD_VERSION_STRING "{version_string}"
""")
