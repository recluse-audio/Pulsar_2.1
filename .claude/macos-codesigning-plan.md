# macOS Code Signing & Notarization Plan (Local Only)

Working plan for implementing the macOS half of Pulsar's release pipeline. PC pipeline is already complete and serves as the reference. **Scope: local builds only.** GitHub Actions / CI is explicitly out of scope for this pass.

The PC equivalents to mirror:
- `SIGNED/release_workflow_pc.py` — 9-step orchestrator
- `SIGNED/PC/sign_pc_{vst3,standalone,installer}.py` — per-artifact signing scripts
- `SIGNED/PC/check_if_signed_pc.py` — verification tool
- `INSTALLERS/PC/build_pc_installer.py` — installer build (Inno Setup)

The macOS stubs that need to be filled in:
- `SIGNED/release_workflow_mac.py` — currently prints "not yet implemented", returns 1
- `SIGNED/MAC/sign_mac_vst3.py` — empty file (0 bytes)
- `SIGNED/MAC/sign_mac_AU.py` — empty file
- `SIGNED/MAC/sign_mac_standalone.py` — empty file
- `SIGNED/MAC/sign_mac_installer.py` — empty file
- `SIGNED/MAC/check_if_signed_mac.py` — interface present, `verify()` returns `False` (TODO)
- `INSTALLERS/MAC/build_mac_installer.py` — empty file

Top-level wiring already handles dispatch:
- `HELPER_SCRIPTS/build_and_release_workflow.py` → `rebuild_all.py` → `release_workflow.py` → `release_workflow_mac.py` (on darwin)
- `HELPER_SCRIPTS/sign_builds.py` already dispatches to the four `sign_mac_*.py` build scripts on darwin
- `HELPER_SCRIPTS/sign_installers.py` already dispatches to `sign_mac_installer.py` on darwin

So the goal of this plan is: fill in the seven stubs above so that running `python HELPER_SCRIPTS/build_and_release_workflow.py` on macOS produces a signed + notarized + stapled `.pkg` plus signed bundles in `RELEASE/MAC/{version}/`, the same way the PC pipeline produces `RELEASE/PC/{version}/`.

---

## Phase 1: Apple Developer Account — DONE

- [x] Enrolled in Apple Developer Program (Individual, $99/yr)
- [x] Account approved

## Phase 2: Certificates — DONE

- [x] CSR generated via Keychain Access
- [x] **Developer ID Application** cert created, downloaded, installed in login keychain
- [x] **Developer ID Installer** cert created, downloaded, installed in login keychain
- [x] Both certs verified paired with private keys (Keychain Access → My Certificates, disclosure triangles show keys)

## Phase 3: Backups — DONE

- [x] `.p12` exported for Developer ID Application (cert + private key)
- [x] `.p12` exported for Developer ID Installer (cert + private key)
- [x] `openssl pkcs12 -info` confirms each contains certificate + private key
- [x] `.p12` files + export passwords stored in password manager

---

## Phase 4: Identify Team ID — IN PROGRESS

- [ ] `security find-identity -v -p codesigning` — note 10-character Team ID in parentheses after each cert name
- [ ] Save Team ID to password manager alongside `.p12` files
- [ ] Record values in the "Reference Values" block at the bottom of this file

## Phase 5: Notarization Credentials

- [ ] `xcrun notarytool --version` — confirm command-line tools installed (run `xcode-select --install` if missing)
- [ ] Generate app-specific password at appleid.apple.com → Sign-In and Security → App-Specific Passwords (label: `notarytool`)
- [ ] Store credentials in keychain (one line, no backslashes):
  ```
  xcrun notarytool store-credentials "notary-profile" \
    --apple-id "you@example.com" \
    --team-id "YOURTEAMID" \
    --password "xxxx-xxxx-xxxx-xxxx"
  ```
- [ ] Verify: `xcrun notarytool history --keychain-profile "notary-profile"` returns without auth error

## Phase 6: First-Test Signing (Manual, Before Scripting)

Goal: prove the cert + entitlements work on one already-built artifact before automating. Pick a freshly-built `BUILD/Pulsar_artefacts/Release/VST3/Pulsar.vst3`.

