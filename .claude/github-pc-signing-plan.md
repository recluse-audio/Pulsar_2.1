# GitHub Actions: Windows Sign + Release Workflow

**Status: PLANNED.** Not yet shipped. Mirrors the Mac plan
(`github-mac-signing-plan.md`) where the platforms agree, and calls out the
PC-specific divergences explicitly.

A plan for adding a remote (GitHub-hosted) build that signs Windows release
artifacts and packages a signed installer, mirroring what
`SIGNED/release_workflow_pc.py` does on a developer's laptop today.

We work in **small, measurable chunks**. Each phase has a clear "done when..."
gate so we can stop, run something, and confirm it worked before moving on.

---

## Background — what already exists

So we don't accidentally rebuild things:

- `SIGNED/release_workflow_pc.py` — the orchestrator. Already does steps 1–9
  (verify → sign builds → update `.iss` version → build installer → sign
  installer → copy to `RELEASE/PC/<version>/`). **We do not need to rewrite
  this.** The CI job's job is to *make it runnable on a fresh GitHub runner.*
- `SIGNED/PC/sign_pc_{vst3,standalone,installer}.py` — invoke
  `signtool.exe sign` with the Azure Code Signing dlib + metadata.json.
- `SIGNED/PC/check_if_signed_pc.py` — `signtool verify /pa` wrapper.
- `INSTALLERS/PC/Pulsar.iss` + `INSTALLERS/PC/build_pc_installer.py` — Inno
  Setup 6 project and its build wrapper.
- `.github/workflows/build-windows.yml` — unsigned Windows CI build. We
  **leave it alone** (fast smoke test for PRs). The new file is for releases.
- `docs/windows-code-signing-setup.md` — the local-machine setup guide. The
  CI runner needs the same toolchain installed at job time.

The Mac remote workflow (`release-macos.yml`) is the closest analogue. The PC
flow drops the notarization step (no Windows equivalent) and swaps the
keychain/p12 dance for an Azure Trusted Signing service-principal login.

---

## Architecture — how the pieces fit on a CI runner

A GitHub `windows-latest` runner is a clean VM. It has the Windows SDK
(signtool), MSBuild, and recent .NET preinstalled, but **no Azure auth**, **no
metadata.json**, and **no Azure Code Signing dlib**, and **no Inno Setup**.
Our job is to provision those temporarily, run the existing release script,
and let the runner VM evaporate.

```
┌──────────────────────────────────────────────────────────────┐
│ windows-latest runner (ephemeral)                            │
│                                                              │
│  1. Checkout repo (with submodules)                          │
│  2. Install Inno Setup 6 (winget or choco)                   │
│  3. Install Azure Trusted Signing Client Tools (winget)      │
│  4. Write metadata.json from secrets to ~/.azure/            │
│  5. Export AZURE_* env vars for DefaultAzureCredential       │
│  6. cmake → build Release (VST3 + Standalone)                │
│  7. python SIGNED/release_workflow_pc.py                     │
│       └─ sign builds → bump .iss → build installer → sign    │
│  8. Verify final installer with signtool verify /pa          │
│  9. Upload RELEASE/PC/<version>/ as artifact                 │
│ 10. (always) clear ~/.azure/metadata.json                    │
└──────────────────────────────────────────────────────────────┘
```

The two trust boundaries:

- **Secrets in:** GitHub Actions secrets (Azure tenant/client/secret for the
  service principal that owns the "Artifact Signing Certificate Profile
  Signer" role; account endpoint, account name, profile name for
  metadata.json). They enter as env vars on specific steps only.
- **Artifacts out:** the signed `.exe` installer plus the signed VST3 bundle
  and standalone exe, uploaded via `actions/upload-artifact@v4`.

Why a service principal (not a user `az login`)? `DefaultAzureCredential`
picks up `AZURE_TENANT_ID` / `AZURE_CLIENT_ID` / `AZURE_CLIENT_SECRET` from
the environment first. Service principals are the standard for unattended CI
auth — no MFA, no browser, scoped to exactly the Signer role on one cert
profile. Revocable in seconds if ever leaked.

---

## Architecture decisions

- **x64 only.** No universal-binary equivalent on Windows. Existing
  `Pulsar.iss` packages a single `x86_64-win` slice; CI matches.
- **One job, not a matrix.** Mirrors `release-macos.yml`.
- **Trigger: `workflow_dispatch` only** at first. Optionally add tag-push
  later in Phase 5.
- **No notarization.** Windows has no notarytool equivalent. SmartScreen
  reputation comes from the Azure Trusted Signing **Public Trust**
  certificate over time. The release is "done" once the installer is signed
  and `signtool verify /pa` passes.

---

## Required GitHub Actions secrets

Add in repo Settings → Secrets and variables → Actions. **Do not run the
workflow until all six exist.**

