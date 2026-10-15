# Windows counterpart of build_pycunls_in_docker.sh: builds the pycunls wheel with MSVC + Ninja
# using the active Python, which needs scikit-build-core and nanobind installed.
# Run from a VS x64 developer shell.
#
# Usage: .\scripts\build_pycunls_windows.ps1 [output_dir]   (default: .\dist)
# Extra CMake flags go in CMAKE_ARGS, with forward slashes in paths. Never set SKBUILD_CMAKE_ARGS:
# it replaces the cmake.args of pyproject.toml, and the result is a wheel of the C++ install
# instead of the extension.
param([string]$OutputDir = "dist")
$ErrorActionPreference = "Stop"

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue) -or $env:VSCMD_ARG_TGT_ARCH -ne "x64") {
  Write-Error ("MSVC x64 environment not found. Run this script from an 'x64 Native Tools Command Prompt " +
    "for VS' (or after calling vcvars64.bat), e.g.:`n  powershell -ExecutionPolicy Bypass -File " +
    "scripts\build_pycunls_windows.ps1")
}
if ($env:SKBUILD_CMAKE_ARGS) { Write-Error "Unset SKBUILD_CMAKE_ARGS and pass extra flags in CMAKE_ARGS." }

$SourceDir = Split-Path -Parent $PSScriptRoot
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$OutputDir = (Resolve-Path $OutputDir).Path

$env:CMAKE_GENERATOR = "Ninja"
& python -m pip wheel (Join-Path $SourceDir "python") --no-build-isolation --no-deps --wheel-dir $OutputDir
if ($LASTEXITCODE -ne 0) { Write-Error "pip wheel failed with exit code $LASTEXITCODE" }
Get-ChildItem (Join-Path $OutputDir "*.whl")