- [ ] Sign any embedded dylibs/frameworks first (inside-out signing). For JUCE plugins this is normally not needed — the bundle has no embedded helpers — but verify with `find Pulsar.vst3 -type f` before assuming.
- [ ] Sign the bundle:
  ```
  codesign --force --timestamp --options runtime \
    --sign "Developer ID Application: Your Name (TEAMID)" \
    Pulsar.vst3
  ```
- [ ] Verify:
  ```
  codesign --verify --deep --strict --verbose=2 Pulsar.vst3
  spctl -a -vvv -t install Pulsar.vst3
  ```
- [ ] If it complains about JIT or unsigned dylib loads, author a `Pulsar.entitlements` plist and resign with `--entitlements`. Pulsar is a synth with no scripting/JIT, so default entitlements should suffice — only add entitlements if signing actually fails.

## Phase 7: Implement `SIGNED/MAC/sign_mac_*.py` Build Scripts

Mirror the structure of `SIGNED/PC/sign_pc_vst3.py` (read it first):
1. Use `plugin_info.get_plugin_info(ROOT)` to get `target` and `product_name`.
2. Resolve source path under `BUILD/{target}_artefacts/Release/{VST3|Standalone|AU}/`.
3. `shutil.copytree` (bundle) or `shutil.copy2` (file) into `SIGNED/MAC/OUTPUT/`.
4. Run `codesign --force --timestamp --options runtime --sign "Developer ID Application: ... (TEAMID)" <copied-artifact>`.
5. Return `subprocess.run().returncode`.

Per-script targets:
- [ ] `sign_mac_vst3.py` — bundle: `Pulsar.vst3` (full bundle, sign at the bundle level — `codesign` handles inside-out for properly-formed bundles)
- [ ] `sign_mac_standalone.py` — bundle: `Pulsar.app`
- [ ] `sign_mac_AU.py` — bundle: `Pulsar.component`
- [ ] Decision: where to source the signing identity name — env var (e.g. `PULSAR_MAC_DEVID_APP`), config file, or hardcoded `~/.pulsar_signing.json`? PC pipeline reads from `~/.azure/metadata.json`. Recommend a similar `~/.pulsar/macos_signing.json` with `developer_id_application` and `developer_id_installer` cert names + `notary_profile` name. Decide and apply consistently across all four scripts.

## Phase 8: Implement `SIGNED/MAC/check_if_signed_mac.py`

Replace the stub `verify()` with real checks. Keep the existing CLI surface (accepts file paths as args, falls back to scanning `OUTPUT/`). For each path:
- [ ] `codesign --verify --deep --strict <path>` — fail if non-zero
- [ ] `codesign -dv --verbose=4 <path>` — parse output to confirm "Authority=Developer ID Application: ..." line exists
- [ ] For `.pkg` files: use `pkgutil --check-signature <path>` instead of `codesign`
- [ ] Return `True` only if every check passes
- [ ] Used by `release_workflow_mac.py` step 3 (assert unsigned before signing) and step 5 (assert signed after)

## Phase 9: Implement `INSTALLERS/MAC/build_mac_installer.py`

Build a signed, notarizable `.pkg` from the signed bundles in `SIGNED/MAC/OUTPUT/`.

- [ ] Stage a build root with the install layout:
  - `build_root/Library/Audio/Plug-Ins/VST3/Pulsar.vst3`
  - `build_root/Library/Audio/Plug-Ins/Components/Pulsar.component`
  - `build_root/Applications/Pulsar.app`
