#define MyAppName "Pulsar"
#define MyAppVersion "1.1.65"
#define MyAppPublisher "recluse-audio"
#define MyAppURL "https://recluse-audio.com"
#define VST3Source SourcePath + "\..\..\BUILD\Pulsar_artefacts\Release\VST3\Pulsar.vst3"
#define OutputDir SourcePath + "\BUILD"

[Setup]
AppId={{B2A4F1D3-9E7C-4A2B-8F6D-3C1E5A0D4B78}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppPublisher}\{#MyAppName}
UninstallFilesDir={app}
DefaultGroupName={#MyAppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename=Pulsar_v{#MyAppVersion}_Windows_Installer
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#VST3Source}\*"; DestDir: "{commoncf}\VST3\Pulsar.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
