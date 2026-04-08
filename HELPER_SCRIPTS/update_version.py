#!/usr/bin/env python3
import sys
import pathlib

version_file = pathlib.Path(sys.argv[1])
header_file = pathlib.Path(sys.argv[2])

# Fallback version
default_version = "0.0.1"

# Read and parse current version
if version_file.exists():
    raw = version_file.read_text().strip()
    parts = raw.split(".")
    if len(parts) == 3 and all(p.isdigit() for p in parts):
        major, minor, patch = map(int, parts)
    else:
        major, minor, patch = 0, 0, 1
else:
    major, minor, patch = 0, 0, 1

# Increment patch version
patch += 1
new_version = f"{major}.{minor}.{patch}"

# Write updated version
version_file.write_text(new_version + "\n")

# Ensure parent directory exists
header_file.parent.mkdir(parents=True, exist_ok=True)

# Write to C++ header
header_file.write_text(f"""#pragma once
#define BUILD_VERSION_MAJOR {major}
#define BUILD_VERSION_MINOR {minor}
#define BUILD_VERSION_PATCH {patch}
#define BUILD_VERSION_STRING "{new_version}"
""")