- [ ] `pkgbuild --root build_root --identifier com.recluseaudio.pulsar --version $(cat VERSION.txt) --install-location / Pulsar-component.pkg`
- [ ] `productbuild --distribution distribution.xml --package-path . --sign "Developer ID Installer: ... (TEAMID)" Pulsar.pkg`
- [ ] Emit a `distribution.xml` template alongside the script (or generate it in-script from `plugin_info`); include welcome/license/conclusion text if desired
- [ ] Output to `INSTALLERS/MAC/BUILD/Pulsar_v{version}_macOS_Installer.pkg` (mirror PC's `INSTALLERS/PC/BUILD/`)
- [ ] Decision: are we shipping all three formats in one installer, or split? PC ships VST3 + Standalone in one `.exe`. Recommend: match — one `.pkg` containing VST3 + AU + Standalone, with optional component selection in `distribution.xml`.

## Phase 10: Implement `SIGNED/MAC/sign_mac_installer.py`

This is separate from `sign_mac_*` build scripts because the installer uses the **Developer ID Installer** cert (not Application).

- [ ] If `productbuild --sign` already produced a signed pkg in Phase 9, this script may just copy the pkg to `SIGNED/MAC/OUTPUT/` and verify. Decide: do we sign at productbuild time, or build unsigned then sign here with `productsign`?
  - PC pattern: `build_pc_installer.py` builds unsigned, then `sign_pc_installer.py` signs. **Match this.**
- [ ] So: `INSTALLERS/MAC/build_mac_installer.py` should use `pkgbuild` + `productbuild` *without* `--sign`, producing an unsigned pkg.
- [ ] `sign_mac_installer.py` then runs:
  ```
  productsign --sign "Developer ID Installer: ... (TEAMID)" \
    --timestamp \
    INSTALLERS/MAC/BUILD/Pulsar_v{version}_macOS_Installer.pkg \
    SIGNED/MAC/OUTPUT/Pulsar_v{version}_macOS_Installer.pkg
  ```

## Phase 11: Notarize + Staple (New Step in Mac Pipeline)

PC has no equivalent — Windows EV signing is trusted directly. macOS requires notarization. This adds a step the PC workflow doesn't have.

- [ ] After signing the installer, submit:
  ```
  xcrun notarytool submit SIGNED/MAC/OUTPUT/Pulsar_v{version}_macOS_Installer.pkg \
    --keychain-profile "notary-profile" \
    --wait
  ```
- [ ] On success, staple:
  ```
  xcrun stapler staple SIGNED/MAC/OUTPUT/Pulsar_v{version}_macOS_Installer.pkg
  xcrun stapler validate SIGNED/MAC/OUTPUT/Pulsar_v{version}_macOS_Installer.pkg
  ```
- [ ] On failure: `xcrun notarytool log <id> --keychain-profile "notary-profile"`, fix, resubmit.
- [ ] Decision: where does this live? Two options:
  - (A) Inline as a new step in `release_workflow_mac.py` (between step 8 "verify installer signed" and step 9 "copy to RELEASE/")
  - (B) Standalone script `SIGNED/MAC/notarize_mac.py` called from the workflow
  - **Recommend (B)** — keeps the orchestrator readable, makes it independently testable.
- [ ] Notarize + staple the installer pkg. Bundles inside the pkg inherit the staple via the installer; if we ever distribute raw `.vst3`/`.component` outside of an installer, those will need their own staples (zip → notarize → unzip → staple — or distribute as a notarized dmg).

## Phase 12: Implement `SIGNED/release_workflow_mac.py`

Read `release_workflow_pc.py` end-to-end first, then mirror its 9 steps with macOS substitutions:

| PC Step | Mac Equivalent |
|---|---|
| 1. Confirm release builds exist | Check `BUILD/Pulsar_artefacts/Release/{VST3,Standalone,AU}/` paths |
| 2. Confirm installer project exists | Check `INSTALLERS/MAC/build_mac_installer.py` exists |
| 3. Confirm builds NOT already signed | `check_if_signed_mac.py <vst3>` should return non-zero |
| 4. Sign release builds | `sign_builds.py` (already dispatches to mac scripts) |
| 5. Verify builds signed | `check_if_signed_mac.py` on each output bundle |
| 6. Update version, build installer | Read `VERSION.txt`, run `build_mac_installer.py`. **No `.iss` to patch — version flows in via `pkgbuild --version`.** |
| 7. Sign installer | `sign_installers.py` (already dispatches to `sign_mac_installer.py`) |
| 8. Verify installer signed | `check_if_signed_mac.py` on the signed pkg |
| **8.5. Notarize + staple** | New step (Phase 11). PC has no equivalent. |
| 9. Copy to `RELEASE/MAC/{version}/` | `shutil.copytree` for `.vst3`/`.component`/`.app`; `shutil.copy2` for `.pkg`. Refuse to overwrite a non-empty existing version dir. |

- [ ] Implement, with the same "fail fast on non-zero return" pattern PC uses
- [ ] Run end-to-end against a real version bump

## Phase 13: End-to-End Verification on a Clean Mac

Don't trust the dev machine — its keychain trusts everything Apple signed.

- [ ] Test installer on a Mac that has never seen the cert (or fresh user account)
- [ ] `spctl -a -vvv -t install Pulsar_v{version}_macOS_Installer.pkg` — expect `source=Notarized Developer ID`
- [ ] Install, then load Pulsar in Logic / Ableton / a clean DAW — confirm no Gatekeeper prompts
- [ ] Confirm both VST3 and AU show up and instantiate

---

## Implementation order (do them in this order)

1. Phase 4 — Team ID (5 min)
2. Phase 5 — Notarization keychain profile (10 min)
3. Phase 6 — Manual smoke test signing one VST3 (30 min, surfaces entitlement issues early)
4. Phase 7 — `sign_mac_vst3.py` first, run it standalone, verify with phase-8 verifier in manual mode, then duplicate the pattern to `_AU` and `_standalone`
5. Phase 8 — `check_if_signed_mac.py` (do this in parallel with phase 7 — they validate each other)
6. Phase 9 + Phase 10 — installer build then installer sign (test the pkg installs locally before notarizing)
7. Phase 11 — `notarize_mac.py` (test on the manually-built installer first to confirm credentials work, before wiring into the orchestrator)
8. Phase 12 — `release_workflow_mac.py` — only after every individual piece works standalone
9. Phase 13 — clean-Mac verification

---

## Reference Values

Fill in as you go. Keep this file out of git if you put real values here — `.claude/` is gitignored by default in many setups; verify before committing.

```
Team ID:                 _______________
Notary keychain profile: notary-profile
Apple ID email:          _______________
Bundle ID:               com.recluseaudio.pulsar
Developer ID App cert:   "Developer ID Application: ____ (TEAMID)"
Developer ID Inst cert:  "Developer ID Installer: ____ (TEAMID)"
.p12 backup location:    _______________
```

---

## Key Gotchas

- **Sign from a local Terminal session, not SSH.** The keychain may not unlock headlessly — `codesign` will hang or fail with "User interaction is not allowed."
- **`--timestamp` and `--options runtime` are required for notarization.** Forgetting either will cause `notarytool` to reject the submission with cryptic errors.
- **Sign inside-out:** any embedded helpers/frameworks first, then the bundle. JUCE-built `.vst3`/`.component` typically have no embedded helpers, but `find Pulsar.vst3 -type f -perm +111` to confirm.
- **Test on a clean Mac.** Your dev machine has the cert in its keychain; everyone else's doesn't. Stapling is what lets the binary be trusted offline — verify it actually works.
- **Certs valid 5 years; timestamped signatures stay valid even after the cert expires.** Always pass `--timestamp`.
- **Don't double-sign.** PC pipeline's step-3 unsigned-check exists for a reason; mirror it on macOS. Re-signing an already-signed bundle can succeed silently but produces ambiguous results.
- **Notarize the installer, not just the bundles.** Stapling the pkg is what propagates trust. Loose `.vst3`/`.component` distribution would require per-bundle notarization + stapling, which is more work — prefer pkg distribution.

---

## Quick Command Reference

### Find Team ID
```bash
security find-identity -v -p codesigning
```

### Sign a plugin bundle
```bash
codesign --force --timestamp --options runtime \
  --sign "Developer ID Application: Your Name (TEAMID)" \
  Pulsar.vst3
```

### Verify signature
```bash
codesign --verify --deep --strict --verbose=2 Pulsar.vst3
spctl -a -vvv -t install Pulsar.vst3
```

### Build + sign pkg (split workflow, matches PC pattern)
```bash
# Build unsigned
pkgbuild --root build_root \
  --identifier com.recluseaudio.pulsar \
  --version $(cat VERSION.txt) \
  --install-location / \
  Pulsar-unsigned.pkg

productbuild --distribution distribution.xml \
  --package-path . \
  Pulsar-unsigned-dist.pkg

# Sign separately (mirrors PC sign_pc_installer.py timing)
productsign --sign "Developer ID Installer: Your Name (TEAMID)" \
  --timestamp \
  Pulsar-unsigned-dist.pkg \
  Pulsar.pkg
```

### Notarize + staple
```bash
xcrun notarytool submit Pulsar.pkg --keychain-profile "notary-profile" --wait
xcrun stapler staple Pulsar.pkg
xcrun stapler validate Pulsar.pkg
```

### Pull notarization log on failure
```bash
xcrun notarytool log <submission-id> --keychain-profile "notary-profile"
```

### Verify installer signature
```bash
pkgutil --check-signature Pulsar.pkg
spctl -a -vvv -t install Pulsar.pkg
```
