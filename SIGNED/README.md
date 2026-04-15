# Code Signing

## Prerequisites

- Azure Trusted Signing credentials configured (see `docs/windows-code-signing-setup.md`)
- `az login` completed before running any signing scripts
- Release builds exist in `BUILD/Pulsar_artefacts/Release/`

## Release Workflow (Recommended)

Run the full automated pipeline from the repo root:

```
python HELPER_SCRIPTS/release_workflow.py
```

This dispatches to `SIGNED/release_workflow_pc.py` (or `release_workflow_mac.py`) and performs:

1. Confirms release builds (VST3, Standalone) exist in `BUILD/Pulsar_artefacts/Release/`
2. Confirms the installer project (`INSTALLERS/PC/Pulsar.iss`) exists
3. Verifies builds are **not** already signed
4. Signs all builds via `HELPER_SCRIPTS/sign_builds.py`
5. Verifies builds **are** now signed
6. Updates `Pulsar.iss` version from `VERSION.txt`, builds installer via `INSTALLERS/PC/build_pc_installer.py`
7. Signs the installer via `HELPER_SCRIPTS/sign_installers.py`
8. Verifies installer is signed
9. Copies all signed assets to `RELEASE/PC/{version}/`

The workflow aborts on any failure and will not overwrite an existing release directory.

## Manual Signing (Without the Workflow)

If you need to sign individual components:

```
# Sign plugin builds (VST3 + Standalone)
python HELPER_SCRIPTS/sign_builds.py

# Build installer from signed assets
python HELPER_SCRIPTS/build_installer.py

# Sign the installer
python HELPER_SCRIPTS/sign_installers.py

# Verify signatures
python SIGNED/PC/check_if_signed_pc.py                      # checks all files in SIGNED/PC/OUTPUT/
python SIGNED/PC/check_if_signed_pc.py path/to/binary.exe   # checks a specific file
```

## Testing Signature Verification

```
python SIGNED/test_digital_signature_check.py
```

Runs the check script against a known signed binary (from `SIGNED/PC/OUTPUT/`) and a known unsigned binary (from `BUILD/Pulsar_artefacts/Release/VST3/`). Both binaries must exist for the tests to run.

## Directory Structure

```
SIGNED/
  PC/
    sign_pc_vst3.py          # Signs VST3 bundle with Azure
    sign_pc_standalone.py     # Signs Standalone exe with Azure
    sign_pc_installer.py      # Signs installer exe with Azure
    check_if_signed_pc.py     # Verifies signatures (signtool)
    OUTPUT/                   # Signed binaries land here (gitignored)
  MAC/
    sign_mac_vst3.py          # Stub
    sign_mac_standalone.py    # Stub
    sign_mac_AU.py            # Stub
    sign_mac_installer.py     # Stub
    check_if_signed_mac.py    # Stub
    OUTPUT/                   # (gitignored)
  release_workflow_pc.py      # Full PC release pipeline
  release_workflow_mac.py     # Stub
  test_digital_signature_check.py
```
