param([switch]$Clean)
$ErrorActionPreference = "Stop"
$root = (Resolve-Path $PSScriptRoot).Path
$toolchain = Join-Path $root "toolchain\x86_64-windows"
$toolBin = Join-Path $toolchain "bin"
$toolPython = Join-Path $toolchain "python\python.exe"
if (-not (Test-Path $toolPython)) { throw "Toolchain Python not found: $toolPython" }
if (-not (Test-Path (Join-Path $toolBin "arm-none-eabi-ar.exe"))) { throw "ARM toolchain not found: $toolBin" }
$env:FBT_NO_SYNC = "1"
$env:FBT_TOOLCHAIN_PATH = $root
$env:FBT_TOOLCHAIN_ROOT = $toolchain
$env:PYTHONHOME = Join-Path $toolchain "python"
$env:PYTHONPATH = ""
$env:PATH = "$toolBin;$($toolchain)\python;C:\Program Files\Git\cmd;C:\Program Files\Git\usr\bin;$env:PATH"
Push-Location $root
try {
    if ($Clean) { & $toolPython -m SCons -c fap_barista -j1; if ($LASTEXITCODE) { throw "Clean failed" } }
    & $toolPython -m SCons fap_barista -j1
    if ($LASTEXITCODE) { throw "Barista FAP build failed with exit code $LASTEXITCODE" }
    $source = Join-Path $root "build\f7-firmware-C\.extapps\barista.fap"
    if (-not (Test-Path -LiteralPath $source)) { throw "Build completed without $source" }
    $destination = Join-Path $root "dist\barista-test"
    New-Item -ItemType Directory -Path $destination -Force | Out-Null
    Copy-Item -LiteralPath $source -Destination (Join-Path $destination "barista.fap") -Force
    Get-Item -LiteralPath (Join-Path $destination "barista.fap") | Select-Object FullName, Length, LastWriteTime
} finally { Pop-Location }
