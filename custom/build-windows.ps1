param(
    [Parameter(Mandatory=$true)][string]$Compiler,
    [Parameter(Mandatory=$true)][string]$RaylibRoot
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$Compiler = (Resolve-Path $Compiler).Path
$RaylibRoot = (Resolve-Path $RaylibRoot).Path
$build = Join-Path $root 'build'
New-Item -ItemType Directory -Force $build | Out-Null
$sources = @(Get-ChildItem "$root/core/src","$root/engine/src","$root/platform/raylib/src" -Filter *.cpp -Recurse | ForEach-Object FullName)
$flags = @('-std=c++20','-O2','-Wall','-Wextra','-Wpedantic',"-I$root/core/include", "-I$root/engine/include", "-I$root/platform/raylib/include", "-I$root/assets", "-I$RaylibRoot/include")
$links = @("-L$RaylibRoot/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm')
& $Compiler @flags @sources "$root/simulator/src/main.cpp" @links -o "$build/sticky-lotus-simulator.exe"
if ($LASTEXITCODE -ne 0) { throw 'Simulator build failed' }
Copy-Item "$root/assets" $build -Recurse -Force
if (Test-Path "$PSScriptRoot/simulator-tests.cpp") {
    & $Compiler @flags @sources "$PSScriptRoot/simulator-tests.cpp" @links -o "$build/simulator-tests.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Simulator test build failed' }
}
