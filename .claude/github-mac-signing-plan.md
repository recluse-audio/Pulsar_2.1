# GitHub Actions: Mac Sign + Notarize Workflow

**Status: SHIPPED.** End-to-end working as of 2026-05-06. Phases 1–4 complete;
Phase 5 (polish) optional. See `.github/workflows/release-macos.yml` for the
final implementation.

For replicating this setup in *another* repo, see
`setup-mac-signing-for-new-project.md` in this directory.

---

A plan for adding a remote (GitHub-hosted) build that signs, notarizes, and
staples the macOS release `.pkg`, mirroring what `SIGNED/release_workflow_mac.py`
does on a developer's laptop today.

We worked in **small, measurable chunks**. Each phase had a clear "done when..."
gate so we could stop, run something, and confirm it worked before moving on.

---

## Background — what already exists

So we don't accidentally rebuild things:

- `SIGNED/release_workflow_mac.py` — the orchestrator. Already does steps 1–9
  (verify → sign builds → build pkg → sign pkg → notarize → staple → copy to
  `RELEASE/MAC/<version>/`). **We do not need to rewrite this.** The CI job's
  job is to *make it runnable on a fresh GitHub runner.*
- `SIGNED/MAC/mac_identity.py` — looks up signing identities from the keychain.
  Already supports CI overrides via env vars:
  - `PULSAR_DEVID_APP` — Developer ID Application identity name
  - `PULSAR_DEVID_INSTALLER` — Developer ID Installer identity name
  - `PULSAR_NOTARY_PROFILE` — name of the notarytool keychain profile
- `SIGNED/MAC/notarize_mac.py` — calls `xcrun notarytool submit --wait` against
  a keychain profile, then `xcrun stapler staple`.
- `.github/workflows/build-macos-arm64.yml` and `build-macos-x86_64.yml` —
  unsigned per-arch CI builds. We **leave these alone** (they're fast CI smoke
  tests). The new file is for releases.

The PC equivalent (`release_workflow_pc.py`) is local-only today; the Mac
remote workflow we're building has no direct PC counterpart. The only PC
parallel is "we follow the same orchestrator → release directory" pattern.

---

## Architecture — how the pieces fit on a CI runner

A GitHub `macos-14` runner is a clean VM. It has Xcode and `xcrun notarytool`
preinstalled, but **no signing certs** and **no keychain profile**. Our job is
to provision those temporarily, run the existing release script, and tear
everything down.

```
┌──────────────────────────────────────────────────────────────┐
│ macos-14 runner (ephemeral)                                  │
│                                                              │
│  1. Checkout repo (with submodules)                          │
│  2. Create temp keychain `build.keychain`                    │
│  3. Import 2 p12 certs into it (App + Installer)             │
│  4. Register notarytool profile in that keychain             │
│  5. cmake → build universal Release                          │
│  6. python SIGNED/release_workflow_mac.py                    │
│       └─ already orchestrates sign → pkg → notarize → staple │
│  7. Upload RELEASE/MAC/<version>/ as artifact                │
│  8. (always) delete temp keychain                            │
└──────────────────────────────────────────────────────────────┘
```

The two trust boundaries:

- **Secrets in:** GitHub Actions secrets (p12 base64, p12 passwords, Apple ID,
  team ID, app-specific password, keychain password). They enter as env vars
  on specific steps only.
- **Artifacts out:** the signed/notarized `.pkg` plus the signed bundles,
  uploaded via `actions/upload-artifact@v4`.

Why a *temp* keychain (not `login.keychain`)? It's the GitHub-recommended
pattern: certs and profile vanish when the runner is destroyed (or our cleanup
step runs), so a leaked p12 can't be reused later.

---

## Universal binary decision

Recommendation: **build universal (`arm64;x86_64`)** in this workflow.

- Local releases ship one `.pkg` that installs on both architectures — the
  CI flow should match.
- One signed/notarized `.pkg` is simpler than two. Notarization isn't free
  (1–10 min); doing it twice doubles the slowest step.
- The existing per-arch workflows (`build-macos-arm64.yml`, `-x86_64.yml`)
  stay as fast smoke tests for PRs.

Trade-off: universal builds are ~2× larger and take longer to compile. For a
release workflow that runs on demand, that's fine.

---

## Required GitHub Actions secrets

We'll add these in repo Settings → Secrets and variables → Actions. **Do
not run the workflow until all seven exist.**

| Secret | What it is | How to obtain |
|---|---|---|
| `MAC_DEVID_APP_P12_BASE64` | Developer ID Application cert + private key, exported as `.p12`, then `base64 -i cert.p12 \| pbcopy` | Keychain Access → export your "Developer ID Application: ..." identity |
| `MAC_DEVID_APP_P12_PASSWORD` | Password set during the p12 export | You choose this at export time |
| `MAC_DEVID_INSTALLER_P12_BASE64` | Same, for "Developer ID Installer: ..." | Same flow |
| `MAC_DEVID_INSTALLER_P12_PASSWORD` | Password for the installer p12 | You choose at export time |
| `MAC_KEYCHAIN_PASSWORD` | Throwaway password for the temp keychain we create on the runner | Generate any random string |
| `APPLE_ID` | Apple ID email | Yours |
| `APPLE_TEAM_ID` | 10-char Developer team ID | Apple Developer portal → Membership |
| `APPLE_NOTARYTOOL_PASSWORD` | App-specific password for notarization | appleid.apple.com → Sign-In and Security → App-Specific Passwords |

