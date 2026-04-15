# Windows Code Signing Setup — Pulsar 2.1

Reference document for the code signing toolchain on this Windows development machine.

---

## Current State (as of 2026-04-14)

### signtool.exe

- **Status:** Installed and on PATH
- **Location:** `C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe`
- **PATH entry:** `C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64` (manually added)
- **Capabilities:** sign, timestamp, verify, catdb, remove

### Windows SDK Versions Installed

Multiple SDK versions are present under `C:\Program Files (x86)\Windows Kits\10\bin\`:

| Version        | Notes                                      |
|----------------|--------------------------------------------|
| 10.0.14393.0   |                                            |
| 10.0.15063.0   |                                            |
| 10.0.16299.0   |                                            |
| 10.0.17134.0   |                                            |
| 10.0.20348.0   |                                            |
| 10.0.22621.0   |                                            |
| 10.0.26100.0   | **Active — on PATH, used by signtool**     |

The minimum required SDK version for Azure Trusted Signing is **10.0.22621.0**.

### Azure Trusted Signing Client Tools

- **Status:** Installed via winget (`Microsoft.Azure.TrustedSigningClientTools` v1.0.0)
- **Install location:** `C:\Users\rdeve\AppData\Local\Microsoft\MicrosoftArtifactSigningClientTools\`
- **Key file (dlib):** `Azure.CodeSigning.Dlib.dll` — used by signtool's `/dlib` flag
- **Sample metadata:** `metadata.sample.json` included at install location

### .NET Runtimes

| Runtime                      | Version |
|------------------------------|---------|
| Microsoft.NETCore.App        | 6.0.28  |
| Microsoft.NETCore.App        | 8.0.8   |
| Microsoft.NETCore.App        | 10.0.6  |
| Microsoft.WindowsDesktop.App | 6.0.28  |
| Microsoft.WindowsDesktop.App | 8.0.8   |

.NET 8.0 runtime is present, which satisfies the Trusted Signing requirement.

### Azure CLI

- **Status:** Not installed

---

## Azure Trusted Signing — Setup Steps

The following steps are required to complete the Azure Trusted Signing setup. Steps already done are marked with a checkbox.

### Prerequisites (local machine)

- [x] Install Windows SDK (10.0.22621.0+) — signtool on PATH at v10.0.26100.0
- [x] Install Azure Trusted Signing Client Tools via winget
- [x] .NET 8.0 runtime installed
- [ ] Install Azure CLI (`winget install -e --id Microsoft.AzureCLI`) — needed for local authentication

### Azure Portal Configuration

- [x] **Step 1 — Register resource provider:** Azure Portal > Subscriptions > Resource providers > search `Microsoft.CodeSigning` > Register
- [x] **Step 2 — Create resource group:** Azure Portal > Resource groups > Create (e.g., `codesigning`, region: East US or West US)
- [x] **Step 3 — Create Artifact Signing account:** Azure Portal > search "Artifact Signing Accounts" > Create > select subscription, resource group, name, region, **Basic** tier ($9.99/mo)
- [x] **Step 4 — Assign Identity Verifier role:** Artifact Signing account > Access control (IAM) > Add Role Assignment > "Artifact Signing Identity Verifier" > add yourself
- [x] **Step 5 — Verify identity:** Artifact Signing account > Identity validations > New identity > Organization: Individual > Public > enter legal name + email > complete ID verification via QR code (government-issued ID with address required; may take 10 min to several days)
- [x] **Step 6 — Create certificate profile:** Once identity validation is complete > Certificate profiles > Create > select **Public Trust** (provides SmartScreen reputation; do NOT select Private Trust)
- [x] **Step 7 — Assign Signer role:** Access control (IAM) > Add Role Assignment > "Artifact Signing Certificate Profile Signer" > add yourself

### Post-Setup — Create metadata.json

After completing Azure portal setup, create a `metadata.json` file for signing:

```json
{
  "Endpoint": "<Artifact Signing Account Endpoint>",
  "CodeSigningAccountName": "<Artifact Signing Account Name>",
  "CertificateProfileName": "<Certificate Profile Name>"
}
```

Store this file in a known location (e.g., the repo root or a secure config directory). Do NOT commit it to source control.

### Signing Command

Once everything is configured, sign a file with:

```powershell
signtool.exe sign /v /debug /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib "C:\Users\rdeve\AppData\Local\Microsoft\MicrosoftArtifactSigningClientTools\Azure.CodeSigning.Dlib.dll" /dmdf "<path-to>\metadata.json" <file-to-sign>
```

**Flags breakdown:**
| Flag | Purpose |
|------|---------|
| `/v` | Verbose output |
| `/debug` | Additional debug info |
| `/fd SHA256` | File digest algorithm |
| `/tr` | RFC 3161 timestamp server URL |
| `/td SHA256` | Timestamp digest algorithm |
| `/dlib` | Path to Azure Code Signing dlib DLL |
| `/dmdf` | Path to metadata.json |

### Authentication

For local signing, authenticate via one of:
- **Azure CLI** — `az login` before running signtool (recommended for local dev)
- **Environment variables** — set `AZURE_TENANT_ID`, `AZURE_CLIENT_ID`, `AZURE_CLIENT_SECRET` for CI/CD
- **Managed Identity** — automatic on Azure VMs, App Service, Functions

The dlib uses `DefaultAzureCredential` and tries providers in order: Environment > ManagedIdentity > WorkloadIdentity > SharedTokenCache > VisualStudio > VS Code > AzureCLI > AzurePowerShell > AzureDeveloperCLI > InteractiveBrowser.

### Verification

After signing, verify with:

```powershell
signtool.exe verify /pa /v <signed-file>
```

---

## Notes

- The dlib DLL is at `C:\Users\rdeve\AppData\Local\Microsoft\MicrosoftArtifactSigningClientTools\Azure.CodeSigning.Dlib.dll` (no architecture subfolder in the current install).
- Public Trust certificates provide SmartScreen reputation on Windows, eliminating the "unknown publisher" warning for end users.
- The Basic tier costs $9.99/month and is sufficient for individual developer use.
