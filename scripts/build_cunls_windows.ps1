# Windows counterpart of build_cunls_in_docker.sh: builds and installs the shared and static
# Release variants with MSVC + Ninja. Run from a VS x64 developer shell.
#
# Usage: .\scripts\build_cunls_windows.ps1 [output_dir]   (default: .\build_windows)
# EXTRA_CMAKE_ARGS (space-separated) is appended to both configure steps.
param([string]$OutputDir = "build_windows")
$ErrorActionPreference = "Stop"

function Assert-Success([string]$Step) {
  if ($LASTEXITCODE -ne 0) { Write-Error "$Step failed with exit code $LASTEXITCODE" }
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue) -or $env:VSCMD_ARG_TGT_ARCH -ne "x64") {
  Write-Error ("MSVC x64 environment not found. Run this script from an 'x64 Native Tools Command Prompt " +
    "for VS' (or after calling vcvars64.bat), e.g.:`n  powershell -ExecutionPolicy Bypass -File " +
    "scripts\build_cunls_windows.ps1")
}

$SourceDir = Split-Path -Parent $PSScriptRoot
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$OutputDir = (Resolve-Path $OutputDir).Path
$ExtraArgs = @(($env:EXTRA_CMAKE_ARGS -split '\s+') | Where-Object { $_ })

foreach ($Variant in @(@{ Name = "shared"; Shared = "ON" }, @{ Name = "static"; Shared = "OFF" })) {
  $BuildDir = Join-Path $OutputDir "build_$($Variant.Name)"
  cmake -G Ninja -S $SourceDir -B $BuildDir -DCMAKE_BUILD_TYPE=Release `
    "-DCMAKE_INSTALL_PREFIX=$(Join-Path $OutputDir $Variant.Name)" "-DBUILD_SHARED_LIBS=$($Variant.Shared)" @ExtraArgs
  Assert-Success "CMake configure ($($Variant.Name))"
  cmake --build $BuildDir
  Assert-Success "Build ($($Variant.Name))"
  cmake --install $BuildDir
  Assert-Success "Install ($($Variant.Name))"
}

# cunls loads cuDSS at runtime and never bundles it: package the downloaded redistributable on its
# own so consumers of the cuDSS solver can put cudss\bin on PATH.
$CudssSrc = Join-Path $OutputDir "build_shared\_deps\cudss-src"
if (Test-Path $CudssSrc) {
  $CudssOut = Join-Path $OutputDir "cudss"
  New-Item -ItemType Directory -Force -Path $CudssOut | Out-Null
  Copy-Item -Recurse -Force (Join-Path $CudssSrc "include"), (Join-Path $CudssSrc "bin") $CudssOut
}