| Secret | What it is | How to obtain |
|---|---|---|
| `AZURE_TENANT_ID` | Tenant (directory) ID of the Azure AD where the Trusted Signing account lives | Azure Portal → Azure Active Directory → Overview |
| `AZURE_CLIENT_ID` | Application (client) ID of the service principal that has Signer role on the cert profile | Create an App Registration → copy "Application (client) ID" |
| `AZURE_CLIENT_SECRET` | Client secret for that app registration | App Registration → Certificates & secrets → New client secret |
| `ACS_ENDPOINT` | Artifact Signing account endpoint URL (e.g. `https://eus.codesigning.azure.net/`) | Azure Portal → Artifact Signing account → Overview |
| `ACS_ACCOUNT_NAME` | Artifact Signing account name | Same place |
| `ACS_PROFILE_NAME` | Certificate profile name (Public Trust) under that account | Artifact Signing account → Certificate profiles |

Setup checklist for the service principal (one-time, in Azure Portal):

1. Azure AD → App registrations → New registration. Note the tenant + client IDs.
2. Certificates & secrets → New client secret. Copy the value immediately.
3. Artifact Signing account → Access control (IAM) → Add role assignment →
   "Artifact Signing Certificate Profile Signer" → assign to the new app.
   (Scope to the *certificate profile*, not the whole subscription.)

---

## Implementation phases

Each phase is small enough to verify on its own. Don't move to the next phase
until the current one's "done when" is green.

### Phase 1 — Skeleton workflow that builds (no signing)

**Goal:** get a `release-windows.yml` that compiles a Release VST3 +
Standalone on a `windows-latest` runner. No secrets, no signing yet. This
proves the runner setup works.

**Steps:**
1. Create `.github/workflows/release-windows.yml` with `workflow_dispatch`
   trigger and a single job on `windows-latest`.
2. Checkout (with submodules), install cmake (`lukka/get-cmake@latest`),
   configure with `-DCMAKE_BUILD_TYPE=Release`, build `Pulsar_All` Release.
3. Upload the unsigned `Pulsar.vst3` and `Pulsar.exe` as artifacts so we can
   eyeball them.

**Done when:** the workflow runs green from the Actions tab and the
artifacts zip contains a usable `.vst3` + `.exe` (open in DAW locally to
sanity-check).

### Phase 2 — Install signing toolchain on the runner

**Goal:** prove signtool + the Azure Code Signing dlib are reachable on the
runner before we attempt to use them.

**Steps:**
1. Add a "Install Inno Setup" step:
   `choco install innosetup --version=6.2.2 -y --no-progress`
   (or `winget install -e --id JRSoftware.InnoSetup`). Verify with
   `where iscc.exe`.
2. Add a "Install Azure Trusted Signing Client Tools" step:
   `winget install -e --id Microsoft.Azure.TrustedSigningClientTools
   --accept-source-agreements --accept-package-agreements`.
3. Diagnostic: confirm both `signtool.exe` and the dlib are found:
   ```pwsh
   where signtool.exe
   Test-Path "$env:LOCALAPPDATA\Microsoft\MicrosoftArtifactSigningClientTools\Azure.CodeSigning.Dlib.dll"
   ```

**Done when:** the diagnostic step prints a path for `signtool.exe` and
`True` for the dlib.

> Gotcha: `windows-latest` images change. If `winget` isn't preinstalled in
> the chosen runner image, fall back to downloading the dlib NuGet package
> directly (`Microsoft.Trusted.Signing.Client`) and pointing
> `SIGNED/PC/sign_pc_*.py` at it via env override. Defer that change unless
> we hit the issue.

### Phase 3 — Wire Azure auth + write metadata.json

**Status: IN PROGRESS — blocked on Azure RBAC/profile visibility.**

Current symptom in CI (`release-windows.yml` smoke step):
- `az resource list --resource-type Microsoft.CodeSigning/.../certificateProfiles -o json` returns `[]` for the SP.
- `signtool sign` fails: "After EKU filter, 0 certs were left. After expiry filter, 0 certs were left." → dlib received zero certs from the service.

Root cause hypothesis (verifying one step at a time with user in Azure portal):
1. SP missing `Artifact Signing Certificate Profile Signer` role on the cert profile resource (most likely).
2. Identity Validation on `RecluseToolsAccount` not in `Completed` state — profile cannot issue certs even with role.
3. `ACS_ENDPOINT` region mismatch vs account location.

Note: Microsoft rebranded again — portal shows **"Artifact Signing Account"** (was Trusted Signing / Azure Code Signing). Same resource provider `Microsoft.CodeSigning/codeSigningAccounts`. Role name to assign: `Artifact Signing Certificate Profile Signer`.

