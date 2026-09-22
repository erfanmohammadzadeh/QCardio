# Build and run QCardio Qt tests with the MinGW kit.
# qmake needs g++ on PATH; a normal PowerShell session does not have that.
#
# Usage (from src/tests):
#   .\run_tests.ps1
#
# Override kit locations if yours differ:
#   $env:QT_MINGW_BIN = "C:\Qt\6.8.2\mingw_64\bin"
#   $env:MINGW_BIN    = "C:\Qt\Tools\mingw1310_64\bin"

$ErrorActionPreference = "Stop"

$testsRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$qtBin = if ($env:QT_MINGW_BIN) { $env:QT_MINGW_BIN } else { "C:\Qt\6.8.2\mingw_64\bin" }
$mingwBin = if ($env:MINGW_BIN) { $env:MINGW_BIN } else { "C:\Qt\Tools\mingw1310_64\bin" }

$qmake = Join-Path $qtBin "qmake.exe"
$gpp = Join-Path $mingwBin "g++.exe"
$make = Join-Path $mingwBin "mingw32-make.exe"

if (-not (Test-Path $qmake)) {
    throw "qmake not found: $qmake`nSet QT_MINGW_BIN to your Qt MinGW bin directory."
}
if (-not (Test-Path $gpp)) {
    throw "g++ not found: $gpp`nSet MINGW_BIN to your MinGW bin directory (same kit Qt Creator uses)."
}
if (-not (Test-Path $make)) {
    throw "mingw32-make not found: $make`nSet MINGW_BIN to your MinGW bin directory."
}

$env:PATH = "$mingwBin;$qtBin;" + $env:PATH

$buildDir = Join-Path $testsRoot "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
Set-Location $buildDir

& $qmake (Join-Path $testsRoot "tests.pro") -spec win32-g++ "CONFIG+=debug"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $make -j8
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$exe = Join-Path $buildDir "debug\tst_qcardio.exe"
if (-not (Test-Path $exe)) {
    throw "Test binary was not produced: $exe"
}

& $exe -o -,txt
exit $LASTEXITCODE
