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

## Phase 4: Identify Team ID — DONE

- [x] `security find-identity -v` — confirmed both certs present
- [x] Team ID: `KP3927B3MT`
- [x] Developer ID Application: `"Developer ID Application: Ryan Devens (KP3927B3MT)"` (SHA1: `B36A96E26580C3E42C1D900E16DE72B537C49A60`)
- [x] Developer ID Installer: `"Developer ID Installer: Ryan Devens (KP3927B3MT)"` (SHA1: `073E5742DC39256778D100A054A6A9CD1405877F`)
- [x] Recorded in Reference Values block below
- [ ] Save Team ID to password manager alongside `.p12` files

## Phase 5: Notarization Credentials — DONE

- [x] `xcrun notarytool --version` → 1.1.1 (40), Command Line Tools at `/Library/Developer/CommandLineTools`
- [x] App-specific password generated at appleid.apple.com (labeled `notarytool`)
- [x] Credentials stored in keychain under profile name `notary-profile`
- [x] Verified: `xcrun notarytool history --keychain-profile "notary-profile"` returns "no submissions history" with no auth error

## Phase 6: First-Test Signing (Manual, Before Scripting) — DONE

All three formats smoke-tested by signing a copy in `/tmp/`:

- [x] **VST3** (`Pulsar.vst3`) — signed cleanly. Before: `Signature=adhoc, TeamIdentifier=not set`. After: `flags=0x10000(runtime), TeamIdentifier=KP3927B3MT, Timestamp=May 4 2026 5:56:18 PM`. `codesign --verify --deep --strict` exit 0. `spctl -t install` rejected with `source=Unnotarized Developer ID` (expected — notarization in Phase 11).
- [x] **AU** (`Pulsar.component`) — signed cleanly. Before: `flags=0x20002(adhoc,linker-signed), Identifier=Pulsar (bare), Info.plist=not bound, Sealed Resources=none`. After: `flags=0x10000(runtime), Identifier=com.recluseaudio.pulsar, Info.plist entries=12, Sealed Resources version=2`. Signing properly bound the Info.plist that JUCE's linker-signed adhoc had left unbound. `verify` exit 0; `spctl -t install` rejected for unnotarized.
- [x] **Standalone** (`Pulsar.app`) — signed cleanly. Same shape as AU. `verify` exit 0; `spctl -t exec` rejected for unnotarized.

**Confirmed:**
- Cert + private key chain works end-to-end (Authority chain: Developer ID Application → Developer ID CA → Apple Root CA)
- `--timestamp` reaches Apple's timestamp server
- `--options runtime` (hardened runtime) accepts cleanly
- **No entitlements needed** — JUCE's default plugin/app layout signs without a custom `.entitlements` plist
- All current Release builds are arm64-only (`Format=bundle with Mach-O thin (arm64)`). Universal-binary support is a separate concern; not blocking for local-mac signing pipeline.
- All bundles have no embedded helpers/frameworks (`find Pulsar.vst3 -type f` returned only Info.plist, PkgInfo, the Mach-O, _CodeSignature/CodeResources, and Resources/moduleinfo.json) — single-pass `codesign` on the bundle is sufficient.

**Quirk to remember:** JUCE's pre-signing AU has `Sealed Resources=none, Info.plist=not bound`. This is the linker-signed adhoc state and is normal — `codesign --force --sign ...` replaces it cleanly.

## Phase 7: Implement `SIGNED/MAC/sign_mac_*.py` Build Scripts — DONE

Implemented and verified end-to-end against current Release builds. Each script copies the bundle from `BUILD/Pulsar_artefacts/Release/<format>/` into `SIGNED/MAC/OUTPUT/` then runs `codesign --force --timestamp --options runtime --sign <identity> <copied-bundle>`.

- [x] `SIGNED/MAC/mac_identity.py` — keychain identity lookup via `security find-identity -v`. No JSON config file. Env overrides: `PULSAR_DEVID_APP`, `PULSAR_DEVID_INSTALLER`, `PULSAR_NOTARY_PROFILE`. Notary profile defaults to `"notary-profile"`.
- [x] `sign_mac_vst3.py` — output: `SIGNED/MAC/OUTPUT/Pulsar.vst3`. Verified: `codesign --verify --deep --strict` exit 0; `spctl -t install` rejects unnotarized (expected).
- [x] `sign_mac_AU.py` — output: `SIGNED/MAC/OUTPUT/Pulsar.component`. Same verification result.
- [x] `sign_mac_standalone.py` — output: `SIGNED/MAC/OUTPUT/Pulsar.app`. Same verification result.

