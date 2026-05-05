#!/usr/bin/env python3
"""Read Apple Developer ID signing identities directly from the macOS keychain.

The keychain is the single source of truth — no separate config file to keep
in sync. The certs were placed in the keychain during Developer ID setup
(Phase 2 of the macos-codesigning-plan) and the notary credentials during
Phase 5.

Each helper accepts an environment variable override for CI / multi-identity
workflows:
  PULSAR_DEVID_APP        -> overrides Developer ID Application lookup
  PULSAR_DEVID_INSTALLER  -> overrides Developer ID Installer lookup
  PULSAR_NOTARY_PROFILE   -> overrides notary keychain profile name
"""

from __future__ import annotations

import os
import re
import subprocess
import sys

DEFAULT_NOTARY_PROFILE = "notary-profile"


def _find_identity(prefix: str) -> str | None:
    """Return the first identity in the keychain whose name starts with `prefix`.

    `prefix` should be e.g. 'Developer ID Application:' or 'Developer ID Installer:'.
    Note: `-p codesigning` filters out Installer certs, so we don't pass it.
    """
    result = subprocess.run(
        ["security", "find-identity", "-v"],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        return None

    pattern = re.compile(r'"(' + re.escape(prefix) + r'[^"]+)"')
    for line in result.stdout.splitlines():
        m = pattern.search(line)
        if m:
            return m.group(1)
    return None


def _resolve(env_var: str, prefix: str, label: str) -> str:
    if env := os.environ.get(env_var):
        return env
    identity = _find_identity(prefix)
    if not identity:
        print(f"ERROR: No '{label}' identity found in keychain.", file=sys.stderr)
        print("Run: security find-identity -v", file=sys.stderr)
        print(f"Or set {env_var} to override.", file=sys.stderr)
        sys.exit(1)
    return identity


def developer_id_application() -> str:
    return _resolve("PULSAR_DEVID_APP", "Developer ID Application:", "Developer ID Application")


def developer_id_installer() -> str:
    return _resolve("PULSAR_DEVID_INSTALLER", "Developer ID Installer:", "Developer ID Installer")


def notary_profile() -> str:
    return os.environ.get("PULSAR_NOTARY_PROFILE", DEFAULT_NOTARY_PROFILE)


if __name__ == "__main__":
    print(f"Developer ID Application: {developer_id_application()}")
    print(f"Developer ID Installer:   {developer_id_installer()}")
    print(f"Notary profile:           {notary_profile()}")
