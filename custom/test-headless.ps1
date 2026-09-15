param([Parameter(Mandatory=$true)][string]$Compiler)
$ErrorActionPreference='Stop'
$Compiler=(Resolve-Path $Compiler).Path
$root=Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force "$root/build" | Out-Null
$sources=@(Get-ChildItem "$root/core/src","$root/engine/src" -Recurse -Filter *.cpp | ForEach-Object FullName)
& $Compiler -std=c++20 -O2 -Wall -Wextra -Wpedantic -DSTICKY_HEADLESS_TEST "-I$root/core/include" "-I$root/engine/include" "-I$root/assets" @sources "$root/custom/simulator-tests.cpp" -o "$root/build/headless-tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Headless test build failed' }
& "$root/build/headless-tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Headless tests failed' }