Identity resolution decision (resolved): **read directly from the keychain.** Earlier proposals (`~/.pulsar/macos_signing.json`, `~/.apple-developer/pulsar_signing.json`) were rejected — keychain is the OS-canonical source of truth, and we already populated it in Phases 4–5. Avoids a parallel JSON config that needs to be kept in sync. Env-var overrides are the escape hatch for CI / multi-identity use.

The umbrella dispatcher `HELPER_SCRIPTS/sign_builds.py` already routes to these three scripts on darwin — no changes needed there.

## Phase 8: Implement `SIGNED/MAC/check_if_signed_mac.py` — DONE

- [x] Implemented `verify(file)` with two paths:
  - Bundles (`.vst3`, `.component`, `.app`): runs `codesign --verify --deep --strict --verbose=2`, then `codesign -dv --verbose=2`, asserts no `Signature=adhoc` and at least one `Authority=Developer ID Application:` or `Authority=Developer ID Installer:` line.
  - Packages (`.pkg`): runs `pkgutil --check-signature` and asserts `"Developer ID Installer:"` appears in the output.
- [x] Tested against signed OUTPUT/ bundles → all 3 pass, exit 0.
- [x] Tested against raw `BUILD/Pulsar.vst3` (adhoc-signed by JUCE) → fails with exit 1. The actual error is `invalid Info.plist (plist or signature have been modified)` because something modifies Info.plist after JUCE's link-time adhoc signing — this is expected; `codesign --force --sign` overwrites the broken adhoc with a fresh Developer ID signature in Phase 7.
- [ ] `.pkg` branch not yet exercised — will be tested when Phase 9 produces a real installer.

## Phase 9: Implement `INSTALLERS/MAC/build_mac_installer.py` — DONE

Builds an **unsigned** distribution pkg from signed bundles in `SIGNED/MAC/OUTPUT/`. Outer pkg signing is Phase 10.

- [x] Single distribution pkg containing 3 component pkgs: VST3 (→ `/Library/Audio/Plug-Ins/VST3/`), AU (→ `/Library/Audio/Plug-Ins/Components/`), Standalone (→ `/Applications/`).
- [x] `pkgbuild` builds component pkgs into a temp dir, then `productbuild` composes them via a generated `distribution.xml` (with `customize="always"` so users can opt out of formats).
- [x] Output: `INSTALLERS/MAC/BUILD/Pulsar_v{version}_macOS_Installer.pkg` (4.7MB for v1.1.83 build). Mirrors PC's `INSTALLERS/PC/BUILD/` location.
- [x] Verified: `pkgutil --check-signature` on output reports `Status: no signature` (expected — unsigned). `check_if_signed_mac.py` correctly fails it with exit 1.
- [x] Verified: `pkgutil --expand` shows the embedded distribution.xml is well-formed with correct identifiers (`com.recluseaudio.pulsar.{vst3,au,standalone}`), `auth="root"`, and the three component pkgs nested inside.

## Phase 10: Implement `SIGNED/MAC/sign_mac_installer.py` — DONE

- [x] Reads the latest unsigned pkg from `INSTALLERS/MAC/BUILD/{name}_v*_macOS_Installer.pkg`, signs with `productsign --sign <Developer ID Installer> --timestamp`, writes to `SIGNED/MAC/OUTPUT/`.
- [x] Identity comes from `mac_identity.developer_id_installer()` (keychain lookup; `PULSAR_DEVID_INSTALLER` env override).
- [x] Verified end-to-end: signed pkg passes `check_if_signed_mac.py` with exit 0. Cert chain: Developer ID Installer → Developer ID CA → Apple Root CA. Trusted timestamp applied (2026-05-04 23:59:51 UTC).
- [x] The umbrella dispatcher `HELPER_SCRIPTS/sign_installers.py` already routes to this script on darwin — no changes needed.

## Phase 11: Notarize + Staple — DONE

