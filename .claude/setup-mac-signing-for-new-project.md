# Porting the Mac Sign + Notarize Setup to a New Project

This is a step-by-step guide for replicating Pulsar's macOS signing + notarization
pipeline in another JUCE/CMake plugin repo. Pulsar's setup ships a notarized
universal `.pkg` from a single GitHub Actions click; this guide gets you to the
same place.

Assumptions about the target project:
- JUCE plugin built with CMake (`juce_add_plugin(...)`).
- VST3 + AU + Standalone formats (drop AU/Standalone if you don't ship them —
  trivial edits).
- Hosted on GitHub.
- `VERSION.txt` at repo root, format `MAJOR.MINOR.PATCH`.
- You already have an Apple Developer account and have generated **Developer
  ID Application** and **Developer ID Installer** certificates locally
  (Keychain Access shows both with private keys attached).

If you don't have the certs yet, follow Apple's flow first
(developer.apple.com → Certificates → "+") then come back. That part is the
same for any project and isn't covered here.

---

## What you're actually building

The setup has three layers:

1. **Local Python orchestrator** (`SIGNED/release_workflow_mac.py`) — runs
   the whole sign/build-pkg/sign-pkg/notarize/staple sequence. Works on a
   developer laptop *and* on a CI runner without modification, because the
   identity-lookup module accepts env-var overrides.
2. **Per-step Python helpers** (`SIGNED/MAC/sign_mac_*.py`,
   `notarize_mac.py`, `INSTALLERS/MAC/build_mac_installer.py`) — each does
   one thing. Stdlib only.
3. **GitHub Actions workflow** (`.github/workflows/release-macos.yml`) —
   provisions a temp keychain, imports certs, registers a notary profile,
   builds universal Release, calls the orchestrator, uploads the result.

The CI workflow is *only* responsible for runner provisioning and artifact
upload. The "what counts as a signed release" logic lives in Python and is
identical local vs CI. **Resist the temptation to inline signing commands
directly into the YAML** — the local/CI symmetry is the thing that makes this
maintainable.

---

## Files you'll copy from Pulsar (verbatim or near-verbatim)

Paths are relative to repo root. Most of these are project-agnostic.

| File | Purpose | Edit needed? |
|---|---|---|
| `SIGNED/release_workflow_mac.py` | Orchestrator | None if you keep the directory layout |
| `SIGNED/release_workflow_pc.py` | PC equivalent (optional, but parallel) | None |
| `SIGNED/MAC/mac_identity.py` | Looks up identities, supports env overrides | None |
| `SIGNED/MAC/check_if_signed_mac.py` | Verifies Developer ID signature | None |
| `SIGNED/MAC/sign_mac_vst3.py` | Signs the .vst3 bundle | None — uses `plugin_info` |
| `SIGNED/MAC/sign_mac_AU.py` | Signs the .component bundle | None |
| `SIGNED/MAC/sign_mac_standalone.py` | Signs the .app bundle | None |
| `SIGNED/MAC/sign_mac_installer.py` | `productsign` on the .pkg | None |
| `SIGNED/MAC/notarize_mac.py` | `xcrun notarytool submit --wait` + staple | None |
| `SIGNED/plugin_info.py` | Resolves `product_name` / `target` per repo | **Yes — see below** |
| `HELPER_SCRIPTS/sign_builds.py` | Calls the three sign_mac_*.py scripts | None |
| `HELPER_SCRIPTS/sign_installers.py` | Calls sign_mac_installer.py | None |
| `INSTALLERS/MAC/build_mac_installer.py` | `pkgbuild` + `productbuild` | Light — paths/component IDs |
| `.github/workflows/release-macos.yml` | The CI workflow | Light — target name |
| `HELPER_SCRIPTS/update_version.py` | VERSION.txt → Version.h | None — see "version handling" below |

`plugin_info.py` is the one shim you'll always edit per-project: it returns
`{"product_name": "...", "target": "..."}` so the rest of the scripts can
locate `BUILD/<target>_artefacts/Release/...`. Open Pulsar's copy and adapt.

---

## Repo layout the scripts assume

