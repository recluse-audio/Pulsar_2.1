#!/usr/bin/env python3
"""Parse plugin name info from CMakeLists.txt for use by signing/release scripts."""

from __future__ import annotations

import re
from pathlib import Path


def get_plugin_info(root: Path) -> dict[str, str]:
    """
    Parse CMakeLists.txt to extract plugin naming info.

    Returns dict with:
        target:       CMake target name (e.g. "Flanger2") — used for artefacts directory
        product_name: PRODUCT_NAME (e.g. "Flanger 2") — used for binary filenames
    """
    cmake_file = root / "CMakeLists.txt"
    text = cmake_file.read_text()

    # Extract CMake target from: project(<Name> VERSION ...)
    m = re.search(r'project\(\s*(\S+)', text)
    target = m.group(1) if m else root.name

    # Extract PRODUCT_NAME from: PRODUCT_NAME "..."
    m = re.search(r'PRODUCT_NAME\s+"([^"]+)"', text)
    product_name = m.group(1) if m else target

    return {"target": target, "product_name": product_name}
