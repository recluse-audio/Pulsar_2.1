# macOS Signing - Next Steps

## 1. Implement Signing Scripts

Each script in `SIGNED/MAC/` is currently an empty stub. They need to use `codesign` with a valid Apple Developer ID:

- `sign_mac_vst3.py` - Sign the `.vst3` bundle with `codesign --deep --force --sign "Developer ID Application: ..." --options runtime`
- `sign_mac_standalone.py` - Sign the standalone `.app` bundle
- `sign_mac_AU.py` - Sign the `.component` Audio Unit bundle
- `sign_mac_installer.py` - Sign the `.pkg` installer with `productsign`

All scripts should copy from `BUILD/Pulsar_artefacts/Release/` to `SIGNED/MAC/OUTPUT/` before signing.

## 2. Implement Signature Verification

`check_if_signed_mac.py` has the correct interface but `verify()` returns `False`. Implement using:

```python
codesign --verify --deep --strict path/to/binary
spctl --assess --type execute path/to/binary  # Gatekeeper check
```

## 3. Build the macOS Installer

`INSTALLERS/MAC/build_mac_installer.py` needs to create a `.pkg` using `pkgbuild` and/or `productbuild` that installs:

- VST3 to `/Library/Audio/Plug-Ins/VST3/`
- AU to `/Library/Audio/Plug-Ins/Components/`
- Standalone to `/Applications/`

## 4. Implement release_workflow_mac.py

`SIGNED/release_workflow_mac.py` is a stub. Follow the same 9-step structure as `release_workflow_pc.py`:

- Artifacts go to `RELEASE/MAC/{version}/`
- Must handle both ARM and Intel builds if supporting universal binaries

## 5. Notarization

After signing, macOS builds must be notarized with Apple:

```
xcrun notarytool submit path/to/binary --apple-id ... --team-id ... --password ...
xcrun notarytool wait <submission-id>
xcrun stapler staple path/to/binary
```

This applies to the VST3, AU, Standalone, and installer `.pkg`.