```
<repo>/
├── BUILD/                                ← cmake build dir (gitignored)
│   └── <Target>_artefacts/Release/
│       ├── VST3/<Name>.vst3
│       ├── AU/<Name>.component
│       └── Standalone/<Name>.app
├── INSTALLERS/MAC/
│   ├── BUILD/                            ← unsigned .pkg lands here
│   └── build_mac_installer.py
├── SIGNED/
│   ├── MAC/
│   │   ├── OUTPUT/                       ← signed bundles + signed pkg land here
│   │   ├── mac_identity.py
│   │   ├── check_if_signed_mac.py
│   │   ├── notarize_mac.py
│   │   └── sign_mac_*.py
│   ├── plugin_info.py
│   └── release_workflow_mac.py
├── HELPER_SCRIPTS/
│   ├── sign_builds.py
│   ├── sign_installers.py
│   └── update_version.py
├── RELEASE/MAC/<version>/                ← final notarized artifacts (gitignored)
├── .github/workflows/release-macos.yml
├── VERSION.txt
└── CMakeLists.txt
```

Add to `.gitignore`: `BUILD/`, `INSTALLERS/MAC/BUILD/`, `SIGNED/MAC/OUTPUT/`,
`RELEASE/`.

---

## Version handling — the gotcha

**Critical:** `VERSION.txt` must be the source of truth, and the build must
NOT mutate it. Pulsar originally had `update_version.py` increment patch on
every build, which created a 1-version skew between the bundle's
`CFBundleShortVersionString` (locked at configure time from
`project(<Name> VERSION ...)`) and `VERSION.txt` (mutated mid-build). The
release script correctly flags this as a stale build and refuses to sign.

Use Pulsar's current (post-fix) `update_version.py` verbatim — it's a pure
read/write. To bump the version, edit `VERSION.txt` by hand and commit. The
deliberate-bump model is a feature: the version you ship is a decision, not a
side effect.

If you copy Pulsar's `release_workflow_mac.py`, it already includes step 1's
"verify bundle plist matches VERSION.txt" check. Keep it. It catches stale
builds (e.g. `VERSION.txt` bumped after the last `cmake --build`).

---

## Phased setup (mirrors how we built it for Pulsar)

Each phase has a clear "done when" gate. Don't skip ahead — the gates catch
configuration mistakes early and cheaply.

### Phase 0 — Get the local flow working first

Before touching CI, prove the orchestrator works on your laptop:

1. Copy the files listed in the table above. Edit `plugin_info.py` to match
   your `juce_add_plugin` PRODUCT_NAME and target name.
2. Adapt `INSTALLERS/MAC/build_mac_installer.py` — most edits are bundle IDs,
   component package paths, and the install location. Read Pulsar's copy
   carefully; comments explain the moving parts.
3. `python HELPER_SCRIPTS/rebuild_all.py` (or your equivalent) → builds.
4. `python SIGNED/release_workflow_mac.py` → should walk through all 9 steps,
   produce `RELEASE/MAC/<version>/Pulsar_v<x>_macOS_Installer.pkg`, and
   notarize against your dev account.

**Done when:** local notarized pkg installs cleanly on a Mac that has never
seen your cert (e.g. a fresh VM), with no Gatekeeper warnings.

If you skip Phase 0, you'll spend Phase 1–4 debugging shell quoting in YAML
when the actual problem is a Python script bug. Don't.

### Phase 1 — CI skeleton: build only

Copy Pulsar's `release-macos.yml` but **delete every step that touches certs,
keychains, or release_workflow_mac.py**. Keep only checkout + cmake +
verify-universal-slices + upload-unsigned-bundles.

Trigger: `workflow_dispatch` only. Runner: `macos-14` (Apple Silicon, supports
universal builds via Xcode).