- [x] Implemented as standalone script `SIGNED/MAC/notarize_mac.py` (option B from the plan — separately testable, keeps orchestrator readable).
- [x] Submits with `xcrun notarytool submit --keychain-profile <profile> --wait`, parses submission id, blocks until status = Accepted.
- [x] On success runs `xcrun stapler staple` then `xcrun stapler validate`.
- [x] On failure prints the submission ID and the `xcrun notarytool log` command to fetch the detailed log.
- [x] End-to-end verified on `Pulsar_v1.1.83_macOS_Installer.pkg`:
  - Submission `0d85b9be-56f4-40a0-9bcd-fe55adb937b5` → `status: Accepted` after ~30s.
  - `xcrun stapler staple` succeeded; `xcrun stapler validate` succeeded.
  - `spctl -a -vvv -t install` flipped from `rejected, source=Unnotarized Developer ID` to **`accepted, source=Notarized Developer ID`**.
  - `pkgutil --check-signature` now shows `Notarization: trusted by the Apple notary service`.
- [x] Confirms staple-on-installer is sufficient — the bundles inside the pkg inherit trust via the installer. If raw `.vst3`/`.component` distribution outside an installer is ever needed, those would require per-bundle notarization (zip → submit → unzip → staple).

## Phase 12: Implement `SIGNED/release_workflow_mac.py` — DONE

- [x] Implemented all 9 steps + the inserted step 8.5 (notarize) per the table below.
- [x] End-to-end run on v1.1.83 succeeded: all 9 steps + 8.5 returned 0; submission `39e0a5cc-130a-4670-a8bf-3d17581cf8cd` accepted by Apple notary in ~35s; final assets copied to `RELEASE/MAC/1.1.83/` (3 bundles + signed+notarized+stapled pkg, ~17MB total). Final pkg passes `spctl -a -t install` as `source=Notarized Developer ID`.

| Step | Action |
|---|---|
| 1 | Verify VST3/AU/Standalone exist in `BUILD/Pulsar_artefacts/Release/` |
| 2 | Verify `INSTALLERS/MAC/build_mac_installer.py` exists |
| 3 | Assert `check_if_signed_mac.py` returns non-zero on raw BUILD VST3 (must be unsigned) |
| 4 | Run `HELPER_SCRIPTS/sign_builds.py` (dispatches to `sign_mac_{vst3,AU,standalone}.py`) |
| 5 | Run `check_if_signed_mac.py` against each `SIGNED/MAC/OUTPUT/` bundle, all must return 0 |
| 6 | Run `INSTALLERS/MAC/build_mac_installer.py` (version flows in via `pkgbuild --version` — no XML patching needed unlike PC's `.iss`) |
| 7 | Run `HELPER_SCRIPTS/sign_installers.py` (dispatches to `sign_mac_installer.py`) |
| 8 | Run `check_if_signed_mac.py` against the signed pkg |
| 8.5 | Run `SIGNED/MAC/notarize_mac.py` (mac-only step; PC has no equivalent) |
| 9 | Copy bundles + pkg to `RELEASE/MAC/{version}/`. Refuse to overwrite a non-empty existing version dir. |

### Post-implementation polish

- [x] **stdout interleaving fix.** Every script in the pipeline now sets `sys.stdout.reconfigure(line_buffering=True)` after imports, so prints flush per-newline instead of waiting for process exit. Previously, child process output (e.g. `codesign: replacing existing signature`) appeared in the log before the parent's setup prints, because the parent's buffered stdout flushed only at exit, by which time the child had already written directly to the inherited fd. Validated by running `sign_mac_vst3.py` standalone and confirming "Copying bundle…" → "Signing…" → "+ codesign…" → "replacing existing signature" appear in source order.
- [x] **VERSION.txt drift detection.** Step 1 now reads `CFBundleShortVersionString` from each bundle's `Contents/Info.plist` via `plutil -extract` and aborts with exit 1 if it doesn't match `VERSION.txt`. Catches the case where someone bumps VERSION.txt without rebuilding (we saw this in the v1.1.83 run — bundles were 1.1.82 inside, pkg labeled 1.1.83 outside). The full `build_and_release_workflow.py` still calls `rebuild_all.py` first so this only fires when running `release_workflow.py` directly without a fresh build.

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
Team ID:                 KP3927B3MT
Notary keychain profile: notary-profile  (not yet stored — Phase 5)
Apple ID email:          _______________ (fill in once stored)
Bundle ID:               com.recluseaudio.pulsar
Developer ID App cert:   "Developer ID Application: Ryan Devens (KP3927B3MT)"
                         SHA1: B36A96E26580C3E42C1D900E16DE72B537C49A60
Developer ID Inst cert:  "Developer ID Installer: Ryan Devens (KP3927B3MT)"
                         SHA1: 073E5742DC39256778D100A054A6A9CD1405877F
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