---

## Implementation phases

Each phase is small enough to verify on its own. Don't move to the next phase
until the current one's "done when" is green.

### Phase 1 — Skeleton workflow that builds (no signing) ✅

**Goal:** get a `release-macos.yml` that compiles a universal Release build on
a macos-14 runner. No secrets, no signing yet. This proves the runner setup
works.

**Steps:**
1. Create `.github/workflows/release-macos.yml` with `workflow_dispatch`
   trigger and a single job on `macos-14`.
2. Checkout (with submodules), install cmake, configure with
   `-G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`, build `Pulsar_All`
   target Release.
3. Upload the unsigned VST3/AU/Standalone bundles as artifacts so we can
   eyeball them.

**Done when:** the workflow runs green from the Actions tab and the artifacts
zip contains a universal `.vst3`, `.component`, `.app`. (We can verify with
`lipo -info` locally on a downloaded artifact.)

### Phase 2 — Add temp keychain + import certs ✅

**Goal:** prove we can land both p12s on the runner without leaking them.

**Steps:**
1. Add the four cert-related secrets to GitHub.
2. New workflow step "Setup signing keychain" (runs after checkout, before
   build):
   - `security create-keychain -p "$KEYCHAIN_PW" build.keychain`
   - `security set-keychain-settings -lut 21600 build.keychain` (no auto-lock)
   - `security unlock-keychain -p "$KEYCHAIN_PW" build.keychain`
   - `security list-keychains -d user -s build.keychain login.keychain`
     (so `find-identity` sees it)
   - Decode each base64 secret → temp `.p12` file → `security import` with
     `-T /usr/bin/codesign -T /usr/bin/productsign` so those tools can use the key.
   - `security set-key-partition-list -S apple-tool:,apple:,codesign:,productsign: -s -k "$KEYCHAIN_PW" build.keychain`
     (suppresses the GUI prompt that would otherwise hang the runner).
3. Add an "always" cleanup step at the end of the job:
   `security delete-keychain build.keychain || true`
4. Diagnostic step: `security find-identity -v build.keychain` — should print
   both identities.

**Done when:** the diagnostic step in the workflow log shows
`Developer ID Application: ...` AND `Developer ID Installer: ...`.

### Phase 3 — Register notary profile ✅

**Goal:** notarytool can submit without prompts.

**Steps:**
1. Add `APPLE_ID`, `APPLE_TEAM_ID`, `APPLE_NOTARYTOOL_PASSWORD` secrets.
2. New step "Register notary profile" after the keychain is set up:
   ```
   xcrun notarytool store-credentials notary-profile \
     --apple-id "$APPLE_ID" \
     --team-id "$APPLE_TEAM_ID" \
     --password "$APPLE_NOTARYTOOL_PASSWORD" \
     --keychain "$HOME/Library/Keychains/build.keychain-db"
   ```
   The profile name `notary-profile` matches `DEFAULT_NOTARY_PROFILE` in
   `mac_identity.py`, so no env override needed.

**Done when:** the step exits 0. (No further verification at this stage —
real verification happens in Phase 4 when we actually submit something.)

### Phase 4 — Wire in `release_workflow_mac.py` ✅

> **Gotcha discovered during this phase:** `HELPER_SCRIPTS/update_version.py`
> previously incremented `VERSION.txt` on every build via the
> `update_version_header` CMake target. That created a 1-version skew between
> the bundle's `CFBundleShortVersionString` (locked at configure-time project
> version) and `VERSION.txt` (mutated post-build), which `release_workflow_mac.py`
> step 1 correctly flagged. Fix: rewrote `update_version.py` to be pure
> (read VERSION.txt → write Version.h, no mutation). Bumping is now an
> explicit, deliberate edit. Carries forward to other projects — see the
> setup guide.

**Goal:** end-to-end signed + notarized + stapled `.pkg`.

**Steps:**
1. After build succeeds, add step "Run release workflow":
   `python SIGNED/release_workflow_mac.py`
   No env overrides needed — the keychain has the right identities and the
   profile is named `notary-profile`.
2. Upload `RELEASE/MAC/<version>/` as the primary release artifact.
   (Read version from `VERSION.txt` to compute the path.)

**Done when:**
- Workflow finishes green.
- Downloaded `.pkg` passes `spctl -a -vvv -t install path/to.pkg` locally.
- `xcrun stapler validate path/to.pkg` reports "ready to be distributed".

### Phase 5 — Polish

Only once Phase 4 is solid:
- Add a job-level concurrency guard so two release runs can't race.
- Decide trigger: stay on `workflow_dispatch` only, or also fire on
  `push: tags: ['v*']`. Recommend manual-only at first.
- Optional: a preflight "verify all required secrets exist" step that fails
  fast with a clear message rather than partway through cert import.

---

## Files we will touch

- **New:** `.github/workflows/release-macos.yml` — the only code we write.
- **No changes expected** to `release_workflow_mac.py`, `mac_identity.py`,
  the `sign_mac_*.py` scripts, or `notarize_mac.py`. They're already
  CI-friendly. If we discover a gap, we revisit and update this doc.

---

## Resolved decisions

1. ✅ Universal binary (`arm64;x86_64`) — confirmed.
2. ✅ Trigger: `workflow_dispatch` only.
3. ✅ Secret name `APPLE_NOTARYTOOL_PASSWORD` (renamed from `APPLE_APP_PASSWORD`
   for clarity about its purpose).
