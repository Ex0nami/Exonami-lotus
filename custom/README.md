# Sticky Lotus custom: four-second life changes

## What this version does

Based on https://github.com/inkOne/sticky-lotus at upstream main commit
`1e5dc875f1fb71e088657967f5b90f0a9f9b76ae` (retrieved 15 September 2026).
Local branch: `custom/life-delta-4s`.

Upstream already implements **Roll for first** and cumulative signed life badges.
This version reuses them. The badges remain in the upstream player-panel corners,
near the corresponding minus/plus side, and rotate with the player.

- Seven minus taps at 40 show **33** and **-7**.
- Two plus taps during that burst show **35** and **-5**.
- Every actual life change restarts that player's four-second timer.
- Other players' actions do not extend the timer.
- At zero net change, the badge is hidden, matching upstream.
- The timer expires while settings/poison/Commander screens are open, without
  painting over those screens. A change after an expired burst starts afresh.
- Hold for at least 250 ms and release for +/-10, matching the physical device.
- Open the central lotus menu, then **Roll for first**. One configured player is
  shaded; upstream clears this after ten seconds without input. No dice animation.
- Poison and Commander delta timers remain at upstream's fifteen seconds.
- Life floor, elimination, rotation, reset, sound, persistence, and hardware
  refresh routines retain upstream behavior. Life badges are transient and are
  not added to saved games. Commander damage retains its separate upstream delta.

## Ready-to-run Windows simulator

In the release package, open `simulator/sticky-lotus-simulator.exe`.
Keep its `assets` folder next to it. It is a native Windows x64 executable.

Mouse click = tap; mouse hold and release = +/-10; drag = swipe.
Space opens settings; left/right arrow keys adjust Player 1.
The simulator uses desktop fonts/rendering; the physical e-paper appearance can differ.

## Build the simulator

From the source repository root, the upstream CMake route remains:

```sh
cmake -S . -B build -G Ninja
cmake --build build
./build/simulator/sticky-lotus-simulator
```

Requirements: CMake 3.24+, Ninja, a C++20 compiler, pkg-config, and raylib with its
pkg-config metadata. Run these commands from the repository root, not `firmware`.

For Windows without pkg-config, this package also provides a direct build script:

```powershell
./custom/build-windows.ps1 -Compiler C:/tools/w64devkit/bin/g++.exe -RaylibRoot C:/tools/raylib-6.0_win64_mingw-w64
./build/sticky-lotus-simulator.exe
```

Tested here with w64devkit 2.10.0 and raylib 6.0 (win64 MinGW build).
Get these tools from https://github.com/skeeto/w64devkit/releases/tag/v2.10.0 and
https://github.com/raysan5/raylib/releases/tag/6.0 . The script accepts absolute
paths to installations elsewhere; it does not download or install anything.

To run the included integration tests after building:

```powershell
Push-Location build
./simulator-tests.exe
Pop-Location
```

They run in a hidden graphics window for about 20 seconds and write screenshots
into the current directory. They exercise the real application, screen manager,
renderer, and Raylib canvas, using scripted gesture events. They do not automate
physical mouse input or test USB hardware.

## Build firmware

Use an activated **ESP-IDF v5.5.4** environment with the ESP32-S3 toolchain.
Install ESP-IDF with its recursive Git submodules. On Windows, a short checkout
path and Git `core.longpaths=true` avoid dependency checkout path limits.

```sh
cd firmware
idf.py build
idf.py merge-bin
```

The project selects ESP32-S3 from `sdkconfig.defaults`. Build outputs:

- `build/sticky_lotus_firmware.bin`: application only.
- `build/bootloader/bootloader.bin`: bootloader.
- `build/partition_table/partition-table.bin`: partition table.
- `build/ota_data_initial.bin`: initial OTA selection.
- `build/merged-binary.bin`: complete merged installation image.
- `build/flasher_args.json` and `build/flash_args`: authoritative offsets/settings.

The delivered image uses ESP32-S3, 32 MB flash, DIO, 80 MHz, octal PSRAM.
DIO is the actual generated flash setting, despite QIO appearing in upstream
`sdkconfig.defaults`; use the generated build settings when flashing.

## Flash the supplied image

The package contains `firmware/sticky-lotus-custom-merged.bin`. It is a merged
installation image, **not an OTA application upload**. Install at offset `0x0`.
No device was flashed during preparation.

With Python and esptool 4.12.0 available, from the release package directory:

```powershell
python -m esptool --chip esp32s3 --port COM5 --baud 460800 write_flash --flash_mode dio --flash_freq 80m --flash_size 32MB 0x0 firmware/sticky-lotus-custom-merged.bin
```

Replace `COM5` with your Sticky's port. This writes bootloader, partition table,
OTA selection and application. It does not issue a whole-device erase, but back
up any important device state before installing a custom firmware. Use your
second Sticky for the first hardware trial. If connection fails, use the
Sticky's documented USB/download-mode procedure; no unverified button sequence
is assumed here.

Alternatively, build from source and run `idf.py -p COM5 flash` from `firmware`.
The browser flasher's acceptance of this custom package has not been tested;
the esptool command above uses offsets verified from the actual build.

The individual files are also included, with `flasher_args.json` and `flash_args`.
For this build their offsets are:

| File | Offset |
| --- | --- |
| bootloader/bootloader.bin | 0x0 |
| partition_table/partition-table.bin | 0x8000 |
| ota_data_initial.bin | 0xf000 |
| sticky_lotus_firmware.bin | 0x20000 |

## Verification and limits

Passed desktop integration tests: cumulative taps, mixed signs, +/-10 gesture
handling, signed badge rendering and rotation, independent player timers,
timer extension, idle disappearance and refresh request, new bursts after
expiry, zero-net hiding, expiry during settings, reset, snapshot restoration,
elimination behavior, the actual settings roll button, and selection across all
configured players in both two- and four-player modes.

Saved screenshots were visually checked. ESP-IDF successfully built bootloader,
partition table and application and merged the binaries. The app uses about
579 KB and fits its configured OTA partition. `SHA256SUMS.txt` identifies the
release files. `BUILD-INFO.json` records the exact source and toolchain versions.

Upstream build warnings remain: unused `FOLLOWUP_*` settings in sdkconfig.defaults
and one GCC `#include_next` extension warning. These did not prevent the build.

Hardware touch, actual panel refresh/ghosting, sleep/wake, and persistence after
flashing remain to be tested on the reTerminal Sticky. The upstream roll chooses
among configured seats, including an eliminated seat; use it at game start.

## Keeping a future fork tidy

Production changes are separate commits:

1. Four-second life-delta behavior and cross-screen expiry.
2. Simulator portability include and mouse-hold support.
3. Custom build script, tests and these instructions.

The upstream README, firmware hardware code and build definitions are untouched.
`source.zip` contains the custom source; `custom-changes.patch` contains just the
commits above upstream. The working repository also retains full Git history and
the `origin` remote pointing to upstream. No remote fork was created or published.

To apply the patch to a separate checkout:

```sh
git checkout -b custom/life-delta-4s 1e5dc875f1fb71e088657967f5b90f0a9f9b76ae
git am /path/to/custom-changes.patch
```

MIT upstream license is retained. Separate third-party notices accompany the
simulator. The firmware package is an experimental custom build, not an official
Sticky Lotus release.
