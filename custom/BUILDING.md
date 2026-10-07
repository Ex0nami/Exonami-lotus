# Build the Exonami edition

Custom development began from upstream commit `1e5dc875f1fb71e088657967f5b90f0a9f9b76ae` on 15 September 2026. Build tools are installed separately.

## Windows simulator

From the repository root, with w64devkit 2.10.0 and raylib 6.0 (Windows x64 MinGW build), substitute your tool paths:

```powershell
./custom/build-windows.ps1 -Compiler C:/tools/w64devkit/bin/g++.exe -RaylibRoot C:/tools/raylib-6.0_win64_mingw-w64
./build/sticky-lotus-simulator.exe
```

Mouse click is tap; hold and release changes ten life; drag is swipe. R rolls; D opens the dice selector. Keep generated assets beside the executable.

Graphical regression tests:

```powershell
Push-Location build
./simulator-tests.exe
Pop-Location
```

Tests without a graphics window:

```powershell
./custom/test-headless.ps1 -Compiler C:/tools/w64devkit/bin/g++.exe
```

The upstream CMake build also remains available. It needs CMake 3.24+, Ninja, a C++20 compiler, pkg-config and raylib with pkg-config metadata:

```sh
cmake -S . -B build -G Ninja
cmake --build build
./build/simulator/sticky-lotus-simulator
```

## Firmware

Activate an ESP-IDF **5.5.4** environment with the ESP32-S3 toolchain. From the repository root:

```sh
cd firmware
idf.py build
idf.py merge-bin
```

Outputs include `build/sticky_lotus_firmware.bin` (application), `build/merged-binary.bin` (installation image), and generated flash arguments with the required settings and offsets.

Run checks from the repository root using the ESP-IDF Python environment with pyelftools installed:

```sh
python custom/verify-dice-stack.py
python custom/verify-history-firmware.py
python custom/verify-menu-firmware.py
python custom/verify-profiles-firmware.py
python custom/verify-refresh-firmware.py
python custom/verify-sleep-firmware.py
```

The v1.3 implementation at `16d9b59` passed desktop regression tests, firmware compilation and firmware checks. Installation on 7 October 2026 verified the application flash, successful boot and restoration of the saved game. End-to-end device validation of the new profile and score interactions remains to be confirmed. Earlier gameplay, dice and refresh changes were tested on the Sticky. Desktop tests cannot reproduce physical touch, refresh timing or ghosting.

## Installing

For an existing Sticky Lotus installation with a matching partition table, use an application-only update to preserve NVS saved data. From `firmware/` in the activated ESP-IDF environment:

```sh
idf.py -p PORT app-flash
```

Replace `PORT` with the device port, for example `COM3`. Close serial monitors before flashing. Check the device's partition layout before using an application-only update.

For a first installation, `idf.py -p PORT flash` installs the complete build. Back up existing device data first. Do not erase flash if you want to preserve saved games, names and scores. Merged installation images and application-only images are not interchangeable.

After installation, check life changes, dice/coin results, player layouts, sleep/wake, and saved names/scores after restarting.

## Where to edit

- `core/`: game state, action history and player profiles.
- `engine/`: screens, gestures, layouts and menus.
- `platform/raylib/` and `simulator/`: desktop implementation.
- `firmware/`: ESP32-S3 hardware, display and persistence.
- `assets/`: icons and sleep artwork.
- `custom/`: regression checks, build helpers and development notes. Older notes describe their original build, not necessarily the current release.
