# Windows counterpart of test_pycunls_in_docker.sh: installs the wheel and its test extras into
# the active Python (use a dedicated venv) and runs the Python tests.
#
# Usage: .\scripts\test_pycunls_windows.ps1 <wheel_dir>
param([Parameter(Mandatory = $true)][string]$WheelDir)
$ErrorActionPreference = "Stop"

$Wheel = Get-ChildItem (Join-Path $WheelDir "pycunls-*-win_amd64.whl") -ErrorAction SilentlyContinue |
  Select-Object -First 1
if (-not $Wheel) {
  Write-Error "No pycunls Windows wheel in $WheelDir. Run first:`n  .\scripts\build_pycunls_windows.ps1 $WheelDir"
}

& nvidia-smi
if ($LASTEXITCODE -ne 0) { Write-Error "nvidia-smi failed: no usable NVIDIA GPU or driver." }

& python -m pip install --force-reinstall "$($Wheel.FullName)[test]"
if ($LASTEXITCODE -ne 0) { Write-Error "pip install failed with exit code $LASTEXITCODE" }

$JunitXml = Join-Path (Resolve-Path $WheelDir) "python-test-results.xml"
# Run from the repository root: from python\ the source package would shadow the installed one.
Push-Location (Split-Path -Parent $PSScriptRoot)
try {
  & python -m pytest python/tests "--junitxml=$JunitXml" -v
  $PytestExit = $LASTEXITCODE
} finally {
  Pop-Location
}
exit $PytestExit
