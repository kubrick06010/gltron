param(
    [string]$Configuration = "Release",
    [string]$Platform = "Win32",
    [string]$Version = "0.72"
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$DepsRoot = Join-Path $RepoRoot "win32\deps"
$DepsZip = Join-Path $RepoRoot "win32\dependencies.zip"
$DistRoot = Join-Path $RepoRoot "dist"
$PackageRoot = Join-Path $DistRoot "gltron-windows-$Version"
$PackageBin = Join-Path $PackageRoot "bin"

if (-not (Test-Path (Join-Path $DepsRoot "dll"))) {
    if (-not (Test-Path $DepsZip)) {
        throw "Missing Windows dependencies archive: $DepsZip"
    }

    Expand-Archive -LiteralPath $DepsZip -DestinationPath $DepsRoot -Force
}

$VsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$MsBuild = $null
if (Test-Path $VsWhere) {
    $InstallPath = (& $VsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath | Select-Object -First 1)
    if ($InstallPath) {
        $Candidate = Join-Path $InstallPath "MSBuild\Current\Bin\MSBuild.exe"
        if (Test-Path $Candidate) {
            $MsBuild = $Candidate
        }
    }
}

if (-not $MsBuild) {
    $MsBuildCommand = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($MsBuildCommand) {
        $MsBuild = $MsBuildCommand.Source
    }
}

if (-not $MsBuild) {
    throw "MSBuild.exe was not found. Install Visual Studio Build Tools with the C++ workload."
}

$Project = Join-Path $RepoRoot "gltron.vcxproj"
& $MsBuild $Project /p:Configuration=$Configuration /p:Platform=$Platform /p:PlatformToolset=v143 /m /v:minimal
if ($LASTEXITCODE -ne 0) {
    throw "MSBuild failed with exit code $LASTEXITCODE"
}

if (Test-Path $PackageRoot) {
    Remove-Item $PackageRoot -Recurse -Force
}
New-Item $PackageBin -ItemType Directory -Force | Out-Null

Copy-Item (Join-Path $RepoRoot "$Configuration\gltron.exe") $PackageBin
Get-ChildItem (Join-Path $DepsRoot "dll") -Filter "*.dll" |
    Where-Object { $_.Name -notlike "*_d.dll" } |
    Copy-Item -Destination $PackageBin

foreach ($ResourceDirectory in @("scripts", "data", "art", "levels", "music", "sounds")) {
    Copy-Item (Join-Path $RepoRoot $ResourceDirectory) $PackageRoot -Recurse
}

# Autotools metadata is useful in the source tree but not at runtime.
Get-ChildItem $PackageRoot -Recurse -File |
    Where-Object { $_.Name -in @("Makefile.am", "Makefile.in") } |
    Remove-Item -Force

$RequiredPackageFiles = @(
    (Join-Path $PackageBin "gltron.exe"),
    (Join-Path $PackageBin "SDL.dll"),
    (Join-Path $PackageBin "sdl_sound.dll"),
    (Join-Path $PackageRoot "scripts\main.lua"),
    (Join-Path $PackageRoot "music\song_revenge_of_cats.it")
)
foreach ($RequiredPackageFile in $RequiredPackageFiles) {
    if (-not (Test-Path $RequiredPackageFile)) {
        throw "Package is missing required runtime file: $RequiredPackageFile"
    }
}

$ZipPath = Join-Path $DistRoot "gltron-windows-$Version.zip"
if (Test-Path $ZipPath) {
    Remove-Item $ZipPath -Force
}
Compress-Archive -Path $PackageRoot -DestinationPath $ZipPath -CompressionLevel Optimal

Write-Output "Package directory: $PackageRoot"
Write-Output "Package archive:   $ZipPath"