NuGet install path confirmed working (commit `c71aa14` switched winget → nuget for hosted runner compat). Dlib lands at `%LOCALAPPDATA%\Microsoft\MicrosoftArtifactSigningClientTools\`.

**Goal:** signtool can sign a throwaway test exe end-to-end.

**Steps:**
1. Add the six secrets listed above to the repo.
2. New step "Configure Azure Code Signing":
   ```pwsh
   $azureDir = Join-Path $HOME ".azure"
   New-Item -ItemType Directory -Force -Path $azureDir | Out-Null
   $metadata = @{
     Endpoint               = "$env:ACS_ENDPOINT"
     CodeSigningAccountName = "$env:ACS_ACCOUNT_NAME"
     CertificateProfileName = "$env:ACS_PROFILE_NAME"
   } | ConvertTo-Json
   Set-Content -Path (Join-Path $azureDir "metadata.json") -Value $metadata -Encoding utf8
   ```
   Step env block exposes `ACS_ENDPOINT`, `ACS_ACCOUNT_NAME`, `ACS_PROFILE_NAME`.
3. Set `AZURE_TENANT_ID` / `AZURE_CLIENT_ID` / `AZURE_CLIENT_SECRET` as
   **job-level** env vars (not echoed). `DefaultAzureCredential` picks them
   up automatically.
4. Smoke test: build a tiny throwaway exe (or take the just-built
   `Pulsar.exe`) and run a single `signtool.exe sign ...` call against it.
   Verify with `signtool verify /pa /v` in the same step.

**Done when:** the smoke-test sign + verify step exits 0 and the verify
output names the Public Trust cert chain.

### Phase 4 — Wire in `release_workflow_pc.py`

**Goal:** end-to-end signed VST3, standalone, and installer in
`RELEASE/PC/<version>/`.

**Steps:**
1. After the build succeeds and Azure is configured, add step
   "Run release workflow":
   `python SIGNED/release_workflow_pc.py`
   No env overrides needed if the local-vs-CI path layout matches
   (`~/.azure/metadata.json` and the dlib in `%LOCALAPPDATA%\Microsoft\...`
   are the defaults the existing scripts already expect).
2. After it finishes, run the verification probe:
   ```pwsh
   $version = Get-Content VERSION.txt
   $installer = Get-ChildItem "RELEASE/PC/$version/Pulsar_v*_Windows_Installer.exe" | Select-Object -First 1
   signtool.exe verify /pa /v $installer.FullName
   ```
3. Upload `RELEASE/PC/<version>/` as the primary release artifact.

**Done when:**
- Workflow finishes green.
- Downloaded installer's "Digital Signatures" tab in Properties shows the
  Public Trust cert.
- Installing on a clean Windows 11 VM does **not** trigger a SmartScreen
  "unknown publisher" warning (may take time after first signing — Public
  Trust reputation is built up across signed downloads).

> Gotcha to watch for (mirrors the Mac one): if `update_version.py` ever
> mutates `VERSION.txt` again, `release_workflow_pc.py` step 1 will flag a
> mismatch. The PC orchestrator currently does not version-check bundles
> (Mac does via `CFBundleShortVersionString`) but the `.iss` rewrite at
> step 6 will produce the wrong installer name. Keep `update_version.py`
> pure (read-only) — same rule as Mac.

### Phase 5 — Polish

Only once Phase 4 is solid:
- Add a job-level `concurrency` guard so two release runs can't race.
- Decide trigger: stay on `workflow_dispatch` only, or also fire on
  `push: tags: ['v*']`. Recommend manual-only at first.
- Optional: a preflight "verify all required secrets exist" step that fails
  fast with a clear message rather than partway through metadata.json write.
- Optional: a teardown step that wipes `~/.azure/metadata.json` even on
  failure, mirroring the Mac `delete-keychain` pattern. (Less critical
  because the runner VM is destroyed anyway, but it's cheap defense.)
- Optional: combine Mac + Windows release jobs into a single
  `release.yml` with a matrix or two parallel jobs, so one click ships both
  platforms.

---

## Files we will touch

- **New:** `.github/workflows/release-windows.yml` — the only code we write.
- **No changes expected** to `release_workflow_pc.py`, `sign_pc_*.py`,
  `check_if_signed_pc.py`, `build_pc_installer.py`, or `Pulsar.iss`. They
  are already CI-friendly because they read paths from `Path.home()` and
  `INFO`. If we discover a gap (e.g. a hardcoded local path), we revisit
  and update this doc.
- **Possible:** thin env-override hooks in `sign_pc_*.py` for the dlib path
  if `winget` doesn't land it at the expected `%LOCALAPPDATA%` path on the
  runner image. Defer until we hit it.

---

## Open decisions

1. Trigger: `workflow_dispatch` only at first; reconsider tag-push in Phase 5.
2. Whether to fold the Inno Setup install into a reusable composite action
   (only if a second project starts needing it).
3. Whether to upload a flat zip (matching `build-windows.yml` style) in
   addition to the directory artifact, for one-click download UX.
