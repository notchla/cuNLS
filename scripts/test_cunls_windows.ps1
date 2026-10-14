# Windows counterpart of test_cunls_in_docker.sh: runs the C++ tests of a build_cunls_windows.ps1
# output directory that was configured with EXTRA_CMAKE_ARGS=-DBUILD_TESTING=ON.
#
# Usage: .\scripts\test_cunls_windows.ps1 <build_output_dir>
param([Parameter(Mandatory = $true)][string]$OutputDir)
$ErrorActionPreference = "Stop"

$BuildDir = Join-Path $OutputDir "build_shared"
if (-not (Test-Path $BuildDir)) {
  Write-Error ("$BuildDir not found. Run first:`n" +
    "  `$env:EXTRA_CMAKE_ARGS = '-DBUILD_TESTING=ON'; .\scripts\build_cunls_windows.ps1 $OutputDir")
}

& nvidia-smi
if ($LASTEXITCODE -ne 0) { Write-Error "nvidia-smi failed: no usable NVIDIA GPU or driver." }

# CUDA 13 installs its DLLs in bin\x64 instead of bin.
if ($env:CUDA_PATH) {
  $env:PATH = "$(Join-Path $env:CUDA_PATH 'bin\x64');$(Join-Path $env:CUDA_PATH 'bin');$env:PATH"
}

$ResultsDir = Join-Path $BuildDir "test-results"
New-Item -ItemType Directory -Force -Path $ResultsDir | Out-Null
$Results = Join-Path $ResultsDir "results.xml"
& ctest --test-dir $BuildDir --output-on-failure --output-junit $Results
$CtestExit = $LASTEXITCODE
if (Test-Path $Results) { Copy-Item -Force $Results (Join-Path $OutputDir "cpp-test-results.xml") }
exit $CtestExit