Configure with:
```
cmake -S . -B BUILD -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

**Done when:** workflow runs green; downloaded artifact contains universal
bundles (verify with `lipo -info`).

### Phase 2 — Temp keychain + cert import

Add five GitHub Actions secrets (Settings → Secrets and variables → Actions):

| Secret | Source |
|---|---|
| `MAC_DEVID_APP_P12_BASE64` | Keychain Access → export "Developer ID Application: ..." as `.p12`, then `base64 -i file.p12 \| pbcopy` |
| `MAC_DEVID_APP_P12_PASSWORD` | Password you set during the .p12 export |
| `MAC_DEVID_INSTALLER_P12_BASE64` | Same flow, "Developer ID Installer: ..." identity |
| `MAC_DEVID_INSTALLER_P12_PASSWORD` | Password for that p12 |
| `MAC_KEYCHAIN_PASSWORD` | `openssl rand -base64 24` — throwaway |

After saving the secrets, **delete local `.p12` files** (`rm` them). The
encrypted GitHub copy + your Keychain Access entries are the only persistent
copies that should exist.

Add the "Setup signing keychain" + "Verify signing identities" + "Tear down
signing keychain" (with `if: always()`) steps from Pulsar's workflow,
verbatim. The shell incantation is fragile; don't try to "improve" it on the
first pass:

- `security create-keychain -p "$KEYCHAIN_PW" build.keychain`
- `security set-keychain-settings -lut 21600 build.keychain` (no auto-lock)
- `security unlock-keychain ...`
- `security list-keychains -d user -s build.keychain login.keychain` (search list)
- `security import file.p12 -k build.keychain -P "$PW" -T /usr/bin/codesign -T /usr/bin/productsign`
- `security set-key-partition-list -S apple-tool:,apple:,codesign:,productsign: -s -k "$KEYCHAIN_PW" build.keychain` ← **the critical "no GUI prompt" line**

**Done when:** the verify step prints both
`Developer ID Application: ...` AND `Developer ID Installer: ...` lines.

### Phase 3 — Notary profile

Add three more secrets:

| Secret | Source |
|---|---|
| `APPLE_ID` | Apple ID email |
| `APPLE_TEAM_ID` | 10-char team ID — visible in your dev cert as `... (TEAMID)` |
| `APPLE_NOTARYTOOL_PASSWORD` | App-specific password from appleid.apple.com → Sign-In and Security → App-Specific Passwords |

App-specific password ≠ your Apple ID password. Generate a fresh one;
notarytool requires it.

Add the "Register notary profile" step from Pulsar's workflow:
```
xcrun notarytool store-credentials notary-profile \
  --apple-id "$APPLE_ID" --team-id "$APPLE_TEAM_ID" \
  --password "$APPLE_NOTARYTOOL_PASSWORD" \
  --keychain "$HOME/Library/Keychains/build.keychain-db"
```

The profile name `notary-profile` is hardcoded in `mac_identity.py` as
`DEFAULT_NOTARY_PROFILE`. Keep them in sync if you rename either.

**Done when:** the step prints `Success. Credentials validated.` (Apple
actually round-trips your creds before saving — strongest possible test that
all three secrets are correct.)

### Phase 4 — Wire in the orchestrator

Replace any unsigned-bundle build/upload with:
```yaml
- name: Sign, build installer, notarize, staple
  run: python SIGNED/release_workflow_mac.py

- name: Verify final pkg is notarized and stapled
  run: |
    set -e
    PKG=$(ls "RELEASE/MAC/${{ steps.version.outputs.version }}"/*.pkg)
    xcrun stapler validate "$PKG"

- name: Upload signed release
  uses: actions/upload-artifact@v4
  with:
    name: <Project>-macOS-${{ steps.version.outputs.version }}
    path: RELEASE/MAC/${{ steps.version.outputs.version }}/
```

Notarization is the slow step (1–10 min, longer under Apple load). The total
runtime budget for a release run is ~10–20 min.

**Done when:**
- Workflow runs green.
- Downloaded `.pkg` passes `xcrun stapler validate` locally.
- Double-clicking the pkg installs cleanly with no Gatekeeper warning on a
  Mac that's never seen the cert before.

### Phase 5 — Polish (optional)

- Add `concurrency:` to the workflow so two release runs can't race.
- Decide on automatic triggering: tag-push (`push: tags: ['v*']`),
  release-created, etc. Recommend staying manual at first.
- Add a preflight step that fails fast if any required secret is missing
  (Pulsar's keychain step uses `: "${VAR:?msg}"` for this).

---

## Common failure modes

| Symptom | Cause | Fix |
|---|---|---|
| `Verify signing identities` finds 0 or 1 identity | p12 exported without the private key | Re-export from Keychain Access; both the cert and the disclosure-triangle key beneath it must be selected |
| `security import ... MAC verify failed` | Wrong p12 password secret | Update the password secret |
| Workflow hangs at codesign | Missing `set-key-partition-list` | Add it; this is the GUI-prompt suppressor |
| `notarytool store-credentials` fails | Real Apple ID password instead of app-specific password | Generate app-specific at appleid.apple.com |
| Step 1 reports stale build | Build mutates VERSION.txt (the gotcha above) | Use the post-fix `update_version.py` — pure read/write |
| `notarytool submit` returns Invalid | Hardened runtime entitlements missing on a binary | Sign with `--options runtime` (Pulsar already does); fetch `notarytool log <id>` for the specific binary |
| `spctl` fails on the pkg but `stapler validate` succeeds | Known noise from `spctl` on installer pkgs | Trust `stapler validate`; treat `spctl` as informational |

---

## What you can skip if you're in a hurry

- Per-arch `build-macos-{arm64,x86_64}.yml` workflows — Pulsar keeps them as
  fast PR smoke tests, but they're independent of the release flow.
- `release_workflow_pc.py` and the Windows pipeline — totally separate.

You only need the Mac files in the table above.
